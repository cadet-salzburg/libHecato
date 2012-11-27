#include "TUIO/TuioServer.h"
#include "HTTUIOServer.h"
#include "HTKalmanFilter.h"

using namespace TUIO;

HTTUIOServer::HTTUIOServer() : HTBlobInterpreter(), tuioServer(0), lMargin(0.f), rMargin(1.f)
{

}

HTTUIOServer::~HTTUIOServer()
{
	if (tuioServer)
		delete tuioServer;
}

void HTTUIOServer::setupConnection(const char* host, int port)
{
	if (tuioServer)
	{
		printf("HTTUIOServer::setupConnection : Connection already set up, skipping...\n");
		return;
	}

	tuioServer = new TuioServer(host, port);
}

void HTTUIOServer::setMargins(float left, float right)
{
	lMargin = left;
	rMargin = right;
	printf("TUIOSERVER: Margins: %2.2f, %2.2f\n", lMargin, rMargin);
}

void HTTUIOServer::handleEvents(const std::vector<BlobRecord>& events)
{
	const unsigned len = events.size();
	if (len == 0)
		return;

	if (!tuioServer || !tuioServer->isConnected())
	{
		printf("HTTUIOServer::handleEvents : Connection not set up, skipping...\n");
		return;
	}

	TuioCursor* cursor = 0;
	std::map<unsigned int, TuioOutput>::iterator curIter;

	tuioServer->initFrame(TUIO::TuioTime::getSessionTime());


	for (unsigned i = 0; i < len; i++)
	{
		const BlobRecord* rec = &(events[i]);
		//for now, skip person events
		//float cX = (float)((int)((lMargin + (rec->curX * (rMargin - lMargin))) * 1000.f)) / 1000.f;
		//float cY = (float)((int)(rec->curY * 1000.f) / 1000.f);
		float cX = (lMargin + (rec->curX * (rMargin - lMargin)));
		float cY = 0.f;

		if (rec->brtype == HTIBlobResultTarget::BRT_HAND)
			cY = rec->curY;
		else
			printf("PERSON!\n");

		switch (rec->type)
		{
		case HTBlobInterpreter::HTET_DRAG_NEW:
			cursor = tuioServer->addTuioCursor(cX, cY);
			printf("Adding   Cursor... BlobID: %i SID: %li (X: %f, Y: %f)\n", rec->blobID, cursor->getSessionID(), cursor->getPosition().getX(), cursor->getPosition().getY());
			TuioOutput to;
			to.cursor = cursor;
			to.filter = new HTKalmanFilter(cX, cY);
			cursorMap.insert(std::pair<unsigned int, TuioOutput>(rec->blobID, to));
			break;

		case HTBlobInterpreter::HTET_DRAG_CONT:
			curIter = cursorMap.find(rec->blobID);
			if (curIter == cursorMap.end())
			{
				printf("(HTTUIOServer::handleEvents) WARNING: DRAG_CONT: Cursor (ID: %i) not found which should be in the list. Continuing...\n", rec->blobID);
				break;
			}
			curIter->second.filter->updateMeasurement(cX, cY, &cX, &cY);
			//            if (cX > .98f || cX < 0.02f || cY > .98f || cY < .02f)
			//            {

			//            }
			if (cX > .999f)
				cX = .999f;
			if (cX < 0.f)
				cX = 0.f;

			if (cY > .999f)
				cY = .999f;
			if (cY < 0.f)
				cY = 0.f;

			printf("CUR %f, %f\n", cX, cY);

			tuioServer->updateTuioCursor(curIter->second.cursor, cX, cY);
			break;

		case HTBlobInterpreter::HTET_DRAG_OFF:
			curIter = cursorMap.find(rec->blobID);
			if (curIter == cursorMap.end())
			{
				printf("(HTTUIOServer::handleEvents) WARNING: DRAG_OFF: Cursor (ID: %i) not found which should be in the list. Continuing...\n", rec->blobID);
				break;
			}
			delete curIter->second.filter;
			printf("Removing Cursor... BlobID: %i SID: %li (mapsize: %lu)\n", rec->blobID, curIter->second.cursor->getSessionID(), cursorMap.size() - 1);
			tuioServer->removeTuioCursor(curIter->second.cursor);
			cursorMap.erase(curIter);
			break;

		default:
			break;
		}
	}

	tuioServer->commitFrame();
}
