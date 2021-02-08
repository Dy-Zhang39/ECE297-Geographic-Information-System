
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

SUITE(street_queries_public_toronto_canada) {

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

    TEST_FIXTURE(MapFixture, all_street_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {2, 3, 10, 747, 748, 751, 752, 753, 754, 757, 758, 762, 763, 766, 767, 775, 776, 777, 779, 780, 1924, 1943, 7130, 9987, 9992, 9993, 10105, 10261, 10262, 10269, 10270, 10314, 10315, 13309, 13316, 13321, 14890, 14891, 14892, 14924, 14925, 15018, 15019, 15020, 16935, 16936, 19252, 20209, 24442, 24463, 24464, 29254, 29549, 29550, 29551, 29560, 39266, 39267, 58608, 58609, 58941, 58942, 58955, 73821, 73835, 73836, 73837, 73850, 74698, 74699, 89294, 89516, 89520, 95111, 110416};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(2)));

        expected = {1783, 5704, 52919, 97534, 97535, 97536, 97537};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20584)));

        expected = {11483, 11486, 75620, 95988};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20493)));

        expected = {16049, 93850};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20298)));

        expected = {16144, 64389};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(14345)));

        expected = {16963, 16972, 16973, 16974, 16975, 72491, 108589};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4069)));

        expected = {18199, 18200, 18201};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4419)));

        expected = {21475, 21476};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5251)));

        expected = {23657, 23659, 23660, 23661, 23662, 23669, 23673, 23675, 23679, 23686, 30814, 53050};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5810)));

        expected = {25354, 26368, 26371, 26372, 26373};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6335)));

        expected = {25452, 25455};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6118)));

        expected = {46278, 73371, 73397};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17005)));

        expected = {48816, 49449, 50368, 51650, 58156, 58175, 58193, 58220};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(9763)));

        expected = {53976, 53981};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(11254)));

        expected = {56567, 56667};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12187)));

        expected = {57751, 57753};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12410)));

        expected = {59378, 59379};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12960)));

        expected = {59722, 59868};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(13071)));

        expected = {68220, 68350, 68632, 68774};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15369)));

        expected = {68360, 68830};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15513)));

        expected = {69617, 116982, 129107, 129108, 136849};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(21840)));

        expected = {69987, 69988, 69989};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15907)));

        expected = {70533, 70540, 70564, 70567, 70586, 70598, 70599};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16047)));

        expected = {71615, 71616, 71617, 71618, 71619, 71620};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16409)));

        expected = {78599, 78600, 78610, 78611, 78617, 78618, 78619, 78620, 78621, 78623, 78627, 78636};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17667)));

        expected = {80386, 80387, 80393, 80394};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17962)));

        expected = {81235, 81236, 81392};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(18311)));

        expected = {88273, 91691, 91702};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20146)));

        expected = {91287, 91314};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20088)));

        expected = {133906, 136983, 136985, 136986, 136987, 136988};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(22438)));

    } //all_street_intersections

    TEST_FIXTURE(MapFixture, intersection_ids_from_street_ids) {
        std::vector<IntersectionIdx> expected;

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(4307, 6554))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(6382, 4558))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7759, 13936))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(8546, 10335))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(9037, 17744))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(10310, 17423))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(11344, 3857))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(13040, 11300))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(13524, 18480))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15713, 18564))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16785, 509))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19179, 17379))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19739, 409))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19942, 22145))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(21504, 20443))));

        expected = {16049};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(20298, 3842))));

        expected = {17590};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(4243, 4242))));

        expected = {25452, 25455};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(6118, 980))));

        expected = {30850, 30854, 36602, 36604};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7409, 0))));

        expected = {33460, 33461, 33464, 33466, 33469};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7209, 0))));

        expected = {43810};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(8366, 8361))));

        expected = {57751};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(12410, 12406))));

        expected = {64389};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(14345, 14342))));

        expected = {67930};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15277, 15276))));

        expected = {70599};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16047, 16077))));

        expected = {71620};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16409, 4507))));

        expected = {72383};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16652, 16649))));

        expected = {78729};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(17700, 20337))));

        expected = {81079};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(18188, 18179))));

        expected = {81392};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(18311, 18217))));

    } //intersection_ids_from_street_ids


} //street_queries_public_toronto_canada