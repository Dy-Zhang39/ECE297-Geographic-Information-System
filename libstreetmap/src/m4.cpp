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
#include <chrono>
#include <time.h>
#define ILLEAGAL 0
#define LEAGAL 1
#define VISTED 2


struct PreCalResult {
    std::vector<WavePoint> result;              //the result of the multi-dest Dijkstra calculation sorted by travel time
    std::vector<WavePoint> resultOrignalOrder;  //the result of the multi-dest Dijkstra calculation sorted by ids order
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
//NOT USED WHEN PRECALCULATION IS IMPLEMENTED
/*CalculateResult calculate(double bestTime, std::vector <IntersectionIdx> result, 
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, double turn_penalty, int depotIdx, double randomLimit);
*/
/*
 * Perturbation to improve the current solution
 * @param bestTime: current best time to pickup and drop off all packages before running this function
 * @param result: current best travel sequence before running this function
 * @param deliveries: a list deliveries to pickup and drop off
 * @param turn_penalty: turn penalty in seconds
 * @param intervals: the interval between the position of the two nodes to be swapped
 *        (e.g. to swap result[0] and result[2], interval is 2)
 */
//NOT USED WHEN PRECALCULATION IS IMPLEMENTED
/*CalculateResult perturbation(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <IntersectionIdx>, double turn_penalty, int intervals);
*/
/*
 * Search for the best path to multiple destinations using Dijkstra Algorithm
 * @param intersect_id_start: starting point
 * @param dest: a list of destinations
 * @param turn_penalty: turn penalty in seconds
 * @param numToFind: the number of best paths to be found
 *        (e.g., if numToFind = 2, the best path of two destinations that takes the shortest time will be found; 
 *         if numToFind = dest.size(), best paths for all destinations will be returned)
 */
//NOT USED WHEN PRECALCULATION IS IMPLEMENTED
//std::vector <WavePoint> multidestDijkstraOpt(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty, int numToFind);

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

/*
 * Calculate the full travel sequence assuming starting from one of the depots
 * @param bestTime: current best time to pickup and drop off all packages before running this function
 * @param result: a list of the intersectionIdx represents the sequence of the travel
 * @param resultIndex:  the index of the global ids vector representing the sequence of travel
 * @param deliveries: a list deliveries to pickup and drop off
 * @param depots: a list of depots given
 * @param ids: a vector of all deliveries and depots intersections in the order of 
 *        pickUp1-dropOff1-pickUp2-dropOff2-...-pickUp[n]-dropOff[n]-depot1-...-depot[n]
 *        The purpose of this vector is to quickly locate the intersection ID and validate it
 * @param depotId: the number of depot IDs according to the sequence of the depots vector provided
 * @param preCalculate: the vector storing all information from any start point to any end point 
 *          e.g preCalculate[i][j], i stands for the ids index of the start point, 
 *              j is the order according to the travel time. If j is 0, that means it is the best path from ids[i] to the next point
 * @param randomLimit: used to generate random number 
 * (e.g. 1.0 means always choose the best next node, 0.9 means 10% of the chance to choose the second best next node)
 * @return CalculateResult: stores the best time, result (intersectionIdx) and resultIdxIndex(index of the ids)
 * 
 */
CalculateResult calculatePreload(double bestTime, std::vector <IntersectionIdx> result, std::vector <int> resultIndex,
        std::vector <DeliveryInf> deliveries, std::vector <IntersectionIdx> depots, 
        std::vector <IntersectionIdx> ids, int depotId, std::vector<std::vector<WavePoint>> preCalculate, double randomLimit);

/*
 * Perturbation to improve the current solution using pre-calculated information
 * @param bestTime: current best time to pickup and drop off all packages before running this function
 * @param result: current best travel sequence before running this function
 * @param deliveries: a list deliveries to pickup and drop off
 * @param resultIndex: the index of the global ids vector representing the sequence of travel
 * @param intervals: the interval between the position of the two nodes to be swapped
 *        (e.g. to swap result[0] and result[2], interval is 2)
 * @param preCalculate: the vector storing all information from any start point to any end point 
 *          e.g preCalculate[i][j], i stands for the ids index of the start point, 
 *              j is the ids index of the end point
 * @return CalculateResult: stores the best time, result (intersectionIdx) and resultIdxIndex(index of the ids)
 */
CalculateResult perturbationPrecalculated(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <int> resultIndex, 
        int intervals, std::vector<std::vector<WavePoint>> preCalculate, std::vector <IntersectionIdx> ids);

/*
 * Search for the best path to multiple destinations using Dijkstra Algorithm
 * @param intersect_id_start: starting point
 * @param dest: a list of destinations
 * @param turn_penalty: turn penalty in seconds
 * @param numToFind: the number of best paths to be found
 *        (e.g., if numToFind = 2, the best path of two destinations that takes the shortest time will be found; 
 *         if numToFind = dest.size(), best paths for all destinations will be returned)
 * @return struct of PreCalResult including the vector sorted by travel time and ids order
 */
PreCalResult multidestDijkstra(IntersectionIdx intersect_id_start, std::vector <IntersectionIdx> dest, double turn_penalty,  int numToFind);

std::vector<CourierSubPath> travelingCourier(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) {
    double remainingTimeBud = 44;
    //std::clock_t begin = clock();
    auto const begin = std::chrono::high_resolution_clock::now();

    std::vector <IntersectionIdx> result;       //the vector to store the current best travel sequence
    std::vector <IntersectionIdx> ids;          //the vector of all deliveries and depots intersections in the order initialized  below
    std::vector <int> legalIds;                 //the vector that is used to determine whether an intersection is legal for travel
    
    //Initialize the ids vector
    //store all delivery intersecton in ids
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
    
    //Pre-calculation
    #pragma omp parallel for
    for (int i = 0; i < ids.size(); i++) {
        PreCalResult costs = multidestDijkstra(ids[i], ids, turn_penalty, ids.size());
        preCalculate[i] = costs.result;
        preCalculateOrigOrder[i] = costs.resultOrignalOrder;
    }

    auto preCalcFin = std::chrono::high_resolution_clock::now();
    
    remainingTimeBud  -= (std::chrono::duration_cast<std::chrono::duration<double>>(preCalcFin - begin)).count();
    CalculateResult cResult;
    std::vector <int> resultIndex;
    //Loop through all depots using greedy algorithm of finding shortest next path
    //NEED TO IMPROVE - multi-thread not working, might need to do perturbation for each solution
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
    bool continueOpt = true;   
    //int firstNode = resultIndex[0] - deliveries.size() * 2;
    auto currentSimple = std::chrono::high_resolution_clock::now();                                                                   ;
    std::cout << "Simple best time: " << currentBestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - preCalcFin)).count() <<  "\n";

    //Stop the perturbation if the time reaches 90% of the total budget (45s)
    if ((std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - preCalcFin)).count() < (remainingTimeBud - 15)) {
    
        //perturbation to improve the current best solution
        for (int i = 0; i < 5; i ++) {       //try all the possible intervals of perturbation
            auto current = std::chrono::high_resolution_clock::now();
            //Stop the perturbation if the time reaches 90% of the total budget (45s)
            if ((std::chrono::duration_cast<std::chrono::duration<double>>(current - preCalcFin)).count() < remainingTimeBud) {
                continueOpt = true;
            } else {
                continueOpt = false;
            }

            if (continueOpt) {
                for (int k = 0; k < 100; k ++) {                  //iterations to run for the same interval
                    if (continueOpt) {
                        cResult = 
                            perturbationPrecalculated(currentBestTime, result, deliveries, resultIndex, i, preCalculateOrigOrder, ids);


                        // If the new solution is better than the current one
                        if (currentBestTime > cResult.bestTime) {   // Update the current one with the new.
                            currentBestTime = cResult.bestTime;
                            result = cResult.result;
                            resultIndex = cResult.resultIdxIndex;
                        } else {    // Otherwise exit the loop
                            break;
                        }

                    } else { 
                        break;
                    }
                }
            } else {
                break;
            }
        }

    } 
    auto currentNext = std::chrono::high_resolution_clock::now();
    std::cout << "Current best time: " << currentBestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentNext - preCalcFin)).count() <<  "\n";

    //Have a 10% chance of taking the second smallest travel time. Using the current solution's first node as the starting point
    //firstNode = resultIndex[0] - deliveries.size() * 2;

    int iterationCount = 0;
    //1000 iterations using the same random ratio.
    //problem: multi-thread not working due to check of time limit. Might be able to solve it. NEED TO IMPROVE
    //#pragma omp parallel for
    for (int randomK = 0; randomK < 50000; randomK++) {    
        auto currentRandom = std::chrono::high_resolution_clock::now();
        //Stop the perturbation if the time reaches 90% of the total budget (45s)
        if ((std::chrono::duration_cast<std::chrono::duration<double>>(currentRandom - preCalcFin)).count() < remainingTimeBud) {
            continueOpt = true;
            iterationCount = randomK;
        } else {
            continueOpt = false;
        }
        if (continueOpt) {
            if (iterationCount > 5000) {
                cResult = 
                    calculatePreload(9999999, {}, resultIndex, deliveries, depots, ids, resultIndex[0] - deliveries.size() * 2, preCalculate, 0.93);
            } else if (iterationCount > 100) {
                cResult = 
                    calculatePreload(9999999, {}, resultIndex, deliveries, depots, ids, resultIndex[0] - deliveries.size() * 2, preCalculate, 0.92);
            } else if (iterationCount > 20) {
                cResult = 
                    calculatePreload(9999999, {}, resultIndex, deliveries, depots, ids, resultIndex[0] - deliveries.size() * 2, preCalculate, 0.91);
            } else {
                cResult = 
                    calculatePreload(9999999, {}, resultIndex, deliveries, depots, ids, resultIndex[0] - deliveries.size() * 2, preCalculate, 0.9);
            }

            double currentBestTimeTemp = cResult.bestTime;
            std::vector <IntersectionIdx>  resultTemp = cResult.result;
            std::vector <int> resultIndexTemp = cResult.resultIdxIndex;
            
            //User perturbation to fine tune the random solution even if it is not the current best
            //perturbation 
            for (int i = 0; i < 5; i ++) {
                if (continueOpt) {
                    for (int k = 0; k < 100; k ++) {
                        auto current = std::chrono::high_resolution_clock::now();

                        if ((std::chrono::duration_cast<std::chrono::duration<double>>(current - preCalcFin)).count() < remainingTimeBud) {
                            continueOpt = true;
                        } else {
                            continueOpt = false;
                        }
                        
                        if (continueOpt) {
                            cResult = 
                                perturbationPrecalculated(currentBestTimeTemp, resultTemp, deliveries, resultIndexTemp, i, preCalculateOrigOrder, ids);

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
            //#pragma omp critical
            {
                if (currentBestTimeTemp < currentBestTime) {
                    currentBestTime = currentBestTimeTemp;
                    result = resultTemp;
                    resultIndex = resultIndexTemp;
                }
            }
        } else {
            break;
        }
    }
    
    //Other improvements might be considered
    //2-opt perturbation
    //simulation annealing
    //2-opt w/ changing order between the two exchange points
    auto currentFin = std::chrono::high_resolution_clock::now();
    std::cout << "Next best time after " << iterationCount <<" iterations: " << currentBestTime << "    Time remaining: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentFin - preCalcFin)).count() <<  "\n";
    
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
    auto end = std::chrono::high_resolution_clock::now();
    remainingTimeBud -= (std::chrono::duration_cast<std::chrono::duration<double>>(end - preCalcFin)).count();
    std::cout<<"remaining cpu time: " << remainingTimeBud << " Total Travel Time: " << totalCourierTime << "  Estimated: " << currentBestTime << "\n";
    return courierPath;
    
} 
/*NOT USED WHEN PRECALCULATION IS IMPLEMENTED*/
/*CalculateResult perturbation(double bestTime, std::vector<IntersectionIdx> result, 
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
 * 
 * */


// Check whether all has been delivered already or not.
bool isAllDelivered(std::vector <int> legalIds, int deliverySize) {
                            
    bool allDelivered = true;
    for (int k = 0; k < 2*deliverySize; k++) {
        if (legalIds[k] != VISTED) {
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

//------------------- Pre-calculation code.---------------------------
CalculateResult perturbationPrecalculated(double bestTime, std::vector<IntersectionIdx> result, 
        std::vector<DeliveryInf> deliveries, std::vector <int> resultIndex, 
        int intervals, std::vector<std::vector<WavePoint>> preCalculate, std::vector <IntersectionIdx> ids) {
    std::clock_t begin = clock();
    //Check the swap validation
    if (deliveries.size() > 1) {
        for (int i = 2; i < result.size() - 1 - intervals; i++) {
            bool valid = true;
            for (int k0 = i -1 ; k0 < i + intervals; k0++) {
                if (resultIndex[k0] % 2  == 0) {
                    for (int k = k0 + 1; k < i + intervals + 1; k++) {
                        //If the dropOff will appear before the current corresponding pickup after swapping, the swap is invalid
                        if (resultIndex[k] - 1 == resultIndex[k0]) {
                            valid = false;
                        }
                    }  
                }
            }

            //If the swap is valid
            if (valid) {
                double originalTime = 0, newTime = 0;

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

                    //get the travel time from the pre-calculated vector
                    originalTime += preCalculate[resultIndex[j]][resultIndex[ j+ 1]].heuristicTime;
                    
                    if (preCalculate[resultIndex[k]].size() > resultIndex[k0] +1) {
                        newTime += preCalculate[resultIndex[k]][resultIndex[k0]].heuristicTime;
                    } else {
                        newTime = 9999999;
                    }
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

        //Get the start point and end point travel time of the current solution
        double minTime = preCalculate[resultIndex[resultIndex.size() - 2]][resultIndex[resultIndex.size() - 1]].heuristicTime;
        double minStartTime = preCalculate[resultIndex[0]][resultIndex[1]].heuristicTime;
        
        double originalLastTime = minTime;
        double originalFirstTime = minStartTime;

        //Find the depot with the least travel time from the last drop off point and first pickup point
        for (int i = deliveries.size() *2; i < ids.size(); i ++) {
            if (minTime > preCalculate[resultIndex[resultIndex.size() - 2]][i].heuristicTime) {
                minTime = preCalculate[resultIndex[resultIndex.size() - 2]][i].heuristicTime;
                result[result.size() - 1] = ids[i];
                resultIndex[result.size() - 1] = i;
            }
            
            if (minStartTime > preCalculate[i][resultIndex[1]].heuristicTime){
                minStartTime = preCalculate[i][resultIndex[1]].heuristicTime;
                result[0] = ids[i];
                resultIndex[0] = i;
            }
        }   
        //Update the best time after optimizing first and last points
        bestTime = bestTime - originalLastTime - originalFirstTime + minTime + minStartTime;

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
    int nextIdx = -1;

    double minTime;
    int currentNode = deliveries.size() * 2 + depotId;      //the ids index of starting depot
    
    std::vector <int> legalIds = initializeLegalIds(deliveries.size(), depots.size());
    bool allDelivered = isAllDelivered(legalIds, deliveries.size());
    
    double travelTimeTotal = 0;
    bool canFind = true;
    
    std::vector <IntersectionIdx> resultTemp;
    std::vector <int> resultTempIndex;
    resultTempIndex.push_back(currentNode);
    resultTemp.push_back(ids[currentNode]);
    
    while (!allDelivered && canFind) {
        if (preCalculate[currentNode].size() > 1) {
            std::vector<int> startPoint;
            int numLegalFound = 0;
            //Find the first two legal points in the sorted preCalculate vector
            for (int k = 0; k < preCalculate[currentNode].size(); k++) {
                if (legalIds[preCalculate[currentNode][k].idx] == 1 && preCalculate[currentNode][k].idx < deliveries.size() * 2) {
                    startPoint.push_back(k); 
                    numLegalFound ++;
                    if (numLegalFound > 1) break;
                }
            }
            
            //Choose the best/second best result based on the possibility provided
            if (startPoint.size() > 0 && preCalculate[currentNode][startPoint[0]].idx < deliveries.size() * 2) {
               
                if (rand() % 1000 > randomLimit * 1000 && startPoint.size() > 1 && preCalculate[currentNode].size() > startPoint[1] 
                        && preCalculate[currentNode][startPoint[1]].idx < deliveries.size() * 2) {
                    minTime = preCalculate[currentNode][startPoint[1]].heuristicTime;
                    nextIdx = preCalculate[currentNode][startPoint[1]].idx;
                } else {
                    minTime = preCalculate[currentNode][startPoint[0]].heuristicTime;
                    nextIdx = preCalculate[currentNode][startPoint[0]].idx;
                }
                
                if (nextIdx >= deliveries.size() * 2) std::cout << "Warning: reach depot which should not be \n";

                travelTimeTotal += minTime;
                resultTemp.push_back(ids[nextIdx]);
                resultTempIndex.push_back(nextIdx);

                //Update legality status according to type of site reached
                if (nextIdx % 2 == 0) {         //a pickup site
                    legalIds[nextIdx + 1] = 1;
                    legalIds[nextIdx] = 2;
                } else {                        //drop off site
                    legalIds[nextIdx] = 2;
                }

                currentNode = nextIdx;
                //allDelivered = isAllDelivered(legalIds, deliveries.size());
                //std::cout << resultTemp.size() << "," << deliveries.size() + 1 << std::endl;
                if (resultTemp.size() == deliveries.size() * 2 + 1){
                    allDelivered = true;
                }else{
                    allDelivered = false;
                }
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
    
    //Store the result in original order
    std::vector<WavePoint> resultOrignalOrder;
    for (int i = 0; i < pathFound.size(); i++) {
        resultOrignalOrder.push_back(WavePoint(i, timeToReach[i]));
    }
    
    // Sort the output using priority queue
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


