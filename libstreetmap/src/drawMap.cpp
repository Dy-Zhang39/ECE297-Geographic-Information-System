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
#include "StreetsDatabaseAPI.h"
#include <chrono>

#include <iostream>
#include <chrono>
#include <thread>

double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude
double avgLat;                  //the average latitude of the map
double worldRatio;              //the ratio of height to width of the screen
double textDisplayRatio = 0.01;
double streetToWorldRatio = 0.5;
double streetToWorldRatio1 = 0.09;
double EARTH_CIRCUMFERENCE = 2* M_PI * kEarthRadiusInMeters;

extern StreetSegment* STREET_SEGMENTS;
extern Street* STREETS;

std::vector <double> topFeatures;
std::vector <double> bottomFeatures;
std::vector <double> leftFeatures;
std::vector <double> rightFeatures;
std::vector <ezgl::point2d> featureTextPoints;
std::string selectedPOI = "all";

std::vector<IntersectionIdx> previousHighlight;

//helper functions
void initialSetUp(ezgl::application *application, bool new_window);
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);
void clearHighlightIntersection();

gboolean searchButtonIsClicked(GtkWidget *, gpointer data);

// function definitions for POI selection buttons
gboolean toggleAllPOI(GtkWidget *, gpointer data);
gboolean toggleEducationPOI(GtkWidget *, gpointer data);
gboolean toggleFoodPOI(GtkWidget *, gpointer data);
gboolean toggleMedicalPOI(GtkWidget *, gpointer data);
gboolean toggleTransportPOI(GtkWidget *, gpointer data);
gboolean toggleRecreationPOI(GtkWidget *, gpointer data);
gboolean toggleFinancePOI(GtkWidget *, gpointer data);
gboolean toggleGovPOI(GtkWidget *, gpointer data);
gboolean toggleOtherPOI(GtkWidget *, gpointer data);

IntersectionIdx clickToHighlightClosestIntersection(LatLon pos);
void drawStreet(ezgl::renderer *g, ezgl::rectangle world);

void drawFeature(ezgl::renderer *g, ezgl::rectangle world);
void initializeFeatureBounding();
void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id);
void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea, double widthToPixelRatio, double heightToPixelRatio);

void drawFeature(ezgl::renderer *g);
void displayStreetName(ezgl::renderer *g, ezgl::rectangle world);

void displayHighlightedIntersection(ezgl::renderer *g);
void displayPopupBox(ezgl::renderer *g, std::string title, std::string content, double x, double y, ezgl::rectangle world);

double textSize(ezgl::rectangle world);
double streetSize(ezgl::rectangle world);

void displayPOI(ezgl::renderer *g);
void displayPOIById(ezgl::renderer *g, POIIdx id, double widthToPixelRatio, double heightToPixelRatio);

struct intersection_data {
  LatLon position;
  std::string name;
  bool isHighlight = false;
};

std::vector<intersection_data> intersections;

void draw_main_canvas (ezgl::renderer *g);
void drawMap(){
    double maxLat = getIntersectionPosition(0).latitude();
    double minLat = maxLat;
    double maxLon = getIntersectionPosition(0).longitude();
    double minLon = maxLon;
    intersections.resize(getNumIntersections());

    for (int id = 0; id < getNumIntersections(); ++id) {
        intersections[id].position = getIntersectionPosition(id);
        intersections[id].name = getIntersectionName(id);

        maxLat = std::max(maxLat, intersections[id].position.latitude());
        minLat = std::min(minLat, intersections[id].position. latitude());
        maxLon = std::max(maxLon, intersections[id].position.longitude());
        minLon = std::min(minLon, intersections[id].position.longitude());
    }
    
    avgLat = (maxLat + minLat)/2;           //force the map to be a reactangle   


    // Initialize coordinates for feature bounding boxes.
    initializeFeatureBounding();

    
    worldRatio = (yFromLat(maxLat) - yFromLat(minLat))/(xFromLon(maxLon) - xFromLon(minLon));

    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    //initial set up, act on mouse press, act on mouse move, act on key press
    application.run(initialSetUp, actOnMouseClick, nullptr, nullptr);
}

