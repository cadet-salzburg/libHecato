#include "HTBlobInterpreter.h"

namespace TUIO
{
	class TuioServer;
	class TuioCursor;
}

//!Implementation of a blob interpreter sending TUIO cursors
/**This class represents the connector towards the TUIO protocol,
broadly used in the context of gestural, object-based user interaction.
The functionality is pretty self-explanatory, just a standard
HTBlobInterpreter keeping track of received events.
\see http://www.tuio.org/
*/
class HTTUIOServer : public HTBlobInterpreter
{
public:
	//!Constructor
	HTTUIOServer();
	//!Destructor
	virtual ~HTTUIOServer();
	//!Implementation of the pure virtual function. Sends TUIO Cursors.
	virtual void handleEvents(const std::vector<BlobRecord>& events);
	//!Sets up the connection. Do this before handling events (Cpt. Obvious).
	void setupConnection(const char* host, int port);
	//!Set a margin for the tracking region
	/**Some setups may require to set a margin. This is the case when e.g. the visualization is for
	some reason bigger than the tracking area, you'd want to set up a space to the left and right
	that is excluded from tracking without havin to adapt the visualization's code. This basically
	modifies the horizontal span in which the tracking's values are valid.
	Use the relative vals, for example, if you want to have a margin of a quarter from the left and
	from the right, use setMargins(0.25f, 0.75f);
	\note It is not guaranteed, that this functionality is implemented in deriving classes
	\param left Margin from the left in floats (e.g. 0.25f)
	\param right Margin from the right in floats (e.g. 0.75f)*/
	void setMargins(float left, float right);

private:
	struct TuioOutput
	{
		class TUIO::TuioCursor* cursor;
		class HTKalmanFilter* filter;
	};

	class TUIO::TuioServer* tuioServer;
	std::map<unsigned int, TuioOutput> cursorMap;
	float lMargin, rMargin;
};
