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
        return NULL;
    }
    GPXdoc* gpxdoc= calloc(1, sizeof(GPXdoc));
    gpxdoc->creator = calloc(256, sizeof(char));
    gpxdoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxdoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    gpxdoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxdoc->version = 1.1;
    strcpy(gpxdoc->creator, "Denis Nikitenko 2021.01.13");

    root_element = xmlDocGetRootElement(doc);
    
    

    for(xmlAttr* a = root_element->properties; a!=NULL; a=a->next){
        xmlNode *value = a->children;
        char *attrName = (char *)a->name;
        char *cont = (char *)(value->content);
        if(strcmp(attrName, "version")==0){
            gpxdoc->version=atof(cont);
        }
        if(strcmp(attrName, "creator")==0){
            strcpy(gpxdoc->creator, cont);
        }
    }

    char* ns = (char*)root_element->ns->href;
    if (ns==NULL){
        strcpy(gpxdoc->namespace, "No ns");
    }
    else if(strlen(ns)==0){
        strcpy(gpxdoc->namespace, "No ns");
    }
    else{
        strcpy(gpxdoc->namespace, ns);
    }

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
    xmlFreeDoc(doc);
    xmlCleanupParser();
    ROUTE_NUM=1;
    return gpxdoc;
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){
    if (fileName == NULL || gpxSchemaFile == NULL){
        return NULL;
    }
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    char *XMLFileName = fileName;
    char *XSDFileName = gpxSchemaFile;
    GPXdoc* newdoc = NULL;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(XSDFileName);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    //xmlSchemaDump(stdout, schema); //To print schema dump

    doc = xmlReadFile(XMLFileName, NULL, 0);

    if (doc == NULL){
        return NULL;
    }
    else{
        xmlSchemaValidCtxtPtr ctxt;
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        if (ret == 0){
            newdoc = createGPXdoc(fileName);
        }
        else if (ret > 0){
            printf("%s fails to validate\n", XMLFileName);
        }
        else{
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
    }

    // free the resource
    if(schema != NULL)
    xmlSchemaFree(schema);

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
    return newdoc;
}

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc){
    if (doc==NULL){
        return NULL;
    }
    GPXdoc* temp = (GPXdoc*)doc;
    char* wpts = toString(temp->waypoints);
    char* rts = toString(temp->routes);
    char* trks = toString(temp->tracks);
    char* output = calloc(1000+strlen(wpts)+strlen(rts)+strlen(trks), sizeof(char));
    sprintf(output, "\nGPXdoc: Namespace: %s, Version: %.2f, Creator: %s, Waypoints: %s, Routes: %s, Tracks: %s \n", temp->namespace, temp->version, temp->creator, wpts, rts, trks);
    free(wpts);
    free(rts);
    free(trks);
    return output;
}

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
int getNumWaypoints(const GPXdoc* doc){
    if (doc == NULL){
        return 0;
    }
    else{
        return getLength(doc->waypoints);
    }
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc){
    if (doc == NULL){
        return 0;
    }
    else{
        return getLength(doc->routes);
    }
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc){
    if (doc == NULL){
        return 0;
    }
    else{
        return getLength(doc->tracks);
    }
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc){
    if (doc == NULL){
        return 0;
    }
    else{
        int sum=0;
        for (Node* head = doc->tracks->head; head!=NULL; head=head->next){
            sum+=getLength(((Track*)head->data)->segments);
        }
        return sum;
    }
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc){
    if (doc == NULL){
        return 0;
    }
    else{
        int sum=0;
        for (Node* head = doc->waypoints->head; head!=NULL; head=head->next){
            Waypoint* wpt = (Waypoint*)head->data;
            sum+=getLength(wpt->otherData);
            if (strcmp(wpt->name, "")!=0){
                sum++;
            }
        }
        for (Node* head = doc->routes->head; head!=NULL; head=head->next){
            sum+=getLength(((Route*)head->data)->otherData);
            Route* route = head->data;
            if (strcmp(route->name, "")!=0){
                sum++;
            }
            for (Node* head2 = route->waypoints->head; head2!=NULL; head2=head2->next){
                sum+=getLength(((Waypoint*)head2->data)->otherData);
                Waypoint* wpt = (Waypoint*)head2->data;
                if (strcmp(wpt->name, "")!=0){
                    sum++;
                }
            }
        }
        for (Node* head = doc->tracks->head; head!=NULL; head=head->next){
            Track* track = head->data;
            sum += getLength(track->otherData);
            if (strcmp(track->name, "")!=0){
                sum++;
            }
            for (Node* head2 = track->segments->head; head2!=NULL; head2=head2->next){
                TrackSegment* segment = head2->data;
                for (Node*head3 = segment->waypoints->head; head3!=NULL; head3=head3->next){
                    Waypoint* wpt = (Waypoint*)head3->data;
                    sum+=getLength(wpt->otherData);
                    if (strcmp(wpt->name, "")!=0){
                        sum++;
                    }
                }
            }
        }
        return sum;
    }
}

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name){
    if (doc==NULL || name==NULL){
        return NULL;
    }
    else{
        List*waypoints=doc->waypoints;
        for (Node* head = waypoints->head; head!=NULL; head=head->next){
            Waypoint* temp = head->data;
            if (strcmp(temp->name, name)==0){
                return temp;
            }
        }
        List*routes=doc->routes;
        for (Node* head = routes->head; head!=NULL; head=head->next){
            Route* temp = head->data;
            waypoints=temp->waypoints;
            for (Node* head2 = waypoints->head; head2!=NULL; head2=head2->next){
                Waypoint* temp2 = head2->data;
                if (strcmp(temp2->name, name)==0){
                    return temp2;
                }
            }
        }
        List*tracks=doc->tracks;
        for (Node*head = tracks->head; head!=NULL; head=head->next){
            Track* temp = head->data;
            List*segments=temp->segments;
            for (Node* head2 = segments->head; head2!=NULL; head2=head2->next){
                TrackSegment*temp2 = head2->data;
                waypoints=temp2->waypoints;
                for (Node* head3 = waypoints->head; head3!=NULL; head3=head3->next){
                    Waypoint* temp3 = head3->data;
                    if (strcmp(temp3->name, name)==0){
                        return temp3;
                    }
                }
            }
        }
    }
    return NULL;
}
// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name){
    if (doc==NULL || name==NULL){
        return NULL;
    }
    else{
        List* tracks = doc->tracks;
        for (Node* head = tracks->head; head!=NULL; head=head->next){
            Track* temp = head->data;
            if (strcmp(temp->name, name)==0){
                return temp;
            }
        }
    }
    return NULL;
}
// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name){
    if (doc==NULL || name==NULL){
        return NULL;
    }
    else{
        List* routes = doc->routes;
        for (Node* head = routes->head; head!=NULL; head=head->next){
            Route* temp = head->data;
            if (strcmp(temp->name, name)==0){
                return temp;
            }
        }
    }
    return NULL;
}

