#ifndef HTBLOBINTERPRETER_H
#define HTBLOBINTERPRETER_H

#include <vector>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <core/core.hpp>

#include "HTIBlobResultTarget.h"

#define BLOB_LIST_SIZE 32
//#ifdef _WIN32
//#define uint32_t unsigned int
//#define uint64_t long unsigned int
//#endif


//!Base class that does basic mouse-style event interpretation
/**HTBlobInterpreter generates drag- and click events respectively.
Derive and implement the pure virtual function \see handleEvent to be notified
whenever an event is generated.*/
class HTBlobInterpreter : public HTIBlobResultTarget
{
public:
	//!Denotes the event type
	enum HTEventType
	{
		//!A click event has been generated
		HTET_CLICK,
		//!A new drag event has been generated
		HTET_DRAG_NEW,
		//!An existing drag event has updated coordinates
		HTET_DRAG_CONT,
		//!An existing drag event has ceased to continue
		HTET_DRAG_OFF
	};
	//!Struct to store all the relevant information about an event
	/**Note that all coordinate-relevant information is stored as
	floats [0..1] spanning from the upper-left corner of the
	detection area (similar to relative screen coordinates).
	The values are relative to the boundary set in the HTDevice
	class*/
	struct TrackRecord
	{
		//!A unique event ID. Useful for continuous drag events
		unsigned int blobID;
		//!The record's time stamp (system millis)
		uint64_t tstamp;
		//!The originating position if HTEventType is HTET_DRAG
		/**If the event type is HTET_CLICK, the values equal those of curX, curY*/
		float originX;
		//!The originating position if HTEventType is HTET_DRAG
		/**If the event type is HTET_CLICK, the values equal those of curX, curY*/
		float originY;
		//!The current position of the record
		float curX;
		//!The current position of the record
		float curY;
		//!The TrackRecord's event type
		HTEventType type;
		//!This shows whether the blob record is valid. (False if blob has just been created but below confidence)
		bool blobValid;
		//!The current confidence level.
		unsigned char confidence;
		//!Used by HTBlobInterpreter internally.
		bool blobHandled;
		//!Used by HTBlobInterpreter internally.
		float expectX;
		//!Used by HTBlobInterpreter internally.
		float expectY;
		HTIBlobResultTarget::BlobResultType brtype;
	};

public:
	//!The default constructor
	HTBlobInterpreter();
	//!The default destructor
	virtual ~HTBlobInterpreter();
	//!This function is called by the base class
	/**Whenever an event is generated (click, drag), this function is called
	in the deriving class.
	\param events Event record that stores all the relevant information*/
	virtual void handleEvents(const std::vector<TrackRecord>& events) = 0;
	virtual void handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id);
	virtual void registerGenerator(class HTIBlobGenerator* kt, int id);
	//!DEPRECATED
	void setBoundary(int origX, int origY, int width, int height);
	//!Set a new radius in which blobs are evaluated as click event
	/**Defines the radius in which consecutive blobs are still counted
	as a click rather than a drag event.
	\param newRadius the new radius in relative coordinates (e.g. 0.05f means 5% of the total width, height)
	\return true if successful, false if out of bounds or greater than drag radius
	\note If you do not wish to have any click events generated (e.g. only need drag events) set this value to zero.*/
	bool setClickRadius(float newRadius);
	//!Set a new radius in which blobs are evaluated as drag event
	/**Defines the radius in which consecutive blobs are still counted
	as one drag event rather separate ones (correlation).
	\param newRadius the new radius in relative coordinates (e.g. 0.05f means 5% of the total width, height)
	\return true if successful, false if out of bounds or smaller than click radius
	*/
	bool setDragRadius(float newRadius);
	//!Get the radius in which blobs are evaluated as click event
	/**Defines the radius in which consecutive blobs are still counted
	as a click rather than a drag event.
	\return the radius in relative coordinates (e.g. 0.05f means 5% of the total width, height)*/
	const float& getClickRadius() const;
	//!Get the radius in which blobs are evaluated as drag event
	/**Defines the radius in which consecutive blobs are still counted
	as one drag event rather separate ones (correlation).
	\return the radius in relative coordinates (e.g. 0.05f means 5% of the total width, height)*/
	const float& getDragRadius() const;
	//!Helper function to get the distance between two coordinates
	inline float getDist(float x1, float y1, float x2, float y2)
	{
		return sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
	}

protected:
	//!DEPRECATED
	void getRelCoords(int x, int y, float& relX, float& relY);
	//!Storage for currently active TrackRecords
	TrackRecord records[BLOB_LIST_SIZE];

private:
	int oX, oY, oW, oH;
	float rClick, rDrag;
	;
	uint32_t blobID;


};




//! Debug implementation of HTIBlobResultTarget that outputs to the console
class HTBlobInterpreterConsole : public HTBlobInterpreter
{
	void handleEvents(const std::vector<TrackRecord>& events)
	{
		const unsigned len = events.size();
		for(unsigned i = 0; i < len; i++)
		{
			const TrackRecord& event = events[i];
			switch(event.type)
			{
			case HTET_CLICK:
				printf("CLICK AT: %2.3f, %2.3f\n", event.originX, event.originY);
				break;

			case HTET_DRAG_CONT:
				printf("DRAG FROM: %2.3f, %2.3f TO %2.3f, %2.3f\n", event.originX, event.originY, event.curX, event.curY);
				break;

			default:
				break;

			}
		}
	}

	void registerGenerator(HTIBlobGenerator* _target, int id)
	{
		printf("NEW GENERATOR REGISTERED! ID: %i\n", id);
	}
};

#endif
