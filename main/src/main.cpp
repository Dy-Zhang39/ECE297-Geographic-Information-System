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
#include <string>

#include "m1.h"
#include "global.h"
#include "m2.h"
//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

void getCityName(std::string* s);        //get city name from the user
void clearAllMap();                      //clear all the global variable in citys
std::vector<std::string> cityNames = {
    "beijing_china", "cairo_egypt", "cape-town_south-africa", "golden-horseshoe_canada",
    "hamilton_canada", "hong-kong_china", "iceland", "interlaken_switzerland",
    "london_england", "moscow_russia", "new-delhi_india", "new-york_usa",
    "rio-de-janeiro_brazil", "saint-helena", "singapore", "sydney_australia",
    "tehran_iran", "tokyo_japan", "toronto_canada"
};
//The default map to load if none is specified

std::string default_mapPath = "/cad2/ece297s/public/maps/tokyo_japan";
std::string mapPath_prefix = "/cad2/ece297s/public/maps/";
std::vector<City*> citys;
bool isFinished = false;
int currentCityIdx;

// The start routine of your program (main) when you are running your standalone
// mapper program. This main routine is *never called* when you are running 
// ece297exercise (the unit tests) -- those tests have their own main routine
// and directly call your functions in /libstreetmap/src/ to testtext to replacetext to replace them.
// Don't write any code in this file that you want run by ece297exerise -- it 
// will not be called!
int main(int argc, char** argv) {

    std::string mapPath;
    
   
    
    if(argc == 1) {
        //Use a default map
        getCityName(&mapPath);
        //mapPath = default_mapPath;
    } else if (argc == 2) {
        //Get the map from the command line
        mapPath = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }
    
    
    while (!isFinished){
        bool load_success = loadMap(mapPath);
        
        if(!load_success) {
            
            std::cerr << "Failed to load map '" << mapPath << "'\n";
            return ERROR_EXIT_CODE;
        }
        
        std::cout << "Successfully loaded map '" << mapPath << "'\n";
        drawMap();
        closeMap();
    }
    
    std::cout << "Closing all the map";
    clearAllMap();
    
    /*
    //Load the map and related data structures
    bool load_success = loadMap(mapPath);
    if(!load_success) {
        std::cerr << "Failed to load map '" << mapPath << "'\n";
        return ERROR_EXIT_CODE;
    }

    std::cout << "Successfully loaded map '" << mapPath << "'\n";

    //You can now do something with the map data
    
    drawMap();
    
    //Clean-up the map data and related data structures
    std::cout << "Closing map\n";
    closeMap(); 
     */
    return SUCCESS_EXIT_CODE;
}

void getCityName(std::string* s){
    std::string name;
    std::cout << "Please enter the name of the city: ";
    std::cin >> name;
//<<<<<<< HEAD
//    *s = mapPath_prefix + name + ".streets.bin";
//    
//=======

    *s = mapPath_prefix + name;
}

void clearAllMap(){
    for(int cityIdx = 0; cityIdx < citys.size(); cityIdx++){
        delete citys[cityIdx]-> street;
        delete citys[cityIdx]->streetSegment;
        delete citys[cityIdx]-> intersection;
        delete citys[cityIdx];
    }
    
    citys.clear();
}