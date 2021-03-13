/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m2.h"
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


double textDisplayRatio = 0.01;
double streetToWorldRatio = 0.5;
double streetToWorldRatio1 = 0.09;
double EARTH_CIRCUMFERENCE = 2* M_PI * kEarthRadiusInMeters;
bool showSubways = false;

bool nightMode;             //Determines whether to show night mode

//global variable storing color for background, text, feature and street
ezgl::color backgroundColor;
ezgl::color textColor;
ezgl::color lakeColor;
ezgl::color islandColor;
ezgl::color parkColor;
ezgl::color buildingColor;
ezgl::color beachColor;
ezgl::color golfColor;
ezgl::color streetColor;
ezgl::color highwayColor;

std::vector<IntersectionIdx> previousHighlight;
LatLon positionOfClicked;
std::vector <ezgl::point2d> featureTextPoints;      //coordinates for display feature text
std::string selectedPOI = "all";                    //selected POI type to display

extern std::vector<City*> cities;
extern int currentCityIdx;
extern std::vector<std::string> cityNames;
extern std::string mapPathPrefix;

void drawMap(){

    // Initialize coordinates for feature bounding boxes.

    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    
    initializeCurrentWorldRatio();

    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    
    ezgl::rectangle initial_world({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    application.add_canvas("MainCanvas", drawMainCanvas, initial_world);
    //initial set up, act on mouse press, act on mouse move, act on key press
    application.run(initialSetUp, actOnMouseClick, nullptr, nullptr);
}

void drawMainCanvas (ezgl::renderer *g){
    
    //determine the color used based on night mode selection   
    if (!nightMode) {
        backgroundColor= ezgl::color(255, 247, 224, 255);
        textColor= ezgl::BLACK;
        lakeColor = ezgl::color(102, 204, 255, 255);
        islandColor = ezgl::color(221, 251, 109, 255);
        parkColor = ezgl::color(128, 234, 85, 255);
        buildingColor = ezgl::GREY_75;
        beachColor = ezgl::color(236, 226, 149, 255);
        golfColor = ezgl::color(110, 184, 66, 255);
        streetColor = ezgl::color(210, 223, 227, 255);
        highwayColor = ezgl::color(244, 208, 63, 255);
        
    } else {
        backgroundColor = ezgl::color(0,0,0);
        textColor= ezgl::WHITE;
        lakeColor = ezgl::color(10, 4, 73);
        islandColor = ezgl::color(2, 91, 4, 255);
        parkColor = ezgl::color(19, 100, 20, 255);
        buildingColor = ezgl::color(98, 98, 98, 255);
        beachColor = ezgl::color(109, 113, 42, 255);
        golfColor = ezgl::color(3, 46, 1, 255);
        streetColor = ezgl::color(142, 142, 142, 255);
        highwayColor = ezgl::color(191, 131, 0, 255);
    }

    g->format_font("Noto Sans CJK SC", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);

    ezgl::rectangle world = g->get_visible_world();
    double xMin, xMax, yMin, yMax;

    yMax = yFromLat(cities[currentCityIdx] -> maxLat);
    yMin = yFromLat(cities[currentCityIdx] -> minLat);
    xMax = xFromLon(cities[currentCityIdx] -> maxLon);
    xMin = xFromLon(cities[currentCityIdx] -> minLon);
    ezgl::point2d start(xMin, yMin);
    ezgl::point2d end(xMax, yMax);
    
    g->draw_rectangle(start, end);
    g->set_color(backgroundColor);
    g->fill_rectangle(start, end);

    //timing function
    std::clock_t begin = clock();

    drawFeature(g, world);
    std::clock_t featureEnd = clock();
    
    drawStreet(g, world);
    std::clock_t streetEnd = clock();
    
    displayPOI(g);
    std::clock_t poiEnd = clock();
    
    displayHighlightedIntersection(g);
    std::clock_t highlighIntersectionEnd = clock();
    
    displayStreetName(g, world);
    std::clock_t streetNameEnd = clock();
    
    if (showSubways) loadSubway(g);

    //time stamps for drawing each element on map
    double elapsedSecondsFeature = double(featureEnd - begin) / CLOCKS_PER_SEC;
    double elapsedSecondsStreet = double(streetEnd-featureEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsPoi = double(poiEnd-streetEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsHighlightIntersection = double(highlighIntersectionEnd-poiEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsStreetName = double(streetNameEnd-highlighIntersectionEnd) / CLOCKS_PER_SEC;
    
    std::cout << "Feature: "<<elapsedSecondsFeature << " Street: " << elapsedSecondsStreet << " POI:  " << elapsedSecondsPoi << 
            " HighlightIntersection: " << elapsedSecondsHighlightIntersection << " StreetName: " << elapsedSecondsStreetName << "\n";
    
    double totalTime = double(streetNameEnd - begin)/CLOCKS_PER_SEC;
    std::cout<<"total time" << totalTime << "\n";
}


//set the height to width ratio of a current city
void initializeCurrentWorldRatio(){
    
    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    cities[currentCityIdx]->worldRatio = (yFromLat(maxLat) - yFromLat(minLat))/(xFromLon(maxLon) - xFromLon(minLon));
}

//connect the signal to call back function
void initialSetUp(ezgl::application *application, bool /*new_window*/){
    
    application->update_message("Map is loaded successfully");
    
    GObject *search = application->get_object("SearchButton");
    g_signal_connect(search, "clicked", G_CALLBACK(searchButtonIsClicked), application);
    
    GObject *textEntry = application->get_object("TextInput");
    g_signal_connect(textEntry, "activate", G_CALLBACK(textEntryPressedEnter), application);
    
    GtkComboBoxText *mapBar = (GtkComboBoxText*) application->get_object("MapBar");
    importNameToTheBar(mapBar);
    g_signal_connect (mapBar, "changed", G_CALLBACK(changeMap), application);
    
    //POI related buttons
    GObject *allPOI = application->get_object("allPOIBtn");
    g_signal_connect(allPOI, "toggled", G_CALLBACK(toggleAllPOI), application);
    
    GObject *educationPOI = application->get_object("educationPOIBtn");
    g_signal_connect(educationPOI, "toggled", G_CALLBACK(toggleEducationPOI), application);

    GObject *foodPOI = application->get_object("foodPOIBtn");
    g_signal_connect(foodPOI, "toggled", G_CALLBACK(toggleFoodPOI), application);
    
    GObject *medicalPOI = application->get_object("medicalPOIBtn");
    g_signal_connect(medicalPOI, "toggled", G_CALLBACK(toggleMedicalPOI), application);
    
    GObject *transportPOI = application->get_object("transportPOIBtn");
    g_signal_connect(transportPOI, "toggled", G_CALLBACK(toggleTransportPOI), application);
    
    GObject *recreationPOI = application->get_object("recreationPOIBtn");
    g_signal_connect(recreationPOI, "toggled", G_CALLBACK(toggleRecreationPOI), application);
    
    GObject *financePOI = application->get_object("financePOIBtn");
    g_signal_connect(financePOI, "toggled", G_CALLBACK(toggleFinancePOI), application);
    
    GObject *govPOI = application->get_object("govPOIBtn");
    g_signal_connect(govPOI, "toggled", G_CALLBACK(toggleGovPOI), application);

    GObject *otherPOI = application->get_object("otherPOIBtn");
    g_signal_connect(otherPOI, "toggled", G_CALLBACK(toggleOtherPOI), application);

    //check box for showing subways
    GObject *showSubwayBox = application->get_object("showSubwayBox");
    g_signal_connect(showSubwayBox, "toggled", G_CALLBACK(toggleSubway), application);
    
    //check box for using night mode
    GObject *snightModeBox = application->get_object("nightModeBox");
    g_signal_connect(snightModeBox, "toggled", G_CALLBACK(toggleNightMode), application);
    
}

//input all the city name to the map bar
void importNameToTheBar(GtkComboBoxText* bar){
    
    for(int idx = 0; idx < cityNames.size(); idx++){
        
        std::string name = cityNames[idx];
        gtk_combo_box_text_append_text(bar, name.c_str());
    }
}


//triggered night mode check box changed
gboolean toggleNightMode(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    nightMode = !nightMode;
    application->refresh_drawing();
    
    return true;
}

//triggered showSubway check box changed
gboolean toggleSubway(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    showSubways = !showSubways;
    application->refresh_drawing();
    
    return true;
}


//triggered when all POI button is changed
gboolean toggleAllPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("all") != 0) {
        
        selectedPOI = "all";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when education POI button is changed
gboolean toggleEducationPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("education") != 0) {
        selectedPOI = "education";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Food POI button is changed
gboolean toggleFoodPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("food") != 0) {
        selectedPOI = "food";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Medical POI button is changed
gboolean toggleMedicalPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("medical") != 0) {
        selectedPOI = "medical";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Transport POI button is changed
gboolean toggleTransportPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("transport") != 0) {
        selectedPOI = "transport";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Finance POI button is changed
gboolean toggleFinancePOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("finance") != 0) {
        selectedPOI = "finance";
        application->refresh_drawing();
    }
    return true;
}

//triggered when Recreation POI button is changed
gboolean toggleRecreationPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("recreation") != 0) {
        selectedPOI = "recreation";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Government POI button is changed
gboolean toggleGovPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("gov") != 0) {
        selectedPOI = "gov";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Other POI button is changed
gboolean toggleOtherPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("other") != 0) {
        selectedPOI = "other";
        std::cout << selectedPOI << "\n";
        application->refresh_drawing();
    }
    
    return true;
}

//Search when user press the enter in the text field
gboolean textEntryPressedEnter(GtkWidget * widget, gpointer data){
    
    return searchButtonIsClicked(widget, data);
    
}

//change the map when the user switch to a different map
gboolean changeMap(GtkWidget *, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    GtkComboBoxText * mapBar = (GtkComboBoxText*) application->get_object("MapBar");
    
    std::string map = gtk_combo_box_text_get_active_text(mapBar);
    
    std::string newMapPath = convertNameToPath(map);
    
    //does not change the map if the user is choose the current map
    if (newMapPath == cities[currentCityIdx] -> mapPath){
        return true;
    }

    closeDataBase();
    bool loadSucessfully = loadMap(newMapPath);
    if(!loadSucessfully){
        std::cerr << "Failed to load map '" << newMapPath << "'\n";
        application->quit();
        return true;
    }
    
    
    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    
    //new world coordinate
    ezgl::rectangle newWorld({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    initializeCurrentWorldRatio();
    
    application ->change_canvas_world_coordinates("MainCanvas", newWorld);
    std::string output = "Map " + map + " has loaded Successfully";
    application->update_message(output);
    application->refresh_drawing();
    
    return true;
    
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

//search the intersections of two streets
gboolean searchButtonIsClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);
    
    GtkEntry* textEntry = (GtkEntry *) application ->get_object("TextInput");
    
    std::string text = gtk_entry_get_text(textEntry);
    std::string firstStreet, secondStreet;
    clearHighlightIntersection();
   
    
    
    bool firstFinished = false;         //finished reading the first street  
    
    //split the input into two street name
    for (auto iterator = text.begin(); iterator != text.end(); iterator++){
        
        if (!firstFinished){
            
            if (*iterator != ',' && *iterator != ' '){
                
                firstStreet.push_back(tolower(*iterator));
            }else if (*iterator == ','){
                
                firstFinished = true;
            }
            
        }else{
            
            if (*iterator != ' '){
                
                secondStreet.push_back(tolower(*iterator));
            }    
        }
    }
    
    std::string output;
    
 
    std::vector<StreetIdx> partialResultFirst = findStreetIdsFromPartialStreetName(firstStreet);
    std::vector<StreetIdx> partialResultSecond = findStreetIdsFromPartialStreetName(secondStreet);
 
    ezgl::point2d sum(0, 0), center(0,0), largest(-1 * EARTH_CIRCUMFERENCE, -1 * EARTH_CIRCUMFERENCE), smallest(EARTH_CIRCUMFERENCE, EARTH_CIRCUMFERENCE);

    
    if (partialResultFirst.size() > 0 && partialResultSecond.size() > 0){
        
       
        std::vector<IntersectionIdx> commonIntersection;
        for (auto firstStreetIdx = partialResultFirst.begin(); firstStreetIdx != partialResultFirst.end();){
            
            for (auto secondStreetIdx= partialResultSecond.begin(); secondStreetIdx != partialResultSecond.end();){
                
                commonIntersection = findIntersectionsOfTwoStreets(std::make_pair(*firstStreetIdx, *secondStreetIdx));
                if (commonIntersection.size() > 0){
                    
                    output = getStreetName(*firstStreetIdx) + ", " + getStreetName(*secondStreetIdx);
                    firstStreetIdx = partialResultFirst.end();
                    secondStreetIdx = partialResultSecond.end();
                }else {
                    
                    secondStreetIdx++;
                }
            }
            
            if(firstStreetIdx != partialResultFirst.end()){
                
                firstStreetIdx++;
            }
        }


        if (commonIntersection.size() > 0){
            
            //position in latitude and longitude
            for (int idx = 0; idx < commonIntersection.size(); idx++){
                //get the position in cartesian coordiante
                auto positionInLL = getIntersectionPosition(commonIntersection[idx]);
                double positionInX = xFromLon(positionInLL.longitude());
                double positionInY = yFromLat(positionInLL.latitude());
                        
                sum.x += positionInX;
                sum.y += positionInY;
                
                if (positionInX > largest.x){
                    
                    largest.x = positionInX;
                }
                
                if (positionInX < smallest.x){
                    
                    smallest.x = positionInX;
                }
                
                if (positionInY > largest.y){
                    
                    largest.y = positionInY;
                }
                
                if (positionInY < smallest.y){
                    
                    smallest.y = positionInY;
                }
                
                //highlight these cities[currentCityIdx] -> intersection -> intersectionInfo
                cities[currentCityIdx] -> intersection -> intersectionInfo[commonIntersection[idx]].isHighlight = true;
                previousHighlight.push_back(commonIntersection[idx]);
                
            }
            center.x = sum.x/commonIntersection.size();
            center.y = sum.y/commonIntersection.size();
            
            //make sure the interested region has some distance with the windows margin
            double gap = 500;
            
            //set up the world zoom level
            double left, bottom, top, right;
            left = smallest.x - gap;
            bottom = smallest.y - gap;
            right = largest.x + gap;
            top = largest.y + gap;
            
            double width = right - left;
            double height = top - bottom;
            
            //making sure the width and height of the screen is in the world ratio
            if (width * cities[currentCityIdx]->worldRatio > height){
                
                bottom = center.y - (width * cities[currentCityIdx]->worldRatio) / 2;
                top =  center.y + (width * cities[currentCityIdx]->worldRatio) / 2;
                
            }else {
                
                left = center.x - (height / cities[currentCityIdx]->worldRatio) / 2;
                right = center.x + (height / cities[currentCityIdx]->worldRatio) / 2;
                
            }
            ezgl::rectangle world({left, bottom}, {right, top});
            canvas->get_camera().set_world(world);
            
        }else{
            output = "No Intersection found.";
        }
        
    }else{
        output = "Street can not be found";
    }
    
    
    application->update_message(output);
    application->refresh_drawing();
    return true;
}


void actOnMouseClick(ezgl::application* , GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    std::cout << "Button " << event->button << " is clicked\n";
    
    //record the position of left mouse clicked
    if (event ->button == 1){
        positionOfClicked = LatLon(latFromY(y), lonFromX(x));
    }    
}

//mouse click to highlight the closest intersection
IntersectionIdx clickToHighlightClosestIntersection(LatLon pos){
    IntersectionIdx id = findClosestIntersection(pos);
    
    if (previousHighlight.size() == 1 && previousHighlight[0] == id){
        
        cities[currentCityIdx] -> intersection -> intersectionInfo[id].isHighlight = false;
        clearHighlightIntersection(); 
    }else{
        
        clearHighlightIntersection();
        cities[currentCityIdx] -> intersection -> intersectionInfo[id].isHighlight = true;
        previousHighlight.push_back(id);
    }
    
    
    std::cout << "Closest Intersection: " << cities[currentCityIdx] -> intersection -> intersectionInfo[id].name << "\n";
    return id;
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


void drawStreet(ezgl::renderer *g, ezgl::rectangle world){
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    double aveToWorldRatio1 = 0.13;
    double highwaySpeed = 22.23;
    double visibleStreetLenRatio = 1.5;
    double streetWidth = 0.5;
    double highwayWidth = 1.3;

    for(int streetSegmentsID=0; streetSegmentsID<getNumStreetSegments(); streetSegmentsID++ ){
        //get street segment streetID
        StreetIdx streetId =getStreetSegmentInfo(streetSegmentsID).streetID;
        //get street name of this street segment
        std::string streetName = getStreetName(streetId);

        //if the street name is unknown, draw when user zoom in
        if(!streetName.compare("<unknown>")){
            //draw as user zooms in
            if(findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * visibleStreetLenRatio * streetToWorldRatio1){
                g->set_color(streetColor);
                g->set_line_width(streetWidth * streetSize(world));
                g->draw_line({x1,y1}, {x2, y2});
            }
            
        }else if (findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * streetToWorldRatio1){
            //draw street according to the length of the street compare to screen  length


            for(int pointsID=1; pointsID < cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID].size(); pointsID++){
                x1 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID - 1].longitude());
                y1 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID- 1].latitude());

                x2 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].longitude());
                y2 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].latitude());
                if(world.contains(x1, y1) || world.contains(x2, y2)){
                    double speedLimit = getStreetSegmentInfo(streetSegmentsID).speedLimit;
                    
                    if(speedLimit>highwaySpeed){// draw highway
                        
                       g->set_color(highwayColor);
                       g->set_line_width(highwayWidth * streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }else if(!isAve(streetName)){ //draw main road and street
                        
                       g->set_color(streetColor);
                       g->set_line_width(streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }else if(isAve(streetName)&&findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * aveToWorldRatio1){ 
                        
                        //draw avenue
                       g->set_color(streetColor);
                       g->set_line_width(streetWidth * streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }

                }
            }
        }        
    }
    return;
}

