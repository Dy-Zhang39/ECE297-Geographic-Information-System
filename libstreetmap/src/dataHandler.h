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

#endif /* DATAHANDLER_H */

