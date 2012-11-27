#include <stdio.h>
#include <stdlib.h>
#include <XnCppWrapper.h>
#include <opencv.hpp>
#include <tinyxml.h>
#include "BlobResult.h"
#include "HTDevice.h"
#include "HTBlobInterpreter.h"

#include "../version.h"

#undef USE_CONVERSION

#define RES_X 640
#define RES_Y 480
#define RES RES_X * RES_Y

using namespace xn;

HTDevice::HTDevice(xn::DepthGenerator* _generator, int _id) : roi(cvRect(0, 0, RES_X, RES_Y)), roiPerson(cvRect(0, 0, RES_X, RES_Y)),
	b0(0.f),
	b1(0.f),
	h(0.f),
	h0(0.f),
	db(0.f),
	w(0.f),
	middle(0.f),
	c0(cvPoint3D32f(0.f, 0.f, 0.f)),
	c1(cvPoint3D32f(0.f, 0.f, 0.f)),
	c2(cvPoint3D32f(0.f, 0.f, 0.f)),
	c3(cvPoint3D32f(0.f, 0.f, 0.f)),
	id(_id),
	interpreter(0),
	personStepWidth(5.f),
	blobSize(300),
	blobSizePerson(1000),
	generator(_generator),
	usePersonTracking(false)

{
	printf("LIBHECATO: v%s%s, BUILD: %li (%s-%s-%s)\n", AutoVersion::FULLVERSION_STRING, AutoVersion::STATUS_SHORT, AutoVersion::BUILDS_COUNT, AutoVersion::YEAR, AutoVersion::MONTH, AutoVersion::DATE);
	bRes = new CBlobResult();
	bResPerson = new CBlobResult();
	meta = new xn::DepthMetaData();
	XnMapOutputMode mode;
	mode.nXRes = RES_X;
	mode.nYRes = RES_Y;
	mode.nFPS  = 30;
	//    if (!generator)
	//        printf("GENERATOR IS NULL!\n");
	//printf("CREATIONINFO: %s\n", _info->);
	generator->SetMapOutputMode(mode);
	generator->GetMetaData(*meta);
	generator->StartGenerating();
	frame = cvCreateImage(cvSize(RES_X,RES_Y), IPL_DEPTH_8U, 1);
	frameThresh = cvCreateImage(cvSize(RES_X,RES_Y), IPL_DEPTH_8U, 1);
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, .5f, .5f, 0, 1);
	depthArray16 = new uint16_t[RES];
	loadSettings();

	for( unsigned int i = 0 ; i < 2048 ; i++)
	{
		float v = (float)i/2048.0f;
		v = pow(v, 3)* 6;
		gammaConversion[i] = (unsigned short)(v*6.0f*256.0f);
		gammaConversionByte[2047-i] = ((float)i / 2048.f * 256.f);
		shortToByteConversion[2047-i] = (unsigned int)(((float)i / 2048.f) * 256.f);
	}
	//    kernel = cvCreateStructuringElementEx(kernelSize, kernelSize, 0, 0, CV_SHAPE_RECT, NULL);
}

HTDevice::~HTDevice()
{
	generator->StopGenerating();
	//    if (interpreter)
	//    {
	//        delete interpreter;
	//        interpreter = 0;
	//    }
	saveSettings();
	printf("Saved settings!\n");
	delete[] depthArray16;
	cvReleaseImage(&frame);
	cvReleaseImage(&frameThresh);
	delete bRes;
	delete bResPerson;
	delete meta;
	delete generator;

}

