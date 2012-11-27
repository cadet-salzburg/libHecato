#include <time.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include "HTBlobInterpreter.h"

HTBlobInterpreter::HTBlobInterpreter() : rClick(.07f), rDrag(.15f), blobID(0)
{
	//init
	for (unsigned i = 0; i < BLOB_LIST_SIZE; i++)
	{
		BlobRecord& b = records[i];
		b.blobID = 0;
		b.blobValid = false;
		b.confidence = 0;
		b.blobHandled = false;
		b.originX = 0.f;
		b.originY = 0.f;
		b.curX = 0.f;
		b.curY = 0.f;
		b.expectX = 0.f;
		b.expectY = 0.f;
		b.type = HTET_CLICK;
	}
}

HTBlobInterpreter::~HTBlobInterpreter()
{
}

void HTBlobInterpreter::setBoundary(int origX, int origY, int width, int height)
{
	oX = origX;
	oY = origY;
	oW = width;
	oH = height;
}

void HTBlobInterpreter::getRelCoords(int x, int y, float& relX, float& relY)
{
	relX = (float)(oX - x) / oW;
	relY = (float)(y - oY) / oH;
}

void HTBlobInterpreter::registerGenerator(HTIBlobGenerator* kt, int id)
{}

void HTBlobInterpreter::handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id)
{
	const unsigned char confLevel = 5;
	for (unsigned i = 0; i < BLOB_LIST_SIZE; i++)
	{
		BlobRecord* curRec = &(records[i]);
		curRec->blobHandled = false;
		if (curRec->blobValid && curRec->type == HTET_DRAG_NEW)
			curRec->type = HTET_DRAG_CONT;
	}

	for (unsigned i = 0; i < points.size(); i++)
	{

		HTIBlobResultTarget::SBlobResult* curPoint = &(points[i]);

		bool handled = false;
		float minDist = 999.f;
		unsigned minDistIndex = BLOB_LIST_SIZE;
		BlobRecord* curRec = 0;

		for (unsigned j = 0; j < BLOB_LIST_SIZE && !handled; j++)   //here we try to find a record's expect pos closest to our observation
		{
			curRec = &(records[j]);
			if (curRec->confidence == 0 || curRec->blobHandled)
				continue;

			float dist = getDist(curRec->expectX, curRec->expectY, curPoint->point.x, curPoint->point.y);
			if (dist < minDist)
			{
				minDist = dist;
				minDistIndex = j;
			}
		}

		if (minDistIndex == BLOB_LIST_SIZE) //haven't found a single record
			curRec = 0;
		else
			curRec = &(records[minDistIndex]);

		if (curRec && curRec->type == HTET_CLICK)
		{
			if (minDist < rClick)
			{
				//we stay in click event type
				curRec->blobHandled = true;
				if (curRec->confidence < confLevel)
				{
					curRec->confidence++;
					if (curRec->confidence == confLevel)
						curRec->blobValid = true;
				}
				handled = true;
			}

			if (minDist > rClick && minDist < rDrag)
			{
				//we change to drag event
				curRec->type = HTET_DRAG_NEW;
				curRec->curX = curRec->originX = curRec->expectX = curPoint->point.x;
				curRec->curY = curRec->originY = curRec->expectY = curPoint->point.y;
				if (curRec->confidence < confLevel)
				{
					curRec->confidence++;
					if (curRec->confidence == confLevel)
						curRec->blobValid = true;
				}
				curRec->blobHandled = true;
				handled = true;
			}
		}
		else if (curRec && (curRec->type == HTET_DRAG_NEW || curRec->type == HTET_DRAG_CONT))
		{
			if (minDist < rDrag)
			{
				curRec->originX = curRec->curX;
				curRec->originY = curRec->curY;
				curRec->curX = curPoint->point.x;
				curRec->curY = curPoint->point.y;
				curRec->expectX = curRec->curX + (curPoint->point.x - curRec->originX);
				curRec->expectY = curRec->curY + (curPoint->point.y - curRec->originY);
				if (curRec->confidence < confLevel)
				{
					curRec->confidence++;
					if (curRec->confidence == confLevel)
					{
						curRec->blobValid = true;
					}

				}
				curRec->blobHandled = true;
				handled = true;
			}
		}

		if (!handled)
		{
			unsigned newPos = 0;
			for (;records[newPos].confidence != 0; ++newPos);
			//new event
			BlobRecord& b = records[newPos];
			b.blobID = ++blobID;
			b.blobValid = false;
			b.confidence = 1;
			b.blobHandled = true;
			b.originX = curPoint->point.x;
			b.originY = curPoint->point.y;
			b.expectX = curPoint->point.x;
			b.expectY = curPoint->point.y;
			b.curX = curPoint->point.x;
			b.curY = curPoint->point.y;
			if (rClick == 0.f)
				b.type = HTET_DRAG_NEW;
			else
				b.type = HTET_CLICK;
			b.brtype = curPoint->type;
		}

	}

	std::vector<BlobRecord> results;
	for (unsigned i = 0; i < BLOB_LIST_SIZE; i++)
	{
		BlobRecord* curRec = &(records[i]);
		if (curRec->confidence == 0)
			continue;

		if (!curRec->blobHandled)
		{
			curRec->confidence--;
			if (curRec->confidence == 0)
			{
				if (curRec->type == HTET_DRAG_CONT)
					curRec->type = HTET_DRAG_OFF;
			}
		}

		if (!curRec->blobValid)
		{
			continue;
		}


		results.push_back(*curRec);
	}
	handleEvents(results);
}

bool HTBlobInterpreter::setClickRadius(float newRadius)
{
	if (newRadius < 0.f || newRadius > 1.f || newRadius > rDrag)
		return false;
	rClick = newRadius;
	return true;
}

bool HTBlobInterpreter::setDragRadius(float newRadius)
{
	if (newRadius < 0.f || newRadius > 1.f || newRadius < rClick)
		return false;
	rDrag = newRadius;
	return true;
}

const float& HTBlobInterpreter::getClickRadius() const
{
	return rClick;
}

const float& HTBlobInterpreter::getDragRadius() const
{
	return rDrag;
}
