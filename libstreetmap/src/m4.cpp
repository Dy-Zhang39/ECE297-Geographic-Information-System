/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <m1.h>
#include <m3.h>
#include <m4.h>
#include <thread>
#include <queue>
#include <vector>
#include "global.h"
#include "dataHandler.h"

#define ILLEAGAL 0
#define LEAGAL 1
#define VISTED 2


CalculateResult calculate(double bestTime, std::vector <IntersectionIdx> result, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, double turn_penalty, int depotIdx, double randomLimit);

CalculateResult perturbation(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <IntersectionIdx>, double turn_penalty, int intervals);

std::vector <double> multidestDijkstra(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty);
std::vector <WavePoint> multidestDijkstraOpt(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind);
std::vector <int> initializeLegalIds(int deliverSize, int depotSize);
bool isAllDelivered(std::vector <int> legalIds, int deliverySize);
std::vector<CourierSubPath> travelingCourier0(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) ;



std::vector<CourierSubPath> travelingCourier(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) {

    std::clock_t begin = clock();
    std::vector <IntersectionIdx> result;
//    double firstCalculationBudget = 20;
    
    //int loopDepotsNum = 2;
    //if (depots.size() < loopDepotsNum) {
    //    loopDepotsNum = depots.size();
    //}
    

    // Initialize variables
    std::vector <IntersectionIdx> ids;
    std::vector <int> legalIds;
    
    
    for (int i = 0; i < deliveries.size(); i++) {
        ids.push_back(deliveries[i].pickUp);
        ids.push_back(deliveries[i].dropOff);
    }
    
    for (int i = 0; i < depots.size(); i++) {
        ids.push_back(depots[i]);
    }

    // Pre calculate all the costs
    /*std::vector <std::vector <double>> preCalculate;
    preCalculate.resize(ids.size());
    for (int i = 0; i < ids.size(); i++) {
        //std::cout << "i=" << ids.size() << "\n";
        std::clock_t beginCost = clock();
        std::vector <double> costs = multidestDijkstra(ids[i], ids, turn_penalty);
        std::clock_t endCost = clock();
        double totalPrecalTime = double(endCost - beginCost) / CLOCKS_PER_SEC;

        std::cout << "i = " << i << "  cpu time: " << totalPrecalTime << "\n";
        preCalculate.push_back(costs);
        
    }*/
    // Find the best CourierSubPath
    double currentBestTime = 9999999999999;
//    double minTimeDepot = 99999999;
    //int firstNode;// = -1;
    
    // First the shortest depot to pickup for first node;
    /*for (int i = 0; i < depots.size(); i++) {

        std::vector <IntersectionIdx> targets;
        
        for (int j = 0; j < deliveries.size(); j++) {
            targets.push_back(deliveries[j].pickUp);
        }

        std::vector <WavePoint> rst = multidestDijkstraOpt(depots[i], targets, turn_penalty, 2);
        
        if (rst.size() > 0) {
            if (minTimeDepot > rst[0].heuristicTime) {
                minTimeDepot = rst[0].heuristicTime;
                firstNode = i;
            }
        }
    }

    loopDepotsNum = firstNode + 1; //depots.size();
    bool loopAll = false;*/
    CalculateResult cResult;// = calculate(currentBestTime, result, deliveries, depots, ids, turn_penalty, firstNode, 1.0);
    /*currentBestTime = cResult.bestTime;
    result = cResult.result;
    
    if (cResult.cpuTime * depots.size() < firstCalculationBudget && depots.size() > 1) loopAll = true;
    
    if (loopAll){*/
    
    //Loop through all depots using greedy algorithm of finding shortest next path
    //#pragma omp parallel for
    for (int i = 0; i < depots.size(); i++) {
        std::cout<<"**************************\n";
        CalculateResult calcResult =
                //if (i != firstNode) {
                calculate(currentBestTime, result, deliveries, depots, ids, turn_penalty, i, 1.0);
        //#pragma omp critical
        {
            currentBestTime = calcResult.bestTime;
            result = calcResult.result;

        }
    }
    //} 
    
    //Have a 10% to 40% chance of taking the second smallest travel time
    std::cout << "Current best time: " << currentBestTime << "\n";
    /*#pragma omp parallel for
    for (int k = 0; k < 4; k++) {    
        cResult = calculate(currentBestTime, result, deliveries, depots, ids, turn_penalty, firstNode, 0.9 - 0.1 * k);
        
        #pragma omp critical
        {
            currentBestTime = cResult.bestTime;
            result = cResult.result;
        }
        
    }*/
    
    bool continueOpt = true;
    
    //perturbation (1-opt)
    for (int k = 0; k < 3; k ++) {
        if (continueOpt) {
            cResult = perturbation(currentBestTime, result, deliveries, depots, turn_penalty, 0);
            //std::cout << "Current pertubation + 0 iteration: " << k << "  Current cpu time: " << cResult.cpuTime << " CurrentBestTime: " << cResult.bestTime << "\n";
            std::clock_t current = clock();

            if (double(current - begin) / CLOCKS_PER_SEC > 30) continueOpt = true;

            if (currentBestTime > cResult.bestTime) {
                currentBestTime = cResult.bestTime;
                result = cResult.result;
            } else {
                break;
            }
        
        } else { 
            break;
        }
    }
    
    //perturbation (2-opt)
    for (int k = 0; k < 2; k ++) {
        if (continueOpt) {
            cResult = perturbation(currentBestTime, result, deliveries, depots, turn_penalty, 1);
            //std::cout << "Current pertubation + 1 iteration: " << k << "  Current cpu time: " << cResult.cpuTime << " CurrentBestTime: " << cResult.bestTime << "\n";

            if (currentBestTime > cResult.bestTime) {
                currentBestTime = cResult.bestTime;
                result = cResult.result;

                std::clock_t current = clock();

                if (double(current - begin) / CLOCKS_PER_SEC + cResult.cpuTime * 2 > 40) {
                    continueOpt = false;
                    break;
                }
            } else {
                break;
            }
        }
    }

    // Form the result;
    std::vector <CourierSubPath> courierPath;
    double totalCourierTime = 0;
    
    if (result.size() > 1) {
        
        for (IntersectionIdx idx = 1; idx < result.size(); idx++) {
            CourierSubPath pathWay;
            pathWay.start_intersection = result[idx - 1];
            pathWay.end_intersection = result[idx];

            pathWay.subpath = findPathBetweenIntersections(pathWay.start_intersection, pathWay.end_intersection, turn_penalty);

            courierPath.push_back(pathWay);
            
            //std::cout << "Final Path --- from: " << result[idx -1] << " to: " << result[idx] << std::endl;
            totalCourierTime += computePathTravelTime(pathWay.subpath, turn_penalty);
        }
    }
    
    std::cout << "From: " << depots[0] << " ---> ";
    std::clock_t end = clock();
    double totalTime = double(end - begin) / CLOCKS_PER_SEC;
    std::cout<<"total cpu time: " << totalTime << " Total Travel Time: " << totalCourierTime << "  Estimated: " << currentBestTime << "\n";
    return courierPath;
    
} 

