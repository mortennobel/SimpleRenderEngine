/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include "sre/Log.hpp"
#include <iostream>
#include <cstdarg>

namespace sre{
    constexpr int maxErrorSize = 255;
    char errorMsg[255];
    std::function<void(const char * function,const char * file, int line,LogType, std::string)> Log::logHandler = [](const char * function,const char * file, int line, LogType type, std::string msg){
        switch (type){
            case LogType::Verbose:
                std::cout <<"SRE Verbose: ";
                std::cout <<file<<":"<<line<<" in "<<function<<"()\n";
            case LogType::Info:
                // No prefix
                break;
            case LogType::Warning:
                std::cout <<"SRE Warning: ";
                std::cout <<file<<":"<<line<<" in "<<function<<"()\n";
                break;
            case LogType::Error:
                std::cout <<"SRE Error: ";
                std::cout <<file<<":"<<line<<" in "<<function<<"()\n";
                break;
            case LogType::Fatal:
                std::cout <<"SRE Error: ";
                std::cout <<file<<":"<<line<<" in "<<function<<"()\n";
                break;
        }
        std::cout <<msg<<std::endl;
        if (type == LogType::Fatal){
            throw std::runtime_error(msg);
        }
    };

    void Log::verbose(const char * function,const char * file, int line, const char *message, ...) {
        va_list args;
        va_start(args, message);
        vsnprintf(errorMsg,maxErrorSize,message,args);
        logHandler(function,file, line, LogType::Verbose, errorMsg);
        va_end(args);
    }

    void Log::info(const char * function,const char * file, int line, const char *message, ...) {
        va_list args;
        va_start(args, message);
        vsnprintf(errorMsg,maxErrorSize,message,args);
        logHandler(function,file, line, LogType::Info, errorMsg);
        va_end(args);
    }

    void Log::warning(const char * function,const char * file, int line, const char *message, ...) {
        va_list args;
        va_start(args, message);
        vsnprintf(errorMsg,maxErrorSize,message,args);
        logHandler(function,file, line, LogType::Warning, errorMsg);
        va_end(args);
    }

    void Log::error(const char * function,const char * file, int line, const char *message, ...) {
        va_list args;
        va_start(args, message);
        vsnprintf(errorMsg,maxErrorSize,message,args);
        logHandler(function,file, line, LogType::Error, errorMsg);
        va_end(args);
    }

    void Log::fatal(const char * function,const char * file, int line, const char *message, ...) {
        va_list args;
        va_start(args, message);
        vsnprintf(errorMsg,maxErrorSize,message,args);
        logHandler(function,file, line, LogType::Fatal, errorMsg);
        va_end(args);
    }
}