//---------HELPER FUNCTIONS---------

void deleteGpxData( void* data){
    if (data==NULL){
        return;
    }
    GPXData* temp = (GPXData*)data;
    free(temp);
}
char* gpxDataToString( void* data){
    if (data==NULL){
        return NULL;
    }
    GPXData* temp = (GPXData*)data;
    char* output = calloc(strlen(temp->name)+strlen(temp->value)+100, sizeof(char));
    sprintf(output, "\ngpxData: %s %s \n", temp->name, temp->value);
    return output;
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
    if (data==NULL){
        return NULL;
    }
    Waypoint* temp = (Waypoint*)data;
    char* otherstring = toString(temp->otherData);
    char* output = calloc(1000+strlen(otherstring), sizeof(char));
    sprintf(output, "\nWaypoint: name: %s longitude: %.2f latitude: %.2f otherData: %s \n", temp->name, temp->longitude, temp->latitude, otherstring);
    free(otherstring);
    return output;
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
    if (data==NULL){
        return NULL;
    }
    Route* temp= (Route*)data;
    char* wpts = toString(temp->waypoints);
    char* otherstring = toString(temp->otherData);
    char* output = calloc(1000+strlen(otherstring)+strlen(wpts), sizeof(char));
    sprintf(output, "\nRoute: Name: %s, Waypoints: %s, otherData: %s\n", temp->name, wpts, otherstring);
    free(wpts);
    free(otherstring);
    return output;
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
    if (data==NULL){
        return NULL;
    }
    TrackSegment* temp = (TrackSegment*)data;
    char* wpts = toString(temp->waypoints);
    char* output = calloc(1000+strlen(wpts), sizeof(char));
    sprintf(output, "\nTrack Segment: Waypoints: %s\n", wpts);
    free(wpts);
    return output;
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
    if (data==NULL){
        return NULL;
    }
    Track* temp= (Track*)data;
    char* sgmts = toString(temp->segments);
    char* otherstring = toString(temp->otherData);
    char* output = calloc(1000+strlen(otherstring)+strlen(sgmts), sizeof(char));
    sprintf(output, "\nTrack: Name: %s, Segments: %s, otherData: %s\n", temp->name, sgmts, otherstring);
    free(sgmts);
    free(otherstring);
    return output;
}
int compareTracks(const void *first, const void *second){
    return -1;
}

