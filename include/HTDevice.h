#ifndef HEADER_5A235891797B19A2
#define HEADER_5A235891797B19A2

#include <unistd.h>
#include <core/core.hpp>
#include "HTIBlobResultTarget.h"

namespace xn
{
	class NodeInfo;
	class Context;
	class DepthGenerator;
	class DepthMetaData;
	class ProductionNode;
}

//!Base class that handles the blob detection
/**	The class makes use of the (pre)defined corner points to
	- set the frame where interaction is triggered
	- to undistort the horizontal value caused by the nature of the
	camera's perspective projection to a orthogonal one
	.
	Further reading provides the PDF doc in the project folder
	which also sheds some light on the 'weird' naming of the variables
	\see setCornerPoint how to define the interaction area.
	Furthermore, the class allows to define a second region of interest
	which then tracks people within its frame. This can be useful for example
	if you want to check whether a person still stands in front of some active
	content or not. This uses maximum tracking, so normally you always the the
	person's head as the result.*/
class HTDevice : public HTIBlobGenerator
{
public:
	//!Enumeration of the four bounding corner points
	enum CornerPoint
	{
		HTCP_UPPERLEFT,
		HTCP_LOWERLEFT,
		HTCP_UPPERRIGHT,
		HTCP_LOWERRIGHT
	};
	//!The constructor
	/**	Takes OpenNI's parameters needed to construct the depth generator.
		\param _generator A valid DepthGenerator to be used. Considered to be owned by the class.
		\param _id This instance's id. Valuable if more HTDevices are instantiated to denote its own highgui window
		\note HTDevice takes ownership over the passed-in _generator*/
	HTDevice(class xn::DepthGenerator* _generator, int _id);
	//!The default destructor
	virtual ~HTDevice();
	//!Directly sets all four corner points
	/**	Normally there's no need to call this function directly as the
		values are either read from the settings file or manually set via
		\see setCornerPoint */
	virtual void setBoundary(const CvPoint3D32f& ul, const CvPoint3D32f& ur, const CvPoint3D32f& ll, const CvPoint3D32f& lr);
	//!Undistorts a point's X coordinate according to its position on the perspective projection
	/**	Again there should be no need to call this function directly.
		\param p The point to undistort*/
	virtual void undistortX(CvPoint3D32f& p);
	//!Set a corner point confining the interaction area
	/**	This is a handy function to live-define the corner points.
		Hold your hand in one of the corners of the interaction area
		and call this function with the respective CornerPoint value
		to set it according to the generated blob.
		\return true if the operation was successful, false otherwise.
		\note Make sure there's only one blob whilst calling the function!*/
	virtual bool setCornerPoint(const CornerPoint& cp);
	//!Adjust the current threshold by the given delta
	/**	\param delta by what value to adjust the threshold*/
	virtual void setThresholdDelta(int delta);
	//!Set a new threshold value
	/**	\param newThreshold the new threshold value to use*/
	virtual void setThreshold(int newThreshold);
	//!Set the minimum blob size
	/**	This sets the mininum size of a blob to be treated as valid.
		\param newBlobSize the new size in 'square pixels'.
		\see setBlobSizeDelta*/
	virtual void setBlobSize(unsigned int newBlobSize);
	//!Set the minimum blob size
	/**	This sets the mininum size of a blob to be treated as valid.
		\param delta increase / decrease for the current blob size
		\see setBlobSize
		\see getBlobSize*/
	virtual void setBlobSizeDelta(int delta);
	//!Enable or disable the supplementary person tracking
	virtual void setEnablePersonTracking(bool enabled);
	//!Gets the current blob size
	/**	Gets the current size a blob has to have to be treated as valid.
		\return The current size in square pixels.*/
	virtual unsigned int getBlobSize() const;
	//!Assign a target for the detected blobs
	/**	\note The instance DOES NOT take ownership of the passed-in HTBlobInterpreter instance
		\param _interpreter The interpreter to send the detected blobs to*/
	virtual void setBlobResultTarget(class HTIBlobResultTarget* _target);
	//!Conducts the actual blob detection <b>without</b> interpretation
	/**	The result of this function is the pure blob detection and that's it.
		Call this only if you want to e.g. do something completely different than the recommended detectBlobsAndAnnotate function*/
	virtual void detectBlobs();
	//!Conducts the actual blob detection <b>with</b> interpretation <i>(recommended)</i>
	/**	A call to this function interpretes the blobs, undistorts positions and also renders (debug) information in the current frame, such as:
		- the region of interest (ROI)
		- the actual touch within the ROI along with its undistorted position
		- information about blob size, threshold and blob count
		- as an overlay the touch position represented as a big white ring - taking the window's boundaries
		as the area the camera detects in (confined by the corner points)
		- this means e.g. if this white ring goes beyond the right border of the window you've left the defined
		space to the right
		.
		.*/
	virtual void detectBlobsAndAnnotate();
	//!Get access to the pixel array of the current image (8 bit GRAYSCALE)
	/**	\note This is not thread-safe in potentially any subclasses using threads. Look there for safe counter-parts.
	\see HTDeviceThreaded*/
	virtual const unsigned char* getCurrentImage() const;
	//!Retrieve the instance ID (useful if multiple cams are used)
	int getID() const;
	static void getBusMappings(const char* settingsFile, std::map<int, int>& mappings);


protected:
	//!Attempts to load the settings from a config file
	/**	Looks for a config file in the execution directory.
		Uses the currently set id to determine its own file,
		e.g. ID: 0 - settings0.xml.*/
	virtual void loadSettings();
	//!Attempts to save the settings in a config file
	/**	Looks for a config file in the execution directory.
		Uses the currently set id to determine its own file,
		e.g. ID: 0 - settings0.xml.*/
	virtual void saveSettings();
	//!Helper function to add information about the detection state
	virtual void annotateInfo(IplImage* img=0);
	//!The current capture image
	IplImage* frame;
	//!The current thresholded image
	IplImage* frameThresh;

private:

	//these vals are used to iron out the horizontal error caused by the perspective
	//and are based on the defined corner points above
	//naming based on an unreadable sketch by me
	//Detailed desc can now be found in the pdf in the project folder
	float b0, b1, h, h0, db, w, middle;
	//these four points denote the 'corners' of the eligible tracking area
	CvPoint3D32f c0, c1, c2, c3;

	int id;

	IplConvKernel* kernel;

	CvRect roi, roiPerson;
	CvFont font;
	class CBlobResult* bRes, *bResPerson;
	float personStepWidth;
	int thresh;
	unsigned int blobSize, blobSizePerson;
	std::vector<CvPoint> persons;
	bool usePersonTracking;

	class xn::Context* ctx;
	class xn::DepthGenerator* generator;
	class xn::DepthMetaData* meta;
	class xn::ProductionNode* pNode;
	class HTIBlobResultTarget* interpreter;

	HTDevice();
	unsigned short *depthArray16, gammaConversion[2048];
	unsigned char gammaConversionByte[2048], shortToByteConversion[2048];
};

/*! \mainpage libHecato API documentation
 *
 * <!--<div align="center"><img src="logobig.png" ></div>-->

 * <!--\section contents Contents
 * General:<br/>
 * @ref OFLGameObject <br/>
 * some more <br/>-->
 *
 * \section intro Introduction
 * libHecato is an easy-to-use framework to quickly set up depth-imaging-enabled (e.g. "Kinect") multi-user interaction applications.
 * It is able to process touch events, gestural movements and to track the presence of users. Got a big projection you've always wanted
 * to use multi-touch? Grab a depth cam and libHecato is your solution.<br/>
 * The framework hereby expects the camera to be placed above the interaction area to minimize occlusion while maximizing the amount of
 * possible simultaneous users.
 * libHecato's key strengths lie in the following:
 * <ul>
 *   <li><b>Multi Aspect</b><br/>Whether it's point-to-click or gestures you want to process, libHecato has a tracking component that
 * robustly correlates the framewise detection to kalman-filtered tracks. Besides a defineable area for any hand actions it is also possible
 * to set a space in the camera image in which persons are to be detected.</li>
 *   <li><b>Scalability</b><br/>The software can handle consistent tracking over a tracking space of arbitrary length. This means that multiple
 * depth cameras can be inconcatenated to form larger installations. This renders it highly usable for big projections.</li>
 *   <li><b>Output / Extendability</b><br/>Hecato uses the <a href="http://www.tuio.org" target="_blank">TUIO</a>-Protocol, a for tangible user
 * interfaces widely used protocol, for standard output.<br/>All the classes along the processing chain - from image acquisition to output are easily
 * extendable to allow for modifications in every step. See @ref HTBlobInterpreter to modify the handling of detections or to
 * change how tracks should be handled.</li>
 *   <li><b>Platform Independence</b><br/>Using only platform-independend libraries such as opencv and OpenNI itself, libHecato is able to be compiled for
 * Linux, Windows and Mac (in this order of support :) )</li>
 *   <li><b>Toolset</b><br/>The framework includes two applications for calibrating (<b>HTCalibration</b>) and for operation (<b>HTApp</b>) which usually
 * suffice for not too specific tasks. In this case, no programming experience is needed to use libHecato.
 * </ul>
 * <br/><br/>
 * \section example Example
 * The following example shows how to use libHecato to output the detected gestures to the command line.<br/>It assumes a set-up xn::DepthGenerator and xn::Context
 * from the <a href="http://www.openni.org" target="_blank">OpenNI</a> framework. Future versions of libHecato are highly likely to do that for you.
 * @code
 * //assumed the following two are correctly set up:
 * xn::DepthGenerator* depthGen;
 * xn::Context ctx;
 * @endcode
 * The next step is to initialize a @ref HTDevice, the base class for image generation and blob detection. As libHecato is intended to be used with multiple cameras
 * you have to pass-in the id of that instance, in this case '0'. During construction of the instance, it tries to load the settings associated with ID zero by looking
 * for a file called "settings<id>.xml, in this case then "settings0.xml" in an attempt to restore the configuration previously set up.<br/>
 * Next we initialize a @ref HTBlobInterpreterConsole, a simple implementation of @ref HTBlobInterpreter that outputs to the command line. We then inform our
 * @ref HTDevice to deliver the detected blobs to this interpreter:
 * @code
 * //use the DepthGenerator and a (here) arbitrary id to construct our HTDevice:
 * HTDevice dev(depthGen, 0);
 * //A simple implementation of HTBlobInterpreter that outputs to the console:
 * HTBlobInterpreterConsole interpreter();
 * //Tell the device where to put the blobs it has detected:
 * dev.setBlobResultTarget(&interpreter);
 *
 * //now loop until we found something else to play with:
 * while (thisMakesFun)
 * {
 *     ctx.WaitAndUpdateAll();
 *     dev.detectBlobsAndAnnotate();
 * }
 * return 0;
 * @endcode

 * <br/>
 */


#endif // header guard