void HTDevice::loadSettings()
{
	std::string loadFile = "../settings";
	char charId[16];
	sprintf(charId, "%d", id);
	loadFile += charId;
	loadFile += ".xml";
	printf("HTDEVICE: Reading file: %s\n", loadFile.c_str());
	CvFileStorage* fs = cvOpenFileStorage(loadFile.c_str(), 0, CV_STORAGE_READ);
	if (!fs)
		printf("HTDEVICE: Unable to find settings file: %s, using defaults. This is bad.\n", loadFile.c_str());
	thresh = cvReadIntByName(fs, 0, "thresh", 128);
	roi.y = cvReadIntByName(fs, 0, "uBound", 0);
	roi.height = cvReadIntByName(fs, 0, "lBound", roi.height);
	blobSize = cvReadIntByName(fs, 0, "blob", 250);
	printf("HTDEVICE: Thresh: %i, ROI lower bound: %i, ROI height: %i\n", thresh, roi.y, roi.height);

	setBoundary(cvPoint3D32f(cvReadIntByName(fs, 0, "c0x", 0), cvReadIntByName(fs, 0, "c0y", 0), cvReadIntByName(fs, 0, "c0z", 0)),
		cvPoint3D32f(cvReadIntByName(fs, 0, "c1x", 0), cvReadIntByName(fs, 0, "c1y", 0), cvReadIntByName(fs, 0, "c1z", 0)),
		cvPoint3D32f(cvReadIntByName(fs, 0, "c2x", 0), cvReadIntByName(fs, 0, "c2y", 0), cvReadIntByName(fs, 0, "c2z", 0)),
		cvPoint3D32f(cvReadIntByName(fs, 0, "c3x", 0), cvReadIntByName(fs, 0, "c3y", 0), cvReadIntByName(fs, 0, "c3z", 0)));

	int useP = cvReadIntByName(fs, 0, "usePersonTracking", 0);
	if (useP == 1)
		usePersonTracking = true;
	else
	{
		printf("HTDEVICE: Not using Person Tracking.\n");
	}
	roiPerson.y = cvReadIntByName(fs, 0, "uBoundPerson", 0);
	roiPerson.height = cvReadIntByName(fs, 0, "lBoundPerson", RES_Y);
	roiPerson.x = cvReadIntByName(fs, 0, "lftBoundPerson", 0);
	roiPerson.width = cvReadIntByName(fs, 0, "wdhBoundPerson", RES_X);
	printf("HTDEVICE: Use Person Tracking. ROIY: %i, HEIGHT: %i\n", roiPerson.y, roiPerson.height);

	cvReleaseFileStorage(&fs);

}

void HTDevice::saveSettings()
{
	std::string loadFile = "../settings";
	char charId[16];
	sprintf(charId, "%d", id);
	loadFile += charId;
	loadFile += ".xml";
	CvFileStorage* fs = cvOpenFileStorage(loadFile.c_str(), 0, CV_STORAGE_WRITE);
	cvWriteInt(fs, "thresh", thresh);
	cvWriteInt(fs, "blob", blobSize);
	cvWriteInt(fs, "uBound", roi.y);
	cvWriteInt(fs, "lBound", roi.height);
	cvWriteInt(fs, "usePersonTracking", usePersonTracking ? 1 : 0);
	cvWriteInt(fs, "uBoundPerson", roiPerson.y);
	cvWriteInt(fs, "lBoundPerson", roiPerson.height);
	cvWriteInt(fs, "lftBoundPerson", roiPerson.x);
	cvWriteInt(fs, "wdhBoundPerson", roiPerson.width);

	cvWriteInt(fs, "c0x", c0.x);
	cvWriteInt(fs, "c0y", c0.y);
	cvWriteInt(fs, "c0z", c0.z);

	cvWriteInt(fs, "c1x", c1.x);
	cvWriteInt(fs, "c1y", c1.y);
	cvWriteInt(fs, "c1z", c1.z);

	cvWriteInt(fs, "c2x", c2.x);
	cvWriteInt(fs, "c2y", c2.y);
	cvWriteInt(fs, "c2z", c2.z);

	cvWriteInt(fs, "c3x", c3.x);
	cvWriteInt(fs, "c3y", c3.y);
	cvWriteInt(fs, "c3z", c3.z);

	cvReleaseFileStorage(&fs);
}

void HTDevice::setBoundary(const CvPoint3D32f& ul, const CvPoint3D32f& ur, const CvPoint3D32f& ll, const CvPoint3D32f& lr)
{
	c0 = ul;
	c1 = ur;
	c2 = ll;
	c3 = lr;

	h0 = c0.z;
	b0 = c0.x - c1.x;
	if (b0 < 0) b0 = -b0;
	b1 = c2.x - c3.x;
	if (b1 < 0) b1 = -b1;
	//NOTE (ottona#9#): calc mean height val for b0 and b1
	h = c0.z - c2.z;
	h0 = c0.z;
	if (h < 0) h = -h;

	db = b0 - b1;
	if (db < 0) db = -db;
	middle = (c0.x + c1.x) / 2;
}

void HTDevice::undistortX(CvPoint3D32f& p)
{
	const float hoffset = p.z-h0;
	w = b0/2 - hoffset/h * db/2;
	float dx = p.x - middle;
	p.x = ((middle+(b0/2 * dx/w)));
}