//-----------VALIDATE FUNCTIONS------------

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile){
    if (doc == NULL || gpxSchemaFile == NULL){
        return false;
    }
    bool status = validateDoc(doc);
    if (status==false){
        return status;
    }
    xmlDoc* xmldoc = NULL;
    xmlNode* root = NULL;
    xmldoc = xmlNewDoc(BAD_CAST "1.0");
    root = gpxDocToNode(doc);
    xmlDocSetRootElement(xmldoc, root);
    xmlSchemaParserCtxtPtr ctxt;
    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    xmlSchemaPtr schema = NULL;
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    xmlSchemaValidCtxtPtr ctxt2;
    int ret;
    ctxt2 = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt2, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxt2, xmldoc);
    if (ret == 0){
        status = true;
    }
    xmlSchemaFreeValidCtxt(ctxt2);
    xmlFreeDoc(xmldoc);
    if(schema != NULL)
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
    return status;
}

bool writeGPXdoc(GPXdoc* doc, char* fileName){
    if (doc == NULL || fileName == NULL){
        return false;
    }
    xmlDoc* xmldoc = NULL;
    xmlNode* root = NULL;
    root = gpxDocToNode(doc);
    xmldoc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(xmldoc, root);
    xmlSaveFormatFileEnc(fileName, xmldoc, "ISO-8859-1", 1);
    /*free the document */
    xmlFreeDoc(xmldoc);
    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return true;
}

//------------ A2 MODULE 2 FUNCTIONS -----------------

float round10(float len){
    int x = len;
    int i = 0;
    while (i<x){
        i+=10;
    }
    if (i-x > 5){
        return i-10;
    }
    else{
        return i;
    }
}

float getRouteLen(const Route *rt){
    if (rt == NULL){
        return 0;
    }
    Route* route = (Route*)rt;
    float dist = 0;
    if (route->waypoints->head!=NULL){
        for (Node* head = route->waypoints->head; head->next!=NULL; head=head->next){
            Waypoint* w1 = head->data;
            Waypoint* w2 = head->next->data;
            dist += calcdistance(w1->latitude, w2->latitude, w1->longitude, w2->longitude);
        }
    }
    return dist;
}

