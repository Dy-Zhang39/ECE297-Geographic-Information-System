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
#include "m4.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "global.h"     //include all the global variable
#include <algorithm>
#include <math.h>
#include <map>
#include "OSMDatabaseAPI.h"
#include "dataHandler.h"
#include <bits/stdc++.h>
#include <stdio.h>
#include <ctype.h>
#include <boost/algorithm/string.hpp>





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


std::vector<std::string> cityNames = {
    "Beijing, China", "Cairo, Egypt", "Cape Town, South Africa", "Golden Horseshoe, Canada",
    "Hamilton, Canada", "Hong Kong, China", "Iceland", "Interlaken, Switzerland",
    "London, England", "Moscow, Russia", "New Delhi, India", "New York, USA",
    "Rio De Janeiro, Brazil", "Saint Helena", "Singapore", "Sydney, Australia",
    "Tehran, Iran", "Tokyo, Japan", "Toronto, Canada"
};

//all the global variables in each city
std::vector<City*> cities;
//the index of current map that is being drawn
int currentCityIdx = 0;
std::string mapPathPrefix = "/cad2/ece297s/public/maps/";


// Number of different characters possible.
int CHAR_SIZE = 256;

//for searches with partial name: if partial name is longer than this number, then use additional 3-character index.
int PREFIX_NUM_CHAR = 2;

// Global variables to create index for the third character if exist. Due to the memory limit, anything with the third character
// 1. before SEPARATE_CHAR will be in one index
// 2. between SPEARATE_CHAR and SEPARATE_CHAR_AFTER including SEPARATE_CHAR itself, it will be in another index.
// 3. after and include SEPARATE_CHAR_AFTER will be in the third index
char SEPARATE_CHAR = 'k';
char SEPARATE_CHAR_AFTER = 's';

std::vector<StreetIdx> mostSimilarFirstName;
std::vector<StreetIdx> mostSimilarSecondName;
bool streetDataBaseIsLoaded = false;
bool OSMIsLoaded = false;
bool checkingFirstName = true;
//helper function


