
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

SUITE(spatial_queries_public_toronto_canada) {

    struct BaseMapFixture {
        BaseMapFixture() {
            //Load the map
            try {
                loadMap("/cad2/ece297s/public/maps/toronto_canada.streets.bin");
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

        ECE297_CHECK_EQUAL(0, findClosestPOI(LatLon(43.87768554687500000, -79.15479278564453125), "Bathurst Street Terminal"));

        ECE297_CHECK_EQUAL(300, findClosestPOI(LatLon(43.64857482910156250, -79.20729827880859375), "Pizza Nova"));

        ECE297_CHECK_EQUAL(1353, findClosestPOI(LatLon(43.60358810424804688, -79.29006958007812500), "Esso"));

        ECE297_CHECK_EQUAL(1887, findClosestPOI(LatLon(43.63180541992187500, -79.30134582519531250), "KFC"));

        ECE297_CHECK_EQUAL(2715, findClosestPOI(LatLon(43.58143234252929688, -79.63671112060546875), "KFC"));

        ECE297_CHECK_EQUAL(3526, findClosestPOI(LatLon(43.57588577270507812, -79.11842346191406250), "College Street Dental"));

        ECE297_CHECK_EQUAL(3760, findClosestPOI(LatLon(43.82131958007812500, -79.20011138916015625), "Unitarian Congregation in Mississauga"));

        ECE297_CHECK_EQUAL(4459, findClosestPOI(LatLon(43.63007736206054688, -79.33528900146484375), "Sika Deer Chinese Cuisine"));

        ECE297_CHECK_EQUAL(5171, findClosestPOI(LatLon(43.55925750732421875, -79.71292877197265625), "Petro-Canada"));

        ECE297_CHECK_EQUAL(5555, findClosestPOI(LatLon(43.74128341674804688, -79.24493408203125000), "Bamiyan Kabob"));

        ECE297_CHECK_EQUAL(5898, findClosestPOI(LatLon(43.60490036010742188, -79.42757415771484375), "Discount"));

        ECE297_CHECK_EQUAL(6546, findClosestPOI(LatLon(43.52923202514648438, -79.37637329101562500), "Hero Certified Burgers"));

        ECE297_CHECK_EQUAL(6646, findClosestPOI(LatLon(43.90112686157226562, -79.58515167236328125), "Kayagum"));

        ECE297_CHECK_EQUAL(6793, findClosestPOI(LatLon(43.81262969970703125, -79.52041625976562500), "eggsmart"));

        ECE297_CHECK_EQUAL(7174, findClosestPOI(LatLon(43.60329818725585938, -79.20772552490234375), "Da-Maria Ristorante"));

        ECE297_CHECK_EQUAL(7327, findClosestPOI(LatLon(43.65444946289062500, -79.62783050537109375), "CIBC Banking Centre"));

        ECE297_CHECK_EQUAL(8283, findClosestPOI(LatLon(43.75362777709960938, -79.57941436767578125), "Dairy Queen"));

        ECE297_CHECK_EQUAL(8303, findClosestPOI(LatLon(43.87591171264648438, -79.01165008544921875), "Montana's Cookhouse"));

        ECE297_CHECK_EQUAL(8993, findClosestPOI(LatLon(43.56196212768554688, -79.05895996093750000), "Fionn MacCool's"));

        ECE297_CHECK_EQUAL(10235, findClosestPOI(LatLon(43.64162445068359375, -79.71449279785156250), "Our Crepe"));

        ECE297_CHECK_EQUAL(10650, findClosestPOI(LatLon(43.81660461425781250, -79.18713378906250000), "Second Cup"));

        ECE297_CHECK_EQUAL(11253, findClosestPOI(LatLon(43.91736984252929688, -79.71572113037109375), "Chipotle"));

        ECE297_CHECK_EQUAL(11359, findClosestPOI(LatLon(43.87495803833007812, -79.56263732910156250), "Pho Vietnam Family Restaurant"));

        ECE297_CHECK_EQUAL(11597, findClosestPOI(LatLon(43.68993759155273438, -79.15906524658203125), "George's Tastee"));

        ECE297_CHECK_EQUAL(12258, findClosestPOI(LatLon(43.86135482788085938, -79.52084350585937500), "Fancy Franks Gourmet Hot Dogs"));

        ECE297_CHECK_EQUAL(12383, findClosestPOI(LatLon(43.86809921264648438, -79.33656311035156250), "Shananxi Legend"));

        ECE297_CHECK_EQUAL(12414, findClosestPOI(LatLon(43.70549774169921875, -79.25299072265625000), "Bahn Mi Boys"));

        ECE297_CHECK_EQUAL(12477, findClosestPOI(LatLon(43.79001235961914062, -79.23818969726562500), "St.James's Gate"));

        ECE297_CHECK_EQUAL(12920, findClosestPOI(LatLon(43.69831848144531250, -79.65620422363281250), "Thai Express"));

        ECE297_CHECK_EQUAL(13192, findClosestPOI(LatLon(43.90982818603515625, -79.60358428955078125), "Coo Cafe Bread or Rice"));

    } //closest_poi

    TEST_FIXTURE(MapFixture, closest_intersection) {
        std::set<IntersectionIdx> valid_results;
        IntersectionIdx actual;

        ECE297_CHECK_EQUAL(1950, findClosestIntersection(LatLon(43.71936416625976562, -79.52052307128906250)));

        ECE297_CHECK_EQUAL(3963, findClosestIntersection(LatLon(43.77373886108398438, -79.47676086425781250)));

        valid_results = {6027, 6028, 6035, 6044, 6045, 107400};
        actual = findClosestIntersection(LatLon(43.48199844360351562, -79.40149688720703125));
        ECE297_CHECK(valid_results.count(actual));

        ECE297_CHECK_EQUAL(6043, findClosestIntersection(LatLon(43.56536865234375000, -79.34043121337890625)));

        ECE297_CHECK_EQUAL(7057, findClosestIntersection(LatLon(43.73253250122070312, -79.09116363525390625)));

        ECE297_CHECK_EQUAL(10200, findClosestIntersection(LatLon(43.82280731201171875, -79.18561553955078125)));

        ECE297_CHECK_EQUAL(10455, findClosestIntersection(LatLon(43.60358810424804688, -79.29006958007812500)));

        ECE297_CHECK_EQUAL(21861, findClosestIntersection(LatLon(43.71036148071289062, -79.60935211181640625)));

        ECE297_CHECK_EQUAL(28593, findClosestIntersection(LatLon(43.71545028686523438, -79.45131683349609375)));

        ECE297_CHECK_EQUAL(43835, findClosestIntersection(LatLon(43.90982818603515625, -79.60358428955078125)));

        ECE297_CHECK_EQUAL(43989, findClosestIntersection(LatLon(43.77970123291015625, -79.60786437988281250)));

        ECE297_CHECK_EQUAL(44286, findClosestIntersection(LatLon(43.91736984252929688, -79.71572113037109375)));

        ECE297_CHECK_EQUAL(44398, findClosestIntersection(LatLon(43.81575775146484375, -79.77580261230468750)));

        ECE297_CHECK_EQUAL(45040, findClosestIntersection(LatLon(43.77691650390625000, -79.58846282958984375)));

        valid_results = {47233, 106419};
        actual = findClosestIntersection(LatLon(43.74923324584960938, -79.02179718017578125));
        ECE297_CHECK(valid_results.count(actual));

        ECE297_CHECK_EQUAL(60274, findClosestIntersection(LatLon(43.73324203491210938, -79.53834533691406250)));

        ECE297_CHECK_EQUAL(68924, findClosestIntersection(LatLon(43.56804275512695312, -79.77233886718750000)));

        ECE297_CHECK_EQUAL(69945, findClosestIntersection(LatLon(43.57031250000000000, -79.68901824951171875)));

        ECE297_CHECK_EQUAL(75530, findClosestIntersection(LatLon(43.58732604980468750, -79.09315490722656250)));

        ECE297_CHECK_EQUAL(75834, findClosestIntersection(LatLon(43.71529388427734375, -79.55553436279296875)));

        ECE297_CHECK_EQUAL(79633, findClosestIntersection(LatLon(43.69831848144531250, -79.65620422363281250)));

        ECE297_CHECK_EQUAL(80287, findClosestIntersection(LatLon(43.83801651000976562, -79.22557830810546875)));

        ECE297_CHECK_EQUAL(88790, findClosestIntersection(LatLon(43.89463806152343750, -79.20742797851562500)));

        ECE297_CHECK_EQUAL(99994, findClosestIntersection(LatLon(43.87495803833007812, -79.56263732910156250)));

        ECE297_CHECK_EQUAL(114993, findClosestIntersection(LatLon(43.65697860717773438, -79.70158386230468750)));

        ECE297_CHECK_EQUAL(119886, findClosestIntersection(LatLon(43.64250946044921875, -79.35969543457031250)));

        ECE297_CHECK_EQUAL(127808, findClosestIntersection(LatLon(43.90194320678710938, -79.57025146484375000)));

        ECE297_CHECK_EQUAL(132962, findClosestIntersection(LatLon(43.83702087402343750, -79.14601898193359375)));

        ECE297_CHECK_EQUAL(133921, findClosestIntersection(LatLon(43.87591171264648438, -79.01165008544921875)));

        ECE297_CHECK_EQUAL(145146, findClosestIntersection(LatLon(43.85248565673828125, -79.43147277832031250)));

    } //closest_intersection

} //spatial_queries_public_toronto_canada

