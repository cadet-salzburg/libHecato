//#include <sys/time.h>
#include <stdio.h>
#include <highgui/highgui.hpp>
#include "HTContext.h"
#include "HTDeviceThreaded.h"


using namespace std;
using namespace xn;

struct HTWindow
{
	HTDeviceThreaded* htt;
	char windowName[16];
};
std::vector<HTWindow> htwindows;

int main()
{

	bool showThresh = false;
	int key = 0;
	int activeWindow = -1;
	//first we need to initialize the context:
	HTContext::initialize();

	//get the devices:
	const std::vector<HTDeviceThreaded*>& devs = HTContext::getDevices();

	for (unsigned int i = 0; i < devs.size(); i++)
    {
		char suffix[16];
		sprintf(suffix, "%d", devs[i]->getID());
		std::string winName = "HTCALIBRATION";
		winName += suffix;
		HTWindow htw;
		htw.htt = devs[i];
		memcpy(htw.windowName, winName.c_str(), 16);
		htwindows.push_back(htw);
		cvNamedWindow(htw.windowName);
	}

	HTDeviceThreaded *curHTT = 0;
	while (key != 27)
	{
		HTContext::updateAll();
		const unsigned int len = htwindows.size();

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


		//=== WINDOW HANDLING ===

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

	htwindows.clear();

	HTContext::shutdown();

	return 0;
}
