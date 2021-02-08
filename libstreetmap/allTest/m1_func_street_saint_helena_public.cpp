
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

SUITE(street_queries_public_saint_helena) {

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

    TEST_FIXTURE(MapFixture, all_street_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {0, 5, 111, 113, 117, 118, 130, 131, 135, 137, 138, 146, 172, 257, 271, 278};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(31)));

        expected = {5, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6)));

        expected = {13, 14, 15, 16, 17};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(1)));

        expected = {13, 44, 46, 262, 263, 360, 412, 414};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(30)));

        expected = {16, 358};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(26)));

        expected = {17, 356, 357};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(21)));

        expected = {23, 106};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(28)));

        expected = {24, 339, 363};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(24)));

        expected = {44, 45};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(3)));

        expected = {46, 47, 48};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4)));

        expected = {49, 276, 277, 284, 357, 365};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(32)));

        expected = {50, 51, 54, 162, 164, 168, 175, 184, 208, 224, 238, 239, 244, 259, 277, 306, 307, 316, 320, 342, 347, 366, 367, 368, 369, 370, 371};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(27)));

        expected = {53, 265, 277};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15)));

        expected = {55, 304, 351, 352, 353, 354, 355};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(18)));

        expected = {112, 136, 137};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(7)));

        expected = {149, 150, 153, 154, 155, 156, 157, 158, 159, 162, 212, 286, 289, 290, 291, 292, 392};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(8)));

        expected = {167, 174, 259, 311, 312, 313};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(19)));

        expected = {176, 177, 178, 179, 180, 181, 182, 183};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(9)));

        expected = {183, 184};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(10)));

        expected = {188, 189, 190, 191, 192, 193};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(11)));

        expected = {193, 237, 238};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(13)));

        expected = {195, 196, 247, 248};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12)));

        expected = {210, 400, 401};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(29)));

        expected = {260, 261};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(14)));

        expected = {277, 283};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16)));

        expected = {282, 300, 301};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17)));

        expected = {348, 349, 350};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20)));

        expected = {359, 360, 364};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(23)));

        expected = {362, 365};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(25)));

        expected = {371, 398, 417};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(33)));

    } //all_street_intersections

    TEST_FIXTURE(MapFixture, intersection_ids_from_street_ids) {
        std::vector<IntersectionIdx> expected;

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(1, 23))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(5, 18))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7, 10))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(9, 28))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(12, 27))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(13, 15))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15, 14))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(17, 4))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(20, 27))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(21, 5))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(21, 24))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(22, 7))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(23, 27))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(25, 33))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(32, 30))));

        expected = {14, 15, 17};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(1, 0))));

        expected = {23};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(28, 2))));

        expected = {45};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(3, 0))));

        expected = {46};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(30, 4))));

        expected = {162, 168, 175, 208, 224, 238, 239, 244, 259, 307, 316, 320, 347, 367, 370, 371};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(27, 0))));

        expected = {188, 191, 193};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(11, 0))));

        expected = {193};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(13, 11))));

        expected = {210};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(29, 0))));

        expected = {262, 263};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(30, 0))));

        expected = {277};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15, 27))));

        expected = {277};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16, 15))));

        expected = {282, 300, 301};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(17, 0))));

        expected = {357};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(21, 32))));

        expected = {360};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(23, 30))));

        expected = {362};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(25, 0))));

    } //intersection_ids_from_street_ids

    TEST_FIXTURE(MapFixture, street_ids_from_partial_street_name) {
        std::vector<StreetIdx> expected;

        expected = {1};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Mar")));

        expected = {1};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Mark")));

        expected = {3};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Castle Terrace")));

        expected = {4};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Churc")));

        expected = {6};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Deadwood")));

        expected = {7};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Colt")));

        expected = {7, 9, 24, 27};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Co")));

        expected = {8};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Caso")));

        expected = {8};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Cason")));

        expected = {8};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Casons")));

        expected = {10};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Wireless Station D")));

        expected = {10};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Wireless Station Drive")));

        expected = {11};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Sapper Way")));

        expected = {11, 12, 18};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Sa")));

        expected = {13};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Red Gate")));

        expected = {14};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("The Briars")));

        expected = {18};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Sandy Bay Ridge")));

        expected = {19};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Evergre")));

        expected = {19};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Evergreen ")));

        expected = {23};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Seafron")));

        expected = {23, 26};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Se")));

        expected = {24};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Constitution Hill")));

        expected = {25};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Shy")));

        expected = {25};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Shy Road")));

        expected = {26};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Seales Corner")));

        expected = {27};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Comm")));

        expected = {27};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Commonweal")));

        expected = {27};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Commonwealt")));

        expected = {29};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Clay Gut")));

        expected = {33};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Unnamed Road")));

    } //street_ids_from_partial_street_name

    TEST_FIXTURE(MapFixture, bounding_box_of_street) {
        LatLonBounds actual;
        std::vector<LatLon> expected_vec, actual_vec;

        expected_vec = {LatLon(-15.99131298065185547, -5.75297117233276367), LatLon(-15.95596027374267578, -5.71805477142333984)};
        actual = findStreetBoundingBox(8);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.95535945892333984, -5.72055673599243164), LatLon(-15.95499324798583984, -5.71955871582031250)};
        actual = findStreetBoundingBox(33);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.94886493682861328, -5.72114181518554688), LatLon(-15.94795513153076172, -5.71809005737304688)};
        actual = findStreetBoundingBox(13);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.95596027374267578, -5.71993112564086914), LatLon(-15.92962360382080078, -5.71801614761352539)};
        actual = findStreetBoundingBox(27);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.93684768676757812, -5.71842050552368164), LatLon(-15.93439102172851562, -5.71624851226806641)};
        actual = findStreetBoundingBox(9);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.93265819549560547, -5.72195291519165039), LatLon(-15.92659473419189453, -5.71278381347656250)};
        actual = findStreetBoundingBox(32);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.93054485321044922, -5.71847295761108398), LatLon(-15.92643260955810547, -5.71541643142700195)};
        actual = findStreetBoundingBox(25);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.93253612518310547, -5.71723222732543945), LatLon(-15.92625522613525391, -5.71227216720581055)};
        actual = findStreetBoundingBox(1);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.92937660217285156, -5.71525382995605469), LatLon(-15.92773628234863281, -5.71432495117187500)};
        actual = findStreetBoundingBox(26);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(-15.93575763702392578, -5.71053838729858398), LatLon(-15.92155170440673828, -5.68830490112304688)};
        actual = findStreetBoundingBox(28);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

    } //bounding_box_of_street

} //street_queries_public_saint_helena

