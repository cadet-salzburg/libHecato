#ifndef HTMUTEX_H
#define HTMUTEX_H

#ifdef WIN32
#include <Windows.h>
#define MUTEX_TYPE RTL_CRITICAL_SECTION
#else
#include <pthread.h>
#define MUTEX_TYPE pthread_mutex_t
#endif
//! Wrapper class to platform-independently handle thread safety
class HTMutex
{
public:
    //!The constructor
	HTMutex();
	//!The destructor
	~HTMutex();
    //!Locks the mutex
    /**A call to lock() will return only upon successfully locking the mutex (waits until another thread unlocks it).
    @return 0 if successful, error code in case of such on POSIX-compliant operating systems. Always 0 on Windows systems
    @see The posix mutex definition for detailed information about return values */
	int lock();
	//!Unlocks the mutex
    /**A call to unlock() will return only upon successfully leaving the mutex (which is the usual case once it's locked).
    @return 0 if successful, error code in case of such on POSIX-compliant operating systems. Always 0 on Windows systems
    @see The posix mutex definition for detailed information about return values */
	int unlock();
    //!Tries to get the mutex
    /**A call to trylock() will return immediately, even if the mutex is being held by another thread.
    Use the return value to determine whether the thread was able to obtain the mutex.
    @return 0 if the mutex was successfully obtained, EBUSY if another thread holds it.
    @see The posix mutex definition for detailed information about return values */
	int trylock();

protected:
	MUTEX_TYPE m_Mutex;
};


#endif //HTMUTEX_H
