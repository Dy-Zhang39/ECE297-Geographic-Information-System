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

#include <iostream>
#include <chrono>
#include <thread>

double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude
double avgLat;                  //the average latitude of the map
double featureToWorldRatio = 0.0001;
double textDisplayRatio = 0.01;
double streetToWorldRatio = 0.5;
double streetToWorldRatio1 = 0.1;

extern StreetSegment* STREET_SEGMENTS;
extern Street* STREETS;
IntersectionIdx previousHighlight = -1;

//helper functions
void initialSetUp(ezgl::application *application, bool new_window);
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);

void searchButton(GtkWidget *widget, ezgl::application *application);

int clickToHighlightClosestIntersection(LatLon pos);
void drawStreet(ezgl::renderer *g, ezgl::rectangle world);

void drawFeature(ezgl::renderer *g, ezgl::rectangle world);
void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id);
void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea);

void drawFeature(ezgl::renderer *g);
void displayStreetName(ezgl::renderer *g, ezgl::rectangle world);


void intersectionPopup(ezgl::application *application, IntersectionIdx id);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);

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
    application.run(initialSetUp, actOnMouseClick, nullptr, nullptr);
}

void draw_main_canvas (ezgl::renderer *g){
    g->draw_rectangle({0,0}, {1000,1000});
        
    ezgl::rectangle world = g->get_visible_world();
    drawStreet(g, world);
    drawFeature(g, world);
    displayStreetName(g, world);
    
    for (size_t i = 0; i < intersections.size(); ++i) {
        float x = xFromLon(intersections[i].position.longitude());
        float y = yFromLat(intersections[i].position.latitude());

        float width = 5;
        float height = width;
        
        if (intersections[i].isHighlight) {
            g->set_color(ezgl::GREY_75);
            std::cout<<world.height()<<" "<<world.width();
            g->fill_rectangle({x - world.width()/8, y - world.height()/35}, {x + world.width()/8, y + world.height()/35});
            g->set_font_size(10);
            g->set_color(ezgl::BLACK);  
            
            g->draw_text({x, y}, intersections[i].name);
            g->set_color(ezgl::RED);
        } else {
            g->set_color(ezgl::GREY_55);
        }
        
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

void initialSetUp(ezgl::application *application, bool /*new_window*/){
    application->update_message("Map is loaded successfully");
    
    application->create_button("Search", 5, searchButton);
}

void searchButton(GtkWidget */*widget*/, ezgl::application *application){
    //update message
    application->update_message("Search button is pressed");

    // Redraw the main canvas
    application->refresh_drawing();
}
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    std::cout << "Button " << event->button << " is clicked\n";
    
    LatLon pos = LatLon(latFromY(y), lonFromX(x));
    IntersectionIdx id = clickToHighlightClosestIntersection(pos);
    
    //intersectionPopup(app, id);

    app->refresh_drawing();
}

//mouse click to highlight the closest intersection
IntersectionIdx clickToHighlightClosestIntersection(LatLon pos){
    IntersectionIdx id = findClosestIntersection(pos);
    intersections[id].isHighlight = true;
    
    if (previousHighlight != -1){
        intersections[previousHighlight].isHighlight = false;   //un-highlight the previous intersection that is clicked
    }
    
    previousHighlight = id;
    std::cout << "Closest Intersection: " << intersections[id].name << "\n";
    
    return id;
}

void drawStreet(ezgl::renderer *g, ezgl::rectangle world){
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    
    for(int StreetSegmentsID=0; StreetSegmentsID<getNumStreetSegments(); StreetSegmentsID++ ){
        if (findStreetLength(getStreetSegmentInfo(StreetSegmentsID).streetID) > diagLength * streetToWorldRatio1){
            for(int pointsID=1; pointsID < STREET_SEGMENTS->streetSegPoint[StreetSegmentsID].size(); pointsID++){
                g->set_color(210,223,227,255);
                g->set_line_width(3);
                x1 = xFromLon(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID - 1].longitude());
                y1 = yFromLat(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID- 1].latitude());

                x2 = xFromLon(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID].longitude());
                y2 = yFromLat(STREET_SEGMENTS->streetSegPoint[StreetSegmentsID][pointsID].latitude());


                g->draw_line({x1,y1}, {x2, y2});
            }
        }
        
        
    }
    return;
}


void displayStreetName(ezgl::renderer *g, ezgl::rectangle world){
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
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
                if (world.contains(x, y)){
                    ezgl::point2d point(x, y);
                    
                    inViewSegment.push_back(point);
                }
            }
        }
        double degree = atan2(y1-y, x1-x)/kDegreeToRadian;
        g->set_font_size(10);
        g->set_color(ezgl::BLACK);  
        if (degree < 0){
            degree = degree+180;
        }
        g->set_text_rotation(degree);
        if (inViewSegment.size() > 1) {
            g->draw_text(inViewSegment[inViewSegment.size()/2], streetName);
        }
        /*auto streetID = getStreetSegmentInfo(StreetSegmentsID).streetID;
        //find a location to draw street text
        
        if(streetID != streetIdUsed){
            std::string streetName = getStreetName(streetID);
            if (streetName.compare("<unknown>") != 0 && findStreetLength(streetID) > diagLength * streetToWorldRatio) {
                streetIdUsed = streetID;
                double x, y;
                x = xFromLon(getIntersectionPosition(getStreetSegmentInfo(StreetSegmentsID).from).longitude());

                y = yFromLat(getIntersectionPosition(getStreetSegmentInfo(StreetSegmentsID).from).latitude());
                ezgl::point2d center(x, y);
                g->set_font_size(10);
                g->set_color(ezgl::BLACK);
                g->draw_text(center, streetName);
            }
        }*/
    }
}


void drawFeature(ezgl:: renderer *g, ezgl::rectangle world){
    //std::cout << "World width: " << world.width() << "\n";
    double visibleArea = world.area();
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double featureArea = findFeatureArea(featureID);
        if (featureArea >= visibleArea * featureToWorldRatio){
            drawFeatureByID(g, featureID);
        }
    }
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        double featureArea = findFeatureArea(featureID);
        if (featureArea >= visibleArea * textDisplayRatio){
            displayFeatureNameByID(g, featureID, visibleArea, featureArea);
        }
    }
}

void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id){
    std::vector<ezgl::point2d> points;
    FeatureType featureType = getFeatureType(id);
    switch (featureType) {
        case PARK:
            g->set_color(ezgl::GREEN);
            break;
        case BEACH:
            g->set_color(ezgl::YELLOW);
            break;
        case LAKE:
            g->set_color(ezgl::BLUE);
            break;
        case RIVER:
            g->set_color(ezgl::BLUE);
            break;
        case ISLAND:
            g->set_color(ezgl::GREEN);
            break;
        case BUILDING:
            g->set_color(ezgl::GREY_75);
            break;
        case GREENSPACE:
            g->set_color(ezgl::GREEN);
            break;
        case GOLFCOURSE:
            g->set_color(ezgl::GREEN);
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
        //std::cout << featureType << "::" << featureName << "   Area: " << findFeatureArea(id) << "\n";
        ezgl::point2d* midPt = new ezgl::point2d(xAvg, yAvg);
        g->draw_text(*midPt, featureName);
    }
}


void intersectionPopup(ezgl::application *application, IntersectionIdx id) {
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
}