bool loadMap(std::string map_streets_database_filename) {
    bool alreadyExist = false;
    std::clock_t start = clock();

    //check if the map is already loaded
    for (int cityIdx = 0; cityIdx < cities.size() && !alreadyExist; cityIdx++){

        if (map_streets_database_filename == cities[cityIdx] -> mapPath){
            alreadyExist = true;
            currentCityIdx = cityIdx;
        }      
    }
    bool load_successful;
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded successfully

    if(!load_successful){
        return load_successful;
    }
    streetDataBaseIsLoaded = true;
    
    //create new global variable for new city
    if(!alreadyExist){
        
        //change to osm file name
        std::string osm_filename = map_streets_database_filename.substr(0, map_streets_database_filename.length() - 12);
    
        load_successful = loadOSMDatabaseBIN(osm_filename + ".osm.bin");
        
        if(!load_successful)
            return load_successful;
        OSMIsLoaded = true;
        
        City* newCity = new City;
        newCity->mapPath = map_streets_database_filename;
        newCity->street = new Street;
        newCity ->streetSegment = new StreetSegment;
        newCity ->intersection = new Intersection;

        cities.push_back(newCity);
        currentCityIdx = cities.size() - 1;     //reset the index to the newest city

    }else{
        std::clock_t end = clock();
        double loadAlready = double(end - start) / CLOCKS_PER_SEC;
        std::cout << "The loading process takes " << loadAlready << "s"<< std::endl;
        return true;
    }

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    
    resizeData();               //resize the global vectors
    streetPartialName();        //pre-load partial name index
    street_Intersection();      //pre-load street intersections
    street_Info();              //pre-load information about street length; street travel time; street segment length
    initializeFeatureBounding();//pre-load the bounding boxes of features
    loadSubways();

    //std::vector <StreetSegmentIdx> rTemp = findPathBetweenIntersections(49067, 7136,16);
    //double tTemp = computePathTravelTime(rTemp, 15.00);
    //std::cout << tTemp << " --Time \n";
    //Extreme Hard
    /*std::vector <DeliveryInf> deliveries = {DeliveryInf(40220, 37954), DeliveryInf(90705, 82446), DeliveryInf(29107, 44932), DeliveryInf(28232, 60062), DeliveryInf(28232, 82070), DeliveryInf(45899, 26959), DeliveryInf(28232, 120267), DeliveryInf(90705, 120353)};
    std::vector <IntersectionIdx>     depots = {26612, 45787, 77377};   
    double turn_penalty = 15.000000000;
    std::vector<CourierSubPath> tt = travelingCourier(deliveries, depots, turn_penalty);
    
    
    std::vector <DeliveryInf> deliveries = 
        //{DeliveryInf(134204, 32615), DeliveryInf(113281, 117913), DeliveryInf(74966, 45024), DeliveryInf(135221, 51749), DeliveryInf(40756, 38216), DeliveryInf(102380, 48326), DeliveryInf(86334, 101772), DeliveryInf(34975, 81132), DeliveryInf(82672, 87076), DeliveryInf(42199, 87321), DeliveryInf(27101, 82634), DeliveryInf(119658, 56035), DeliveryInf(29435, 28262), DeliveryInf(136515, 72388), DeliveryInf(65038, 96006), DeliveryInf(149474, 140389), DeliveryInf(81186, 125686), DeliveryInf(105969, 37007), DeliveryInf(103341, 133821), DeliveryInf(137127, 122694), DeliveryInf(117691, 82535), DeliveryInf(95563, 121164), DeliveryInf(145491, 10229), DeliveryInf(121985, 42614), DeliveryInf(106896, 109313)};
        {DeliveryInf(136850, 88138), DeliveryInf(39515, 41847), DeliveryInf(119109, 129608), DeliveryInf(22090, 86944), DeliveryInf(8544, 111172), DeliveryInf(38141, 13410), DeliveryInf(21231, 53318), DeliveryInf(48074, 90057), DeliveryInf(59, 11768), DeliveryInf(43916, 143499), DeliveryInf(85756, 25980), DeliveryInf(70222, 135906), DeliveryInf(22956, 115427), DeliveryInf(33629, 26367), DeliveryInf(112809, 20083), DeliveryInf(143270, 49610), DeliveryInf(121227, 35188), DeliveryInf(86531, 26189), DeliveryInf(85098, 30182), DeliveryInf(73646, 2930), DeliveryInf(80277, 17269), DeliveryInf(81032, 129117), DeliveryInf(672, 88205), DeliveryInf(116516, 17610), DeliveryInf(107446, 133996), DeliveryInf(141071, 148864), DeliveryInf(8827, 38419), DeliveryInf(117488, 97901), DeliveryInf(107498, 98053), DeliveryInf(12093, 48657), DeliveryInf(47008, 48799), DeliveryInf(54227, 19651), DeliveryInf(64041, 34854), DeliveryInf(54953, 151292), DeliveryInf(64616, 22652), DeliveryInf(89171, 119867), DeliveryInf(45258, 127557), DeliveryInf(8937, 23692), DeliveryInf(67765, 146778), DeliveryInf(111189, 67096), DeliveryInf(122873, 111709), DeliveryInf(3916, 139461), DeliveryInf(131354, 40918), DeliveryInf(80178, 38803), DeliveryInf(5978, 1450), DeliveryInf(40871, 22528), DeliveryInf(52534, 23231), DeliveryInf(83198, 49382), DeliveryInf(139488, 115414), DeliveryInf(97646, 69320), DeliveryInf(58508, 33309), DeliveryInf(107688, 38019), DeliveryInf(91672, 76728), DeliveryInf(32703, 45512), DeliveryInf(52179, 110413), DeliveryInf(896, 8651), DeliveryInf(57666, 100172), DeliveryInf(33902, 49782), DeliveryInf(143540, 134830), DeliveryInf(85060, 110788), DeliveryInf(135213, 86053), DeliveryInf(102955, 77918), DeliveryInf(45004, 13846), DeliveryInf(44988, 87768), DeliveryInf(109048, 144065), DeliveryInf(25808, 87986), DeliveryInf(84003, 141313), DeliveryInf(98589, 93279), DeliveryInf(30178, 6392), DeliveryInf(110518, 109858), DeliveryInf(41215, 108723), DeliveryInf(8259, 62492), DeliveryInf(26577, 67689), DeliveryInf(99433, 112680), DeliveryInf(39611, 2527), DeliveryInf(10908, 19325), DeliveryInf(47142, 135225), DeliveryInf(62190, 110338), DeliveryInf(97404, 56971), DeliveryInf(24152, 329), DeliveryInf(14332, 92974), DeliveryInf(120603, 77149), DeliveryInf(22370, 113335), DeliveryInf(64018, 32092), DeliveryInf(49919, 93123), DeliveryInf(59442, 101748), DeliveryInf(133211, 137225), DeliveryInf(18902, 72747), DeliveryInf(45245, 141709), DeliveryInf(84101, 112766), DeliveryInf(94358, 38210), DeliveryInf(28971, 106487), DeliveryInf(76103, 54553), DeliveryInf(128678, 57370), DeliveryInf(110424, 103984), DeliveryInf(136921, 123671), DeliveryInf(40414, 93925), DeliveryInf(70837, 111091), DeliveryInf(9991, 3959), DeliveryInf(80942, 15016), DeliveryInf(4988, 19973), DeliveryInf(52039, 75387), DeliveryInf(41251, 111254), DeliveryInf(67249, 67229), DeliveryInf(119010, 140778), DeliveryInf(29863, 41545), DeliveryInf(98934, 126905), DeliveryInf(136780, 89961), DeliveryInf(86626, 13628), DeliveryInf(99037, 128133), DeliveryInf(14024, 122897), DeliveryInf(12248, 38754), DeliveryInf(7705, 103846), DeliveryInf(134970, 109399), DeliveryInf(8057, 41908), DeliveryInf(99201, 39071), DeliveryInf(117512, 53729), DeliveryInf(53972, 22913), DeliveryInf(23901, 27014), DeliveryInf(23938, 36187), DeliveryInf(101197, 113365), DeliveryInf(22279, 51961), DeliveryInf(50981, 99705), DeliveryInf(149035, 47786), DeliveryInf(35195, 115602), DeliveryInf(84621, 50569), DeliveryInf(134521, 115110), DeliveryInf(1178, 141862), DeliveryInf(102453, 42766), DeliveryInf(128197, 5136), DeliveryInf(141946, 150050), DeliveryInf(134097, 108596), DeliveryInf(22092, 50279), DeliveryInf(17286, 67966), DeliveryInf(14770, 45099), DeliveryInf(70177, 130961), DeliveryInf(41957, 125175), DeliveryInf(134418, 105922), DeliveryInf(52782, 7166), DeliveryInf(144269, 100995), DeliveryInf(125009, 122091), DeliveryInf(132842, 13859), DeliveryInf(81406, 80716), DeliveryInf(23210, 18087), DeliveryInf(72883, 70710), DeliveryInf(132374, 52514), DeliveryInf(89900, 50187), DeliveryInf(138739, 47309), DeliveryInf(122137, 22146), DeliveryInf(44980, 80656), DeliveryInf(126524, 123498), DeliveryInf(110860, 33517), DeliveryInf(114338, 15940), DeliveryInf(125756, 145316), DeliveryInf(36323, 73346), DeliveryInf(84274, 88110), DeliveryInf(39214, 74235), DeliveryInf(10684, 70412), DeliveryInf(113964, 113540), DeliveryInf(67584, 122694), DeliveryInf(93065, 42916), DeliveryInf(103376, 112967), DeliveryInf(101876, 57186), DeliveryInf(59692, 72362), DeliveryInf(135534, 19465), DeliveryInf(28897, 53275), DeliveryInf(8191, 66478), DeliveryInf(128926, 100613), DeliveryInf(80287, 73681), DeliveryInf(64916, 13685), DeliveryInf(58821, 57410), DeliveryInf(31881, 37949), DeliveryInf(35401, 98954), DeliveryInf(10491, 121748), DeliveryInf(128417, 24597)};
    std::vector <IntersectionIdx> depots = //{10, 38701};
        {23, 90303, 31778, 36604, 117681, 49735, 91760, 119254, 127490, 31038, 95098, 139989, 11470, 101186, 21760, 88294, 20699, 146491, 12665, 116253};

    double turn_penalty = 15.000000000;
    
    std::vector<CourierSubPath> tt = travelingCourier(deliveries, depots, turn_penalty);
    // Extreme hard multi
    deliveries = {DeliveryInf(141634, 127054), DeliveryInf(95416, 20671), DeliveryInf(109835, 43893), DeliveryInf(138416, 109151), DeliveryInf(48116, 122039), DeliveryInf(79938, 56365), DeliveryInf(90926, 3521), DeliveryInf(131063, 43893), DeliveryInf(24857, 133675), DeliveryInf(25981, 15792), DeliveryInf(89283, 140137), DeliveryInf(27925, 131465), DeliveryInf(11418, 102019), DeliveryInf(115213, 61167), DeliveryInf(12201, 89072), DeliveryInf(119680, 125833), DeliveryInf(78513, 22086), DeliveryInf(28016, 40673), DeliveryInf(53733, 102333), DeliveryInf(69495, 133675), DeliveryInf(143838, 105498), DeliveryInf(92505, 135909), DeliveryInf(26361, 142379), DeliveryInf(22307, 127054), DeliveryInf(72295, 46695), DeliveryInf(53434, 86214), DeliveryInf(53434, 125954), DeliveryInf(103416, 84652), DeliveryInf(73295, 56781), DeliveryInf(118383, 67981), DeliveryInf(54782, 106428), DeliveryInf(18075, 110023), DeliveryInf(113638, 133675), DeliveryInf(18075, 138296), DeliveryInf(72295, 66597), DeliveryInf(82041, 21160), DeliveryInf(123128, 60507), DeliveryInf(145746, 99349), DeliveryInf(82092, 3417), DeliveryInf(60106, 87285), DeliveryInf(94837, 61942), DeliveryInf(133611, 108087), DeliveryInf(95199, 91934), DeliveryInf(111188, 98980), DeliveryInf(98045, 83801), DeliveryInf(51708, 134254), DeliveryInf(102979, 63472), DeliveryInf(27925, 135443), DeliveryInf(83804, 59978), DeliveryInf(14215, 76286), DeliveryInf(86975, 150957), DeliveryInf(113638, 42084), DeliveryInf(69495, 59186), DeliveryInf(115213, 23536), DeliveryInf(8805, 50403), DeliveryInf(72295, 17680), DeliveryInf(115213, 22673), DeliveryInf(72295, 102582), DeliveryInf(69495, 53303), DeliveryInf(111183, 43893), DeliveryInf(130568, 100897), DeliveryInf(89646, 115510), DeliveryInf(132954, 125954), DeliveryInf(144599, 15792), DeliveryInf(72723, 55008), DeliveryInf(13704, 1255), DeliveryInf(87461, 10241), DeliveryInf(47239, 84703), DeliveryInf(61960, 147035), DeliveryInf(18075, 48832), DeliveryInf(17941, 84703), DeliveryInf(107, 59186), DeliveryInf(48166, 109973), DeliveryInf(57835, 83801), DeliveryInf(148375, 61563), DeliveryInf(113638, 108706), DeliveryInf(111188, 22009), DeliveryInf(73295, 17186), DeliveryInf(27373, 61764), DeliveryInf(69495, 125954), DeliveryInf(14215, 81139), DeliveryInf(14215, 69498), DeliveryInf(45270, 79133), DeliveryInf(4608, 116826), DeliveryInf(18075, 6620), DeliveryInf(61960, 117462), DeliveryInf(52769, 21234), DeliveryInf(139336, 98980), DeliveryInf(139586, 141853), DeliveryInf(105024, 57462), DeliveryInf(127073, 98954), DeliveryInf(73295, 133558), DeliveryInf(142373, 40673), DeliveryInf(59049, 52523), DeliveryInf(69533, 29198), DeliveryInf(69495, 135909), DeliveryInf(77045, 125954), DeliveryInf(27925, 69544), DeliveryInf(81146, 80764), DeliveryInf(33981, 135594), DeliveryInf(69495, 64076), DeliveryInf(106448, 102144), DeliveryInf(40821, 37810), DeliveryInf(83295, 135909), DeliveryInf(44231, 48929), DeliveryInf(111188, 149341), DeliveryInf(74458, 113090), DeliveryInf(49031, 63472), DeliveryInf(135955, 40673), DeliveryInf(97070, 20671), DeliveryInf(101910, 87999), DeliveryInf(63426, 88208), DeliveryInf(113638, 61563), DeliveryInf(59607, 56365), DeliveryInf(53733, 142859), DeliveryInf(95901, 75436), DeliveryInf(124838, 125954), DeliveryInf(51737, 49915), DeliveryInf(111188, 17186), DeliveryInf(137832, 96143), DeliveryInf(105024, 29621), DeliveryInf(48286, 149951), DeliveryInf(73295, 83801), DeliveryInf(73847, 10415), DeliveryInf(73295, 58342), DeliveryInf(131063, 52547), DeliveryInf(33884, 72325), DeliveryInf(31438, 113213), DeliveryInf(61960, 141853), DeliveryInf(112928, 10498), DeliveryInf(105024, 37414), DeliveryInf(143838, 31856), DeliveryInf(142466, 142710), DeliveryInf(115213, 61563), DeliveryInf(50190, 111059), DeliveryInf(77836, 8599), DeliveryInf(93980, 20651), DeliveryInf(60651, 17186), DeliveryInf(14215, 107829), DeliveryInf(126915, 61596), DeliveryInf(116430, 146433), DeliveryInf(107838, 84703), DeliveryInf(89232, 39794), DeliveryInf(41131, 136155), DeliveryInf(81047, 146700), DeliveryInf(51517, 59978), DeliveryInf(63013, 134587), DeliveryInf(99859, 61405), DeliveryInf(111188, 127054), DeliveryInf(74798, 73062), DeliveryInf(115213, 63472), DeliveryInf(48116, 146877), DeliveryInf(73295, 65581), DeliveryInf(126450, 15792), DeliveryInf(113638, 17186), DeliveryInf(53733, 23304), DeliveryInf(60039, 56365), DeliveryInf(27728, 35536), DeliveryInf(146419, 142859), DeliveryInf(101399, 17427), DeliveryInf(69495, 20651), DeliveryInf(112163, 50403), DeliveryInf(72295, 138241), DeliveryInf(48286, 1878), DeliveryInf(140551, 83801), DeliveryInf(5053, 15598), DeliveryInf(103572, 98980), DeliveryInf(36856, 78633), DeliveryInf(93524, 72325), DeliveryInf(72841, 146700), DeliveryInf(65263, 145043), DeliveryInf(115213, 127054), DeliveryInf(113638, 28111), DeliveryInf(42232, 134915), DeliveryInf(3757, 133675), DeliveryInf(98529, 141339), DeliveryInf(61974, 66173), DeliveryInf(55336, 39213), DeliveryInf(53733, 51364), DeliveryInf(111188, 65043), DeliveryInf(53434, 376), DeliveryInf(138126, 119087), DeliveryInf(89923, 114072), DeliveryInf(96053, 143586), DeliveryInf(77239, 97009), DeliveryInf(131063, 55457), DeliveryInf(6085, 50403), DeliveryInf(61960, 35081), DeliveryInf(108571, 149341), DeliveryInf(117878, 86796), DeliveryInf(140601, 10955), DeliveryInf(68383, 60869), DeliveryInf(48116, 111967), DeliveryInf(49319, 94882), DeliveryInf(144468, 98980), DeliveryInf(134741, 76673), DeliveryInf(48286, 59186), DeliveryInf(113638, 139587), DeliveryInf(113638, 142727), DeliveryInf(18075, 61563), DeliveryInf(115213, 48261), DeliveryInf(100375, 43893), DeliveryInf(14215, 76286), DeliveryInf(61960, 67183), DeliveryInf(53865, 141853), DeliveryInf(53733, 48929), DeliveryInf(73295, 61563), DeliveryInf(31887, 113483), DeliveryInf(111188, 59576), DeliveryInf(138107, 104853), DeliveryInf(48116, 59978), DeliveryInf(41512, 103362), DeliveryInf(18075, 35024), DeliveryInf(14215, 41015), DeliveryInf(88039, 41304), DeliveryInf(80957, 122376), DeliveryInf(53434, 120790), DeliveryInf(132065, 79057), DeliveryInf(39507, 48929), DeliveryInf(25934, 72325), DeliveryInf(88539, 13495), DeliveryInf(5643, 131465), DeliveryInf(36907, 141853), DeliveryInf(129612, 35844), DeliveryInf(133614, 131318), DeliveryInf(111188, 83801), DeliveryInf(79017, 103362), DeliveryInf(59397, 123941), DeliveryInf(111188, 59186), DeliveryInf(115213, 21193), DeliveryInf(9171, 48929), DeliveryInf(33981, 15792), DeliveryInf(14215, 3695), DeliveryInf(113373, 20651), DeliveryInf(48116, 131036), DeliveryInf(73295, 117860), DeliveryInf(72295, 60507), DeliveryInf(69495, 14854), DeliveryInf(45551, 100027), DeliveryInf(131063, 65592), DeliveryInf(145746, 128875), DeliveryInf(115213, 109843), DeliveryInf(5064, 59186), DeliveryInf(68219, 90765), DeliveryInf(26278, 24029), DeliveryInf(44888, 131465), DeliveryInf(88601, 148267), DeliveryInf(138739, 59978), DeliveryInf(86760, 10415), DeliveryInf(117987, 131465), DeliveryInf(64949, 1878), DeliveryInf(72295, 3609), DeliveryInf(1813, 41407), DeliveryInf(122371, 24561), DeliveryInf(33981, 72691), DeliveryInf(53733, 41494), DeliveryInf(69495, 72325), DeliveryInf(48116, 131465), DeliveryInf(14215, 15792), DeliveryInf(77486, 22009)};
    depots = {27, 103203, 123011, 20161, 112819};

    tt = travelingCourier(deliveries, depots, turn_penalty);
    // Hard
    deliveries = {DeliveryInf(134309, 12549), DeliveryInf(70077, 116527), DeliveryInf(50598, 138497), DeliveryInf(114749, 139057), DeliveryInf(144736, 131725), DeliveryInf(125464, 115497), DeliveryInf(32317, 92094), DeliveryInf(63694, 5503), DeliveryInf(113763, 47015), DeliveryInf(11108, 83481), DeliveryInf(87153, 89090), DeliveryInf(75474, 50466), DeliveryInf(136819, 89235), DeliveryInf(97700, 17097), DeliveryInf(9091, 27572), DeliveryInf(49683, 124009), DeliveryInf(29465, 146120), DeliveryInf(16878, 130519), DeliveryInf(94887, 114917), DeliveryInf(70858, 115629), DeliveryInf(36474, 52059), DeliveryInf(31690, 30193), DeliveryInf(138232, 27681), DeliveryInf(148245, 88762), DeliveryInf(53464, 78521), DeliveryInf(102473, 121308), DeliveryInf(82599, 147449), DeliveryInf(114469, 90306), DeliveryInf(71324, 25945), DeliveryInf(145875, 60419), DeliveryInf(10654, 14447), DeliveryInf(82270, 35746), DeliveryInf(146436, 129059), DeliveryInf(95739, 125804), DeliveryInf(144781, 26707), DeliveryInf(120266, 79420), DeliveryInf(126384, 48274), DeliveryInf(24695, 38734), DeliveryInf(17908, 10473), DeliveryInf(7256, 122900), DeliveryInf(86145, 30422), DeliveryInf(54359, 31561), DeliveryInf(67399, 146690), DeliveryInf(143962, 23193), DeliveryInf(51138, 42637), DeliveryInf(150157, 70239), DeliveryInf(135889, 81376), DeliveryInf(72768, 79021), DeliveryInf(57816, 102471), DeliveryInf(54403, 4831), DeliveryInf(67927, 3466), DeliveryInf(67619, 137899), DeliveryInf(151370, 102251), DeliveryInf(103348, 37524), DeliveryInf(131114, 10461), DeliveryInf(84740, 8820), DeliveryInf(121665, 130081), DeliveryInf(55111, 2381), DeliveryInf(145952, 124749), DeliveryInf(40202, 36838), DeliveryInf(105676, 31369), DeliveryInf(12665, 87984), DeliveryInf(122532, 130880), DeliveryInf(145581, 90374), DeliveryInf(24806, 4688), DeliveryInf(19761, 85361), DeliveryInf(143961, 49067), DeliveryInf(64132, 7136), DeliveryInf(147983, 142498), DeliveryInf(102290, 122349), DeliveryInf(56218, 135923), DeliveryInf(28294, 21571), DeliveryInf(132386, 84536), DeliveryInf(42314, 117240), DeliveryInf(135794, 135609), DeliveryInf(71023, 27321), DeliveryInf(79014, 1099), DeliveryInf(34080, 98597), DeliveryInf(74734, 1455), DeliveryInf(73316, 139297), DeliveryInf(53584, 139432), DeliveryInf(41731, 19590), DeliveryInf(124944, 74340), DeliveryInf(24470, 72513), DeliveryInf(81048, 33400), DeliveryInf(124344, 140589), DeliveryInf(137559, 116132), DeliveryInf(73657, 16849), DeliveryInf(109439, 146764), DeliveryInf(43986, 134916), DeliveryInf(106680, 111928), DeliveryInf(148957, 83453), DeliveryInf(150907, 81386), DeliveryInf(145536, 87241), DeliveryInf(86575, 46154), DeliveryInf(58131, 113093), DeliveryInf(7194, 100548), DeliveryInf(142386, 136851), DeliveryInf(112921, 18900), DeliveryInf(77019, 115997)};
    depots = {17, 64502, 1026, 69492, 127404, 143891, 87216, 150201, 47718, 108862};
    tt = travelingCourier(deliveries, depots, turn_penalty);
    // Hard multi
    deliveries = {DeliveryInf(150557, 95305), DeliveryInf(143341, 95186), DeliveryInf(96042, 72091), DeliveryInf(74015, 144050), DeliveryInf(137363, 92346), DeliveryInf(39415, 97191), DeliveryInf(149213, 147255), DeliveryInf(93676, 63128), DeliveryInf(144777, 26110), DeliveryInf(69950, 57929), DeliveryInf(20957, 139197), DeliveryInf(149933, 77216), DeliveryInf(84115, 105370), DeliveryInf(6777, 46480), DeliveryInf(56524, 37642), DeliveryInf(58871, 103580), DeliveryInf(103812, 93771), DeliveryInf(40300, 45488), DeliveryInf(1378, 149908), DeliveryInf(76433, 42160), DeliveryInf(72040, 51164), DeliveryInf(111281, 115432), DeliveryInf(69950, 130487), DeliveryInf(90616, 70423), DeliveryInf(54971, 137643), DeliveryInf(69950, 135854), DeliveryInf(10661, 108413), DeliveryInf(99660, 80879), DeliveryInf(147237, 31134), DeliveryInf(20957, 6358), DeliveryInf(134233, 78674), DeliveryInf(103374, 138709), DeliveryInf(130077, 120304), DeliveryInf(115931, 8564), DeliveryInf(139270, 114659), DeliveryInf(121319, 134314), DeliveryInf(28476, 50562), DeliveryInf(57636, 54658), DeliveryInf(118731, 36989), DeliveryInf(33958, 109490), DeliveryInf(81512, 137425), DeliveryInf(62081, 106472), DeliveryInf(117240, 86134), DeliveryInf(71925, 3798), DeliveryInf(20957, 35968), DeliveryInf(111462, 135137), DeliveryInf(84696, 120662), DeliveryInf(60718, 38781), DeliveryInf(33953, 120662), DeliveryInf(66915, 36966), DeliveryInf(90049, 66029), DeliveryInf(103498, 137699), DeliveryInf(96652, 28523), DeliveryInf(51832, 33199), DeliveryInf(22025, 103594), DeliveryInf(65231, 119356), DeliveryInf(129067, 3159), DeliveryInf(13359, 137699), DeliveryInf(22441, 104689), DeliveryInf(22931, 117358), DeliveryInf(85228, 137699), DeliveryInf(85029, 123116), DeliveryInf(54202, 45029), DeliveryInf(57379, 116254), DeliveryInf(70143, 71101), DeliveryInf(106225, 31674), DeliveryInf(69950, 1706), DeliveryInf(74851, 47570), DeliveryInf(51384, 29630), DeliveryInf(58871, 12552), DeliveryInf(10553, 19486), DeliveryInf(112070, 62283), DeliveryInf(5448, 50859), DeliveryInf(28996, 65596), DeliveryInf(79836, 23603), DeliveryInf(7152, 136977), DeliveryInf(113977, 103536), DeliveryInf(20957, 120662), DeliveryInf(83671, 120662), DeliveryInf(94499, 29595), DeliveryInf(95627, 87322), DeliveryInf(66126, 9150), DeliveryInf(20458, 105162), DeliveryInf(84398, 148659), DeliveryInf(68776, 124568), DeliveryInf(3789, 120662), DeliveryInf(80420, 62280), DeliveryInf(144707, 54542), DeliveryInf(71594, 57784), DeliveryInf(98713, 137699), DeliveryInf(116696, 66574), DeliveryInf(60227, 104890), DeliveryInf(58871, 7531), DeliveryInf(120403, 11019), DeliveryInf(20957, 131557), DeliveryInf(69379, 6604), DeliveryInf(122050, 65070), DeliveryInf(13556, 76565), DeliveryInf(151137, 137699), DeliveryInf(53048, 77679)};
    depots = {20, 77402, 92258};    
    tt = travelingCourier(deliveries, depots, turn_penalty);
    */
        
    std::clock_t end = clock();
    double loadAlready = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "The loading process takes " << loadAlready << "s"<< std::endl;
    return load_successful;
    
    
}

