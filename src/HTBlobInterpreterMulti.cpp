#ifndef WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "HTBlobInterpreterMulti.h"

HTBlobInterpreterMulti::HTBlobInterpreterMulti() : HTBlobInterpreter(),
	numInstances(0),
	curInstances(0),
	corrRadiusW(0.08f),
	corrRadiusH(0.25f),
	corrPerson(0.25f)
{
	for (unsigned i = 0; i < MAX_GENERATORS; i++)
	{
		generators[i] = false;
	}
}

HTBlobInterpreterMulti::~HTBlobInterpreterMulti()
{
}

void HTBlobInterpreterMulti::registerGenerator(HTIBlobGenerator* kt, int id)
{
	numInstances++;
	stepWidth = 1.f / numInstances;
}

void HTBlobInterpreterMulti::setCorrelationRadiusW(float newDist)
{
	corrRadiusW = newDist;
}

void HTBlobInterpreterMulti::setCorrelationRadiusH(float newDist)
{
	corrRadiusH = newDist;
}

int HTBlobInterpreterMulti::getNumGenerators()
{
	return numInstances;
}

void HTBlobInterpreterMulti::setCorrelationPerson(float newDist)
{
	corrPerson = newDist;
}

void HTBlobInterpreterMulti::handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id)
{
	mutti.lock();

	if (id < 0 || id >= MAX_GENERATORS)
	{
		printf("HTBLOBINTERPRETERMULTI: Generator ID out of bounds (%i)!\n", id);
		mutti.unlock();
		return;
	}

	if (generators[id])
	{
		printf("Generator %i has already dumped data!\n", id);
		mutti.unlock();
		return;
	}
	generators[id] = true;


	const unsigned len = points.size();

	for (unsigned i = 0; i < len; i++)
	{
		//for the virtual global space, project the x coord into global space
		globalPoints.push_back(BlobHandled(points[i].type, points[i].point.x*stepWidth + id * stepWidth, points[i].point.y, points[i].point.z, id));
	}

	curInstances++;
	if (curInstances == numInstances)
	{
		//  printf("Global points: %lu\n", globalPoints.size());
		std::vector<HTIBlobResultTarget::SBlobResult> corrBlobs;
		correlateBlobs(corrBlobs);
		globalPoints.clear();
		HTBlobInterpreter::handleBlobResult(corrBlobs, 0xBEEF);
		curInstances = 0;
		for (unsigned i = 0; i < MAX_GENERATORS; i++)
		{
			generators[i] = false;
		}
	}

	mutti.unlock();
}

void HTBlobInterpreterMulti::correlateBlobs(std::vector<HTIBlobResultTarget::SBlobResult>& corrBlobs)
{
	const unsigned len = globalPoints.size();
	BlobHandled *compare, *compareTo;

	for (unsigned i = 0; i < len; i++)
	{
		compare = &(globalPoints[i]);

		if (compare->handled)
			continue;

		float meanX = compare->x, meanY = compare->y, nBlobs = 1.f, meanZ = 10100.f;

		switch (compare->type)
		{
		case HTIBlobResultTarget::BRT_HAND:
			for (unsigned j = i+1; j < len; j++)
			{
				compareTo = &(globalPoints[j]);
				if (compareTo->type != compare->type || compareTo == compare || compareTo->handled || compare->originId == compareTo->originId)
					continue;

				//this here is executed when two neighboring blobs are to be correlated
				float dX = compare->x - compareTo->x;
				if (dX < 0.f)
					dX = -dX;
				float dY = compare->y - compareTo->y;
				if (dY < 0.f)
					dY = -dY;
				if(dX < corrRadiusW && dY < corrRadiusH)
				{
					//calc mean
					meanX += compareTo->x;
					meanY += compareTo->y;
					nBlobs++;
					compareTo->handled = true;
				}
			}
			corrBlobs.push_back(HTIBlobResultTarget::SBlobResult(compare->type, meanX / nBlobs, meanY / nBlobs));
			compare->handled = true;
			break;

		case HTIBlobResultTarget::BRT_PERSON:
			meanX = compare->x;
			meanY = compare->y;
			meanZ = compare->z;
			for (unsigned j = i+1; j < len; j++)
			{
				compareTo = &(globalPoints[j]);
				if (compareTo->type != compare->type || compareTo == compare || compareTo->handled || compare->originId == compareTo->originId)
					continue;

				//this here is executed when two neighboring blobs are to be correlated
				float dX = compare->x - compareTo->x;
				if (dX < 0.f)
					dX = -dX;
				//                float dY = compare->y - compareTo->y;
				//                if (dY < 0.f)
				//                    dY = -dY;
				if(dX < corrPerson)
				{
					if (meanZ > compareTo->z)
					{
						meanX = compareTo->x;
						meanY = compareTo->y;
						meanZ = compareTo->z;
					}
					compareTo->handled = true;
				}
			}
			corrBlobs.push_back(HTIBlobResultTarget::SBlobResult(compare->type, meanX, meanY, meanZ));
			compare->handled = true;
			break;
		}


	}
}
