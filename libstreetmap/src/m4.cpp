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
struct PreCalResult {
    std::vector<WavePoint> result;
    std::vector<WavePoint> resultOrignalOrder;
};
/*
 * Calculate the full travel sequence assuming starting from one of the depots
 * @param bestTime: current best time to pickup and drop off all packages before running this function
 * @param result: current best travel sequence before running this function
 * @param deliveries: a list deliveries to pickup and drop off
 * @param depots: a list of depots given
 * @param ids: a vector of all deliveries and depots intersections in the order of 
 *        pickUp1-dropOff1-pickUp2-dropOff2-...-pickUp[n]-dropOff[n]-depot1-...-depot[n]
 *        The purpose of this vector is to quickly locate the intersection ID and validate it
 * @param turn_penalty: turn penalty in seconds
 * @param depotId: the number of depot IDs according to the sequence of the depots vector provided
 * @param randomLimit: used to generate random number 
 * (e.g. 1.0 means always choose the best next node, 0.9 means 10% of the chance to choose the second best next nod)
 */
CalculateResult calculate(double bestTime, std::vector <IntersectionIdx> result, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, double turn_penalty, int depotIdx, double randomLimit);

/*
 * Perturbation to improve the current solution
 * @param bestTime: current best time to pickup and drop off all packages before running this function
 * @param result: current best travel sequence before running this function
 * @param deliveries: a list deliveries to pickup and drop off
 * @param turn_penalty: turn penalty in seconds
 * @param intervals: the interval between the position of the two nodes to be swapped
 *        (e.g. to swap result[0] and result[2], interval is 2)
 */
CalculateResult perturbation(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <IntersectionIdx>, double turn_penalty, int intervals);

/*
 * Search for the best path to multiple destinations using Dijkstra Algorithm
 * @param intersect_id_start: starting point
 * @param dest: a list of destinations
 * @param turn_penalty: turn penalty in seconds
 * @param numToFind: the number of best paths to be found
 *        (e.g., if numToFind = 2, the best path of two destinations that takes the shortest time will be found; 
 *         if numToFind = dest.size(), best paths for all destinations will be returned)
 */
std::vector <WavePoint> multidestDijkstraOpt(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind);

/*
 * Initializes the vector of legal ids.  This vector is used to determine whether an intersection is legal for travel
 * For each individual value, 0 means this intersection is illegal, 1 means it is legal, 2 means it has been traveled to
 * @param deliverSize: the size of all deliveries (pickUp and dropOff)
 * @param depotSize: the number of depots available
 */
std::vector <int> initializeLegalIds(int deliverSize, int depotSize);

/*
 * Determine if all delivery has been picked up and delivered
 * @param legalIds: the vector that stores the legality information
 * @param depotSize: the number of depots available
 */
bool isAllDelivered(std::vector <int> legalIds, int deliverySize);

CalculateResult calculatePreload(double bestTime, std::vector <IntersectionIdx> result, std::vector <int> resultIndex,
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, int depotId, std::vector<std::vector<WavePoint>> preCalculate, double randomLimit);
CalculateResult perturbationPrecalculated(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <int> resultIndex, 
        int intervals, std::vector<std::vector<WavePoint>> preCalculate, std::vector <IntersectionIdx> ids);
PreCalResult multidestDijkstra(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty,  int numToFind);

