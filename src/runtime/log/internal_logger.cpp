#include <cstdio>

#include "internal_logger.h"

namespace leanclr
{
namespace log
{

void InternalLogger::debug(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_DEBUG
    std::printf("[DEBUG] %s\n", message);
#endif
}

void InternalLogger::info(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_INFO
    std::printf("[INFO] %s\n", message);
#endif
}

void InternalLogger::warning(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_WARNING
    std::printf("[WARNING] %s\n", message);
#endif
}

void InternalLogger::error(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_ERROR
    std::printf("[ERROR] %s\n", message);
#endif
}

void InternalLogger::fatal(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_FATAL
    std::printf("[FATAL] %s\n", message);
#endif
}

} // namespace log
} // namespace leanclr