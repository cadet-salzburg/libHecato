#ifndef HTIBLOBRESULTTARGET_H
#define HTIBLOBRESULTTARGET_H
#include <vector>
#include <opencv.hpp>

//!Interface class that assures the minimal method set for blob targets.
/**A blob target is everything that can receive blob information from a generator
for further processing (such as HTBlobInterpreter or
HTNetworkSender).
*/
class HTIBlobResultTarget
{
public:
	//!Enumeration of blob types.
	/**	Depending in which area the blob occurred it is either treated as a hand gesture
		or a whole person.
		\note Person tracking must be enabled to receive results other than BRT_HAND*/
	enum BlobResultType
	{
		//!The current blob is recognized as hand
		BRT_HAND,
		//!The current blob is recognized as a person
		BRT_PERSON
	};

	//!Structure that stores all necessary data about a recognized blob
	/**	SBlobResult is the struct used for any derivate of HTIBlobResultTarget to do
		whatever treatment of a generator's detection results.*/
	struct SBlobResult
	{
		//!Default constructor
		SBlobResult() : type(BRT_HAND), point(cvPoint3D32f(0.f, 0.f, 0.f)){}
		//!Constructor
		SBlobResult(BlobResultType t, const CvPoint3D32f& p)
		{
			type = t;
			point.x = p.x;
			point.y = p.y;
		}
		//!Constructor
		SBlobResult(BlobResultType t, float x, float y, float z = 0.f)
		{
			type = t;
			point.x = x;
			point.y = y;
			point.z = z;
		}
		//!The result's type, either hand or person for now.
		BlobResultType type;
		//!The blob's current position
		/**	The blob position is represented in the following way:
			- BRT_HAND
			- <b>x</b> the hand's horizontal value in a float ]0.f, 1.f[
			- this value expresses the value relative to the space confined by the set corner points
			.
			- <b>y</b> the hand's vertical value in a float ]0.f, 1.f[
			- this value expresses the value relative to the space confined by the set corner points
			.
			- <b>z</b> not used
			.
			- BRT_PERSON
			- <b>x</b> the person's horizontal value in a float ]0.f, 1.f[
			- this value expresses the value relative to the defined person Region Of Interest (ROI)
			.
			- <b>y</b> not used
			- could be extended if a two-dimensional tracking of persons is required
			.
			- <b>z</b> the person's highest elevation (head) in raw depth value
			.
			.*/
		CvPoint3D32f point;
	};
	//!Handles the currently detected blobs.
	/**	The blob-generating class HTDevice calls this function every iteration
		with a vector containing the currently detected blobs.
		\param points A vector containing the blobs
		\param id The id of the calling generator. Necessary if a registered generator
		wraps multiple (HTBlobInterpreterMulti, HTNetworkSender)**/
	virtual void handleBlobResult(std::vector<SBlobResult>& points, int id) = 0;
	//!Allows a HTDevice instance to register itself when the HTBlobInterpreter is assigned to it
	/**	Once a HTBlobInterpreter is assigned to a HTDevice, the latter
		calls this function in case this the interpreter needs to know about it.
		\param kt The HTDevice instance the interpreter has just been registered with.
		\param id The id of the current generator. \note An HTIBlobGenerator can wrap multiple instances
		of different generators (in other words: one generator can show up with different ids). \see HTNetworkSender*/
	virtual void registerGenerator(class HTIBlobGenerator* kt, int id) = 0;
};

//!Interface class that assures the minimal method set for blob generators.
/**	A blob generator is everything that makes blob information available. In the most usual
	case this is a (derivate of) HTDevice. Other useful generators are e.g. HTNetworkReceiver.*/
class HTIBlobGenerator
{
public:
	//!Assigns a recipient to the generator
	/**	Once the generator has evaluated the blob info, it calls the recipient ("target") to further
		on process the blob information. In the usual case this is one of the HTBlobInterpreter or
		a HTNetworkSender.
		\param _target The recipient of blob info to register*/
	virtual void setBlobResultTarget(HTIBlobResultTarget* _target) = 0;
};
#endif
