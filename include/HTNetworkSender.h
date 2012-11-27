#include "HTIBlobResultTarget.h"
#include "HTMutex.h"

#define MAX_IDS 16

//!Blob result handler that transmits the result over UDP.
/**HTNetworkSender can be assigned just like any HTInterpreter
and takes care for the transmission of blob data. Its counterpart
HTNetworkReceiver acts again as HTIBlobGenerator so the
whole network handling is completely transparent to the application.**/
class HTNetworkSender : public HTIBlobResultTarget
{
public:
	//!Default constructor
	HTNetworkSender();
	//!Default destructor
	virtual ~HTNetworkSender();
	//!Set the target host and port to use.
	/**	Sets the host ip address (where the receiver is run) and the port it
		listens on.
		\param pHost The host name or IP address
		\param usPort The port the receiver listens on*/
	void setHostAndPort(const char *pHost, unsigned short usPort);
	//!Receives the generator's blob result and packs it onto the network
	/**	Once person tracking is enabled, such blobs are transmitted with the
		y-axis set to zero. Note that the TUIO server also denotes such tracks with
		y set to 0.*/
	void handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id);
	//!Registers a generator with the network sender
	/**	The network sender can handle virtually any number of generators, the
		receiver takes this into account on the remote side. So if you have
		multiple generators on one side and need to carry it to the other, just
		use the same sender for that.
		\param kt The registering generator
		\param id The registering generator's id
		\note Since one generator can wrap multiple ones, the two input parameters
		don't necessarily have to be the same.*/
	void registerGenerator(class HTIBlobGenerator* kt, int id);

private:
	class HTIBlobGenerator* generator;
	class UDPManager* udpMan;
	bool connOK;
	HTMutex mutti;
	bool genSent[MAX_IDS];
	int genSentCounter;
	int numID;
	char sndBuf[2048];
	int cursor;
	int blobsSent;
};