double textSize(ezgl::rectangle world){
    
    double mapArea = world.area();
    double k=3.5;
    double textSize = k*log(log(1000/sqrt(mapArea)+1.5))+5;
    
    return textSize;
}

double streetSize(ezgl::rectangle world){
    
    double mapArea = world.area();
    double k=6;
    double streetSize = k*log(log(1000/sqrt(mapArea)+1.5))+5;
 
    return streetSize;
}


void displayStreetName(ezgl::renderer *g, ezgl::rectangle world){
    
    std::vector<ezgl::point2d> displayedNames;

    
    double fontSize = 10;
    double streetNameSize = 200;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    displayedNames.clear();
    
    for(int streetID = 0; streetID < getNumStreets(); streetID++ ){
        //get segments position that within the screen
        std::vector<ezgl::point2d> inViewSegment;
        inViewSegment.clear();
        
        inViewSegment.clear();
        
        std::string streetName = getStreetName(streetID);
        
        double xFrom = 0, yFrom = 0, xTo = 0, yTo = 0;
        for(int segmentIndex = 0; segmentIndex < cities[currentCityIdx]->street->streetSegments[streetID].size(); segmentIndex++){
            
            if (streetName.compare("<unknown>") != 0 && findStreetLength(streetID) > diagLength * streetToWorldRatio) {

                xFrom = xFromLon(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).from).longitude());
                yFrom = yFromLat(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).from).latitude());
                xTo = xFromLon(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).to).longitude());
                yTo = yFromLat(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).to).latitude());

                if (world.contains(xFrom, yFrom) || world.contains(xTo, yTo)){
                    inViewSegment.push_back({xFrom, yFrom});
                    
                    //store the segment that is one way                   
                    if(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).oneWay){
                        
                        oneWaySegInfo oneWay;
                        oneWay.fromX=xFrom;
                        oneWay.fromY=yFrom;
                        oneWay.toX=xTo;
                        oneWay.toY=yTo;
                        oneWay.distance=sqrt((xTo-xFrom)*(xTo-xFrom)+(yTo-yFrom)*(yTo-yFrom));
                        cities[currentCityIdx]->streetSegment->oneWaySegment.push_back(oneWay);
                    }
                }
            }
        }

        if (inViewSegment.size() > 2) {
            
            //find the middle segment of the street on the screen
            ezgl::point2d midPoint = inViewSegment[inViewSegment.size()/2];
            ezgl::point2d midNextPoint = inViewSegment[inViewSegment.size()/2 + 1];
            
            //find the degree to rotate
            double degree = atan2(midNextPoint.y - midPoint.y, midNextPoint.x - midPoint.x) / kDegreeToRadian;
            bool overlap = false;
            
            //estimate the text width and height
            double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
            double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
            
            //check if the street name overlap with each other
            for (int displayedNamesNum = 0; displayedNamesNum < displayedNames.size(); displayedNamesNum++ ){
                
                if(abs(midPoint.x - displayedNames[displayedNamesNum].x) < streetNameSize * widthToPixelRatio && 
                    abs(midPoint.y - displayedNames[displayedNamesNum].y) < streetNameSize * heightToPixelRatio){
                    overlap = true;
                }
            }
            
            //make sure text is readable for user by adjusting rotation
            if (degree > 90){
                degree = degree - 180;
            }else if (degree < -90){
                degree = degree + 180;
            }
            
            if(!overlap){
                g->set_font_size(fontSize);
                g->set_color(textColor);
                g->set_text_rotation(degree);
                g->draw_text(midPoint, streetName);
                
                
                //save the place where a name has been displayed
                displayedNames.push_back(midPoint);
            }
            //draw one way symbol
           drawOneWayStreet(g, diagLength);
              
        }
    }
}
void drawArrow(ezgl::renderer *g, ezgl::point2d position, double theta){
    double delta = 15;
    double h = 10;
    double arrowThickness = 0.1;
    
    //points for the arrow
    ezgl::point2d firstPoint(position.x + h * cos(theta * kDegreeToRadian), position.y + h * sin(theta * kDegreeToRadian));
    ezgl::point2d secondPoint(firstPoint.x + h * cos((theta + delta) * kDegreeToRadian), firstPoint.y + h * sin((theta + delta) * kDegreeToRadian));
    ezgl::point2d thirdPoint(firstPoint.x + h * cos((theta - delta) * kDegreeToRadian), firstPoint.y + h * sin((theta - delta) * kDegreeToRadian));

    //draw the arrow
    g->set_line_width(arrowThickness * streetSize(g->get_visible_world()));
    g->draw_line(firstPoint, secondPoint);
    g->draw_line(firstPoint, thirdPoint);
}