std::vector<CourierSubPath> travelingCourier(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) {

    std::clock_t begin = clock();
    std::vector <IntersectionIdx> result;       //the vector to store the current best travel sequence
    std::vector <IntersectionIdx> ids;          //the vector of all deliveries and depots intersections in the order initialized  below
    std::vector <int> legalIds;                 //the vector that is used to determine whether an intersection is legal for travel
    
    //Initialize the ids vector
    for (int i = 0; i < deliveries.size(); i++) {
        ids.push_back(deliveries[i].pickUp);
        ids.push_back(deliveries[i].dropOff);
    }
    
    for (int i = 0; i < depots.size(); i++) {
        ids.push_back(depots[i]);
    }

    
    // Find the best CourierSubPath
    double currentBestTime = 9999999999999;     //the current best time will be updated if a better solution is found
    //---------------------------------------------------PRECALC------------------------------------------------------------
    // Pre calculate all the costs
    std::vector <std::vector <WavePoint>> preCalculate;
    std::vector <std::vector <WavePoint>> preCalculateOrigOrder;
    preCalculate.resize(ids.size());
    preCalculateOrigOrder.resize(ids.size());
    
    #pragma omp parallel for
    for (int i = 0; i < ids.size(); i++) {
        PreCalResult costs = multidestDijkstra(ids[i], ids, turn_penalty, ids.size());
        preCalculate[i] = costs.result;
        preCalculateOrigOrder[i] = costs.resultOrignalOrder;
    }
    
    CalculateResult cResult;// = calculate(currentBestTime, result, deliveries, depots, ids, turn_penalty, firstNode, 1.0);
    std::vector <int> resultIndex;

    //Loop through all depots using greedy algorithm of finding shortest next path
    //#pragma omp parallel for
    for (int i = 0; i < depots.size(); i++) {
        CalculateResult calcResult =
                calculatePreload(currentBestTime, result, resultIndex, deliveries, depots, ids, i, preCalculate, 1.0);
        if (calcResult.bestTime < currentBestTime) {
            currentBestTime = calcResult.bestTime;
            result = calcResult.result;
            resultIndex = calcResult.resultIdxIndex;
        }
    }
    
    IntersectionIdx firstNode = 0;
    for (int k = 0; k < depots.size(); k++) {
        if (depots[k] == result[0]) {
            firstNode = k;
        }
    }

    bool continueOpt = true;
    
    //perturbation (1-opt) for the current best solution
    for (int i = 0; i < deliveries.size() *2; i ++) {
        if (continueOpt) {
            for (int k = 0; k < 3; k ++) {
                if (continueOpt) {
                    cResult = 
                        perturbationPrecalculated(currentBestTime, result, deliveries, resultIndex, i, preCalculateOrigOrder, ids);
                    //std::cout << "Current pertubation + "<<i<<" iteration: " << k << "  Current cpu time: " << cResult.cpuTime << " CurrentBestTime: " << cResult.bestTime << "\n";
                    std::clock_t current = clock();

                    if (double(current - begin) / CLOCKS_PER_SEC < 45) {
                        continueOpt = true;
                    } else {
                        continueOpt = false;
                    }

                    if (currentBestTime > cResult.bestTime) {
                        currentBestTime = cResult.bestTime;
                        result = cResult.result;
                        resultIndex = cResult.resultIdxIndex;
                    } else {
                        break;
                    }

                } else { 
                    break;
                }
            }
        }
    }
    //Have a 10% to 40% chance of taking the second smallest travel time
    std::cout << "Current best time: " << currentBestTime << "\n";


    for (int randomK = 0; randomK < 100; randomK++) {    
        if (continueOpt) {
            cResult = 
                calculatePreload(currentBestTime, result, resultIndex, deliveries, depots, ids, firstNode, preCalculate, 0.9);
            
            double currentBestTimeTemp = cResult.bestTime;
            std::vector <IntersectionIdx>  resultTemp = cResult.result;
            std::vector <int> resultIndexTemp = cResult.resultIdxIndex;
            
            //User perturbation to fine tune the random solution even if it is not the current best
            //perturbation (1-opt)
            for (int i = 0; i < deliveries.size() *2; i ++) {
                if (continueOpt) {
                    for (int k = 0; k < 3; k ++) {
                        if (continueOpt) {
                            cResult = 
                                perturbationPrecalculated(currentBestTimeTemp, resultTemp, deliveries, resultIndexTemp, i, preCalculateOrigOrder, ids);
                            //std::cout << "Current pertubation + "<<i<<" iteration: " << k << "  Current cpu time: " << cResult.cpuTime << " CurrentBestTime: " << cResult.bestTime << "\n";
                            std::clock_t current = clock();

                            if (double(current - begin) / CLOCKS_PER_SEC < 45) {
                                continueOpt = true;
                            } else {
                                continueOpt = false;
                            }

                            if (currentBestTimeTemp > cResult.bestTime) {
                                currentBestTimeTemp = cResult.bestTime;
                                resultTemp = cResult.result;
                                resultIndexTemp = cResult.resultIdxIndex;
                            } else {
                                break;
                            }

                        } else { 
                            break;
                        }
                    }
                }
            }

            if (currentBestTimeTemp < currentBestTime) {
                currentBestTime = currentBestTimeTemp;
                result = resultTemp;
                resultIndex = resultIndexTemp;
            }
        }
    }
    
    std::clock_t currentFin = clock();
    std::cout << "Next best time: " << currentBestTime << "    Time elapsed: " << double(currentFin - begin) / CLOCKS_PER_SEC <<  "\n";
    
    
    //-------------------------------------------   end of precalc -----------------------------------------------------
    //----------------------------------------BELOW IS WITHOUT PRECALCULATION--------------------------------------------
    /*
    //Loop through all depots using greedy algorithm of finding shortest next path
    #pragma omp parallel for
    for (int i = 0; i < depots.size(); i++) {
        CalculateResult calcResult =  calculate(currentBestTime, result, deliveries, depots, ids, turn_penalty, i, 1.0);
        #pragma omp critical
        {
            currentBestTime = calcResult.bestTime;
            result = calcResult.result;

        }
    }
    
    std::cout << "Current best time: " << currentBestTime << "\n";
    
    bool continueOpt = true;                    //bool to determine whether we should continue optimizing using perturbation
    CalculateResult cResult;                    //the struct to store the solution optimized by perturbation
    
    //perturbation (1-opt)
    for (int k = 0; k < 3; k ++) {
        
        if (continueOpt) {
            cResult = perturbation(currentBestTime, result, deliveries, depots, turn_penalty, 0);
            std::clock_t current = clock();
            std::cout << "Opt 0" << k <<" cputime: " << cResult.cpuTime << " Estimated time: " << cResult.bestTime << "\n";
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
            std::cout << "Opt 1 " << k <<" cputime: " << cResult.cpuTime << " Estimated time: " << cResult.bestTime << "\n";

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

    //perturbation (2-opt with 2 intervals)
    for (int k = 0; k < 1; k ++) {
        if (continueOpt) {
            cResult = perturbation(currentBestTime, result, deliveries, depots, turn_penalty, 2);
            std::cout << "Opt 2" << k <<" cputime: " << cResult.cpuTime << " Estimated time: " << cResult.bestTime << "\n";

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
    */
    // Form the result;
    std::vector <CourierSubPath> courierPath;
    double totalCourierTime = 0;
    
    if (result.size() > 1) {        //if a solution exists
        
        for (IntersectionIdx idx = 1; idx < result.size(); idx++) {
            CourierSubPath pathWay;
            pathWay.start_intersection = result[idx - 1];
            pathWay.end_intersection = result[idx];
            pathWay.subpath = findPathBetweenIntersections(pathWay.start_intersection, pathWay.end_intersection, turn_penalty);

            courierPath.push_back(pathWay);           
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
                    
                    //Prepare the new order after swapping
                    int k = j;
                    int k0 = j + 1;

                    if (k == i - 1) k = i + intervals;
                    else if (k == i + intervals) k = i - 1;
                    if (k0 == i - 1) {
                        k0 = i + intervals;
                    } else if (k0 == i + intervals) {
                        k0 = i -1;
                    }

                    //According to the new order, calculate the new time
                    newTime += computePathTravelTime(findPathBetweenIntersections(result[k], result[k0], turn_penalty), turn_penalty);
                }
                
                //Update the result and bestTime if the newTime is shorter
                if (originalTime > newTime) {
                    int tmp = result[i - 1];
                    result[i -1] = result[i + intervals];
                    result[i + intervals] = tmp;
                    bestTime = bestTime - originalTime + newTime;
                }
            }
        }

        //Find the depot with the least travel time from the final drop off point
        double originalTime = computePathTravelTime(findPathBetweenIntersections(result[result.size() - 2], 
                result[result.size() - 1], turn_penalty), turn_penalty);
        std::vector <WavePoint> rst = multidestDijkstraOpt(result[result.size() - 2], depots, turn_penalty, 2);

        //Use the best result as the final depot to reach
        if (rst[0].heuristicTime < originalTime) {
            result[result.size() -1] = rst[0].idx;
            bestTime = bestTime - originalTime + rst[0].heuristicTime;
        }
    }
    
    //Finalize the result struct to return
    CalculateResult cResult;
    cResult.bestTime = bestTime;
    cResult.result = result;
    std::clock_t end = clock();
    cResult.cpuTime = double(end - begin) / CLOCKS_PER_SEC;     //cpu time spent on running this function
    return cResult;
}

