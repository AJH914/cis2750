#ifndef HELPER
#define HELPER

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


GPXdoc* initDoc();

Waypoint* initWaypoint();

Route* initRoute();

GPXData* createGPXData(char* name, char* data);

Waypoint* createWaypoint(char* name, double longitude, double latitude);

Route* createRoute(char* name);

TrackSegment* createTrackSegment(void);

Track* createTrack(char* name);



//function to open file using file name, read contents of file into char*, return char*
char* fileOpener(char* filename);

//function to copy one string to another, starting and ending at respective indexes
char* stringCopy(char* string1, int startindex, int endindex);

GPXData* makeGPXData(xmlNode* node);

Waypoint* makeWaypoint(xmlNode* node);

Route* makeRoute(xmlNode* node);

TrackSegment* makeTrackSegment(xmlNode* node);

Track* makeTrack(xmlNode* node);

char* getName(xmlNode* node);

bool isName(xmlNode* node);

bool isText(xmlNode* node);



//A2 HELPER FUNCTIONS-----------------------

xmlNode* gpxDataToNode(GPXData* data, xmlNs* namespace);

xmlNode* nameToNode(char* name, xmlNs* namespace);

xmlNode* waypointToNode(Waypoint* wpt, char* name, xmlNs* namespace);

xmlNode* routeToNode(Route* rte, xmlNs* namespace);

xmlNode* segmentToNode(TrackSegment* seg, xmlNs* namespace);

xmlNode* trackToNode(Track* trk, xmlNs* namespace);

xmlNode* gpxDocToNode(GPXdoc* doc);

float calcdistance(float lat1, float lat2, float lon1, float lon2);

TrackSegment* getSegWithWaypoint(List* segments, int direction);

bool validateData(GPXData* data);

bool validateWaypoint(Waypoint* wpt);

bool validateRoute(Route* rte);

bool validateSegment(TrackSegment* seg);

bool validateTrack(Track* trk);

bool validateDoc(GPXdoc* doc);

void dummyDelete(void* data);

char* gpxDataToJSON(void* gpx);

char* listToJSON(List* list, char* (*toJSON)(void*));

char* routeToJSON2(void* data);

char* trackToJSON2(void* data);

int getIndex(char* string, char c);

char* gpxFileToJSON(char* filename);

bool validateGPXFile(char* filename, char* schemaFile);

char* routeComponentToJSON(int routeNum, Route* route);

char* trackComponentToJSON(int trackNum, Track* track);

char* gpxComponentsToJSON(char* filename);

int getNumWaypointsTrack(Track* track);

char* otherDataListToJSON(char* filename, char* componentName);

bool changeName(char* filename, char* oldName, char* newName);

char* routesFromFileToJson(char* filename);

char* waypointToJson(Waypoint* wpt, int index);

char* waypointsFromFileToJson(char* filename, char* routeName);

#endif