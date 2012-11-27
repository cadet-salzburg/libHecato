#include <string>

class HTAppSettings
{
public:
    static bool useTUIO();
    static bool useSender();
    static bool useReceiver();
    static bool useFPS();
    static bool usePersonTracking();
    static const char* getTUIOHost();
    static const char* getSenderHost();
    static int getTUIOPort();
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

private:
    static HTAppSettings* getInstance();
    HTAppSettings();
    ~HTAppSettings();
    HTAppSettings(const HTAppSettings&);
    static HTAppSettings* instance;

    std::string tHost, sHost;
    int tPort, sPort, rPort, delay;
    bool useT, useS, useR, useF, useP;
    float lMargin, rMargin, corrW, corrH, corrPerson, rClick, rDrag;
};
