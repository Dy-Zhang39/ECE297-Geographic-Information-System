
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

SUITE(distance_time_queries_perf_public_toronto_canada) {

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


    struct MapFixture : BaseMapFixture {
        MapFixture()
            : BaseMapFixture()
            , rng(3)
            , rand_intersection(0, getNumIntersections()-1)
            , rand_street(1, getNumStreets()-1) // Start from 1 to avoid getting id 0 (<unknown>)
            , rand_segment(0, getNumStreetSegments()-1)
            , rand_poi(0, getNumPointsOfInterest()-1)
            , rand_feature(0, getNumFeatures()-1)
            , rand_node(0, 7780698)
            , rand_way(0, 975994)
            , rand_relation(0, 7144)
            , rand_lat(43.479999542, 43.919982910)
            , rand_lon(-79.789985657, -79.000000000)
        { }

        std::minstd_rand rng;
        std::uniform_int_distribution<int> rand_intersection;
        std::uniform_int_distribution<int> rand_street;
        std::uniform_int_distribution<int> rand_segment;
        std::uniform_int_distribution<int> rand_poi;
        std::uniform_int_distribution<int> rand_feature;
        std::uniform_int_distribution<int> rand_node;
        std::uniform_int_distribution<int> rand_way;
        std::uniform_int_distribution<int> rand_relation;
        std::uniform_real_distribution<double> rand_lat;
        std::uniform_real_distribution<double> rand_lon;
    };

    TEST_FIXTURE(MapFixture, distance_between_two_points_perf) {
        //Verify Functionality
        double expected;

        expected = 2356.64505946482040599;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.79279708862304688, -79.40152740478515625), LatLon(43.78813934326171875, -79.37289428710937500))), 0.001000000);

        expected = 2427.80270119967008213;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.71529388427734375, -79.55553436279296875), LatLon(43.73324203491210938, -79.53834533691406250))), 0.001000000);

        expected = 2949.60709968117043900;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.59196853637695312, -79.37844085693359375), LatLon(43.59408187866210938, -79.41493988037109375))), 0.001000000);

        expected = 3548.76272402025188057;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.82280731201171875, -79.18561553955078125), LatLon(43.83702087402343750, -79.14601898193359375))), 0.001000000);

        expected = 7082.94972707004035328;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.64250946044921875, -79.35969543457031250), LatLon(43.60358810424804688, -79.29006958007812500))), 0.001000000);

        expected = 9273.46528542371379444;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.54529571533203125, -79.10992431640625000), LatLon(43.56031417846679688, -79.22308349609375000))), 0.001000000);

        expected = 9502.16744354860202293;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.71036148071289062, -79.60935211181640625), LatLon(43.65697860717773438, -79.70158386230468750))), 0.001000000);

        expected = 10259.27792697115364717;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.59600830078125000, -79.11286163330078125), LatLon(43.52816772460937500, -79.19910430908203125))), 0.001000000);

        expected = 10811.38925209360422741;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.87495803833007812, -79.56263732910156250), LatLon(43.85248565673828125, -79.43147277832031250))), 0.001000000);

        expected = 14256.83954341826029122;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.61819839477539062, -79.22166442871093750), LatLon(43.72824859619140625, -79.31252288818359375))), 0.001000000);

        expected = 14466.80948114287093631;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.77970123291015625, -79.60786437988281250), LatLon(43.71545028686523438, -79.45131683349609375))), 0.001000000);

        expected = 16151.57112973360381147;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.73253250122070312, -79.09116363525390625), LatLon(43.58732604980468750, -79.09315490722656250))), 0.001000000);

        expected = 17244.38714028189860983;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.56804275512695312, -79.77233886718750000), LatLon(43.69831848144531250, -79.65620422363281250))), 0.001000000);

        expected = 23088.23867834949487587;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.84590911865234375, -79.17972564697265625), LatLon(43.66650009155273438, -79.32427978515625000))), 0.001000000);

        expected = 26697.57579962744057411;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.77907943725585938, -79.08943939208984375), LatLon(43.88920593261718750, -79.38510131835937500))), 0.001000000);

        expected = 28346.94087889042202733;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.57031250000000000, -79.68901824951171875), LatLon(43.77373886108398438, -79.47676086425781250))), 0.001000000);

        expected = 29222.10386375499729184;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.65841293334960938, -79.78363800048828125), LatLon(43.66227722167968750, -79.42051696777343750))), 0.001000000);

        expected = 30851.74677904916461557;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.77691650390625000, -79.58846282958984375), LatLon(43.56536865234375000, -79.34043121337890625))), 0.001000000);

        expected = 31806.78265816414932488;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.71936416625976562, -79.52052307128906250), LatLon(43.89463806152343750, -79.20742797851562500))), 0.001000000);

        expected = 34409.99452569775894517;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.74295806884765625, -79.03385162353515625), LatLon(43.64626312255859375, -79.44029235839843750))), 0.001000000);

        expected = 36763.18711706031172071;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.89926528930664062, -79.00794982910156250), LatLon(43.57030868530273438, -79.05247497558593750))), 0.001000000);

        expected = 38140.77731261219742009;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.87306594848632812, -79.04129028320312500), LatLon(43.88062286376953125, -79.51689147949218750))), 0.001000000);

        expected = 38384.63474691180454101;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.89250946044921875, -79.44643402099609375), LatLon(43.57640457153320312, -79.25479125976562500))), 0.001000000);

        expected = 40275.83257943762146169;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.83801651000976562, -79.22557830810546875), LatLon(43.91736984252929688, -79.71572113037109375))), 0.001000000);

        expected = 41625.99285252964182291;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.56082916259765625, -79.01220703125000000), LatLon(43.72190475463867188, -79.47900390625000000))), 0.001000000);

        expected = 44870.51818089283915469;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.87591171264648438, -79.01165008544921875), LatLon(43.90194320678710938, -79.57025146484375000))), 0.001000000);

        expected = 45647.26265787136071594;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.64857482910156250, -79.20729827880859375), LatLon(43.71326065063476562, -79.76768493652343750))), 0.001000000);

        expected = 50284.51768783960142173;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.90982818603515625, -79.60358428955078125), LatLon(43.48199844360351562, -79.40149688720703125))), 0.001000000);

        expected = 51319.73420976037596120;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.65852737426757812, -79.65677642822265625), LatLon(43.54740142822265625, -79.03836059570312500))), 0.001000000);

        expected = 60998.75850568081659731;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(43.74923324584960938, -79.02179718017578125), LatLon(43.81575775146484375, -79.77580261230468750))), 0.001000000);

        //Generate random inputs
        std::vector<LatLon> latlons_1;
        std::vector<LatLon> latlons_2;
        for(size_t i = 0; i < 30000000; i++) {
            latlons_1.push_back(LatLon(rand_lat(rng), rand_lon(rng)));
            latlons_2.push_back(LatLon(rand_lat(rng), rand_lon(rng)));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(877);
            double result;
            for(size_t i = 0; i < 30000000; i++) {
                result += findDistanceBetweenTwoPoints(std::make_pair(latlons_1[i], latlons_2[i]));
            }
        }
    } //distance_between_two_points_perf

    TEST_FIXTURE(MapFixture, street_segment_length_perf) {
        //Verify Functionality
        double expected;

        expected = 4.31280873966657730;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(193330), 0.001000000);

        expected = 9.51348675579425773;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(93960), 0.001000000);

        expected = 11.78072151740800244;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(119434), 0.001000000);

        expected = 15.71349032996043960;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(39152), 0.001000000);

        expected = 17.36607119585176306;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(60964), 0.001000000);

        expected = 17.99349788432688513;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(157923), 0.001000000);

        expected = 18.86090475545442047;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(149295), 0.001000000);

        expected = 20.38139778527202139;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(147907), 0.001000000);

        expected = 20.62569373896849712;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(117289), 0.001000000);

        expected = 22.05094202135635939;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(193882), 0.001000000);

        expected = 22.46923560183619628;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(163656), 0.001000000);

        expected = 23.32245156621486615;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(195352), 0.001000000);

        expected = 25.94955128609908357;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(170026), 0.001000000);

        expected = 29.10016435138784630;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(172867), 0.001000000);

        expected = 32.79992338435404520;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(154431), 0.001000000);

        expected = 42.36715806538496309;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(124726), 0.001000000);

        expected = 43.35451533048303219;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(138059), 0.001000000);

        expected = 43.89667046573810438;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(210188), 0.001000000);

        expected = 45.01237278631888472;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(215943), 0.001000000);

        expected = 46.61367907230290797;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(197221), 0.001000000);

        expected = 47.51506246278246692;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(176229), 0.001000000);

        expected = 47.94623084470583763;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(198105), 0.001000000);

        expected = 62.30570654538148290;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(108303), 0.001000000);

        expected = 122.27786259614822484;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(153092), 0.001000000);

        expected = 188.98141998662543983;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(50528), 0.001000000);

        expected = 223.77606192597608015;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(42525), 0.001000000);

        expected = 238.92309859297819230;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(55911), 0.001000000);

        expected = 242.52749386327775483;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(14), 0.001000000);

        expected = 261.85450993295478384;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(125798), 0.001000000);

        expected = 388.37572511922161311;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(58880), 0.001000000);

        //Generate random inputs
        std::vector<StreetSegmentIdx> segment_ids;
        for(size_t i = 0; i < 1600000; i++) {
            segment_ids.push_back(rand_segment(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(865);
            double result;
            for(size_t i = 0; i < 1600000; i++) {
                result += findStreetSegmentLength(segment_ids[i]);
            }
        }
    } //street_segment_length_perf

    TEST_FIXTURE(MapFixture, street_length_perf) {
        //Verify Functionality
        double expected;

        expected = 93.07334090613004207;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(12410), 0.001000000);

        expected = 125.28853353475008703;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(12960), 0.001000000);

        expected = 164.49682745984065946;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(15513), 0.001000000);

        expected = 200.61563603314465354;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20298), 0.001000000);

        expected = 206.64428235332405848;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(22438), 0.001000000);

        expected = 214.94020172395775603;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(11254), 0.001000000);

        expected = 222.99840361452817206;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(18311), 0.001000000);

        expected = 230.01405153585452013;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20493), 0.001000000);

        expected = 256.54110131468996769;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(4069), 0.001000000);

        expected = 259.14929220348693661;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(4419), 0.001000000);

        expected = 275.55296396373290690;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(12187), 0.001000000);

        expected = 319.14146299211256519;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(14345), 0.001000000);

        expected = 330.89974831586062010;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(15369), 0.001000000);

        expected = 335.14712314012967909;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(13071), 0.001000000);

        expected = 357.89878663703865413;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(6118), 0.001000000);

        expected = 366.08307611424879724;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(17005), 0.001000000);

        expected = 381.49679509792264298;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(16409), 0.001000000);

        expected = 389.29983212986877561;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20088), 0.001000000);

        expected = 409.16064472401035346;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(5251), 0.001000000);

        expected = 417.19186507197258607;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20146), 0.001000000);

        expected = 548.33384680555775503;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(6335), 0.001000000);

        expected = 633.38944802786579658;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(15907), 0.001000000);

        expected = 728.43434942414228317;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(21840), 0.001000000);

        expected = 750.66151510553083881;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(17962), 0.001000000);

        expected = 807.60529228814266389;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(5810), 0.001000000);

        expected = 900.87256582271174921;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(16047), 0.001000000);

        expected = 925.27005821933244079;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(9763), 0.001000000);

        expected = 1098.72154526266263019;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(17667), 0.001000000);

        expected = 1209.66563725881701430;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20584), 0.001000000);

        expected = 39128.53764451635652222;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(2), 0.001000000);

        //Generate random inputs
        std::vector<StreetIdx> street_ids;
        for(size_t i = 0; i < 400000; i++) {
            street_ids.push_back(rand_street(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(1177);
            double result;
            for(size_t i = 0; i < 400000; i++) {
                result += findStreetLength(street_ids[i]);
            }
        }
    } //street_length_perf

    TEST_FIXTURE(MapFixture, street_segment_travel_time_perf) {
        //Verify Functionality
        double expected;

        expected = 0.08625617479333154;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(193330), 0.001000000);

        expected = 0.23561443034816004;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(119434), 0.001000000);

        expected = 0.23783716889485645;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(93960), 0.001000000);

        expected = 0.31426980659920878;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(39152), 0.001000000);

        expected = 0.34732142391703524;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(60964), 0.001000000);

        expected = 0.35986995768653768;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(157923), 0.001000000);

        expected = 0.37721809510908844;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(149295), 0.001000000);

        expected = 0.40762795570544041;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(147907), 0.001000000);

        expected = 0.41251387477936996;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(117289), 0.001000000);

        expected = 0.44101884042712719;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(193882), 0.001000000);

        expected = 0.44938471203672392;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(163656), 0.001000000);

        expected = 0.46644903132429733;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(195352), 0.001000000);

        expected = 0.51899102572198164;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(170026), 0.001000000);

        expected = 0.58200328702775694;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(172867), 0.001000000);

        expected = 0.65599846768708092;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(154431), 0.001000000);

        expected = 0.72257525550805057;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(138059), 0.001000000);

        expected = 0.84734316130769927;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(124726), 0.001000000);

        expected = 0.87793340931476205;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(210188), 0.001000000);

        expected = 0.90024745572637765;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(215943), 0.001000000);

        expected = 0.93227358144605821;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(197221), 0.001000000);

        expected = 0.95030124925564929;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(176229), 0.001000000);

        expected = 0.95892461689411679;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(198105), 0.001000000);

        expected = 1.24611413090762957;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(108303), 0.001000000);

        expected = 2.03796437660247021;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(153092), 0.001000000);

        expected = 4.47552123851952199;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(42525), 0.001000000);

        expected = 4.72453549966563635;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(50528), 0.001000000);

        expected = 4.85054987726555531;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(14), 0.001000000);

        expected = 5.97307746482445445;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(55911), 0.001000000);

        expected = 6.54636274832386977;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(125798), 0.001000000);

        expected = 9.70939312798054033;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(58880), 0.001000000);

        //Generate random inputs
        std::vector<StreetSegmentIdx> segment_ids;
        for(size_t i = 0; i < 250000000; i++) {
            segment_ids.push_back(rand_segment(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(3742);
            double result;
            for(size_t i = 0; i < 250000000; i++) {
                result += findStreetSegmentTravelTime(segment_ids[i]);
            }
        }
    } //street_segment_travel_time_perf

    TEST_FIXTURE(MapFixture, feature_area_perf) {
        //Verify Functionality
        double expected;

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(16421), 0.001000000);

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(16488), 0.001000000);

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(49749), 0.001000000);

        expected = 124.96187222517821169;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(559983), 0.001000000);

        expected = 188.61399476763219241;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(72481), 0.001000000);

        expected = 202.74939210209373641;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(50657), 0.001000000);

        expected = 275.56810564325911628;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(336), 0.001000000);

        expected = 294.40177539319398647;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(54844), 0.001000000);

        expected = 371.24918989714763029;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(55128), 0.001000000);

        expected = 382.13592573895226678;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(60866), 0.001000000);

        expected = 422.13887475054474407;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(36988), 0.001000000);

        expected = 528.63680691683305213;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(17892), 0.001000000);

        expected = 533.97308849429236943;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(49638), 0.001000000);

        expected = 598.82006918463548573;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(277301), 0.001000000);

        expected = 685.45179380526599289;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(57640), 0.001000000);

        expected = 744.65137890983135094;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(20226), 0.001000000);

        expected = 766.38201605606332123;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(23988), 0.001000000);

        expected = 873.15499104129969510;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(53278), 0.001000000);

        expected = 979.97981836125575228;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(18672), 0.001000000);

        expected = 1238.52908596976999434;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(11947), 0.001000000);

        expected = 1911.79558111590267799;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(10148), 0.001000000);

        expected = 2221.56368318081740654;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(71818), 0.001000000);

        expected = 2558.81803154585259108;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(31880), 0.001000000);

        expected = 4498.56878442368906690;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(6045), 0.001000000);

        expected = 8075.19854863216824015;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(59984), 0.001000000);

        expected = 10396.36380346155237930;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(72353), 0.001000000);

        expected = 11531.07775197444425430;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(68692), 0.001000000);

        expected = 17635.11881403398001567;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(7747), 0.001000000);

        expected = 101534.86114964804437477;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(373), 0.001000000);

        expected = 122695.76964092248817906;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(10616), 0.001000000);

        //Generate random inputs
        std::vector<FeatureIdx> feature_ids;
        for(size_t i = 0; i < 100000; i++) {
            feature_ids.push_back(rand_feature(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(100);
            double result;
            for(size_t i = 0; i < 100000; i++) {
                result += findFeatureArea(feature_ids[i]);
            }
        }
    } //feature_area_perf

} //distance_time_queries_perf_public_toronto_canada

