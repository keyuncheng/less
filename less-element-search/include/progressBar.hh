#ifndef __PROGRESS_BAR__
#define __PROGRESS_BAR__

#include <iostream>
#include <unistd.h>
#include <string>

/**
 * @brief A simple progress bar for terminal
 * @param total The total number of iterations
 * @param width The width of the progress bar
 */

class ProgressBar
{
public:
    ProgressBar(int total, int width = 50) : total_(total), width_(width), current_(0)
    {
        is_terminal_ = isatty(fileno(stdout));
    }

    void update(int current)
    {
        current_ = current;
        print();
    }

    void increment()
    {
        current_++;
        print();
    }

private:
    void print();

    int total_;
    int width_;
    int current_;
    bool is_terminal_;
};

#endif // #ifndef __PROGRESS_BAR__