#include <tinyxml.h>
#include "HTAppSettings.h"
#include "HTApp.h"


HTApp::HTApp() : sf::RenderWindow(sf::VideoMode(1920, 480), "HTApp"),
                                   circle(15.f),
                                   rectShape(sf::Vector2f(4.f, getSize().y)),
                                   sender(0),
                                   recv(0),
                                   tuioServer(0)
{
    printf("=====\nHTAPP\n=====\n");
    sf::Vector2u winDim = getSize();
    winW = (float)(winDim.x);
    winH = (float)(winDim.y);
    if (HTAppSettings::useSender())
    {
        printf("Using sender...\n");
        sender = new HTNetworkSender();
        sender->setHostAndPort(HTAppSettings::getSenderHost(), HTAppSettings::getSenderPort());
    }
    else
    {
        printf("Sender: OFF.\n");
    }

    if (HTAppSettings::useReceiver())
    {
        printf("Using receiver...\n");
        recv = new HTNetworkReceiver(HTAppSettings::getReceiverPort());
        recv->setBlobResultTarget(this);
    }
    else
    {
        printf("Receiver: OFF.\n");
    }

    if (HTAppSettings::useTUIO())
    {
        printf("Using TUIO...\n");
        tuioServer = new HTTUIOServer();
        tuioServer->setupConnection(HTAppSettings::getTUIOHost(), HTAppSettings::getTUIOPort());
        tuioServer->setMargins(HTAppSettings::getLeftMargin(), HTAppSettings::getRightMargin());
    }
    else
    {
        printf("TUIO: OFF.\n");
    }

    xn::NodeInfoList devices;
    xn::NodeInfoList sensors;

    std::map<int, int> mappings;
    HTDevice::getBusMappings("../mapping.xml", mappings);

    ctx.Init();
    int numDevs = 0;
    XnStatus status = ctx.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, devices);
    if (status != XN_STATUS_OK || (devices.Begin() == devices.End()))
    {
        printf("HTApp: Enumeration of devices failed. No local cameras detected.\n");
    }
    int id = -1;
    for (xn::NodeInfoList::Iterator iter = devices.Begin(); iter != devices.End(); ++iter)
    {
        unsigned char bus = 0;
        unsigned short vendor_id;
        unsigned short product_id;
        unsigned char address;
        sscanf ((*iter).GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
        printf("VENDOR: %x, PRODUCTID: %x, connected %i @ %i\n", vendor_id, product_id, bus, address);

        //use only xTion or kinect cameras:
        if ((vendor_id != 0x1d27 || product_id != 0x600) /*xtion*/ && (vendor_id != 0x45e || product_id != 0x2ae) && (vendor_id != 0x45e || product_id != 0x2c2) /* Kinect (xbox)*/ )
        {
            printf("HTApp: No entry for this device!\n");
            continue;
        }


        xn::NodeInfo deviceInfo = *iter;
        xn::DepthGenerator* depthGen = new xn::DepthGenerator();
        xn::Query q;
        //create a production tree that's dependent on the current device
        ctx.CreateProductionTree(deviceInfo);
        q.AddNeededNode(deviceInfo.GetInstanceName());
        ctx.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &q, *depthGen);
        //find the id matching the bus the device is connected to
        std::map<int, int>::iterator mapIter = mappings.find(bus);
        if (mapIter == mappings.end())
        {
            printf("WARNING: No entry for bus %i found!\n", bus);
            id++;
        }
        else
        {
            id = mapIter->second;
        }

        printf("BUS IS: %i, put it as id: %i\n", bus, id);
        HTDeviceThreaded* k = new HTDeviceThreaded(depthGen, id);
        k->setBlobResultTarget(this);
        k->setEnablePersonTracking(HTAppSettings::usePersonTracking());
        ktt.push_back(k);
        if (sender)
            sender->registerGenerator(k, numDevs);
        printf("HTApp: Adding Device %i to production.\n", numDevs);
        numDevs++;
    }

    setCorrelationRadiusW(HTAppSettings::getCorrelationW());
    setCorrelationRadiusH(HTAppSettings::getCorrelationH());
    setCorrelationPerson(HTAppSettings::getPersonCorrelation());

    //in this example we won't need any generation of click events, so set this to 0.f
    printf("HTApp: Setting click radius to: %f\n", HTAppSettings::getRadiusClick());
    printf("HTApp: Setting drag  radius to: %f\n", HTAppSettings::getRadiusDrag());
    setClickRadius(HTAppSettings::getRadiusClick());
    setDragRadius(HTAppSettings::getRadiusDrag());
}

