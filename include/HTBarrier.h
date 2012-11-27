#ifndef HTBARRIER_H
#define HTBARRIER_H

#ifdef WIN32
#include <Windows.h>
// from Windows 8 on there will be a barrier implementation in win32
// http://msdn.microsoft.com/en-us/library/windows/desktop/hh706897%28v=vs.85%29.aspx
// #define USE_WINDOWS_BARRIER
#ifdef USE_WINDOWS_BARRIER
#define BARRIER_TYPE SYNCHRONIZATION_BARRIER
#else
// for now we have to do some hacking
#include "HTMutex.h"
#define BARRIER_TYPE HANDLE
#include <set>
#endif
#else
#include <pthread.h>
#define BARRIER_TYPE pthread_barrier_t
#endif

class HTBarrier
{
public:
	HTBarrier();
	~HTBarrier();

	int init(int nThreads);
	int wait();

protected:
	BARRIER_TYPE m_Barrier;

#ifdef WIN32
#ifndef USE_WINDOWS_BARRIER
	// just some helper stuff
	std::set<DWORD> m_ThreadIDs;
	HTMutex m_Mutex;
	int m_nNumThreads;
	int m_nCurrentThreads;

	void continueThreads();
#endif
#endif
};


#endif //HTBARRIER_H
