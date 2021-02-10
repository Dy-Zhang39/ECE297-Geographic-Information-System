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
#include <algorithm>
#include <math.h>
#include <map>


#include<bits/stdc++.h>
#include<stdio.h>
#include<ctype.h>
#include<boost/algorithm/string.hpp>

using namespace std;
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
vector<vector<StreetSegmentIdx>> INTERSECTION_STREET_SEGMENT;
vector<vector<IntersectionIdx>> STREET_INTERSECTION;
vector<vector<StreetIdx>> STREET_NAMES;
int CHAR_SIZE = 256;

vector<double> streetSegLength;
vector<double>streetSegTravelTime;
vector<double>streetLength;



bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded 
                                  //successfully
    if(!load_successful)
        return load_successful;
    
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    //
    // Load your map related data structures here.
    //
    // Given a intersection find all street segments connect to it

    //    initialize vector<double> streetSegLength;
    streetLength.resize(getNumStreets());
        for (int i = 0; i < getNumStreets(); i++) { 
            streetLength[i] = 0;
        }


    INTERSECTION_STREET_SEGMENT.resize(getNumIntersections()); //create empty vector for each intersection
    STREET_INTERSECTION.resize(getNumStreets());
    for( int intersection = 0; intersection < getNumIntersections(); ++intersection){
        //iterate through all intersections
        for( int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
            //iterate through all segments at intersection
            int streetSegID = getIntersectionStreetSegment(intersection, i);
            auto streetSegInfo = getStreetSegmentInfo(streetSegID);
            auto streetID = streetSegInfo.streetID;
            auto existed = find(STREET_INTERSECTION[streetID].begin(), STREET_INTERSECTION[streetID].end(), intersection);
            if(existed == STREET_INTERSECTION[streetID].end()){
                STREET_INTERSECTION[streetID].push_back(intersection);
            }
            INTERSECTION_STREET_SEGMENT[intersection].push_back(streetSegID);        //save segments connected to intersection
        }
    }
    
    //Find streets length and their corresponding travel time
    streetSegLength.resize(getNumStreetSegments());
    streetSegTravelTime.resize(getNumStreetSegments());
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        double streetSegmentLength = 0;
        struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);

        //find the starting and ending position of given street segment
        auto from = getIntersectionPosition(streetSegmentID.from);
        auto to = getIntersectionPosition(streetSegmentID.to);

        //find curve points on the given street segment
        if (streetSegmentID.numCurvePoints > 1) {
            //more than one curve points
            pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0)); //calculate distance between start point and first curve point
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints);

            int i = 1;
            for (; i < streetSegmentID.numCurvePoints; i++) {
                pair <LatLon, LatLon> curvePoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), getStreetSegmentCurvePoint(street_segment_id, i));

                streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(curvePoints); //add the distance between each curve points
            }

            pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), to);

            streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(lastPoints); //add the distance between last curve point and end point
            streetSegLength[street_segment_id]=streetSegmentLength;           
        }
        else if (streetSegmentID.numCurvePoints == 1) { //only has one curve point
            pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, 0), to);

            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints) + findDistanceBetweenTwoPoints(lastPoints); // add distance from start point to the only curve point to the end point 
            streetSegLength[street_segment_id]=streetSegmentLength;

        } else {
            //straight segment
            pair <LatLon, LatLon> points(from, to);

            streetSegmentLength = findDistanceBetweenTwoPoints(points); //calculate distance between start and end point
            streetSegLength[street_segment_id] = streetSegmentLength;
        }
        //find street segment travel tinme
        double speedLimit = streetSegmentID.speedLimit;
        streetSegTravelTime[street_segment_id] = streetSegmentLength / speedLimit;
        
        //find street length
        streetLength[streetSegmentID.streetID] += streetSegmentLength;
    }

   

     
     

    
     //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed
    
    // Load street name into the first vector using first 2 characters as index
    
    STREET_NAMES.resize(CHAR_SIZE * CHAR_SIZE);

    for (int i = 0; i < getNumStreets(); i++){
        // Get street name, remove space and convert to lower cases. Got only the first 2 letters.
        std::string streetName = getStreetName(i);
        std::string streetNameSub = "";
    
        for (int j = 0; j < streetName.length(); j++) {
            if (streetName[j] != ' ') {
                streetNameSub.push_back(tolower(streetName[j]));
            }
            
            if (streetNameSub.length() > 2) break;
        }

        // Store the street id into the index vector: STREET_NAMES
        STREET_NAMES[tolower(streetName[0]) * CHAR_SIZE + tolower(streetName[1])].push_back(i);
    }
    
    return load_successful;
}

void closeMap() {
    closeStreetDatabase();
    //Clean-up your map related data structures here

    
}

