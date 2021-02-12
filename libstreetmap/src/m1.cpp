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


#include<bits/stdc++.h>
#include<stdio.h>
#include<ctype.h>
#include<boost/algorithm/string.hpp>


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


//global variable
Street* STREETS;
StreetSegment* STREET_SEGMENTS;
Intersection* INTERSECTIONS;

int CHAR_SIZE = 256;
int PREFIX_NUM_CHAR = 2;      //for searches with partial name: if partial name is longer than this number, then use 2-character index index, otherwise use 1-character index
char SEPARATE_CHAR = 'k';
char SEPARATE_CHAR_AFTER = 's';
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded 
                                                                                  //successfully
    if(!load_successful)
        return load_successful;

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    
    //dynamic allocate the global variable
    STREETS = new Street;
    STREET_SEGMENTS = new StreetSegment;
    INTERSECTIONS = new Intersection;
    
    //    initialize vector<double> streetSegLength;
    STREETS->streetLength.resize(getNumStreets());

    // Load index vectors used to quick search street names
    // Load street name into the vector using first 1 characters as index for STREETS->streetNamesOneChar,
    // Load street name into the vector using first 2 characters as index for STREETS->streetNamedTwoChar,
    // Load street name into the vector using first 3 characters as index for STREETS->streetNamesThreeChar.
    STREETS->streetNamesOneChar.resize(CHAR_SIZE);
    STREETS->streetNamesTwoChar.resize(CHAR_SIZE * CHAR_SIZE);
    STREETS->streetNamesThreeChar.resize(CHAR_SIZE * CHAR_SIZE * 3);

    for (int i = 0; i < getNumStreets(); i++){
        //initialize all the element in the street length to 0 to prevent undefined variable
        STREETS->streetLength[i] = 0;
        
        // Get street name, remove space and convert to lower cases. Got only the first 2 letters.
        std::string streetName = getStreetName(i);
        std::string streetNameSub = "";
    
        for (int j = 0; j < streetName.length(); j++) {
            
            if (streetName[j] != ' ') {
                streetNameSub.push_back(tolower(streetName[j]));
            }
            
            if (streetNameSub.length() > PREFIX_NUM_CHAR) break;
        }

        // Store the street id into the index vector: streetNamesOneChar, streetNamesTwoChar
        if (streetNameSub.length() > 0)
            STREETS->streetNamesOneChar[tolower(streetNameSub[0])].push_back(i);
        
        if (streetNameSub.length() > 1)
            STREETS->streetNamesTwoChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);

        if (streetNameSub.length() > PREFIX_NUM_CHAR && PREFIX_NUM_CHAR > 1) {
            // Due to the memory limit, anything with the third character before SEPARATE_CHAR will be in one index, otherwise if it is before SEPARATE_CHAR_AFTER, will be in another, else it will be in the remained index.
            if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                STREETS->streetNamesThreeChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);
            } else if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                STREETS->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE].push_back(i);
            } else {
                STREETS->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE].push_back(i);
            }
        }
    }

    INTERSECTIONS->intersectionStreetSegments.resize(getNumIntersections()); //create empty vector for each intersection
    STREETS->streetIntersections.resize(getNumStreets());
    
    for(int intersection = 0; intersection < getNumIntersections(); ++intersection){
        
        //iterate through all intersections
        for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
            
            //iterate through all segments at intersection
            int streetSegID = getIntersectionStreetSegment(intersection, i);
            auto streetSegInfo = getStreetSegmentInfo(streetSegID);
            auto streetID = streetSegInfo.streetID;

            if (STREETS->streetIntersections[streetID].size() == 0 || STREETS->streetIntersections[streetID].back() < intersection) {
                STREETS->streetIntersections[streetID].push_back(intersection);              //save the intersection to the street it belongs to
            }
            
            INTERSECTIONS->intersectionStreetSegments[intersection].push_back(streetSegID);  //save segments connected to intersection
        }
    }

    //Find streets length and their corresponding travel time
    STREET_SEGMENTS->streetSegLength.resize(getNumStreetSegments());
    STREET_SEGMENTS->streetSegTravelTime.resize(getNumStreetSegments());
    
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        double streetSegmentLength = 0;
        struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);

        //find the starting and ending position of given street segment
        auto from = getIntersectionPosition(streetSegmentID.from);
        auto to = getIntersectionPosition(streetSegmentID.to);

        //find curve points on the given street segment (more than one curve point)
        if (streetSegmentID.numCurvePoints > 1) {

            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            //calculate distance between start point and first curve point
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints);

            int i = 1;
            for (; i < streetSegmentID.numCurvePoints; i++) {
                std::pair <LatLon, LatLon> curvePoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), getStreetSegmentCurvePoint(street_segment_id, i));
                //add the distance between each curve points
                streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(curvePoints);
            }

            //add the distance between last curve point and end point
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), to);
            streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(lastPoints);
            STREET_SEGMENTS->streetSegLength[street_segment_id] = streetSegmentLength;

        } else if (streetSegmentID.numCurvePoints == 1) { //only has one curve point
            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, 0), to);
            
            // add distance from start point to the only curve point to the end point 
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints) + findDistanceBetweenTwoPoints(lastPoints);
            STREET_SEGMENTS->streetSegLength[street_segment_id] = streetSegmentLength;

        } else {
            //straight segment
            std::pair <LatLon, LatLon> points(from, to);
            //calculate distance between start and end point
            streetSegmentLength = findDistanceBetweenTwoPoints(points);
            STREET_SEGMENTS->streetSegLength[street_segment_id] = streetSegmentLength;
        }
        
        
        //find street segment travel time
        double speedLimit = streetSegmentID.speedLimit;
        STREET_SEGMENTS->streetSegTravelTime[street_segment_id] = streetSegmentLength / speedLimit;
        
        //find street length
        STREETS->streetLength[streetSegmentID.streetID] += streetSegmentLength;
    }
 
    //MAP_LOADED.push_back(map_streets_database_filename);
    return load_successful;
}

    
    