void HTDevice::annotateInfo(IplImage* img)
{
	std::string info = "Thresh: ";
	char nmb[32];
	sprintf(nmb, "%i", thresh);
	info += nmb;
	info += "  Blobs: ";
	sprintf(nmb, "%i", bRes->GetNumBlobs()-1);
	info += nmb;
	info += "  BlobSize: ";
	sprintf(nmb, "%u", blobSize);
	info += nmb;
	if (img)
	{
		cvPutText(img,info.c_str(),cvPoint(20,20), &font, cvScalar(255,0,0));
		cvRectangleR(img, roi, cvScalar(255, 255, 255));
	}

	std::vector<HTIBlobResultTarget::SBlobResult> ptsVec;

	for (int i = 1; i < bRes->GetNumBlobs(); i++)
	{
		CvBox2D blobBox = bRes->GetBlob(i)->GetEllipse();
		CvPoint blobPoint = cvPoint(blobBox.center.x, blobBox.center.y + blobBox.size.height/2 + roi.y);
		const int idx = RES_X*((int)(blobBox.center.y) + roi.y) + (int)(blobBox.center.x);
		if (idx > RES || idx < 0)
		{
			continue;
		}
		int colVal = depthArray16[idx];
		CvPoint3D32f realPoint = cvPoint3D32f(blobPoint.x, blobPoint.y, colVal);
		undistortX(realPoint);
		//this is the undistorted, cleaned horizontal value between the defined corner points [0.f,...1.f]
		const float intervalX = ((c0.x - realPoint.x) / b0);
		const float intervalY = ((float)((colVal - h0))/h);


		if (intervalX > 1.f || intervalX < 0.f || intervalY < 0.f || intervalY > 1.f)
			continue;

		if (img)
		{
			cvCircle(img, blobPoint, 5, cvScalar(255, 0, 0));
			cvCircle(img, cvPoint(realPoint.x, realPoint.y), 5, cvScalar(255, 0, 0));
			cvCircle(img, cvPoint( intervalX * RES_X, intervalY * RES_Y), 20, cvScalar(255, 0, 0), 5);
		}
		ptsVec.push_back(HTIBlobResultTarget::SBlobResult(HTIBlobResultTarget::BRT_HAND, intervalX, intervalY));
	}

	if (usePersonTracking)
	{
		if (img)
		{
			cvRectangleR(img, roiPerson, cvScalar(255, 255, 255));
		}
		persons.clear();
		for (int i = 1; i < bResPerson->GetNumBlobs(); i++)
		{
			CvBox2D blobBox = bResPerson->GetBlob(i)->GetEllipse();
			uint16_t maxHeight = 65535;
			CvPoint maxPoint;
			//calc the upper left corner of the person's blob in the 'real' image
			CvPoint blobPoint = cvPoint((int)(blobBox.center.x + blobBox.size.height * -0.5f) + roiPerson.x, (int)(blobBox.center.y + blobBox.size.width * -0.5f) + roiPerson.y);
			//            CvRect personBB = cvRect(blobPoint.x, blobPoint.y, blobBox.size.height, blobBox.size.width);
			//
			//            if (img)
			//            {
			//                cvRectangleR(img, personBB, cvScalar(255, 255, 255));
			//            }


			for (int runX = 0; runX < blobBox.size.height; runX += personStepWidth)
			{
				for (int runY = 0; runY < blobBox.size.width; runY += personStepWidth)
				{
					unsigned int idx = RES_X * (blobPoint.y + runY) + blobPoint.x + runX;
					if (idx > RES-1)
						continue;
					uint16_t thisHeight = depthArray16[idx];
					if (thisHeight < maxHeight)
					{
						maxHeight = thisHeight;
						maxPoint.x = blobPoint.x + runX;
						maxPoint.y = blobPoint.y + runY;
					}
				}
			}
			persons.push_back(maxPoint);
			ptsVec.push_back(HTIBlobResultTarget::SBlobResult(HTIBlobResultTarget::BRT_PERSON, (1.f - ((float)(maxPoint.x - roiPerson.x) / roiPerson.width)), 0.f, maxHeight));
		}
		const unsigned len = persons.size();
		for (unsigned i = 0; i < len; i++)
		{
			cvCircle(img, persons[i], 10, cvScalar(255, 0, 0));
		}
	}
	if (interpreter)
		interpreter->handleBlobResult(ptsVec, id);
}

void HTDevice::setBlobResultTarget(class HTIBlobResultTarget* _target)
{
	//    if (interpreter)
	//    {
	//        delete interpreter;
	//    }
	interpreter = _target;
	interpreter->registerGenerator(this, id);
}

int HTDevice::getID() const
{
	return id;
}

bool HTDevice::setCornerPoint(const CornerPoint& cp)
{
	if (bRes->GetNumBlobs() != 2)    //dunno why it offsets with 1
		return false;

	CvBox2D blobBox = bRes->GetBlob(1)->GetEllipse();
	CvPoint blobPoint = cvPoint(blobBox.center.x, blobBox.center.y + blobBox.size.height/2 + roi.y);
	int colVal = depthArray16[RES_X*((int)(blobBox.center.y) + roi.y) + (int)(blobBox.center.x)];
	CvPoint3D32f realPoint = cvPoint3D32f(blobPoint.x, blobPoint.y, colVal);

	switch (cp)
	{
	case HTCP_LOWERLEFT:
		setBoundary(c0, c1, realPoint, c3);
		break;
	case HTCP_LOWERRIGHT:
		setBoundary(c0, c1, c2, realPoint);
		break;
	case HTCP_UPPERLEFT:
		setBoundary(realPoint, c1, c2, c3);
		break;
	case HTCP_UPPERRIGHT:
		setBoundary(c0, realPoint, c2, c3);
		break;
	default:
		break;
	}
	printf("SETCORNERPOINT: %4.2f, %4.2f, %4.2f\n", realPoint.x, realPoint.y, realPoint.z);
	return true;
}

