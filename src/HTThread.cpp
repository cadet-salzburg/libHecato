
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "HTThread.h"

HTThread::HTThread()
	: tExit(false)
	, threadRunning(false)
{
}

HTThread::~HTThread()
{
	stopThread(1000);
}

void HTThread::startThread()
{
	if (threadRunning)
		return;

#ifdef WIN32
	dtThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(HTThread::threadRouter), reinterpret_cast<void*>(this), 0, NULL);
#else
	pthread_create(&dtThread, NULL, HTThread::threadRouter, reinterpret_cast<void*>(this));
#endif
}

void HTThread::sleep(unsigned int milliseconds)
{
#ifdef WIN32
	Sleep(milliseconds);
#else
	usleep(milliseconds*1000);
#endif
}

void HTThread::stopThread(const unsigned short timeoutInMs)
{
	if (!threadRunning)
		return;
	tExit = true;
	sleep(timeoutInMs);
	if (threadRunning)
	{
		printf("HTThread: Thread still running after timeout, killing it with SIGTERM\n");
#ifdef WIN32
		if (TerminateThread(dtThread, 0))
#else
		if (pthread_kill(dtThread, SIGTERM) == 0)
#endif
		{
			printf("HTThread: Successfully killed thread.\n");
			threadRunning = false;
		}
		else
		{
			printf("HTThread: Unable to kill thread.\n");
		}
	}

#ifdef WIN32
	CloseHandle(dtThread);
#endif
}

THREAD_RET HTThread::threadRouter(void* arg)
{
	HTThread* thread = reinterpret_cast<HTThread*>(arg);
	thread->threadRunning = true;
	thread->run();
	thread->threadRunning = false;
#ifdef WIN32
	ExitThread(0);
#else
	pthread_exit(NULL);
#endif
	return NULL;
}

void HTThread::lockMutex()
{
	mtx.lock();
}

void HTThread::unlockMutex()
{
	mtx.unlock();
}