HTApp::~HTApp()
{
    for (unsigned i = 0; i < ktt.size(); i++)
    {
        delete ktt[i];
    }
    if (sender)
        delete sender;
    if (recv)
        delete recv;
    if (tuioServer)
        delete tuioServer;
}

void HTApp::update()
{
    ctx.WaitAndUpdateAll();
    const unsigned len = ktt.size();
    for (unsigned i = 0; i < len; i++)
    {
        ktt[i]->compute();
    }
}

void HTApp::lockAndDrawAll()
{
	drawMutti.lock();
	char suffix[16] = {0};
	std::string drawInfo;
	drawInfo = "TUIO    : ";
	HTAppSettings::useTUIO() ? drawInfo += "YES\n" : drawInfo += "NO\n";
	drawInfo += "SENDER  : ";
	HTAppSettings::useSender() ? drawInfo += "YES\n" : drawInfo += "NO\n";
	drawInfo += "RECEIVER: ";
	HTAppSettings::useReceiver() ? drawInfo += "YES\n" : drawInfo += "NO\n";
	drawInfo += "PERSTRACKING: ";
	HTAppSettings::usePersonTracking() ? drawInfo += "YES\n" : drawInfo += "NO\n";
	drawInfo += "Registered Generators: ";
	sprintf(suffix, "%u", getNumGenerators());
	drawInfo += suffix;
	sf::Text infoText(drawInfo);
	infoText.setCharacterSize(15);
	infoText.setPosition(10, 10);
	draw(infoText);
    const unsigned len = drawBlobs.size();
    for (unsigned i = 0; i < len; i++)
    {
        //CASE 1: we have a hand blob
        if (drawBlobs[i].brtype == HTIBlobResultTarget::BRT_HAND)
        {
            sprintf(suffix, "%u", (drawBlobs[i].id));
            std::string blobid = "";
            blobid += suffix;
            circle.setPosition(drawBlobs[i].vec);
            draw(circle);
            sf::String str(blobid);
            sf::Text t(str);
            t.setCharacterSize(20);
            t.setPosition(drawBlobs[i].vec.x, drawBlobs[i].vec.y+30.f);
            draw(t);
        }
        //CASE 2: we have a person blob
        else
        {
            rectShape.setPosition(drawBlobs[i].vec.x, 0.f);
            draw(rectShape);
        }
    }
//    drawBlobs.clear();
}

void HTApp::unlock()
{
	drawMutti.unlock();
}

void HTApp::handleEvents(const std::vector<HTBlobInterpreter::BlobRecord>& events)
{
	drawMutti.lock();
    drawBlobs.clear();
    const unsigned len = events.size();

    for (unsigned i = 0; i < len; i++)
    {
        const HTBlobInterpreter::BlobRecord* rec = &(events[i]);
        drawBlobs.push_back(BlobDisplay(rec->curX * winW, rec->curY * winH, rec->blobID, rec->brtype));

    }
	drawMutti.unlock();
    if (tuioServer)
        tuioServer->handleEvents(events);
}

void HTApp::handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id)
{
    if (sender)
        sender->handleBlobResult(points, id);

    HTBlobInterpreterMulti::handleBlobResult(points, id);
}

void HTApp::onResize()
{
    winW = (float)(getSize().x);
    winH = (float)(getSize().y);
}