//calculate x coordinate from longitude
double xFromLon(double lon){
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(cities[currentCityIdx] -> avgLat * kDegreeToRadian);
}

//calculate y coordinate from latitude
double yFromLat(double lat){
    return lat * kDegreeToRadian* kEarthRadiusInMeters;
}

//calculate longitude from x coordinate
double lonFromX(double x){
    return x/(kDegreeToRadian * kEarthRadiusInMeters * std::cos(cities[currentCityIdx] -> avgLat * kDegreeToRadian));
}

//calculate latitude from y coordinate
double latFromY(double y){
    return y/(kDegreeToRadian* kEarthRadiusInMeters);
}

void resizeData(){
    // initialize vector<double> streetSegLength;
    cities[currentCityIdx]->street->streetLength.resize(getNumStreets());
    cities[currentCityIdx]->street->streetSegments.resize(getNumStreets());
    // Load index vectors used to quick search street names
    // Load street name into the vector using first 1 characters as index for cities[currentCityIdx]->street->streetNamesOneChar,
    // Load street name into the vector using first 2 characters as index for cities[currentCityIdx]->street->streetNamedTwoChar,
    // Load street name into the vector using first 3 characters as index for cities[currentCityIdx]->street->streetNamesThreeChar.
    cities[currentCityIdx]->street->streetNames.resize(getNumStreets());
    cities[currentCityIdx]->street->streetNamesOneChar.resize(CHAR_SIZE);
    cities[currentCityIdx]->street->streetNamesTwoChar.resize(CHAR_SIZE * CHAR_SIZE);
    cities[currentCityIdx]->street->streetNamesThreeChar.resize(CHAR_SIZE * CHAR_SIZE * 3);
    
    // Find streets length and their corresponding travel time
    cities[currentCityIdx]->streetSegment->streetSegLength.resize(getNumStreetSegments());
    cities[currentCityIdx]->streetSegment->streetSegTravelTime.resize(getNumStreetSegments());
    cities[currentCityIdx]->streetSegment->streetSegPoint.resize(getNumStreetSegments());
    
}