CalculateResult perturbation(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, double turn_penalty, int intervals) {
    std::clock_t begin = clock();
    
    //Check the swap validation
    if (deliveries.size() * 2 > intervals + 3) {
        for (int i = 2; i < result.size() - 1 - intervals; i++) {
            bool valid = true;
            for (int j = 0; j < deliveries.size(); j++) {
                for (int k0 = i -1 ; k0 < i + intervals; k0++) {
                    if (deliveries[j].pickUp == result[k0] ) {
                        for (int k = k0 + 1; k < i + intervals + 1; k++) {

                            //If the dropOff will appear before the current corresponding pickup after swapping, the swap is invalid
                            if (deliveries[j].dropOff == result[k]) {
                                valid = false;
                            }
                        }  
                    }
                }
            }

            //If the swap is valid
            if (valid) {
                double originalTime = 0;
                double newTime = 0;

                for (int j = i - 2; j < i + intervals + 1; j ++) {
                    originalTime += computePathTravelTime(findPathBetweenIntersections(result[j], result[j + 1], turn_penalty), turn_penalty);
                    
                    int k = j;
                    int k0 = j + 1;

                    if (k == i - 1) k = i + intervals;
                    else if (k == i + intervals) k = i - 1;
                    if (k0 == i - 1) {
                        k0 = i + intervals;
                    } else if (k0 == i + intervals) {
                        k0 = i -1;
                    }
                    //std::cout << result[k] << " to " << result[k0] << "; ";
                    newTime += computePathTravelTime(findPathBetweenIntersections(result[k], result[k0], turn_penalty), turn_penalty);
                }
                
                //std::cout <<std::endl;
                
                //Update the result and bestTime if the newTime is shorter
                if (originalTime > newTime) {
                    int tmp = result[i - 1];
                    result[i -1] = result[i + intervals];
                    result[i + intervals] = tmp;
                    bestTime = bestTime - originalTime + newTime;
                }
            }
        }

        //Find the depot with the least travel time from the last drop off point
        double originalTime = computePathTravelTime(findPathBetweenIntersections(result[result.size() - 2], result[result.size() - 1], turn_penalty), turn_penalty);
        std::vector <WavePoint> rst = multidestDijkstraOpt(result[result.size() - 2], depots, turn_penalty, 2);

        if (rst[0].heuristicTime < originalTime) {
            result[result.size() -1] = rst[0].idx;
            bestTime = bestTime - originalTime + rst[0].heuristicTime;
        }
    }
    
    CalculateResult cResult;
    cResult.bestTime = bestTime;
    cResult.result = result;
    std::clock_t end = clock();
    cResult.cpuTime = double(end - begin) / CLOCKS_PER_SEC;
    return cResult;
}

