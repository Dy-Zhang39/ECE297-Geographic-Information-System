/* 
 * Copyright 2021 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "global.h"     //include all the global variable
#include <algorithm>
#include <math.h>
#include <map>
#include "OSMDatabaseAPI.h"
#include "dataHandler.h"
#include <bits/stdc++.h>
#include <stdio.h>
#include <ctype.h>
#include <boost/algorithm/string.hpp>





// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.


std::vector<std::string> cityNames = {
    "Beijing, China", "Cairo, Egypt", "Cape Town, South Africa", "Golden Horseshoe, Canada",
    "Hamilton, Canada", "Hong Kong, China", "Iceland", "Interlaken, Switzerland",
    "London, England", "Moscow, Russia", "New Delhi, India", "New York, USA",
    "Rio De Janeiro, Brazil", "Saint Helena", "Singapore", "Sydney, Australia",
    "Tehran, Iran", "Tokyo, Japan", "Toronto, Canada"
};

//all the global variables in each city
std::vector<City*> cities;
//the index of current map that is being drawn
int currentCityIdx = 0;
std::string mapPathPrefix = "/cad2/ece297s/public/maps/";


// Number of different characters possible.
int CHAR_SIZE = 256;

//for searches with partial name: if partial name is longer than this number, then use additional 3-character index.
int PREFIX_NUM_CHAR = 2;

// Global variables to create index for the third character if exist. Due to the memory limit, anything with the third character
// 1. before SEPARATE_CHAR will be in one index
// 2. between SPEARATE_CHAR and SEPARATE_CHAR_AFTER including SEPARATE_CHAR itself, it will be in another index.
// 3. after and include SEPARATE_CHAR_AFTER will be in the third index
char SEPARATE_CHAR = 'k';
char SEPARATE_CHAR_AFTER = 's';

std::vector<StreetIdx> mostSimilarFirstName;
std::vector<StreetIdx> mostSimilarSecondName;
bool streetDataBaseIsLoaded = false;
bool OSMIsLoaded = false;
bool checkingFirstName = true;
//helper function


bool loadMap(std::string map_streets_database_filename) {
    bool alreadyExist = false;
    std::clock_t start = clock();

    //check if the map is already loaded
    for (int cityIdx = 0; cityIdx < cities.size() && !alreadyExist; cityIdx++){

        if (map_streets_database_filename == cities[cityIdx] -> mapPath){
            alreadyExist = true;
            currentCityIdx = cityIdx;
        }      
    }
    bool load_successful;
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded successfully

    if(!load_successful){
        return load_successful;
    }
    streetDataBaseIsLoaded = true;
    
    //std::clock_t loadMapEnd = clock();
        
    //std::clock_t osmMapEnd = clock();
    
    //create new global variable for new city
    if(!alreadyExist){
        
        //change to osm file name
        std::string osm_filename = map_streets_database_filename.substr(0, map_streets_database_filename.length() - 12);
    
        load_successful = loadOSMDatabaseBIN(osm_filename + ".osm.bin");
        
        if(!load_successful)
            return load_successful;
        OSMIsLoaded = true;
        
        City* newCity = new City;
        newCity->mapPath = map_streets_database_filename;
        newCity->street = new Street;
        newCity ->streetSegment = new StreetSegment;
        newCity ->intersection = new Intersection;

        cities.push_back(newCity);
        currentCityIdx = cities.size() - 1;     //reset the index to the newest city

    }else{
        std::clock_t end = clock();
        double loadAlready = double(end - start) / CLOCKS_PER_SEC;
        std::cout << "The loading process takes " << loadAlready << "s"<< std::endl;
        return true;
    }


    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    
    resizeData();               //resize the global vectors
    streetPartialName();        //pre-load partial name index
    street_Intersection();      //pre-load street intersections
    street_Info();              //pre-load information about street length; street travel time; street segment length
    initializeFeatureBounding();//pre-load the bounding boxes of features
    loadSubways();
    std::clock_t end = clock();
    double loadAlready = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "The loading process takes " << loadAlready << "s"<< std::endl;
    return load_successful;
    
    
}

//calculate x coordinate from longitude
double xFromLon(double lon){
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(cities[currentCityIdx] -> avgLat * kDegreeToRadian);
}

//calculate y coordinate from latitude
double yFromLat(double lat){
    return lat * kDegreeToRadian* kEarthRadiusInMeters;
}

//calculate longitude from x coordinate
double lonFromX(double x){
    return x/(kDegreeToRadian * kEarthRadiusInMeters * std::cos(cities[currentCityIdx] -> avgLat * kDegreeToRadian));
}

//calculate latitude from y coordinate
double latFromY(double y){
    return y/(kDegreeToRadian* kEarthRadiusInMeters);
}

void resizeData(){
    // initialize vector<double> streetSegLength;
    cities[currentCityIdx]->street->streetLength.resize(getNumStreets());
    cities[currentCityIdx]->street->streetSegments.resize(getNumStreets());
    // Load index vectors used to quick search street names
    // Load street name into the vector using first 1 characters as index for cities[currentCityIdx]->street->streetNamesOneChar,
    // Load street name into the vector using first 2 characters as index for cities[currentCityIdx]->street->streetNamedTwoChar,
    // Load street name into the vector using first 3 characters as index for cities[currentCityIdx]->street->streetNamesThreeChar.
    cities[currentCityIdx]->street->streetNames.resize(getNumStreets());
    cities[currentCityIdx]->street->streetNamesOneChar.resize(CHAR_SIZE);
    cities[currentCityIdx]->street->streetNamesTwoChar.resize(CHAR_SIZE * CHAR_SIZE);
    cities[currentCityIdx]->street->streetNamesThreeChar.resize(CHAR_SIZE * CHAR_SIZE * 3);
    
    // Find streets length and their corresponding travel time
    cities[currentCityIdx]->streetSegment->streetSegLength.resize(getNumStreetSegments());
    cities[currentCityIdx]->streetSegment->streetSegTravelTime.resize(getNumStreetSegments());
    cities[currentCityIdx]->streetSegment->streetSegPoint.resize(getNumStreetSegments());
    
}

// initialize the bounding coordinates of all features into vectors
void initializeFeatureBounding() {
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double minX = xFromLon(getFeaturePoint(featureID, 0).longitude());
        double maxX = minX;
        double minY = yFromLat(getFeaturePoint(featureID, 0).latitude());
        double maxY = minY;
        
        // find bounding box of feature
        for (int pt = 1; pt < getNumFeaturePoints(featureID); pt++){
            double x, y;
            x = xFromLon(getFeaturePoint(featureID, pt).longitude());
            y = yFromLat(getFeaturePoint(featureID, pt).latitude());

            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
        Feature keyPoints(maxY, minY, minX, maxX, findFeatureArea(featureID));
        // record the bounding boxes for each feature
        cities[currentCityIdx]->featurePts.push_back(keyPoints);
    }
}

void loadSubways() {

    std::vector<const OSMRelation *> osm_subway_lines;
    
    //step length for looping and searching with OSM nodes
    int stepLength = 1000;
    
    std::vector<OSMID> osm_nodes;
    
    std::vector<ezgl::point2d> stationCoordindate;
    std::vector<std::string> stationName;
    std::string lineColor;                  //color of the subway line
    
    // Create index for the nodes by grouping them
    for (unsigned k = 0; k < getNumberOfNodes(); k++) {
        const OSMNode  *newNode = getNodeByIndex(k);
        if (k % stepLength == 0) {
            osm_nodes.push_back(newNode->id());
        }
    }

    // Loop through all OSM relations
    for (unsigned i = 0; i < getNumberOfRelations(); i++) {
        const OSMRelation *currRel = getRelationByIndex(i);

        // Check the tag of the currRel
        for (unsigned j = 0; j < getTagCount(currRel); j++) {
            std::pair<std::string, std::string> tagPair = getTagPair(currRel, j);

            // Push relations with the route=subway tag
            if (tagPair.first == "route" && tagPair.second == "subway") {
                osm_subway_lines.push_back(currRel);
                break;
            }
        }
    }

    
    // For each subway line (relation), get its name, color, and members
    for (unsigned i = 0; i < osm_subway_lines.size(); i++) {

        // Get subway line color and name
        for (unsigned j = 0; j < getTagCount(osm_subway_lines[i]); j++) {
            std::pair<std::string, std::string> tagPair = getTagPair(osm_subway_lines[i], j);
            
            if (tagPair.first == "colour") {
                lineColor = tagPair.second;
            }
        }

        // Get relation members
        std::vector<TypedOSMID> route_members = getRelationMembers(osm_subway_lines[i]); 
        
        // Grab subway names
      
        for(unsigned j = 0; j < route_members.size(); j++) {

            // A member of type node represents a subway station
            if(route_members[j].type() == TypedOSMID::Node) {

                const OSMNode *currNode = nullptr;
                unsigned indexRange = 0;
                
                //find the osm_nodes (section including [stepLength] of nodes) that contains the OSM Node for the station
                for (unsigned k = 0; k < osm_nodes.size(); k++) {
                    
                    if (route_members[j] < osm_nodes[k]) {
                        indexRange = k;
                        break;
                    }
                }
                unsigned startIdx = 0, endIdx = getNumberOfNodes();      
                
                //loop through all OSM Nodes in the found osm_nodes section
                if ((indexRange - 1) * stepLength > 0) 
                    startIdx = (indexRange - 1) * stepLength;
                
                if ((indexRange + 1) * stepLength < endIdx) 
                    endIdx = (indexRange + 1) * stepLength ;

                // Node lookup by OSMID
                for (unsigned k = startIdx; k < endIdx; k++) {
                    currNode = getNodeByIndex(k);
                    
                    if (currNode->id() == route_members[j]) {
                        break;
                    }
                }

                // Get the name tag of that node
                for (unsigned k = 0; k < getTagCount(currNode); k++) {
                    std::pair<std::string, std::string> tagPair = getTagPair(currNode, k);
                    LatLon stationCoord = getNodeCoords(currNode);
                    
                    if (tagPair.first == "name") {
                        Subway s;
                        double x = xFromLon(stationCoord.longitude());
                        double y = yFromLat(stationCoord.latitude());
                        
                        s.location = {x, y};
                        s.red = 12;
                        s.green = 124;
                        s.blue = 92;

                        if (lineColor.compare("green") == 0) {

                            s.red = 57;
                            s.green = 180;
                            s.blue = 96;

                        } else if (lineColor.compare("purple") == 0) {

                            s.red = 118;
                            s.green = 23;
                            s.blue = 132;
                            
                        } else if (lineColor.compare("yellow") == 0) {

                            s.red = 199;
                            s.green = 182;
                            s.blue = 39;
                            
                        } else if (lineColor.compare("brown") == 0) {

                            s.red = 128;
                            s.green = 0;
                            s.blue = 0;
                            
                        } else if ( lineColor.length() == 7 && lineColor.at(0) == '#') {
                            //convert given HEX color into RGB
                            s.red = std::stoi(lineColor.substr(1,2), nullptr, 16);
                            s.green = std::stoi(lineColor.substr(3,2), nullptr, 16);
  
                        }

                        s.name = tagPair.second;

                        cities[currentCityIdx]->subways.push_back(s);
                        break;
                    }
                }
            }
        }
    }
}

void streetPartialName(){
    
    for (int i = 0; i < getNumStreets(); i++){
        //initialize all the element in the street length to 0 to prevent undefined variable
        cities[currentCityIdx]->street->streetLength[i] = 0;
        
        // Get street name, remove space and convert to lower cases. Got only the first 2 letters.
        std::string streetName = getStreetName(i);
        std::string streetNameSub = "";
        std::string streetFullName;
        
        for (int j = 0; j < streetName.length(); j++) {
            
            if (streetName[j] != ' ' && streetNameSub.length() <= PREFIX_NUM_CHAR) {
                streetNameSub.push_back(tolower(streetName[j]));
            }
            
            if (streetName[j] != ' ') {
                cities[currentCityIdx]->street->streetNames[i].push_back(tolower(streetName[j]));
            }
        }

        // Store the street id into the index vector: streetNamesOneChar, streetNamesTwoChar
        if (streetNameSub.length() > 0)
            cities[currentCityIdx]->street->streetNamesOneChar[tolower(streetNameSub[0])].push_back(i);
        
        if (streetNameSub.length() > 1)
            cities[currentCityIdx]->street->streetNamesTwoChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);

        if (streetNameSub.length() > PREFIX_NUM_CHAR) {
            if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                cities[currentCityIdx]->street->streetNamesThreeChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);
            } else if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE].push_back(i);
            } else {
                cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE].push_back(i);
            }
        }
    }
    
    
}
void street_Intersection(){
    cities[currentCityIdx]->intersection->intersectionStreetSegments.resize(getNumIntersections()); //create empty vector for each intersection
    cities[currentCityIdx]->street->streetIntersections.resize(getNumStreets());
    cities[currentCityIdx] -> intersection -> intersectionInfo.resize(getNumIntersections());
    
    double maxLat = getIntersectionPosition(0).latitude();
    double minLat = maxLat;
    double maxLon = getIntersectionPosition(0).longitude();
    double minLon = maxLon;
    
    //iterate through all intersections
    for(int intersectionID = 0; intersectionID < getNumIntersections(); ++intersectionID){
        
        cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position = getIntersectionPosition(intersectionID);
        cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].name = getIntersectionName(intersectionID);

        maxLat = std::max(maxLat, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.latitude());
        minLat = std::min(minLat, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position. latitude());
        maxLon = std::max(maxLon, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.longitude());
        minLon = std::min(minLon, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.longitude());
        
        for(int i = 0; i < getNumIntersectionStreetSegment(intersectionID); ++i) {         
        
            //iterate through all segments at intersection
            int streetSegID = getIntersectionStreetSegment(intersectionID, i);
            auto streetSegInfo = getStreetSegmentInfo(streetSegID);
            auto streetID = streetSegInfo.streetID;

            if (cities[currentCityIdx]->street->streetIntersections[streetID].size() == 0 || cities[currentCityIdx]->street->streetIntersections[streetID].back() < intersectionID) {
                cities[currentCityIdx]->street->streetIntersections[streetID].push_back(intersectionID);              //save the intersection to the street it belongs to
            }
            
            cities[currentCityIdx]->intersection->intersectionStreetSegments[intersectionID].push_back(streetSegID);  //save segments connected to intersection
        }
    }
    
    cities[currentCityIdx] -> maxLat = maxLat;
    cities[currentCityIdx] -> minLat = minLat;
    cities[currentCityIdx] -> maxLon = maxLon;
    cities[currentCityIdx] -> minLon = minLon;
    cities[currentCityIdx] -> avgLat = (maxLat + minLat)/2;
            
}

//street length; street travel time; street segment length
void street_Info(){
    
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        double streetSegmentLength = 0;
        struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);
        int twoTerminals = 2;
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id].resize(twoTerminals + streetSegmentID.numCurvePoints);
        //find the starting and ending position of given street segment
        auto from = getIntersectionPosition(streetSegmentID.from);
        auto to = getIntersectionPosition(streetSegmentID.to);
        
        //put the street segment into the street that it belong to
        cities[currentCityIdx]->street->streetSegments[streetSegmentID.streetID].push_back(street_segment_id);  
        
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][0] = from;
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][twoTerminals + streetSegmentID.numCurvePoints - 1] = to;
        
        //find curve points on the given street segment (more than one curve point)
        if (streetSegmentID.numCurvePoints > 1) {

            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            //calculate distance between start point and first curve point
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints);
            
            cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][1] = getStreetSegmentCurvePoint(street_segment_id, 0);
            int i = 1;
            for (; i < streetSegmentID.numCurvePoints; i++) {
                std::pair <LatLon, LatLon> curvePoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), getStreetSegmentCurvePoint(street_segment_id, i));
                
                cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][i+1] = getStreetSegmentCurvePoint(street_segment_id, i);
                //add the distance between each curve points
                streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(curvePoints);
            }
            
            //check if the cities[currentCityIdx]->streetSegment vector is properly filled
            if(cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][i+1].latitude()!= to.latitude()){
                std::cout<<"STREEET_SEGMENTS not properly filled"<<std::endl;
                exit (-1);
            }
             
            //add the distance between last curve point and end point
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), to);
            streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(lastPoints);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;

        } else if (streetSegmentID.numCurvePoints == 1) { //only has one curve point
            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, 0), to);
            
            cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][1] = getStreetSegmentCurvePoint(street_segment_id, 0);
            
            // add distance from start point to the only curve point to the end point 
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints) + findDistanceBetweenTwoPoints(lastPoints);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;

        } else {
            //straight segment
            std::pair <LatLon, LatLon> points(from, to);
            //calculate distance between start and end point
            streetSegmentLength = findDistanceBetweenTwoPoints(points);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;
        }
        
        
        //find street segment travel time
        double speedLimit = streetSegmentID.speedLimit;
        cities[currentCityIdx]->streetSegment->streetSegTravelTime[street_segment_id] = streetSegmentLength / speedLimit;
        
        //find street length
        cities[currentCityIdx]->street->streetLength[streetSegmentID.streetID] += streetSegmentLength;
    }
}
    
    
//clear all the global data structure to prevent it get bigger and bigger    
void closeMap() {

    //Clean-up your map related data structures here
    closeDataBase();
    for(int cityIdx = 0; cityIdx < cities.size(); cityIdx++){
        delete (cities[cityIdx]-> street);
        delete (cities[cityIdx]-> streetSegment);
        delete (cities[cityIdx]-> intersection);
        delete cities[cityIdx];
    }
    
    cities.clear(); 
}

//close the database
void closeDataBase(){
    
    if (streetDataBaseIsLoaded){
        closeStreetDatabase();
        streetDataBaseIsLoaded = false;
    }
        
    if (OSMIsLoaded){
        closeOSMDatabase();
        OSMIsLoaded = false;
    }
        
    
}


//Returns all street ids corresponding to street names that start with the given prefix
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {
    std::vector<StreetIdx> streets;

    //create a new string streetPrefix and remove space of street_prefix, then change to lower case
    std::string streetPrefix = "";

    for (int i = 0; i < street_prefix.length(); i++) {
        if (street_prefix[i] != ' ') {
            streetPrefix.push_back(tolower(street_prefix[i]));
        }
    }

    //if streetPrefix is not empty
    if (streetPrefix.length() > 0) {
        std::vector <int> adjustedNameList;
        
        //according to the length of streetPrefix, use the correct index vector to retrieve the street names starting with the first 1, 2 or 3 characters of streetPrefix
        if (streetPrefix.length() > PREFIX_NUM_CHAR) {
            if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
            } else if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE];
            } else {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE];
            }
        } else if (streetPrefix.length() > 1) {
            adjustedNameList = cities[currentCityIdx]->street->streetNamesTwoChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
        } else {
            adjustedNameList = cities[currentCityIdx]->street->streetNamesOneChar[tolower(streetPrefix[0])];
        }
        
        int leastEdit = INT_MAX;            //numbers of edits requires to change from input name to the similar name
        
        //loop through the street names within the index vector
        for (int i = 0; i < adjustedNameList.size(); i++){

            std::string streetName = getStreetName(adjustedNameList[i]);
            
            //find the more similar name to the input
            int numOfEdit = levenshteinDistance(street_prefix, streetName);
            if (numOfEdit < leastEdit){
                
                if (checkingFirstName){
                    mostSimilarFirstName.clear();
                    mostSimilarFirstName.push_back(adjustedNameList[i]);
                }else{
                    mostSimilarSecondName.clear();
                    mostSimilarSecondName.push_back(adjustedNameList[i]);
                }
                
                leastEdit = numOfEdit;
            }else if (numOfEdit == leastEdit){
                
                if (checkingFirstName){

                    mostSimilarFirstName.push_back(adjustedNameList[i]);
                }else{
                    mostSimilarSecondName.push_back(adjustedNameList[i]);
                }
            }
            
            //compare streetName and streetPrefix character by character
            int k = 1;
            if (streetPrefix.length() == 1){
                streets = adjustedNameList;
            }
            
            //loop through all characters in streetName
            for (int j = 1; j < streetName.length(); j++) {

                //if character is a space, skip to the next character
                if (streetName[j] != ' ') {

                    //if the character does not match, break out of the loop
                    if (tolower(streetName[j]) != streetPrefix[k]) {
                        break;
                    }
                    
                    k++;
                    //if all characters in streetPrefix has been compared and matched, store the street id
                    if (streetPrefix.length() <= k) {
                       
                        streets.push_back(adjustedNameList[i]);
                        break;
                    }
                }
            }

        }
        
    }

    return streets;

}

// Returns the length of a given street in meters
double findStreetLength(StreetIdx street_id){
    if (street_id < 0 || street_id >= getNumStreets()){
        return 0.0;
    }
    return cities[currentCityIdx]->street->streetLength[street_id];
}

// Return the smallest axis-aligned rectangle that contains all the intersections and curve points of the given street
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    //break the street into intersections, use the first intersection position as min/max LatLon
    std::vector<IntersectionIdx> intersections = findIntersectionsOfStreet(street_id);
    LatLon firstPoint = getIntersectionPosition(intersections[0]);
    LatLon maxPoint = firstPoint;
    LatLon minPoint = firstPoint;
    
    //loop through all intersections and update the min/max LatLon
    for (int i = 0; i < intersections.size(); i++){
        LatLon point = getIntersectionPosition(intersections[i]);
        
        minPoint = findMaxMin(minPoint, point, "min");
        maxPoint = findMaxMin(maxPoint, point, "max");

        //loop through the all the Street Segments
        for (int j = 0; j < getNumStreetSegments(); j++){
            StreetSegmentInfo ss_info = getStreetSegmentInfo(j);
        
            //locate the street id and loop through all the related curve points
            if(ss_info.streetID == street_id){
                
                for (int k = 0; k < ss_info.numCurvePoints; k++){
                    point = getStreetSegmentCurvePoint(j, k);
                    
                    minPoint = findMaxMin(minPoint, point, "min");
                    maxPoint = findMaxMin(maxPoint, point, "max");
                }
            }
        }
    }

    //use the min/max latitude and longitude to create LatLonBounds
    LatLonBounds box;
    box.min = minPoint;
    box.max = maxPoint;
    
    return box;
  
}

// Helper function to find the min/max LatLon point
LatLon findMaxMin(LatLon point, LatLon current, std::string method) {
    
    if (method.compare("max") == 0) {
        current = LatLon (std::max(current.latitude(), point.latitude()),
            std::max(current.longitude(), point.longitude()));
    } else {
        current = LatLon (std::min(current.latitude(), point.latitude()),
            std::min(current.longitude(), point.longitude()));
    }
    
    return current;
}
// Returns the nearest point of interest of the given name to the given position
POIIdx findClosestPOI(LatLon my_position, std::string POIname){
    //declare variables
    std::vector<POIIdx> matchedName;
    LatLon POIPos;
    LatLon myPos = my_position;
    
    //use the farthest distance of two points on earth in meters as the initial distance.  This value will be updated.  Data retrieved from Wikipedia.
    const double farthestDistanceOfTwoPointsOnEarth = 19996000;
    double distance = farthestDistanceOfTwoPointsOnEarth;
    POIIdx closest = 0;
    
    //loop through all POIs
    for (int i = 0; i < getNumPointsOfInterest(); i++){
        std::string name = getPOIName(i);
        
        //identify POIs with the given name, and get its position
        if (name.compare(POIname) == 0){
            POIPos =  getPOIPosition(i);
            std::pair<LatLon, LatLon> twoPoints(POIPos,myPos);
            
            //calculate distance between my_position and POI, update distance and the closest index of the POI
            if (distance > findDistanceBetweenTwoPoints(twoPoints)){
                distance = findDistanceBetweenTwoPoints(twoPoints);
                closest = i;               
            } 
        }
    }
    
    return closest;
}

// Returns the area of the given closed feature in square meters. Return 0 if this feature is not a closed polygon.
double findFeatureArea(FeatureIdx feature_id){
    //break the feature into feature points
    int numFeaturePoints = getNumFeaturePoints(feature_id);
    LatLon ptsPos;
    LatLon ptsPosPrev;
    double area = 0;
    double latAvg, sum = 0;
    const double polygonFormulaMultConstant = 0.5;
    
    //if the feature is a closed polygon, then calculate its area. Otherwise, keep the area as 0
    if (getFeaturePoint(feature_id, 0) == getFeaturePoint(feature_id, numFeaturePoints-1)){
        //calculate the average latitude
        for (int i = 0; i < numFeaturePoints; i++){
            sum += getFeaturePoint(feature_id, i).latitude();
        }
        
        latAvg = sum/numFeaturePoints;
        
        //calculate the feature area
        for (int i = 0; i < numFeaturePoints; i++){
            
            ptsPos = getFeaturePoint(feature_id, i);
            if (i > 0){
                area += polygonFormulaMultConstant * kEarthRadiusInMeters * kEarthRadiusInMeters * kDegreeToRadian * kDegreeToRadian * cos(latAvg * kDegreeToRadian) *
                        (ptsPos.longitude() * ptsPosPrev.latitude() - ptsPos.latitude() * ptsPosPrev.longitude());
            }
            
            ptsPosPrev = ptsPos;
        }
    }
    
    return abs(area);
}


/// Returns the street names at the given intersection (includes duplicate 
// street names in the returned vector)
// Speed Requirement --> high 
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    
    std::vector<std::string> streetNames;

    //check street segment around the intersection and get the street name
    for(StreetSegmentIdx index = 0; index < getNumIntersectionStreetSegment(intersection_id); index++){
        
        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, index);
        std::string nameOfStreet = getStreetName(getStreetSegmentInfo(ss_id).streetID);
        streetNames.push_back(nameOfStreet);
        
    }
    
    return streetNames;
    
}

// Returns all intersections reachable by traveling down one street segment 
// from the given intersection (hint: you can't travel the wrong way on a 
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    
    std::vector<IntersectionIdx> adjIntersections;
    
    for(StreetSegmentIdx i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++){
        
        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, i);
        IntersectionIdx from, to;
        
        from = getStreetSegmentInfo(ss_id).from;
        to = getStreetSegmentInfo(ss_id).to;
        bool oneWay = getStreetSegmentInfo(ss_id).oneWay;
        
        //only load the intersection that is reachable
        if (!oneWay || to != intersection_id){
            std::vector<IntersectionIdx>::iterator exist;        //used to check whether the index is already in the vector
            
            if (from == intersection_id){
                
                //prevent duplicate intersection index
                exist = std::find(adjIntersections.begin(), adjIntersections.end(), to);
                
                if (exist == adjIntersections.end())
                    adjIntersections.push_back(to);
                
            }else{
                exist = std::find(adjIntersections.begin(), adjIntersections.end(), from);
                
                if (exist == adjIntersections.end())
                    adjIntersections.push_back(from);
            }
        }
    }
    
    return adjIntersections;
}

// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    
    //index is out of bound
    if (street_id < 0 || street_id >= getNumStreets()){
        std::vector<IntersectionIdx> empty;
        return empty;
    }
    
    return cities[currentCityIdx]->street->streetIntersections[street_id];

}


// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){
    
    std::vector<IntersectionIdx> commonIntersections;
    std::vector<IntersectionIdx> firstStreet = findIntersectionsOfStreet(street_ids.first);
    std::vector<IntersectionIdx> secondStreet = findIntersectionsOfStreet(street_ids.second);
    
    for (std::vector<IntersectionIdx>::iterator i = firstStreet.begin();  i != firstStreet.end(); i++){
        
        //found the common item from these two vector
        std::vector<IntersectionIdx>::iterator common = std::find(secondStreet.begin(), secondStreet.end(), *i);
        
        if (common != secondStreet.end()){
            commonIntersections.push_back(*i);
        }
    }
    

    return commonIntersections;
}




// Returns the distance between two (latitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    double distanceBetweenTwoPoints = 0;
    double latInRadius1,lonInRadius1,latInRadius2,lonInRadius2;
    
    //converting longitude and latitude from degree to radius
    latInRadius1 = points.first.latitude() * kDegreeToRadian;
    lonInRadius1 = points.first.longitude() * kDegreeToRadian;
    latInRadius2 = points.second.latitude() * kDegreeToRadian;
    lonInRadius2 = points.second.longitude() * kDegreeToRadian;
    
    //convert to position to (x,y) in meters
    double x1, y1, x2, y2;
    x1 = lonInRadius1 * cos((latInRadius2+latInRadius1)/2);
    y1 = latInRadius1;
    x2 = lonInRadius2 * cos((latInRadius2+latInRadius1)/2);
    y2 = latInRadius2;
    
    //using Pythagora's theorem to calculate distance between two points
    distanceBetweenTwoPoints = kEarthRadiusInMeters * sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    
    return distanceBetweenTwoPoints;
}



// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {
    
    if(street_segment_id < 0 || street_segment_id >= getNumStreetSegments()){
        return 0;
    }
    return cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id];
    
}



// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    
    if (street_segment_id < 0 || street_segment_id >= getNumStreetSegments()) {
        return 0;
    }
    
    return cities[currentCityIdx]->streetSegment->streetSegTravelTime[street_segment_id];

}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection_id = -1;
    double distance=2*M_PI*kEarthRadiusInMeters;
    
    for(int i = 0; i < getNumIntersections(); i++) {
        std::pair <LatLon, LatLon> points (my_position, getIntersectionPosition(i));
        
        if(findDistanceBetweenTwoPoints(points) < distance){
            distance = findDistanceBetweenTwoPoints(points);
            closestIntersection_id = i;
        }
    }
    return closestIntersection_id;
}

// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
 
    //check if out of boundary of index
    if(intersection_id < 0 || intersection_id >= getNumIntersections()){
        std::vector<StreetSegmentIdx> empty;
        return empty;
    }
    
    return cities[currentCityIdx]->intersection->intersectionStreetSegments[intersection_id];
    
}

//an algorithm that gives how much edit I need to let the two string to be the same
int levenshteinDistance(std::string first, std::string second){
    int rows = first.length() + 1;
    int cols = second.length() + 1;
    
    std::vector<std::vector<int>> matrix;
    
    matrix.resize(rows);
    
    for (int row = 0; row < rows; row++){
        matrix[row].resize(cols);
        for (int col = 0; col < cols; col++){
            matrix[row][col] = 0;
        }      
    }
    
    for (int row = 1; row < rows; row++){
        for(int col = 1; col < cols; col++){
            matrix[row][0] = row;
            matrix[0][col] = col;
        }
    }
    
    for (int col = 1; col < cols; col++){
        for (int row = 1; row < rows; row++){
            
            int adder;
            
            if (first[row - 1] == second [col - 1]){
                adder = 0;
            }else{
                adder = 1;
            }
            
            int temp =  std::min(matrix[row - 1][col] + 1, matrix[row][col - 1] + 1);
            temp = std::min(temp, matrix[row - 1][col - 1] + adder);                         
            matrix[row][col] = temp;
        }
    }
    
    return matrix[rows - 1][cols -1];
    
}