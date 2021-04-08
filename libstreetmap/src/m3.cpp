/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <m1.h>
#include <m3.h>
#include <thread>
#include <queue>
#include <vector>
#include "global.h"
#include "dataHandler.h"

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"

#include "drawMap.h"


extern std::vector<City*> cities;
extern int currentCityIdx;
extern std::vector <StreetSegmentIdx> exploredPath;

std::vector <std::string> displayRoute(std::vector <StreetSegmentIdx> route);



bool operator < (const WavePoint& lhs, const WavePoint& rhs){
    return lhs.heuristicTime < rhs.heuristicTime;
}

bool operator > (const WavePoint& lhs, const WavePoint& rhs){
    return lhs.heuristicTime > rhs.heuristicTime;
}



WavePoint::WavePoint(IntersectionIdx index, double time) {
        
     WavePoint::idx = index;
     WavePoint::heuristicTime = time;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination,
        const double turn_penalty){

    //std::clock_t start = clock();
    //exploredPath.clear();
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > wavePoints;
    std::vector <PathNode> allIntersections;
    std::vector<StreetSegmentIdx> route;
    
    double averageSpeed = 30;
    double speedSortingWavefront = 30;
    bool pathFound = false;
    double timeToReach = 99999999;
    int totalIntersections = getNumIntersections();

    // initialize allIntersections vector
    allIntersections.resize(totalIntersections);
    
    for (int i = 0; i < allIntersections.size(); i ++) {
        allIntersections[i].lastIntersection = -1;
        allIntersections[i].travelTime = -1;
    }
    
    // Prepare the variables for the start optDistanceinterseoptDistancection.
    PathNode node;
    
    node.from = -1;
    node.travelTime = 0;
    node.distance = findDistanceBetweenIntersections(intersect_id_start, intersect_id_destination);
    node.lastIntersection = -2;

    allIntersections[intersect_id_start] = node;

    // Get the first wavePoint (waveFront)
    wavePoints.push(WavePoint(intersect_id_start, 0));
    
    while (!wavePoints.empty()) {

        //Find next nodes to explore and remove it from wavePoints
        WavePoint wavePoint = wavePoints.top();
        wavePoints.pop();
        
        // Find adjacent Intersections.
        std::vector<StreetSegmentIdx> adjacentSegments = findStreetSegmentsOfIntersection(wavePoint.idx);
        
        for (int i = 0; i < adjacentSegments.size(); i++) {

            StreetSegmentInfo segmentInfo = getStreetSegmentInfo(adjacentSegments[i]);
            //this line is use for performance tuning, and to show all the path explored.
            //exploredPath.push_back(adjacentSegments[i]);
            
            // The node is reachable and need to be explored.
            if (!segmentInfo.oneWay || segmentInfo.from == wavePoint.idx) {
                
                // Preapare the structs for the new node;
                IntersectionIdx nextNode = -1;
                PathNode thisNode;

                thisNode.from = adjacentSegments[i];
                
                // Calculate distance for the next intersection and store it in the lastIntersection property.
                if (segmentInfo.from == wavePoint.idx) {
                    nextNode = segmentInfo.to;
                    if (segmentInfo.from < allIntersections.size()) thisNode.lastIntersection = segmentInfo.from;
                } else {
                    nextNode = segmentInfo.from;
                    if (segmentInfo.to < allIntersections.size()) thisNode.lastIntersection = segmentInfo.to;
                }
                
                // Calculate distance for the next intersection to destination.
                thisNode.distance = findDistanceBetweenIntersections(nextNode, intersect_id_destination);
                
                // calculate the travel time for the current segment according to the travel time stored 
                // in the last intersection.
                if (thisNode.lastIntersection >= 0) {
                    PathNode lastIntersect = allIntersections[thisNode.lastIntersection];

                    if (lastIntersect.from >= 0) {
                        thisNode.travelTime = lastIntersect.travelTime + 
                                findStreetSegmentTravelTime(adjacentSegments[i]);
                        
                        if (turn_penalty != 0) {
                            StreetSegmentInfo segTemp = getStreetSegmentInfo(lastIntersect.from);

                            // consider the turn penalty
                            if (segTemp.streetID != segmentInfo.streetID) {
                                thisNode.travelTime += turn_penalty;
                            }
                        }
                    } else {
                        thisNode.travelTime = findStreetSegmentTravelTime(adjacentSegments[i]);
                    }
                }

                // If the total travel time of the current route can still be less than the fasted path we found,
                if ((thisNode.travelTime + thisNode.distance/averageSpeed) < timeToReach) {
                
                    // If one of the intersection of the segment is the destination, we found the path.
                    if (segmentInfo.to == intersect_id_destination || segmentInfo.from == intersect_id_destination) {
                        pathFound = true;

                        // If the total travel time is shorter than update the last node the travel time.
                        if (allIntersections[intersect_id_destination].travelTime < 0 
                                || allIntersections[intersect_id_destination].travelTime > thisNode.travelTime) {
                            allIntersections[intersect_id_destination] = thisNode;
                            timeToReach = thisNode.travelTime;
                        }
                    } else {
                        // If the intersection has not been reached before or the last travel time took longer than the current calculation.
                        if (allIntersections[nextNode].lastIntersection == -1 
                               || allIntersections[nextNode].travelTime > thisNode.travelTime
                           ) {
                            allIntersections[nextNode] = thisNode;
                            wavePoints.push(WavePoint(nextNode, thisNode.distance / speedSortingWavefront 
                                    + thisNode.travelTime));
                        }
                    }
                }
            }
        }
    }

    if (pathFound) {
        // get the path founded from allIntersections.
        PathNode pathNode = allIntersections[intersect_id_destination];
        
        while (pathNode.from != -1 ) {
            route.insert(route.begin(), pathNode.from);
            pathNode = allIntersections[pathNode.lastIntersection];
        }      
    }
    return route;
}

double computePathTravelTime(const std::vector<StreetSegmentIdx>& path,const double turn_penalty) {
    StreetIdx previousStreet = -1;
    
    double travelTimeTotal = 0;
    for (int i = 0; i < path.size(); i ++) {
        StreetSegmentInfo currentStreet = getStreetSegmentInfo(path[i]);
        travelTimeTotal += findStreetSegmentTravelTime(path[i]);
        
        // If street ID changes, turn penalty need to be considered
        if (currentStreet.streetID != previousStreet && previousStreet >= 0) {
            travelTimeTotal += turn_penalty;
        }

        previousStreet = currentStreet.streetID;
    }
    
    return travelTimeTotal;
}

double findDistanceBetweenIntersections(IntersectionIdx from, IntersectionIdx to) {
    std::pair<LatLon, LatLon> points;
    points.first = getIntersectionPosition(from);
    points.second = getIntersectionPosition(to);

    return findDistanceBetweenTwoPoints(points);;
}


