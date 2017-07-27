/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */
#pragma once

#include <string>
#include <functional>

// Logging in SRE is done using the preprocessor macros:
//
// LOG_VERBOSE(format, ...)
// LOG_INFO(format, ...)
// LOG_WARNING(format, ...)
// LOG_ERROR(format, ...)
// LOG_FATAL(format, ...)
//
// Each function works similar to printf, e.g.
// LOG_INFO("Hello %s. Meaning of life: %i", "world, 42); // prints "Hello world Meaning of life: 42"
//
// If the symbol SRE_LOG_DISABLED is defined all logging is disabled
//
// The default behavior (defined in the logHandler - which may be overwritten) is verbose logging is ignored and
// fatal errors throws a std::runtime_error
//
namespace sre{
    enum class LogType {
        Verbose,
        Info,
        Warning,
        Error,
        Fatal
    };

    class Log {
    public:
        static void verbose(const char * function,const char * file, int line, const char * format, ...);
        static void info(const char * function,const char * file, int line, const char * format, ...);
        static void warning(const char * function,const char * file, int line, const char * format, ...);
        static void error(const char * function,const char * file, int line, const char * format, ...);
        static void fatal(const char * function,const char * file, int line, const char * format, ...);

        static std::function<void(const char * function,const char * file, int line, LogType type, std::string msg)> logHandler;
    };
}

#ifdef SRE_LOG_DISABLED
#define LOG_VERBOSE(X, ...)
#define LOG_INFO(X, ...)
#define LOG_WARNING(X, ...)
#define LOG_ERROR(X, ...)
#define LOG_FATAL(X, ...)
#else
#define LOG_LOCATION __func__, __FILE__,__LINE__
#ifdef SRE_LOG_VERBOSE
#define LOG_VERBOSE(X, ...) sre::Log::verbose(LOG_LOCATION, X,##  __VA_ARGS__)
#else
#define LOG_VERBOSE(X, ...)
#endif
#define LOG_INFO(X, ...) sre::Log::info(LOG_LOCATION, X, ## __VA_ARGS__)
#define LOG_WARNING(X, ...) sre::Log::warning(LOG_LOCATION, X, ## __VA_ARGS__)
#define LOG_ERROR(X, ...) sre::Log::error(LOG_LOCATION, X,##  __VA_ARGS__)
#define LOG_FATAL(X, ...) sre::Log::fatal(LOG_LOCATION, X,##  __VA_ARGS__)
#endif