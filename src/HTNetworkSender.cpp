#include "UDPManager.h"
#include "HTNetworkSender.h"
#include "HTIBlobResultTarget.h"

HTNetworkSender::HTNetworkSender() : generator(0), connOK(false), genSentCounter(0), numID(0), cursor(0), blobsSent(0), udpMan(0)
{
	for (int i = 0; i < MAX_IDS; i++)
	{
		genSent[i] = false;
	}

	udpMan = new UDPManager();
}

HTNetworkSender::~HTNetworkSender()
{
	connOK = false;
	udpMan->Close();
	delete udpMan;
	udpMan = 0;
}

void HTNetworkSender::registerGenerator(HTIBlobGenerator* kt, int id)
{
	generator = kt;
	numID++;
}

void HTNetworkSender::setHostAndPort(const char *pHost, USHORT usPort)
{
	if (connOK)
		return;

	if (!udpMan->Create())
	{
		printf("HTNETWORKSENDER: Unable to create connection!\n");
		return;
	}
	if (!udpMan->Connect(pHost, usPort))
	{
		printf("HTNETWORKSENDER: Unable to connect to %s at port %u!\n", pHost, usPort);
		return;
	}
	udpMan->SetBlocking(false);
	connOK = true;

}

void HTNetworkSender::handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id)
{
	if (!connOK)
		return;

	if (genSent[id])
		return;

	mutti.lock();
	genSent[id] = true;
	genSentCounter++;
	// printf("ID %i has dumped, waiting for %i more\n", id, numID - genSentCounter);

	size_t intsize = sizeof(int);
	size_t uintsize = sizeof(unsigned int);
	size_t pointsize = sizeof(HTIBlobResultTarget::SBlobResult);
	char c = 0x0A;
	//STEP 0: set the start byte
	memcpy(sndBuf + cursor, &c, 1);
	cursor++;
	//STEP 1: set the generator id
	memcpy(sndBuf + cursor, &id, intsize);
	cursor+=intsize;
	//STEP 2: set the points count
	const unsigned int sz = points.size();
	memcpy(sndBuf + cursor, &(sz), uintsize);
	cursor+=uintsize;
	//STEP 3: copy all the point info
	const unsigned len = points.size();
	for (unsigned i = 0; i < len; i++)
	{
		memcpy(sndBuf + cursor, &(points[i]), pointsize);
		cursor+=pointsize;
		blobsSent++;
	}
	//STEP 4: Terminate packet with 0x0E
	c = 0x0E;
	memcpy(sndBuf + cursor, &c, 1);
	cursor++;

	if (genSentCounter == numID)
	{
		for (int i = 0; i < MAX_IDS; i++)
		{
			genSent[i] = false;
		}

		udpMan->SendAll((const void*)sndBuf, cursor);
		// printf("SENT data from %i generators.\n", genSentCounter);
		//printf("Sent %i blobs.\n", blobsSent);
		blobsSent = 0;

		genSentCounter = 0;
		cursor = 0;

	}
	mutti.unlock();
}
