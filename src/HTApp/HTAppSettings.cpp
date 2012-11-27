#include "HTAppSettings.h"
#include <iostream>
#include <tinyxml.h>

HTAppSettings* HTAppSettings::instance = NULL;

HTAppSettings::HTAppSettings() : tHost(""),
                             sHost(""),
                             tPort(0),
                             sPort(0),
                             rPort(0),
                             delay(30),
                             useT(false),
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
    TiXmlElement* curElem = handle.FirstChildElement().FirstChildElement("tuioserver").ToElement();

    if (!curElem)
    {
        printf("HTAppSettings: Unable to find TUIO-related elems!\n");
    }
    else
    {
        curElem->QueryStringAttribute("host", &tHost);
        curElem->QueryIntAttribute("port", &tPort);
        curElem->QueryIntAttribute("on", &intHelper);
        useT = intHelper == 1 ? true : false;
    }


    curElem = handle.FirstChildElement().FirstChildElement("sender").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find sender-related elems!\n");
    }
    else
    {
        curElem->QueryStringAttribute("host", &sHost);
        curElem->QueryIntAttribute("port", &sPort);
        curElem->QueryIntAttribute("on", &intHelper);
        useS = intHelper == 1 ? true : false;
    }

    curElem = handle.FirstChildElement().FirstChildElement("receiver").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find receiver-related elems!\n");
    }
    else
    {
        curElem->QueryIntAttribute("on", &intHelper);
        useR = intHelper == 1 ? true : false;
        curElem->QueryIntAttribute("port", &rPort);
    }


    curElem = handle.FirstChildElement().FirstChildElement("trackingmargin").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find tracking margin elems!\n");
    }
    else
    {
        curElem->QueryFloatAttribute("left", &lMargin);
        curElem->QueryFloatAttribute("right", &rMargin);
    }

    curElem = handle.FirstChildElement().FirstChildElement("fps").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find FPS elems!\n");
    }
    else
    {
        curElem->QueryIntAttribute("on", &intHelper);
        useF = intHelper == 1 ? true : false;
        curElem->QueryIntAttribute("delay", &delay);
    }
    curElem = handle.FirstChildElement().FirstChildElement("correlation").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find correlation elems!\n");
    }
    else
    {
        curElem->QueryFloatAttribute("width", &corrW);
        curElem->QueryFloatAttribute("height", &corrH);
    }
    curElem = handle.FirstChildElement().FirstChildElement("persontracking").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find persontracking elems!\n");
    }
    else
    {
        curElem->QueryIntAttribute("on", &intHelper);
        useP = intHelper == 1 ? true : false;
        curElem->QueryFloatAttribute("corr", &corrPerson);
    }
    curElem = handle.FirstChildElement().FirstChildElement("radius").ToElement();
    if (!curElem)
    {
        printf("HTAppSettings: Unable to find radius elems!\n");
    }
    else
    {
        curElem->QueryFloatAttribute("click", &rClick);
        curElem->QueryFloatAttribute("drag", &rDrag);
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

const char* HTAppSettings::getTUIOHost()
{
    return getInstance()->tHost.c_str();
}

int HTAppSettings::getTUIOPort()
{
    return getInstance()->tPort;
}

int HTAppSettings::getReceiverPort()
{
    return getInstance()->rPort;
}

bool HTAppSettings::useTUIO()
{
    return getInstance()->useT;
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