void draw_main_canvas (ezgl::renderer *g){
    
    
    
    g->draw_rectangle({0,0}, {1000,1000});
   
    ezgl::rectangle world = g->get_visible_world();
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


    double elapsedSecondsFeature = double(featureEnd - begin) / CLOCKS_PER_SEC;
    double elapsedSecondsStreet = double(streetEnd-featureEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsPoi = double(poiEnd-streetEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsHighlightIntersection = double(highlighIntersectionEnd-poiEnd) / CLOCKS_PER_SEC;
    double elapsedSecondsStreetName = double(streetNameEnd-highlighIntersectionEnd) / CLOCKS_PER_SEC;
    
    std::cout << "Feature: "<<elapsedSecondsFeature << " Street: " << elapsedSecondsStreet << " POI:  " << elapsedSecondsPoi << 
            " HighlightIntersection: " << elapsedSecondsHighlightIntersection << " StreetName: " << elapsedSecondsStreetName << "\n";
    
    double totalTime = double(streetNameEnd - begin)/CLOCKS_PER_SEC;
    std::cout<<"total time" << totalTime << "\n";
    //std::clock_t intersection_end = clock();
    //double elapsedSecondsStreet = double(street_end-begin) / CLOCKS_PER_SEC;
    //double elapsedSecondsFeature = double(feature_end - street_end) / CLOCKS_PER_SEC;
    //double elapsedSecondsStreetName = double(street_name_end-feature_end) / CLOCKS_PER_SEC;
    //double elapsedSecondsPoi = double(poi_end-street_name_end) / CLOCKS_PER_SEC;
    //double elapsedSecondsIntersections = double(intersection_end-poi_end) / CLOCKS_PER_SEC;
    //std::cout << elapsedSecondsStreet << " -> (Load Feature) " << elapsedSecondsFeature << " -> " << elapsedSecondsStreetName << " -> (Load POI) " << elapsedSecondsPoi << " -> " << elapsedSecondsIntersections << "\n";

    //drawFeature(g, world);
    
/*<<<<<<< HEAD
    

=======
   /* for (size_t i = 0; i < intersections.size(); ++i) {
        float x = xFromLon(intersections[i].position.longitude());
        float y = yFromLat(intersections[i].position.latitude());

        float width = 5;
        float height = width;
        
        if (intersections[i].isHighlight) {

            g->set_color(ezgl::GREY_75);
            
            if (intersections[i].name.compare("<unknown>") != 0){
                displayPopupBox(g, "Intersection: ", intersections[i].name, x, y, world);
            }

            g->set_color(ezgl::RED);
        } else {
            g->set_color(ezgl::GREY_55);
        }
        
        g->fill_rectangle({x - width/2, y - height/2},
                          {x + width/2, y + height/2});
    }
>>>>>>> 767c592517ad55764cec93bf9124fed704595e65
*/

}

double xFromLon(double lon){
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(avgLat * kDegreeToRadian);
}
double yFromLat(double lat){
    return lat * kDegreeToRadian* kEarthRadiusInMeters;
}

double lonFromX(double x){
    return x/(kDegreeToRadian * kEarthRadiusInMeters * std::cos(avgLat * kDegreeToRadian));
}
double latFromY(double y){
    return y/(kDegreeToRadian* kEarthRadiusInMeters);
}

void initialSetUp(ezgl::application *application, bool /*new_window*/){
    application->update_message("Map is loaded successfully");
    
    GObject *search = application->get_object("SearchButton");
    g_signal_connect(search, "clicked", G_CALLBACK(searchButtonIsClicked), application);
    
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

gboolean searchButtonIsClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);
    
    GtkEntry* textEntry = (GtkEntry *) application ->get_object("TextInput");
    
    std::string text = gtk_entry_get_text(textEntry);
    std::string firstStreet, secondStreet;
    bool firstFinished = false;         //finished reading the first street

    clearHighlightIntersection();
    
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
    
    StreetIdx firstStreetIdx = -1, secondStreetIdx = -1;
    
    for(int idx = 0; idx < STREETS->streetNames.size(); idx++){
        std::string name = STREETS->streetNames[idx];
        if(name == firstStreet){
            firstStreetIdx = idx;
        }
        
        if (name == secondStreet){
            secondStreetIdx = idx;
        }
    }
    
    
    std::string output;
    
    ezgl::point2d sum(0, 0), center(0,0), largest(-1 * EARTH_CIRCUMFERENCE, -1 * EARTH_CIRCUMFERENCE), smallest(EARTH_CIRCUMFERENCE, EARTH_CIRCUMFERENCE);

    
    if (firstStreetIdx != -1 && secondStreetIdx != -1){
        output = getStreetName(firstStreetIdx) + ", " + getStreetName(secondStreetIdx);
       
        std::vector<IntersectionIdx> commonIntersection = findIntersectionsOfTwoStreets(std::make_pair(firstStreetIdx, secondStreetIdx));

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
                
                //highlight these intersections
                intersections[commonIntersection[idx]].isHighlight = true;
                previousHighlight.push_back(commonIntersection[idx]);
                
            }
            center.x = sum.x/commonIntersection.size();
            center.y = sum.y/commonIntersection.size();
            
            //set up the world zoom level
            double left, bottom, top, right;
            left = smallest.x - 500;
            bottom = smallest.y - 500;
            right = largest.x + 500;
            top = largest.y + 500;
            
            double width = right - left;
            double height = top - bottom;
            
            //making sure the width and height of the screen is in the world ratio
            if (width * worldRatio > height){
                
                bottom = center.y - (width * worldRatio) / 2;
                top =  center.y + (width * worldRatio) / 2;
                
            }else {
                
                left = center.x - (height / worldRatio) / 2;
                right = center.x + (height / worldRatio) / 2;
                
            }
            ezgl::rectangle world({left, bottom}, {right, top});
            canvas->get_camera().set_world(world);
            
        }else{
            output = "No Intersection found.";
        }
        
    }else{
        output = "Street can not be found.";
    }
    
    
    application->update_message(output);
    application->refresh_drawing();
    return true;
}

