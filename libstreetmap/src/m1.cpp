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
constexpr double kEarthRadiusInMeters = 6372797.560856;
constexpr double kDegreeToRadian = 0.017453292519943295769236907684886;
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







std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> streets;
    std::string streetPrefix = street_prefix;
    std::for_each(streetPrefix.begin(), streetPrefix.end(), [](char & c){
            c = ::tolower(c);
    });
    //get street name, compare with street prefix.
    for (int i = 0; i < getNumStreets(); i++){
        std::string streetName = getStreetName(i);
        std::string streetNameSub = streetName.substr(0, street_prefix.length()-1);
        std::for_each(streetNameSub.begin(), streetNameSub.end(), [](char & c){
            c = ::tolower(c);
        });
        
        if (streetNameSub.compare(streetPrefix) == 0){
            streets.push_back(i);
        }
    }
    return streets;
}

double findStreetLength(StreetIdx street_id){
    
    std::vector<IntersectionIdx> intersections = findIntersectionsOfStreet(street_id);
    double length = 0;
    for (int i = 0; i < getNumStreetSegments(); i++){
        StreetSegmentInfo ss_info = getStreetSegmentInfo(i);
        if(ss_info.streetID == street_id){
            length = length + findStreetSegmentLength(i);
        }
    }
    return length;
}

LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    std::vector<IntersectionIdx> intersections = findIntersectionsOfStreet(street_id);
    LatLon point = getIntersectionPosition(0);
    double maxLat = point.latitude(), minLat = point.latitude();
    double maxLon = point.longitude(), minLon = point.longitude();
    for (int i = 0; i < intersections.size(); i++){
        LatLon point = getIntersectionPosition(intersections[i]);
        if (point.latitude() > maxLat){
            maxLat = point.latitude();
        }
        if (point.latitude() < minLat){
            minLat = point.latitude();
        }
        if (point.longitude()> maxLon){
            maxLon = point.longitude();
        }
        if (point.longitude() < minLon){
            minLon = point.longitude();
        }
        
    }
    LatLon min(minLat, minLon);
    LatLon max(maxLat, maxLon);
    LatLonBounds box;
    box.min = min;
    box.max = max;
    return box;
  
}

POIIdx findClosestPOI(LatLon my_position, std::string POIname){
    std::vector<POIIdx> matchedName;
    LatLon POIPos;
    LatLon myPos = my_position;
    double distance = 100000000000000;
    POIIdx closest;
    for (int i = 0; i < getNumPointsOfInterest(); i++){
        std::string name = getPOIName(i);
        if (name.compare(POIname) == 0){
            POIPos =  getPOIPosition(i);
            std::pair<LatLon, LatLon> twoPoints(POIPos,myPos);
            
            if (distance > findDistanceBetweenTwoPoints(twoPoints)){
                distance = findDistanceBetweenTwoPoints(twoPoints);
                closest = i;               
            }
            
        }
    }
    return closest;
}

double findFeatureArea(FeatureIdx feature_id){
    int numFeaturePoints = getNumFeaturePoints(feature_id);
    LatLon ptsPos;
    LatLon ptsPosPrev;
    double area = 0;
    double latAvg, sum = 0;
    if (getFeaturePoint(feature_id, 0) == getFeaturePoint(feature_id, numFeaturePoints-1)){
        for (int i = 0; i < numFeaturePoints; i++){
            sum += getFeaturePoint(feature_id, i).latitude();
        }
        latAvg = sum/numFeaturePoints;
        for (int i = 0; i < numFeaturePoints; i++){
            ptsPos = getFeaturePoint(feature_id, i);
            if (i > 0){
                area += kEarthRadiusInMeters*(ptsPos.longitude()*cos(latAvg*kDegreeToRadian)/*x1*/*ptsPosPrev.latitude()/*y2*/
                        -ptsPos.latitude()/*y1*/*ptsPosPrev.longitude()*cos(latAvg*kDegreeToRadian));
            }
            ptsPosPrev = ptsPos;
        }
    }
    
    return abs(area/2);
    
}
