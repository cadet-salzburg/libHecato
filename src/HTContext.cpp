#include <XnCppWrapper.h>
#include <tinyxml.h>
#include "HTContext.h"
#include "HTDeviceThreaded.h"
#include "version.h"

HTContext* HTContext::instance = 0;

HTContext::HTContext() : initialized(false)
{
    printf("LIBHECATO: v%s%s, BUILD: %li (%s-%s-%s)\n", AutoVersion::FULLVERSION_STRING, AutoVersion::STATUS_SHORT, AutoVersion::BUILDS_COUNT, AutoVersion::YEAR, AutoVersion::MONTH, AutoVersion::DATE);
    ctx = new xn::Context();
}

HTContext::~HTContext()
{
    HTContext* inst = getInstance();
    for (unsigned int i = 0; i < inst->devices.size(); i++)
    {
        delete inst->devices[i];
    }
    inst->devices.clear();
    delete ctx;
}

const std::vector<HTDeviceThreaded*>& HTContext::getDevices()
{
    return getInstance()->devices;
}

void HTContext::updateAll()
{
    HTContext* inst = getInstance();
    if (!inst->initialized)
    {
        printf("HTCONTEXT: Not initialized! Call initialize() first!\n");
        return;
    }
    inst->ctx->WaitAndUpdateAll();
    const unsigned int len = inst->devices.size();

    for (unsigned i = 0; i < len; i++)
    {
        inst->devices[i]->compute();
    }
}

void HTContext::shutdown()
{
    delete getInstance();
}


HTContext* HTContext::getInstance()
{
    if (!instance)
    {
        instance = new HTContext();
    }

    return instance;
}

void HTContext::getBusMappings(std::map<int, int>& m)
{
    TiXmlDocument doc("../mapping.xml");
	if (!doc.LoadFile())
	{
		printf("GETBUSMAPPINGS: Unable to read config file!\n");
		return;
	}
	TiXmlHandle handle(&doc);
	TiXmlElement* curElem = handle.FirstChildElement().FirstChildElement("mapping").ToElement();

	if (!curElem)
	{
		printf("GETBUSMAPPINGS: Unable to find \"mapping\" elements!\n");
		return;
	}


	for (;curElem != 0; curElem = curElem->NextSiblingElement("mapping"))
	{
		int bus;
		int address;
		curElem->QueryIntAttribute("bus", &bus);
		curElem->QueryIntAttribute("id", &address);
		m.insert(std::pair<int, int>(bus, address));
		printf("MAPPING: BUS: %i to ADDRESS: %i\n", bus, address);
	}
}

bool HTContext::initialize()
{
    HTContext* inst = getInstance();
    getBusMappings(inst->mapping);

	xn::NodeInfoList devices;
	xn::NodeInfoList sensors;

	if (inst->initialized)
    {
        printf("HTCONTEXT: Already initialized! Call shutdown() first!\n");
        return false;
    }

	inst->ctx->Init();
	int numDevs = 0;
	XnStatus status = inst->ctx->EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, devices);
	if (status != XN_STATUS_OK || (devices.Begin() == devices.End()))
	{
		printf("HTCONTEXT: Enumeration of devices failed.\n");
		return false;
	}
	int id = -1;
	for (xn::NodeInfoList::Iterator iter = devices.Begin(); iter != devices.End(); ++iter, ++numDevs)
	{
		unsigned char bus = 0;
		unsigned short vendor_id;
		unsigned short product_id;
		unsigned char address;
		sscanf ((*iter).GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
		printf("VENDOR: %x, PRODUCTID: %x, connected %i @ %i\n", vendor_id, product_id, bus, address);

		//use only xTion cameras:
		if ((vendor_id != 0x1d27 || product_id != 0x600) && (vendor_id != 0x45e || product_id != 0x2ae))
		{
			printf("HTCONTEXT: No entry for this device!\n");
			continue;
		}


		xn::NodeInfo deviceInfo = *iter;
		xn::DepthGenerator* depthGen = new xn::DepthGenerator();
		xn::Query q;
		//create a production tree that's dependent on the current device
		inst->ctx->CreateProductionTree(deviceInfo);
		q.AddNeededNode(deviceInfo.GetInstanceName());
		inst->ctx->CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &q, *depthGen);
		//find the id matching the bus the device is connected to
		std::map<int, int>::iterator mapIter = inst->mapping.find(bus);
		if (mapIter == inst->mapping.end())
		{
			printf("WARNING: No entry for bus %i found!\n", bus);
			id++;
			id += 100;
		}
		else
		{
			id = mapIter->second;
		}

		printf("BUS IS: %i, put it as id: %i\n", bus, id);
		inst->devices.push_back(new HTDeviceThreaded(depthGen, id));
	}
	inst->initialized = true;
	return true;
}
