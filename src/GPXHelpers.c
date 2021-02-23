#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GPXHelpers.h"
#include "GPXParser_A2temp.h"

char* fileOpener(char* filename){
    if (filename==NULL){
        return NULL;
    }
    else if (strcmp(filename, "")==0){
        return NULL;
    }
    else{
        FILE* newFile=fopen(filename, "r");
        fseek(newFile, 0L, SEEK_END);
        unsigned long numBytes=ftell(newFile);
        rewind(newFile);

        char* content = calloc(numBytes +10, sizeof(char));
        int counter=0;

        while(1){
            char c=fgetc(newFile);
            if (feof(newFile)){
                break;
            }
            else{
                content[counter]=c;
                counter++;
            }
        }
        fclose(newFile);
        return content;
    }
}

char* stringCopy(char* string1, int startindex, int endindex){
    //if string1 is null, return NULL
    if (string1==NULL){
        return NULL;
    }
    //check if startindex is out of bounds
    if (startindex<0 || startindex>strlen(string1)){
        return NULL;
    }
    //check if endindex is out of bounds
    if (endindex<0 || endindex>strlen(string1)){
        return NULL;
    }
    //check if startindex is greater than endindex
    if (startindex>endindex){
        return NULL;
    }
    //set string2
    char* string2=calloc((endindex-startindex)+10, sizeof(char));
    //set counter for location in string2
    int index=0;
    //assign chosen characters in string1 to string2
    for (int i=startindex; i<endindex; i++){
        string2[index]=string1[i];
        index++;
    }
    return string2;
}

///////////////////////CONSTRUCTORS/////////////////////////

char* getName(xmlNode* node){
    xmlNode* newNode=NULL;
    for (xmlNode* n = node->children; n!=NULL; n=n->next){
        char* name = (char*)n->name;
        if (strcmp(name, "name")==0){
            newNode= n->children;
            break;
        }
    }
    if(newNode==NULL){
        return "";
    }
    char*name=(char*)newNode->content;
    if (name==NULL){
        return "";
    }
    return name;
}

GPXData* createGPXData(char* name, char* data){
    if (name==NULL || data==NULL){
        return NULL;
    }
    else{
        GPXData* newdata = calloc(1, sizeof(GPXData)+256);
        strcpy(newdata->value, data);
        strcpy(newdata->name, name);
        return newdata;
    }
}

GPXData* makeGPXData(xmlNode* node){
    return createGPXData((char*)node->name, (char*)node->children->content);
}

Waypoint* createWaypoint(char* name, double longitude, double latitude){
    if (name==NULL){
        return NULL;
    }
    else if (latitude < -90.00 || latitude > 90.00){
        return NULL;
    }
    else if (longitude<-180.00 || longitude > 180.00){
        return NULL;
    }
    else{
        Waypoint* newWaypoint = calloc(1, sizeof(Waypoint));
        newWaypoint->name=stringCopy(name, 0, strlen(name));
        newWaypoint->otherData=initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        newWaypoint->latitude=latitude;
        newWaypoint->longitude=longitude;
        return newWaypoint;
    }
}

Waypoint* makeWaypoint(xmlNode* node){
    char* name= getName(node);
    double lat=0.00;
    double lon=0.00;
    for(xmlAttr* a = node->properties; a!=NULL; a=a->next){
        xmlNode *value = a->children;
        char *attrName = (char *)a->name;
        char *cont = (char *)(value->content);
        if(strcmp(attrName, "lat")==0){
            lat=atof(cont);
        }
        if(strcmp(attrName, "lon")==0){
            lon=atof(cont);
        }
    }
    Waypoint* newWaypoint= createWaypoint(name, lon, lat);
    for (xmlNode* child = node->children; child!=NULL; child=child->next){
        char* name= (char*)child->name;
        if(strcmp(name, "name")!=0 && isText(child)==false){
            GPXData* g = makeGPXData(child);
            insertBack(newWaypoint->otherData, g);
        }
    }
    return newWaypoint;
}

Route* createRoute(char* name){
    if (name==NULL){
        return NULL;
    }
    else{
        Route* newRoute = calloc(1, sizeof(Route));
        newRoute->waypoints=initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
        newRoute->otherData=initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        newRoute->name=stringCopy(name, 0, strlen(name));
        return newRoute;
    }
}

Route* makeRoute(xmlNode* node){
    char* name =getName(node);
    Route* newRoute=createRoute(name);
    for(xmlNode* child = node->children; child!=NULL; child=child->next){
        if(strcmp((char*)child->name, "rtept")==0){
            Waypoint* w = makeWaypoint(child);
            insertBack(newRoute->waypoints, w);
        }
        else if (isText(child)==false && isName(child)==false){
            GPXData* g = makeGPXData(child);
            insertBack(newRoute->otherData, g);
        }
    }
    return newRoute;
}

TrackSegment* createTrackSegment(void){
    TrackSegment* newTrackSegment = calloc(1, sizeof(TrackSegment));
    newTrackSegment->waypoints=initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    return newTrackSegment;
}

TrackSegment* makeTrackSegment(xmlNode* node){
    TrackSegment* t = createTrackSegment();
    for (xmlNode* child = node->children; child!=NULL; child=child->next){
        if(strcmp((char*)child->name, "trkpt")==0){
            Waypoint* w = makeWaypoint(child);
            insertBack(t->waypoints, w);
        }
    }
    return t;
}

Track* createTrack(char* name){
    if (name==NULL){
        return NULL;
    }
    else{
        Track* newTrack = calloc(1, sizeof(Track));
        newTrack->name=stringCopy(name, 0, strlen(name));
        newTrack->segments=initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
        newTrack->otherData=initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        return newTrack;
    }
}

Track* makeTrack(xmlNode* node){
    char* name =getName(node);
    Track* t= createTrack(name);
    for (xmlNode* child = node->children; child!=NULL; child=child->next){
        if (strcmp((char*)child->name, "trkseg")==0){
            TrackSegment* s=makeTrackSegment(child);
            insertBack(t->segments, s);
        }
        else if (isText(child)==false && isName(child)==false){
            GPXData* d=makeGPXData(child);
            insertBack(t->otherData, d);
        }
    }
    return t;
}

bool isName(xmlNode* node){
    char* name = (char*)node->name;
    if (strcmp(name, "name")==0){
        return true;
    }
    return false;
}

bool isText(xmlNode* node){
    char* text = (char*)node->name;
    if (strcmp(text, "text")==0){
        return true;
    }
    return false;
}

