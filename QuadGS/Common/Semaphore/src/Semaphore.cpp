
#include "Semaphore.h"

namespace QuadGS{

void Semaphore::notify() 
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    ++count_;
    condition_.notify_one();
}

void Semaphore::wait() 
{
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    while(!count_ && !mStop) // Handle spurious wake-ups.
        condition_.wait(lock);
    if(!mStop)
    {
        --count_;
    }
}

void Semaphore::stop()
{
    mStop = true;
    condition_.notify_all();
}

}