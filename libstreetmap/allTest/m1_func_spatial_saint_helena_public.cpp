
#include "m1.h"
#include "unit_test_util.h"

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include <UnitTest++/UnitTest++.h>

#include <random>
#include <algorithm>
#include <set>

using ece297test::relative_error;
using ece297test::sorted;

SUITE(spatial_queries_public_saint_helena) {

    struct BaseMapFixture {
        BaseMapFixture() {
            //Load the map
            try {
                loadMap("/cad2/ece297s/public/maps/saint-helena.streets.bin");
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test setup: loadMap threw an exceptinon !!!!" << std::endl;
                throw; // re-throw exceptinon
            }
        }
    
        ~BaseMapFixture() {
            //Clean-up
            try {
                closeMap();
            } catch (const std::exception& e) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon. what(): " << e.what() << " !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            }
        }
    };


    struct MapFixture : BaseMapFixture {};

    TEST_FIXTURE(MapFixture, closest_poi) {
        std::set<POIIdx> valid_results;
        POIIdx actual;

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(-15.99659729003906250, -5.70543003082275391), "General Hospital"));

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(-15.98335742950439453, -5.69544744491577148), "General Hospital"));

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(-15.96384239196777344, -5.69518327713012695), "General Hospital"));

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(-15.94273185729980469, -5.67657995223999023), "General Hospital"));

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(-15.92930412292480469, -5.67123746871948242), "General Hospital"));

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.99930572509765625, -5.76344060897827148), "Butcher's Graves"));

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.96873569488525391, -5.75406932830810547), "Butcher's Graves"));

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.92393589019775391, -5.74639940261840820), "Butcher's Graves"));

        ECE297_CHECK_EQUAL(2, findClosestPOI(LatLon(-16.00590515136718750, -5.70784187316894531), "Boer Cemetery"));

        ECE297_CHECK_EQUAL(2, findClosestPOI(LatLon(-15.98373699188232422, -5.70105504989624023), "Boer Cemetery"));

        ECE297_CHECK_EQUAL(2, findClosestPOI(LatLon(-15.94169521331787109, -5.67872428894042969), "Boer Cemetery"));

        ECE297_CHECK_EQUAL(3, findClosestPOI(LatLon(-15.98956108093261719, -5.69358444213867188), "South West Point"));

        ECE297_CHECK_EQUAL(3, findClosestPOI(LatLon(-15.95928955078125000, -5.68612813949584961), "South West Point"));

        ECE297_CHECK_EQUAL(4, findClosestPOI(LatLon(-15.98962402343750000, -5.67998218536376953), "Flagstaff"));

        ECE297_CHECK_EQUAL(4, findClosestPOI(LatLon(-15.92969417572021484, -5.64759063720703125), "Flagstaff"));

        ECE297_CHECK_EQUAL(5, findClosestPOI(LatLon(-15.99443054199218750, -5.75084877014160156), "Lot's Wife's Ponds"));

        ECE297_CHECK_EQUAL(5, findClosestPOI(LatLon(-15.96485519409179688, -5.66559791564941406), "Lot's Wife's Ponds"));

        ECE297_CHECK_EQUAL(5, findClosestPOI(LatLon(-15.96221923828125000, -5.65219736099243164), "Lot's Wife's Ponds"));

        ECE297_CHECK_EQUAL(5, findClosestPOI(LatLon(-15.92456054687500000, -5.64697933197021484), "Lot's Wife's Ponds"));

        ECE297_CHECK_EQUAL(6, findClosestPOI(LatLon(-15.98119831085205078, -5.76369810104370117), "Thompsons Bay"));

        ECE297_CHECK_EQUAL(6, findClosestPOI(LatLon(-15.96916770935058594, -5.76188278198242188), "Thompsons Bay"));

        ECE297_CHECK_EQUAL(6, findClosestPOI(LatLon(-15.95489215850830078, -5.75431251525878906), "Thompsons Bay"));

        ECE297_CHECK_EQUAL(6, findClosestPOI(LatLon(-15.95419692993164062, -5.69319963455200195), "Thompsons Bay"));

        ECE297_CHECK_EQUAL(6, findClosestPOI(LatLon(-15.92219638824462891, -5.65973854064941406), "Thompsons Bay"));

        ECE297_CHECK_EQUAL(7, findClosestPOI(LatLon(-15.94731903076171875, -5.75680255889892578), "Prosperous Bay"));

        ECE297_CHECK_EQUAL(7, findClosestPOI(LatLon(-15.93289375305175781, -5.73170804977416992), "Prosperous Bay"));

        ECE297_CHECK_EQUAL(7, findClosestPOI(LatLon(-15.93141174316406250, -5.70126581192016602), "Prosperous Bay"));

        ECE297_CHECK_EQUAL(8, findClosestPOI(LatLon(-15.98215866088867188, -5.74930047988891602), "Blue Point"));

        ECE297_CHECK_EQUAL(8, findClosestPOI(LatLon(-15.94857788085937500, -5.68501377105712891), "Blue Point"));

        ECE297_CHECK_EQUAL(8, findClosestPOI(LatLon(-15.93003654479980469, -5.67493200302124023), "Blue Point"));

    } //closest_poi

    TEST_FIXTURE(MapFixture, closest_intersection) {
        std::set<IntersectionIdx> valid_results;
        IntersectionIdx actual;

        ECE297_CHECK_EQUAL(106, findClosestIntersection(LatLon(-15.92557716369628906, -5.67993307113647461)));

        ECE297_CHECK_EQUAL(136, findClosestIntersection(LatLon(-15.94136810302734375, -5.67632865905761719)));

        ECE297_CHECK_EQUAL(180, findClosestIntersection(LatLon(-15.93484401702880859, -5.71694469451904297)));

        ECE297_CHECK_EQUAL(185, findClosestIntersection(LatLon(-15.93824386596679688, -5.66978836059570312)));

        ECE297_CHECK_EQUAL(186, findClosestIntersection(LatLon(-15.95215511322021484, -5.72442626953125000)));

        ECE297_CHECK_EQUAL(196, findClosestIntersection(LatLon(-15.99211502075195312, -5.70818281173706055)));

        ECE297_CHECK_EQUAL(200, findClosestIntersection(LatLon(-15.96410846710205078, -5.73165512084960938)));

        ECE297_CHECK_EQUAL(248, findClosestIntersection(LatLon(-16.01629066467285156, -5.71199226379394531)));

        ECE297_CHECK_EQUAL(248, findClosestIntersection(LatLon(-15.99796295166015625, -5.70190429687500000)));

        ECE297_CHECK_EQUAL(249, findClosestIntersection(LatLon(-15.96500301361083984, -5.73743915557861328)));

        ECE297_CHECK_EQUAL(249, findClosestIntersection(LatLon(-15.96105766296386719, -5.73459911346435547)));

        valid_results = {250, 251};
        actual = findClosestIntersection(LatLon(-15.95084381103515625, -5.74608373641967773));
        ECE297_CHECK(valid_results.count(actual));

        ECE297_CHECK_EQUAL(251, findClosestIntersection(LatLon(-15.95145702362060547, -5.74287843704223633)));

        ECE297_CHECK_EQUAL(264, findClosestIntersection(LatLon(-15.92990398406982422, -5.73861217498779297)));

        ECE297_CHECK_EQUAL(273, findClosestIntersection(LatLon(-15.96873569488525391, -5.75406932830810547)));

        ECE297_CHECK_EQUAL(273, findClosestIntersection(LatLon(-15.96608734130859375, -5.74632930755615234)));

        ECE297_CHECK_EQUAL(274, findClosestIntersection(LatLon(-15.94291782379150391, -5.77382612228393555)));

        ECE297_CHECK_EQUAL(275, findClosestIntersection(LatLon(-15.92058086395263672, -5.76390171051025391)));

        valid_results = {275, 276};
        actual = findClosestIntersection(LatLon(-15.92397212982177734, -5.73987007141113281));
        ECE297_CHECK(valid_results.count(actual));

        ECE297_CHECK_EQUAL(290, findClosestIntersection(LatLon(-15.96496868133544922, -5.72022294998168945)));

        ECE297_CHECK_EQUAL(297, findClosestIntersection(LatLon(-15.92969417572021484, -5.64759063720703125)));

        ECE297_CHECK_EQUAL(300, findClosestIntersection(LatLon(-15.96121406555175781, -5.66072654724121094)));

        ECE297_CHECK_EQUAL(304, findClosestIntersection(LatLon(-15.98956108093261719, -5.69358444213867188)));

        ECE297_CHECK_EQUAL(321, findClosestIntersection(LatLon(-15.97782325744628906, -5.76156663894653320)));

        ECE297_CHECK_EQUAL(322, findClosestIntersection(LatLon(-15.99737358093261719, -5.77325439453125000)));

        ECE297_CHECK_EQUAL(340, findClosestIntersection(LatLon(-15.99687480926513672, -5.75948953628540039)));

        ECE297_CHECK_EQUAL(352, findClosestIntersection(LatLon(-15.98100376129150391, -5.70508718490600586)));

        ECE297_CHECK_EQUAL(383, findClosestIntersection(LatLon(-15.95892238616943359, -5.65125846862792969)));

        ECE297_CHECK_EQUAL(384, findClosestIntersection(LatLon(-15.95754241943359375, -5.64926624298095703)));

        ECE297_CHECK_EQUAL(390, findClosestIntersection(LatLon(-15.99313545227050781, -5.66105508804321289)));

    } //closest_intersection

} //spatial_queries_public_saint_helena

