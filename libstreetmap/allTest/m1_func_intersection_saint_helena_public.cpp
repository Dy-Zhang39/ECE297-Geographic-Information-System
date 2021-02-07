
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

    TEST_FIXTURE(MapFixture, intersection_street_segments) {
        std::vector<StreetSegmentIdx> expected;

        expected = {0, 402, 403};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(0)));

        expected = {47, 48};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(75)));

        expected = {51, 255};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(81)));

        expected = {62, 63};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(96)));

        expected = {90, 91};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(112)));

        expected = {122, 123};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(180)));

        expected = {142, 176, 457};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(207)));

        expected = {155, 325, 326};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(224)));

        expected = {159, 343, 344};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(228)));

        expected = {170, 327, 328};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(239)));

        expected = {171, 172, 179};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(241)));

        expected = {192, 463};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(264)));

        expected = {209, 210, 211};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(283)));

        expected = {212, 359, 360};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(286)));

        expected = {215, 220, 221};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(293)));

        expected = {216, 223, 224};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(295)));

        expected = {222, 223};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(302)));

        expected = {234, 235, 239};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(313)));

        expected = {250, 251};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(331)));

        expected = {270, 271};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(337)));

        expected = {322, 323, 385};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(370)));

        expected = {326, 327, 422, 459};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(371)));

        expected = {335, 336, 338};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(379)));

        expected = {337};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(374)));

        expected = {339};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(377)));

        expected = {364, 453};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(325)));

        expected = {384};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(402)));

        expected = {397, 398};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(107)));

        expected = {398, 399};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(116)));

        expected = {439, 440};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(413)));

    } //intersection_street_segments

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

} //intersection_queries_public_saint_helena

