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

GPXdoc* initDoc(){
    GPXdoc* doc = calloc(1, sizeof(GPXdoc));
    doc->creator = calloc(256, sizeof(char));
    doc->version = 1.1;
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    return doc;
}

Waypoint* initWaypoint(){
    Waypoint* wpt = calloc(1, sizeof(Waypoint));
    wpt->name = calloc(256, sizeof(char));
    wpt->latitude = 0.0;
    wpt->longitude = 0.0;
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    return wpt;
}

Route* initRoute(){
    Route* rte = calloc(1, sizeof(Route));
    rte->name = calloc(256, sizeof(char));
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    return rte;    
}

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

//----------------A2 HELPER FUNCTIONS------------------
xmlNode* gpxDataToNode(GPXData* data, xmlNs* namespace){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST data->name);
    xmlSetNs(newNode, namespace);
    xmlNode* node1 = xmlNewText(BAD_CAST data->value);
    xmlAddChild(newNode, node1);
    return newNode;
}

xmlNode* nameToNode(char* name, xmlNs* namespace){
    if (strlen(name)==0){
        return NULL;
    }
    else{
        xmlNode* newNode = xmlNewNode(NULL, BAD_CAST "name");
        xmlSetNs(newNode, namespace);
        xmlNode* node1 = xmlNewText(BAD_CAST name);
        xmlAddChild(newNode, node1);
        return newNode;
    }
}

xmlNode* waypointToNode(Waypoint* wpt, char* name, xmlNs* namespace){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST name);
    xmlSetNs(newNode, namespace);
    xmlNode* nameNode = nameToNode(wpt->name, namespace);
    if (nameNode!=NULL){
        xmlAddChild(newNode, nameNode);
    }
    char array[100] = "\0"; 
    sprintf(array, "%2.6f", wpt->latitude);
    xmlNewProp(newNode, BAD_CAST "lat", BAD_CAST array);
    strcpy(array, "\0");
    sprintf(array, "%2.6f", wpt->longitude);
    xmlNewProp(newNode, BAD_CAST "lon", BAD_CAST array);
    for (Node* head = wpt->otherData->head; head!=NULL; head=head->next){
        GPXData* data = (GPXData*)head->data;
        xmlNode* child = gpxDataToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    return newNode;
}

xmlNode* routeToNode(Route* rte, xmlNs* namespace){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST "rte");
    xmlSetNs(newNode, namespace);
    xmlNode* nameNode = nameToNode(rte->name, namespace);
    if (nameNode!=NULL){
        xmlAddChild(newNode, nameNode);
    }
    for (Node* head = rte->otherData->head; head!=NULL; head=head->next){
        GPXData* data = (GPXData*)head->data;
        xmlNode* child = gpxDataToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    for (Node* head = rte->waypoints->head; head!=NULL; head=head->next){
        Waypoint* data = (Waypoint*)head->data;
        xmlNode* child = waypointToNode(data, "rtept", namespace);
        xmlAddChild(newNode, child);
    }
    return newNode;
}

xmlNode* segmentToNode(TrackSegment* seg, xmlNs* namespace){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST "trkseg");
    xmlSetNs(newNode, namespace);
    for (Node* head = seg->waypoints->head; head!=NULL; head=head->next){
        Waypoint* data = (Waypoint*)head->data;
        xmlNode* child = waypointToNode(data, "trkpt", namespace);
        xmlAddChild(newNode, child);
    }
    return newNode;
}

