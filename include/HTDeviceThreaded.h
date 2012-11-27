#ifndef HEADER_B7EF01D0F56FECA7
#define HEADER_B7EF01D0F56FECA7

#include "HTDevice.h"
#include "HTThread.h"
#include "HTBarrier.h"

//!A HTDevice that handles most processing in a separate thread
/**This class handles image generation, data processing and blob detection
in a separate thread and is therefor quite handy to be used when multiple
generators are connected to one machine to parallelize the work load.
As it is quite frequently necessary to have some way of syncronization,
the working thread syncs with a call to compute(), so make sure to call this
function every time the thread should proceed doing his job.
\see HTVisualizer for an examplary usage.*/
class HTDeviceThreaded : public HTDevice, public HTThread
{
public:
	//!The constructor
	HTDeviceThreaded(xn::DepthGenerator* _generator, int _id);
	//!The destructor
	virtual ~HTDeviceThreaded();
	//!Procedes the computation of the next frame
	/**	A call to this function syncs with the worker thread and
		is meant as a way to allow the app programmer to keep multiple
		instances timed to each other.*/
	virtual void compute();
	//!Gets the current image thread-safe
	/**	This function retrieves the last computed image (including annotation)
		and locks with the worker thread. Make sure to call unlockCurrentImage()
		as soon as you don't need the image any more.
		\return a const pointer to the latest (annotated) image*/
	virtual const IplImage* getAndLockCurrentImage();
	//!Gets the current threshold image thread-safe
	/**	This function retrieves the last computed threshold image
		and locks with the worker thread. Make sure to call unlockCurrentImage()
		as soon as you don't need the image any more.
		\return a const pointer to the latest threshold image*/
	virtual const IplImage* getAndLockCurrentThresholdImage();
	//!Unlock the previously locked IplImage
	/**	Call this once you're finished with the previously acquired image to let it go for
		the processing thread again*/
	void unlockCurrentImage();

private:
	HTBarrier barrier;
	//!The thread function. Not to be called explicitly.
	void run();
};

#endif // header guard
