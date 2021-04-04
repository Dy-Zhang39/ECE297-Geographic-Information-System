/*
 * This file store some of the helper function implementation
 */


#include "m2.h"
#include "m3.h"
#include "drawMap.h"

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "m1.h"
#include "global.h"
#include "dataHandler.h"
#include "callBack.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <chrono>

#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

extern std::vector<City*> cities;
extern int currentCityIdx;
extern std::vector<std::string> cityNames;
extern std::string mapPathPrefix;
extern std::vector<IntersectionIdx> previousHighlight;
extern ezgl::color textColor;

//set the height to width ratio of a current city
void initializeCurrentWorldRatio(){
    
    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    cities[currentCityIdx]->worldRatio = (yFromLat(maxLat) - yFromLat(minLat))/(xFromLon(maxLon) - xFromLon(minLon));
}

//input all the city name to the map bar
void importNameToTheBar(GtkComboBoxText* bar){
    
    for(int idx = 0; idx < cityNames.size(); idx++){
        
        std::string name = cityNames[idx];
        gtk_combo_box_text_append_text(bar, name.c_str());
    }
}

std::string separateNamesByCommas(std::string locationName){
    
    std::string nameByCommas;
    for (int idx = 0; idx < locationName.size(); idx++){
        
        //change " &" to ","
        if (idx != locationName.size() - 1 && locationName[idx] == ' ' && locationName[idx + 1] == '&'){
            nameByCommas.push_back(',');
        }else if (locationName[idx] != '&'){
            nameByCommas.push_back(locationName[idx]);
        }
    }
    
    return nameByCommas;
}

std::pair <std::string, std::string> getStreetNames(std::string text){
    
    std::string firstStreet, secondStreet;        
    
    bool firstFinished = false;         //finished reading the first street  
    
    
    //split the input into two street name
    for (auto iterator = text.begin(); iterator != text.end(); ){
        
        if (!firstFinished){
            
            if (*iterator != ',' && *iterator != ' '){
                
                firstStreet.push_back(tolower(*iterator));
            }else if (*iterator == ','){
                
                firstFinished = true;
            }
            
        }else{
            
            if (*iterator != ' ' && *iterator != ','){
                
                secondStreet.push_back(tolower(*iterator));
            }else if (*iterator == ','){
                
                iterator = text.end();
            }    
        }
        
        if (iterator != text.end()){
            iterator++;
        }
    }
    
    return std::make_pair(firstStreet, secondStreet);
}

std::vector<IntersectionIdx> findAllPossibleIntersections (std::vector<StreetIdx> first, std::vector<StreetIdx> second){
    
    std::vector<IntersectionIdx> possibleIntersections;
    
    for (auto firstStreetIdx = first.begin(); firstStreetIdx != first.end(); firstStreetIdx++) {

            for (auto secondStreetIdx = second.begin(); secondStreetIdx != second.end(); secondStreetIdx++) {

            auto commonIntersections = findIntersectionsOfTwoStreets(std::make_pair(*firstStreetIdx, *secondStreetIdx));

            for (auto commonStreetIdx = commonIntersections.begin(); commonStreetIdx != commonIntersections.end(); commonStreetIdx++) {
                possibleIntersections.push_back(*commonStreetIdx);
            }
        }
    }
    
    return possibleIntersections;
}

//covert a city name to a mapPath
std::string convertNameToPath(std::string name){
    
    std::string nameInLower;
    char previous = '1';
    
    //covert the name into file format
    for (int c = 0; c < name.size(); c++){
        
        if (name[c] == ' ' && previous != ','){
            
            nameInLower.push_back('-');
            
        }else if (name[c] == ','){
            
            nameInLower.push_back('_');
            
        }else if (previous != ','){
            
            nameInLower.push_back(tolower(name[c]));
            
        }
        
        previous = name[c];
    }
    
    std::string path = mapPathPrefix + nameInLower + ".streets.bin";
    return path;
}

//add all the element in a vector to another vector
void addVectorToVector (std::vector<IntersectionIdx>& to, const std::vector<IntersectionIdx>& from){
    for (auto it = from.begin(); it != from.end(); it++){
        to.push_back(*it);
    }
}


//clear all the highlight intersection
void clearHighlightIntersection(){
    
    for (auto iterator = previousHighlight.begin(); iterator != previousHighlight.end(); iterator++){
        cities[currentCityIdx] -> intersection -> intersectionInfo[*iterator].isHighlight = false;
    }
    previousHighlight.clear();
}

bool isAve(std::string streetName){
    bool isAve =false;
    std::string s1 ("Avenue\0");
    
    if (streetName.find(s1) != std::string::npos){
        isAve = true;
    }
    return isAve;
}

double streetSize(ezgl::rectangle world){
    //this function make the streetSize change as the  map area changes, 
    //it is bounded by ln(), size between around 0.5 - 10
    double mapArea = world.area();
    
    //function parameter
    double par1=6;
    double par2=1000;
    double par3=1.5;
    double par4=5;
    
    //adjust street size
    double streetSize = par1*log(log(par2/sqrt(mapArea)+par3))+par4;
 
    return streetSize;
}

