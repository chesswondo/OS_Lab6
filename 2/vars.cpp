#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <conio.h>
#include "utils.hpp"

using namespace std;

constexpr size_t Count = 10'000'000;  // Number of summations

int var = 0;                          // Shared variable
atomic<int> V = 0;                    // Atomic variable
mutex m_var;                          // Mutex

// Variable increment function using mutex
void addMutex()
{
    for(int i = 0; i < Count; ++i)
    {
        lock_guard<mutex> guard(m_var);
        var = var + 1;
    }
}

// Variable increase function without synchronization
// This code must be compiled with optimization turned off!
#pragma optimize("",off)
void addFree()
{
    for(int i = 0; i < Count; ++i)
    {
        var = var + 1;
    }
}
#pragma optimize("",on)

// Function for fast increasing an atomic variable using increment
void addAtom()
{
    for(int i = 0; i < Count; ++i)
    {
        V++;
    }
}

// Function for fast increasing an atomic variable using fetch_add
void addAtomF()
{
    for(int i = 0; i < Count; ++i)
    {
        V.fetch_add(1,memory_order_relaxed);
    }
}


constexpr size_t Small = 1000;     // Number of summations for 2.3
size_t who = 0;                    // Thread id for switch
condition_variable chg;            // Condition variable to switch

// Variable increment function via intermediate value
// using condition_variable for thread switching
void addSync(unsigned int color)
{
    int save;               // Intermediate variable

    bool alone = false;     // Is single thread?

    for(int i = 0; i < Small; ++i)    // Small iteration
    {
        cout << attr(color) << '#';   // Color output what is thread active
        if (alone)                    // In single-thread mode don't use
        {                             // syncronization
            save = var;
            var = save + 1;
        }
        else
        {
            {                         
                unique_lock lck(m_var);
                save = var;           // Get original value of var
                who = _threadid;      // Write out current thread id
                chg.notify_one();     // Notify the thread about switch
            }
            {
                unique_lock lck(m_var);
                // Wait ANOTHER thread! if timeout,
                // the thread is considered to be the only thread
                // (single mode)
                if (chg.wait_for(lck,chrono::milliseconds(200),
                                 [&](){ return who != _threadid; }) == false)
                {
                    alone = true;
                }
                // Store new value of var
                var = save+1;
            }
        }
    }
}


int main()
{
    unsigned long long mutextime, freetime, atomtime;
    {   // 2a
        cout<< attr(GREEN) << "Shared variable 2x" << Count << " increments "
            << attr(MAGENTA) << "using mutex\n";
        muTimer mt;  // Timer

        thread t1(addMutex), t2(addMutex);  // Two threads
        t1.join();
        t2.join();
        mt.stop();
        cout << attr(GRAY) << "Var = " << var << " for " << (mutextime = mt.duration<muTimer::ms>()) << " ms\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }
    int saveVar = var;
    var = 0;
    {   // 2b
        cout<< attr(GREEN) << "Shared variable 2x" << Count << " increments "
            << attr(MAGENTA) << "without sync\n";
        muTimer mt;
        thread t1(addFree), t2(addFree);
        t1.join();
        t2.join();
        mt.stop();
        cout<< attr(GRAY) << "Var = " << var << " for " << (freetime = mt.duration<muTimer::ms>())
            << " ms\n";


        // 2.1, 2.2
        cout << attr(RED)  << "Results " << saveVar << " and " << var << " are " <<
            (saveVar == var ? "the same\n" : "different\n");
        cout<< attr(RED) << "The time with synchronization is "
            << fixed << setprecision(1) << double(mutextime)/freetime
            << " times the time without synchronization.\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }
    V = 0;
    {   //2.3.1
        cout<< attr(GREEN) << "Shared variable 2x" << Count << " fast increments "
            << attr(MAGENTA) << "using atomic ++\n";
        muTimer mt;
        thread t1(addAtom), t2(addAtom);
        t1.join();
        t2.join();
        mt.stop();
        cout<< attr(GRAY) << "Var = " << V << " for " << (atomtime = mt.duration<muTimer::ms>())
            << " ms\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }
    V = 0;
    {   //2.3.2
        cout<< attr(GREEN) << "Shared variable 2x" << Count << " fast increments "
            << attr(MAGENTA) << "using atomic feth_add\n";
        muTimer mt;
        thread t1(addAtom), t2(addAtom);
        t1.join();
        t2.join();
        mt.stop();
        cout<< attr(GRAY) << "Var = " << V << " for " << (atomtime = mt.duration<muTimer::ms>())
            << " ms\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }
    var = 0;
    {   //2.3*
        // Demonstration of how synchronized thread switching enables
        // step-by-step code execution as required in the task.

        // "That is, not only does each of the two parallel threads increment
        // the value from 0 to 1000, but both threads running in parallel also
        // increment from 0 to 1000 (and not to 2000, as expected)."
        // The task is completed.

        // First, two threads mode
        cout<< attr(GREEN) << "Shared variable " << Small << " increments "
            << attr(MAGENTA) << "using full syncronization\n";
        muTimer mt;
        thread t1(addSync,RED), t2(addSync,GREEN);
        t1.join();
        t2.join();
        mt.stop();
        cout<< attr(GRAY) << "\nVar = " << var << " for " << mt.duration<muTimer::ms>()
            << " ms\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }
    var = 0;
    {
        // Second, single thread mode
        cout<< attr(GREEN) << "Shared variable " << Small << " increments "
            << attr(MAGENTA) << "using full syncronization - single thread\n";
        muTimer mt;
        thread t1(addSync,GREEN); //, t2(addSync,RED);
        t1.join();
        //t2.join();
        mt.stop();
        cout<< attr(GRAY) << "\nVar = " << var << " for " << mt.duration<muTimer::ms>()
            << " ms\n";
        cout << attr(CYAN) << "Press any key to continue\r";
        _getch();
        cout << attr(CYAN) << "                         \n";
    }



}

