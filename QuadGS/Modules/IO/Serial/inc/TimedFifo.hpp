/*
 * threadSafeFifo.h
 *
 * Copyright (C) 2015 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
                mTimedMutex.unlock(); // TODO unsafe.
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

    bool empty()
    {
        if (mTimedMutex.try_lock_for(Ms(1000)))
        {
            bool result = mQueue.empty();
            mTimedMutex.unlock();
            return result;
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


