
#ifndef GLOBAL_H
#define GLOBAL_H
class Street {
public:
    std::vector<std::vector<IntersectionIdx>> streetIntersections; //store all intersection in one specific street for every street
    std::vector<std::vector<StreetIdx>> streetNamesOneChar; //index vector using the first one characters of the street name
    std::vector<std::vector<StreetIdx>> streetNamesTwoChar; //index vector using the first two characters of the street name
    std::vector<std::vector<StreetIdx>> streetNamesThreeChar; //index vector using the first three characters of the street name
    std::vector<double> streetLength; //the length of every street
};

//class for global variables about street segment
class StreetSegment{
public:
    std::vector<double> streetSegLength;                                    //the length of every street segment
    std::vector<double> streetSegTravelTime;                                //the time required to tavel every street segment
};

//class for global variables about intersection
class Intersection{
public:
    std::vector<std::vector<StreetSegmentIdx>> intersectionStreetSegments;  //store every street segments to one intersection for every intersection

};


#endif /* GLOBAL_H */