void drawOneWayStreet(ezgl::renderer *g, double diagLength){
    
    for(int oneWaySegId = 0; oneWaySegId < cities[currentCityIdx]->streetSegment->oneWaySegment.size(); oneWaySegId++){
        if(cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].distance > 0.1 * diagLength){

            double degree = atan2(cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].toY - 
            cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromY,
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].toX - 
            cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromX) / kDegreeToRadian;

            ezgl::point2d position(cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromX ,
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromY);

            g->set_text_rotation(degree);
            drawArrow(g, position, degree);

        }
    }
}






//draw all features in map
void drawFeature(ezgl:: renderer *g, ezgl::rectangle world){
    featureTextPoints.clear();
    double visibleArea = world.area();
    
    //level of detail ratio to display feature
    double featureToWorldRatio = 0.00005;    
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    
    std::vector <FeatureIdx> islands;

    //loop through all features, if the feature area is at the predefined ratio 
    //of the visible area and within the window, draw it
    for (FeatureIdx featureID = 0; featureID < getNumFeatures(); featureID++){
        
        double minX = cities[currentCityIdx]->featurePts[featureID].left;
        double maxX = cities[currentCityIdx]->featurePts[featureID].right;
        double maxY = cities[currentCityIdx]->featurePts[featureID].top;
        double minY = cities[currentCityIdx]->featurePts[featureID].bottom;
        double featureArea = findFeatureArea(featureID);
        
        // If the feature is in the visible area, call helper function to display the feature.
        if ((world.contains(minX, minY) || world.contains(minX, maxY)
                || world.contains(maxX, minY) || world.contains(maxX, maxY)
                || (minY <= world.top() && maxY >= world.bottom() 
                && minX <= world.right() && maxX >= world.left())) 
                && featureArea >= visibleArea * featureToWorldRatio){
                    
            if(getFeatureType(featureID) == ISLAND){
                
                islands.push_back(featureID);
            }else{
                
                drawFeatureByID(g, featureID);
            }
        }
    }
    
    //draw the islands last, this prevents lakes from covering and islands
    for (int islandIdx = 0; islandIdx < islands.size(); islandIdx++){
        drawFeatureByID(g, islands[islandIdx]);
    }

    //loop through all features, if the feature area is at the predefined ratio 
    //of the visible area and within the window, display its name
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        
        double minX = cities[currentCityIdx]->featurePts[featureID].left;
        double maxX = cities[currentCityIdx]->featurePts[featureID].right;
        double maxY = cities[currentCityIdx]->featurePts[featureID].top;
        double minY = cities[currentCityIdx]->featurePts[featureID].bottom;
        double featureArea = findFeatureArea(featureID);
        
        if ((world.contains(minX, minY) || world.contains(minX, maxY)
                || world.contains(maxX, minY) || world.contains(maxX, maxY)
                || (minY <= world.top() && maxY >= world.bottom() 
                && minX <= world.right() && maxX >= world.left())) 
                && featureArea >= visibleArea * featureToWorldRatio){
            
            if (featureArea >= visibleArea * textDisplayRatio){
                displayFeatureNameByID(g, featureID, visibleArea, featureArea,widthToPixelRatio, heightToPixelRatio);
            }
        }
    }
}

