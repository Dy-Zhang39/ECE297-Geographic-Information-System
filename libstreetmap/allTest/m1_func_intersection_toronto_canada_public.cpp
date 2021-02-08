
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

SUITE(intersection_queries_public_toronto_canada) {

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

    TEST_FIXTURE(MapFixture, intersection_street_segments) {
        std::vector<StreetSegmentIdx> expected;

        expected = {5, 7811, 102107};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(10)));

        expected = {24821, 24822, 36814};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(27101)));

        expected = {27049, 27050, 27056};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(29435)));

        expected = {32703, 75704, 75705};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(34975)));

        expected = {36718, 36719, 36721};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(38701)));

        expected = {39235, 39236, 39323};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(40756)));

        expected = {40679, 124834, 212566, 212567};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(42199)));

        expected = {68750, 68767, 128254};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(65038)));

        expected = {82678, 136261, 136262};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(74966)));

        expected = {91207, 91431};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(81186)));

        expected = {93174, 180669, 180673, 180682};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(82672)));

        expected = {98040, 98041, 106569};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(86334)));

        expected = {99080, 99154, 99158};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(87076)));

        expected = {111298, 111318, 111322, 141351, 159502};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(95563)));

        expected = {121876};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(102380)));

        expected = {123437, 123438, 134241, 134243};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(103341)));

        expected = {127437, 127460, 127461};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(105969)));

        expected = {128935, 128936, 128949};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(106896)));

        expected = {132911, 132912, 132922};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(109313)));

        expected = {139808, 139818, 139819};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(113281)));

        expected = {147526, 181228, 181237};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(117691)));

        expected = {151332, 218770};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(119658)));

        expected = {155838, 155843};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(121985)));

        expected = {178999, 179000, 179023};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(133821)));

        expected = {179810, 218811};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(134204)));

        expected = {182658, 182659, 195593};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(135221)));

        expected = {186606, 186609, 186610};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(136515)));

        expected = {187909, 187910, 187918, 187919};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(137127)));

        expected = {202703, 202704, 211356};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(145491)));

        expected = {213128, 213131, 213132};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetSegmentsOfIntersection(149474)));

    } //intersection_street_segments

    TEST_FIXTURE(MapFixture, intersection_street_names) {
        std::vector<std::string> expected;

        expected = {"<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(102380)));

        expected = {"<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(121985)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(38701)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(40756)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(87076)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(105969)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(106896)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(109313)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(113281)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(133821)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(136515)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(145491)));

        expected = {"<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(149474)));

        expected = {"<unknown>", "<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(137127)));

        expected = {"<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(95563)));

        expected = {"<unknown>", "Bovaird Drive East", "Bovaird Drive East"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(117691)));

        expected = {"<unknown>", "Denison Street", "Denison Street"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(135221)));

        expected = {"<unknown>", "Highway 401 Express", "Highway 401 Express"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(10)));

        expected = {"<unknown>", "Kitimat Road", "Kitimat Road"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(27101)));

        expected = {"<unknown>", "North Service Road", "North Service Road"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(86334)));

        expected = {"<unknown>", "South Millway", "South Millway"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(34975)));

        expected = {"Athabasca Common", "Wheat Boom Drive"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(134204)));

        expected = {"Claremont Street", "Queen Street West", "Queen Street West"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(74966)));

        expected = {"Constitution Boulevard", "Constitution Boulevard", "Vera Cruz Drive"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(29435)));

        expected = {"Don Mills Road", "Don Mills Road", "Eglinton Avenue East", "Eglinton Avenue East"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(42199)));

        expected = {"Don Mills Road", "Don Mills Road", "York Mills Road", "York Mills Road"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(103341)));

        expected = {"Hurontario Street", "Hurontario Street", "Queensway", "Queensway East"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(82672)));

        expected = {"Isabella Street", "South Unionville Avenue", "South Unionville Avenue"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(65038)));

        expected = {"Moraine Drive", "Moraine Drive"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(81186)));

        expected = {"The East Mall", "The East Mall"};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetNamesOfIntersection(119658)));

    } //intersection_street_names

    TEST_FIXTURE(MapFixture, adjacent_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(4338)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(112860)));

        expected = {1236, 76603, 76604};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(76602)));

        expected = {3709};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(3708)));

        expected = {4056, 4058, 143782};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(4057)));

        expected = {7020, 7021, 17415};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(7019)));

        expected = {7989};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(88834)));

        expected = {13853, 74339, 136641};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(74338)));

        expected = {14666, 106912};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(107457)));

        expected = {27998, 89642, 89644};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(89643)));

        expected = {34494, 89805};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(34495)));

        expected = {35507, 35510, 35786, 35790};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(35785)));

        expected = {38811, 38813, 38814};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(38812)));

        expected = {41550, 41553, 73705};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(41552)));

        expected = {42354, 42363, 117651};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(42361)));

        expected = {42860, 42862, 42997};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(42861)));

        expected = {47152, 47154};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(47153)));

        expected = {47902, 51507, 52533, 104934};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(47903)));

        expected = {50295, 51378, 51640};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(51497)));

        expected = {62341, 62354};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(62342)));

        expected = {66516, 66518, 66543};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(66517)));

        expected = {76845, 76848, 76850};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(76851)));

        expected = {93448, 93450};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(109721)));

        expected = {97050, 124625, 130211};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(124624)));

        expected = {108588};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(108587)));

        expected = {123751, 123770, 123772};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(123773)));

        expected = {125236, 125238, 125240};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(125237)));

        expected = {126916, 126932, 126936};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(126931)));

        expected = {131029, 131030, 131031};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(131032)));

        expected = {143484, 143520, 143525};
        ECE297_CHECK_EQUAL(expected, sorted(findAdjacentIntersections(143527)));

    } //adjacent_intersections

} //intersection_queries_public_toronto_canada