// initialize the bounding coordinates of all features into vectors
void initializeFeatureBounding() {
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double minX = xFromLon(getFeaturePoint(featureID, 0).longitude());
        double maxX = minX;
        double minY = yFromLat(getFeaturePoint(featureID, 0).latitude());
        double maxY = minY;
        
        // find bounding box of feature
        for (int pt = 1; pt < getNumFeaturePoints(featureID); pt++){
            double x, y;
            x = xFromLon(getFeaturePoint(featureID, pt).longitude());
            y = yFromLat(getFeaturePoint(featureID, pt).latitude());

            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
        Feature keyPoints(maxY, minY, minX, maxX, findFeatureArea(featureID));
        // record the bounding boxes for each feature
        cities[currentCityIdx]->featurePts.push_back(keyPoints);
    }
}

void loadSubways() {

    std::vector<const OSMRelation *> osm_subway_lines;
    
    //step length for looping and searching with OSM nodes
    int stepLength = 1000;
    
    std::vector<OSMID> osm_nodes;
    
    std::vector<ezgl::point2d> stationCoordindate;
    std::vector<std::string> stationName;
    std::string lineColor;                  //color of the subway line
    
    // Create index for the nodes by grouping them
    for (unsigned k = 0; k < getNumberOfNodes(); k++) {
        const OSMNode  *newNode = getNodeByIndex(k);
        if (k % stepLength == 0) {
            osm_nodes.push_back(newNode->id());
        }
    }

    // Loop through all OSM relations
    for (unsigned i = 0; i < getNumberOfRelations(); i++) {
        const OSMRelation *currRel = getRelationByIndex(i);

        // Check the tag of the currRel
        for (unsigned j = 0; j < getTagCount(currRel); j++) {
            std::pair<std::string, std::string> tagPair = getTagPair(currRel, j);

            // Push relations with the route=subway tag
            if (tagPair.first == "route" && tagPair.second == "subway") {
                osm_subway_lines.push_back(currRel);
                break;
            }
        }
    }

    
    // For each subway line (relation), get its name, color, and members
    for (unsigned i = 0; i < osm_subway_lines.size(); i++) {

        // Get subway line color and name
        for (unsigned j = 0; j < getTagCount(osm_subway_lines[i]); j++) {
            std::pair<std::string, std::string> tagPair = getTagPair(osm_subway_lines[i], j);
            
            if (tagPair.first == "colour") {
                lineColor = tagPair.second;
            }
        }

        // Get relation members
        std::vector<TypedOSMID> route_members = getRelationMembers(osm_subway_lines[i]); 
        
        // Grab subway names
      
        for(unsigned j = 0; j < route_members.size(); j++) {

            // A member of type node represents a subway station
            if(route_members[j].type() == TypedOSMID::Node) {

                const OSMNode *currNode = nullptr;
                unsigned indexRange = 0;
                
                //find the osm_nodes (section including [stepLength] of nodes) that contains the OSM Node for the station
                for (unsigned k = 0; k < osm_nodes.size(); k++) {
                    
                    if (route_members[j] < osm_nodes[k]) {
                        indexRange = k;
                        break;
                    }
                }
                unsigned startIdx = 0, endIdx = getNumberOfNodes();      
                
                //loop through all OSM Nodes in the found osm_nodes section
                if ((indexRange - 1) * stepLength > 0) 
                    startIdx = (indexRange - 1) * stepLength;
                
                if ((indexRange + 1) * stepLength < endIdx) 
                    endIdx = (indexRange + 1) * stepLength ;

                // Node lookup by OSMID
                for (unsigned k = startIdx; k < endIdx; k++) {
                    currNode = getNodeByIndex(k);
                    
                    if (currNode->id() == route_members[j]) {
                        break;
                    }
                }

                // Get the name tag of that node
                for (unsigned k = 0; k < getTagCount(currNode); k++) {
                    std::pair<std::string, std::string> tagPair = getTagPair(currNode, k);
                    LatLon stationCoord = getNodeCoords(currNode);
                    
                    if (tagPair.first == "name") {
                        Subway s;
                        double x = xFromLon(stationCoord.longitude());
                        double y = yFromLat(stationCoord.latitude());
                        
                        s.location = {x, y};
                        s.red = 12;
                        s.green = 124;
                        s.blue = 92;

                        if (lineColor.compare("green") == 0) {

                            s.red = 57;
                            s.green = 180;
                            s.blue = 96;

                        } else if (lineColor.compare("purple") == 0) {

                            s.red = 118;
                            s.green = 23;
                            s.blue = 132;
                            
                        } else if (lineColor.compare("yellow") == 0) {

                            s.red = 199;
                            s.green = 182;
                            s.blue = 39;
                            
                        } else if (lineColor.compare("brown") == 0) {

                            s.red = 128;
                            s.green = 0;
                            s.blue = 0;
                            
                        } else if ( lineColor.length() == 7 && lineColor.at(0) == '#') {
                            //convert given HEX color into RGB
                            s.red = std::stoi(lineColor.substr(1,2), nullptr, 16);
                            s.green = std::stoi(lineColor.substr(3,2), nullptr, 16);
  
                        }

                        s.name = tagPair.second;

                        cities[currentCityIdx]->subways.push_back(s);
                        break;
                    }
                }
            }
        }
    }
}