const unsigned char* HTDevice::getCurrentImage() const
{
	return (unsigned char*)frame->imageData;
}

void HTDevice::detectBlobs()
{
	// generator->WaitAndUpdateData();
	if (!generator->IsDataNew())
		return;
	//    if (generator->IsDataNew())
	//    {
	generator->GetMetaData(*meta);
	const XnDepthPixel* depVals = meta->Data();

	for (unsigned i = 0; i < RES; i++)
	{
		uint16_t px = depVals[i];
		if (px < 1 || px > 2047)
			px = 2047;
#ifdef USE_CONVERSION
		depthArray16[i] = gammaConversion[px];
		frame->imageData[i] = gammaConversionByte[px];
#else
		depthArray16[i] = px;
		frame->imageData[i] = shortToByteConversion[px];
#endif
	}

	//threshold the image
	cvThreshold( frame, frameThresh, thresh, 255, CV_THRESH_BINARY_INV );
	//set region of interest according to the rect
	cvSetImageROI(frameThresh, roi);
	//get blobs from image
	delete bRes;
	bRes = new CBlobResult(frameThresh, NULL, 0, true);
	//blob should be 20x20 min
	bRes->Filter(*bRes, B_INCLUDE, CBlobGetArea(), B_GREATER, blobSize);
	//if person tracking is enabled, look for them in the second ROI
	if (usePersonTracking)
	{
		cvSetImageROI(frameThresh, roiPerson);
		delete bResPerson;
		bResPerson = new CBlobResult(frameThresh, NULL, 0, true);
		bResPerson->Filter(*bResPerson, B_INCLUDE, CBlobGetArea(), B_GREATER, blobSizePerson);

		//            for (int i = 1; i < bResPerson->GetNumBlobs(); i++) //still dunno why it starts with 1
		//            {
		//                CvBox2D blobBox = bResPerson->GetBlob(1)->GetEllipse();
		//                CvPoint2D32f highestPoint;
		//                unsigned char highval = 0;
		//                for (float x = blobBox.center.x + blobBox.size.width * -0.5f; x < blobBox.size.width; x+=personStepWidth)
		//                {
		//                    for (float y = blobBox.center.y + blobBox.size.height * -0.5f; y < blobBox.size.height; y += personStepWidth)
		//                    {
		//                        if ()
		//                    }
		//                }
		//            }
	}
	//set back ROI to whole image
	cvResetImageROI(frameThresh);

	//    }
}

void HTDevice::detectBlobsAndAnnotate()
{
	detectBlobs();
	annotateInfo();
}

void HTDevice::setThresholdDelta(int delta)
{
	const int newThresh = thresh + delta;
	setThreshold(newThresh);
}

void HTDevice::setThreshold(int newThreshold)
{
	thresh = newThreshold;
	if (thresh < 0)
		thresh = 0;
	if (thresh > 255)
		thresh = 255;
}

void HTDevice::setBlobSize(unsigned int newBlobSize)
{
	blobSize = newBlobSize;
}

void HTDevice::setEnablePersonTracking(bool enabled)
{
	usePersonTracking = enabled;
}

void HTDevice::setBlobSizeDelta(int delta)
{
	if (((int)blobSize + delta) < 0)
	{
		blobSize = 0;
		return;
	}

	blobSize += delta;
}

unsigned int HTDevice::getBlobSize() const
{
	return blobSize;
}

void HTDevice::getBusMappings(const char* settingsFile, std::map<int, int>& mappings)
{
	TiXmlDocument doc(settingsFile);
	if (!doc.LoadFile())
	{
		printf("GETBUSMAPPINGS: Unable to read config file!\n");
		return;
	}
	TiXmlHandle handle(&doc);
	TiXmlElement* curElem = handle.FirstChildElement().FirstChildElement("mapping").ToElement();

	if (!curElem)
	{
		printf("GETBUSMAPPINGS: Unable to find \"mapping\" elements!\n");
		return;
	}


	for (;curElem != 0; curElem = curElem->NextSiblingElement("mapping"))
	{
		int bus;
		int address;
		curElem->QueryIntAttribute("bus", &bus);
		curElem->QueryIntAttribute("id", &address);
		mappings.insert(std::pair<int, int>(bus, address));
		printf("MAPPING: BUS: %i to ADDRESS: %i\n", bus, address);
	}
}
