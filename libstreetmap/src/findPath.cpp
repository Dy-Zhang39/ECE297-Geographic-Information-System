/*
 * This file stores all the function implementations relates finding a path in the map
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
extern std::vector <StreetSegmentIdx> pathRoute;
extern std::vector<IntersectionIdx> previousHighlight;
extern IntersectionIdx fromPath;
extern IntersectionIdx toPath;
extern bool choosingStartingPoint; //the mode that ask user to choose a starting point
extern bool choosingDestination;    //the mode that ask user to choose a destination
extern std::string instructionString;
extern std::vector<streetInfo> travelPathInfo;

void setFromBtnClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    GtkSwitch* selectingMode = (GtkSwitch *)application->get_object("UsingPinLocationInstead");
    GtkEntry* textEntry = (GtkEntry *) application->get_object("TextInput2");
    
    bool isPinPoint  = gtk_switch_get_state (selectingMode);
    
    if(!isPinPoint){        
        singleSearchMode(textEntry, data);        
    }
    
    setFromOrDestination (application, false);

    
    application->refresh_drawing();
}

void setFromOrDestination(ezgl::application *  application, bool isDestination){
    
    GtkEntry* start = (GtkEntry*) application->get_object("TextInput2");
    GtkEntry* destination = (GtkEntry*) application->get_object("TextInput");
    
    //set the highlight intersection for from
    if (previousHighlight.size() == 1) {
        
        pathRoute.clear();
        if (!isDestination){
            fromPath = previousHighlight[0];
            
            //set the name of the the starting in the from text entry
            std::string fromName = separateNamesByCommas(getIntersectionName(fromPath));
            gtk_entry_set_text(start, fromName.c_str());
        }else{
            toPath = previousHighlight[0];
            std::string toName = separateNamesByCommas(getIntersectionName(toPath));
            gtk_entry_set_text(destination, toName.c_str());
        }

        displayStartAndDestination(application);

    } else if (previousHighlight.size() > 1) {
        application->update_message("Multiple intersections are selected, please choose(mouse clicking) one of them");
        
        pathRoute.clear();
        if (!isDestination){
            choosingStartingPoint = true;
        }else{
            choosingDestination = true;
        }
        

    } else {
        application->update_message("No intersection is selected, please make sure you are in the right mode");
    }
}

void displayStartAndDestination(ezgl::application * application){
    
    // Display from to points
    std::string fromName = "";
    std::string toName = "";
    
    int numOfIntersections = cities[currentCityIdx] -> intersection -> intersectionInfo.size();
    
    if (fromPath > 0 && fromPath < numOfIntersections) fromName = cities[currentCityIdx] -> intersection -> intersectionInfo[fromPath].name;
    if (toPath > 0 && toPath < numOfIntersections) toName = cities[currentCityIdx] -> intersection -> intersectionInfo[toPath].name;

    std::string output = "From: " + fromName + ",  To: " + toName;
    application->update_message(output);
    application->refresh_drawing();
}

void setToBtnClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    GtkSwitch* selectingMode = (GtkSwitch *)application->get_object("UsingPinLocationInstead");
    GtkEntry* textEntry = (GtkEntry *) application->get_object("TextInput");
    
    bool isPinPoint  = gtk_switch_get_state (selectingMode);
    if(!isPinPoint){
        singleSearchMode(textEntry, data);
    }
    
    setFromOrDestination (application, true);
    
    application->refresh_drawing();
}

void searchPathBtnClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    std::string main_canvas_id = application->get_main_canvas_id();
    
    auto canvas = application->get_canvas(main_canvas_id);
    clearHighlightIntersection();
    
    //find the route and travel time
    if (toPath >= 0 && fromPath >= 0) {
        pathRoute = findPathBetweenIntersections(fromPath,toPath,15);
    
        std::vector<IntersectionIdx> routeIntersections;
        
        //display no route is found
        if (pathRoute.size() == 0){
            instructionString = "No path can be found!";
            GtkLabel *label = (GtkLabel *) application->get_object("instructionLabel");
            gtk_label_set_text(label, &instructionString[0]);
            return;
        }
        
        //draw the found route
        for (int i = 0; i < pathRoute.size(); i++) {
            
            //store all the intersections of the route
            IntersectionIdx fromIdx = getStreetSegmentInfo(pathRoute[i]).from;
            IntersectionIdx toIdx = getStreetSegmentInfo(pathRoute[i]).to;
            routeIntersections.push_back(fromIdx);
            routeIntersections.push_back(toIdx);
        }
        
        
        if (routeIntersections.size() != 0){
            //zoom the screen to the path
            ezgl::rectangle newWorld = getZoomLevelToIntersections(routeIntersections);
            canvas->get_camera().set_world(newWorld);
        }
        application->refresh_drawing();
        GtkLabel *label = (GtkLabel *) application->get_object("instructionLabel");
        gtk_label_set_text(label, &instructionString[0]);
    }
}

//clear the route on screen when Clear Route button is clicked
void clearRouteBtnClicked(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
   
    fromPath = -1;
    toPath = -1;
    pathRoute.clear();
    
    //clear the route instructions
    instructionString = "";
    GtkLabel *label = (GtkLabel *) application->get_object("instructionLabel");
    gtk_label_set_text(label, &instructionString[0]);
    
    application->refresh_drawing();
    
}

void drawRoute(ezgl::renderer *g, ezgl::rectangle world, std::vector<StreetSegmentIdx> route) {
    ezgl::surface *iconSurface;
    
    //draw the found route
    for (int i = 0; i < route.size(); i++) {
        drawSegment(g, world, ezgl::RED, route[i]);
    }

    iconSurface = g->load_png("./libstreetmap/resources/images/tracking.png");
    drawIcon(g, world, iconSurface, toPath);
    
    //displayIntersectionPopup(g, world, toPath);
    //displayIntersectionPopup(g, world, fromPath);
    //display from/ to icon

    iconSurface = g->load_png("./libstreetmap/resources/images/start_pin.png");
    drawIcon(g, world, iconSurface, fromPath);

}

void displayTravelInfo(std::vector<StreetSegmentIdx> route) {

    instructionString = "";
    //the vector that store the travel street information
    travelPathInfo.clear();
    double distance = 0;
    double travelTime = 0;
    double angle = 0;
    int numberOfStreets = 0;
    int streetID = getStreetSegmentInfo(route[0]).streetID;
    std::string streetName;
    std::string previousStreetName = getStreetName(streetID);
    streetInfo street;
    IntersectionIdx from = 0;
    IntersectionIdx mid = 0;
    IntersectionIdx to = 0;
    int minutes = 60;
    IntersectionIdx from1, from2, from3, from4;

    //check through the route found previously, and collect street name, street distance 
    //and travel time for a street
    for (int segIdIndex = 0; segIdIndex < route.size(); segIdIndex++) {
        //find the start street name
        streetID = getStreetSegmentInfo(route[segIdIndex]).streetID;
        streetName = getStreetName(streetID);
        //find the total distance on the same street
        distance += findStreetSegmentLength(route[segIdIndex]);
        //calculate the total distance travel in the same street
        travelTime += findStreetSegmentTravelTime(route[segIdIndex]);
        //if street change or reach the destination update the street
        if (streetName != previousStreetName) {
            //track the turn point, before and after turn point
            if (segIdIndex > 0) {
                from1 = getStreetSegmentInfo(route[segIdIndex - 1]).from;
                from2 = getStreetSegmentInfo(route[segIdIndex - 1]).to;
                from3 = getStreetSegmentInfo(route[segIdIndex]).from;
                from4 = getStreetSegmentInfo(route[segIdIndex]).to;
                //make sure the from mid and to points are in correct order
                if (from1 == from3) {
                    mid = from1;
                    from = from2;
                    to = from4;
                } else if (from1 == from4) {
                    mid = from1;
                    from = from2;
                    mid = from3;
                } else if (from2 == from3) {
                    mid = from2;
                    from = from1;
                    to = from4;
                } else if (from2 == from4) {
                    mid = from2;
                    from = from1;
                    to = from3;
                }
                //check if it is right or left turn
                if (crossProduct(from, mid, to) > 0) {
                    street.turn = "left";
                } else {
                    street.turn = "right";
                }

            }
            //subtract the newly street  distance and travel time
            distance = distance - findStreetSegmentLength(route[segIdIndex]);
            //calculate the total distance travel in the same street
            travelTime = travelTime - findStreetSegmentTravelTime(route[segIdIndex]);
            //calculate the turn angle
            angle = turnAngle(from, mid, to);
            street.angle = angle;
            //update the street info and store in the vector
            street.distance = distance;
            street.streetName = previousStreetName;
            street.travelTime = travelTime / minutes;

            travelPathInfo.push_back(street);
            //track the total number of street traveled
            numberOfStreets++;
            //reset for new street distance and travel time
            distance = findStreetSegmentLength(route[segIdIndex]);
            travelTime = findStreetSegmentTravelTime(route[segIdIndex]);

        }

        if (segIdIndex == (route.size() - 1)) {// reach the last street segment
            if (segIdIndex > 0) {
                from = getStreetSegmentInfo(route[segIdIndex - 1]).from;
                mid = getStreetSegmentInfo(route[segIdIndex - 1]).to;
                to = getStreetSegmentInfo(route[segIdIndex]).to;
            }

            street.angle = 0;
            //update the street info and store in the vector
            street.distance = distance;
            street.streetName = streetName;
            street.travelTime = travelTime / minutes;
            travelPathInfo.push_back(street);
            //track the total number of street traveled
            numberOfStreets++;
        }

        //update the street change
        previousStreetName = streetName;

    }
    
    //First instruction, need to include the direction
    if (route.size() > 0) {

        //Get the from and to intersection position for the first segment
        LatLon fromLatLon= getIntersectionPosition(fromPath);
        LatLon toLatLon;
        StreetSegmentInfo temp = getStreetSegmentInfo(route[0]);
        if (temp.from == fromPath) {
            toLatLon = getIntersectionPosition(temp.to);
        } else {
            toLatLon = getIntersectionPosition(temp.from);
        }

        double distanceInit =  pow(pow(yFromLat(toLatLon.latitude()) - yFromLat(fromLatLon.latitude()), 2) + 
            pow(xFromLon(toLatLon.longitude()) - xFromLon(fromLatLon.longitude()), 2), 0.5);

        std::string instructionInit = "";
        
        //Get initial angle
        if(distanceInit){
            double angleInit = asin(
                (yFromLat(toLatLon.latitude()) - yFromLat(fromLatLon.latitude()) ) /
                    distanceInit) / kDegreeToRadian;

            double angleInit0 = acos(
                (xFromLon(toLatLon.longitude()) - xFromLon(fromLatLon.longitude())) /
                    distanceInit) / kDegreeToRadian;

            //Identify the direction based on angle
            if (abs(angleInit - angleInit0) < 0.001 && angleInit > 0) {
                if (angleInit < 30) {
                    instructionInit = "Head east \n";
                } else if (angleInit < 60) {
                    instructionInit = "Head northeast \n";
                } else {
                    instructionInit = "Head north \n";
                }
            } else if (abs(angleInit + angleInit0 - 180) < 0.001 && angleInit > 0 && angleInit0 > 0) {
                if (angleInit < 30) {
                    instructionInit = "Head west. ";
                } else if (angleInit < 60) {
                    instructionInit = "Head northwest. ";
                } else {
                    instructionInit = "Head north. ";
                }
            } else if (abs(angleInit + angleInit0) < 0.001 && angleInit < 0) {
                if (angleInit0 < 30) {
                    instructionInit = "Head east. ";
                } else if (angleInit0 < 60) {
                    instructionInit = "Head southeast. ";
                } else {
                    instructionInit = "Head south. ";
                }
            } else if (abs( 180 + angleInit - angleInit0) < 0.001 && angleInit < 0) {
                if (- angleInit < 30) {
                    instructionInit = "Head west. ";
                } else if (- angleInit < 60) {
                    instructionInit = "Head southwest. ";
                } else {
                    instructionInit = "Head south. ";
                }
            }

            instructionString += instructionInit;
        }
    }

    displayTravelInstructions(numberOfStreets, streetID, route);
}

void displayTravelInstructions(int numberOfStreets, int streetID, std::vector<StreetSegmentIdx> route) {
    streetInfo streetInformation;
    std::string streetName;

    //print the street travel instruction
    for (int i = 0; i < numberOfStreets - 1; i++) {
        streetInformation = travelPathInfo[i];

        //the turn angle is 175-185, travel straight
        if (streetInformation.angle < 185 && streetInformation.angle >= 175) {
            //check if travel time is under one minutes
            if (streetInformation.travelTime < 1) {
                //distance less than 1km
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance)
                            + " m, under 1 minute. Then continue straight \n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000)
                            + " km, under 1 minute. Then continue straight \n";
                }
            } else {//travel time greater than 1 minutes
                //travel time greater than 1 minutes
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance) + " m, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then continue straight \n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000) + " km, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then continue straight \n";
                }
            }
        } else if (streetInformation.angle < 175 && streetInformation.angle >= 120) {//slight turn
            //check if travel time is under one minutes
            if (streetInformation.travelTime < 1) {
                //distance less than 1km
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance)
                            + " m, under 1 minute. Then turn slightly " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000)
                            + " km, under 1 minute. Then turn slightly " + streetInformation.turn + "\n";
                }
            } else {//travel time greater than 1 minutes
                //travel time greater than 1 minutes
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance) + " m, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn slightly " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000) + " km, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn slightly " + streetInformation.turn + "\n";
                }
            }
        } else if (streetInformation.angle < 120 && streetInformation.angle >= 60) {//normal around 90 degree turn
            //check if travel time is under one minutes
            if (streetInformation.travelTime < 1) {
                //distance less than 1km
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance)
                            + " m, under 1 minute. Then turn  " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000)
                            + " km, under 1 minute. Then turn " + streetInformation.turn + "\n";
                }
            } else {//travel time greater than 1 minutes
                //travel time greater than 1 minutes
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance) + " m, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn  " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000) + " km, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn " + streetInformation.turn + "\n";
                }
            }
        } else if (streetInformation.angle < 60 && streetInformation.angle >= 0) {//sharp turn
            //check if travel time is under one minutes
            if (streetInformation.travelTime < 1) {
                //distance less than 1km
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance)
                            + " m, under 1 minute. Then turn sharply " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000)
                            + " km, under 1 minute. Then turn sharply " + streetInformation.turn + "\n";
                }
            } else {//travel time greater than 1 minutes
                //travel time greater than 1 minutes
                if (streetInformation.distance < 1000) {
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance) + " m, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn sharply " + streetInformation.turn + "\n";
                } else {//distance grater than 1km
                    instructionString += "Travel along " + streetInformation.streetName
                            + " for " + std::to_string(streetInformation.distance / 1000) + " km, "
                            + "for " + std::to_string(streetInformation.travelTime)
                            + " minute. Then turn sharply " + streetInformation.turn + "\n";
                }
            }
        }

    }
    //reach the destination
    streetInfo lastStreetInformation = travelPathInfo[numberOfStreets-1];
    streetID = getStreetSegmentInfo(route[route.size() - 1]).streetID;
    streetName = getStreetName(streetID);
    //print the last street

    instructionString += "Travel along " + lastStreetInformation.streetName + "for "
            + std::to_string(lastStreetInformation.distance) + " m," +" destination reached ! \n";
    //instructionString += "Destination " + streetName + " reached!\n";

}


double turnAngle(IntersectionIdx from, IntersectionIdx mid, IntersectionIdx to){
    //triangle with points A,B,C
    LatLon A= getIntersectionPosition(from);
    LatLon B=getIntersectionPosition(mid);
    LatLon C=getIntersectionPosition(to);
    
    double a=findDistanceBetweenTwoPoints(std::make_pair(B,C));
    double b=findDistanceBetweenTwoPoints(std::make_pair(C,A));
    double c=findDistanceBetweenTwoPoints(std::make_pair(A,B));
    //check if it is a valid angle, avoid straight path without turn
    if(a==0||b==0||c==0){
        return 0;
    }
    //turn angle
    double angle = acos((c*c+a*a-b*b)/(2*abs(c*a)))/kDegreeToRadian;
    
    return angle;
}