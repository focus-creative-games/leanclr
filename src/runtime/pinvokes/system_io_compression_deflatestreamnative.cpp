#include "build_config.h"
#include "metadata/rt_metadata.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

#if LEANCLR_USE_SHIPPING_ZLIB
#include "3rd/zlib/zlib.h"
#else
#include "../external/zlib/zlib.h"
#endif

namespace leanclr
{
namespace pinvokes
{

enum class CompressMode : int32_t
{
    Decompress = 0,
    Compress = 1,
};

using NativeReadOrWrite = int32_t (*)(intptr_t buffer, int32_t length, intptr_t user_data);

class DeflateStream
{
public:
    static constexpr int32_t kArgumentError = -10;
    static constexpr int32_t kIoError = -11;

    static DeflateStream* Create(
        CompressMode mode,
        bool gzip,
        NativeReadOrWrite callback,
        void* user_data)
    {
        if (callback == nullptr)
        {
            return nullptr;
        }
        auto* zs = static_cast<z_stream*>(calloc(1, sizeof(z_stream)));
        if (zs == nullptr)
        {
            return nullptr;
        }

        const bool compress = mode == CompressMode::Compress;
        const int window_bits = gzip ? 31 : -15;
        const int init_status = compress
            ? deflateInit2(zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, window_bits, 8, Z_DEFAULT_STRATEGY)
            : inflateInit2(zs, window_bits);

        if (init_status != Z_OK)
        {
            free(zs);
            return nullptr;
        }

        auto* stream = new (std::nothrow) DeflateStream();
        if (stream == nullptr)
        {
            if (compress)
            {
                deflateEnd(zs);
            }
            else
            {
                inflateEnd(zs);
            }
            free(zs);
            return nullptr;
        }

        stream->m_io_buffer = static_cast<uint8_t*>(malloc(kIoBufferSize));
        if (stream->m_io_buffer == nullptr)
        {
            if (compress)
            {
                deflateEnd(zs);
            }
            else
            {
                inflateEnd(zs);
            }
            free(zs);
            delete stream;
            return nullptr;
        }

        zs->zalloc = ZlibAlloc;
        zs->zfree = ZlibFree;
        zs->next_out = stream->m_io_buffer;
        zs->avail_out = kIoBufferSize;
        zs->total_in = 0;

        stream->m_zlib_stream = zs;
        stream->m_io_callback = callback;
        stream->m_user_data = user_data;
        stream->m_is_compress = compress ? 1 : 0;
        stream->m_at_eof = 0;
        stream->m_bytes_fed = 0;
        return stream;
    }

    int32_t Close()
    {
        if (m_closed)
        {
            return 0;
        }

        int32_t status = 0;
        int32_t flush_status = 0;
        z_stream* zs = m_zlib_stream;

        if (m_is_compress)
        {
            if (zs->total_in > 0)
            {
                do
                {
                    status = deflate(zs, Z_FINISH);
                    flush_status = FlushCompressOutput(true);
                } while (status == Z_OK);

                if (status == Z_STREAM_END)
                {
                    status = flush_status;
                }
            }

            deflateEnd(zs);
        }
        else
        {
            inflateEnd(zs);
        }

        ReleaseBuffers();
        m_closed = true;
        return status;
    }

    int32_t Flush()
    {
        return FlushCompressOutput(false);
    }

    int32_t Read(void* buffer, int32_t count)
    {
        auto* out = static_cast<uint8_t*>(buffer);
        if (out == nullptr || count < 0)
        {
            return kArgumentError;
        }

        if (m_at_eof)
        {
            return 0;
        }

        z_stream* zs = m_zlib_stream;
        zs->next_out = out;
        zs->avail_out = static_cast<uInt>(count);

        while (zs->avail_out > 0)
        {
            if (zs->avail_in == 0)
            {
                const intptr_t buffer_ptr = reinterpret_cast<intptr_t>(m_io_buffer);
                const intptr_t user_data_ptr = reinterpret_cast<intptr_t>(m_user_data);
                int32_t read = m_io_callback(buffer_ptr, kIoBufferSize, user_data_ptr);
                if (read < 0)
                {
                    read = 0;
                }

                m_bytes_fed += static_cast<uint32_t>(read);
                zs->next_in = m_io_buffer;
                zs->avail_in = static_cast<uInt>(read);
            }

            const int status = inflate(zs, Z_SYNC_FLUSH);
            if (status == Z_STREAM_END)
            {
                m_at_eof = 1;
                break;
            }

            if (status == Z_BUF_ERROR && m_bytes_fed == zs->total_in)
            {
                if (zs->avail_in != 0)
                {
                    m_at_eof = 1;
                }
                break;
            }

            if (status != Z_OK)
            {
                return status;
            }
        }

        return count - static_cast<int32_t>(zs->avail_out);
    }

