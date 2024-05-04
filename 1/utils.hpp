#pragma once

#include <iostream>
#include <cstdarg>
#include <string>
#include <chrono>
#include <Windows.h>

// Timer auxiliary class for accurate measurement of time intervals
class muTimer
{
    using Clock = std::chrono::high_resolution_clock;
    bool active = false;
    Clock::duration   duration_;
    Clock::time_point start_ = Clock::now(), stop_ = Clock::now();

    muTimer(const muTimer&)             = delete;
    muTimer& operator=(const muTimer&)  = delete;
public:
    using ns       = std::chrono::nanoseconds;
    using mks      = std::chrono::microseconds;
    using ms       = std::chrono::milliseconds;
    muTimer() { reset(); start(); }
    ~muTimer() = default;
    muTimer& reset()
    {
        duration_ = std::chrono::nanoseconds(0);
        active    = false;
        return *this;
    }
    muTimer& start()
    {
        if (!active)
        {
            start_ = Clock::now();
            active = true;
        }
        return *this;
    }
    muTimer& stop()
    {
        if (active)
        {
            stop_      = Clock::now();
            duration_ += stop_ - start_;
            active     = false;
        }
        return *this;
    }
    template<typename T = mks>
        unsigned long long duration()
    {
        return static_cast<unsigned long long>
            (std::chrono::duration_cast<T>(stop_-start_).count());
    }
};


// Console output in color (Windows)

enum {
    BLACK             = 0,
    DARKBLUE          = FOREGROUND_BLUE,
    DARKGREEN         = FOREGROUND_GREEN,
    DARKCYAN          = FOREGROUND_GREEN | FOREGROUND_BLUE,
    DARKRED           = FOREGROUND_RED,
    DARKMAGENTA       = FOREGROUND_RED | FOREGROUND_BLUE,
    DARKYELLOW        = FOREGROUND_RED | FOREGROUND_GREEN,
    DARKGRAY          = FOREGROUND_INTENSITY,
    GRAY              = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    BLUE              = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    GREEN             = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    CYAN              = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    RED               = FOREGROUND_INTENSITY | FOREGROUND_RED,
    MAGENTA           = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    YELLOW            = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    WHITE             = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};

struct ConsoleAttribute
{
    unsigned int att; //green = FOREGROUND_GREEN
};

ConsoleAttribute attr(unsigned int x)
{
    return ConsoleAttribute{x};
}

// The new type is introduced just so that an output statement can be written,
// which calls SetConsoleTextAttribute with the desired color

std::ostream& operator << (std::ostream& os, const ConsoleAttribute& att)
{
    if (&os == &std::cout)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(att.att));
    return os;
}

// Get string in sprintf-style
std::string stringf(const char * fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    int len = std::vsnprintf(nullptr,0,fmt,args);
    std::string s(len,' ');
    std::vsnprintf(s.data(),len+1,fmt,args);
    va_end(args);
    return s;
}