//draw a specific feature by a given feature id
void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id){
    std::vector<ezgl::point2d> points;
    
    //choose the color for different types of feature
    FeatureType featureType = getFeatureType(id);   
    switch (featureType) {
        case PARK:
            g->set_color(parkColor);
            break;
        case BEACH:
            g->set_color(beachColor);
            break;
        case LAKE:
            //blue
            g->set_color(lakeColor);
            break;
        case RIVER:
            g->set_color(lakeColor);
            break;
        case ISLAND:
            g->set_color(islandColor);
            break;
        case BUILDING:
            g->set_color(buildingColor);
            break;
        case GREENSPACE:
            g->set_color(parkColor);
            break;
        case GOLFCOURSE:
            g->set_color(golfColor);
            break;
        case STREAM:
            g->set_color(lakeColor);
            break;
        case UNKNOWN:
            g->set_color(ezgl::PURPLE);
            break;
        default:
            break;

    }
    
    //draw the feature outline
    g->set_line_width(0);
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){

        double x1, x2, y1, y2;
        x1 = xFromLon(getFeaturePoint(id, pt - 1).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt - 1).latitude());
        points.push_back({x1, y1});

        x2 = xFromLon(getFeaturePoint(id, pt).longitude());
        y2 = yFromLat(getFeaturePoint(id, pt).latitude());
        g->draw_line({x1,y1}, {x2, y2});
        
    }
    
    //fill the feature with color chosen if it is closed
    if (points.size() > 1 && (getFeaturePoint(id,0) == getFeaturePoint(id, getNumFeaturePoints(id) - 1))){
        g->fill_poly(points);
    }
}

