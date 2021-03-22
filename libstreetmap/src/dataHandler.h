/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dataHandler.h
 * Description: Some extra function created in m1.cpp
 */

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

//initialize global variables about and intersection
void street_Intersection();

//initialize global variables about street
void street_Info();

//initialize global variables about partial street name
void streetPartialName();

//resize all the global variable
void resizeData();

void initializeFeatureBounding();

double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude

//close the streetMap data base and osm data base
void closeDataBase();


//an algorithm that gives how much edit I need to let the two string to be the same
int levenshteinDistance(std::string first, std::string second);

// find a min/max LatLon from 2 points
LatLon findMaxMin(LatLon point, LatLon current, std::string method);

// load suway info
void loadSubways();

#endif /* DATAHANDLER_H */

