#ifndef HTTHREAD_H
#define HTTHREAD_H

#ifdef WIN32
#include <Windows.h>
#define THREAD_TYPE HANDLE
#define THREAD_RET DWORD
#else
#include <pthread.h>
#define THREAD_TYPE pthread_t
#define THREAD_RET void*
#endif

#include "HTMutex.h"

//!A convenience wrapper for platform-independent thread handling
/** This class provides a "java-like" usage of threading where the
	deriving class has to implement the run() function which is
	then executed by the pthread.*/
class HTThread
{
public:
	//!The constructor
	HTThread();
	//!The destructor
	virtual ~HTThread();

private:

	static THREAD_RET threadRouter(void* arg);
	THREAD_TYPE dtThread;
	bool tExit;
	bool threadRunning;
	HTMutex mtx;

protected:
	//!The thread's computation routine.
	/**	Upon calling startThread(), the thread enters run() and
		executes the code in it. The function is only called once
		so it's up to the programmer to keep it in the loop.
		A typical use case is the following:
		@code
		void MyClass::run()
		{
			while (!threadShouldExit())
			{
				//do your stuff
			}
		}
		@endcode
	*/
	virtual void run() = 0;
	//!Obtain whether the thread should leave its main loop
	/**	For examplary usage, see run()
		\return whether the thread should leave its main loop*/
	virtual bool threadShouldExit(){return tExit;};
	//!Create a new thread and assign run() to it
	virtual void startThread();
	//!Stop and tear down the thread
	/**	After calling this function, the main thread leaves the passed-in
		amount of milliseconds to the worker thread to close. After that
		resources that it might use are deleted.
		\param timeoutInMs Milliseconds to leave the worker thread to close.*/
	virtual void stopThread(const unsigned short timeoutInMs);
	//!Lock the mutex the worker thread syncs with
	void lockMutex();
	//!Unlock the mutex the worker thread syncs with
	void unlockMutex();

	void sleep(unsigned int milliseconds);
};
#endif