//Calculate the best path to deliver all packages
CalculateResult calculate(double bestTime, std::vector <IntersectionIdx> result, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, double turn_penalty, int depotIdx, double randomLimit) {
    
    
    std::clock_t begin = clock();
    std::vector <IntersectionIdx> targetsPickup;
    int nextIdx = -1;

    for (int j = 0; j < deliveries.size(); j++) {
        targetsPickup.push_back(deliveries[j].pickUp);
    }

    //std::vector <WavePoint> rstPickup;
    
    
    //rstPickup = multidestDijkstraOpt(depots[depotIdx], targetsPickup, turn_penalty, 2);
    double minTime = 999999999;
    int currentNode = deliveries.size() * 2 + depotIdx;
    std::vector <int> legalIds = initializeLegalIds(deliveries.size(), depots.size());
    bool allDelivered = isAllDelivered(legalIds, deliveries.size());
    double travelTimeTotal = 0;
    bool canFind = true;
    std::vector <IntersectionIdx> resultTemp;
    resultTemp.push_back(ids[currentNode]);
    while (!allDelivered && canFind) {
        minTime = 999999999;
        std::cout << resultTemp.size() << "  total Travel: " << travelTimeTotal << " --   Next ID: " << nextIdx << " Intersection: " << ids[nextIdx] << "\n";
        nextIdx = -1;
        std::vector <IntersectionIdx> targets;
        std::vector <int> positions;
        
        for (int j = 0; j < deliveries.size() * 2; j++) {
            
            if (legalIds[j] == 1) {         //If this intersection is legal (represented by 1)
                targets.push_back(ids[j]);
                positions.push_back(j);
            }
        }
        std::cout<<"Current Node: "<<ids[currentNode] << "\n" << ids.size() << ".\n";
        std::vector <WavePoint> rst = multidestDijkstraOpt(ids[currentNode], targets, turn_penalty, 2);

        //Choose the best/second best result based on the possibility provided
        if (rand() % 1000 > randomLimit * 1000 && rst.size() > 1) {
            minTime = rst[1].heuristicTime;
            nextIdx = positions[rst[1].idx];
        } else {
            minTime = rst[0].heuristicTime;
            nextIdx = positions[rst[0].idx];
        }
        std::cout<<"...............";
        //If the next valid ID is found
        if (nextIdx >= 0) {
            travelTimeTotal += minTime;

            resultTemp.push_back(ids[nextIdx]);

            //Update legality status according to type of site reached
            if (nextIdx % 2 == 0) {         //a pickup site
                legalIds[nextIdx + 1] = 1;
                legalIds[nextIdx] = 2;
            } else {                        //drop off site
                legalIds[nextIdx] = 2;
            }

            currentNode = nextIdx;
            allDelivered = isAllDelivered(legalIds, deliveries.size());
        } else {
            canFind = false;
        }
    }


    if (canFind) {      //If the route can be found and all packages are delivered
        minTime = 9999999999999;
        IntersectionIdx lastIntersection = 0;

        //Find the depot with the least travel time
        std::vector <WavePoint> rst = multidestDijkstraOpt(ids[currentNode], depots, turn_penalty, 2);

        if (rst.size() > 0) {
            minTime = rst[0].heuristicTime;
            lastIntersection = depots[rst[0].idx];
        }

        travelTimeTotal += minTime;
        resultTemp.push_back(lastIntersection);
        if (bestTime > travelTimeTotal) {
            bestTime = travelTimeTotal;
            result = resultTemp;
        }

    }

    CalculateResult cResult;
    cResult.bestTime = bestTime;
    cResult.result = result;
    std::clock_t end = clock();
    cResult.cpuTime = double(end - begin) / CLOCKS_PER_SEC;
    return cResult;
}