//display the name of a specific feature with a given feature id
void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea, double widthToPixelRatio, double heightToPixelRatio){
    
    //obtain the middle point of the feature
    double xAvg = 0;
    double yAvg = 0;
    double area = 0;
    double x = xFromLon(getFeaturePoint(id, 0).longitude());
    double y = yFromLat(getFeaturePoint(id, 0).latitude());
    std::string featureName = getFeatureName(id); 
    FeatureType featureType = getFeatureType(id);
    
    if(id==6){
            std::cout<<x<<" "<<y<<std::endl;
            
     }
    
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){
        double x1, y1;
        x1 = xFromLon(getFeaturePoint(id, pt).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt).latitude());
        
        area += (x * y1 -x1 * y) / 2;
        xAvg += (x * y1 - x1 * y) * (x + x1);
        yAvg += (x * y1 - x1 * y) * (y + y1);
        x = x1;
        y = y1;
    }
    
    xAvg /= 6*area;
    yAvg /= 6*area;
    
    //display the name only with features that is not type UNKNOWN
    bool displayName = false;
    switch (featureType) {
        case PARK:
            g->set_color(textColor);
            displayName = true;
            break;
        case BEACH:
            g->set_color(textColor);
            displayName = true;
            break;
        case LAKE:
            g->set_color(textColor);
            displayName = true;
            break;
        case RIVER:
            g->set_color(textColor);
            displayName = true;
            break;
        case ISLAND:
            g->set_color(textColor);
            displayName = true;
            break;
        case BUILDING:
            g->set_color(textColor);
            displayName = true;
            break;
        case GREENSPACE:
            g->set_color(textColor);
            displayName = true;
            break;
        case GOLFCOURSE:
            g->set_color(textColor);
            displayName = true;
            break;
        case STREAM:
            g->set_color(textColor);
            break;
        case UNKNOWN:
            g->set_color(textColor);
            break;
        default:
            break;
    }
    bool overlapped = false;
    
    //range for a line of text
    double featureRangeX = 30;
    double featureRangeY = 30;

    //identify if the text will overlap with others
    for(int displayedNameIdx = 0; displayedNameIdx < featureTextPoints.size(); displayedNameIdx++){
        
        if(abs(xAvg - featureTextPoints[displayedNameIdx].x) < featureRangeX * widthToPixelRatio && 
           abs(yAvg - featureTextPoints[displayedNameIdx].y) < featureRangeY * heightToPixelRatio){
            overlapped = true;
            break;
        }
    }
    //display the feature name at predefined text display ratio when its name is not <noname>,
    //at the predefined level of detail ratio, and not overlapping with other texts
    if (displayName && featureName.compare("<noname>") != 0 && featureArea > visibleArea * textDisplayRatio && !overlapped){
        g->draw_text({xAvg, yAvg}, featureName);
        featureTextPoints.push_back({xAvg, yAvg});
    }
}