void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    std::cout << "Button " << event->button << " is clicked\n";
    
    LatLon pos = LatLon(latFromY(y), lonFromX(x));
    clickToHighlightClosestIntersection(pos);
    
    //intersectionPopup(app, id);
    app->refresh_drawing();
}

//mouse click to highlight the closest intersection

IntersectionIdx clickToHighlightClosestIntersection(LatLon pos){
    
    clearHighlightIntersection(); 

    IntersectionIdx id = findClosestIntersection(pos);
    intersections[id].isHighlight = true;
    
    
    
    previousHighlight.push_back(id);
    std::cout << "Closest Intersection: " << intersections[id].name << "\n";
    return id;
}

void clearHighlightIntersection(){
    for (auto iterator = previousHighlight.begin(); iterator != previousHighlight.end(); iterator++){
        intersections[*iterator].isHighlight = false;
    }
    previousHighlight.clear();
}

bool isAve(std::string streetName){
    bool isAve =false;
    //code here checks the street type,return true if ave still working on...
    return isAve;
}

void drawStreet(ezgl::renderer *g, ezgl::rectangle world){
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    


    for(int streetSegmentsID=0; streetSegmentsID<getNumStreetSegments(); streetSegmentsID++ ){
        //get street segment streetID
        StreetIdx streetId =getStreetSegmentInfo(streetSegmentsID).streetID;
        //get street name of this street segment
        std::string streetName = getStreetName(streetId);

        //if the street name is unknown or end with Ave, draw when user zoom in
        if(!streetName.compare("<unknown>")){
            //draw as user zooms in
        }else if (findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * streetToWorldRatio1){
            for(int pointsID=1; pointsID < STREET_SEGMENTS->streetSegPoint[streetSegmentsID].size(); pointsID++){
                
                x1 = xFromLon(STREET_SEGMENTS->streetSegPoint[streetSegmentsID][pointsID - 1].longitude());
                y1 = yFromLat(STREET_SEGMENTS->streetSegPoint[streetSegmentsID][pointsID- 1].latitude());

                x2 = xFromLon(STREET_SEGMENTS->streetSegPoint[streetSegmentsID][pointsID].longitude());
                y2 = yFromLat(STREET_SEGMENTS->streetSegPoint[streetSegmentsID][pointsID].latitude());
                if(world.contains(x1, y1) || world.contains(x2, y2)){
                    double speedLimit = getStreetSegmentInfo(streetSegmentsID).speedLimit;
                    if(speedLimit>22.23){
                       g->set_color(244, 208, 63, 255);
                       g->set_line_width(1.3*streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }else{
                       g->set_color(210,223,227,255);
                       g->set_line_width(streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }

                }
            }
        }
        
        
    }
    //  std::cout<<"Diag size: "<<diagLength<<std::endl;
    //  std::cout<<"street size: "<<streetSize(world)<<std::endl;
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
    double k=3.5;
    double streetSize = k*log(log(1000/sqrt(mapArea)+1.5))+5;
    
   //std::cout<<"text size: "<<textSize<<std::endl;
    return streetSize;
}


void displayStreetName(ezgl::renderer *g, ezgl::rectangle world){
    std::vector<ezgl::point2d> displayedNames;
    
    double fontSize = 10;
    double streetNameSize = 200;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    displayedNames.clear();
    for(int streetID = 0; streetID < getNumStreets(); streetID++ ){
        std::vector<ezgl::point2d> inViewSegment;
        inViewSegment.clear();
        std::string streetName = getStreetName(streetID);
        double x = 0, y = 0, x1 = 0, y1 = 0;
        for(int segmentIndex = 0; segmentIndex < STREETS->streetSegments[streetID].size(); segmentIndex++){
            
            if (streetName.compare("<unknown>") != 0 && findStreetLength(streetID) > diagLength * streetToWorldRatio) {
                x = xFromLon(getIntersectionPosition(getStreetSegmentInfo(STREETS->streetSegments[streetID][segmentIndex]).from).longitude());
                y = yFromLat(getIntersectionPosition(getStreetSegmentInfo(STREETS->streetSegments[streetID][segmentIndex]).from).latitude());
                x1 = xFromLon(getIntersectionPosition(getStreetSegmentInfo(STREETS->streetSegments[streetID][segmentIndex]).to).longitude());
                y1 = yFromLat(getIntersectionPosition(getStreetSegmentInfo(STREETS->streetSegments[streetID][segmentIndex]).to).latitude());

                if (world.contains(x, y) || world.contains(x1, y1)){
                    inViewSegment.push_back({x, y});
                }
            }
        }

        if (inViewSegment.size() > 2) {
            ezgl::point2d midPoint = inViewSegment[inViewSegment.size()/2];
            ezgl::point2d midNextPoint = inViewSegment[inViewSegment.size()/2 + 1];
            double degree = atan2(midNextPoint.y - midPoint.y, midNextPoint.x - midPoint.x) / kDegreeToRadian;
            bool overlap = false;
            double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
            double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
            for (int displayedNamesNum = 0; displayedNamesNum < displayedNames.size(); displayedNamesNum++ ){
                if(abs(midPoint.x - displayedNames[displayedNamesNum].x) < streetNameSize * widthToPixelRatio && 
                        abs(midPoint.y - displayedNames[displayedNamesNum].y) < streetNameSize * heightToPixelRatio){
                    overlap = true;
                }
            }
            
           
            
            if (degree < 0){
                degree = degree+180;
            }
            

            if(!overlap){
                g->set_font_size(fontSize);
                g->set_color(ezgl::BLACK);
                g->set_text_rotation(degree);
                g->draw_text(midPoint, streetName);
                displayedNames.push_back(midPoint);
            }
        }
    }
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
        
        // record the bounding boxes for each feature
        topFeatures.push_back(maxY);
        bottomFeatures.push_back(minY);
        leftFeatures.push_back(minX);
        rightFeatures.push_back(maxX);
    }
}

//draw all features in map
void drawFeature(ezgl:: renderer *g, ezgl::rectangle world){
    featureTextPoints.clear();
    
    double featureToWorldRatio = 0.0001;
    double visibleArea = world.area();
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    std::vector <FeatureIdx> islands;
    //loop through all features, if the feature area is at the predefined ratio of the visible area, draw it
    for (FeatureIdx featureID = 0; featureID < getNumFeatures(); featureID++){
        double minX = leftFeatures[featureID];
        double maxX = rightFeatures[featureID];
        double maxY = topFeatures[featureID];
        double minY = bottomFeatures[featureID];
        double featureArea = findFeatureArea(featureID);
        
        // If the feature is in the visible area, call helper function to display the feature.
        if ((world.contains(minX, minY) || world.contains(minX, maxY)
                || world.contains(maxX, minY) || world.contains(maxX, maxY)
                || (minY <= world.top() && maxY >= world.bottom() && minX <= world.right() && maxX >= world.left())
                ) 
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
    
    //loop through all features, if the feature area is at the predefined ratio of the visible area, display its name
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double featureArea = findFeatureArea(featureID);
        
        if (featureArea >= visibleArea * textDisplayRatio){
            displayFeatureNameByID(g, featureID, visibleArea, featureArea,widthToPixelRatio, heightToPixelRatio);
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
            g->set_color(204, 255, 153, 255);
            break;
        case BEACH:
            g->set_color(236, 226, 149, 255);
            break;
        case LAKE:
            //blue
            g->set_color(102, 204, 255, 255);
            break;
        case RIVER:
            g->set_color(102, 204, 255, 255);
            break;
        case ISLAND:
            g->set_color(204, 255, 153, 255);
            break;
        case BUILDING:
            g->set_color(ezgl::GREY_75);
            break;
        case GREENSPACE:
            g->set_color(204, 255, 153, 255);
            break;
        case GOLFCOURSE:
            g->set_color(110, 184, 66, 255);
            break;
        case STREAM:
            g->set_color(102, 204, 255, 255);
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
    
    FeatureType featureType = getFeatureType(id);
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){
        double x1, y1;
        x1 = xFromLon(getFeaturePoint(id, pt - 1).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt - 1).latitude());
        
        xAvg += x1;
        yAvg += y1;
        
    }
    xAvg /= getNumFeaturePoints(id) - 1;
    yAvg /= getNumFeaturePoints(id) - 1;
    
    //display the name only with features that is not type UNKNOWN
    bool displayName = false;
    switch (featureType) {
        case PARK:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case BEACH:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case LAKE:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case RIVER:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case ISLAND:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case BUILDING:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case GREENSPACE:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case GOLFCOURSE:
            g->set_color(ezgl::BLACK);
            displayName = true;
            break;
        case STREAM:
            g->set_color(ezgl::BLACK);
            break;
        case UNKNOWN:
            g->set_color(ezgl::BLACK);
            break;
        default:
            break;
    }
    bool overlapped = false;
    double featureRangeX = 30;
    double featureRangeY = 30;

    for(int displayedNameIdx = 0; displayedNameIdx < featureTextPoints.size(); displayedNameIdx++){
        if(abs(xAvg - featureTextPoints[displayedNameIdx].x) < featureRangeX * widthToPixelRatio && 
           abs(yAvg - featureTextPoints[displayedNameIdx].y) < featureRangeY * heightToPixelRatio){
            overlapped = true;
            break;
        }
    }
    //display the feature name at predefined text display ratio when its name is not <noname>
    std::string featureName = getFeatureName(id); 
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
    for (size_t i = 0; i < intersections.size(); ++i) {
        float x = xFromLon(intersections[i].position.longitude());
        float y = yFromLat(intersections[i].position.latitude());
        
        if (intersections[i].isHighlight) {

            g->set_color(ezgl::GREY_75);
            
            if (intersections[i].name.compare("<unknown>") != 0){
                displayPopupBox(g, "Intersection: ", intersections[i].name, x, y, world);
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
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), y - world.height() * windowToPopupBoxRatio  / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), y + world.height() * windowToPopupBoxRatio  / screenHeight });
    
    //draw the text of the title
    g->set_text_rotation(0);
    g->set_font_size(10);
    g->set_color(ezgl::BLACK);
    g->draw_text({x, y}, title);
    
    //draw the rectangle for the contents
    y -= world.height() * windowToPopupBoxRatio / screenHeight * 2;
    g->set_color(ezgl::GREY_75);
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), y - world.height() * windowToPopupBoxRatio / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), y + world.height() * windowToPopupBoxRatio / screenHeight });
                      
    //draw the text of the contents
    g->set_color(ezgl::BLACK);
    g->set_font_size(10);
    g->draw_text({x, y}, content);
}

