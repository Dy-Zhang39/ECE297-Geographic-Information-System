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
#include "drawMap.h"
#include <chrono>
#include <climits>
#include <time.h>
#include <math.h>
#define ILLEAGAL 0
#define LEAGAL 1
#define VISTED 2

double tempDropRate = 0.95;

struct PreCalResult {
    std::vector<WavePoint> result;              //the result of the multi-dest Dijkstra calculation sorted by travel time
    std::vector<WavePoint> resultOrignalOrder;  //the result of the multi-dest Dijkstra calculation sorted by ids order
};

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
CalculateResult perturbationPrecalculated(CalculateResult solution, std::vector<DeliveryInf> deliveries, 
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

/*
 * Simulation annealing
 * @param currentSolution: the current best solution to perturbate
 * @param deliveries: a list deliveries to pickup and drop off
 * @param preCalculate: the vector storing all information from any start point to any end point 
 *          e.g preCalculate[i][j], i stands for the ids index of the start point, 
 *              j is the order according to the travel time. If j is 0, that means it is the best path from ids[i] to the next point
 * @param ids: a vector of all deliveries and depots intersections in the order of 
 *        pickUp1-dropOff1-pickUp2-dropOff2-...-pickUp[n]-dropOff[n]-depot1-...-depot[n]
 * @param remaiinTimeBud: the time left to pass the tests safely
 * @param maxIntervals: the max interval of perturbation (for swapping)
 * @param startTemp: the starting temperature of annealing
 * @return CalculateResult: stores the best time, result (intersectionIdx) and resultIdxIndex(index of the ids)
 */
CalculateResult simulatedAnnealing(CalculateResult currentSolution, 
        std::vector<DeliveryInf> deliveries,std::vector<std::vector<WavePoint>> preCalculate, 
        std::vector <IntersectionIdx> ids, double remainTimeBud, int maxIntervals, double startTemp);

/*
 * Simulation annealing
 * @param solution: the current best solution to perturbate
 * @param firstIdx: the end index of first segment in solution
 * @param secondIdx: the end index of second segment in solution
 * @param preCalculateOriginalOrder e.g preCalculate[i][j], i stands for the ids index of the start point, 
 *              j is the order according to the travel time. If j is 0, that means it is the best path from ids[i] to the next point
 * @param ids: a vector of all deliveries and depots intersections in the order of 
 *        pickUp1-dropOff1-pickUp2-dropOff2-...-pickUp[n]-dropOff[n]-depot1-...-depot[n]
 * @return CalculateResult: stores the best time, result (intersectionIdx) and resultIdxIndex(index of the ids)
 */
CalculateResult twoOptSingle (const CalculateResult& solution, int firstIdx, int secondIdx, 
                       const std::vector <IntersectionIdx> & ids, const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder);

CalculateResult twoOptNonChangingSolution(const CalculateResult& currentSolution, const std::vector <IntersectionIdx> & ids, 
        const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder);

CalculateResult findLocalMinWithTwoOpt (const CalculateResult& currentSolution, const std::vector <IntersectionIdx> & ids, 
        const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder, double reaminingTime);
        
std::vector<CourierSubPath> travelingCourier(
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots,
    const float turn_penalty) {
    
    
    
    double remainingTimeBud = 44;
    //std::clock_t begin = clock();
    auto const begin = std::chrono::high_resolution_clock::now();

    //std::vector <IntersectionIdx> result;       //the vector to store the current best travel sequence
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
    //---------------------------------------------------PRECALC------------------------------------------------------------
    // Pre calculate all the costs
    std::vector <std::vector <WavePoint>> preCalculate;
    std::vector <std::vector <WavePoint>> preCalculateOrigOrder;
    preCalculate.resize(ids.size());
    preCalculateOrigOrder.resize(ids.size());
    CalculateResult currentSolution;
    currentSolution.bestTime = INT_MAX;
    double currentBestTime = INT_MAX;
    
    //Pre-calculation
    #pragma omp parallel for
    for (int i = 0; i < ids.size(); i++) {
        PreCalResult costs = multidestDijkstra(ids[i], ids, turn_penalty, ids.size());
        preCalculate[i] = costs.result;
        preCalculateOrigOrder[i] = costs.resultOrignalOrder;
        //auto current = std::chrono::high_resolution_clock::now();
        //std::cout << "Time it takes" << (std::chrono::duration_cast<std::chrono::duration<double>>(current - begin)).count() << std::endl;
    }
    
    auto preCalcFin = std::chrono::high_resolution_clock::now();
    
    remainingTimeBud  -= (std::chrono::duration_cast<std::chrono::duration<double>>(preCalcFin - begin)).count();

    std::cout << "Pre calculation is finished: " << currentSolution.bestTime << "    Time remained: " << remainingTimeBud<<  "\n";
    
    

    //std::vector <int> resultIndex;
    
    //Loop through all depots using greedy algorithm of finding shortest next path

    //NEED TO IMPROVE - multi-thread not working, might need to do perturbation for each solution
    
    std::vector<double> bestTimesDepots(depots.size(), INT_MAX);
    std::vector<std::vector <int>> bestResultsIdx(depots.size());
    std::vector<std::vector <IntersectionIdx>> bestResults(depots.size());
    
    //std::cout << "Number of depots: " << depots.size() << std::endl;
    //#pragma omp parallel for
    for (int i = 0; i < depots.size(); i++) {
        
        CalculateResult calcResult =
                calculatePreload(bestTimesDepots[i], currentSolution.result, currentSolution.resultIdxIndex, deliveries, depots, ids, i, preCalculate, 1.0);
        
        auto current = std::chrono::high_resolution_clock::now();
        //std::cout << "Time Remaining: " << 35 - (std::chrono::duration_cast<std::chrono::duration<double>>(current - begin)).count() << std::endl;
        simulatedAnnealing(calcResult, deliveries, preCalculateOrigOrder, ids, 
        30 - (std::chrono::duration_cast<std::chrono::duration<double>>(current - begin)).count(), 5, 0);
        if (calcResult.bestTime < bestTimesDepots[i]){
            bestTimesDepots[i] = calcResult.bestTime;
            bestResults[i] = calcResult.result;
            bestResultsIdx[i] = calcResult.resultIdxIndex;
            
        }
    }
    
    for (int i = 0; i < bestTimesDepots.size(); i++){
        if (bestTimesDepots[i] < currentSolution.bestTime) {
            currentSolution.bestTime = bestTimesDepots[i];
            currentSolution.result = bestResults[i];
            currentSolution.resultIdxIndex = bestResultsIdx[i];
        }
    }
    

    

    //bool continueOpt = true;   
    //int firstNode = resultIndex[0] - deliveries.size() * 2;
    std::cout<<"Using Clock"  << std::endl;
    auto currentSimple = std::chrono::high_resolution_clock::now();
    std::cout << "Simple best time: " << currentSolution.bestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - preCalcFin)).count() <<  "\n";
    
    /*
    currentSolution = findLocalMinWithTwoOpt(currentSolution, ids, preCalculateOrigOrder, remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - preCalcFin)).count());
    
    auto twoOpt = std::chrono::high_resolution_clock::now();
    std::cout << "best time after Two opt: " << currentSolution.bestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(twoOpt - preCalcFin)).count() <<  "\n";
     */

    //regular perturbation (without simulation annealing since the temperature is set to 0)
    //currentSolution = simulatedAnnealing(currentSolution, deliveries, preCalculateOrigOrder, ids, 
        //35 - (std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - begin)).count(), 5, 0);
    currentBestTime = currentSolution.bestTime;
 
    auto currentNext = std::chrono::high_resolution_clock::now();
    std::cout << "Current best time: " << currentBestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentNext - preCalcFin)).count() <<  "\n";
    int iterationCount = 0;

    //Have a 10% chance of taking the second smallest travel time. Using the current solution's first node as the starting point
    //250000 iterations using the same random ratio.

    //problem: multi-thread not working due to check of time limit. Might be able to solve it. NEED TO IMPROVE

    //#pragma omp parallel for
    
    //while (true){
    //for (int randomK = 0; randomK < 50000; randomK++) {    

   
    CalculateResult cResult;
    bool continueOptRandom = true;
    tempDropRate = 0.99;
    for (int randomK = 0; randomK < 250000; randomK++) {    
        auto currentRandom = std::chrono::high_resolution_clock::now();
        //Stop the perturbation if the time reaches 90% of the total budget (45s)
        if ((std::chrono::duration_cast<std::chrono::duration<double>>(currentRandom - preCalcFin)).count() < remainingTimeBud - 1) {
            continueOptRandom = true;
            iterationCount = randomK;
        } else {
            continueOptRandom = false;
        }
        
        
        if (continueOptRandom) {
            if (iterationCount > 50000) {
                cResult = 
                    calculatePreload(9999999, {}, currentSolution.resultIdxIndex, deliveries, depots, ids, randomK % depots.size(), preCalculate, 0.92);
            } else if (iterationCount > 5000) {
                cResult = 
                    calculatePreload(9999999, {}, currentSolution.resultIdxIndex, deliveries, depots, ids, currentSolution.resultIdxIndex[0] - deliveries.size() * 2, preCalculate, 0.92);
            } else {
                cResult = 
                    calculatePreload(9999999, {}, currentSolution.resultIdxIndex, deliveries, depots, ids, currentSolution.resultIdxIndex[0] - deliveries.size() * 2, preCalculate, 0.92);
            }

            std::vector <IntersectionIdx>  resultTemp = cResult.result;
            std::vector <int> resultIndexTemp = cResult.resultIdxIndex;
            if (resultTemp.size() > 0) {
                //User perturbation to fine tune the random solution even if it is not the current best
                cResult = simulatedAnnealing(cResult, deliveries, preCalculateOrigOrder, ids, 
                    44 - (std::chrono::duration_cast<std::chrono::duration<double>>(currentRandom - begin)).count(), 5, 5);

                if (cResult.bestTime < currentSolution.bestTime)
                    currentSolution = cResult;
            }
        } else {
            break;
        }
    }
    
    //2-opt perturbation
    //2-opt w/ changing order between the two exchange points
    auto currentFin = std::chrono::high_resolution_clock::now();
    std::cout << "Next best time after " << iterationCount <<" iterations: " << currentSolution.bestTime << "    Time remaining: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentFin - preCalcFin)).count() <<  "\n";
    
    // If enough time start simulated annealing
    if (45 - (std::chrono::duration_cast<std::chrono::duration<double>>(currentFin - begin)).count() > 0) {
        std::cout << "-----------------Start annealing-------------\n";
        CalculateResult resultSolution = simulatedAnnealing(currentSolution, deliveries, preCalculateOrigOrder, ids, 
            45 - (std::chrono::duration_cast<std::chrono::duration<double>>(currentFin - begin)).count(), deliveries.size() - 2, 0);
        tempDropRate = 0.99;
        currentSolution = simulatedAnnealing(currentSolution, deliveries, preCalculateOrigOrder, ids, 
            45 - (std::chrono::duration_cast<std::chrono::duration<double>>(currentFin - begin)).count(), deliveries.size() - 2, 9);
        
        if (currentSolution.bestTime > resultSolution.bestTime) currentSolution = resultSolution;
    }
    std::vector <CourierSubPath> courierPath;
    double totalCourierTime = 0;
    //result = currentSolution.result;
    if (currentSolution.result.size() > 1) {        //if a solution exists
        
        for (IntersectionIdx idx = 1; idx < currentSolution.result.size(); idx++) {
            CourierSubPath pathWay;
            pathWay.start_intersection = currentSolution.result[idx - 1];
            pathWay.end_intersection = currentSolution.result[idx];
            pathWay.subpath = findPathBetweenIntersections(pathWay.start_intersection, pathWay.end_intersection, turn_penalty);

            courierPath.push_back(pathWay);           
            totalCourierTime += computePathTravelTime(pathWay.subpath, turn_penalty);
        }
    }
    
    currentSolution = findLocalMinWithTwoOpt(currentSolution, ids, preCalculateOrigOrder, remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(currentSimple - preCalcFin)).count());
    
    auto twoOpt = std::chrono::high_resolution_clock::now();
    std::cout << "best time after Two opt: " << currentSolution.bestTime << "    Time remained: " << remainingTimeBud - (std::chrono::duration_cast<std::chrono::duration<double>>(twoOpt - preCalcFin)).count() <<  "\n";
    
    std::cout << "   From: " << depots[0] << " ---> ";
    auto end = std::chrono::high_resolution_clock::now();
    remainingTimeBud -= (std::chrono::duration_cast<std::chrono::duration<double>>(end - preCalcFin)).count();
    std::cout<<"remaining cpu time: " << remainingTimeBud << " Total Travel Time: " << totalCourierTime << "  Estimated: " << currentSolution.bestTime << "\n";
    return courierPath;
    
} 



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