//highlight intersection by showing a red square and display the pop-up box
void displayHighlightedIntersection(ezgl::renderer *g) {
    ezgl::rectangle world = g->get_visible_world();
    double width = 6 * world.width() / g->get_visible_screen().width();
    double height = width;
    
    for (size_t i = 0; i < cities[currentCityIdx] -> intersection -> intersectionInfo.size(); ++i) {
        float x = xFromLon(cities[currentCityIdx] -> intersection -> intersectionInfo[i].position.longitude());
        float y = yFromLat(cities[currentCityIdx] -> intersection -> intersectionInfo[i].position.latitude());
        
        if (cities[currentCityIdx] -> intersection -> intersectionInfo[i].isHighlight) {

            g->set_color(ezgl::GREY_75);
            
            if (cities[currentCityIdx] -> intersection -> intersectionInfo[i].name.compare("<unknown>") != 0){
                displayPopupBox(g, "Intersection: ", cities[currentCityIdx] -> intersection -> intersectionInfo[i].name, x, y, world);
            }

            g->set_color(ezgl::RED);
            g->fill_rectangle({x - width/2, y - height/2},
                              {x + width/2, y + height/2});
        }
    }
}

//display a pop-up box at given location with given title and content
void displayPopupBox(ezgl::renderer *g, std::string title, std::string content, double x, double y, ezgl::rectangle world) {
    //useful ratios (retrieved from try and error)
    double strLenToBoxRatio = 3.266;
    double windowToPopupBoxRatio = 8.385;
    
    //get the width and height in pixel coordinates of the visible screen
    ezgl::rectangle screen = g->get_visible_screen();
    double screenWidth = screen.width();
    double screenHeight = screen.height();

    //get the width of the pop-up window according to string length 
    int strLen = std::max(content.length(), title.length());
    
    //draw the rectangle for title
    y -= world.height() * windowToPopupBoxRatio / screenHeight;
    g->set_color(ezgl::GREY_55);
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y - world.height() * windowToPopupBoxRatio  / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y + world.height() * windowToPopupBoxRatio  / screenHeight });
    
    //draw the text of the title
    g->set_text_rotation(0);
    g->set_font_size(10);
    g->set_color(textColor);
    g->draw_text({x, y}, title);
    
    //draw the rectangle for the contents
    y -= world.height() * windowToPopupBoxRatio / screenHeight * 2;
    g->set_color(ezgl::GREY_75);
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y - world.height() * windowToPopupBoxRatio / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y + world.height() * windowToPopupBoxRatio / screenHeight });
                      
    //draw the text of the contents
    g->set_color(textColor);
    g->set_font_size(10);
    g->draw_text({x, y}, content);
}