xmlNode* trackToNode(Track* trk, xmlNs* namespace){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST "trk");
    xmlSetNs(newNode, namespace);
    xmlNode* nameNode = nameToNode(trk->name, namespace);
    if (nameNode!=NULL){
        xmlAddChild(newNode, nameNode);
    }
    for (Node* head = trk->otherData->head; head!=NULL; head=head->next){
        GPXData* data = (GPXData*)head->data;
        xmlNode* child = gpxDataToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    for (Node* head = trk->segments->head; head!=NULL; head=head->next){
        TrackSegment* data = (TrackSegment*)head->data;
        xmlNode* child = segmentToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    return newNode;
}

xmlNode* gpxDocToNode(GPXdoc* doc){
    xmlNode* newNode = xmlNewNode(NULL, BAD_CAST "gpx");
    char ver[50];
    strcpy(ver, "\0");
    sprintf(ver, "%.1f", doc->version);
    xmlNewProp(newNode, BAD_CAST "version", BAD_CAST ver);
    xmlNewProp(newNode, BAD_CAST "creator", BAD_CAST doc->creator);
    xmlNs* namespace = xmlNewNs(newNode, BAD_CAST doc->namespace, NULL);
    xmlSetNs(newNode, namespace);
    for (Node* head = doc->waypoints->head; head!=NULL; head=head->next){
        Waypoint* data = (Waypoint*)head->data;
        xmlNode* child = waypointToNode(data, "wpt", namespace);
        xmlAddChild(newNode, child);
    }
    for (Node* head = doc->routes->head; head!=NULL; head=head->next){
        Route* data = (Route*)head->data;
        xmlNode* child = routeToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    for (Node* head = doc->tracks->head; head!=NULL; head=head->next){
        Track* data = (Track*)head->data;
        xmlNode* child = trackToNode(data, namespace);
        xmlAddChild(newNode, child);
    }
    return newNode;
}

float calcdistance(float lat1, float lat2, float lon1, float lon2){
    float R = 6371000; // metres
    float phi1 = lat1 * M_PI/180; // φ, λ in radians
    float phi2 = lat2 * M_PI/180;
    float deltaphi = (lat2-lat1) * M_PI/180;
    float deltalambda = (lon2-lon1) * M_PI/180;
    float a = sin(deltaphi/2) * sin(deltaphi/2) + cos(phi1) * cos(phi2) * sin(deltalambda/2) * sin(deltalambda/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    float d = R * c; // in metres
    return d;
}

TrackSegment* getSegWithWaypoint(List* segments, int direction){
    if (direction == 1){
        for (Node* head = segments->head; head!=NULL; head=head->next){
            TrackSegment* seg = head->data;
            if (getLength(seg->waypoints)>0){
                return seg;
            }
        }
    }
    else{
        for (Node* tail = segments->tail; tail!=NULL; tail=tail->previous){
            TrackSegment* seg = tail->data;
            if (getLength(seg->waypoints)>0){
                return seg;
            }
        }
    }
    return NULL;
}

bool validateData(GPXData* data){
    if (data==NULL){
        return false;
    }
    if (strlen(data->name)==0 || strlen(data->value)==0){
        return false;
    }
    if (strcmp(data->name, "name")==0){
        return false;
    }
    return true;
}

bool validateWaypoint(Waypoint* wpt){
    if (wpt==NULL){
        return false;
    }
    if (wpt->name == NULL){
        return false;
    }
    if (wpt->otherData==NULL){
        return false;
    }
    if (wpt->latitude<-90 || wpt->latitude>90){
        return false;
    }
    if (wpt->longitude<-180 || wpt->longitude>180){
        return false;
    }
    for (Node* headNode = wpt->otherData->head; headNode!=NULL; headNode=headNode->next){
        GPXData* data = headNode->data;
        if (validateData(data)==false){
            return false;
        }
    }
    return true;
}

bool validateRoute(Route* rte){
    if (rte==NULL){
        return false;
    }
    if (rte->name == NULL){
        return false;
    }
    if (rte->waypoints==NULL){
        return false;
    }
    if (rte->otherData==NULL){
        return false;
    }
    for (Node* headNode = rte->waypoints->head; headNode!=NULL; headNode=headNode->next){
        Waypoint* data = headNode->data;
        if (validateWaypoint(data)==false){
            return false;
        }
    }
    for (Node* headNode = rte->otherData->head; headNode!=NULL; headNode=headNode->next){
        GPXData* data = headNode->data;
        if (validateData(data)==false){
            return false;
        }
    }
    return true;
}

bool validateSegment(TrackSegment* seg){
    if (seg==NULL){
        return false;
    }
    if (seg->waypoints==NULL){
        return false;
    }
    for (Node* headNode = seg->waypoints->head; headNode!=NULL; headNode=headNode->next){
        Waypoint* data = headNode->data;
        if (validateWaypoint(data)==false){
            return false;
        }
    }
    return true;
}

bool validateTrack(Track* trk){
    if (trk==NULL){
        return false;
    }
    if (trk->name == NULL){
        return false;
    }
    if (trk->segments==NULL){
        return false;
    }
    if (trk->otherData==NULL){
        return false;
    }
    for (Node* headNode = trk->segments->head; headNode!=NULL; headNode=headNode->next){
        TrackSegment* data = headNode->data;
        if (validateSegment(data)==false){
            return false;
        }
    }
    for (Node* headNode = trk->otherData->head; headNode!=NULL; headNode=headNode->next){
        GPXData* data = headNode->data;
        if (validateData(data)==false){
            return false;
        }
    }
    return true;
}

bool validateDoc(GPXdoc* doc){
    if (doc==NULL){
        return false;
    }
    if (strlen(doc->namespace)==0){
        return false;
    }
    if (doc->creator==NULL || strlen(doc->creator)==0){
        return false;
    }
    if (doc->waypoints==NULL){
        return false;
    }
    if (doc->routes==NULL){
        return false;
    }
    if (doc->tracks==NULL){
        return false;
    }
    for (Node* headNode = doc->waypoints->head; headNode!=NULL; headNode=headNode->next){
        Waypoint* data = headNode->data;
        if (validateWaypoint(data)==false){
            return false;
        }
    }
    for (Node* headNode = doc->routes->head; headNode!=NULL; headNode=headNode->next){
        Route* data = headNode->data;
        if (validateRoute(data)==false){
            return false;
        }
    }
    for (Node* headNode = doc->tracks->head; headNode!=NULL; headNode=headNode->next){
        Track* data = headNode->data;
        if (validateTrack(data)==false){
            return false;
        }
    }
    return true;
}

void dummyDelete(void* data){
    return;
}

char* gpxDataToJSON(void* gpx){
    GPXData* data = gpx;
    char* json = calloc(1000, sizeof(char));
    sprintf(json, "{\"name\":\"%s\",\"value\":\"%s\"}",data->name, data->value);
    return json;
}

char* listToJSON(List* list, char* (*toJSON)(void*)){
    char* json = calloc(100, sizeof(char));
    if (list == NULL || getLength(list)==0){
        strcpy(json, "[]");
        return json;
    }
    free(json);
    int memsize = 100;
    json = calloc(memsize, sizeof(char));
    strcat(json, "[");
    for (Node* head = list->head; head!=NULL; head=head->next){
        char* temp = toJSON(head->data);
        memsize+=strlen(temp)+10;
        json=realloc(json, sizeof(char)*memsize);
        strcat(json, temp);
        if (head->next != NULL){
            strcat(json, ",");
        }
        free(temp);
    }
    strcat(json, "]");
    return json;
}

char* routeToJSON2(void* data){
    char* json = calloc(100, sizeof(char));
    const Route* rt = (const Route*)data;
    if (rt == NULL){
        strcpy(json, "{}");
        return json;
    }
    char* isLoop = calloc(20, sizeof(char));
    if (isLoopRoute(rt, 10)){
        strcat(isLoop, "true");
    }
    else{
        strcat(isLoop, "false");
    }
    char* name = calloc(256, sizeof(char));
    if (strlen(rt->name)>0){
        strcat(name, rt->name);
    }
    else{
        strcat(name, "None");
    }
    sprintf(json, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, getLength(rt->waypoints), round10(getRouteLen(rt)), isLoop);
    free(isLoop);
    free(name);
    return json;
}

char* trackToJSON2(void* data){
    char* json = calloc(100, sizeof(char));
    const Track* tr = (const Track*)data;
    if (tr == NULL){
        strcpy(json, "{}");
        return json;
    }
    char* isLoop = calloc(20, sizeof(char));
    if (isLoopTrack(tr, 10)){
        strcat(isLoop, "true");
    }
    else{
        strcat(isLoop, "false");
    }
    char* name = calloc(256, sizeof(char));
    if (strlen(tr->name)>0){
        strcat(name, tr->name);
    }
    else{
        strcat(name, "None");
    }
    sprintf(json, "{\"name\":\"%s\",\"len\":%.1f,\"loop\":%s}", name, round10(getTrackLen(tr)), isLoop);
    free(isLoop);
    free(name);
    return json;
}

int getIndex(char* string, char c){
    for (int i=0; i<strlen(string); i++){
        if (string[i] == c){
            return i;
        }
    }
    return -1;
}

// --------------   A3 FUNCTIONS   --------------

char* gpxFileToJSON(char* filename){
    GPXdoc* gpx = createGPXdoc(filename);
    if (gpx == NULL){
        return "{}";
    }
    char* json = calloc(10000, sizeof(char));
    if (validateDoc((GPXdoc*)gpx) == false){
        strcpy(json, "{}");
        return json;
    }
    sprintf(json, "{\"fn\":\"%s\",\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", filename, gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
    return json;
}

bool validateGPXFile(char* filename, char* schemaFile){
    GPXdoc* gpx = createValidGPXdoc(filename, schemaFile);
    if (gpx == NULL){
        return false;
    }
    return validateGPXDoc(gpx, schemaFile);
}

char* routeComponentToJSON(int routeNum, Route* route){
    char* json = calloc(100, sizeof(char));
    Route* rt = route;
    if (rt == NULL){
        strcpy(json, "{}");
        return json;
    }
    char* isLoop = calloc(20, sizeof(char));
    if (isLoopRoute(rt, 10)){
        strcat(isLoop, "true");
    }
    else{
        strcat(isLoop, "false");
    }
    char* name = calloc(256, sizeof(char));
    if (strlen(rt->name)>0){
        strcat(name, rt->name);
    }
    else{
        strcat(name, "None");
    }
    char component[256];
    sprintf(component, "Route %d", routeNum);
    sprintf(json, "{\"component\":\"%s\",\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", component, name, getLength(rt->waypoints), round10(getRouteLen(rt)), isLoop);
    free(isLoop);
    free(name);
    return json;
}

char* trackComponentToJSON(int trackNum, Track* track){
    char* json = calloc(100, sizeof(char));
    Track* tr = track;
    if (tr == NULL){
        strcpy(json, "{}");
        return json;
    }
    char* isLoop = calloc(20, sizeof(char));
    if (isLoopTrack(tr, 10)){
        strcat(isLoop, "true");
    }
    else{
        strcat(isLoop, "false");
    }
    char* name = calloc(256, sizeof(char));
    if (strlen(tr->name)>0){
        strcat(name, tr->name);
    }
    else{
        strcat(name, "None");
    }
    char component[256];
    int numWaypoints = getNumWaypointsTrack(track);
    sprintf(component, "Track %d", trackNum);
    sprintf(json, "{\"component\":\"%s\",\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", component, name, numWaypoints, round10(getTrackLen(tr)), isLoop);
    
    free(isLoop);
    free(name);
    return json;
}

char* gpxComponentsToJSON(char* filename){
    GPXdoc* doc = createGPXdoc(filename);
    int memsize = 1000;
    char* json = calloc(memsize, sizeof(char));
    strcat(json, "[");
    int routeLen = getLength(doc->routes);
    int trackLen = getLength(doc->tracks);
    int count = 1;
    printf("%d %d\n", routeLen, trackLen);
    if (routeLen>0 && trackLen>0){
        for (Node* node = doc->routes->head; node!=NULL; node=node->next){
            Route* route = node->data;
            char* temp = routeComponentToJSON(count, route);
            memsize+=strlen(temp);
            json = realloc(json, memsize*sizeof(char));
            strcat(json, temp);
            strcat(json, ",");
            count++;
            free(temp);
        }
        count = 1;
        for (Node* node = doc->tracks->head; node!=NULL; node=node->next){
            Track* track = node->data;
            char* temp = trackComponentToJSON(count, track);
            memsize+=strlen(temp);
            json = realloc(json, memsize*sizeof(char));
            strcat(json, temp);
            if (node->next!=NULL){
                strcat(json, ",");
            }
            count++;
            free(temp);
        }
    }
    else if (routeLen>0){
        for (Node* node = doc->routes->head; node!=NULL; node=node->next){
            Route* route = node->data;
            char* temp = routeComponentToJSON(count, route);
            memsize+=strlen(temp);
            json = realloc(json, memsize*sizeof(char));
            strcat(json, temp);
            if (node->next!=NULL){
                strcat(json, ",");
            }
            count++;
            free(temp);
        }
    }
    else if (trackLen>0){
        for (Node* node = doc->tracks->head; node!=NULL; node=node->next){
            Track* track = node->data;
            char* temp = trackComponentToJSON(count, track);
            memsize+=strlen(temp);
            json = realloc(json, memsize*sizeof(char));
            strcat(json, temp);
            if (node->next!=NULL){
                strcat(json, ",");
            }
            count++;
            free(temp);
        }
    }
    strcat(json, "]");
    return json;
}

int getNumWaypointsTrack(Track* track){
    int count = 0;
    if (track == NULL){
        return count;
    }
    for (Node* node = track->segments->head; node!=NULL; node=node->next){
        TrackSegment* seg = node->data;
        count+=getLength(seg->waypoints);
    }
    return count;
}

char* otherDataListToJSON(char* filename, char* componentName){
    if (filename == NULL || componentName == NULL){
        return "[]";
    }
    GPXdoc* doc = createGPXdoc(filename);
    if (doc == NULL){
        return "[]";
    }
    Route* route = getRoute(doc, componentName);
    Track* track = getTrack(doc, componentName);
    if (route == NULL && track == NULL){
        return "[]";
    }
    else if (route != NULL){
        char* json = listToJSON(route->otherData, &gpxDataToJSON);
        return json;
    }
    else{
        char* json = listToJSON(track->otherData, &gpxDataToJSON);
        return json;
    }
}

bool changeName(char* filename, char* oldName, char* newName){
    if (filename == NULL || oldName == NULL || newName == NULL){
        return false;
    }
    GPXdoc* doc = createGPXdoc(filename);
    if (doc == NULL){
        return false;
    }
    Route* route = getRoute(doc, oldName);
    if (route != NULL){
        strcpy(route->name, newName);
        return writeGPXdoc(doc, filename);
    }
    Track* track = getTrack(doc, oldName);
    if (track != NULL){
        strcpy(track->name, newName);
        return writeGPXdoc(doc, filename);
    }
    return false;
}

char* routesFromFileToJson(char* filename){
    if (filename == NULL){
        char* bla = calloc(100, sizeof(char));
        strcat(bla, "[]");
        return bla;
    }
    GPXdoc* doc = createGPXdoc(filename);
    if (doc == NULL){
        char* bla = calloc(100, sizeof(char));
        strcat(bla, "[]");
        return bla;
    }
    int unnamedCounter = 1;
    for (Node* node = doc->routes->head; node!=NULL; node=node->next){
        Route* rte = node->data;
        if (strcmp(rte->name, "") == 0){
            sprintf(rte->name, "Unnamed route %d", unnamedCounter);
            unnamedCounter++;
        } 
    }
    return routeListToJSON(doc->routes);
}

char* waypointsFromFileToJson(char* filename, char* routeName){
    if (filename == NULL || routeName == NULL){
        char* bla = calloc(100, sizeof(char));
        strcat(bla, "[]");
        return bla;
    }
    GPXdoc* doc = createGPXdoc(filename);
    if (doc == NULL){
        char* bla = calloc(100, sizeof(char));
        strcat(bla, "[]");
        return bla;
    }
    int unnamedCounter = 1;
    for (Node* node = doc->routes->head; node!=NULL; node=node->next){
        Route* rte = node->data;
        if (strcmp(rte->name, "") == 0){
            sprintf(rte->name, "Unnamed route %d", unnamedCounter);
            unnamedCounter++;
        } 
    }
    Route* route = getRoute(doc, routeName);
    if (route == NULL){
        char* bla = calloc(100, sizeof(char));
        strcat(bla, "[]");
        return bla;
    }
    int memsize = 100;
    char* json = calloc(memsize, sizeof(char));
    strcat(json, "[");
    int index = 0;
    for (Node* node = route->waypoints->head; node!=NULL; node=node->next){
        char* temp = waypointToJson(node->data, index);
        memsize+=strlen(temp)+5;
        json = realloc(json, memsize*sizeof(char));
        strcat(json, temp);
        free(temp);
        if (node->next!=NULL){
            strcat(json, ",");
        }
    }
    strcat(json, "]");
    return json;
}

char* waypointToJson(Waypoint* wpt, int index){
    if (wpt == NULL || index<0){
        return "{}";
    }
    char* json = calloc(1000, sizeof(char));
    char* name = calloc(100, sizeof(char));
    if (strlen(wpt->name) == 0){
        strcat(name, "null");
    }
    else{
        strcat(name, wpt->name);
    }
    sprintf(json, "{\"name\":\"%s\",\"lat\":%.6f,\"lon\":%.6f,\"index\":%d}", name, wpt->latitude, wpt->longitude, index);
    free(name);
    return json;
}