void streetPartialName(){
    
    for (int i = 0; i < getNumStreets(); i++){
        //initialize all the element in the street length to 0 to prevent undefined variable
        cities[currentCityIdx]->street->streetLength[i] = 0;
        
        // Get street name, remove space and convert to lower cases. Got only the first 2 letters.
        std::string streetName = getStreetName(i);
        std::string streetNameSub = "";
        std::string streetFullName;
        
        for (int j = 0; j < streetName.length(); j++) {
            
            if (streetName[j] != ' ' && streetNameSub.length() <= PREFIX_NUM_CHAR) {
                streetNameSub.push_back(tolower(streetName[j]));
            }
            
            if (streetName[j] != ' ') {
                cities[currentCityIdx]->street->streetNames[i].push_back(tolower(streetName[j]));
            }
        }

        // Store the street id into the index vector: streetNamesOneChar, streetNamesTwoChar
        if (streetNameSub.length() > 0)
            cities[currentCityIdx]->street->streetNamesOneChar[tolower(streetNameSub[0])].push_back(i);
        
        if (streetNameSub.length() > 1)
            cities[currentCityIdx]->street->streetNamesTwoChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);

        if (streetNameSub.length() > PREFIX_NUM_CHAR) {
            if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                cities[currentCityIdx]->street->streetNamesThreeChar[tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])].push_back(i);
            } else if (tolower(streetNameSub[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE].push_back(i);
            } else {
                cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetNameSub[0]) * CHAR_SIZE + tolower(streetNameSub[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE].push_back(i);
            }
        }
    }
    
    
}
void street_Intersection(){
    cities[currentCityIdx]->intersection->intersectionStreetSegments.resize(getNumIntersections()); //create empty vector for each intersection
    cities[currentCityIdx]->street->streetIntersections.resize(getNumStreets());
    cities[currentCityIdx] -> intersection -> intersectionInfo.resize(getNumIntersections());
    
    double maxLat = getIntersectionPosition(0).latitude();
    double minLat = maxLat;
    double maxLon = getIntersectionPosition(0).longitude();
    double minLon = maxLon;
    
    //iterate through all intersections
    for(int intersectionID = 0; intersectionID < getNumIntersections(); ++intersectionID){
        
        cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position = getIntersectionPosition(intersectionID);
        cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].name = getIntersectionName(intersectionID);

        maxLat = std::max(maxLat, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.latitude());
        minLat = std::min(minLat, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position. latitude());
        maxLon = std::max(maxLon, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.longitude());
        minLon = std::min(minLon, cities[currentCityIdx] -> intersection -> intersectionInfo[intersectionID].position.longitude());
        
        for(int i = 0; i < getNumIntersectionStreetSegment(intersectionID); ++i) {         
        
            //iterate through all segments at intersection
            int streetSegID = getIntersectionStreetSegment(intersectionID, i);
            auto streetSegInfo = getStreetSegmentInfo(streetSegID);
            auto streetID = streetSegInfo.streetID;

            if (cities[currentCityIdx]->street->streetIntersections[streetID].size() == 0 || cities[currentCityIdx]->street->streetIntersections[streetID].back() < intersectionID) {
                cities[currentCityIdx]->street->streetIntersections[streetID].push_back(intersectionID);              //save the intersection to the street it belongs to
            }
            
            cities[currentCityIdx]->intersection->intersectionStreetSegments[intersectionID].push_back(streetSegID);  //save segments connected to intersection
        }
    }
    
    cities[currentCityIdx] -> maxLat = maxLat;
    cities[currentCityIdx] -> minLat = minLat;
    cities[currentCityIdx] -> maxLon = maxLon;
    cities[currentCityIdx] -> minLon = minLon;
    cities[currentCityIdx] -> avgLat = (maxLat + minLat)/2;
            
}

