/*
 * threadSafeFifo.h
 *
 *  Created on: Sep 1, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_IO_SERIAL_INC_TIMEDFIFO_HPP_
#define QUADGS_MODULES_IO_SERIAL_INC_TIMEDFIFO_HPP_

#include <queue>
#include <mutex>
namespace QuadGS
{

template<class T>
class TimedFifo
{
public:
    TimedFifo(int size = 1, int timeout = 1000): mQueue(), mTimedMutex(), mSize(size), mTimeoutMs(timeout)
    {}

    ~TimedFifo()
    {}

    void push(T& item)
    {
        if (mTimedMutex.try_lock_for(Ms(1000)))
        {
            if(mQueue.size() >= mSize)
            {
                mTimedMutex.unlock();
                throw std::runtime_error("FIFO fifo full.");
            }
            else
            {
                mQueue.push(item);
                mTimedMutex.unlock();
            }
        }
        else
        {
            throw std::runtime_error("FIFO fifo not availible.");
        }

    }

    T& front()
    {
        if (mTimedMutex.try_lock_for(Ms(1000)))
        {
            if(!mQueue.empty())
            {
                mTimedMutex.unlock();
                return mQueue.front();
            }
            else
            {
                mTimedMutex.unlock();
                throw std::runtime_error("FIFO empty.");
            }
        }
        else
        {
            throw std::runtime_error("FIFO fifo not availible.");
        }

    }
    void pop()
    {
        if (mTimedMutex.try_lock_for(Ms(1000)))
        {
            if(!mQueue.empty())
            {
                mQueue.pop();
                mTimedMutex.unlock();
            }
            else
            {
                mTimedMutex.unlock();
                throw std::runtime_error("FIFO empty.");
            }
        }
        else
        {
            throw std::runtime_error("FIFO fifo not availible.");
        }
    }

private:
    using Ms = std::chrono::milliseconds;
    std::queue<T> mQueue;
    std::timed_mutex mTimedMutex;
    int mSize;
    int mTimeoutMs;
};

}





#endif /* QUADGS_MODULES_IO_SERIAL_INC_TIMEDFIFO_HPP_ */


