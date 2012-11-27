#include <highgui/highgui.hpp>
#include "HTDeviceThreaded.h"

HTDeviceThreaded::HTDeviceThreaded(xn::DepthGenerator* _generator, int _id) : HTDevice(_generator, _id)
{
	barrier.init(2);
	startThread();
}

HTDeviceThreaded::~HTDeviceThreaded()
{
	barrier.wait();
	stopThread(50);
}

void HTDeviceThreaded::compute()
{
	barrier.wait();
}

void HTDeviceThreaded::run()
{
	while (!threadShouldExit())
	{
		lockMutex();
		detectBlobs();
		annotateInfo(frame);
		unlockMutex();

		if(!threadShouldExit())
			barrier.wait();
	}
}

const IplImage* HTDeviceThreaded::getAndLockCurrentImage()
{
	lockMutex();
	return frame;
}

const IplImage* HTDeviceThreaded::getAndLockCurrentThresholdImage()
{
	lockMutex();
	return frameThresh;
}

void HTDeviceThreaded::unlockCurrentImage()
{
	unlockMutex();
}
