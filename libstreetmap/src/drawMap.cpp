/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m2.h"
#include "drawMap.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m1.h"
#include "StreetsDatabaseAPI.h"


double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);
void clickToHighlightClosestIntersection(LatLon pos);
double avgLat;                  //the average latitude of the map
IntersectionIdx previousHighlight = -1;

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

        float width = 100;
        float height = width;
        
        if (intersections[i].isHighlight)
            g->set_color(ezgl::RED);
        else
            g->set_color(ezgl::GREY_55);
        
        g->fill_rectangle({x - width/2, y - height/2},
                          {x + width/2, y + height/2});
    }
    
    

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