//street length; street travel time; street segment length
void street_Info(){
    
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        double streetSegmentLength = 0;
        struct StreetSegmentInfo streetSegmentID = getStreetSegmentInfo(street_segment_id);
        int twoTerminals = 2;
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id].resize(twoTerminals + streetSegmentID.numCurvePoints);
        //find the starting and ending position of given street segment
        auto from = getIntersectionPosition(streetSegmentID.from);
        auto to = getIntersectionPosition(streetSegmentID.to);
        
        //put the street segment into the street that it belong to
        cities[currentCityIdx]->street->streetSegments[streetSegmentID.streetID].push_back(street_segment_id);  
        
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][0] = from;
        cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][twoTerminals + streetSegmentID.numCurvePoints - 1] = to;
        
        //find curve points on the given street segment (more than one curve point)
        if (streetSegmentID.numCurvePoints > 1) {

            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            //calculate distance between start point and first curve point
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints);
            
            cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][1] = getStreetSegmentCurvePoint(street_segment_id, 0);
            int i = 1;
            for (; i < streetSegmentID.numCurvePoints; i++) {
                std::pair <LatLon, LatLon> curvePoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), getStreetSegmentCurvePoint(street_segment_id, i));
                
                cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][i+1] = getStreetSegmentCurvePoint(street_segment_id, i);
                //add the distance between each curve points
                streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(curvePoints);
            }
            
            //check if the cities[currentCityIdx]->streetSegment vector is properly filled
            if(cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][i+1].latitude()!= to.latitude()){
                std::cout<<"STREEET_SEGMENTS not properly filled"<<std::endl;
                exit (-1);
            }
             
            //add the distance between last curve point and end point
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, i - 1), to);
            streetSegmentLength = streetSegmentLength + findDistanceBetweenTwoPoints(lastPoints);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;

        } else if (streetSegmentID.numCurvePoints == 1) { //only has one curve point
            std::pair <LatLon, LatLon> firstPoints(from, getStreetSegmentCurvePoint(street_segment_id, 0));
            std::pair <LatLon, LatLon> lastPoints(getStreetSegmentCurvePoint(street_segment_id, 0), to);
            
            cities[currentCityIdx]->streetSegment->streetSegPoint[street_segment_id][1] = getStreetSegmentCurvePoint(street_segment_id, 0);
            
            // add distance from start point to the only curve point to the end point 
            streetSegmentLength = findDistanceBetweenTwoPoints(firstPoints) + findDistanceBetweenTwoPoints(lastPoints);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;

        } else {
            //straight segment
            std::pair <LatLon, LatLon> points(from, to);
            //calculate distance between start and end point
            streetSegmentLength = findDistanceBetweenTwoPoints(points);
            cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id] = streetSegmentLength;
        }
        
        
        //find street segment travel time
        double speedLimit = streetSegmentID.speedLimit;
        cities[currentCityIdx]->streetSegment->streetSegTravelTime[street_segment_id] = streetSegmentLength / speedLimit;
        
        //find street length
        cities[currentCityIdx]->street->streetLength[streetSegmentID.streetID] += streetSegmentLength;
    }
}
    
    
//clear all the global data structure to prevent it get bigger and bigger    
void closeMap() {

    //Clean-up your map related data structures here
    closeDataBase();
    for(int cityIdx = 0; cityIdx < cities.size(); cityIdx++){
        delete (cities[cityIdx]-> street);
        delete (cities[cityIdx]-> streetSegment);
        delete (cities[cityIdx]-> intersection);
        delete cities[cityIdx];
    }
    
    cities.clear(); 
}