// Returns all street ids corresponding to street names that start with the given prefix

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
        
        //get street name (lower case), compare with street prefix
        vector <int> adjustedNameList = STREET_NAMES[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
        for (int i = 0; i < adjustedNameList.size(); i++){
            std::string streetName = getStreetName(adjustedNameList[i]);

            //compare streetName and streetPrefix character by character
            int k = 1;
            
            //loop through all characters in streetName
            for (int j = 1; j < streetName.length(); j++) {
                
                //if character is a space, skip to the next character
                if (streetName[j] != ' ') {
                    
                    //if the character does not match, break out of the loop
                    if (tolower(streetName[j]) != streetPrefix[k]) {
                        break;
                    }
                    
                    //if all characters in streetPrefix has been compared and matched, store the street id
                    if (streetPrefix.length() <= k + 1) {
                        streets.push_back(adjustedNameList[i]);
                        break;
                    }  
                                        
                    k++;
                }    
            }


    return streets;
}

// Returns the length of a given street in meters
double findStreetLength(StreetIdx street_id){

    return streetLength[street_id];
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
vector<string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    
    vector<string> streetNames;

    //check street segment around the intersection and get the street name
    for(StreetSegmentIdx i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++){
        
        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, i);
        string nameOfStreet = getStreetName(getStreetSegmentInfo(ss_id).streetID);
        streetNames.push_back(nameOfStreet);
        
    }
    
    return streetNames;
    
}

// Returns all intersections reachable by traveling down one street segment 
// from the given intersection (hint: you can't travel the wrong way on a 
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high 
vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    
    vector<IntersectionIdx> adjIntersections;
    
    for(StreetSegmentIdx i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++){
        
        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, i);
        IntersectionIdx from, to;
        
        
        from = getStreetSegmentInfo(ss_id).from;
        to = getStreetSegmentInfo(ss_id).to;
        bool oneWay = getStreetSegmentInfo(ss_id).oneWay;
        
        //only load the intersection that is reachable
        if (!oneWay || to != intersection_id){
            vector<IntersectionIdx>::iterator exist;        //used to check whether the index is already in the vector
            
            if (from == intersection_id){
                
                //prevent duplicate intersection index
                exist = find(adjIntersections.begin(), adjIntersections.end(), to);
                
                if (exist == adjIntersections.end())
                    adjIntersections.push_back(to);
                
            }else{
                exist = find(adjIntersections.begin(), adjIntersections.end(), from);
                
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
vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return STREET_INTERSECTION[street_id];
    vector<IntersectionIdx> i_ids;                                        //final output structure
    map<IntersectionIdx, int> i_ids_map;                                  //used for faster find(), note that the second data is useless
    
    for (StreetSegmentIdx ss_id = 0; ss_id < getNumStreetSegments(); ss_id++){
        
        StreetSegmentInfo ss_info = getStreetSegmentInfo(ss_id);
        
        if (ss_info.streetID == street_id){
            
            map<IntersectionIdx, int>::iterator fromExist, toExist;     //used to prevent duplicate intersections   
            
            fromExist = i_ids_map.find(ss_info.from);                      
            if (fromExist == i_ids_map.end())
                i_ids_map.insert(make_pair(ss_info.from,0));
            
            toExist = i_ids_map.find(ss_info.to);
            if (toExist == i_ids_map.end())
                i_ids_map.insert(make_pair(ss_info.to, 0));
             
        }
    }
    
    //put all the data from the map structure to the vector structure
    i_ids.resize(i_ids_map.size());
    int index = 0;
    for (map<IntersectionIdx, int>::iterator i = i_ids_map.begin(); i != i_ids_map.end(); i++){
        i_ids[index] = i->first;
        index++;
    }

}


// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx, StreetIdx> street_ids){
    
    vector<IntersectionIdx> intersections;
    vector<IntersectionIdx> firstStreet = findIntersectionsOfStreet(street_ids.first);
    vector<IntersectionIdx> secondStreet = findIntersectionsOfStreet(street_ids.second);
    
    
    for (vector<IntersectionIdx>::iterator i = firstStreet.begin();  i != firstStreet.end(); i++){
        
        //found the common item from these two vector
        vector<IntersectionIdx>::iterator common = find(secondStreet.begin(), secondStreet.end(), *i);
        
        if (common != secondStreet.end()){
            intersections.push_back(*i);
        }
    }
    return intersections;
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
    return streetSegLength[street_segment_id];
    
}



// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){

    return streetSegTravelTime[street_segment_id];

}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection_id = -1;
    double distance=2*3.15*kEarthRadiusInMeters;
    for(int i = 0; i < getNumIntersections(); i++) {
        pair <LatLon, LatLon> points (my_position, getIntersectionPosition(i));
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
 
    return INTERSECTION_STREET_SEGMENT[intersection_id];
    
}
