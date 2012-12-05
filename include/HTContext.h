#include <vector>
#include <map>

namespace xn
{
    class Context;
}

//! Main class (singleton) that handles OpenNI-specific tasks
/** HTContext mainly wraps the OpenNI tasks and provides a simple and clean structure to the user.
  * During initialization it creates all cameras found and puts them into the HTDeviceThreaded structure.
  * For that it also assures the correct mapping of the cams (bus id) with their intended order
  * as stated in the mappings.xml file.*/
class HTContext
{
public:
    //! Access the devices
    static const std::vector<class HTDeviceThreaded*>& getDevices();
    //! Initialize HTContext and set up the camera(s)
    static void initialize();
    //! Update all cameras (grab next frame) and invoke frame evaluation
    /** This function tells all cameras to update the frame and evaluate it.
      * In other words, after this call all image-related processing (blob detection,
      * interpretation, optional sending of blob data and the like) are being handled.
      * In again other words, a call to HTDeviceThreaded::getAndLockCurrentImage() will
      * yield the next frame.*/
    static void updateAll();
    //! Shutdown the context and delete devices
    static void shutdown();

private:
    HTContext();
    ~HTContext();
    static HTContext* getInstance();
    static void getBusMappings(std::map<int, int>& m);
    static HTContext* instance;
    std::map<int, int> mapping;
    std::vector<class HTDeviceThreaded*> devices;
    xn::Context* ctx;
    bool initialized;
};
