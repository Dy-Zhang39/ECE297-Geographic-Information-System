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
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    //
    // Load your map related data structures here.
    //

    

    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here
    
}


// Returns the distance between two (latitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    double distanceBetweenTwoPoints=0;
    double latInRadius1,lonInRadius1,latInRadius2,lonInRadius2;
    //converting longitude and latitude from degree to radius
    latInRadius1=points.first.latitude()*kDegreeToRadian;
    lonInRadius1=points.first.longitude()*kDegreeToRadian;
    latInRadius2=points.second.latitude()*kDegreeToRadian;
    lonInRadius2=points.second.longitude()*kDegreeToRadian;
    //convert to position to (x,y) in meters
    double x1, y1, x2, y2;
    x1=lonInRadius1*cos((latInRadius2+latInRadius1)/2);
    y1=latInRadius1;
    x2=lonInRadius2*cos((latInRadius2+latInRadius1)/2);
    y2=latInRadius2;
    //using Pythagora's theorem to calculate distance between two points
    distanceBetweenTwoPoints=kEarthRadiusInMeters*sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    return distanceBetweenTwoPoints;
}

/*********************below functions are not properly implemented, only used to testing
// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    double streetSegmentLength;
    
    return streetSegmentLength;
}


// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    double streetSegmentTravelTime;
    
    return streetSegmentTravelTime;
}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection;
    
    return closestIntersection;
}


///////////////////
double findStreetLength(StreetIdx street_id){
    double x;
    return x;
}

double findFeatureArea(FeatureIdx feature_id){
    double y;
    return y;
}
 * 
 * /