void drawArrow(ezgl::renderer *g, ezgl::point2d position, double theta) {
    double delta = 15;
    double h = 10;
    double arrowThickness = 0.1;

    //points for the arrow
    ezgl::point2d firstPoint(position.x +h * cos(theta * kDegreeToRadian), position.y + h * sin(theta * kDegreeToRadian));
    ezgl::point2d secondPoint(firstPoint.x - h * cos((theta + delta) * kDegreeToRadian), firstPoint.y - h * sin((theta + delta) * kDegreeToRadian));
    ezgl::point2d thirdPoint(firstPoint.x - h * cos((theta - delta) * kDegreeToRadian), firstPoint.y - h * sin((theta - delta) * kDegreeToRadian));

    //draw the arrow
    g->set_line_width(arrowThickness * streetSize(g->get_visible_world()));
    g->draw_line(firstPoint, secondPoint);
    g->draw_line(firstPoint, thirdPoint);

    return;
}

//highlight intersection by showing a red square and display the pop-up box
void displayHighlightedIntersection(ezgl::renderer *g) {
    ezgl::rectangle world = g->get_visible_world();
    double width = 6 * world.width() / g->get_visible_screen().width();
    double height = width;
    
    for (size_t i = 0; i < cities[currentCityIdx] -> intersection -> intersectionInfo.size(); ++i) {
        //get x,y position of the intersection
        float x = xFromLon(cities[currentCityIdx] -> 
        intersection -> intersectionInfo[i].position.longitude());
        float y = yFromLat(cities[currentCityIdx] -> intersection -> intersectionInfo[i].position.latitude());
        
        if (cities[currentCityIdx] -> intersection -> intersectionInfo[i].isHighlight) {
            g->set_color(ezgl::GREY_75);
            //display pop up box
            if (cities[currentCityIdx] -> intersection -> intersectionInfo[i].name.compare("<unknown>") != 0){
                displayPopupBox(g, "Intersection: ", cities[currentCityIdx] -> intersection -> intersectionInfo[i].name, x, y, world);
            }

            g->set_color(ezgl::RED);
            g->fill_rectangle({x - width/2, y - height/2},
                              {x + width/2, y + height/2});
        }
    }
    return;
}

void displayIntersectionPopup(ezgl::renderer *g, ezgl::rectangle world, IntersectionIdx id) {
        
    float x = xFromLon(cities[currentCityIdx] -> intersection -> intersectionInfo[id].position.longitude());
    float y = yFromLat(cities[currentCityIdx] -> intersection -> intersectionInfo[id].position.latitude());
        
    g->set_color(ezgl::GREY_75);
    //display pop up box
    displayPopupBox(g, "Intersection: ", cities[currentCityIdx] -> intersection -> intersectionInfo[id].name, x, y, world);
}

void drawIcon(ezgl::renderer *g, ezgl::rectangle world, ezgl::surface *iconSurface, IntersectionIdx location) {
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    
    if (location >= 0 && location < getNumIntersections()){
        
        double surfaceWidth = (double)cairo_image_surface_get_width(iconSurface) * widthToPixelRatio;
        double surfaceHeight = (double)cairo_image_surface_get_height(iconSurface) * heightToPixelRatio;
        LatLon locationLatLon = getIntersectionPosition(location);
        g->draw_surface(iconSurface, {xFromLon(locationLatLon.longitude()) - surfaceWidth / 2 , yFromLat(locationLatLon.latitude()) + surfaceHeight} );
        
        //draw the text of the title
        g->set_text_rotation(0);
        g->set_font_size(10);
        g->set_color(textColor);
        g->draw_text({xFromLon(locationLatLon.longitude()), yFromLat(locationLatLon.latitude()) + surfaceHeight * 1.2}, cities[currentCityIdx] -> intersection -> intersectionInfo[location].name);
    }
}

void drawSegment(ezgl::renderer *g, ezgl::rectangle world, ezgl::color segColor, StreetSegmentIdx streetSegmentsID) {
    double x1, y1, x2, y2;
    double routeWidth = 3;

    for (int pointsID = 1; pointsID < cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID].size(); pointsID++) {
        x1 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID - 1].longitude());
        y1 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID - 1].latitude());

        x2 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].longitude());
        y2 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].latitude());
        if (world.contains(x1, y1) || world.contains(x2, y2)) {
            g->set_color(segColor);
            g->set_line_width(routeWidth);
            g->draw_line({x1, y1},{x2, y2});
        }
    }
}

double crossProduct(IntersectionIdx from, IntersectionIdx mid, IntersectionIdx to){
    LatLon A= getIntersectionPosition(from);
    LatLon B=getIntersectionPosition(mid);
    LatLon C=getIntersectionPosition(to);
    
    double Ax,Bx,Cx,Ay,By,Cy;
    
    Ax=xFromLon(A.longitude());
    Ay=yFromLat(A.latitude());
    Bx=xFromLon(B.longitude());
    By=yFromLat(B.latitude());
    Cx=xFromLon(C.longitude());
    Cy=yFromLat(C.latitude());
    
    //vector BA, from point B to A;Vector BC from B to C
    //vector BA<BAi,BAj,BAk>
    double BAi,BAj;
    BAi=Ax-Bx;
    BAj=Ay-By;

    double BCi,BCj;
    BCi=Cx-Bx;
    BCj=Cy-By;

    //BC cross BA result only in k direction
    double k= BCi*BAj-BCj*BAi;
    return k;
}
