#pragma once

namespace leanclr
{
namespace log
{

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4
#define LOG_LEVEL_NONE 5

#ifndef LEANCLR_LOG_LEVEL
#if LEANCLR_DEBUG
#define LEANCLR_LOG_LEVEL LOG_LEVEL_DEBUG
#else
#define LEANCLR_LOG_LEVEL LOG_LEVEL_INFO
#endif
#endif

class InternalLogger
{
  public:
    static void debug(const char* message);
    static void info(const char* message);
    static void warning(const char* message);
    static void error(const char* message);
    static void fatal(const char* message);
};
} // namespace log
} // namespace leanclr