// Check whether all has been delivered already or not.
bool isAllDelivered(std::vector <int> legalIds, int deliverySize) {
                            
    bool allDelivered = true;
    for (int k = 0; k < deliverySize; k++) {
        if (legalIds[k * 2] != VISTED || legalIds[k*2 + 1] != VISTED) {
            allDelivered = false;
            break;
        }
    }

    return allDelivered;
}

// Initialize legalIds
//1 represents a legal next point, 2 represents an already reached one, 0 represents an illegal one
std::vector <int> initializeLegalIds(int deliverSize, int depotSize) {
    std::vector <int> legalIds;
    legalIds.clear();
    
    for (int i = 0; i < deliverSize; i++) {
        legalIds.push_back(LEGAL);
        legalIds.push_back(ILLEGAL);
    }
    
    for (int i = 0; i < depotSize; i++) {
        legalIds.push_back(ILLEGAL);
    }
    
    return legalIds;
}

//Multi-Dijkstra to find a best path to a list of destination
//numToFind means how many the first number of shortest paths to be found
std::vector <WavePoint> multidestDijkstraOpt(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind) {
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > wavePoints;
    std::vector <PathNode> allIntersections;
    
    std::vector <bool> pathFound;
    std::vector <double> timeToReach;
    int totalIntersections = getNumIntersections();
    timeToReach.clear();

    // initialize allIntersections vector
    allIntersections.resize(totalIntersections);
    
    for (int i = 0; i < dest.size(); i ++) {
        pathFound.push_back(false);
        timeToReach.push_back(9999999999);
    }
    
    for (int i = 0; i < allIntersections.size(); i ++) {
        allIntersections[i].lastIntersection = -1;
        allIntersections[i].travelTime = -1;
    }
    
    // Prepare the variables for the start optDistanceinterseoptDistancection.
    PathNode node;
    
    node.from = -1;
    node.travelTime = 0;
    node.distance = 0;
    node.lastIntersection = -2;
    
    // If any destination is the same as from point, mark this as reached already
    for (int j = 0; j < dest.size(); j++) {
        if (intersect_id_start == dest[j] || intersect_id_start == dest[j]) {
            pathFound[j] = true;
            timeToReach[j] = 0;
        }
    }

    if (intersect_id_start < allIntersections.size()) 
        allIntersections[intersect_id_start] = node;

    // Get the first wavePoint (waveFront)
    wavePoints.push(WavePoint(intersect_id_start, 0));
    bool allDestFound = false;
    
    while (!wavePoints.empty() && !allDestFound) {

        //Find next nodes to explore and remove it from wavePoints
        WavePoint wavePoint = wavePoints.top();
        wavePoints.pop();
        
        // Find adjacent Intersections.
        std::vector<StreetSegmentIdx> adjacentSegments = findStreetSegmentsOfIntersection(wavePoint.idx);
        
        for (int i = 0; i < adjacentSegments.size(); i++) {

            StreetSegmentInfo segmentInfo = getStreetSegmentInfo(adjacentSegments[i]);
            //this line is use for performance tuning, and to show all the path explored.
            
            // The node is reachable and need to be explored.
            if (!segmentInfo.oneWay || segmentInfo.from == wavePoint.idx) {
                
                // Preapare the structs for the new node;
                IntersectionIdx nextNode = -1;
                PathNode thisNode;

                thisNode.from = adjacentSegments[i];
                
                // Calculate distance for the next intersection and store it in the lastIntersection property.
                if (segmentInfo.from == wavePoint.idx) {
                    nextNode = segmentInfo.to;
                    if (segmentInfo.from < allIntersections.size()) 
                        thisNode.lastIntersection = segmentInfo.from;
                } else {
                    nextNode = segmentInfo.from;
                    if (segmentInfo.to < allIntersections.size()) 
                        thisNode.lastIntersection = segmentInfo.to;
                }
                
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

                int totalFoundBest = 0;

                for (int j = 0; j < dest.size(); j++) {

                    if (thisNode.travelTime > timeToReach[j]) {
                        totalFoundBest ++;
                    } else { 

                        if (segmentInfo.to == dest[j] || segmentInfo.from == dest[j]) {
                            pathFound[j] = true;
                            if (timeToReach[j] > thisNode.travelTime) 
                                timeToReach[j] = thisNode.travelTime;
                        }
                    }                  
                }
                
                if (totalFoundBest < numToFind) {
                    // If the intersection has not been reached before or 
                    //the last travel time took longer than the current calculation.
                    if (allIntersections[nextNode].lastIntersection == -1 
                           || allIntersections[nextNode].travelTime > thisNode.travelTime
                       ) {
                        allIntersections[nextNode] = thisNode;
                        wavePoints.push(WavePoint(nextNode, thisNode.travelTime)); 
                    }
                }
            }
        }
    }
    
    // Sort the output
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > result;
    for (int i = 0; i < pathFound.size(); i++) {
        std::cout<< i<<"---------9999999---------------999999-----99999999999\n";
        if (pathFound[i]) {
            std::cout<<"9999999---------------99999999999999999\n";
            result.push(WavePoint(i, timeToReach[i]));
        }
    }
    
    std::vector<WavePoint> results;

    while(!result.empty()) {
        results.push_back(result.top());
        result.pop();
    }
    
    return results;
}




