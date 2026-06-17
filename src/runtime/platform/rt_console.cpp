#include "rt_console.h"

#if LEANCLR_PLATFORM_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstring>

namespace leanclr
{
namespace platform
{

namespace
{

int32_t g_cols_and_lines = -1;

HANDLE get_std_input_handle() noexcept
{
    return GetStdHandle(STD_INPUT_HANDLE);
}

} // namespace

bool RtConsole::isatty(intptr_t handle) noexcept
{
    DWORD mode = 0;
    return GetConsoleMode(reinterpret_cast<HANDLE>(handle), &mode) != 0;
}

int32_t RtConsole::internal_key_available(int32_t timeout_msec) noexcept
{
    HANDLE input = get_std_input_handle();
    if (input == INVALID_HANDLE_VALUE || input == nullptr)
    {
        return 0;
    }

    const DWORD deadline = timeout_msec >= 0 ? GetTickCount() + static_cast<DWORD>(timeout_msec) : 0;
    for (;;)
    {
        DWORD event_count = 0;
        if (!GetNumberOfConsoleInputEvents(input, &event_count))
        {
            return 0;
        }

        if (event_count > 0)
        {
            INPUT_RECORD buffer[64];
            DWORD peeked = 0;
            const DWORD batch = event_count > 64 ? 64 : event_count;
            if (!PeekConsoleInput(input, buffer, batch, &peeked))
            {
                return 0;
            }

            int32_t keys = 0;
            for (DWORD i = 0; i < peeked; ++i)
            {
                if (buffer[i].EventType == KEY_EVENT && buffer[i].Event.KeyEvent.bKeyDown)
                {
                    ++keys;
                }
            }
            return keys;
        }

        if (timeout_msec < 0)
        {
            return 0;
        }

        if (GetTickCount() >= deadline)
        {
            return 0;
        }

        Sleep(10);
    }
}

bool RtConsole::set_echo(bool enable) noexcept
{
    HANDLE input = get_std_input_handle();
    if (input == INVALID_HANDLE_VALUE || input == nullptr)
    {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(input, &mode))
    {
        return false;
    }

    if (enable)
    {
        mode |= ENABLE_ECHO_INPUT;
    }
    else
    {
        mode &= ~static_cast<DWORD>(ENABLE_ECHO_INPUT);
    }

    return SetConsoleMode(input, mode) != 0;
}

bool RtConsole::set_break(bool enable) noexcept
{
    HANDLE input = get_std_input_handle();
    if (input == INVALID_HANDLE_VALUE || input == nullptr)
    {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(input, &mode))
    {
        return false;
    }

    // ENABLE_PROCESSED_INPUT allows Ctrl+C to raise a break signal; clearing it
    // matches TreatControlCAsInput / SetBreak(false) semantics on Unix.
    if (enable)
    {
        mode |= ENABLE_PROCESSED_INPUT;
    }
    else
    {
        mode &= ~static_cast<DWORD>(ENABLE_PROCESSED_INPUT);
    }

    return SetConsoleMode(input, mode) != 0;
}

bool RtConsole::tty_setup(const char* /*keypad_xmit*/, const char* /*teardown*/, TtySetupResult* out_result) noexcept
{
    if (out_result == nullptr)
    {
        return false;
    }

    std::memset(out_result->control_chars, 0, static_cast<size_t>(kControlCharsCount));

    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE || output == nullptr)
    {
        g_cols_and_lines = -1;
        out_result->cols_and_lines = g_cols_and_lines;
        return false;
    }

    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(output, &info))
    {
        g_cols_and_lines = -1;
        out_result->cols_and_lines = g_cols_and_lines;
        return false;
    }

    const int32_t cols = static_cast<int32_t>(info.srWindow.Right - info.srWindow.Left + 1);
    const int32_t rows = static_cast<int32_t>(info.srWindow.Bottom - info.srWindow.Top + 1);
    g_cols_and_lines = (cols << 16) | rows;
    out_result->cols_and_lines = g_cols_and_lines;

    // Terminfo setup is Unix-only; return false like Mono on Windows.
    return false;
}

int32_t* RtConsole::get_cols_and_lines_ptr() noexcept
{
    return &g_cols_and_lines;
}

} // namespace platform
} // namespace leanclr

