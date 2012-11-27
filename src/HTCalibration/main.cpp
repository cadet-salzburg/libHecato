//#include <sys/time.h>
#include <highgui/highgui.hpp>
#include <XnCppWrapper.h>
#include "HTDeviceThreaded.h"


using namespace std;
using namespace xn;

struct HTWindow
{
	HTDeviceThreaded* htt;
	char windowName[16];
};
std::vector<HTWindow> htwindows;
xn::Context ctx;

int main()
{

	bool showThresh = false;
	int key = 0;
	int activeWindow = -1;
	xn::NodeInfoList devices;
	xn::NodeInfoList sensors;

	std::map<int, int> mappings;
	HTDevice::getBusMappings("../mapping.xml", mappings);

	ctx.Init();
	int numDevs = 0;
	XnStatus status = ctx.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, devices);
	if (status != XN_STATUS_OK || (devices.Begin() == devices.End()))
	{
		printf("HTCALIBRATION: Enumeration of devices failed. Quitting.\n");
		exit(0);
	}
	int id = -1;
	for (xn::NodeInfoList::Iterator iter = devices.Begin(); iter != devices.End(); ++iter, ++numDevs)
	{
		unsigned char bus = 0;
		unsigned short vendor_id;
		unsigned short product_id;
		unsigned char address;
		sscanf ((*iter).GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
		printf("VENDOR: %x, PRODUCTID: %x, connected %i @ %i\n", vendor_id, product_id, bus, address);

		//use only xTion cameras:
		if ((vendor_id != 0x1d27 || product_id != 0x600) && (vendor_id != 0x45e || product_id != 0x2ae))
		{
			printf("HTCALIBRATION: No entry for this device!\n");
			continue;
		}


		xn::NodeInfo deviceInfo = *iter;
		xn::DepthGenerator* depthGen = new xn::DepthGenerator();
		xn::Query q;
		//create a production tree that's dependent on the current device
		ctx.CreateProductionTree(deviceInfo);
		q.AddNeededNode(deviceInfo.GetInstanceName());
		ctx.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &q, *depthGen);
		//find the id matching the bus the device is connected to
		std::map<int, int>::iterator mapIter = mappings.find(bus);
		if (mapIter == mappings.end())
		{
			printf("WARNING: No entry for bus %i found!\n", bus);
			id++;
		}
		else
		{
			id = mapIter->second;
		}

		printf("BUS IS: %i, put it as id: %i\n", bus, id);
		HTDeviceThreaded* k = new HTDeviceThreaded(depthGen, id);

		char suffix[16];
		sprintf(suffix, "%d", id);
		std::string winName = "HTCALIBRATION";
		winName += suffix;
		HTWindow htw;
		htw.htt = k;
		memcpy(htw.windowName, winName.c_str(), 16);
		htwindows.push_back(htw);
		printf("HTCALIBRATION: Adding Device %i to production.\n", numDevs);
		cvNamedWindow(htw.windowName);
	}

	HTDeviceThreaded *curHTT = 0;
	while (key != 27)
	{
		ctx.WaitAndUpdateAll();
		const unsigned int len = htwindows.size();
		//first step: compute next frame
		for (unsigned i = 0; i < len; i++)
		{
			//this activates the threads waiting at their barriers
			htwindows[i].htt->compute();
		}
		//second step: show the frame
		for (unsigned i = 0; i < len; i++)
		{
			HTWindow* cur = &(htwindows[i]);
			if (showThresh)
				cvShowImage(cur->windowName, cur->htt->getAndLockCurrentThresholdImage());
			else
				cvShowImage(cur->windowName, cur->htt->getAndLockCurrentImage());
			cur->htt->unlockCurrentImage();

		}

		key = (cvWaitKey(1) & 255);

		if (key == ' ')
		{
			showThresh = !showThresh;
		}

		else if (key > 47 && key < 58) //number between 0 and 9 has been pressed
		{
			activeWindow = key - 48;
			curHTT = 0;
			for (unsigned i = 0; i < len; i++)
			{
				if (htwindows[i].htt->getID() == activeWindow)
				{
					curHTT = htwindows[i].htt;
					break;
				}
			}
			if (!curHTT)
				printf("Active Window: NONE!\n");
			else
				printf("Active Window: %i\n", activeWindow);
		}

		if (curHTT)
		{
			switch (key)
			{
			case 'q':
				printf("%s set corner point UL on window %i\n", curHTT->setCornerPoint(HTDevice::HTCP_UPPERLEFT) ? "Successfully" : "Unable to", activeWindow);
				break;

			case 'w':
				if (!curHTT->setCornerPoint(HTDevice::HTCP_UPPERRIGHT))
					printf("Unable to set corner point UR on window %i\n", activeWindow);
				else
					printf("Successfully set corner point UR on window %i\n", activeWindow);
				break;

			case 'a':
				if (!curHTT->setCornerPoint(HTDevice::HTCP_LOWERLEFT))
					printf("Unable to set corner point LL on window %i\n", activeWindow);
				else
					printf("Successfully set corner point LL on window %i\n", activeWindow);
				break;

			case 's':
				if (!curHTT->setCornerPoint(HTDevice::HTCP_LOWERRIGHT))
					printf("Unable to set corner point LR on window %i\n", activeWindow);
				else
					printf("Successfully set corner point LR on window %i\n", activeWindow);
				break;

			case '+':
				curHTT->setThresholdDelta(1);
				break;

			case '-':
				curHTT->setThresholdDelta(-1);
				break;

			default:
				break;
			}
		}
	}
	for (unsigned i = 0; i < htwindows.size(); i++)
	{
		delete htwindows[i].htt;
	}
	htwindows.clear();

	return 0;
}
