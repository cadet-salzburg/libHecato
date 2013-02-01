
#include "HTBarrier.h"
#include <errno.h>

HTBarrier::HTBarrier()
{
#ifdef WIN32
#ifndef USE_WINDOWS_BARRIER
	memset(&m_Barrier, 0, sizeof(BARRIER_TYPE));
	m_nNumThreads = 0;
	m_nCurrentThreads = 0;
#endif
#endif
}

HTBarrier::~HTBarrier()
{
#ifdef WIN32
#ifdef USE_WINDOWS_BARRIER
	DeleteSynchronizationBarrier(&m_Barrier);
#else
	// TODO
#endif
#else
	pthread_barrier_destroy(&m_Barrier);
#endif
}

int HTBarrier::wait()
{
#ifdef WIN32
#ifdef USE_WINDOWS_BARRIER
	return (EnterSynchronizationBarrier(&m_Barrier, 0) ? 0 : EFAULT); // error number is bit of a guess
#else
	m_Mutex.lock();
	if (m_nCurrentThreads + 1 == m_nNumThreads)
	{
		continueThreads();
		m_Mutex.unlock();
		return 0;
	}

	m_nCurrentThreads += 1;
	DWORD currentThreadId = GetCurrentThreadId();
	HANDLE currentThread = GetCurrentThread();
	m_ThreadIDs.insert(currentThreadId);

	// THIS IS A BIG PROBLEM, WE HAVE TO RELEASE THE MUTEX BEFORE WE SUSPEND!!!
	m_Mutex.unlock();
	SuspendThread(currentThread);
#endif
#else
	return pthread_barrier_wait(&m_Barrier);
#endif
}

int HTBarrier::init(int numThreads)
{
#ifdef WIN32
#ifdef USE_WINDOWS_BARRIER
	return (InitializeSynchronizationBarrier(&m_Barrier, numThreads, -1) ? 0 : EFAULT); // error number is bit of a guess
#else
	m_Mutex.lock();
	if (m_nCurrentThreads == 0)
	{
		m_nNumThreads = numThreads;
		m_Mutex.unlock();
		return 0;
	}
	m_Mutex.unlock();
	return EFAULT;
#endif
#else
	return pthread_barrier_init(&m_Barrier, NULL, numThreads);
#endif
}


#ifdef WIN32
#ifndef USE_WINDOWS_BARRIER

void HTBarrier::continueThreads()
{
	std::set<DWORD>::iterator it = m_ThreadIDs.begin();
	while (it != m_ThreadIDs.end())
	{
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, *it);
		ResumeThread(hThread);
		it = m_ThreadIDs.erase(it);
	}
	m_nCurrentThreads = 0;
}

#endif
#endif