//close the database
void closeDataBase(){
    
    if (streetDataBaseIsLoaded){
        closeStreetDatabase();
        streetDataBaseIsLoaded = false;
    }
        
    if (OSMIsLoaded){
        closeOSMDatabase();
        OSMIsLoaded = false;
    }
        
    
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
        if (streetPrefix.length() > PREFIX_NUM_CHAR) {
            if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR)) {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
            } else if (tolower(streetPrefix[PREFIX_NUM_CHAR]) < tolower(SEPARATE_CHAR_AFTER)) {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE];
            } else {
                adjustedNameList = cities[currentCityIdx]->street->streetNamesThreeChar[(tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])) + CHAR_SIZE * CHAR_SIZE + CHAR_SIZE * CHAR_SIZE];
            }
        } else if (streetPrefix.length() > 1) {
            adjustedNameList = cities[currentCityIdx]->street->streetNamesTwoChar[tolower(streetPrefix[0]) * CHAR_SIZE + tolower(streetPrefix[1])];
        } else {
            adjustedNameList = cities[currentCityIdx]->street->streetNamesOneChar[tolower(streetPrefix[0])];
        }
        
        int leastEdit = INT_MAX;            //numbers of edits requires to change from input name to the similar name
        
        //loop through the street names within the index vector
        for (int i = 0; i < adjustedNameList.size(); i++){

            std::string streetName = getStreetName(adjustedNameList[i]);
            
            //find the more similar name to the input
            int numOfEdit = levenshteinDistance(street_prefix, streetName);
            if (numOfEdit < leastEdit){
                
                if (checkingFirstName){
                    mostSimilarFirstName.clear();
                    mostSimilarFirstName.push_back(adjustedNameList[i]);
                }else{
                    mostSimilarSecondName.clear();
                    mostSimilarSecondName.push_back(adjustedNameList[i]);
                }
                
                leastEdit = numOfEdit;
            }else if (numOfEdit == leastEdit){
                
                if (checkingFirstName){

                    mostSimilarFirstName.push_back(adjustedNameList[i]);
                }else{
                    mostSimilarSecondName.push_back(adjustedNameList[i]);
                }
            }
            
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
    return cities[currentCityIdx]->street->streetLength[street_id];
}