//display all the POIs qualified for displaying
void displayPOI(ezgl::renderer *g) {
    
    //if the visible world area is smaller than this number, the POI will be displayed
    double areaToShowPOI = 8400000;   
    
    //approximate range of an POI icon
    double POIRange = 60;
    
    std::vector<ezgl::point2d> displayedPoints;
    
    //calculated the world to pixel coordinate ratio
    ezgl::rectangle world = g->get_visible_world();
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    
    displayedPoints.clear();
    
    // loop through all the poi and show it 
    for(int i = 0; i < getNumPointsOfInterest(); i ++){
        double x = xFromLon(getPOIPosition(i).longitude());
        double y = yFromLat(getPOIPosition(i).latitude());
        
        //prevent overlapping of POI: if the POI is in a certain range of a previously displayed POI, it will not be displayed
        bool overlapped = false;
        for(int displayedPOIIdx = 0; displayedPOIIdx < displayedPoints.size(); displayedPOIIdx++){
            
            if(abs(x - displayedPoints[displayedPOIIdx].x) < POIRange * widthToPixelRatio && 
                    abs(y - displayedPoints[displayedPOIIdx].y) < POIRange * heightToPixelRatio){
                overlapped = true;
            }
        }
        
        // if the map is showing enough level of detail, and the poi is visible in the screen, then display it. 
        if (world.contains({x, y}) && world.area() < areaToShowPOI && !overlapped) {
            displayPOIById(g, i, widthToPixelRatio, heightToPixelRatio);
            displayedPoints.push_back({x, y});
        }
    }
}