//Calculate the best path to deliver all packages
CalculateResult calculate(double bestTime, std::vector <IntersectionIdx> result, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, double turn_penalty, int depotId, double randomLimit) {
    
    
    std::clock_t begin = clock();
    std::vector <IntersectionIdx> targetsPickup;        //vector for all available pickUp intersections
    int nextIdx = -1;

    for (int j = 0; j < deliveries.size(); j++) {
        targetsPickup.push_back(deliveries[j].pickUp);
    }


    int currentNode = deliveries.size() * 2 + depotId;  //convert the first depot into the index of ids

    std::vector <int> legalIds = initializeLegalIds(deliveries.size(), depots.size());
    bool allDelivered = isAllDelivered(legalIds, deliveries.size());
    double travelTimeTotal = 0;
    bool canFind = true;
    
    //Result vector of travel order
    std::vector <IntersectionIdx> resultTemp;
    resultTemp.push_back(ids[currentNode]);
    
    while (!allDelivered && canFind) {          //if delivery is not finished and the solution still can be found
        double minTime = 9999999999999;
        nextIdx = -1;
        std::vector <IntersectionIdx> targets;  //vector to store all legal next interest points
        std::vector <int> positions;            //vector to store the index of ids related targets
        
        for (int j = 0; j < deliveries.size() * 2; j++) {
            
            if (legalIds[j] == 1) {         //If this intersection is legal (represented by 1)
                targets.push_back(ids[j]);
                positions.push_back(j);
            }
        }

        //Get the two best next interest points along with the travel time
        std::vector <WavePoint> rst = multidestDijkstraOpt(ids[currentNode], targets, turn_penalty, 2);

        //Choose the best/second best result based on the possibility provided
        if (rst.size() > 0) {
            if (rand() % 1000 > randomLimit * 1000 && rst.size() > 1) {
                minTime = rst[1].heuristicTime;
                nextIdx = positions[rst[1].idx];
            } else {
                minTime = rst[0].heuristicTime;
                nextIdx = positions[rst[0].idx];
            }
            
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
        double minTime = 9999999999999;
        IntersectionIdx lastIntersection = 0;

        //Find the depot with the least travel time
        std::vector <WavePoint> rst = multidestDijkstraOpt(ids[currentNode], depots, turn_penalty, 2);

        //If an result depot is available, take the one with shortest travel time
        if (rst.size() > 0) {
            minTime = rst[0].heuristicTime;
            lastIntersection = depots[rst[0].idx];
        }

        travelTimeTotal += minTime;
        resultTemp.push_back(lastIntersection);
        
        //Update the result if the new one is better
        if (bestTime > travelTimeTotal) {
            bestTime = travelTimeTotal;
            result = resultTemp;
        }
    }
    
    //Prepare the result for return
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
std::vector <int> initializeLegalIds(int deliverSize, int depotSize) {
    std::vector <int> legalIds;
    legalIds.clear();
    
    for (int i = 0; i < deliverSize; i++) {
        legalIds.push_back(1);
        legalIds.push_back(0);
    }
    
    for (int i = 0; i < depotSize; i++) {
        legalIds.push_back(0);
    }
    
    return legalIds;
}

//Multi-Dijkstra to find a best path to a list of destination
std::vector <WavePoint> multidestDijkstraOpt(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind) {
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > wavePoints;
    
    std::vector <PathNode> allIntersections;    
    std::vector <bool> pathFound;
    std::vector <double> timeToReach;       //the time used to reach each destination
    int totalIntersections = getNumIntersections();

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

    //Give the pathNode value for the starting point
    if (intersect_id_start < allIntersections.size()) 
        allIntersections[intersect_id_start] = node;

    // Get the first wavePoint (waveFront)
    wavePoints.push(WavePoint(intersect_id_start, 0));
    bool allDestFound = false;
    
    while (!wavePoints.empty() && !allDestFound) {

        //Find next nodes to explore and remove it from wavePoints
        WavePoint wavePoint = wavePoints.top();
        wavePoints.pop();
        
        // Find adjacent Intersections and loop through
        std::vector<StreetSegmentIdx> adjacentSegments = findStreetSegmentsOfIntersection(wavePoint.idx);
        
        for (int i = 0; i < adjacentSegments.size(); i++) {

            StreetSegmentInfo segmentInfo = getStreetSegmentInfo(adjacentSegments[i]);
            
            // The node is reachable and needs to be explored.
            if (!segmentInfo.oneWay || segmentInfo.from == wavePoint.idx) {
                
                // Prepare the structs for the new node;
                IntersectionIdx nextNode = -1;
                PathNode thisNode;

                thisNode.from = adjacentSegments[i];
                
                //Correct the from and to according the last intersection
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

                int totalFoundBest = 0;     //number of best paths found

                for (int j = 0; j < dest.size(); j++) {     //loop through all destinations

                    //If current travel time is longer, then this destination has been explored as the best one
                    if (thisNode.travelTime > timeToReach[j]) {
                        totalFoundBest ++;
                    } else { 
                        //If a destination is reached
                        if (segmentInfo.to == dest[j] || segmentInfo.from == dest[j]) {
                            pathFound[j] = true;
                            
                            //Update the time to reach this destination if this path is better
                            if (timeToReach[j] > thisNode.travelTime) 
                                timeToReach[j] = thisNode.travelTime;
                        }
                    }                  
                }
                
                if (totalFoundBest <= numToFind) {
                    // If the intersection has not been reached before or 
                    //the last travel time took longer than the current calculation
                    if (allIntersections[nextNode].lastIntersection == -1 
                           || allIntersections[nextNode].travelTime > thisNode.travelTime
                       ) {
                        allIntersections[nextNode] = thisNode;
                        wavePoints.push(WavePoint(nextNode, thisNode.travelTime)); //push it into the wave front
                    }
                }
            }
        }
    }
    
    // Sort the output
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > result;
    for (int i = 0; i < pathFound.size(); i++) {
        if (pathFound[i]) {
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

// Pre-calculation code.


CalculateResult perturbationPrecalculated(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <int> resultIndex, 
        int intervals, std::vector<std::vector<WavePoint>> preCalculate, std::vector <IntersectionIdx> ids) {
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
//                double oTime = 0, nTime = 0;

                for (int j = i - 2; j < i + intervals + 1; j ++) {
                    int k = j;
                    int k0 = j + 1;

                    if (k == i - 1) k = i + intervals;
                    else if (k == i + intervals) k = i - 1;
                    if (k0 == i - 1) {
                        k0 = i + intervals;
                    } else if (k0 == i + intervals) {
                        k0 = i -1;
                    }

                    originalTime += preCalculate[resultIndex[j]][resultIndex[ j+ 1]].heuristicTime;
                    //oTime += computePathTravelTime(findPathBetweenIntersections(result[j], result[j + 1], 15), 15);
                    
                    if (preCalculate[resultIndex[k]].size() > resultIndex[k0] +1) {
                        newTime += preCalculate[resultIndex[k]][resultIndex[k0]].heuristicTime;
                    } else {
                        newTime = 9999999;
                    }
                    
                    //nTime += computePathTravelTime(findPathBetweenIntersections(ids[resultIndex[k]], ids[resultIndex[k0]], 15), 15);
                }

                //Update the result and bestTime if the newTime is shorter
                if (originalTime > newTime) {
                    int tmp = result[i - 1];
                    int tmpIdx = resultIndex[i - 1];
                    result[i -1] = result[i + intervals];
                    result[i + intervals] = tmp;
                    resultIndex[i -1] = resultIndex[i + intervals];
                    resultIndex[i + intervals] = tmpIdx;
                    bestTime = bestTime - originalTime + newTime;
                }
            }
        }

        double minTime = 99999999;
        //Find the depot with the least travel time from the last drop off point
        for (int i = deliveries.size() *2; i < ids.size(); i ++) {
            if (minTime > preCalculate[result[result.size() - 2]][i].idx) {
                minTime = preCalculate[result[result.size() - 2]][i].heuristicTime;
            }
        }
    }
    
    CalculateResult cResult;
    cResult.bestTime = bestTime;
    cResult.resultIdxIndex = resultIndex;
    cResult.result = result;
    std::clock_t end = clock();
    cResult.cpuTime = double(end - begin) / CLOCKS_PER_SEC;
    return cResult;
}



//Calculate the best path to deliver all packages
CalculateResult calculatePreload(double bestTime, std::vector <IntersectionIdx> result, std::vector <int> resultIndex, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, int depotId, std::vector<std::vector<WavePoint>> preCalculate, double randomLimit) {
    std::clock_t begin = clock();
    std::vector <IntersectionIdx> targetsPickup;
    int nextIdx = -1;

    for (int j = 0; j < deliveries.size(); j++) {
        targetsPickup.push_back(deliveries[j].pickUp);
    }

    double minTime;
    int currentNode = deliveries.size() * 2 + depotId;
    std::vector <int> legalIds = initializeLegalIds(deliveries.size(), depots.size());
    bool allDelivered = isAllDelivered(legalIds, deliveries.size());
    double travelTimeTotal = 0;
    bool canFind = true;
    std::vector <IntersectionIdx> resultTemp;
    std::vector <IntersectionIdx> resultTempIndex;
    resultTempIndex.push_back(currentNode);
    resultTemp.push_back(ids[currentNode]);
    while (!allDelivered && canFind) {
        if (preCalculate[currentNode].size() > 1) {
            int startPoint = -1;
            for (int k = 0; k < preCalculate[currentNode].size(); k++) {
                if (legalIds[preCalculate[currentNode][k].idx] == 1 && preCalculate[currentNode][k].idx < deliveries.size() * 2) {
                    startPoint = k; 
                    break;
                }
            }
            if (startPoint >= 0 && preCalculate[currentNode][startPoint].idx < deliveries.size() * 2) {
               
                if (rand() % 1000 > randomLimit * 1000 && preCalculate[currentNode].size() > startPoint + 1 
                        && preCalculate[currentNode][startPoint + 1].idx < deliveries.size() * 2) {
                    minTime = preCalculate[currentNode][startPoint + 1].heuristicTime;
                    nextIdx = preCalculate[currentNode][startPoint + 1].idx;
                } else {
                    minTime = preCalculate[currentNode][startPoint].heuristicTime;
                    nextIdx = preCalculate[currentNode][startPoint].idx;
                }
                
                if (nextIdx >= deliveries.size() * 2) std::cout << "Warning: reach depot which should not be \n";

                travelTimeTotal += minTime;
                resultTemp.push_back(ids[nextIdx]);
                resultTempIndex.push_back(nextIdx);
                //std::cout << "     " << ids[nextIdx] << " nextidx= " << nextIdx << " tmp size: " << resultTemp.size() << "size: " << resultTempIndex.size() << "\n";
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
        } else {
            canFind = false;
        }
    }

    if (canFind) {      //If the route can be found and all packages are delivered
        minTime = 9999999999999;
        IntersectionIdx lastIntersection = 0;

        //Find the depot with the least travel time
        
        for (int i = 0; i < preCalculate[currentNode].size(); i ++) {
            if (preCalculate[currentNode][i].idx >= deliveries.size() * 2) {
                minTime = preCalculate[currentNode][i].heuristicTime;
                lastIntersection = preCalculate[currentNode][i].idx;
            
            }
        }

        travelTimeTotal += minTime;
        resultTemp.push_back(ids[lastIntersection]);
        resultTempIndex.push_back(lastIntersection);
        if (bestTime > travelTimeTotal) {
            bestTime = travelTimeTotal;
            result = resultTemp;
            resultIndex = resultTempIndex;
        }
    }

    CalculateResult cResult;
    cResult.bestTime = bestTime;
    cResult.resultIdxIndex = resultIndex;
    cResult.result = result;
    std::clock_t end = clock();
    cResult.cpuTime = double(end - begin) / CLOCKS_PER_SEC;
    return cResult;
}

//Multi-Dijkstra to find a best path to a list of destination
//numToFind means how many the first number of shortest paths to be found
PreCalResult multidestDijkstra(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind) {
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

                for (int j = 0; j <= dest.size(); j++) {

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
                
                if (totalFoundBest <= numToFind) {
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
    
    PreCalResult finalResult;
    // Sort the output
    std::vector<WavePoint> resultOrignalOrder;
    for (int i = 0; i < pathFound.size(); i++) {
        resultOrignalOrder.push_back(WavePoint(i, timeToReach[i]));
    }
        // Sort the output
    std::priority_queue<WavePoint, std::vector<WavePoint>, std::greater<std::vector<WavePoint>::value_type> > result;
    for (int i = 0; i < pathFound.size(); i++) {
        if (pathFound[i]) {
            result.push(WavePoint(i, timeToReach[i]));
        }
    }
    
    std::vector<WavePoint> results;

    while(!result.empty()) {
        results.push_back(result.top());
        result.pop();
    }
    
    finalResult.result = results;
    finalResult.resultOrignalOrder = resultOrignalOrder;
    return finalResult;
}


