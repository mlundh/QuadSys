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
#include <condition_variable>
namespace QuadGS
{

template<class T>
class ThreadSafeFifo
{
public:
	ThreadSafeFifo(unsigned int size = 0): mQueue(), mMutex(), mSize(size)
	{}

	~ThreadSafeFifo()
	{}

	void pushWait(T item)
	{
		{
		std::unique_lock<std::mutex> l(mMutex);
		while((mQueue.size() >= mSize))
		{
			// release lock, wait and then reaquire it.
			mCv.wait(l);
		}
		if((mQueue.size() >= mSize)) // Should never happen...
		{
			throw std::runtime_error("FIFO fifo full, even after wait...!");
		}
		mQueue.push(std::move(item));
		}
		mCv.notify_all();
	}

	void push(T item)
	{
		{
		std::unique_lock<std::mutex> l(mMutex);
		if((mSize > 0) && (mQueue.size() >= mSize))
		{
			throw std::runtime_error("FIFO fifo full.");
		}
		else
		{
			mQueue.push(std::move(item));
		}
		}
		mCv.notify_all();
	}

	T& front()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while(mQueue.empty())
		{
			// release lock, wait and then reaquire it.
			mCv.wait(lock);
		}
		return mQueue.front();
	}

	void pop()
	{

		std::unique_lock<std::mutex> lock(mMutex);
		while(mQueue.empty())
		{
			// release lock, wait and then reaquire it.
			mCv.wait(lock);
		}
		mQueue.pop();
	}

	T dequeue()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while(mQueue.empty())
		{
			// release lock, wait and then reaquire it.
			mCv.wait(lock);
		}
		T result = std::move(mQueue.front());
		mQueue.pop();
		return std::move(result);
	}
	bool empty()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		return mQueue.empty();

	}
private:
	std::queue<T> mQueue;
	std::mutex mMutex;
	std::condition_variable mCv;
	unsigned int mSize;
};

}





#endif /* QUADGS_MODULES_IO_SERIAL_INC_TIMEDFIFO_HPP_ */