float getTrackLen(const Track *tr){
    if (tr == NULL){
        return 0;
    }
    Track* track = (Track*)tr;
    float dist = 0;
    List* waypoints = initializeList(&waypointToString, &dummyDelete, &compareWaypoints);
    for (Node* head = track->segments->head; head!=NULL; head=head->next){
        TrackSegment* seg = head->data;
        for (Node* head2 = seg->waypoints->head; head2!=NULL; head2=head2->next){
            insertBack(waypoints, head2->data);
        }
    }
    if (waypoints->head!=NULL){
        for (Node* head = waypoints->head; head->next!=NULL; head=head->next){
            Waypoint* w1 = head->data;
            Waypoint* w2 = head->next->data;
            dist += calcdistance(w1->latitude, w2->latitude, w1->longitude, w2->longitude);
        }
    }
    freeList(waypoints);
    return dist;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
    if (doc == NULL || len<0 || delta<0){
        return 0;
    }
    GPXdoc* gpxdoc = (GPXdoc*)doc;
    int counter=0;
    for (Node* head = gpxdoc->routes->head; head!=NULL; head=head->next){
        float dist = getRouteLen(head->data);
        if (fabs(dist-len)<=delta){
            counter++;
        }
    }
    return counter;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta){
    if (doc == NULL || len<0 || delta<0){
        return 0;
    }
    GPXdoc* gpxdoc = (GPXdoc*)doc;
    int counter=0;
    for (Node* head = gpxdoc->tracks->head; head!=NULL; head=head->next){
        float dist = getTrackLen(head->data);
        if (fabs(dist-len)<=delta){
            counter++;
        }
    }
    return counter;
}

bool isLoopRoute(const Route* route, float delta){
    if (route == NULL || delta<0){
        return false;
    }
    Route* rt = (Route*)route;
    if (getLength(rt->waypoints)<4){
        return false;
    }
    Waypoint* w1 = rt->waypoints->head->data;
    Waypoint* w2 = rt->waypoints->tail->data;
    float dist = calcdistance(w1->latitude, w2->latitude, w1->longitude, w2->longitude);
    if (dist<delta){
        return true;
    }
    return false;
}

bool isLoopTrack(const Track *tr, float delta){
    if (tr == NULL || delta<0){
        return false;
    }
    Track* track = (Track*)tr;
    if (getLength(track->segments)==0){
        return false;
    }
    int numWaypoints = 0;
    for (Node* head = track->segments->head; head!=NULL; head=head->next){
        TrackSegment* seg = head->data;
        numWaypoints += getLength(seg->waypoints);
    }
    if (numWaypoints<4){
        return false;
    }
    else{
        TrackSegment* seg1 = getSegWithWaypoint(track->segments, 1);
        TrackSegment* seg2 = getSegWithWaypoint(track->segments, 0);
        Waypoint* w1 = seg1->waypoints->head->data;
        Waypoint* w2 = seg2->waypoints->tail->data;
        float dist = calcdistance(w1->latitude, w2->latitude, w1->longitude, w2->longitude);
        if (dist<delta){
            return true;
        }
    }
    return false;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if (doc==NULL){
        return NULL;
    }
    List* list = initializeList(&routeToString, &dummyDelete, &compareRoutes);
    for (Node* head = doc->routes->head; head!=NULL; head=head->next){
        Route* rte = head->data;
        Waypoint* wpt1 = rte->waypoints->head->data;
        Waypoint* wpt2 = rte->waypoints->tail->data;
        if (calcdistance(sourceLat, wpt1->latitude, sourceLong, wpt1->longitude)<=delta && calcdistance(destLat, wpt2->latitude, destLong, wpt2->longitude)<=delta){
            insertBack(list, rte);
        }
    }
    if (getLength(list)>0){
        return list;
    }
    else{
        freeList(list);
        return NULL;
    }
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if (doc==NULL){
        return NULL;
    }
    List* list = initializeList(&trackToString, &dummyDelete, &compareTracks);
    for (Node* head = doc->tracks->head; head!=NULL; head=head->next){
        Track* trk = head->data;
        TrackSegment* seg1 = getSegWithWaypoint(trk->segments, 1);
        TrackSegment* seg2 = getSegWithWaypoint(trk->segments, 0);
        Waypoint* wpt1 = seg1->waypoints->head->data;
        Waypoint* wpt2 = seg2->waypoints->tail->data;
        if (calcdistance(sourceLat, wpt1->latitude, sourceLong, wpt1->longitude)<=delta && calcdistance(destLat, wpt2->latitude, destLong, wpt2->longitude)<=delta){
            insertBack(list, trk);
        }
    }
    if (getLength(list)>0){
        return list;
    }
    else{
        freeList(list);
        return NULL;
    }
}