/*
 * 
 * Old way of pre computing. Having time constraints with 4 test cases not pass.
 * 
 * 
 */


std::vector<CourierSubPath> travelingCourier0(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) {

    std::clock_t begin = clock();
    std::vector <IntersectionIdx> result;
    

    // Pre-calculate the shortest path travel time
    std::vector <IntersectionIdx> ids;
    std::vector <int> legalIds;
    
    for (int i = 0; i < deliveries.size(); i++) {
        ids.push_back(deliveries[i].pickUp);
        ids.push_back(deliveries[i].dropOff);
    }
    
    for (int i = 0; i < depots.size(); i++) {
        ids.push_back(depots[i]);
    }

    std::vector <std::vector <double>> preCalculate;
    std::cout << deliveries.size() << ", " << depots.size() << "\n";
    for (int i = 0; i < ids.size(); i++) {
        //std::cout << "i=" << ids.size() << "\n";
        std::clock_t beginCost = clock();
        std::vector <double> costs = multidestDijkstra(ids[i], ids, turn_penalty);
        std::clock_t endCost = clock();
        double totalPrecalTime = double(endCost - beginCost) / CLOCKS_PER_SEC;

        std::cout << "i = " << i << "  cpu time: " << totalPrecalTime << "\n";
        preCalculate.push_back(costs);
        
    }
    std::clock_t endPrecal = clock();
    double totalPrecalTime = double(endPrecal - begin) / CLOCKS_PER_SEC;

    std::cout << "cpu time: " << totalPrecalTime << "           Pre-calculation compelted\n";
    // Find the best CourierSubPath
    double currentBestTime = 9999999999999;

    double minTime = 999999999;
    int currentNode = deliveries.size() * 2 + 1;
    legalIds = initializeLegalIds(deliveries.size(), depots.size());
    bool allDelivered = isAllDelivered(legalIds, deliveries.size());
    double travelTimeTotal = 0;
    bool canFind = true;
    int nextIdx = -1;
    std::vector <IntersectionIdx> resultTemp;

    for (int i = 0; i < depots.size(); i++) {
        //std::cout << "Testing Depot " << i << " Intersection: " << depots[i] << "\n";
        //legalIds[deliveries.size() * 2 + i] = 2;
        int currentNodeTemp = deliveries.size() * 2 + i;
        
        //std::cout << " --   Next ID: " << nextIdx << " Intersection: " << ids[nextIdx] << "\n";

        for (int j = 0; j < deliveries.size(); j++) {
            if (preCalculate[currentNodeTemp][j * 2] < minTime) {
                minTime = preCalculate[currentNodeTemp][j];
                currentNode = currentNodeTemp;
                nextIdx = j * 2;
            }
        }
    }
    
            
    if (nextIdx >= 0) {
        travelTimeTotal += minTime;
        resultTemp.push_back(ids[currentNode]);
        resultTemp.push_back(ids[nextIdx]);

        legalIds[nextIdx + 1] = 1;
        legalIds[nextIdx] = 2;

        currentNode = nextIdx;
        allDelivered = isAllDelivered(legalIds, deliveries.size());
    }

    while (!allDelivered && canFind) {
        minTime = 999999999;
        //std::cout << " --   Next ID: " << nextIdx << " Intersection: " << ids[nextIdx] << "\n";
        nextIdx = -1;


        for (int j = 0; j < legalIds.size(); j++) {
            //if (legalIds[j] == 1) std::cout << " -----  J = " << j << "  preCalculate[" << currentNode << "][" << j << "] = " << preCalculate[currentNode][j] << "  minTime: " << minTime << "\n";
            if (legalIds[j] == 1 && preCalculate[currentNode][j] < minTime) {
                minTime = preCalculate[currentNode][j];
                nextIdx = j;
            }
        }
        //std::cout << " ----->  Found Next ID: " << nextIdx << " Intersection: " << ids[nextIdx] << " minTime: " << minTime << "\n";

        if (nextIdx >= 0) {
            travelTimeTotal += minTime;

            resultTemp.push_back(ids[nextIdx]);

            // If the point reached is a pick up site
            if (nextIdx % 2 == 0) {
                legalIds[nextIdx + 1] = 1;
                legalIds[nextIdx] = 2;
            } else {
                legalIds[nextIdx] = 2;
            }

            currentNode = nextIdx;
            allDelivered = isAllDelivered(legalIds, deliveries.size());
        } else {
            canFind = false;
        }
    }
        
        
    if (canFind) {
        //std::cout << "I found it\n";
        minTime = 999999999;
        IntersectionIdx lastIntersection = 0;

        for (int j = 0; j < depots.size() ; j++) {
            if (preCalculate[currentNode][deliveries.size() * 2 + j] < minTime) {
                minTime = preCalculate[currentNode][deliveries.size() * 2 + j];
                lastIntersection = depots[j];
            }

            travelTimeTotal += minTime;
        }

        travelTimeTotal += minTime;
        resultTemp.push_back(lastIntersection);

        if (travelTimeTotal < currentBestTime) {
            result = resultTemp;
            currentBestTime = travelTimeTotal;
        }
    }

    // Form the result;
    std::vector <CourierSubPath> courierPath;
    double totalCourierTime = 0;

    if (result.size() > 1) {
        
        for (IntersectionIdx idx = 1; idx < result.size(); idx++) {
            CourierSubPath pathWay;
            pathWay.start_intersection = result[idx - 1];
            pathWay.end_intersection = result[idx];

            pathWay.subpath = findPathBetweenIntersections(pathWay.start_intersection, pathWay.end_intersection, turn_penalty);

            courierPath.push_back(pathWay);
            
            //std::cout << "Final Path --- from: " << result[idx -1] << " to: " << result[idx] << std::endl;
            totalCourierTime += computePathTravelTime(pathWay.subpath, turn_penalty);
        }
    }
    
    std::cout << "From: " << depots[0] << " ---> ";
    std::clock_t end = clock();
    double totalTime = double(end - begin) / CLOCKS_PER_SEC;
    std::cout<<"total cpu time: " << totalTime << " Total Travel Time: " << totalCourierTime << "\n";
    return courierPath;
    
} 