CalculateResult simulatedAnnealing(CalculateResult currentSolution, 
        std::vector<DeliveryInf> deliveries,std::vector<std::vector<WavePoint>> preCalculate, 
        std::vector <IntersectionIdx> ids, double remainTimeBud, int maxIntervals, double startTemp) {
    
    
    auto start = std::chrono::high_resolution_clock::now();
    double currentBestTimeTemp = currentSolution.bestTime;
    std::vector <IntersectionIdx>  resultTemp = currentSolution.result;
    std::vector <int> resultIndexTemp = currentSolution.resultIdxIndex;

    
    CalculateResult cResult = currentSolution;
    bool continueOpt = true;
    currentSolution.currentTemperature = startTemp;
        
    //if not annealing, find the best solution by keeping the current solution same
    //until it reach the local minimum
    if(startTemp == 0){
        
        //while does not reach the local minimum and still have time
        while (continueOpt) {
            std::vector<CalculateResult> betterTimes;

            for (int i = 0; i < deliveries.size() - 2 && continueOpt; i++) { //try all the possible intervals of perturbation
                auto current = std::chrono::high_resolution_clock::now();
                //Stop the perturbation if the time reaches 90% of the total budget (45s)
                if ((std::chrono::duration_cast<std::chrono::duration<double>>(current - start)).count() < remainTimeBud) {
                    continueOpt = true;
                } else {
                    continueOpt = false;
                    std::cout << "Time out" << std::endl;
                }

                if (continueOpt) {

                    cResult = perturbationPrecalculated(currentSolution, deliveries, i, preCalculate, ids);


                    // If the new solution is better than the current one
                    if (currentSolution.bestTime > cResult.bestTime) { // Update the current one with the new.
                        betterTimes.push_back(cResult);
                        //std::cout << "New Best Time: " << currentBestTime << std::endl;
                    }



                }
            }

            if (betterTimes.size() != 0) {

                for (int better = 0; better < betterTimes.size(); better++) {
                    if (currentSolution.bestTime > betterTimes[better].bestTime) {
                        currentSolution.bestTime = betterTimes[better].bestTime;
                        currentSolution.result = betterTimes[better].result;
                        currentSolution.resultIdxIndex = betterTimes[better].resultIdxIndex;
                    }

                }

            } else {
                continueOpt = false;
                std::cout << "reach local minimum" << std::endl;
            }
        }
       
    }
    //if it is simulation annealing the solution will change every time when called perturbation
    else{
        for (int i = 0; i < maxIntervals; i++) {
            if (continueOpt) {
                cResult.currentTemperature = startTemp; //set the initial temperature


                for (int k = 0; k < 100000; k++) {
                    auto current = std::chrono::high_resolution_clock::now();
                    if ((std::chrono::duration_cast<std::chrono::duration<double>>(current - start)).count() < remainTimeBud) {
                        continueOpt = true;



                    } else {
                        continueOpt = false;
                    }

                    if (continueOpt) {
                        cResult =

                                perturbationPrecalculated(cResult, deliveries, i, preCalculate, ids);

                        if (currentBestTimeTemp > cResult.bestTime) {
                            currentBestTimeTemp = cResult.bestTime;
                            resultTemp = cResult.result;
                            resultIndexTemp = cResult.resultIdxIndex;
                        }

                        //Continue to perturbation even if the result is worse when the temperature is greater than the threshold
                        if (currentBestTimeTemp == cResult.bestTime ||
                                (currentBestTimeTemp < cResult.bestTime && cResult.currentTemperature > 0.0000001)) {
                            break;
                        } else {
                            cResult.currentTemperature *= tempDropRate;

                            currentBestTimeTemp = cResult.bestTime;
                            resultTemp = cResult.result;
                            resultIndexTemp = cResult.resultIdxIndex;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
        if (currentBestTimeTemp < currentSolution.bestTime) {
            currentSolution.bestTime = currentBestTimeTemp;
            currentSolution.result = resultTemp;
            currentSolution.resultIdxIndex = resultIndexTemp;
        }
    }
    
    
    return currentSolution;
}

//------------------- Pre-calculation code.---------------------------
CalculateResult perturbationPrecalculated(CalculateResult solution, std::vector<DeliveryInf> deliveries, 
        int intervals, std::vector<std::vector<WavePoint>> preCalculate, std::vector <IntersectionIdx> ids) {
    std::clock_t begin = clock();
    
    
    std::vector<std::vector<IntersectionIdx>> betterResults;
    std::vector<std::vector <int>> betterResultsIdx;
    std::vector<double> betterTime;
    
    if (solution.resultIdxIndex.size() == 0){
        std::cout << "Nothing is the solution" <<std::endl;
        return solution;
    }
    //Check the swap validation
    if (deliveries.size() * 2 > intervals + 3) {
        for (int i = 2; i < solution.result.size() - 1 - intervals; i++) {
            
            bool valid = true;
            
            bool checkFirst = false, checkSecond = false;
            int firstElement = solution.resultIdxIndex[i - 1];
            int secondElement = solution.resultIdxIndex[i + intervals];
            //if first swap element is a pick up
            if (firstElement% 2 == 0){
                checkFirst = true;
            }

            //if second swap element is a drop off
            if (secondElement % 2 == 1){
                checkSecond = true;
            }

            if (checkFirst || checkSecond){
                    
                //also need to check to last swapping element to prevent the case of swapping pick up A and drop off A
                for (int locationBetween = i; locationBetween < i + intervals + 1 && valid; locationBetween ++){

                    //drop off appear before pick up
                    if (solution.resultIdxIndex[locationBetween] - 1 == firstElement){
                        valid = false;
                    }

                    if (solution.resultIdxIndex[locationBetween] + 1 == secondElement){
                        valid = false;
                    }
                }
            }
            
            

            //If the swap is valid
            if (valid) {
                double originalTime = 0, newTime = 0;
                
                int newStartDepotIdx = -1, newEndDepotIdx = -1;
                
                int firstSwap = i - 1;
                int secondSwap = i + intervals;
                
                int previousFirstSwap = firstSwap - 1;                
                int afterFirstSwap = firstSwap + 1;
                
                int previousSecondSwap = secondSwap - 1;               
                int afterSecondSwap = secondSwap + 1;

                //previous location of first swap to the location of first swap
                originalTime += preCalculate[solution.resultIdxIndex[previousFirstSwap]][solution.resultIdxIndex[firstSwap]].heuristicTime;
                
                //when two swap are right beside each other
                if (afterFirstSwap == secondSwap && previousSecondSwap == firstSwap){
                    originalTime += preCalculate[solution.resultIdxIndex[firstSwap]][solution.resultIdxIndex[secondSwap]].heuristicTime;
                }else{
                    
                    //location of first swap to the location after first swap
                    originalTime += preCalculate[solution.resultIdxIndex[firstSwap]][solution.resultIdxIndex[afterFirstSwap]].heuristicTime;
                    
                    //previous location of second swap to second swap
                    originalTime += preCalculate[solution.resultIdxIndex[previousSecondSwap]][solution.resultIdxIndex[secondSwap]].heuristicTime;
                }
                
                //location of second swap to the next location of second swap
                originalTime += preCalculate[solution.resultIdxIndex[secondSwap]][solution.resultIdxIndex[afterSecondSwap]].heuristicTime;

                
                //calculate the new time if two locations are swapped
                
                //if previous location of the first swap is a depot
                //find the nearest depot to the second swap
                if (previousFirstSwap == 0){

                    //check every depot to the second swap
                    double minTime = INT_MAX;
                    for (int depotIdx = deliveries.size() *2; depotIdx < ids.size(); depotIdx ++) {
                        
                        if (minTime > preCalculate[depotIdx][solution.resultIdxIndex[secondSwap]].heuristicTime) {
                            minTime = preCalculate[depotIdx][solution.resultIdxIndex[secondSwap]].heuristicTime;
                            newStartDepotIdx = depotIdx;
                        }
                    }

                    newTime += minTime;
                }else{
                    newTime += preCalculate[solution.resultIdxIndex[previousFirstSwap]][solution.resultIdxIndex[secondSwap]].heuristicTime;
                }
                
                //std::cout << preCalculate[resultIndex[secondSwap]][resultIndex[afterFirstSwap]].heuristicTime << std::endl; 
                
                //if two swapping element are right beside each other
                if (afterFirstSwap == secondSwap && previousSecondSwap == firstSwap){
                    newTime += preCalculate[solution.resultIdxIndex[secondSwap]][solution.resultIdxIndex[firstSwap]].heuristicTime;
                }else{
                    newTime += preCalculate[solution.resultIdxIndex[secondSwap]][solution.resultIdxIndex[afterFirstSwap]].heuristicTime;
                    newTime += preCalculate[solution.resultIdxIndex[previousSecondSwap]][solution.resultIdxIndex[firstSwap]].heuristicTime;
                }
                
                //if the next location of the second swap is a depot
                //find the nearest depot to the first swap after swapping
                if (afterSecondSwap == solution.result.size() - 1){

                    double minTime = INT_MAX;

                    for (int depotIdx = deliveries.size() * 2; depotIdx < ids.size(); depotIdx ++){

                        if (minTime > preCalculate[solution.resultIdxIndex[firstSwap]][depotIdx].heuristicTime){
                            minTime = preCalculate[solution.resultIdxIndex[firstSwap]][depotIdx].heuristicTime;
                            newEndDepotIdx = depotIdx;
                        }
                    }

                    newTime += minTime;

                }else{
                    newTime +=  preCalculate[solution.resultIdxIndex[firstSwap]][solution.resultIdxIndex[afterSecondSwap]].heuristicTime;
                }
                
                
               

                //Update the result and bestTime if the newTime is shorter

                if (originalTime > newTime || (solution.currentTemperature > 0.0000000001 && 
                        originalTime < newTime && exp((newTime - originalTime)/ solution.currentTemperature * -1.0) * 1000 > (rand() % 1000 ))) {
                    
                    std::vector<IntersectionIdx> tempResult(solution.result);
                    std::vector<int> tempResultIdx(solution.resultIdxIndex);
                    double tempBetterTime;
                    
                    //if the starting depot is changes due to swapping
                    if (newStartDepotIdx != -1){
                        tempResultIdx[0] = newStartDepotIdx;
                        tempResult[0] = ids[newStartDepotIdx];
                    }
                    
                    if (newEndDepotIdx != -1){
                        tempResultIdx[solution.result.size() - 1] = newEndDepotIdx;
                        tempResult[solution.result.size() - 1] = ids[newEndDepotIdx];
                    }
                    
                    tempResultIdx[i - 1] = solution.resultIdxIndex[i + intervals];
                    tempResultIdx[i + intervals]  = solution.resultIdxIndex[i - 1];
                    
                    tempResult[i - 1] = solution.result[i + intervals];
                    tempResult[i + intervals]  = solution.result[i - 1];
                    
                    
                    tempBetterTime = solution.bestTime - originalTime + newTime;
                    
                    betterResults.push_back(tempResult);
                    betterResultsIdx.push_back(tempResultIdx);
                    betterTime.push_back(tempBetterTime);
                    solution.currentTemperature *= tempDropRate;
                }    

               /*
                if (originalTime > newTime || (solution.currentTemperature > 0.0000000001 && 
                        originalTime < newTime && exp((newTime - originalTime)/ solution.currentTemperature * -1.0) * 1000 > (rand() % 1000 ))) {
                    int tmp = solution.result[i - 1];
                    int tmpIdx = solution.resultIdxIndex[i - 1];
                    solution.result[i -1] = solution.result[i + intervals];
                    solution.result[i + intervals] = tmp;
                    solution.resultIdxIndex[i -1] = solution.resultIdxIndex[i + intervals];
                    solution.resultIdxIndex[i + intervals] = tmpIdx;
                    solution.bestTime = solution.bestTime - originalTime + newTime;
                    solution.currentTemperature *= tempDropRate;
                }*/
            }

        }
        
        
        
        for (int idx = 0; idx < betterTime.size(); idx++){
            if (betterTime[idx] < solution.bestTime){
                solution.bestTime = betterTime[idx];
                solution.result = betterResults[idx];
                solution.resultIdxIndex =  betterResultsIdx[idx];
            }
        }
        
        /*
        //Get the start point and end point travel time of the current solution
        double minTime = preCalculate[solution.resultIdxIndex[solution.resultIdxIndex.size() - 2]][solution.resultIdxIndex[solution.resultIdxIndex.size() - 1]].heuristicTime;
        double minStartTime = preCalculate[solution.resultIdxIndex[0]][solution.resultIdxIndex[1]].heuristicTime;
        
        double originalLastTime = minTime;
        double originalFirstTime = minStartTime;

        //Find the depot with the least travel time from the last drop off point and first pickup point
        for (int i = deliveries.size() *2; i < ids.size(); i ++) {
            if (minTime > preCalculate[solution.resultIdxIndex[solution.resultIdxIndex.size() - 2]][i].heuristicTime) {
                minTime = preCalculate[solution.resultIdxIndex[solution.resultIdxIndex.size() - 2]][i].heuristicTime;
                solution.result[solution.result.size() - 1] = ids[i];
                solution.resultIdxIndex[solution.result.size() - 1] = i;
            }
            
            if (minStartTime > preCalculate[i][solution.resultIdxIndex[1]].heuristicTime){
                minStartTime = preCalculate[i][solution.resultIdxIndex[1]].heuristicTime;
                solution.result[0] = ids[i];
                solution.resultIdxIndex[0] = i;
            }
        }   
        //Update the best time after optimizing first and last points

        bestTime = bestTime - originalLastTime - originalFirstTime + minTime + minStartTime;
         

        solution.bestTime = solution.bestTime - originalLastTime - originalFirstTime + minTime + minStartTime;*/


    }

    std::clock_t end = clock();
    solution.cpuTime = double(end - begin) / CLOCKS_PER_SEC;
    return solution;
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
    bool allDelivered = false;
    
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
                allDelivered = (resultTemp.size() == deliveries.size() * 2 + 1);
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
    
    int totalIntersections = getNumIntersections();
    
    PathNode temp;
    temp.travelTime = -1;
    temp.lastIntersection = -1;
    std::vector <PathNode> allIntersections (totalIntersections, temp);
    std::vector <bool> pathFound(dest.size(), false);
    std::vector <double> timeToReach(dest.size(), 9999999999);
    
    
    //timeToReach.clear();
    /*
    // initialize allIntersections vector
    allIntersections.resize(totalIntersections);
    
    for (int i = 0; i < dest.size(); i ++) {
        pathFound.push_back(false);
        timeToReach.push_back(9999999999);
    }

    for (int i = 0; i < allIntersections.size(); i ++) {
        allIntersections[i].lastIntersection = -1;
        allIntersections[i].travelTime = -1;
    }*/
    
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

CalculateResult twoOptSingle (const CalculateResult& solution, int firstIdx, int secondIdx, 
                       const std::vector <IntersectionIdx> & ids, const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder){
    
    CalculateResult bestSolution(solution);
    
    
    if (solution.resultIdxIndex.size() < 3){
        return bestSolution;
    }
    
    if (firstIdx >= solution.resultIdxIndex.size() || secondIdx >= solution.resultIdxIndex.size() || firstIdx >= secondIdx){
        std::cout << "Invalid input in two opt function" <<std::endl;
        exit (EXIT_FAILURE);
    }
    
    std::vector <int> firstSeg = {solution.resultIdxIndex.begin() + 1, solution.resultIdxIndex.begin() + firstIdx + 1};
    std::vector <int> firstSegReverse(firstSeg.rbegin(), firstSeg.rend());
    
    std::vector <int> secondSeg = {solution.resultIdxIndex.begin() + firstIdx + 1, solution.resultIdxIndex.begin() + secondIdx + 1};
    std::vector <int> secondSegReverse(secondSeg.rbegin(), secondSeg.rend());
    
    std::vector <int> thirdSeg = {solution.resultIdxIndex.begin() + secondIdx + 1, solution.resultIdxIndex.end() - 1};
    std::vector <int> thirdSegReverse(thirdSeg.rbegin(), thirdSeg.rend());
    
    std::vector <std::vector <int>> options = {firstSeg, secondSeg , thirdSeg , firstSegReverse, secondSegReverse, thirdSegReverse};
    
    for (int option1 = 0; option1 < options.size(); option1++){
        bool firstValid = true;
        std::vector <int> firstNewSeg = options[option1];
        
        //first element of the solution can not be a drop off
        if (firstNewSeg[0] % 2 == 1){
            firstValid = false;
        }
        
        if (firstValid) {
            
            //choose second segment
            for (int option2 = 0; option2 < options.size(); option2 ++) {

                bool secondValid = true;
                std::vector <int> secondNewSeg = options[option2];
                
                //can not use the same segment twice
                if (option2 % 3 == option1 % 3) {
                    secondValid = false;
                }
                
                if (secondValid){
                    
                    //choose third segment
                    for (int option3 = 0; option3 < options.size(); option3 ++){
                        
                        bool thirdValid = true;
                        std::vector <int> thirdNewSeg = options[option3];
                        
                        if (option3 % 3 == option1 % 3 || option3 % 3 == option2 % 3){
                            thirdValid = false;
                            
                        }
                        //the last location can not be a pick up
                        else if (thirdNewSeg[thirdNewSeg.size() - 1] % 2 == 0){
                            thirdValid = false;
                        }
                        
                        if (thirdValid){
                            
                            bool finalCheck = true;
                            
                            std::vector <int> newSolution(options[option1]);
                            addVectorToVector(newSolution, options[option2]);
                            addVectorToVector(newSolution, options[option3]);
                            
                            if (newSolution.size() % 2 != 0){
                                std::cout << "New 2 opt solution has a non even solution" << std::endl;
                                exit(EXIT_FAILURE);
                            }
                            
                            std::vector <bool> legal(newSolution.size() / 2, false);
                            double newTotalTime = 0;
                            for (int location = 0; location < newSolution.size() && finalCheck; location++){
                                
                                int idIdx = newSolution[location];
                                
                                //if the location is a pick up
                                if (idIdx % 2 == 0){
                                    
                                    if (idIdx / 2 >= legal.size()){
                                        std::cout << "In two Opt a pick up location's index in ids is outside of pick up range" << std::endl;
                                        exit(EXIT_FAILURE);
                                    }
                                    
                                    legal[idIdx / 2] = true;
                                }else if (idIdx % 2 == 1 && legal[idIdx / 2] == false){
                                    finalCheck = false;
                                }
                                
                                //find the travel from current location to the next location
                                if (finalCheck && location != newSolution.size() - 1){
                                    
                                    int nextIdIdx = newSolution[location + 1];
                                    newTotalTime += preCalculateOriginalOrder[idIdx][nextIdIdx].heuristicTime;
                                }
                            }
                            
                            //the solution is valid
                            if (finalCheck){
                                
                                double bestStartDepotTime = INT_MAX;
                                int startDepotIdx = -1, endDepotIdx = -1;
                                double bestEndDepotTime = INT_MAX;
                                
                                for (int depot = newSolution.size(); depot < preCalculateOriginalOrder.size(); depot++){
                                    
                                    //find closest starting depot
                                    if (preCalculateOriginalOrder[depot][newSolution[0]].heuristicTime < bestStartDepotTime){
                                        
                                        startDepotIdx = depot;
                                        bestStartDepotTime = preCalculateOriginalOrder[depot][newSolution[0]].heuristicTime;
                                    }
                                    
                                    //find closest ending depot
                                    if (preCalculateOriginalOrder[newSolution[newSolution.size() - 1]][depot].heuristicTime < bestEndDepotTime){
                                        
                                        endDepotIdx = depot;
                                        bestEndDepotTime = preCalculateOriginalOrder[newSolution[newSolution.size() - 1]][depot].heuristicTime;
                                    }
                                }
                                
                                if (startDepotIdx == -1 && endDepotIdx == -1){
                                    std::cout << "Did not find the depot in 2 opt solution" << std::endl;
                                    exit(EXIT_FAILURE);
                                }
                                
                                newTotalTime += bestStartDepotTime;
                                newTotalTime += bestEndDepotTime;
                                newSolution.insert(newSolution.begin(), startDepotIdx);
                                newSolution.push_back(endDepotIdx);
                                
                                if (newTotalTime < solution.bestTime){
                                    CalculateResult newResult;
                                    newResult.bestTime = newTotalTime;
                                    newResult.resultIdxIndex = newSolution;
                                    bestSolution = newResult;
                                }
                                
                            }
                            
                        }
                    }
                    
                }
            }
        }
    }
    
    
    //if the solution changes
    if (bestSolution.bestTime != solution.bestTime){
        
        bestSolution.currentTemperature = 0;
        bestSolution.cpuTime = 0;
        
        for (int idx = 0; idx < bestSolution.resultIdxIndex.size(); idx++){
            
            bestSolution.result.push_back(ids[bestSolution.resultIdxIndex[idx]]);
        }
    }
    return bestSolution;
}


CalculateResult twoOptNonChangingSolution(const CalculateResult& currentSolution, const std::vector <IntersectionIdx> & ids, 
        const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder){
    
    if (currentSolution.resultIdxIndex.size() < 5){
        return currentSolution;
    }
    
    CalculateResult bestSolution = currentSolution;
 
    //the first segment must have at least one location and also the second and third segment must have one location
    for (int firstIdx = 1; firstIdx < currentSolution.resultIdxIndex.size() - 3; firstIdx ++){
        
        for (int secondIdx = firstIdx + 1; secondIdx < currentSolution.resultIdxIndex.size() -2; secondIdx++){
            
            auto newSolution = twoOptSingle(currentSolution, firstIdx, secondIdx, ids, preCalculateOriginalOrder);
            
            if (newSolution.bestTime < bestSolution.bestTime){
                bestSolution = newSolution;
            }
        }
    }
    
    return bestSolution;
}

CalculateResult findLocalMinWithTwoOpt (const CalculateResult& currentSolution, const std::vector <IntersectionIdx> & ids, 
        const std::vector<std::vector<WavePoint>> & preCalculateOriginalOrder, double remainingTime){
    
    if (currentSolution.resultIdxIndex.size() < 5){
        return currentSolution;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    bool continueOpt = true;
    
    CalculateResult bestSolution = currentSolution;
    
    do{
        
        auto newSolution = twoOptNonChangingSolution(bestSolution, ids, preCalculateOriginalOrder);
        
        if (newSolution.bestTime < bestSolution.bestTime){
            bestSolution = newSolution;
        }else{
            std::cout << "Reach Local Minimum in two opt" << std::endl;
            continueOpt = false;
        }
        
        auto current = std::chrono::high_resolution_clock::now();
        
        if ((std::chrono::duration_cast<std::chrono::duration<double>>(current - start)).count() > remainingTime - 5){
            std::cout << "Time out in two opt" << std::endl;
            continueOpt = false;
        }
    }while (continueOpt);
    
    return bestSolution;
}