/*
 * The MIT License
 *
 * Copyright (c) 1997-2015 The University of Utah
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


/*
 *  MutexPool: A set of mutex objects
 *
 *  Written by:
 *   Author: Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *   Date: September 1999
 *
 */

#include <Core/Thread/MutexPool.h>
namespace SCIRun {


MutexPool::MutexPool(const char* name, int size)
    :  nextID_("MutexPool ID lock", 0), size_(size)
{
    // Mutex has no default CTOR so we must allocate them
    // indepdently.
    pool_=new Mutex*[size_];
    for(int i=0;i<size_;i++)
	pool_[i]=new Mutex(name);
}

MutexPool::~MutexPool()
{
    for(int i=0;i<size_;i++)
	delete pool_[i];
    delete[] pool_;
}

int MutexPool::nextIndex()
{
    for(;;) {
	int next=nextID_++;
	if(next < size_)
	    return next;
	// The above is atomic, but if it exceeds size, we need to
	// reset it.
	nextID_.set(0);
    }
}

Mutex* MutexPool::getMutex(int idx)
{
    return pool_[idx];
}

void MutexPool::lockMutex(int idx)
{
    pool_[idx]->lock();
}

void MutexPool::unlockMutex(int idx)
{
    pool_[idx]->unlock();
}


} // End namespace SCIRun
