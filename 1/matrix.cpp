#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <random>

// Utility functions for color output, timer etc
#include "utils.hpp"

using namespace std;


[[noreturn]] void help()
{
    cout << "matrix N M K ThreadCount -v[erbose] timeFile: A[N][M]*B[M][K]\n";
    exit(1);
}

// Global variables:

mutex output; // Mutex for console output

// Generator for matrix fill
default_random_engine rgen(std::random_device{}());
uniform_real_distribution<double> dist{-10,10};

// Verbose output
bool verbose = false;

// Main experiment
void experiment(int tCount,
                double**A, double**B, double**C,
                int N, int M, int K);

int main(int argc, char * argv[])
{
    double **A, **B, **C;               // Matrices
    int N = 200, M = 2000, K = 200;     // Default values
    int TC = 0;                         // Threads count. 0 => massiva experiment;
                                        //                else run once
    string file = "result.txt";

    // Read parameters
    if (argc > 1 && (N = atoi(argv[1])) <= 0) help();
    if (argc > 2 && (M = atoi(argv[2])) <= 0) help();
    if (argc > 3 && (K = atoi(argv[3])) <= 0) help();
    if (argc > 4 && (TC = atoi(argv[4])) < 0) help();
    if (argc > 5 && strncmp(argv[5],"-v",2)==0) verbose = true;
    if (argc > 6) file = argv[6];

    // Check file
    ofstream out(file.c_str());
    if (!out.is_open())
    {
        cerr << "Error open " << file << "\n";
        help();
    }
    else
    {
        out << stringf("A[%d][%d] x B[%d][%d]\n",N,M,M,K);
    }

    // Alloc and fill matrices
    A = new double*[N];
    for(int i = 0; i < N; ++i)
    {
        A[i] = new double[M];
        for(int j = 0; j < M; A[i][j++] = dist(rgen));
    }
    B = new double*[M];
    for(int i = 0; i < M; ++i)
    {
        B[i] = new double[K];
        for(int j = 0; j < K; B[i][j++] = dist(rgen));
    }
    C = new double*[N];
    for(int i = 0; i < N; ++i)
    {
        C[i] = new double[K];
    }

    // Run experiments or run once (TC != 0)

    for(int tCount = (TC == 0) ? 1 : TC;      // Since N*K can be a large number,
        tCount <= ((TC == 0) ? N*K : TC);     // performing all experiments for all
        tCount <= 19   ? ++tCount:            // numbers of threads can be very tedious,
        tCount <= 240  ? tCount +=   10 :     // so we use variable size steps
        tCount <= 440  ? tCount +=   50 :
        tCount <= 1900 ? tCount +=  100 :
        tCount <= 19000? tCount += 1000 :
                         tCount +=10000)
    {
        // Zeroing the matrix C
        for(int i = 0; i < N; ++i)
            for(int j = 0; j< K; ++j)
                C[i][j] = 0;

        // Using timer for experiments
        muTimer mt;

        experiment(tCount,A,B,C,N,M,K);

        mt.stop();

        // Writing the calculation time (mks) to a file
        out << tCount << "   " << mt.duration<>() << endl;

        // Recording the sum of all elements of matrix C
        // to confirm the correctness of calculations
        if (!verbose && TC == 0)
        {
            double sum = 0;
            for(int i = 0; i < N; ++i)
                for(int j = 0; j < K; ++j)
                    sum += C[i][j];
            cout << tCount << " threads. Sum = " << sum << endl;
        }
    }

    // Information about the number of processor cores
    cout << attr(GRAY) << "thread::hardware_concurrency() = " << thread::hardware_concurrency() << endl;

    // Free memory
    for(int i = 0; i < N; ++i) { free(C[i]); free(A[i]); }
    for(int i = 0; i < M; ++i) { free(B[i]); }
    free(A); free(B); free(C);
}

/*  All elements of the matrix C are renumbered line by line
 *  from 0 to N*K-1. Thus, we divide all the work into tCount
 *  threads, passing the ranges of numbers to be calculated by
 *  this thread.
 *
 *  The element number q corresponds to the element C[q/N][q%N].
 *
 *  Since different threads only read elements of matrices A and B,
 *  and write to different elements of matrix C, no synchronization
 *  using mutexes, critical partitions, etc. is required.
 *
 *  Mutex synchronization is only used for console output!
 */


// Function for calculating elements of matrix C
// starting from start and up to stop; debug output in color
void multiply(double**A, double**B, double**C, // Matrices
              int /*N*/, int M, int K,         // Sizes (N is redundant)
              int start, int stop,             // Range
              unsigned int color)              // Output color
{
    for(int j = start; j < stop; ++j)   // Range of elements to be 
    {                                   // calculated by this thread
        int n = j/K, k = j%K;           // Element C[n][k] indices

        if (verbose)
        {
            lock_guard<mutex> lock(output);  // Mutex lock
            cout << attr(color) << stringf("%6d Calc C[%3d][%3d]\n",this_thread::get_id(),n,k);
        }

        // Calculations
        C[n][k] = 0;
        for(int i = 0; i < M; ++i) C[n][k] += A[n][i]*B[i][k];
    }
}

// Experiment on calculating the product of matrices using tCount threads
void experiment(int tCount, double**A, double**B, double**C, int N, int M, int K)
{
    // Colors for output
    static unsigned int colors[] = { BLUE,GREEN,CYAN,RED,MAGENTA,YELLOW,WHITE };

    // Vector of threads
    vector<thread> tv;

    // l - one "bucket" size
    for(int j = 0, l = (N*K+tCount-1)/tCount; j < tCount; ++j)
    {
        // Adding a created (and running) thread to a vector
        tv.emplace_back(multiply, A,B,C, N,M,K, l*j,min(l*(j+1),N*K), colors[j%7]);
    }
    // Waiting for all running threads to finish
    for(auto& t: tv) t.join();

}



