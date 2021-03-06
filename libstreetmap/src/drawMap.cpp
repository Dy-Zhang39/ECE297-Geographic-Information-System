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

double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude
double avgLat;                  //the average latitude of the map
double featureToWorldRatio = 0.0001;
double textDisplayRatio = 0.01;

extern StreetSegment* STREET_SEGMENTS;
IntersectionIdx previousHighlight = -1;

//helper functions
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);
void clickToHighlightClosestIntersection(LatLon pos);
void drawStreet(ezgl::renderer *g);
void drawFeature(ezgl::renderer *g, ezgl::rectangle world);
void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id, double visibleArea, double featureArea);
void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea);

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

    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
 
    
    
    
    //initial set up, act on mouse press, act on mouse move, act on key press
    application.run(nullptr, actOnMouseClick, nullptr, nullptr);
}

void draw_main_canvas (ezgl::renderer *g){
    g->draw_rectangle({0,0}, {1000,1000});
    
    for (size_t i = 0; i < intersections.size(); ++i) {
        float x = xFromLon(intersections[i].position.longitude());
        float y = yFromLat(intersections[i].position.latitude());

        float width = 10;
        float height = width;
        
        if (intersections[i].isHighlight)
            g->set_color(ezgl::RED);
        else
            g->set_color(ezgl::GREY_55);
        
        g->fill_rectangle({x - width/2, y - height/2},
                          {x + width/2, y + height/2});
    }
    
    ezgl::rectangle world = g->get_visible_world();
    drawStreet(g);
    drawFeature(g, world);

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

void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    std::cout << "Button " << event->button << " is clicked\n";
    
    LatLon pos = LatLon(latFromY(y), lonFromX(x));
    clickToHighlightClosestIntersection(pos);
    

    
    app->refresh_drawing();
}

//mouse click to highlight the closest intersection
void clickToHighlightClosestIntersection(LatLon pos){
    IntersectionIdx id = findClosestIntersection(pos);
    intersections[id].isHighlight = true;
    
    if (previousHighlight != -1){
        intersections[previousHighlight].isHighlight = false;   //un-highlight the previous intersection that is clicked
    }
    
    previousHighlight = id;

    std::cout << "Closest Intersection: " << intersections[id].name << "\n";
}

void drawStreet(ezgl::renderer *g){
    for(int StreetSegmentsID=0; StreetSegmentsID<getNumStreetSegments(); StreetSegmentsID++ ){
        for(int pointsID=1; pointsID < STREET_SEGMENTS->streetSegPoint[StreetSegmentsID].size(); pointsID++){
            g->set_color(ezgl::BLACK);
            g->set_line_width(0);
            double x1, x2 ,y1,y2;
            x1 = xFromLon(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID - 1].longitude());
            y1 = yFromLat(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID- 1].latitude());
            
            x2 = xFromLon(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID].longitude());
            y2 = yFromLat(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID].latitude());
            
            g->draw_line({x1,y1}, {x2, y2});
        }
    }
}


void drawFeature(ezgl:: renderer *g, ezgl::rectangle world){
    std::cout << "World width: " << world.width() << "\n";
    double visibleArea = world.area();
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double featureArea = findFeatureArea(featureID);
        if (featureArea >= visibleArea * featureToWorldRatio){
            drawFeatureByID(g, featureID, visibleArea, featureArea);
        }
    }
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double featureArea = findFeatureArea(featureID);
        if (featureArea >= visibleArea * textDisplayRatio){
            displayFeatureNameByID(g, featureID, visibleArea, featureArea);
        }
    }
}

void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id, double visibleArea, double featureArea){
    std::vector<ezgl::point2d> points;
    bool displayName = false;
    FeatureType featureType = getFeatureType(id);
    switch (featureType) {
        case PARK:
            g->set_color(ezgl::GREEN);
            displayName = true;
            break;
        case BEACH:
            g->set_color(ezgl::YELLOW);
            displayName = true;
            break;
        case LAKE:
            g->set_color(ezgl::BLUE);
            displayName = true;
            break;
        case RIVER:
            g->set_color(ezgl::BLUE);
            break;
        case ISLAND:
            displayName = true;
            g->set_color(ezgl::GREEN);
            break;
        case BUILDING:
            g->set_color(ezgl::GREY_75);
            displayName = true;
            break;
        case GREENSPACE:
            g->set_color(ezgl::GREEN);
            displayName = true;
            break;
        case GOLFCOURSE:
            g->set_color(ezgl::GREEN);
            displayName = true;
            break;
        case STREAM:
            g->set_color(ezgl::BLUE);
            break;
        case UNKNOWN:
            g->set_color(ezgl::PURPLE);
            break;
        default:
            break;

    }
    g->set_line_width(0);
    //double xAvg = 0, yAvg = 0;
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){

        double x1, x2, y1, y2;
        x1 = xFromLon(getFeaturePoint(id, pt - 1).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt - 1).latitude());
        
        //xAvg += x1;
        //yAvg += y1;
        
        ezgl::point2d* ptr = new ezgl::point2d(x1, y1);
        
        points.push_back(*ptr);

        x2 = xFromLon(getFeaturePoint(id, pt).longitude());
        y2 = yFromLat(getFeaturePoint(id, pt).latitude());

        g->draw_line({x1,y1}, {x2, y2});
        
    }
    
    if (points.size() > 1 && (getFeaturePoint(id,0) == getFeaturePoint(id, getNumFeaturePoints(id) - 1))){
        g->fill_poly(points);
    }
    
    /*xAvg /= getNumFeaturePoints(id) - 1;
    yAvg /= getNumFeaturePoints(id) - 1;
    std::string featureName = getFeatureName(id); 
    if (displayName && featureName.compare("<noname>") != 0 && featureArea > visibleArea * textDisplayRatio){
        switch (featureType) {
            case PARK:
                g->set_color(ezgl::BLACK);
                break;
            case BEACH:
                g->set_color(ezgl::BLACK);
                break;
            case LAKE:
                g->set_color(ezgl::BLACK);
                break;
            case RIVER:
                g->set_color(ezgl::BLACK);
                break;
            case ISLAND:
                displayName = true;
                g->set_color(ezgl::BLACK);
                break;
            case BUILDING:
                g->set_color(ezgl::BLACK);
                break;
            case GREENSPACE:
                g->set_color(ezgl::BLACK);
                break;
            case GOLFCOURSE:
                g->set_color(ezgl::BLACK);
                break;
            case STREAM:
                g->set_color(ezgl::BLACK);
                break;
            case UNKNOWN:
                g->set_colo        

        r(ezgl::BLACK);
                break;
            default:
                break;
        }

        
        std::cout << featureType << "::" << featureName << "   Area: " << findFeatureArea(id) << "\n";
        ezgl::point2d* midPt = new ezgl::point2d(xAvg, yAvg);
        g->draw_text(*midPt, featureName);
    }*/
    


}

void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea){
    double xAvg = 0;
    double yAvg = 0;
    bool displayName = false;
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
    std::string featureName = getFeatureName(id); 
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
    if (displayName && featureName.compare("<noname>") != 0 && featureArea > visibleArea * textDisplayRatio){
        std::cout << featureType << "::" << featureName << "   Area: " << findFeatureArea(id) << "\n";
        ezgl::point2d* midPt = new ezgl::point2d(xAvg, yAvg);
        g->draw_text(*midPt, featureName);
    }
}