//------------ A2 MODULE 3 FUNCTIONS -----------------

char* routeToJSON(const Route *rt){
    return routeToJSON2((Route*)rt);
}

char* trackToJSON(const Track *tr){
    return trackToJSON2((Track*)tr);
}

char* routeListToJSON(const List *list){
    return listToJSON((List*)list, &routeToJSON2);
}

char* trackListToJSON(const List *list){
    return listToJSON((List*)list, &trackToJSON2);
}

char* GPXtoJSON(const GPXdoc* gpx){
    char* json = calloc(10000, sizeof(char));
    if (gpx == NULL){
        strcpy(json, "{}");
        return json;
    }
    if (validateDoc((GPXdoc*)gpx) == false){
        strcpy(json, "{}");
        return json;
    }
    sprintf(json, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
    return json;
}


//----------------A2 BONUS FUNCTIONS---------------
void addWaypoint(Route *rt, Waypoint *pt){
    if (rt == NULL || pt == NULL){
        return;
    }
    insertBack(rt->waypoints, pt);
    return;
}

void addRoute(GPXdoc* doc, Route* rt){
    if (rt == NULL || doc == NULL){
        return;
    }
    insertBack(doc->routes, rt);
    return;
}

GPXdoc* JSONtoGPX(const char* gpxString){
    if (gpxString == NULL){
        return NULL;
    }
    char* json = (char*)gpxString;
    int commaindex = getIndex(json, ',');
    char* firstItem = stringCopy(json, 2, commaindex);
    char* secondItem = stringCopy(json, commaindex + 1, strlen(json)-2);
    int colonindex = getIndex(firstItem, ':');
    char* version = stringCopy(firstItem, colonindex+1, strlen(firstItem));
    colonindex = getIndex(secondItem, ':');
    char* creator = stringCopy(secondItem, colonindex+2, strlen(secondItem));
    free(firstItem);
    free(secondItem);
    GPXdoc* newdoc = initDoc();
    newdoc->version = atof(version);
    strcpy(newdoc->creator, creator);
    free(version);
    free(creator);
    return newdoc;
}

Waypoint* JSONtoWaypoint(const char* gpxString){
    if (gpxString == NULL){
        return NULL;
    }
    char* json = (char*)gpxString;
    int commaindex = getIndex(json, ',');
    char* firstItem = stringCopy(json, 2, commaindex);
    char* secondItem = stringCopy(json, commaindex + 1, strlen(json)-1);
    int colonindex = getIndex(firstItem, ':');
    char* latVal = stringCopy(firstItem, colonindex + 1, strlen(firstItem));
    colonindex = getIndex(secondItem, ':');
    char* lonVal = stringCopy(secondItem, colonindex + 1, strlen(secondItem));
    free(firstItem);
    free(secondItem);
    Waypoint* wpt = initWaypoint();
    wpt->latitude = atof(latVal);
    wpt->longitude = atof(lonVal);
    free(latVal);
    free(lonVal);
    return wpt;
}

Route* JSONtoRoute(const char* gpxString){
    if (gpxString == NULL){
        return NULL;
    }
    char* json = (char*)gpxString;
    int colonindex = getIndex(json, ':');
    char* nameVal = stringCopy(json, colonindex + 2, strlen(json)-2);
    Route* rte = initRoute();
    strcpy(rte->name, nameVal);
    free(nameVal);
    return rte;
}