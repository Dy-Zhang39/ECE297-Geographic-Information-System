
#ifndef GLOBAL_H
#define GLOBAL_H

#include "StreetsDatabaseAPI.h"
#include "ezgl/point.hpp"

// Stores the information of each node that has been calculated
struct PathNode {
    StreetSegmentIdx from;  //the street segment that connect this node from the last node
    double travelTime;      //travel time between starting point to the current point
    double distance;        //distance between this cureent point and the destination
    IntersectionIdx lastIntersection;
};

struct CalculateResult {
    double bestTime;                        //the time that need to travel according to the result vector
    std::vector <IntersectionIdx> result;
    std::vector <int> resultIdxIndex;
    double cpuTime;                         //the time that to complete this calculate or perturbation function
};

struct intersectionData {
  LatLon position;
  std::string name;
  bool isHighlight = false;
};

struct oneWaySegInfo {
    int fromX;
    int fromY;
    int toX;
    int toY;
    int distance;
};


//vector that store the street name and it travel time and distance
struct streetInfo{
    std::string streetName;
    std::string turn;
    int distance;
    int travelTime;
    double angle;
    
};

class Street {
public:
    std::vector<std::vector<IntersectionIdx>> streetIntersections; //store all intersection in one specific street for every street
    std::vector<std::vector<StreetSegmentIdx>> streetSegments; //store all segments of a specific street (in inner vector), store every street in outer vector
    std::vector<std::string> streetNames;                   //store every street name to the corresponding index location
    std::vector<std::vector<StreetIdx>> streetNamesOneChar; //index vector using the first one characters of the street name
    std::vector<std::vector<StreetIdx>> streetNamesTwoChar; //index vector using the first two characters of the street name
    std::vector<std::vector<StreetIdx>> streetNamesThreeChar; //index vector using the first three characters of the street name
    std::vector<double> streetLength; //the length of every street
};

//class for global variables about street segment
class StreetSegment{
public:
    std::vector<oneWaySegInfo> oneWaySegment;
    std::vector<double> streetSegLength;                                    //the length of every street segment
    std::vector<double> streetSegTravelTime;                                //the time required to tavel every street segment
    std::vector<std::vector<LatLon>> streetSegPoint;                         //the outside vector has the content of each street segment
                                                                            //the vector insides store the position of each point on the street segment such as start point, curve points, and end points
};

//class for global variables about intersection
class Intersection{
public:
    std::vector<std::vector<StreetSegmentIdx>> intersectionStreetSegments;  //store every street segments to one intersection for every intersection
    std::vector<intersectionData> intersectionInfo;
};

class Feature{
public:
    double top;
    double bottom;
    double left;
    double right;
    double area;
    Feature (double t, double b, double l, double r, double a){
        top = t;
        bottom = b;
        left = l;
        right = r;
        area = a;
    }   
};

class Subway {
public:
    std::string name;
    std::string line;
    ezgl::point2d location = {0, 0};
    int red;
    int green;
    int blue;
};

class City{
public:
    std::string mapPath;
    Street* street;
    StreetSegment* streetSegment;
    Intersection* intersection;
    double maxLat;
    double minLat;
    double maxLon;
    double minLon;
    double avgLat;
    double worldRatio;

    std::vector<Feature> featurePts;
    std::vector<Subway> subways;
};

class WavePoint {
public:
    //virtual ~WavePoint();
    IntersectionIdx idx;
    double heuristicTime;
    
    friend bool operator < (const WavePoint& lhs, const WavePoint& rhs);
    friend bool operator > (const WavePoint& lhs, const WavePoint& rhs);
    
    WavePoint();
    WavePoint(IntersectionIdx idx, double time);

};

double findDistanceBetweenIntersections(IntersectionIdx from, IntersectionIdx to);
#endif /* GLOBAL_H */

