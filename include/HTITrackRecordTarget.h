#include "HTBlobInterpreter.h"

//! Interface for all classes that want to receive events from any HTBlobInterpreter
/** Derive from that class and set it as the tracking results' receiver to any HTBlobInterpreter*/
class HTITrackRecordTarget
{
public:
    virtual void handleEvents(const std::vector<HTBlobInterpreter::TrackRecord>& events) = 0;
};


