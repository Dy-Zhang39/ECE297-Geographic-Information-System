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

#include "StreetsDatabaseAPI.h"

//highlight the closest intersection when user click somewhere 
IntersectionIdx clickToHighlightClosestIntersection(LatLon pos);
double xFromLon(double lon);  //convert longitude to meter
double yFromLat(double lat);  //convert latitude to meter
double lonFromX(double x);    //convert meter to longitude
double latFromY(double y);    //convert meter to latitude


#endif /* DRAWMAP_H */

