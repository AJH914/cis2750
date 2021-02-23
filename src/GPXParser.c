#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"
#include "GPXParser_A2temp.h"
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
    return gpxdoc;
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){
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
        return;
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