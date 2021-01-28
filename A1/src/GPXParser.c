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

//---------HELPER FUNCTIONS---------

void deleteGpxData( void* data){

}
char* gpxDataToString( void* data){
    return NULL;
}
int compareGpxData(const void *first, const void *second){
    return -1;
}

void deleteWaypoint(void* data){

}
char* waypointToString( void* data){
    return NULL;
}
int compareWaypoints(const void *first, const void *second){
    return -1;
}

void deleteRoute(void* data){

}
char* routeToString(void* data){
    return NULL;
}
int compareRoutes(const void *first, const void *second){
    return -1;
}

void deleteTrackSegment(void* data){

}
char* trackSegmentToString(void* data){
    return NULL;
}
int compareTrackSegments(const void *first, const void *second){
    return -1;

}

void deleteTrack(void* data){

}
char* trackToString(void* data){
    return NULL;
}
int compareTracks(const void *first, const void *second){
    return -1;
}