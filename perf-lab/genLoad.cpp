#include <iostream>
#include <chrono>
#include <thread>

void genLoad(int duration_sec)
{
    auto start = std::chrono::steady_clock::now();
    while
    (
        std::chrono::duration_cast<std::chrono::seconds>
        (
            std::chrono::steady_clock::now() - start
        ).count()
        < duration_sec
    ) 
            { } // busy loop to generate cpu-load 
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage :" << argv[0] << " <duration_seconds> " << std::endl;
        return 1; // error exit code
    }

    int duration_sec = atoi(argv[1]); //reading from cmd-line args

    if (duration_sec < 0)
    {
        std::cout << "Duration Seconds must be a positive value." << std::endl;
        return 1; 
    }

    std::cout << "Generating load for " << duration_sec << " seconds..." << std::endl;
    genLoad(duration_sec);
    std::cout << "Load Gen complete" << std::endl;

    return 0; //successful exit code
}

/*
how to complile and run:

g++ -o genLoad genLoad.cpp -std=c++17 
*/ 