    int32_t Write(const void* buffer, int32_t count)
    {
        auto* in = static_cast<const uint8_t*>(buffer);
        if (in == nullptr || count < 0)
        {
            return kArgumentError;
        }

        if (m_at_eof)
        {
            return kIoError;
        }

        z_stream* zs = m_zlib_stream;
        zs->next_in = const_cast<uint8_t*>(in);
        zs->avail_in = static_cast<uInt>(count);

        while (zs->avail_in > 0)
        {
            if (zs->avail_out == 0)
            {
                zs->next_out = m_io_buffer;
                zs->avail_out = kIoBufferSize;
            }

            const int status = deflate(zs, Z_NO_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END)
            {
                return status;
            }

            if (zs->avail_out == 0)
            {
                const int32_t push_status = PushOutputToManaged();
                if (push_status < 0)
                {
                    return push_status;
                }
            }
        }

        return count;
    }

    ~DeflateStream()
    {
        if (!m_closed)
        {
            Close();
        }
    }

    DeflateStream(const DeflateStream&) = delete;
    DeflateStream& operator=(const DeflateStream&) = delete;

private:
    static constexpr int32_t kIoBufferSize = 4096;

    DeflateStream() = default;

    static void* ZlibAlloc(void*, uint32_t item_count, uint32_t item_size)
    {
        return calloc(item_count, item_size);
    }

    static void ZlibFree(void*, void* ptr)
    {
        free(ptr);
    }

    int32_t PushOutputToManaged()
    {
        z_stream* zs = m_zlib_stream;
        if (zs->avail_out == kIoBufferSize)
        {
            return 0;
        }

        const int32_t bytes_ready = kIoBufferSize - static_cast<int32_t>(zs->avail_out);
        const intptr_t buffer_ptr = reinterpret_cast<intptr_t>(m_io_buffer);
        const intptr_t user_data_ptr = reinterpret_cast<intptr_t>(m_user_data);
        const int32_t written = m_io_callback(buffer_ptr, bytes_ready, user_data_ptr);

        zs->next_out = m_io_buffer;
        zs->avail_out = kIoBufferSize;
        if (written < 0)
        {
            return kIoError;
        }

        return 0;
    }

    int32_t FlushCompressOutput(bool final_flush)
    {
        if (!m_is_compress)
        {
            return 0;
        }

        z_stream* zs = m_zlib_stream;
        if (!final_flush && zs->avail_in != 0)
        {
            const int32_t status = deflate(zs, Z_PARTIAL_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END)
            {
                return status;
            }
        }

        return PushOutputToManaged();
    }

    void ReleaseBuffers()
    {
        if (m_io_buffer != nullptr)
        {
            free(m_io_buffer);
            m_io_buffer = nullptr;
        }

        if (m_zlib_stream != nullptr)
        {
            free(m_zlib_stream);
            m_zlib_stream = nullptr;
        }
    }

    z_stream* m_zlib_stream = nullptr;
    uint8_t* m_io_buffer = nullptr;
    NativeReadOrWrite m_io_callback = nullptr;
    void* m_user_data = nullptr;
    uint8_t m_is_compress = 0;
    uint8_t m_at_eof = 0;
    uint32_t m_bytes_fed = 0;
    bool m_closed = false;
};

} // namespace pinvokes
} // namespace leanclr

extern "C"
{
intptr_t LEANCLR_PINVOKE_CALL_CDECL CreateZStream(
    leanclr::pinvokes::CompressMode compress_mode,
    bool gzip,
    leanclr::pinvokes::NativeReadOrWrite freader,
    void* data)
{
    leanclr::pinvokes::DeflateStream* stream =
        leanclr::pinvokes::DeflateStream::Create(compress_mode, gzip, freader, data);
    return reinterpret_cast<intptr_t>(stream);
}

int32_t LEANCLR_PINVOKE_CALL_CDECL Flush(intptr_t zstream)
{
    auto* stream = reinterpret_cast<leanclr::pinvokes::DeflateStream*>(zstream);
    return stream->Flush();
}

int32_t LEANCLR_PINVOKE_CALL_CDECL CloseZStream(intptr_t zstream)
{
    auto* stream = reinterpret_cast<leanclr::pinvokes::DeflateStream*>(zstream);
    if (stream == nullptr)
    {
        return leanclr::pinvokes::DeflateStream::kArgumentError;
    }

    const int32_t status = stream->Close();
    delete stream;
    return status;
}

int32_t LEANCLR_PINVOKE_CALL_CDECL ReadZStream(intptr_t zstream, void* buffer, int32_t count)
{
    auto* stream = reinterpret_cast<leanclr::pinvokes::DeflateStream*>(zstream);
    if (stream == nullptr)
    {
        return leanclr::pinvokes::DeflateStream::kArgumentError;
    }

    return stream->Read(buffer, count);
}

int32_t LEANCLR_PINVOKE_CALL_CDECL WriteZStream(intptr_t zstream, const void* buffer, int32_t count)
{
    auto* stream = reinterpret_cast<leanclr::pinvokes::DeflateStream*>(zstream);
    if (stream == nullptr)
    {
        return leanclr::pinvokes::DeflateStream::kArgumentError;
    }

    return stream->Write(buffer, count);
}
}