//clear all the global data structure to prevent it get bigger and bigger    
void closeMap() {
    
    delete STREETS;
    delete STREET_SEGMENTS;
    delete INTERSECTIONS;
    
    closeStreetDatabase();
    //Clean-up your map related data structures here

    
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
        if (streetPrefix.length() > PREFIX_NUM_CHAR && PREFIX_NUM_CHAR > 1) {
            if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                adjustedNameList = STREETS->streetNamesThreeChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
            } else if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                adjustedNameList = STREETS->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE];
            } else {
                adjustedNameList = STREETS->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE];
            }
        } else if (streetPrefix.length() > 1) {
            adjustedNameList = STREETS->streetNamesTwoChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
        } else {
            adjustedNameList = STREETS->streetNamesOneChar[tolower(streetPrefix[0])];
        }
        
        //loop through the street names within the index vector
        for (int i = 0; i < adjustedNameList.size(); i++){

            std::string streetName = getStreetName(adjustedNameList[i]);

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
    return STREETS->streetLength[street_id];
}

// Return the smallest axis-aligned rectangle that contains all the intersections and curve points of the given street
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    //break the street into intersections, use the first intersection position as min/max LatLon
    std::vector<IntersectionIdx> intersections = findIntersectionsOfStreet(street_id);
    LatLon firstPoint = getIntersectionPosition(intersections[0]);
    double maxLat = firstPoint.latitude();
    double minLat = firstPoint.latitude();
    double maxLon = firstPoint.longitude();
    double minLon = firstPoint.longitude();
    
    //loop through all intersections and update the min/max LatLon
    for (int i = 0; i < intersections.size(); i++){
        LatLon point = getIntersectionPosition(intersections[i]);
        
        //update min/max latitude
        if (point.latitude() > maxLat){
            maxLat = point.latitude();
        } else if (point.latitude() < minLat) {
            minLat = point.latitude();
        }
        
        //update min/max longitude
        if (point.longitude() > maxLon){
            maxLon = point.longitude();
        } else if (point.longitude() < minLon) {
            minLon = point.longitude();
        }

        //loop through the all the Street Segments
        for (int j = 0; j < getNumStreetSegments(); j++){
            StreetSegmentInfo ss_info = getStreetSegmentInfo(j);
        
            //locate the street id and loop through all the related curve points
            if(ss_info.streetID == street_id){
                
                for (int k = 0; k < ss_info.numCurvePoints; k++){
                    point = getStreetSegmentCurvePoint(j, k);
                    
                    //update min/max latitude
                    if (point.latitude() > maxLat){
                        maxLat = point.latitude();
                    } else if (point.latitude() < minLat) {
                        minLat = point.latitude();
                    }

                    //update min/max longitude
                    if (point.longitude() > maxLon){
                        maxLon = point.longitude();
                    } else if (point.longitude() < minLon) {
                        minLon = point.longitude();
                    }
                }
            }
        }
    }
    
    //use the min/max latitude and longitude to create LatLonBounds
    LatLon min(minLat, minLon);
    LatLon max(maxLat, maxLon);
    LatLonBounds box;
    box.min = min;
    box.max = max;
    
    return box;
  
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
    
    return STREETS->streetIntersections[street_id];

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
    return STREET_SEGMENTS->streetSegLength[street_segment_id];
    
}



// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    
    if (street_segment_id < 0 || street_segment_id >= getNumStreetSegments()) {
        return 0;
    }
    
    return STREET_SEGMENTS->streetSegTravelTime[street_segment_id];

}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection_id = -1;
    double distance=2*3.15*kEarthRadiusInMeters;
    
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
    
    return INTERSECTIONS->intersectionStreetSegments[intersection_id];
    
}
