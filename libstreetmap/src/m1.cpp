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
vector<vector<StreetSegmentIdx>> intersection_street_segments;


bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded 
                                  //successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    
    //
    // Load your map related data structures here.
    //
    intersection_street_segments.resize(getNumIntersections()); //create empty vector for each intersection
    
    for( int intersection = 0; intersection < getNumIntersections(); ++intersection){
        //iterate through all intersections
        for( int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
            //iterate through all segments at intersection
            int ss_id = getIntersectionStreetSegment(intersection, i);
            intersection_street_segments[intersection].push_back(ss_id);        //save segments connected to intersection
        }
    }


     //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    closeStreetDatabase();
    //Clean-up your map related data structures here
    
}


//return the duplicates street names of a intersection
vector<string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    vector<string> streetNames;

  
//    for(StreetSegmentIdx i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++){
//        
//        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, i);
//        string nameOfStreet = getStreetName(getStreetSegmentInfo(ss_id).streetID);
//        streetNames.push_back(nameOfStreet);
//        
//    }
    
    return streetNames;
    
}


vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    
    vector<IntersectionIdx> adjIntersections;
    
//    for(StreetSegmentIdx i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++){
//        
//        StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection_id, i);
//        IntersectionIdx from, to;
//        from = getStreetSegmentInfo(ss_id).from;
//        to = getStreetSegmentInfo(ss_id).to;
//        bool oneWay = getStreetSegmentInfo(ss_id).oneWay;
//        
//        if (!oneWay || to != intersection_id){
//            vector<IntersectionIdx>::iterator exist; 
//            
//            if (from == intersection_id){
//                exist = find(adjIntersections.begin(), adjIntersections.end(), to);
//                
//                if (exist == adjIntersections.end())
//                    adjIntersections.push_back(to);
//                
//            }else{
//                exist = find(adjIntersections.begin(), adjIntersections.end(), from);
//                
//                if (exist == adjIntersections.end())
//                    adjIntersections.push_back(from);
//            }
//        }
//    }
    
    return adjIntersections;
}


vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
   
    vector<IntersectionIdx> i_ids;
//    
//    for (StreetSegmentIdx ss_id = 0; ss_id < getNumStreetSegments(); ss_id++){
//        StreetSegmentInfo ss_info = getStreetSegmentInfo(ss_id);
//        
//        if (ss_info.streetID == street_id){
//            vector<IntersectionIdx>::iterator fromExist, toExist;
//            
//            fromExist = find(i_ids.begin(), i_ids.end(), ss_info.from);            
//            if (fromExist == i_ids.end())
//                i_ids.push_back(ss_info.from);
//            
//            toExist = find(i_ids.begin(), i_ids.end(), ss_info.to);
//            if (toExist == i_ids.end())
//                i_ids.push_back(ss_info.to);
//        }
//    }
    
    return i_ids;
}



vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx, StreetIdx> street_ids){
    
    vector<IntersectionIdx> intersections;
//    vector<IntersectionIdx> firstStreet = findIntersectionsOfStreet(street_ids.first);
//    vector<IntersectionIdx> secondStreet = findIntersectionsOfStreet(street_ids.second);
//    
//    
//    for (vector<IntersectionIdx>::iterator i = firstStreet.begin();  i != firstStreet.end(); i++){
//        vector<IntersectionIdx>::iterator common = find(secondStreet.begin(), secondStreet.end(), *i);
//        if (common != secondStreet.end()){
//            intersections.push_back(*i);
//        }
//    }
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
    double streetSegmentLength=0;
    struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);  
    
    //find the starting and ending position of given street segment
    auto from =getIntersectionPosition(streetSegmentID.from);
    auto to = getIntersectionPosition(streetSegmentID.to);
    
    //find curve points on the given street segment
    if(streetSegmentID.numCurvePoints > 1) {
        //more than one curve points
        pair <LatLon, LatLon> firstPoints (from, getStreetSegmentCurvePoint( street_segment_id, 0 ));   //calculate distance between start point and first curve point
        streetSegmentLength=findDistanceBetweenTwoPoints(firstPoints);
        
        int i = 1;
        for( ; i < streetSegmentID.numCurvePoints; i++) {
            pair <LatLon, LatLon> curvePoints (getStreetSegmentCurvePoint( street_segment_id, i - 1),getStreetSegmentCurvePoint( street_segment_id, i ));
            
            streetSegmentLength = streetSegmentLength+findDistanceBetweenTwoPoints(curvePoints);    //add the distance between each curve points
        }
        
        pair <LatLon, LatLon> lastPoints (getStreetSegmentCurvePoint( street_segment_id, i-1 ), to);
        
        streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(lastPoints);       //add the distance between last curve point and end point
        return streetSegmentLength;
       
    }if(streetSegmentID.numCurvePoints==1){ //only has one curve point
        pair <LatLon, LatLon> firstPoints (from, getStreetSegmentCurvePoint ( street_segment_id, 0));   
        pair <LatLon, LatLon> lastPoints (getStreetSegmentCurvePoint( street_segment_id, 0), to);
        
        streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints) + findDistanceBetweenTwoPoints(lastPoints); // add distance from start point to the only curve point to the end point 
        return streetSegmentLength;
        
    }else{
        //straight segment
        pair <LatLon, LatLon> points (from, to);
        
        streetSegmentLength=findDistanceBetweenTwoPoints(points);   //calculate distance between start and end point
        return streetSegmentLength;
    }
    
}



// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    double streetSegmentTravelTime = -1;
    double distance = findStreetSegmentLength(street_segment_id);
    struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);
    double speedLimit = streetSegmentID.speedLimit;
    
    streetSegmentTravelTime = distance/speedLimit;
    return streetSegmentTravelTime;

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
 
    return intersection_street_segments[intersection_id];
    
}



//Below are draft functions

std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    vector<StreetIdx> s;
    return s;
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id){
    return 0;
}

// Return the smallest rectangle that contains all the intersections and 
// curve points of the given street (i.e. the min,max lattitude 
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none 
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    LatLonBounds s;
    return s;
}

// Returns the nearest point of interest of the given name to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POIname){
    POIIdx i;
    return i;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    return 0;
}

