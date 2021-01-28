#ifndef HELPER
#define HELPER

#include "GPXParser.h"

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

#endif