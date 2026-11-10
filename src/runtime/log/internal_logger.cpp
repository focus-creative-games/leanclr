#include <cstdio>

#include "internal_logger.h"
#include "host/services.h"

namespace leanclr
{
namespace log
{

void InternalLogger::debug(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_DEBUG
    host::Services::log(host::LogLevel::Debug, message);
#endif
}

void InternalLogger::info(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_INFO
    host::Services::log(host::LogLevel::Info, message);
#endif
}

void InternalLogger::warning(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_WARNING
    host::Services::log(host::LogLevel::Warning, message);
#endif
}

void InternalLogger::error(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_ERROR
    host::Services::log(host::LogLevel::Error, message);
#endif
}

void InternalLogger::fatal(const char* message)
{
#if LEANCLR_LOG_LEVEL <= LOG_LEVEL_FATAL
    host::Services::log(host::LogLevel::Fatal, message);
#endif
}

} // namespace log
} // namespace leanclr