//display all the POIs qualified for displaying
void displayPOI(ezgl::renderer *g) {
    double areaToShowPOI = 4200000;           // If the visible world area is smaller than this number, the POI will be displayed
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
    
    // Load icon image by poiType
    if (poiType.compare("ferry_termial") == 0){   
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/ferry.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("theatre") != std::string::npos && recreation) {       
        if(recreation) 
            iconSurface = g->load_png("./libstreetmap/images/theater.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("school") != std::string::npos 
            || poiType.rfind("university") != std::string::npos 
            || poiType.rfind("college") != std::string::npos)) {
        if (education) 
            iconSurface = g->load_png("./libstreetmap/images/university.png");
        else 
            displayPOI = false;

    } else if (poiType.rfind("parking") != std::string::npos) {       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/parkinggarage.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fast_food") != std::string::npos) {       
        if (food) 
            iconSurface = g->load_png("./libstreetmap/images/fastfood.png");
        else 
            displayPOI = false;

    } else if (poiType.compare("community_centre") == 0) {       
        if (other) 
            iconSurface = g->load_png("./libstreetmap/images/communitycentre.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("pharmacy") == 0) {       
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/images/drogerie.png");
        else 
            displayPOI = false;
       
    } else if (poiType.rfind("cafe") != std::string::npos) {       
        if (food) 
            iconSurface = g->load_png("./libstreetmap/images/coffee.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("place_of_worship") == 0) {        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/images/chapel-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("bank") == 0) {       
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/images/bank.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("atm") == 0) {       
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/images/atm-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("cinema") == 0) {       
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/images/cinema.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("hospital") == 0 
            || poiType.compare("doctors") == 0 
            || poiType.find("clinic") != std::string::npos) {        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/images/hospital-building.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("library") == 0) {        
        if (education) 
            iconSurface = g->load_png("./libstreetmap/images/library.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("restaurant") != std::string::npos 
            || poiType.rfind("food_") != std::string::npos)) {        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/images/restaurant.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("police") == 0) {       
        if (gov) 
            iconSurface = g->load_png("./libstreetmap/images/police.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("gym") != std::string::npos 
            || poiType.rfind("weight") != std::string::npos) {
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/images/fitness.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("dentist") != std::string::npos 
            || poiType.rfind("orthodon") != std::string::npos) {
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/images/dentist.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bus_s") != std::string::npos) {       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/bus.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fuel") != std::string::npos) {       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/fillingstation.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("child") != std::string::npos) {        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/images/daycare.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bicyle") != std::string::npos) {       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/bicyle_parking.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("toilets") != std::string::npos) {        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/images/toilets_inclusive.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("post_") != std::string::npos) {       
        if (other) iconSurface = g->load_png("./libstreetmap/images/postal.png");
        else displayPOI = false;
        
    } else if (poiType == "airport" && transport) {     
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/images/airport.png");
        else 
            displayPOI = false;
        
    } else if (other) {
        iconSurface = g->load_png("./libstreetmap/images/sight-2.png");
        
    } else {                                // else do not display anything
        
        displayPOI = false;
    }
    
    if (displayPOI) {
        // make the middle bottom of the icon at the poi location
        double surfaceWidth = (double)cairo_image_surface_get_width(iconSurface) * widthToPixelRatio;
        double surfaceHeight = (double)cairo_image_surface_get_height(iconSurface) * heightToPixelRatio;
        g->draw_surface(iconSurface, {x - surfaceWidth / 2 , y + surfaceHeight} );

        // display poi name
        g->set_color(ezgl::BLACK);
        g->set_font_size(10);
        g->set_text_rotation(0);
        g->draw_text({x, y }, poiName);
    }
}

/*void intersectionPopup(ezgl::application *application, IntersectionIdx id) {
    GObject *window;            // the parent window over which to add the dialog
    GtkWidget *content_area;    // the content area of the dialog
    GtkWidget *label;           // the label we will create to display a message in the contentarea
    GtkWidget *dialog;          // the dialog box we will create
 
    window = application->get_object(application->get_main_window_id().c_str());
    dialog = gtk_dialog_new_with_buttons(
            "Intersection",(GtkWindow*) window,GTK_DIALOG_MODAL,
            ("OK"),GTK_RESPONSE_ACCEPT);
    
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(intersections[id].name.c_str());
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    // The main purpose of this is to show dialog's child widget, label
    gtk_widget_show_all(dialog);// Connecting the "response" signal from the user to the associated callback function
    g_signal_connect(GTK_DIALOG(dialog),"response",G_CALLBACK(on_dialog_response),NULL);
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data){
    // For demonstration purposes, this will show the int value of the response type
    std::cout << "response is ";
    switch(response_id) {
        case GTK_RESPONSE_ACCEPT:
            std::cout << "GTK_RESPONSE_ACCEPT ";
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ??X?? button) ";
            break;
        case GTK_RESPONSE_REJECT:
            std::cout << "GTK_RESPONSE_REJECT ";
            break;
        default:std::cout << "UNKNOWN ";
        break;
    }
    std::cout << "(" << response_id << ")\n";
    gtk_widget_destroy(GTK_WIDGET (dialog));
}*/
