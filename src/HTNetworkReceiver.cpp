#include "UDPManager.h"
#include "HTNetworkReceiver.h"
#include "../version.h"

HTNetworkReceiver::HTNetworkReceiver(unsigned short port) : interpreter(0)
{
	printf("LIBHECATO: v%s%s, BUILD: %li (%s-%s-%s)\n", AutoVersion::FULLVERSION_STRING, AutoVersion::STATUS_SHORT, AutoVersion::BUILDS_COUNT, AutoVersion::YEAR, AutoVersion::MONTH, AutoVersion::DATE);
	printf("Starting network receiver...\n");
	bool ok = true;
	udpMan = new UDPManager();
	if (!udpMan->Create())
	{
		printf("HTNETWORKRECEIVER: Unable to create connection!\n");
		ok = false;
	}
	if (!udpMan->Bind(port))
	{
		printf("HTNETWORKRECEIVER: Unable to bind to port %u\n", port);
		ok = false;
	}

	if (ok)
	{
		startThread();
		udpMan->SetTimeoutReceive(1);
	}


}

HTNetworkReceiver::~HTNetworkReceiver()
{
	stopThread(1000);
	udpMan->Close();
	delete udpMan;
	udpMan = 0;
}

void HTNetworkReceiver::run()
{
	char recvBuf[4096];
	int cursor = 0;
	//int packSize = -1;

	size_t intsize = sizeof(int);
	size_t uintsize = sizeof(unsigned int);
	size_t pointsize = sizeof(HTIBlobResultTarget::SBlobResult);

	std::vector<HTIBlobResultTarget::SBlobResult> pts;

	int genID = 0;
	int bytesRcvd = 0;
	unsigned numBlobs = 0;
	HTIBlobResultTarget::SBlobResult curPoint;
	char termByte = 0x00;
	char headByte = 0x00;

	while (!threadShouldExit())
	{
		cursor = 0;
		pts.clear();
		bool stillToRead = true;
		bytesRcvd = udpMan->Receive(recvBuf, 4096);
		if (bytesRcvd != SOCKET_TIMEOUT)
		{
			if (!interpreter)
				continue;
			while (stillToRead)
			{
				//STEP 0: check for header byte
				memcpy(&headByte, recvBuf + cursor, 1);
				cursor++;
				if (headByte != 0x0A)
				{
					printf("HTNetworkReceiver: Heading byte erroneus! Dropping...\n");
					stillToRead = false;
					continue;
				}
				//STEP 1: get the generator ID
				memcpy(&genID, recvBuf + cursor, intsize);
				cursor += intsize;
				//does the interpreter know about it (we can receive from multiple)?
				interpreterKnowsAbout(genID);

				//STEP 2: extract number of blobs
				memcpy(&numBlobs, recvBuf + cursor, uintsize);
				cursor += uintsize;

				for (unsigned i = 0; i < numBlobs; i++)
				{
					memcpy(&curPoint, recvBuf + cursor, pointsize),
						cursor += pointsize;
					pts.push_back(curPoint);
					//printf("ID: %i, POINT: %2.2f, %2.2f\n", genID, curPoint.x, curPoint.y);
				}

				//STEP 3: check if now comes the terminating byte
				memcpy(&termByte, recvBuf + cursor, 1);
				cursor++;
				if (termByte != 0x0E)
				{
					printf("HTNetworkReceiver: Erroneous packet received. Sender: %i, numBlobs %u\n", genID, numBlobs);
					stillToRead = false;
					continue;
				}
				// printf("Received OK package from %i, size %u\n", genID, numBlobs);
				interpreter->handleBlobResult(pts, genID);
				if (cursor == bytesRcvd)
				{
					stillToRead=false;
				}
				pts.clear();
			}
		}
	}
}

void HTNetworkReceiver::setBlobResultTarget(HTIBlobResultTarget* _target)
{
	interpreter = _target;
}

void HTNetworkReceiver::interpreterKnowsAbout(int generatorID)
{
	if (!interpreter)
		return;

	const unsigned len = idsKnown.size();

	for (unsigned i = 0; i < len; i++)
	{
		if (idsKnown[i] == generatorID)
			return;
	}

	//need to notify interpreter about new gen
	interpreter->registerGenerator(this, generatorID);
	idsKnown.push_back(generatorID);
}
