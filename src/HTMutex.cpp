
#include "HTMutex.h"
#include <errno.h>

HTMutex::HTMutex()
{
#ifdef WIN32
	InitializeCriticalSection(&m_Mutex);
#else
	pthread_mutex_init(&m_Mutex, NULL);
#endif
}

HTMutex::~HTMutex()
{
#ifdef WIN32
	DeleteCriticalSection(&m_Mutex);
#else
	pthread_mutex_destroy(&m_Mutex);
#endif
}

int HTMutex::lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_Mutex);
	return 0;
#else
	return pthread_mutex_lock(&m_Mutex);
#endif
}

int HTMutex::unlock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_Mutex);
	return 0;
#else
	return pthread_mutex_unlock(&m_Mutex);
#endif
}

int HTMutex::trylock()
{
#ifdef WIN32
	BOOL ret = TryEnterCriticalSection(&m_Mutex);
	return (ret ? 0 : EBUSY); // EBUSY is just a guess
#else
	return pthread_mutex_trylock(&m_Mutex);
#endif
}
