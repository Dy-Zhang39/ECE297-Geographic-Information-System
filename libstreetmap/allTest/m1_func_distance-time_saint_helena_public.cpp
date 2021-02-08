
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

SUITE(distance_time_queries_public_saint_helena) {

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

    TEST_FIXTURE(MapFixture, distance_between_two_points) {
        double expected;

        expected = 518.56996111314856535;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.94796562194824219, -5.71199750900268555), LatLon(-15.94898891448974609, -5.70726680755615234))), 0.001000000);

        expected = 533.67273782537722582;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.96500301361083984, -5.73743915557861328), LatLon(-15.96105766296386719, -5.73459911346435547))), 0.001000000);

        expected = 646.81417546958221010;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.99211502075195312, -5.70818281173706055), LatLon(-15.99165058135986328, -5.71421289443969727))), 0.001000000);

        expected = 781.04487220694420557;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.94136810302734375, -5.67632865905761719), LatLon(-15.93824386596679688, -5.66978836059570312))), 0.001000000);

        expected = 1555.20541039505997105;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.98100376129150391, -5.70508718490600586), LatLon(-15.98956108093261719, -5.69358444213867188))), 0.001000000);

        expected = 2032.03825974654228048;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-16.00237464904785156, -5.66382551193237305), LatLon(-15.99907398223876953, -5.68251848220825195))), 0.001000000);

        expected = 2087.76344887593995736;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.96608734130859375, -5.74632930755615234), LatLon(-15.97782325744628906, -5.76156663894653320))), 0.001000000);

        expected = 2252.14965549958787960;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.99122619628906250, -5.66430997848510742), LatLon(-16.00613975524902344, -5.67855787277221680))), 0.001000000);

        expected = 2381.66534915951342555;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.92990398406982422, -5.73861217498779297), LatLon(-15.93484401702880859, -5.71694469451904297))), 0.001000000);

        expected = 3133.25003460576863290;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.98634910583496094, -5.68228387832641602), LatLon(-15.96215629577636719, -5.69729471206665039))), 0.001000000);

        expected = 3180.64645869247715382;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.95084381103515625, -5.74608373641967773), LatLon(-15.96496868133544922, -5.72022294998168945))), 0.001000000);

        expected = 3550.67153415110124115;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.96121406555175781, -5.66072654724121094), LatLon(-15.99313545227050781, -5.66105508804321289))), 0.001000000);

        expected = 3788.70268015585224930;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.99737358093261719, -5.77325439453125000), LatLon(-15.96873569488525391, -5.75406932830810547))), 0.001000000);

        expected = 5076.00033060668738472;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.93628978729248047, -5.67535591125488281), LatLon(-15.97572994232177734, -5.69923639297485352))), 0.001000000);

        expected = 5876.92045638761555892;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.95098114013671875, -5.66044139862060547), LatLon(-15.92677211761474609, -5.70928430557250977))), 0.001000000);

        expected = 6228.83780151572682371;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.99687480926513672, -5.75948953628540039), LatLon(-15.95215511322021484, -5.72442626953125000))), 0.001000000);

        expected = 6415.10359920979317394;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.97750759124755859, -5.77512168884277344), LatLon(-15.97665786743164062, -5.71513462066650391))), 0.001000000);

        expected = 6778.89342495606342709;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.95145702362060547, -5.74287843704223633), LatLon(-15.99796295166015625, -5.70190429687500000))), 0.001000000);

        expected = 6997.49198010715008422;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.96410846710205078, -5.73165512084960938), LatLon(-15.92557716369628906, -5.67993307113647461))), 0.001000000);

        expected = 7559.06104934466657141;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.95892238616943359, -5.65125846862792969), LatLon(-15.98017883300781250, -5.71840095520019531))), 0.001000000);

        expected = 8081.72585648078984377;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.92456054687500000, -5.64697933197021484), LatLon(-15.99687576293945312, -5.65433502197265625))), 0.001000000);

        expected = 8405.38987461763281317;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.93031978607177734, -5.65248680114746094), LatLon(-15.92865848541259766, -5.73105573654174805))), 0.001000000);

        expected = 8438.23250511763762916;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.92604541778564453, -5.71941566467285156), LatLon(-15.99553680419921875, -5.68775653839111328))), 0.001000000);

        expected = 8874.92889832758010016;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.93802452087402344, -5.68293094635009766), LatLon(-15.92058086395263672, -5.76390171051025391))), 0.001000000);

        expected = 9137.86513847491914930;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.99896049499511719, -5.64768314361572266), LatLon(-15.96355056762695312, -5.72479629516601562))), 0.001000000);

        expected = 9890.39729273709417612;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.92969417572021484, -5.64759063720703125), LatLon(-15.92397212982177734, -5.73987007141113281))), 0.001000000);

        expected = 10024.82079483745292237;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.97967052459716797, -5.67991113662719727), LatLon(-15.96545028686523438, -5.77248620986938477))), 0.001000000);

        expected = 11053.46618153271447227;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.92223834991455078, -5.74537706375122070), LatLon(-16.01629066467285156, -5.71199226379394531))), 0.001000000);

        expected = 11256.16647222144638363;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.97748279571533203, -5.75416374206542969), LatLon(-16.00191307067871094, -5.65200376510620117))), 0.001000000);

        expected = 13419.90131970157017349;
        ECE297_CHECK_RELATIVE_ERROR(expected, findDistanceBetweenTwoPoints(std::make_pair(LatLon(-15.95754241943359375, -5.64926624298095703), LatLon(-15.94291782379150391, -5.77382612228393555))), 0.001000000);

    } //distance_between_two_points

    TEST_FIXTURE(MapFixture, street_segment_length) {
        double expected;

        expected = 6.01499246631190587;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(266), 0.001000000);

        expected = 10.06006162205377308;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(83), 0.001000000);

        expected = 15.11445034174861135;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(349), 0.001000000);

        expected = 28.88258988369799596;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(413), 0.001000000);

        expected = 32.09710091414628153;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(268), 0.001000000);

        expected = 37.39519121424380188;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(0), 0.001000000);

        expected = 40.21689070648031361;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(315), 0.001000000);

        expected = 55.65102123739954720;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(448), 0.001000000);

        expected = 57.06124089315957804;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(119), 0.001000000);

        expected = 97.37768341259490512;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(125), 0.001000000);

        expected = 98.32375752653975098;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(337), 0.001000000);

        expected = 119.85954426732284617;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(461), 0.001000000);

        expected = 126.27513995861261265;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(318), 0.001000000);

        expected = 137.03950545999404653;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(231), 0.001000000);

        expected = 207.20894630133020087;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(107), 0.001000000);

        expected = 250.08658964238307476;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(130), 0.001000000);

        expected = 254.32475817565372722;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(423), 0.001000000);

        expected = 297.66580288127738640;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(363), 0.001000000);

        expected = 319.45995523208523537;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(200), 0.001000000);

        expected = 335.37363800636410360;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(417), 0.001000000);

        expected = 351.31106607031216527;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(255), 0.001000000);

        expected = 424.71310416024385859;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(412), 0.001000000);

        expected = 580.07266661545077113;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(90), 0.001000000);

        expected = 591.87589918236017184;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(376), 0.001000000);

        expected = 717.27502527137244215;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(329), 0.001000000);

        expected = 764.42021396600011940;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(326), 0.001000000);

        expected = 874.73561340888795712;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(250), 0.001000000);

        expected = 964.85577290594585520;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(369), 0.001000000);

        expected = 1252.14243964988850166;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(294), 0.001000000);

        expected = 1555.50688775104481465;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentLength(421), 0.001000000);

    } //street_segment_length

    TEST_FIXTURE(MapFixture, street_length) {
        double expected;

        expected = 61.13750850343078724;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(21), 0.001000000);

        expected = 64.97587657871555678;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(4), 0.001000000);

        expected = 135.17676071363320034;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(33), 0.001000000);

        expected = 239.30466179233232538;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(26), 0.001000000);

        expected = 251.04497891117836161;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(23), 0.001000000);

        expected = 277.47184120762909743;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(16), 0.001000000);

        expected = 278.59413522871352598;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(30), 0.001000000);

        expected = 282.80808804293843650;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(3), 0.001000000);

        expected = 345.47899672071702071;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(15), 0.001000000);

        expected = 366.39575404465892916;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(9), 0.001000000);

        expected = 418.77844976559811130;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(13), 0.001000000);

        expected = 435.82671214958440942;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(19), 0.001000000);

        expected = 520.58640395330996853;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(14), 0.001000000);

        expected = 526.28413553087739274;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(11), 0.001000000);

        expected = 534.83931202641861091;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(10), 0.001000000);

        expected = 715.23651530164988799;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(29), 0.001000000);

        expected = 723.45904361895338752;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(25), 0.001000000);

        expected = 902.65559039357253823;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(1), 0.001000000);

        expected = 1034.83948774600776233;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(7), 0.001000000);

        expected = 1423.13552690968026582;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(24), 0.001000000);

        expected = 1645.88789406114369740;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(32), 0.001000000);

        expected = 1898.18388406669259894;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(20), 0.001000000);

        expected = 2020.18703257245169880;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(31), 0.001000000);

        expected = 2074.08820386742718256;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(6), 0.001000000);

        expected = 2442.93575972656617523;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(12), 0.001000000);

        expected = 2917.67870206709812919;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(17), 0.001000000);

        expected = 3052.75596453587513679;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(27), 0.001000000);

        expected = 5549.68743135912518483;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(28), 0.001000000);

        expected = 5814.18226190625682648;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(18), 0.001000000);

        expected = 8088.32711482079685084;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetLength(8), 0.001000000);

    } //street_length

    TEST_FIXTURE(MapFixture, street_segment_travel_time) {
        double expected;

        expected = 0.12029984932623812;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(266), 0.001000000);

        expected = 0.25150154055134433;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(83), 0.001000000);

        expected = 0.37786125854371527;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(349), 0.001000000);

        expected = 0.57765179767395991;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(413), 0.001000000);

        expected = 0.64194201828292563;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(268), 0.001000000);

        expected = 0.93487978035609509;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(0), 0.001000000);

        expected = 1.39127553093498868;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(448), 0.001000000);

        expected = 1.42653102232898954;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(119), 0.001000000);

        expected = 2.43444208531487272;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(125), 0.001000000);

        expected = 2.45809393816349386;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(337), 0.001000000);

        expected = 2.99648860668307115;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(461), 0.001000000);

        expected = 2.99883947471881118;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(315), 0.001000000);

        expected = 3.42598763649985116;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(231), 0.001000000);

        expected = 5.18022365753325520;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(107), 0.001000000);

        expected = 5.95331605762554794;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(363), 0.001000000);

        expected = 6.25216474105957687;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(130), 0.001000000);

        expected = 6.35811895439134300;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(423), 0.001000000);

        expected = 6.70747276012728211;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(417), 0.001000000);

        expected = 7.02622132140624345;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(255), 0.001000000);

        expected = 7.98649888080213088;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(200), 0.001000000);

        expected = 8.49426208320487675;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(412), 0.001000000);

        expected = 9.41591624144409955;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(318), 0.001000000);

        expected = 14.50181666538626857;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(90), 0.001000000);

        expected = 14.79689747955900359;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(376), 0.001000000);

        expected = 17.93187563178431176;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(329), 0.001000000);

        expected = 19.29711545811891682;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(369), 0.001000000);

        expected = 21.86839033522219822;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(250), 0.001000000);

        expected = 25.04284879299776989;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(294), 0.001000000);

        expected = 31.11013775502089729;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(421), 0.001000000);

        expected = 57.00026711773771382;
        ECE297_CHECK_RELATIVE_ERROR(expected, findStreetSegmentTravelTime(326), 0.001000000);

    } //street_segment_travel_time

    TEST_FIXTURE(MapFixture, feature_area) {
        double expected;

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(475), 0.001000000);

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(651), 0.001000000);

        expected = 0.00000000000000000;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1953), 0.001000000);

        expected = 26.45990824007036579;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1548), 0.001000000);

        expected = 43.03588788344544014;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1550), 0.001000000);

        expected = 53.86393759098163514;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1549), 0.001000000);

        expected = 99.19344586727360991;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1551), 0.001000000);

        expected = 121.52193479670057741;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1547), 0.001000000);

        expected = 129.91130438695017801;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1553), 0.001000000);

        expected = 131.82321832665618899;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1193), 0.001000000);

        expected = 141.96979027095352421;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(2354), 0.001000000);

        expected = 186.92710305223971545;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(2140), 0.001000000);

        expected = 192.81685194159868502;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1552), 0.001000000);

        expected = 195.27680842449041165;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1176), 0.001000000);

        expected = 207.21115869195008941;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1543), 0.001000000);

        expected = 212.52341629367657561;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(141), 0.001000000);

        expected = 226.62922398060780438;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1556), 0.001000000);

        expected = 247.44370927806915006;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1542), 0.001000000);

        expected = 273.03720445783648074;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(139), 0.001000000);

        expected = 277.82762748990410273;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1541), 0.001000000);

        expected = 348.73351122132083901;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1848), 0.001000000);

        expected = 349.67657555903565481;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(421), 0.001000000);

        expected = 382.54982854666735648;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(425), 0.001000000);

        expected = 506.12739842822099945;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(346), 0.001000000);

        expected = 566.89727794660120708;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(347), 0.001000000);

        expected = 714.75287182057638802;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1481), 0.001000000);

        expected = 1477.00825927083451461;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(344), 0.001000000);

        expected = 3477.18255188786315557;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(1546), 0.001000000);

        expected = 4536.42077741445609718;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(342), 0.001000000);

        expected = 510902584.08360993862152100;
        ECE297_CHECK_RELATIVE_ERROR(expected, findFeatureArea(2523), 0.001000000);

    } //feature_area

} //distance_time_queries_public_saint_helena

