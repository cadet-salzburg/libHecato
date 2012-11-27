#include <vector>
#include "HTThread.h"
#include "HTIBlobResultTarget.h"

//!Counterpart of HTNetworkSender to receive blob data and act as a generator
/**	This class receives the data being received from a network sender. As it also acts as
	a blob generator it can easily be attached to a HTBlobInterpreter, just as any
	HTDevice instance.*/
class HTNetworkReceiver : public HTThread, public HTIBlobGenerator
{
public:
	//!The constructor
	/**	Pass in the port the receiver should listen on
		\param port The network port the receiver listens for
		incoming data*/
	HTNetworkReceiver(unsigned short port);
	//!The destructor
	~HTNetworkReceiver();
	void run();
	void setBlobResultTarget(class HTIBlobResultTarget* _target);


private:
	std::vector<int> idsKnown;
	class UDPManager* udpMan;
	class HTIBlobResultTarget* interpreter;
	void interpreterKnowsAbout(int generatorID);
};