//display POI name and icon with a given POI id
void displayPOIById(ezgl::renderer *g, POIIdx id, double widthToPixelRatio, double heightToPixelRatio) {
    ezgl::surface *iconSurface;
    
    //get the coordinates of the POI
    double x = xFromLon(getPOIPosition(id).longitude());
    double y = yFromLat(getPOIPosition(id).latitude());
    bool education, food, medical, transport, recreation,finance, gov, other, displayPOI;
    
    //boolean values for whether to call EZGL function to draw POI icon
    displayPOI = true;
    
    //boolean values for whether to display each type of POI
    education = (selectedPOI.compare("all") == 0 || selectedPOI.compare("education") == 0);
    food = (selectedPOI.compare("all") == 0 || selectedPOI.compare("food") == 0);
    medical = (selectedPOI.compare("all") == 0 || selectedPOI.compare("medical") == 0);
    transport = (selectedPOI.compare("all") == 0 || selectedPOI.compare("transport") == 0);
    recreation = (selectedPOI.compare("all") == 0 || selectedPOI.compare("recreation") == 0);
    finance = (selectedPOI.compare("all") == 0 || selectedPOI.compare("finance") == 0);
    gov = (selectedPOI.compare("all") == 0 || selectedPOI.compare("gov") == 0);
    other = (selectedPOI.compare("all") == 0 || selectedPOI.compare("other") == 0);
        
    std::string poiType = getPOIType(id);
    std::string poiName = getPOIName(id);
    
    //load icon image by poiType
    if (poiType.compare("ferry_termial") == 0){   
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/ferry.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("theatre") != std::string::npos && recreation) {       
        
        if(recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/theater.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("school") != std::string::npos 
            || poiType.rfind("university") != std::string::npos 
            || poiType.rfind("college") != std::string::npos)) {
        
        if (education) 
            iconSurface = g->load_png("./libstreetmap/resources/images/university.png");
        else 
            displayPOI = false;

    } else if (poiType.rfind("parking") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/parkinggarage.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fast_food") != std::string::npos) {       
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fastfood.png");
        else 
            displayPOI = false;

    } else if (poiType.compare("community_centre") == 0) {       
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/communitycentre.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("pharmacy") == 0) {       
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/drogerie.png");
        else 
            displayPOI = false;
       
    } else if (poiType.rfind("cafe") != std::string::npos) {       
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/coffee.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("place_of_worship") == 0) {        
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/chapel-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("bank") == 0) {       
       
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bank.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("atm") == 0) {       
        
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/resources/images/atm-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("cinema") == 0) {       
        
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/cinema.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("hospital") == 0 
            || poiType.compare("doctors") == 0 
            || poiType.find("clinic") != std::string::npos) {        
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/hospital-building.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("library") == 0) {        
        
        if (education) 
            iconSurface = g->load_png("./libstreetmap/resources/images/library.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("restaurant") != std::string::npos 
            || poiType.rfind("food_") != std::string::npos)) {        
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/restaurant.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("police") == 0) {       
        
        if (gov) 
            iconSurface = g->load_png("./libstreetmap/resources/images/police.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("gym") != std::string::npos 
            || poiType.rfind("weight") != std::string::npos) {
        
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fitness.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("dentist") != std::string::npos 
            || poiType.rfind("orthodon") != std::string::npos) {
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/dentist.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bus_s") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bus.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fuel") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fillingstation.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("child") != std::string::npos) {        
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/daycare.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bicyle") != std::string::npos) {       
       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bicyle_parking.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("toilets") != std::string::npos) {        
       
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/toilets_inclusive.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("post_") != std::string::npos) {       
        
        if (other) iconSurface = g->load_png("./libstreetmap/resources/images/postal.png");
        else displayPOI = false;
        
    } else if (poiType == "airport" && transport) {     
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/airport.png");
        else 
            displayPOI = false;
        
    } else if (other) {
        iconSurface = g->load_png("./libstreetmap/resources/images/sight-2.png");
        
    } else {                                // else do not display anything       
        displayPOI = false;
    }
    
    if (displayPOI) {
        
        // make the middle bottom of the icon at the poi location
        double surfaceWidth = (double)cairo_image_surface_get_width(iconSurface) * widthToPixelRatio;
        double surfaceHeight = (double)cairo_image_surface_get_height(iconSurface) * heightToPixelRatio;
        g->draw_surface(iconSurface, {x - surfaceWidth / 2 , y + surfaceHeight} );

        // display poi name
        g->set_color(textColor);
        g->set_font_size(10);
        g->set_text_rotation(0);
        g->draw_text({x, y }, poiName);
    }
}

//load the subway data
void loadSubway(ezgl::renderer *g){
    //level of detail ratios for display
    double showTextRatio = 10;
    double showLineRatio = 50;
    double stationWidth = 30;
    ezgl::rectangle world = g->get_visible_world();
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    
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
                        double x = xFromLon(stationCoord.longitude());
                        double y = yFromLat(stationCoord.latitude());
                        
                        //if the level of detail satisfies the ratio to display subway
                        if (showLineRatio > widthToPixelRatio) {

                            //draw the subway lines and station according to color given
                            g->set_color(12, 124, 92, 255);
                            
                            if (lineColor.compare("green") == 0) {
                                g->set_color(57, 180, 96, 255);
                                
                            } else if (lineColor.compare("purple") == 0) {
                                g->set_color(ezgl::PURPLE);
                                
                            } else if (lineColor.compare("yellow") == 0) {
                                g->set_color(199, 182, 39, 255);
                                
                            } else if (lineColor.compare("brown") == 0) {
                                g->set_color(128, 0, 0, 255);
                                
                            } else if ( lineColor.length() == 7 && lineColor.at(0) == '#') {
                                //convert given HEX color into RGB
                                int rColor = std::stoi(lineColor.substr(1,2), nullptr, 16);
                                int gColor = std::stoi(lineColor.substr(3,2), nullptr, 16);
                                int bColor = std::stoi(lineColor.substr(5,2), nullptr, 16);
                                g->set_color(rColor,gColor,bColor,255);   
                            }

                            //draw the stations
                            g->fill_arc({x, y}, 5 * widthToPixelRatio, 0, 360);
                            
                            
                            //add the station coordinate and name to the vector
                            if (world.contains(x, y) && showTextRatio > widthToPixelRatio) {
                                stationCoordindate.push_back({x, y});
                                stationName.push_back(tagPair.second );
                            }
                        }

                        break;
                    }
                }
            }
        }
    }
    
    //display all the station name when it is not overlapped
    std::vector <ezgl::point2d> displayedCoord;
    displayedCoord.clear();

    for (int stationIdx = 0; stationIdx < stationName.size(); stationIdx ++) {
        bool displayStation = true;

        for (int j = 0; j < displayedCoord.size(); j ++) {
            if (abs(stationCoordindate[stationIdx].x - displayedCoord[j].x) < (stationWidth * widthToPixelRatio) 
                    && abs(stationCoordindate[stationIdx].y - displayedCoord[j].y) < (stationWidth * widthToPixelRatio)) {
                displayStation = false;
                break;
            }

        }

        if (displayStation) {
            //draw the text of the contents
            g->set_text_rotation(0);
            g->set_color(textColor);
            g->set_font_size(10);
            g->draw_text(stationCoordindate[stationIdx], stationName[stationIdx]);
            displayedCoord.push_back(stationCoordindate[stationIdx]);
        }
    } 
}

