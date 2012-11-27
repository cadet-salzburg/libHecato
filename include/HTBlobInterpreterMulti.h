#ifndef HEADER_925CCDDB5338B7E0
#define HEADER_925CCDDB5338B7E0

#include "HTThread.h"
#include "HTMutex.h"
#include "HTBlobInterpreter.h"
#include "HTDeviceThreaded.h"

#define MAX_GENERATORS 16

//NOTE (ottona##):  Due to low computational complexity, this class is not threaded. The last generator sending blob info has to do this. (Normally takes <1ms)

//!HTBlobInterpreter that handles multiple generators at once
/**This implementation of a blob interpreter takes information from multiple
generators and correlates it in a global coordinate system. This works under
the assumption that the IDs of the generators are ascending from left to right
meaning that e.g. generator 2 is to the left of generator 3.
The interpreter also accounts for blobs located on the border of two neighboring
generators are likely to be the same gesture and hence assures continuous
tracking.*/
class HTBlobInterpreterMulti : public HTBlobInterpreter
{
public:
	//!Helper struct to simplify blob correlation
	struct BlobHandled
	{
		int originId;
		float x, y, z;
		bool handled;
		HTIBlobResultTarget::BlobResultType type;
		BlobHandled(HTIBlobResultTarget::BlobResultType t, float _x, float _y, float _z, int id)
		{
			x = _x;
			y = _y;
			z = _z;
			type = t;
			handled = false;
			originId = id;
		}
	};
	//!The constructor
	HTBlobInterpreterMulti();
	//!The destructor
	virtual ~HTBlobInterpreterMulti();
	virtual void registerGenerator(HTIBlobGenerator* kt, int id);
	virtual void handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id);
	virtual void handleEvents(const std::vector<BlobRecord>& events) = 0;
	//!Set the horizontal correlation distance
	/**Within this horizontal distance, two blobs on the borders of two neighboring
	generators are assumed to be the same gesture.
	This only treats blobs close to the edge AND of neighboring generators
	so it has no impact on the overall interpreter performance.
	\note Blob correlation obviously is performed before interpretation
	\param newDist The distance (in relative coordinates) blobs can be
	correlated.*/
	void setCorrelationRadiusW(float newDist);
	//!Set the vertical correlation distance
	/**Within this horizontal distance, two blobs on the borders of two neighboring
	generators are assumed to be the same gesture.
	This only treats blobs close to the edge AND of neighboring generators
	so it has no impact on the overall interpreter performance.
	\note Blob correlation obviously is performed before interpretation
	\param newDist The distance (in relative coordinates) blobs can be
	correlated.*/
	void setCorrelationRadiusH(float newDist);
	//!Set the correlation distance for person blobs, if active
	/**Within this distance, two person blobs from neighboring cameras are correlated and
	hence seen as the same user.
	Requires person tracking to be activated in HTDevice.
	\param newDist The distance (in relative coordinates) blobs can be
	correlated.*/
	void setCorrelationPerson(float newDist);
	//!Returns the number of generators currently registered with this interpreter
	int getNumGenerators();

private:
	void correlateBlobs(std::vector<HTIBlobResultTarget::SBlobResult>& corrBlobs);
	int numInstances, curInstances;
	float stepWidth, corrRadiusW, corrRadiusH, corrPerson;
	std::vector<BlobHandled> globalPoints;
	//results can pop in from multiple threads, be prepared
	HTMutex mutti, workerMutti;
	//Thread signal_worker, signal_handler;
	bool generators[MAX_GENERATORS];
	std::vector<HTDeviceThreaded*> ktInstances;

};

#endif // header guard
