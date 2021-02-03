#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"
#include "GPXParser.h"
#include "GPXHelpers.h"


GPXdoc* createGPXdoc(char* filename){
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filename);
    }
    GPXdoc* gpxdoc= calloc(1, sizeof(GPXdoc));
    gpxdoc->creator = calloc(256, sizeof(char));
    gpxdoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxdoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    gpxdoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    root_element = xmlDocGetRootElement(doc);

    for (xmlNode* child = root_element->children; child!=NULL; child=child->next){
        char*name = (char*)child->name;
        if (strcmp(name, "rte")==0){
            Route* r = makeRoute(child);
            insertBack(gpxdoc->routes, r);
        }
        if (strcmp(name, "trk")==0){
            Track* t = makeTrack(child);
            insertBack(gpxdoc->tracks, t);
        }
        if (strcmp(name, "wpt")==0){
            Waypoint* w = makeWaypoint(child);
            insertBack(gpxdoc->waypoints, w);
        }
    }
    //get creator, version, namespace
    return gpxdoc;
}

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc);

/** Function to delete doc content and free all the memory.
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object had been freed
 *@return none
 *@param obj - a pointer to an GPX struct
**/
void deleteGPXdoc(GPXdoc* doc){
    if (doc==NULL){
        return;
    }
    GPXdoc* temp= (GPXdoc*)doc;
    free(temp->creator);
    freeList(temp->routes);
    freeList(temp->tracks);
    freeList(temp->waypoints);
    free(temp);
}

/* For the five "get..." functions below, return the count of specified entities from the file.  
They all share the same format and only differ in what they have to count.
 
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return the number of entities in the GPXdoc object
 *@param obj - a pointer to an GPXdoc struct
 */


//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc);

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc);

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc);

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc);

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc);

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name);
// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name);
// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name);

//---------HELPER FUNCTIONS---------

void deleteGpxData( void* data){
    if (data==NULL){
        return;
    }
    GPXData* temp = (GPXData*)data;
    free(temp);
}
char* gpxDataToString( void* data){
    return NULL;
}
int compareGpxData(const void *first, const void *second){
    return -1;
}

void deleteWaypoint(void* data){
    if (data==NULL){
        return;
    }
    Waypoint* temp=(Waypoint*)data;
    free(temp->name);
    freeList(temp->otherData);
    free(temp);
}
char* waypointToString( void* data){
    return NULL;
}
int compareWaypoints(const void *first, const void *second){
    return -1;
}

void deleteRoute(void* data){
    if (data==NULL){
        return;
    }
    Route* temp=(Route*)data;
    free(temp->name);
    freeList(temp->otherData);
    freeList(temp->waypoints);
    free(temp);
}
char* routeToString(void* data){
    return NULL;
}
int compareRoutes(const void *first, const void *second){
    return -1;
}

void deleteTrackSegment(void* data){
    if (data==NULL){
        return;
    }
    TrackSegment* temp=(TrackSegment*)data;
    freeList(temp->waypoints);
    free(temp);
}
char* trackSegmentToString(void* data){
    return NULL;
}
int compareTrackSegments(const void *first, const void *second){
    return -1;

}

void deleteTrack(void* data){
    if (data==NULL){
        return;
    }
    Track* temp=(Track*)data;
    free(temp->name);
    freeList(temp->otherData);
    freeList(temp->segments);
    free(temp);
}
char* trackToString(void* data){
    return NULL;
}
int compareTracks(const void *first, const void *second){
    return -1;
}