std::vector <double> multidestDijkstra(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty) {
    //double averageSpeed = 30;
    //double speedSortingWavefront = 30;
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > wavePoints;
    //std::vector <IntersectionIdx> waveFront;
    std::vector <PathNode> allIntersections;
    
    std::vector <bool> pathFound;
    std::vector <double> timeToReach;
    int totalIntersections = getNumIntersections();
    timeToReach.clear();

    // initialize allIntersections vector
    allIntersections.resize(totalIntersections);
    
    for (int i = 0; i < dest.size(); i ++) {
        pathFound.push_back(false);
        timeToReach.push_back(9999999999);
    }
    
    for (int i = 0; i < allIntersections.size(); i ++) {
        allIntersections[i].lastIntersection = -1;
        allIntersections[i].travelTime = -1;
    }
    
    // Prepare the variables for the start optDistanceinterseoptDistancection.
    PathNode node;
    
    node.from = -1;
    node.travelTime = 0;
    node.distance = 0;
    node.lastIntersection = -2;

    allIntersections[intersect_id_start] = node;

    // Get the first wavePoint (waveFront)
    wavePoints.push(WavePoint(intersect_id_start, 0));
    bool allDestFound = false;
    
    while (!wavePoints.empty() && !allDestFound) {

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
                    //if (segmentInfo.from < allIntersections.size()) 
                        thisNode.lastIntersection = segmentInfo.from;
                } else {
                    nextNode = segmentInfo.from;
                    //if (segmentInfo.to < allIntersections.size()) 
                        thisNode.lastIntersection = segmentInfo.to;
                }
                
                // Calculate distance for the next intersection to destination.
                //thisNode.distance = findDistanceBetweenIntersections(nextNode, intersect_id_destination);
                
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

                int totalFoundBest = 0;
                //double optDistance = 0;

                for (int j = 0; j < dest.size(); j++) {

                    if (thisNode.travelTime > timeToReach[j]) {
                        totalFoundBest ++;
                    } else { 

                        if (segmentInfo.to == dest[j] || segmentInfo.from == dest[j]) {
                            pathFound[j] = true;
                            //if (timeToReach[j] > thisNode.travelTime) 
                            timeToReach[j] = thisNode.travelTime;
                        }
                    }                  
                }
                
                if (totalFoundBest < dest.size()) {
                    // If the intersection has not been reached before or 
                    //the last travel time took longer than the current calculation.
                    if (allIntersections[nextNode].lastIntersection == -1 
                           //|| allIntersections[nextNode].travelTime > thisNode.travelTime
                       ) {
                        allIntersections[nextNode] = thisNode;
                        wavePoints.push(WavePoint(nextNode, thisNode.travelTime)); 
                    }
                } else {
                    allDestFound = true;
                }
            }
        }
    }
    
    return timeToReach;
}

