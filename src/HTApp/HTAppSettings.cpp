#include "HTAppSettings.h"
#include <iostream>
#include <tinyxml.h>

HTAppSettings* HTAppSettings::instance = NULL;

HTAppSettings::HTAppSettings() : sHost(""),
                                 sPort(0),
                                 rPort(0),
                                 delay(30),
                                 useS(false),
                                 useR(false),
                                 useF(false),
                                 lMargin(0.f),
                                 rMargin(0.f),
                                 corrW(0.1f),
                                 corrH(0.1f),
                                 corrPerson(0.15f),
                                 rClick(0.f),
                                 rDrag(0.1f)
{
    int intHelper = 0;

    TiXmlDocument doc("../appsettings.xml");
    if (!doc.LoadFile())
    {
        printf("HTAppSettings: Unable to read config file!\n");
        return;
    }
    TiXmlHandle handle(&doc);
    TiXmlElement* curElem = handle.FirstChildElement().FirstChildElement().ToElement();

    while (curElem)
    {
        if (curElem->ValueStr().compare("tuioserver") == 0)
        {
            curElem->QueryIntAttribute("on", &intHelper);
            if (intHelper == 1)
            {
                TUIOServerData data;
                curElem->QueryStringAttribute("host", &data.host);
                curElem->QueryIntAttribute("port", &data.port);
                tuioServers.push_back(data);
            }
        }
        else if (curElem->ValueStr().compare("sender") == 0)
        {
            curElem->QueryStringAttribute("host", &sHost);
            curElem->QueryIntAttribute("port", &sPort);
            curElem->QueryIntAttribute("on", &intHelper);
            useS = intHelper == 1 ? true : false;
        }
        else if (curElem->ValueStr().compare("receiver") == 0)
        {
            curElem->QueryIntAttribute("on", &intHelper);
            useR = intHelper == 1 ? true : false;
            curElem->QueryIntAttribute("port", &rPort);
        }
        else if (curElem->ValueStr().compare("trackingmargin") == 0)
        {
            curElem->QueryFloatAttribute("left", &lMargin);
            curElem->QueryFloatAttribute("right", &rMargin);
        }
        else if (curElem->ValueStr().compare("fps") == 0)
        {
            curElem->QueryIntAttribute("on", &intHelper);
            useF = intHelper == 1 ? true : false;
            curElem->QueryIntAttribute("delay", &delay);
        }
        else if (curElem->ValueStr().compare("correlation") == 0)
        {
            curElem->QueryFloatAttribute("width", &corrW);
            curElem->QueryFloatAttribute("height", &corrH);
        }
        else if (curElem->ValueStr().compare("persontracking") == 0)
        {
            curElem->QueryIntAttribute("on", &intHelper);
            useP = intHelper == 1 ? true : false;
            curElem->QueryFloatAttribute("corr", &corrPerson);
        }
        else if (curElem->ValueStr().compare("radius") == 0)
        {
            curElem->QueryFloatAttribute("click", &rClick);
            curElem->QueryFloatAttribute("drag", &rDrag);
        }

        curElem = curElem->NextSiblingElement();

    }
}

HTAppSettings::~HTAppSettings()
{

}

HTAppSettings* HTAppSettings::getInstance()
{
    if (!instance)
    {
        instance = new HTAppSettings();
    }
    return instance;
}

float HTAppSettings::getLeftMargin()
{
    return getInstance()->lMargin;
}

float HTAppSettings::getRightMargin()
{
    return getInstance()->rMargin;
}

int HTAppSettings::getReceiverPort()
{
    return getInstance()->rPort;
}

bool HTAppSettings::useTUIO()
{
    return (getInstance()->tuioServers.size() != 0);
}

bool HTAppSettings::useSender()
{
    return getInstance()->useS;
}

bool HTAppSettings::useReceiver()
{
    return getInstance()->useR;
}

int HTAppSettings::getSenderPort()
{
    return getInstance()->sPort;
}

const char* HTAppSettings::getSenderHost()
{
    return getInstance()->sHost.c_str();
}

bool HTAppSettings::useFPS()
{
    return getInstance()->useF;
}

int HTAppSettings::getFPSDelay()
{
    return getInstance()->delay;
}

float HTAppSettings::getCorrelationW()
{
    return getInstance()->corrW;
}

float HTAppSettings::getCorrelationH()
{
    return getInstance()->corrH;
}

bool HTAppSettings::usePersonTracking()
{
    return getInstance()->useP;
}

float HTAppSettings::getPersonCorrelation()
{
    return getInstance()->corrPerson;
}

float HTAppSettings::getRadiusClick()
{
    return getInstance()->rClick;
}

float HTAppSettings::getRadiusDrag()
{
    return getInstance()->rDrag;
}

const std::vector<HTAppSettings::TUIOServerData>& HTAppSettings::getTUIOServers()
{
    return getInstance()->tuioServers;
}
