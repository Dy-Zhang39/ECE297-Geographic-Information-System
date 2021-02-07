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

SUITE(intersection_queries_public_saint_helena) {

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

    TEST_FIXTURE(MapFixture, intersection_street_names) {
        std::vector<std::string> expected;

        expected = {"<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(374)));

        expected = {"<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(377)));

        expected = {"<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(402)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(75)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(81)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(107)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(116)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(264)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(302)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(325)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(331)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(337)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(413)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(207)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(228)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(241)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(293)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(295)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(379)));

        expected = {"<unknown>", "<unknown>", "The Pavement"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(283)));

        expected = {"<unknown>", "Casons", "Casons"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(286)));

        expected = {"<unknown>", "Commonwealth Avenue", "Commonwealth Avenue"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(224)));

        expected = {"<unknown>", "Commonwealth Avenue", "Commonwealth Avenue"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(239)));

        expected = {"<unknown>", "Commonwealth Avenue", "Commonwealth Avenue"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(370)));

        expected = {"<unknown>", "Commonwealth Avenue", "Commonwealth Avenue", "Unnamed Road"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(371)));

        expected = {"<unknown>", "Evergreen Drive", "Evergreen Drive"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(313)));

        expected = {"<unknown>", "Longwood Avenue", "Longwood Avenue"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(0)));

        expected = {"Colt Sheds", "Colt Sheds"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(112)));

        expected = {"Cow Path", "Cow Path"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(180)));

        expected = {"Deadwood", "Deadwood"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(96)));

    } //intersection_street_names

    TEST_FIXTURE(MapFixture, adjacent_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {1, 117, 131};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(0)));

        expected = {50, 175, 313};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(316)));

        expected = {55, 57};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(56)));

        expected = {69, 71};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(68)));

        expected = {73, 372};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(72)));

        expected = {88, 91, 305};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(89)));

        expected = {108};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(109)));

        expected = {120, 128};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(129)));

        expected = {121, 123};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(122)));

        expected = {125, 127};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(126)));

        expected = {150, 152, 272};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(151)));

        expected = {155, 330};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(328)));

        expected = {157, 324, 392};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(158)));

        expected = {159, 163, 239};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(162)));

        expected = {163, 225};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(407)));

        expected = {170, 172, 185};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(173)));

        expected = {188, 190};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(189)));

        expected = {193, 397, 417};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(396)));

        expected = {204, 220, 386};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(205)));

        expected = {234, 287, 395};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(288)));

        expected = {251};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(416)));

        expected = {260, 338, 339};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(261)));

        expected = {267};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(266)));

        expected = {300};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(301)));

        expected = {304, 352};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(351)));

        expected = {328, 331, 341};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(330)));

        expected = {348, 350};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(349)));

        expected = {350, 354, 394};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(355)));

        expected = {360};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(359)));

        expected = {369, 370};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(342)));

    } //adjacent_intersections
    

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
} //intersection_queries_public_saint_helena