#elif LEANCLR_PLATFORM_POSIX
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace leanclr
{
namespace platform
{

namespace
{

bool g_setup_finished = false;
bool g_atexit_registered = false;
char* g_teardown_str = nullptr;
char* g_keypad_xmit_str = nullptr;
termios g_initial_attr{};
termios g_mono_attr{};
int32_t g_cols_and_lines = -1;

#if !LEANCLR_PLATFORM_WASM && defined(SIGCONT)
struct sigaction g_save_sigcont{};
struct sigaction g_save_sigwinch{};
#endif

bool set_termios_lflag_property(tcflag_t property, bool value) noexcept
{
    termios attr{};
    if (tcgetattr(STDIN_FILENO, &attr) == -1)
    {
        return false;
    }

    const bool has_property = (attr.c_lflag & property) != 0;
    if (value == has_property)
    {
        return true;
    }

    if (value)
    {
        attr.c_lflag |= property;
    }
    else
    {
        attr.c_lflag &= ~property;
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &attr) == -1)
    {
        return false;
    }

    g_mono_attr = attr;
    return true;
}

#ifdef TIOCGWINSZ
int32_t terminal_get_dimensions() noexcept
{
    winsize ws{};
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
    {
        return (static_cast<int32_t>(ws.ws_col) << 16) | static_cast<int32_t>(ws.ws_row);
    }
    return -1;
}
#else
int32_t terminal_get_dimensions() noexcept
{
    return -1;
}
#endif

int32_t terminal_get_dimensions_from_env() noexcept
{
    int32_t dims = terminal_get_dimensions();
    if (dims != -1)
    {
        return dims;
    }

    int32_t cols = 0;
    int32_t rows = 0;
    if (const char* columns = std::getenv("COLUMNS"))
    {
        cols = static_cast<int32_t>(std::atoi(columns));
    }
    if (const char* lines = std::getenv("LINES"))
    {
        rows = static_cast<int32_t>(std::atoi(lines));
    }
    if (cols != 0 && rows != 0)
    {
        return (cols << 16) | rows;
    }
    return -1;
}

void fill_control_chars(uint8_t* dest, const unsigned char* cc) noexcept
{
    std::memset(dest, 0, static_cast<size_t>(RtConsole::kControlCharsCount));
#ifdef VINTR
    dest[0] = cc[VINTR];
#endif
#ifdef VQUIT
    dest[1] = cc[VQUIT];
#endif
#ifdef VERASE
    dest[2] = cc[VERASE];
#endif
#ifdef VKILL
    dest[3] = cc[VKILL];
#endif
#ifdef VEOF
    dest[4] = cc[VEOF];
#endif
#ifdef VTIME
    dest[5] = cc[VTIME];
#endif
#ifdef VMIN
    dest[6] = cc[VMIN];
#endif
#ifdef VSWTC
    dest[7] = cc[VSWTC];
#endif
#ifdef VSTART
    dest[8] = cc[VSTART];
#endif
#ifdef VSTOP
    dest[9] = cc[VSTOP];
#endif
#ifdef VSUSP
    dest[10] = cc[VSUSP];
#endif
#ifdef VEOL
    dest[11] = cc[VEOL];
#endif
#ifdef VREPRINT
    dest[12] = cc[VREPRINT];
#endif
#ifdef VDISCARD
    dest[13] = cc[VDISCARD];
#endif
#ifdef VWERASE
    dest[14] = cc[VWERASE];
#endif
#ifdef VLNEXT
    dest[15] = cc[VLNEXT];
#endif
#ifdef VEOL2
    dest[16] = cc[VEOL2];
#endif
}

void tty_teardown() noexcept
{
    if (!g_setup_finished)
    {
        return;
    }

    if (g_teardown_str != nullptr)
    {
        const size_t len = std::strlen(g_teardown_str);
        if (len > 0)
        {
            (void)write(STDOUT_FILENO, g_teardown_str, len);
        }
        std::free(g_teardown_str);
        g_teardown_str = nullptr;
    }

    tcflush(STDIN_FILENO, TCIFLUSH);
    tcsetattr(STDIN_FILENO, TCSANOW, &g_initial_attr);
    set_termios_lflag_property(ECHO, true);
    g_setup_finished = false;
}

#if !LEANCLR_PLATFORM_WASM && defined(SIGCONT)
void sigcont_handler(int) noexcept
{
    tcsetattr(STDIN_FILENO, TCSANOW, &g_mono_attr);
    if (g_keypad_xmit_str != nullptr)
    {
        const size_t len = std::strlen(g_keypad_xmit_str);
        if (len > 0)
        {
            (void)write(STDOUT_FILENO, g_keypad_xmit_str, len);
        }
    }

    if (g_save_sigcont.sa_handler != SIG_DFL && g_save_sigcont.sa_handler != SIG_IGN && g_save_sigcont.sa_handler != nullptr)
    {
        g_save_sigcont.sa_handler(0);
    }
}

void sigwinch_handler(int) noexcept
{
    const int32_t dims = terminal_get_dimensions();
    if (dims != -1)
    {
        g_cols_and_lines = dims;
    }

    if (g_save_sigwinch.sa_handler != SIG_DFL && g_save_sigwinch.sa_handler != SIG_IGN && g_save_sigwinch.sa_handler != nullptr)
    {
        g_save_sigwinch.sa_handler(0);
    }
}

void install_console_signal_handlers() noexcept
{
    struct sigaction sigcont{};
    struct sigaction sigwinch{};

    std::memset(&sigcont, 0, sizeof(sigcont));
    std::memset(&sigwinch, 0, sizeof(sigwinch));

    sigcont.sa_handler = sigcont_handler;
    sigemptyset(&sigcont.sa_mask);
    sigaction(SIGCONT, &sigcont, &g_save_sigcont);

    sigwinch.sa_handler = sigwinch_handler;
    sigemptyset(&sigwinch.sa_mask);
    sigaction(SIGWINCH, &sigwinch, &g_save_sigwinch);
}
#endif

char* duplicate_cstr(const char* src) noexcept
{
    if (src == nullptr)
    {
        return nullptr;
    }
    const size_t len = std::strlen(src);
    char* copy = static_cast<char*>(std::malloc(len + 1));
    if (copy == nullptr)
    {
        return nullptr;
    }
    std::memcpy(copy, src, len + 1);
    return copy;
}

} // namespace

bool RtConsole::isatty(intptr_t handle) noexcept
{
    return ::isatty(static_cast<int>(handle)) != 0;
}

int32_t RtConsole::internal_key_available(int32_t timeout_msec) noexcept
{
    fd_set rfds{};
    timeval tv{};
    timeval* tvptr = nullptr;

    int ret = 0;
    do
    {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        if (timeout_msec >= 0)
        {
            tv.tv_sec = timeout_msec / 1000;
            tv.tv_usec = (timeout_msec % 1000) * 1000;
            tvptr = &tv;
        }
        ret = select(STDIN_FILENO + 1, &rfds, nullptr, nullptr, tvptr);
    } while (ret == -1 && errno == EINTR);

    if (ret > 0)
    {
        int nbytes = 0;
        ret = ioctl(STDIN_FILENO, FIONREAD, &nbytes);
        if (ret >= 0)
        {
            ret = nbytes;
        }
    }

    return ret > 0 ? ret : 0;
}

bool RtConsole::set_echo(bool enable) noexcept
{
    return set_termios_lflag_property(ECHO, enable);
}

bool RtConsole::set_break(bool enable) noexcept
{
    return set_termios_lflag_property(IGNBRK, !enable);
}

bool RtConsole::tty_setup(const char* keypad_xmit, const char* teardown, TtySetupResult* out_result) noexcept
{
    if (out_result == nullptr)
    {
        return false;
    }

    g_cols_and_lines = terminal_get_dimensions_from_env();
    out_result->cols_and_lines = g_cols_and_lines;

    if (tcgetattr(STDIN_FILENO, &g_initial_attr) == -1)
    {
        return false;
    }

    g_mono_attr = g_initial_attr;
    g_mono_attr.c_lflag &= ~static_cast<tcflag_t>(ICANON);
    g_mono_attr.c_iflag &= ~static_cast<tcflag_t>(IXON | IXOFF);
    g_mono_attr.c_cc[VMIN] = 1;
    g_mono_attr.c_cc[VTIME] = 0;
#ifdef VDSUSP
    g_mono_attr.c_cc[VDSUSP] = static_cast<cc_t>(255);
#endif
    if (tcsetattr(STDIN_FILENO, TCSANOW, &g_mono_attr) == -1)
    {
        return false;
    }

    fill_control_chars(out_result->control_chars, g_mono_attr.c_cc);

    if (g_setup_finished)
    {
        return true;
    }

    std::free(g_keypad_xmit_str);
    g_keypad_xmit_str = duplicate_cstr(keypad_xmit);

#if !LEANCLR_PLATFORM_WASM && defined(SIGCONT)
    install_console_signal_handlers();
#endif

    g_setup_finished = true;
    if (!g_atexit_registered)
    {
        if (teardown != nullptr)
        {
            std::free(g_teardown_str);
            g_teardown_str = duplicate_cstr(teardown);
        }
        std::atexit(tty_teardown);
        g_atexit_registered = true;
    }

    return true;
}

int32_t* RtConsole::get_cols_and_lines_ptr() noexcept
{
    return &g_cols_and_lines;
}

} // namespace platform
} // namespace leanclr

#endif
