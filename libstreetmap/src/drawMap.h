/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   drawMap.h
 * Author: hanxuleo
 *
 * Created on March 2, 2021, 10:17 PM
 */

#ifndef DRAWMAP_H
#define DRAWMAP_H

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

//highlight the closest intersection when user click somewhere 
IntersectionIdx clickToHighlightClosestIntersection(LatLon pos);

//helper functions

//un-highlight all the highlight intersection
void clearHighlightIntersection();

//world width to height ratio
void initializeCurrentWorldRatio();

bool isAve(std::string streetName);

void drawStreet(ezgl::renderer *g, ezgl::rectangle world);

double textSize(ezgl::rectangle world);

double streetSize(ezgl::rectangle world);

void displayStreetName(ezgl::renderer *g, ezgl::rectangle world);

//functions for drawing features and display their names
void drawFeature(ezgl::renderer *g, ezgl::rectangle world);

void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id);

void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea, double widthToPixelRatio, double heightToPixelRatio);

void storeStreetSeg(std::string streetName, std::vector<ezgl::point2d> &inViewSegment, int streetID, int diagLength, ezgl::rectangle world);

void displayHighlightedIntersection(ezgl::renderer *g);

void displayPopupBox(ezgl::renderer *g, std::string title, std::string content, double x, double y, ezgl::rectangle world);

//functions for displaying POI
void displayPOI(ezgl::renderer *g);

bool displayPOIById(ezgl::renderer *g, POIIdx id, double widthToPixelRatio, double heightToPixelRatio, bool transportationOnly);

void drawOneWayStreet(ezgl::renderer *g, double diagLength);

void drawArrow(ezgl::renderer *g, ezgl::point2d position, double theta);

//covert name of the city to the path to the city file
std::string convertNameToPath(std::string name);

//find all the possible location from two sets of street name;
std::vector<IntersectionIdx> findAllPossibleIntersections (std::vector<StreetIdx> first, std::vector<StreetIdx> second);

//import every name to the drop-down bar
void importNameToTheBar(GtkComboBoxText* bar);

//functions for displaying subways
void loadSubway(ezgl::renderer *g);

//change all letters to lower case, ignore all the space, separate two street name from a string
std::pair <std::string, std::string> getStreetNames(std::string text);

//get the world zoom level that fits all the intersections
ezgl::rectangle getZoomLevelToIntersections(std::vector<IntersectionIdx> commonIntersection);

////get the world zoom level that fits one intersection
ezgl::rectangle getZoomLevelToIntersections(IntersectionIdx id);
void drawMainCanvas (ezgl::renderer *g);

//add all the element in a vector to another vector
void addVectorToVector (std::vector<IntersectionIdx>& to, const std::vector<IntersectionIdx>& from);


//calculate the turn angle
double turnAngle(IntersectionIdx from, IntersectionIdx mid, IntersectionIdx to);

//find the turn direction using cross product
double crossProduct(IntersectionIdx from, IntersectionIdx mid, IntersectionIdx to);

//helper function when search button is pressed during the searching mode
void singleSearchMode(GtkEntry * entry, gpointer data);

//display starting point and destination point in the canvas
void displayStartAndDestination(ezgl::application * application);

//convert the name separated by & to the name separated by ,
std::string separateNamesByCommas(std::string locationName);

//set the global variable for starting point or destination depend on how many intersection are highlighted
void setFromOrDestination(ezgl::application *  application, bool isDestination);

//find the path between two intersections
void searchPathBtnClicked(GtkWidget *, gpointer data);

void displayIntersectionPopup(ezgl::renderer *g, ezgl::rectangle world, IntersectionIdx id);

void drawSegment(ezgl::renderer *g, ezgl::rectangle world, ezgl::color segColor, StreetSegmentIdx streetSegmentsID);

void drawRoute(ezgl::renderer *g, ezgl::rectangle world, std::vector<StreetSegmentIdx> route);

void drawIcon(ezgl::renderer *g, ezgl::rectangle world, ezgl::surface *iconSurface, StreetSegmentIdx location);

void displayTravelInfo(std::vector<StreetSegmentIdx> route);

void displayTravelInstructions(int numberOfStreets, int streetID,std::vector<StreetSegmentIdx> route,int totalTravelTime);
#endif /* DRAWMAP_H */

