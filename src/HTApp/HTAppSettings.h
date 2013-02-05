#include <vector>
#include <string>

class HTAppSettings
{
public:
    struct TUIOServerData
    {
        std::string host;
        int port;
    };

    static bool useTUIO();
    static bool useSender();
    static bool useReceiver();
    static bool useFPS();
    static bool usePersonTracking();
    static const char* getSenderHost();
    static int getSenderPort();
    static int getReceiverPort();
    static int getFPSDelay();
    static float getLeftMargin();
    static float getRightMargin();
    static float getCorrelationW();
    static float getCorrelationH();
    static float getPersonCorrelation();
    static float getRadiusClick();
    static float getRadiusDrag();
    static const std::vector<TUIOServerData>& getTUIOServers();

private:
    static HTAppSettings* getInstance();
    HTAppSettings();
    ~HTAppSettings();
    HTAppSettings(const HTAppSettings&);
    static HTAppSettings* instance;

    std::string sHost;
    std::vector<TUIOServerData> tuioServers;
    int sPort, rPort, delay;
    bool useS, useR, useF, useP;
    float lMargin, rMargin, corrW, corrH, corrPerson, rClick, rDrag;
};