// Return the smallest axis-aligned rectangle that contains all the intersections and curve points of the given street
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    //break the street into intersections, use the first intersection position as min/max LatLon
    std::vector<IntersectionIdx> intersections = findIntersectionsOfStreet(street_id);
    LatLon firstPoint = getIntersectionPosition(intersections[0]);
    LatLon maxPoint = firstPoint;
    LatLon minPoint = firstPoint;
    
    //loop through all intersections and update the min/max LatLon
    for (int i = 0; i < intersections.size(); i++){
        LatLon point = getIntersectionPosition(intersections[i]);
        
        minPoint = findMaxMin(minPoint, point, "min");
        maxPoint = findMaxMin(maxPoint, point, "max");

       
    }
    //loop through the all the Street Segments
    for (int j = 0; j < getNumStreetSegments(); j++) {
        StreetSegmentInfo ss_info = getStreetSegmentInfo(j);
        LatLon point;
        //locate the street id and loop through all the related curve points
        if (ss_info.streetID == street_id) {

            for (int k = 0; k < ss_info.numCurvePoints; k++) {
                point = getStreetSegmentCurvePoint(j, k);

                minPoint = findMaxMin(minPoint, point, "min");
                maxPoint = findMaxMin(maxPoint, point, "max");
            }
        }
    }

    //use the min/max latitude and longitude to create LatLonBounds
    LatLonBounds box;
    box.min = minPoint;
    box.max = maxPoint;
    
    return box;
  
}

// Helper function to find the min/max LatLon point
LatLon findMaxMin(LatLon point, LatLon current, std::string method) {
    
    if (method.compare("max") == 0) {
        current = LatLon (std::max(current.latitude(), point.latitude()),
            std::max(current.longitude(), point.longitude()));
    } else {
        current = LatLon (std::min(current.latitude(), point.latitude()),
            std::min(current.longitude(), point.longitude()));
    }
    
    return current;
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
    
    return cities[currentCityIdx]->street->streetIntersections[street_id];

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
    return cities[currentCityIdx]->streetSegment->streetSegLength[street_segment_id];
    
}



// Returns the travel time to drive from one end of a street segment in 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    
    if (street_segment_id < 0 || street_segment_id >= getNumStreetSegments()) {
        return 0;
    }
    
    return cities[currentCityIdx]->streetSegment->streetSegTravelTime[street_segment_id];

}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection_id = -1;
    double distance=2*M_PI*kEarthRadiusInMeters;
    
    int numOfIntersections = cities[currentCityIdx]-> intersection -> intersectionInfo.size();
    for(int i = 0; i < numOfIntersections; i++) {
        std::pair <LatLon, LatLon> points (my_position, getIntersectionPosition(i));
        
        if(findDistanceBetweenTwoPoints(points) < distance){
            distance = findDistanceBetweenTwoPoints(points);
            closestIntersection_id = i;
        }
    }
    return closestIntersection_id;
}

IntersectionIdx findClosestIntersection(LatLon my_position, std::vector<IntersectionIdx> lists){
    IntersectionIdx closestIntersection_id = -1;
    double distance=2*M_PI*kEarthRadiusInMeters;
    
    
    for(int i = 0; i < lists.size(); i++) {
        std::pair <LatLon, LatLon> points (my_position, getIntersectionPosition(lists[i]));
        
        if(findDistanceBetweenTwoPoints(points) < distance){
            distance = findDistanceBetweenTwoPoints(points);
            closestIntersection_id = lists[i];
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
    
    return cities[currentCityIdx]->intersection->intersectionStreetSegments[intersection_id];
    
}

//an algorithm that gives how much edit I need to let the two string to be the same
int levenshteinDistance(std::string first, std::string second){
    int rows = first.length() + 1;
    int cols = second.length() + 1;
    
    std::vector<std::vector<int>> matrix;
    
    matrix.resize(rows);
    
    for (int row = 0; row < rows; row++){
        matrix[row].resize(cols);
        for (int col = 0; col < cols; col++){
            matrix[row][col] = 0;
        }      
    }
    
    for (int row = 1; row < rows; row++){
        for(int col = 1; col < cols; col++){
            matrix[row][0] = row;
            matrix[0][col] = col;
        }
    }
    
    for (int col = 1; col < cols; col++){
        for (int row = 1; row < rows; row++){
            
            int adder;
            
            if (first[row - 1] == second [col - 1]){
                adder = 0;
            }else{
                adder = 1;
            }
            
            int temp =  std::min(matrix[row - 1][col] + 1, matrix[row][col - 1] + 1);
            temp = std::min(temp, matrix[row - 1][col - 1] + adder);                         
            matrix[row][col] = temp;
        }
    }
    
    return matrix[rows - 1][cols -1];
    
}