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
 *  CleanupManager.cc: Manage exitAll callbacks.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   November 2004
 *
 */

//  How to use this class:
//  
//  #include <Core/Thread/CleanupManager.h>
//  
//  Make a callback function that takes zero arguements and returns
//  void.  Be very careful about introducing crashes into this
//  callback function as it then becomes difficult to exit from
//  scirun.  It's recommended that you avoid calling scinew from
//  within your callback.
//  
//  Register with CleanupManager::add_callback(YOUR_CALLBACK_HERE, MISC_DATA);
//  
//  Your callback will only ever be called once, no matter how many
//  times you register it.  In addition you can unregister it or
//  design it such that it doesn't do anything if it doesn't need to.
//  Here is an example of how this could be used:
//
//  class Myclass {
//  public:
//    Myclass() {
//      CleanupManager::add_callback(this->cleanup_wrap, this);
//    }
//  
//    ~Myclass() {
//      // Need to remove and call callback at same time or else you get
//      // a race condition and the callback could be called twice.
//      CleanupManger::invoke_remove_callback(this->cleanup_wrap, this);
//    }
//  
//  private: 
//    static void cleanup_wrap(void *ptr) {
//      ((Myclass *)ptr)->cleanup();
//    }
//  
//    void cleanup();
//  }
//  

#ifndef SCI_project_CleanupManager_h
#define SCI_project_CleanupManager_h 1

#include <Core/Thread/Mutex.h>
#include <vector>

namespace SCIRun {

typedef void (*CleanupManagerCallback)(void *);

class CleanupManager {
public:
  
  // Initializes the mutex lock for the cleanup manager.  Initialize
  // is called from the Thread::initialize and is only called once.
  static void initialize();

  static void add_callback(CleanupManagerCallback cb, void *data);
  static void invoke_remove_callback(CleanupManagerCallback cb, void *data);
  static void remove_callback(CleanupManagerCallback cb, void *data);

  // After calling this, all registered functions will be removed.
  // Should this also delete all the memory allocated by initialize()?
  static void call_callbacks();

protected:
  // callbacks_ needs to be allocated off the heap to make sure it's
  // still around when exitAll is called.
  static std::vector<std::pair<CleanupManagerCallback, void *> >* callbacks_;
  static bool    initialized_;
  static Mutex * lock_;
};

} // End namespace SCIRun


#endif /* SCI_project_CleanupManager_h */
