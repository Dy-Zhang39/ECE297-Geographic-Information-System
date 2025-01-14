/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m2.h"
#include "m3.h"
#include "drawMap.h"

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "m1.h"
#include "global.h"
#include "dataHandler.h"
#include "callBack.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <chrono>

#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

double textDisplayRatio = 0.01;
double streetToWorldRatio = 0.5;
double streetToWorldRatio1 = 0.035;
double EARTH_CIRCUMFERENCE = 2* M_PI * kEarthRadiusInMeters;
bool showSubways = false;
std::string instructionString;

//make sure the interested region has some distance with the windows margin
double GAP = 1000;

//maximum intersections that can display on the screen
int MAX_INTERSECTIONS_DISPLAY = 7;

bool nightMode;                     //Determines whether to show night mode
bool choosingStartingPoint = false; //the mode that ask user to choose a starting point
bool choosingDestination = false;    //the mode that ask user to choose a destination

//the vector that store the travel street information
std::vector<streetInfo> travelPathInfo;

IntersectionIdx fromPath = -1;
IntersectionIdx toPath = -1;

std::vector <StreetSegmentIdx> pathRoute;
std::vector <StreetSegmentIdx> exploredPath;

//global variable storing color for background, text, feature and street
ezgl::color backgroundColor;
ezgl::color textColor;
ezgl::color lakeColor;
ezgl::color islandColor;
ezgl::color parkColor;
ezgl::color buildingColor;
ezgl::color beachColor;
ezgl::color golfColor;
ezgl::color streetColor;
ezgl::color highwayColor;

ezgl::renderer *gg;

std::vector<IntersectionIdx> previousHighlight;
LatLon positionOfClicked;
std::vector <ezgl::point2d> featureTextPoints;      //coordinates for display feature text
std::string selectedPOI = "all";                    //selected POI type to display

//a vector that store the item that is stored in the PossibleLocation drop Down Box
std::vector<std::pair<std::string, int>> possibleLocations;

extern std::vector<City*> cities;
extern int currentCityIdx;
extern std::vector<std::string> cityNames;
extern std::string mapPathPrefix;
extern std::vector<StreetIdx> mostSimilarFirstName;
extern std::vector<StreetIdx> mostSimilarSecondName;
extern bool checkingFirstName;

void drawMap(){

    // Initialize coordinates for feature bounding boxes.

    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    
    initializeCurrentWorldRatio();

    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    
    ezgl::rectangle initial_world({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    application.add_canvas("MainCanvas", drawMainCanvas, initial_world);
    //initial set up, act on mouse press, act on mouse move, act on key press
    application.run(initialSetUp, actOnMouseClick, nullptr, nullptr);
}

void drawMainCanvas (ezgl::renderer *g){
    
    //determine the color used based on night mode selection   
    if (!nightMode) {
        backgroundColor= ezgl::color(255, 247, 224, 255);
        textColor= ezgl::BLACK;
        lakeColor = ezgl::color(102, 204, 255, 255);
        islandColor = ezgl::color(221, 251, 109, 255);
        parkColor = ezgl::color(128, 234, 85, 255);
        buildingColor = ezgl::GREY_75;
        beachColor = ezgl::color(236, 226, 149, 255);
        golfColor = ezgl::color(110, 184, 66, 255);
        streetColor = ezgl::color(210, 223, 227, 255);
        highwayColor = ezgl::color(244, 208, 63, 255);
        
    } else {
        backgroundColor = ezgl::color(0,0,0);
        textColor= ezgl::WHITE;
        lakeColor = ezgl::color(10, 4, 73);
        islandColor = ezgl::color(2, 91, 4, 255);
        parkColor = ezgl::color(19, 100, 20, 255);
        buildingColor = ezgl::color(98, 98, 98, 255);
        beachColor = ezgl::color(109, 113, 42, 255);
        golfColor = ezgl::color(3, 46, 1, 255);
        streetColor = ezgl::color(142, 142, 142, 255);
        highwayColor = ezgl::color(191, 131, 0, 255);
    }

    g->format_font("Noto Sans CJK SC", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);

    ezgl::rectangle world = g->get_visible_world();
    double xMin, xMax, yMin, yMax;

    yMax = yFromLat(cities[currentCityIdx] -> maxLat);
    yMin = yFromLat(cities[currentCityIdx] -> minLat);
    xMax = xFromLon(cities[currentCityIdx] -> maxLon);
    xMin = xFromLon(cities[currentCityIdx] -> minLon);
    ezgl::point2d start(xMin, yMin);
    ezgl::point2d end(xMax, yMax);
    
    g->draw_rectangle(start, end);
    g->set_color(backgroundColor);
    g->fill_rectangle(start, end);

    drawFeature(g, world);
    drawStreet(g, world);
    displayPOI(g);
    displayHighlightedIntersection(g);
    displayStreetName(g, world);

    if (showSubways) loadSubway(g);

    if (pathRoute.size() > 0) {
        for (int i = 0; i < exploredPath.size(); i ++) {
            drawSegment(g, g->get_visible_world(), ezgl::BLUE, exploredPath[i]);
        }
        displayTravelInfo(pathRoute);
    }
    
    drawRoute(g, g->get_visible_world(), pathRoute);
}




//connect the signal to call back function
void initialSetUp(ezgl::application *application, bool /*new_window*/){
    
    application->update_message("Map is loaded successfully");
    
    GObject *search = application->get_object("SearchButton");
    g_signal_connect(search, "clicked", G_CALLBACK(searchButtonIsClicked), application);
    
    GObject *searching = application ->get_object("SwitchBetweenSearchAndFindPath");
    g_signal_connect(searching, "state-set", G_CALLBACK(changeSearchingMode), application);
    
    GObject *selecting = application ->get_object("UsingPinLocationInstead");
    g_signal_connect(selecting, "state-set", G_CALLBACK(changeSelectingMode), application);
    
    GObject *textEntry = application->get_object("TextInput");
    GObject *secondTextEntry = application->get_object("TextInput2");
    g_signal_connect(textEntry, "activate", G_CALLBACK(textEntryPressedEnter), application);
    g_signal_connect(secondTextEntry, "activate", G_CALLBACK(textEntryPressedEnter), application);
    
    g_signal_connect(textEntry, "changed", G_CALLBACK(textEntryChanges), application);
    g_signal_connect(secondTextEntry, "changed", G_CALLBACK(textEntryChanges), application);
    
    GtkComboBoxText *mapBar = (GtkComboBoxText*) application->get_object("MapBar");
    importNameToTheBar(mapBar);
    g_signal_connect (mapBar, "changed", G_CALLBACK(changeMap), application);

    GtkComboBoxText *possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation");
    GtkComboBoxText *secondPossibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation2");
    g_signal_connect (possibleLocationBar, "changed", G_CALLBACK(possibleLocationIsChosen), application);
    g_signal_connect (secondPossibleLocationBar, "changed", G_CALLBACK(possibleLocationIsChosen), application);
    
    //POI related buttons
    GObject *allPOI = application->get_object("allPOIBtn");
    g_signal_connect(allPOI, "toggled", G_CALLBACK(toggleAllPOI), application);
    
    GObject *educationPOI = application->get_object("educationPOIBtn");
    g_signal_connect(educationPOI, "toggled", G_CALLBACK(toggleEducationPOI), application);

    GObject *foodPOI = application->get_object("foodPOIBtn");
    g_signal_connect(foodPOI, "toggled", G_CALLBACK(toggleFoodPOI), application);
    
    GObject *medicalPOI = application->get_object("medicalPOIBtn");
    g_signal_connect(medicalPOI, "toggled", G_CALLBACK(toggleMedicalPOI), application);
    
    GObject *transportPOI = application->get_object("transportPOIBtn");
    g_signal_connect(transportPOI, "toggled", G_CALLBACK(toggleTransportPOI), application);
    
    GObject *recreationPOI = application->get_object("recreationPOIBtn");
    g_signal_connect(recreationPOI, "toggled", G_CALLBACK(toggleRecreationPOI), application);
    
    GObject *financePOI = application->get_object("financePOIBtn");
    g_signal_connect(financePOI, "toggled", G_CALLBACK(toggleFinancePOI), application);
    
    GObject *govPOI = application->get_object("govPOIBtn");
    g_signal_connect(govPOI, "toggled", G_CALLBACK(toggleGovPOI), application);

    GObject *otherPOI = application->get_object("otherPOIBtn");
    g_signal_connect(otherPOI, "toggled", G_CALLBACK(toggleOtherPOI), application);
    
    GObject *hidePOI = application->get_object("hidePOIBtn");
    g_signal_connect(hidePOI, "toggled", G_CALLBACK(toggleHidePOI), application);

    //check box for showing subways
    GObject *showSubwayBox = application->get_object("showSubwayBox");
    g_signal_connect(showSubwayBox, "toggled", G_CALLBACK(toggleSubway), application);
    
    //check box for using night mode
    GObject *snightModeBox = application->get_object("nightModeBox");
    g_signal_connect(snightModeBox, "toggled", G_CALLBACK(toggleNightMode), application);
    
    //button to set from intersection
    GObject *setFromBtn = application->get_object("setFromBtn");
    g_signal_connect(setFromBtn, "clicked", G_CALLBACK(setFromBtnClicked), application);
    
    //button to set to intersection
    GObject *setToBtn = application->get_object("setToBtn");
    g_signal_connect(setToBtn, "clicked", G_CALLBACK(setToBtnClicked), application);
    
    //button to clear route
    GObject *clearRouteBtn = application->get_object("clearRouteBtn");
    g_signal_connect(clearRouteBtn, "clicked", G_CALLBACK(clearRouteBtnClicked), application);
    
    GObject *helpBtn = application->get_object("helpBtn");
    g_signal_connect(helpBtn, "clicked", G_CALLBACK(helpBtnClicked), application);
}



gboolean changeSearchingMode (GtkWidget * sw, gboolean state, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    GtkButton* search = (GtkButton* ) application->get_object("SearchButton");
    GtkButton* setFrom = (GtkButton* ) application->get_object("setFromBtn");
    GtkButton* setTo = (GtkButton* ) application->get_object("setToBtn");
    GtkButton* clearRoute = (GtkButton* ) application->get_object("clearRouteBtn");
    GtkEntry* textEntry = (GtkEntry *) application ->get_object("TextInput2");
    GtkComboBox* dropDown = (GtkComboBox *) application ->get_object("PossibleLocation2");
    GtkSwitch* selectingMode = (GtkSwitch *) application ->get_object("UsingPinLocationInstead");
    GtkLabel* labelSelectingMode = (GtkLabel *) application ->get_object("LabelPinPoint");
    
    gtk_switch_set_state ((GtkSwitch *) sw, state);
    
    if (state == TRUE){
        gtk_widget_set_sensitive((GtkWidget *) textEntry, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) dropDown, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) setFrom, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) setTo, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) clearRoute, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) selectingMode, TRUE);
        gtk_widget_set_sensitive((GtkWidget *) labelSelectingMode, TRUE);
        
        gtk_button_set_label (search, "Find Path");
        application->update_message("Change to Path Finding Mode");
        
    }else{
        clearRouteBtnClicked(sw, data);
        gtk_widget_set_sensitive((GtkWidget *) textEntry, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) dropDown, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) setFrom, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) setTo, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) clearRoute, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) selectingMode, FALSE);
        gtk_widget_set_sensitive((GtkWidget *) labelSelectingMode, FALSE);
        
        gtk_button_set_label (search, "Search");
        application->update_message("Change to Searching Mode");
        
    }
    return TRUE;
}

gboolean changeSelectingMode(GtkWidget * sw, gboolean state, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    gtk_switch_set_state ((GtkSwitch *) sw, state);
    
    if (state == FALSE){
        application ->update_message("Change to search bar mode, you can set starting point and destination using search bar");
    }else{
        application ->update_message("Change to pin point mode, you can set starting point and destination using mouse clicking");
    }
    
    return TRUE;
    
}






//triggered night mode check box changed
gboolean toggleNightMode(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    nightMode = !nightMode;
    application->refresh_drawing();
    
    return true;
}

//triggered showSubway check box changed
gboolean toggleSubway(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    showSubways = !showSubways;
    application->refresh_drawing();
    
    return true;
}


//triggered when all POI button is changed
gboolean toggleAllPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("all") != 0) {
        
        selectedPOI = "all";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when education POI button is changed
gboolean toggleEducationPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("education") != 0) {
        selectedPOI = "education";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Food POI button is changed
gboolean toggleFoodPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("food") != 0) {
        selectedPOI = "food";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Medical POI button is changed
gboolean toggleMedicalPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("medical") != 0) {
        selectedPOI = "medical";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Transport POI button is changed
gboolean toggleTransportPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("transport") != 0) {
        selectedPOI = "transport";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Finance POI button is changed
gboolean toggleFinancePOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("finance") != 0) {
        selectedPOI = "finance";
        application->refresh_drawing();
    }
    return true;
}

//triggered when Recreation POI button is changed
gboolean toggleRecreationPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("recreation") != 0) {
        selectedPOI = "recreation";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Government POI button is changed
gboolean toggleGovPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("gov") != 0) {
        selectedPOI = "gov";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when Other POI button is changed
gboolean toggleOtherPOI(GtkWidget *, gpointer data) {
    
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("other") != 0) {
        selectedPOI = "other";
        application->refresh_drawing();
    }
    
    return true;
}

//triggered when the Hide All POIs button is changed
gboolean toggleHidePOI(GtkWidget *, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    if (selectedPOI.compare("hide") != 0) {
        selectedPOI = "hide";
        application->refresh_drawing();
    }
    
    return true;
}

//Search when user press the enter in the text field
gboolean textEntryPressedEnter(GtkWidget * widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    GtkSwitch* sw = (GtkSwitch *) application ->get_object("SwitchBetweenSearchAndFindPath");
    GtkSwitch* selecting = (GtkSwitch *) application ->get_object("UsingPinLocationInstead");
    bool mode = gtk_switch_get_state(sw);
    bool isPinPoint = gtk_switch_get_state(selecting);
    
    if(mode == false){
        singleSearchMode((GtkEntry *)widget, data);
    }else{
        
        std::string widgetName = gtk_widget_get_name (widget);
        
        if (isPinPoint){
            application->update_message("You can not using searching bar in pin point mode, please swap to searching bar mode");
            return TRUE;
        }
        
        if (widgetName == "TextInput2"){
            setFromBtnClicked(widget, data);
        }else if (widgetName == "TextInput"){
            setToBtnClicked(widget, data);
        }else{
            std::cout << "The widget name is not text entry in textEntryPressedEnter()" << std::endl;
            exit (EXIT_FAILURE);
        }
        
    }
    
    return true;
    
}

//zoom in to the intersection when user choose a location from the bar
gboolean possibleLocationIsChosen(GtkWidget* widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);
    
    std::string widgetName = gtk_widget_get_name (widget);
    GtkComboBoxText *possibleLocationBar = NULL;
    GtkEntry *textEntry = NULL;
    
    if (widgetName == "PossibleLocation"){
        possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation");
        textEntry = (GtkEntry *) application ->get_object("TextInput");
    }else if(widgetName == "PossibleLocation2"){
        possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation2");
        textEntry = (GtkEntry *) application ->get_object("TextInput2");
    }
    
    if (possibleLocationBar == NULL || textEntry == NULL){
        std::cout << "The widget pointer is still NULL in possibleLocationIsChosen()" << std::endl;
        exit (EXIT_FAILURE);
    }
    
    if (gtk_combo_box_text_get_active_text(possibleLocationBar) == NULL){
        return TRUE;
    }
    
    clearHighlightIntersection();
    
    std::string locationName = gtk_combo_box_text_get_active_text(possibleLocationBar);
    
    IntersectionIdx locationIdx = -1;
    
    for(auto it = possibleLocations.begin(); it != possibleLocations.end();){
        
        if (locationName == it -> first){
            locationIdx = it -> second;
            it = possibleLocations.end();
        }
        
        if (it != possibleLocations.end()){
            it++;
        }
    }
    
    if (locationIdx == -1){
        application->update_message("Oops, something went wrong");
        return TRUE;
    }
    
    auto world = getZoomLevelToIntersections(locationIdx);
    cities[currentCityIdx] -> intersection -> intersectionInfo[locationIdx].isHighlight = true;
    previousHighlight.push_back(locationIdx);
    
    
    GtkSwitch* sw = (GtkSwitch *) application ->get_object("SwitchBetweenSearchAndFindPath");
    bool mode = gtk_switch_get_state(sw);
    
    if (mode == true){
        if (widgetName == "PossibleLocation"){
            toPath = locationIdx;        
        }else if (widgetName == "PossibleLocation2"){
            fromPath = locationIdx;
        }else{
            std::cout << "The widget name is not the name for the text entry in possibleLocationIsChosen()." << std::endl;
            exit (EXIT_FAILURE);
        }
        
        displayStartAndDestination(application);
    }else{
        application->update_message(locationName);
    }
    
    
    std::string nameByCommas = separateNamesByCommas(locationName);
    canvas->get_camera().set_world(world);
    gtk_entry_set_text(textEntry, nameByCommas.c_str());    
    application->refresh_drawing();
    
    
    return TRUE;
}


//put the possible location in the drop down bar
gboolean textEntryChanges(GtkWidget * widget, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    std::string widgetName = gtk_widget_get_name (widget);
    GtkComboBoxText *possibleLocationBar = NULL;
    GtkEntry* textEntry = NULL;
    
    
    if (widgetName == "TextInput"){
        possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation");
        textEntry = (GtkEntry *) application ->get_object("TextInput");
    }else if(widgetName == "TextInput2"){
        possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation2");
        textEntry = (GtkEntry *) application ->get_object("TextInput2");
    }
    
    if (possibleLocationBar == NULL || textEntry == NULL){
        std::cout << "The widget pointer is still NULL in textEntryChanges()" << std::endl;
        exit (EXIT_FAILURE);
    }
    
    possibleLocations.clear();
    gtk_combo_box_text_remove_all(possibleLocationBar);
    std::string text = gtk_entry_get_text(textEntry);
    
    mostSimilarFirstName.clear();            
    mostSimilarSecondName.clear();           
    checkingFirstName = true;
     
    auto twoNames = getStreetNames(text);
    std::string firstStreet = twoNames.first;
    std::string secondStreet = twoNames.second;
    
    auto firstPartialResult = findStreetIdsFromPartialStreetName(firstStreet);
    auto secondPartialResult = findStreetIdsFromPartialStreetName(secondStreet);
    
    //put all the similar name to the partial name for finding common intersection
    for (int idx = 0; idx < mostSimilarFirstName.size(); idx++){
        firstPartialResult.push_back(mostSimilarFirstName[idx]);
    }
    
    for (int idx = 0; idx < mostSimilarSecondName.size(); idx++){
        secondPartialResult.push_back(mostSimilarSecondName[idx]);
    }
    
    auto possibleIntersections = findAllPossibleIntersections(firstPartialResult, secondPartialResult);
    
    for(auto it = possibleIntersections.begin(); it != possibleIntersections.end(); it++){
        
        auto name = cities[currentCityIdx] -> intersection -> intersectionInfo[*it].name;
        possibleLocations.push_back(std::make_pair(name, *it));
        
        gtk_combo_box_text_append_text (possibleLocationBar, name.c_str());
    }
    

    return TRUE;
}

//change the map when the user switch to a different map
gboolean changeMap(GtkWidget * widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    GtkComboBoxText * mapBar = (GtkComboBoxText*) application->get_object("MapBar");
    
    std::string map = gtk_combo_box_text_get_active_text(mapBar);
    
    std::string newMapPath = convertNameToPath(map);
    
    clearRouteBtnClicked(widget, data);
    //does not change the map if the user is choose the current map
    if (newMapPath == cities[currentCityIdx] -> mapPath){
        return true;
    }
    //the vector that store the travel street information
    travelPathInfo.clear();
    
    closeDataBase();
    bool loadSucessfully = loadMap(newMapPath);
    if(!loadSucessfully){
        std::cerr << "Failed to load map '" << newMapPath << "'\n";
        application->quit();
        return true;
    }
    
    GtkComboBoxText *possibleLocationBar = (GtkComboBoxText*) application->get_object("PossibleLocation");    
    possibleLocations.clear();
    gtk_combo_box_text_remove_all(possibleLocationBar);
    
    double maxLat = cities[currentCityIdx] -> maxLat;
    double minLat = cities[currentCityIdx] -> minLat;
    double maxLon = cities[currentCityIdx] -> maxLon;
    double minLon = cities[currentCityIdx] -> minLon;
    
    //new world coordinate
    ezgl::rectangle newWorld({xFromLon(minLon), yFromLat(minLat)},
                                {xFromLon(maxLon), yFromLat(maxLat)});
    
    initializeCurrentWorldRatio();
    
    application ->change_canvas_world_coordinates("MainCanvas", newWorld);
    std::string output = "Map " + map + " has loaded Successfully";
    application->update_message(output);
    application->refresh_drawing();
    
    return true;
    
}



//search the intersections of two streets
gboolean searchButtonIsClicked(GtkWidget * widget, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    
    GtkSwitch* sw = (GtkSwitch *) application->get_object("SwitchBetweenSearchAndFindPath");
    
    bool mode =  gtk_switch_get_state (sw);
    
    if (mode == false){
        GtkEntry* textEntry = (GtkEntry *) application ->get_object("TextInput");
        singleSearchMode(textEntry, data);
    }else{
        searchPathBtnClicked(widget, data);
    }
    return true;
}

void singleSearchMode(GtkEntry * textEntry, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    
    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);
    

    std::string text = gtk_entry_get_text(textEntry);
    clearHighlightIntersection();
    choosingStartingPoint = false;
    choosingDestination = false;
    
    mostSimilarFirstName.clear();            //make sure number of element is not more than 2
    mostSimilarSecondName.clear();            //make sure number of element is not more than 2
    checkingFirstName = true;
    
    auto twoNames = getStreetNames(text);
    std::string firstStreet = twoNames.first;
    std::string secondStreet = twoNames.second;
    
    std::string output;
    
    //make sure user enter two streets name
    if (firstStreet.size() == 0 || secondStreet.size() == 0){
        application ->update_message("You need to enter two streets separated by the commas");
        return;
    }
    
    std::vector<StreetIdx> partialResultFirst = findStreetIdsFromPartialStreetName(firstStreet);
    checkingFirstName = false;
    std::vector<StreetIdx> partialResultSecond = findStreetIdsFromPartialStreetName(secondStreet);
    
    
    //put all the similar name to the partial name for finding common intersection
    for (int idx = 0; idx < mostSimilarFirstName.size(); idx++){
        partialResultFirst.push_back(mostSimilarFirstName[idx]);
    }
    
    for (int idx = 0; idx < mostSimilarSecondName.size(); idx++){
        partialResultSecond.push_back(mostSimilarSecondName[idx]);
    }
    
    ezgl::point2d sum(0, 0), center(0,0), largest(-1 * EARTH_CIRCUMFERENCE, -1 * EARTH_CIRCUMFERENCE), smallest(EARTH_CIRCUMFERENCE, EARTH_CIRCUMFERENCE);

    std::vector<IntersectionIdx> totalCommonIntersections;
    if ((partialResultFirst.size() > 0 && partialResultSecond.size() > 0)){
        
        
       
        for (auto firstStreetIdx = partialResultFirst.begin(); firstStreetIdx != partialResultFirst.end();) {

            for (auto secondStreetIdx = partialResultSecond.begin(); secondStreetIdx != partialResultSecond.end(); ) {
                
                auto  commonIntersection = findIntersectionsOfTwoStreets(std::make_pair(*firstStreetIdx, *secondStreetIdx));
                if (commonIntersection.size() > 0) {
                    
                    addVectorToVector(totalCommonIntersections, commonIntersection);
                }
                
                if (totalCommonIntersections.size() > MAX_INTERSECTIONS_DISPLAY){
                    
                    firstStreetIdx = partialResultFirst.end();
                    secondStreetIdx = partialResultSecond.end();
                }
                
                if (secondStreetIdx != partialResultSecond.end()){
                    secondStreetIdx++;
                }
            }
            
            if (firstStreetIdx != partialResultFirst.end()){
                firstStreetIdx++;
            }
        }
        
        
        
        if (totalCommonIntersections.size() > 0){
            
            auto world = getZoomLevelToIntersections(totalCommonIntersections);
            for (int idx = 0; idx < totalCommonIntersections.size(); idx++){
                //highlight these cities[currentCityIdx] -> intersection -> intersectionInfo
                cities[currentCityIdx] -> intersection -> intersectionInfo[totalCommonIntersections[idx]].isHighlight = true;
                previousHighlight.push_back(totalCommonIntersections[idx]);
            }
            canvas->get_camera().set_world(world);
            
        }else{
            output = "No Intersection found.";
        }
        
    }else{
        output = "Street can not be found.";
    }
    
    
    application->update_message(output);
    application->refresh_drawing();
}



ezgl::rectangle getZoomLevelToIntersections(std::vector<IntersectionIdx> commonIntersection){
    
    if (commonIntersection.size() == 0){
        return (ezgl::rectangle({0, 0}, {0, 0}));
    }
    
    ezgl::point2d sum(0, 0), center(0,0), largest(-1 * EARTH_CIRCUMFERENCE, -1 * EARTH_CIRCUMFERENCE), smallest(EARTH_CIRCUMFERENCE, EARTH_CIRCUMFERENCE);
    
    for (int idx = 0; idx < commonIntersection.size(); idx++) {
        //get the position in cartesian coordiante
        auto positionInLL = getIntersectionPosition(commonIntersection[idx]);
        double positionInX = xFromLon(positionInLL.longitude());
        double positionInY = yFromLat(positionInLL.latitude());

        sum.x += positionInX;
        sum.y += positionInY;

        if (positionInX > largest.x) {

            largest.x = positionInX;
        }

        if (positionInX < smallest.x) {

            smallest.x = positionInX;
        }

        if (positionInY > largest.y) {

            largest.y = positionInY;
        }

        if (positionInY < smallest.y) {

            smallest.y = positionInY;
        }

        
        previousHighlight.push_back(commonIntersection[idx]);

    }

    

    //set up the world zoom level
    double left, bottom, top, right;
    left = smallest.x - GAP;
    bottom = smallest.y - GAP;
    right = largest.x + GAP;
    top = largest.y + GAP;

    double width = right - left;
    double height = top - bottom;
    
    center.x = (left + right) / 2;
    center.y = (top + bottom) / 2;
    //making sure the width and height of the screen is in the world ratio
    if (width * cities[currentCityIdx]->worldRatio > height) {

        bottom = center.y - (width * cities[currentCityIdx]->worldRatio) / 2;
        top = center.y + (width * cities[currentCityIdx]->worldRatio) / 2;

    } else {

        left = center.x - (height / cities[currentCityIdx]->worldRatio) / 2;
        right = center.x + (height / cities[currentCityIdx]->worldRatio) / 2;

    }
    return (ezgl::rectangle ({left, bottom}, {right, top}));
}

ezgl::rectangle getZoomLevelToIntersections(IntersectionIdx id){
    
    auto positionInLL = cities[currentCityIdx] -> intersection -> intersectionInfo[id].position;
    double positionInX = xFromLon(positionInLL.longitude());
    double positionInY = yFromLat(positionInLL.latitude());
    
    //set up the world zoom level
    double left, bottom, top, right;
    left = positionInX - GAP;
    bottom = positionInY - GAP;
    right = positionInX + GAP;
    top = positionInY + GAP;

    double width = right - left;
    double height = top - bottom;

    //making sure the width and height of the screen is in the world ratio
    if (width * cities[currentCityIdx]->worldRatio > height) {

        bottom = positionInY - (width * cities[currentCityIdx]->worldRatio) / 2;
        top = positionInY+ (width * cities[currentCityIdx]->worldRatio) / 2;

    } else {

        left = positionInX - (height / cities[currentCityIdx]->worldRatio) / 2;
        right = positionInX + (height / cities[currentCityIdx]->worldRatio) / 2;

    }
    

    return (ezgl::rectangle ({left, bottom}, {right, top}));
}

void actOnMouseClick(ezgl::application* , GdkEventButton* event, double x, double y){
    
    //record the position of left mouse clicked
    if (event ->button == 1){
           
        positionOfClicked = LatLon(latFromY(y), lonFromX(x));
        
    }
    
}

//mouse click to highlight the closest intersection
IntersectionIdx clickToHighlightClosestIntersection(LatLon pos){
    IntersectionIdx id;
    if (choosingStartingPoint || choosingDestination){
        id = findClosestIntersection(pos, previousHighlight);
    }else{
        id = findClosestIntersection(pos);
    }
    
    if (previousHighlight.size() == 1 && previousHighlight[0] == id){
        
        cities[currentCityIdx] -> intersection -> intersectionInfo[id].isHighlight = false;
        clearHighlightIntersection();
        
    }else{
        
        clearHighlightIntersection();
        cities[currentCityIdx] -> intersection -> intersectionInfo[id].isHighlight = true;
        previousHighlight.push_back(id);        
    }
    
    return id;
}

void drawStreet(ezgl::renderer *g, ezgl::rectangle world){
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    double diagLength = sqrt(world.height()*world.height() + world.width()*world.width());
    double aveToWorldRatio1 = 0.13;
    double highwaySpeed = 22.23;
    double visibleStreetLenRatio = 1.2;
    double streetWidth = 0.5;
    double highwayWidth = 1.3;

    for(int streetSegmentsID=0; streetSegmentsID<getNumStreetSegments(); streetSegmentsID++ ){
        //get street segment streetID, loop through each ID and connect its point.
        StreetIdx streetId =getStreetSegmentInfo(streetSegmentsID).streetID;
        //get street name of this street segment
        std::string streetName = getStreetName(streetId);

        //if the street name is unknown, draw when user zoom in
        if(!streetName.compare("<unknown>")){
            //draw as user zooms in
            if(findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * visibleStreetLenRatio * streetToWorldRatio1){
                g->set_color(streetColor);
                g->set_line_width(streetWidth * streetSize(world));
                g->draw_line({x1,y1}, {x2, y2});
            }
            
        }else if (findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * streetToWorldRatio1){
            //draw street according to the length of the street compare to screen  length


            for(int pointsID=1; pointsID < cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID].size(); pointsID++){
                x1 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID - 1].longitude());
                y1 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID- 1].latitude());

                x2 = xFromLon(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].longitude());
                y2 = yFromLat(cities[currentCityIdx]->streetSegment->streetSegPoint[streetSegmentsID][pointsID].latitude());
                if(world.contains(x1, y1) || world.contains(x2, y2)){
                    double speedLimit = getStreetSegmentInfo(streetSegmentsID).speedLimit;
                    
                    if(speedLimit>highwaySpeed){// draw highway
                        
                       g->set_color(highwayColor);
                       g->set_line_width(highwayWidth * streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }else if(!isAve(streetName)){ //draw main road and street
                        
                       g->set_color(streetColor);
                       g->set_line_width(streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }else if(isAve(streetName)&&findStreetLength(getStreetSegmentInfo(streetSegmentsID).streetID) > diagLength * aveToWorldRatio1){ 
                        
                        //draw avenue
                       g->set_color(streetColor);
                       g->set_line_width(streetWidth * streetSize(world));
                       g->draw_line({x1,y1}, {x2, y2});
                    }

                }
            }
        }        
    }
    return;
}

void displayStreetName(ezgl::renderer *g, ezgl::rectangle world) {
    //collect the street name that has been display in a vector
    std::vector<ezgl::point2d> displayedNames;

    double fontSize = 10; //street name size
    double boundStreetNameSize = 200; //the street name text is bounded within the size
    double diagLength = sqrt(world.height() * world.height() + world.width() * world.width());

    displayedNames.clear();

    for (int streetID = 0; streetID < getNumStreets(); streetID++) {
        //get segments position that within the screen
        std::vector<ezgl::point2d> inViewSegment;
        inViewSegment.clear();

        //get street name
        std::string streetName = getStreetName(streetID);
        //store the one way segments and segments that with the screen view
        storeStreetSeg(streetName, inViewSegment, streetID, diagLength, world);

        if (inViewSegment.size() > 2) {
            //find the middle segment of the street on the screen
            ezgl::point2d midPoint = inViewSegment[inViewSegment.size() / 2];
            ezgl::point2d midNextPoint = inViewSegment[inViewSegment.size() / 2 + 1];

            //find the degree to rotate
            double degree = atan2(midNextPoint.y - midPoint.y, midNextPoint.x - midPoint.x) / kDegreeToRadian;
            bool overlap = false;

            //estimate the text width and height
            double widthToPixelRatio = world.width() / g->get_visible_screen().width();
            double heightToPixelRatio = world.height() / g->get_visible_screen().height();

            //check if the street name overlap with each other
            for (int displayedNamesNum = 0; displayedNamesNum < displayedNames.size(); displayedNamesNum++) {
                if (abs(midPoint.x - displayedNames[displayedNamesNum].x) < boundStreetNameSize * widthToPixelRatio &&
                        abs(midPoint.y - displayedNames[displayedNamesNum].y) < boundStreetNameSize * heightToPixelRatio) {
                    overlap = true;
                }
            }

            //make sure text is readable for user by adjusting rotation
            if (degree > 90) {
                degree = degree - 180;
            } else if (degree < -90) {
                degree = degree + 180;
            }

            if (!overlap) { // if not overlap then display
                g->set_font_size(fontSize);
                g->set_color(textColor);
                g->set_text_rotation(degree);
                g->draw_text(midPoint, streetName);
                //save the place where a name has been displayed
                displayedNames.push_back(midPoint);
            }
            //draw one way symbol
            drawOneWayStreet(g, diagLength);

        }
    }
    return;
}

//store the one way street segment in oneWayStreetSegmentInfo and in view segment in inVewSegment for later use
void storeStreetSeg(std::string streetName, std::vector<ezgl::point2d> &inViewSegment, int streetID, int diagLength, ezgl::rectangle world) {
    //get the start and end position of the street segment
    double xFrom = 0, yFrom = 0, xTo = 0, yTo = 0;
    for (int segmentIndex = 0; segmentIndex < cities[currentCityIdx]->street->streetSegments[streetID].size(); segmentIndex++) {
        //check if the street name is unknown, unknown street name is not displayed, since it is useless
        if (streetName.compare("<unknown>") != 0 && findStreetLength(streetID) > diagLength * streetToWorldRatio) {
            //get the x,y position of the point on the map
            xFrom = xFromLon(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).from).longitude());
            yFrom = yFromLat(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).from).latitude());
            xTo = xFromLon(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).to).longitude());
            yTo = yFromLat(getIntersectionPosition(getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).to).latitude());
            //save the points that within the screen
            if (world.contains(xFrom, yFrom) || world.contains(xTo, yTo)) {
                inViewSegment.push_back({xFrom, yFrom});

                //store the segment that is one way                   
                if (getStreetSegmentInfo(cities[currentCityIdx]->street->streetSegments[streetID][segmentIndex]).oneWay) {
                    oneWaySegInfo oneWay;
                    oneWay.fromX = xFrom;
                    oneWay.fromY = yFrom;
                    oneWay.toX = xTo;
                    oneWay.toY = yTo;
                    oneWay.distance = sqrt((xTo - xFrom)*(xTo - xFrom)+(yTo - yFrom)*(yTo - yFrom));

                    cities[currentCityIdx]->streetSegment->oneWaySegment.push_back(oneWay);
                }
            }
        }
    }
}



void drawOneWayStreet(ezgl::renderer *g, double diagLength) {
    //loop through the one way street segment, and draw arrow based on the direction
    for (int oneWaySegId = 0; oneWaySegId < cities[currentCityIdx]->streetSegment->oneWaySegment.size(); oneWaySegId++) {
        if (cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].distance > 0.1 * diagLength) {
            //find the direction to draw arrow based on the degree
            double degree = atan2(cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].toY -
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromY,
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].toX -
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromX) / kDegreeToRadian;
            //find the position to draw arrow
            ezgl::point2d position(cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromX,
                    cities[currentCityIdx]->streetSegment->oneWaySegment[oneWaySegId].fromY);
            //draw the arrow
            g->set_text_rotation(degree);
            drawArrow(g, position, degree);
        }
    }
    return;
}


//draw all features in map
void drawFeature(ezgl:: renderer *g, ezgl::rectangle world){
    featureTextPoints.clear();
    double visibleArea = world.area();
    
    //level of detail ratio to display feature
    double featureToWorldRatio = 0.00005;    
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    
    std::vector <FeatureIdx> features;

    //loop through all features, if the feature area is at the predefined ratio 
    //of the visible area and within the window, draw it
    for (FeatureIdx featureID = 0; featureID < getNumFeatures(); featureID++){
        
        double minX = cities[currentCityIdx]->featurePts[featureID].left;
        double maxX = cities[currentCityIdx]->featurePts[featureID].right;
        double maxY = cities[currentCityIdx]->featurePts[featureID].top;
        double minY = cities[currentCityIdx]->featurePts[featureID].bottom;
        double featureArea = cities[currentCityIdx]->featurePts[featureID].area;
        
        // If the feature is in the visible area, call helper function to display the feature.
        if ((world.contains(minX, minY) || world.contains(minX, maxY)
                || world.contains(maxX, minY) || world.contains(maxX, maxY)
                || (minY <= world.top() && maxY >= world.bottom() 
                && minX <= world.right() && maxX >= world.left())) 
                && featureArea >= visibleArea * featureToWorldRatio){
                    
               
            features.push_back(featureID);
        }
    }
    
    // Sort features by area
    for (int i = 0; i < features.size(); i++) {
        for (int j = i; j < features.size(); j++) {
            if (cities[currentCityIdx]->featurePts[features[i]].area < 
                    cities[currentCityIdx]->featurePts[features[j]].area) {
                int temp = features[i];
                features[i] = features[j];
                features[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < features.size(); i++) {
        drawFeatureByID(g, features[i]);
    }

    //loop through all features, if the feature area is at the predefined ratio 
    //of the visible area and within the window, display its name
    for (int featureID = 0; featureID < getNumFeatures(); featureID++){
        
        double minX = cities[currentCityIdx]->featurePts[featureID].left;
        double maxX = cities[currentCityIdx]->featurePts[featureID].right;
        double maxY = cities[currentCityIdx]->featurePts[featureID].top;
        double minY = cities[currentCityIdx]->featurePts[featureID].bottom;
        double featureArea = findFeatureArea(featureID);
        
        if ((world.contains(minX, minY) || world.contains(minX, maxY)
                || world.contains(maxX, minY) || world.contains(maxX, maxY)
                || (minY <= world.top() && maxY >= world.bottom() 
                && minX <= world.right() && maxX >= world.left())) 
                && featureArea >= visibleArea * featureToWorldRatio){
            
            if (featureArea >= visibleArea * textDisplayRatio){
                displayFeatureNameByID(g, featureID, visibleArea, featureArea,widthToPixelRatio, heightToPixelRatio);
            }
        }
    }
}

//draw a specific feature by a given feature id
void drawFeatureByID(ezgl:: renderer *g, FeatureIdx id){
    std::vector<ezgl::point2d> points;
    
    //choose the color for different types of feature
    FeatureType featureType = getFeatureType(id);   
    switch (featureType) {
        case PARK:
            g->set_color(parkColor);
            break;
        case BEACH:
            g->set_color(beachColor);
            break;
        case LAKE:
            //blue
            g->set_color(lakeColor);
            break;
        case RIVER:
            g->set_color(lakeColor);
            break;
        case ISLAND:
            g->set_color(islandColor);
            break;
        case BUILDING:
            g->set_color(buildingColor);
            break;
        case GREENSPACE:
            g->set_color(parkColor);
            break;
        case GOLFCOURSE:
            g->set_color(golfColor);
            break;
        case STREAM:
            g->set_color(lakeColor);
            break;
        case UNKNOWN:
            g->set_color(ezgl::PURPLE);
            break;
        default:
            break;

    }
    
    //draw the feature outline
    g->set_line_width(0);
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){

        double x1, x2, y1, y2;
        x1 = xFromLon(getFeaturePoint(id, pt - 1).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt - 1).latitude());
        points.push_back({x1, y1});

        x2 = xFromLon(getFeaturePoint(id, pt).longitude());
        y2 = yFromLat(getFeaturePoint(id, pt).latitude());
        g->draw_line({x1,y1}, {x2, y2});
        
    }
    
    //fill the feature with color chosen if it is closed
    if (points.size() > 1 && (getFeaturePoint(id,0) == getFeaturePoint(id, getNumFeaturePoints(id) - 1))){
        g->fill_poly(points);
    }
}

//display the name of a specific feature with a given feature id
void displayFeatureNameByID(ezgl:: renderer *g, FeatureIdx id, double featureArea, double visibleArea, double widthToPixelRatio, double heightToPixelRatio){
    
    //obtain the middle point of the feature
    double xAvg = 0;
    double yAvg = 0;
    double area = 0;
    double x = xFromLon(getFeaturePoint(id, 0).longitude());
    double y = yFromLat(getFeaturePoint(id, 0).latitude());
    std::string featureName = getFeatureName(id); 
    FeatureType featureType = getFeatureType(id);
    
    for (int pt = 1; pt < getNumFeaturePoints(id); pt++){
        double x1, y1;
        x1 = xFromLon(getFeaturePoint(id, pt).longitude());
        y1 = yFromLat(getFeaturePoint(id, pt).latitude());
        
        area += (x * y1 -x1 * y) / 2;
        xAvg += (x * y1 - x1 * y) * (x + x1);
        yAvg += (x * y1 - x1 * y) * (y + y1);
        x = x1;
        y = y1;
    }
    
    xAvg /= 6*area;
    yAvg /= 6*area;
    
    //display the name only with features that is not type UNKNOWN
    bool displayName = false;
    switch (featureType) {
        case PARK:
            g->set_color(textColor);
            displayName = true;
            break;
        case BEACH:
            g->set_color(textColor);
            displayName = true;
            break;
        case LAKE:
            g->set_color(textColor);
            displayName = true;
            break;
        case RIVER:
            g->set_color(textColor);
            displayName = true;
            break;
        case ISLAND:
            g->set_color(textColor);
            displayName = true;
            break;
        case BUILDING:
            g->set_color(textColor);
            displayName = true;
            break;
        case GREENSPACE:
            g->set_color(textColor);
            displayName = true;
            break;
        case GOLFCOURSE:
            g->set_color(textColor);
            displayName = true;
            break;
        case STREAM:
            g->set_color(textColor);
            break;
        case UNKNOWN:
            g->set_color(textColor);
            break;
        default:
            break;
    }
    bool overlapped = false;
    
    //range for a line of text
    double featureRangeX = 30;
    double featureRangeY = 30;

    //identify if the text will overlap with others
    for(int displayedNameIdx = 0; displayedNameIdx < featureTextPoints.size(); displayedNameIdx++){
        
        if(abs(xAvg - featureTextPoints[displayedNameIdx].x) < featureRangeX * widthToPixelRatio && 
           abs(yAvg - featureTextPoints[displayedNameIdx].y) < featureRangeY * heightToPixelRatio){
            overlapped = true;
            break;
        }
    }
    //display the feature name at predefined text display ratio when its name is not <noname>,
    //at the predefined level of detail ratio, and not overlapping with other texts
    if (displayName && featureName.compare("<noname>") != 0 && featureArea > visibleArea * textDisplayRatio && !overlapped){
        g->draw_text({xAvg, yAvg}, featureName);
        featureTextPoints.push_back({xAvg, yAvg});
    }
}



//display a pop-up box at given location with given title and content
void displayPopupBox(ezgl::renderer *g, std::string title, std::string content, double x, double y, ezgl::rectangle world) {
    //useful ratios (retrieved from try and error)
    double strLenToBoxRatio = 3.266;
    double windowToPopupBoxRatio = 8.385;
    
    //get the width and height in pixel coordinates of the visible screen
    ezgl::rectangle screen = g->get_visible_screen();
    double screenWidth = screen.width();
    double screenHeight = screen.height();

    //get the width of the pop-up window according to string length 
    int strLen = std::max(content.length(), title.length());
    
    //draw the rectangle for title
    y -= world.height() * windowToPopupBoxRatio / screenHeight;
    g->set_color(ezgl::GREY_55);
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y - world.height() * windowToPopupBoxRatio  / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y + world.height() * windowToPopupBoxRatio  / screenHeight });
    
    //draw the text of the title
    g->set_text_rotation(0);
    g->set_font_size(10);
    g->set_color(textColor);
    g->draw_text({x, y}, title);
    
    //draw the rectangle for the contents
    y -= world.height() * windowToPopupBoxRatio / screenHeight * 2;
    g->set_color(ezgl::GREY_75);
    g->fill_rectangle({x - world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y - world.height() * windowToPopupBoxRatio / screenHeight},
                      {x + world.width() * (strLen * strLenToBoxRatio / screenWidth), 
                       y + world.height() * windowToPopupBoxRatio / screenHeight });
                      
    //draw the text of the contents
    g->set_color(textColor);
    g->set_font_size(10);
    g->draw_text({x, y}, content);
}

//display all the POIs qualified for displaying
void displayPOI(ezgl::renderer *g) {
    
    //if the visible world area is smaller than this number, the POI will be displayed
    double areaToShowPOI = 8400000;
    double transportationOnlyRatio = 840000000;
    
    //approximate range of an POI icon
    double POIRange = 60;
    
    std::vector<ezgl::point2d> displayedPoints;
    
    //calculated the world to pixel coordinate ratio
    ezgl::rectangle world = g->get_visible_world();
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    double heightToPixelRatio =  world.height() / g->get_visible_screen().height();
    
    displayedPoints.clear();
    
    // loop through all the poi and show it 
    for(int i = 0; i < getNumPointsOfInterest(); i ++){
        double x = xFromLon(getPOIPosition(i).longitude());
        double y = yFromLat(getPOIPosition(i).latitude());
        
        //prevent overlapping of POI: if the POI is in a certain range of a previously displayed POI, it will not be displayed
        bool overlapped = false;
        for(int displayedPOIIdx = 0; displayedPOIIdx < displayedPoints.size(); displayedPOIIdx++){
            
            if(abs(x - displayedPoints[displayedPOIIdx].x) < POIRange * widthToPixelRatio && 
                    abs(y - displayedPoints[displayedPOIIdx].y) < POIRange * heightToPixelRatio){
                overlapped = true;
            }
        }
        
        // if the map is showing enough level of detail, and the poi is visible in the screen, then display it. 
        if (world.contains({x, y}) && world.area() < transportationOnlyRatio && !overlapped) {
            if (displayPOIById(g, i, widthToPixelRatio, heightToPixelRatio, world.area() > areaToShowPOI))
                displayedPoints.push_back({x, y});
        }
    }
}

//display POI name and icon with a given POI id
bool displayPOIById(ezgl::renderer *g, POIIdx id, double widthToPixelRatio, double heightToPixelRatio, bool transportationOnly) {
    ezgl::surface *iconSurface;
    
    //get the coordinates of the POI
    double x = xFromLon(getPOIPosition(id).longitude());
    double y = yFromLat(getPOIPosition(id).latitude());
    bool education, food, medical, transport, recreation,finance, gov, other, displayPOI;
    
    //boolean values for whether to call EZGL function to draw POI icon
    displayPOI = true;
    
    if (transportationOnly) {
        education = false;
        food = false;
        medical = false;
        transport = (selectedPOI.compare("all") == 0 || selectedPOI.compare("transport") == 0);
        recreation = false;
        finance = false;
        gov = false;
        other = false;
    } else {
    //boolean values for whether to display each type of POI
        education = (selectedPOI.compare("all") == 0 || selectedPOI.compare("education") == 0);
        food = (selectedPOI.compare("all") == 0 || selectedPOI.compare("food") == 0);
        medical = (selectedPOI.compare("all") == 0 || selectedPOI.compare("medical") == 0);
        transport = (selectedPOI.compare("all") == 0 || selectedPOI.compare("transport") == 0);
        recreation = (selectedPOI.compare("all") == 0 || selectedPOI.compare("recreation") == 0);
        finance = (selectedPOI.compare("all") == 0 || selectedPOI.compare("finance") == 0);
        gov = (selectedPOI.compare("all") == 0 || selectedPOI.compare("gov") == 0);
        other = (selectedPOI.compare("all") == 0 || selectedPOI.compare("other") == 0);
    }
        
    std::string poiType = getPOIType(id);
    std::string poiName = getPOIName(id);
    
    //load icon image by poiType
    if (poiType.compare("ferry_termial") == 0){   
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/ferry.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("theatre") != std::string::npos && recreation) {       
        
        if(recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/theater.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("school") != std::string::npos 
            || poiType.rfind("university") != std::string::npos 
            || poiType.rfind("college") != std::string::npos)) {
        
        if (education) 
            iconSurface = g->load_png("./libstreetmap/resources/images/university.png");
        else 
            displayPOI = false;

    } else if (poiType.rfind("parking") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/parkinggarage.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fast_food") != std::string::npos) {       
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fastfood.png");
        else 
            displayPOI = false;

    } else if (poiType.compare("community_centre") == 0) {       
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/communitycentre.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("pharmacy") == 0) {       
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/drogerie.png");
        else 
            displayPOI = false;
       
    } else if (poiType.rfind("cafe") != std::string::npos) {       
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/coffee.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("place_of_worship") == 0) {        
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/chapel-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("bank") == 0) {       
       
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bank.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("atm") == 0) {       
        
        if (finance) 
            iconSurface = g->load_png("./libstreetmap/resources/images/atm-2.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("cinema") == 0) {       
        
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/cinema.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("hospital") == 0 
            || poiType.compare("doctors") == 0 
            || poiType.find("clinic") != std::string::npos) {        
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/hospital-building.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("library") == 0) {        
        
        if (education) 
            iconSurface = g->load_png("./libstreetmap/resources/images/library.png");
        else 
            displayPOI = false;
        
    } else if ((poiType.rfind("restaurant") != std::string::npos 
            || poiType.rfind("food_") != std::string::npos)) {        
        
        if (food) 
            iconSurface = g->load_png("./libstreetmap/resources/images/restaurant.png");
        else 
            displayPOI = false;
        
    } else if (poiType.compare("police") == 0) {       
        
        if (gov) 
            iconSurface = g->load_png("./libstreetmap/resources/images/police.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("gym") != std::string::npos 
            || poiType.rfind("weight") != std::string::npos) {
        
        if (recreation) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fitness.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("dentist") != std::string::npos 
            || poiType.rfind("orthodon") != std::string::npos) {
        
        if (medical) 
            iconSurface = g->load_png("./libstreetmap/resources/images/dentist.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bus_s") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bus.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("fuel") != std::string::npos) {       
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/fillingstation.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("child") != std::string::npos) {        
        
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/daycare.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("bicyle") != std::string::npos) {       
       
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/bicyle_parking.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("toilets") != std::string::npos) {        
       
        if (other) 
            iconSurface = g->load_png("./libstreetmap/resources/images/toilets_inclusive.png");
        else 
            displayPOI = false;
        
    } else if (poiType.rfind("post_") != std::string::npos) {       
        
        if (other) iconSurface = g->load_png("./libstreetmap/resources/images/postal.png");
        else displayPOI = false;
        
    } else if (poiType == "airport" && transport) {     
        
        if (transport) 
            iconSurface = g->load_png("./libstreetmap/resources/images/airport.png");
        else 
            displayPOI = false;
        
    } else if (other) {
        iconSurface = g->load_png("./libstreetmap/resources/images/sight-2.png");
        
    } else {                                // else do not display anything       
        displayPOI = false;
    }
    
    if (displayPOI) {
        
        // make the middle bottom of the icon at the poi location
        double surfaceWidth = (double)cairo_image_surface_get_width(iconSurface) * widthToPixelRatio;
        double surfaceHeight = (double)cairo_image_surface_get_height(iconSurface) * heightToPixelRatio;
        g->draw_surface(iconSurface, {x - surfaceWidth / 2 , y + surfaceHeight} );

        // display poi name
        g->set_color(textColor);
        g->set_font_size(10);
        g->set_text_rotation(0);
        g->draw_text({x, y }, poiName);
    }
    
    return displayPOI;
}

//load the subway data
void loadSubway(ezgl::renderer *g){
    //level of detail ratios for display
    double showTextRatio = 10;
    double showLineRatio = 50;
    double stationWidth = 30;
    std::vector <ezgl::point2d> displayedCoord;
    displayedCoord.clear();
    ezgl::rectangle world = g->get_visible_world();
    double widthToPixelRatio =  world.width() / g->get_visible_screen().width();
    std::vector<Subway> displayText;
    for (int i = 0; i < cities[currentCityIdx]->subways.size(); i++){
        if (showLineRatio > widthToPixelRatio && world.contains(cities[currentCityIdx]->subways[i].location)) {
            g->set_color(cities[currentCityIdx]->subways[i].red, cities[currentCityIdx]->subways[i].green,
                    cities[currentCityIdx]->subways[i].blue, 255);
            g->fill_arc(cities[currentCityIdx]->subways[i].location, 5 * widthToPixelRatio, 0, 360);
            
        }
        if (world.contains(cities[currentCityIdx]->subways[i].location) && showTextRatio > widthToPixelRatio) {
            displayText.push_back(cities[currentCityIdx]->subways[i]);
        }
    }
        
    for (int stationIdx = 0; stationIdx < displayText.size(); stationIdx ++) {
        bool displayStation = true;

        for (int j = 0; j < displayedCoord.size(); j ++) {
            if (abs(displayText[stationIdx].location.x - displayedCoord[j].x) < (stationWidth * widthToPixelRatio) 
                    && abs(displayText[stationIdx].location.y - displayedCoord[j].y) < (stationWidth * widthToPixelRatio)) {
                displayStation = false;
                break;
            }

        }

        if (displayStation) {
            //draw the text of the contents
            g->set_text_rotation(0);
            g->set_color(textColor);
            g->set_font_size(10);
            g->draw_text(displayText[stationIdx].location, displayText[stationIdx].name);
            displayedCoord.push_back(displayText[stationIdx].location);
        }
    }    
}

// Callback for which the User Guide button is clicked
void helpBtnClicked(GtkWidget *, ezgl::application *application){

    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog (i.e. where to put stuffin the dialog)
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog; // the dialog box we will create
    // Update the status bar message
    application->update_message("Help Button Pressed");

    // Redraw the main canvas
    application->refresh_drawing();

    // BEGIN: CODE FOR SHOWING DIALOG
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    // Create the dialog window. Modal windows prevent interaction with other windows inthe same application
    dialog = gtk_dialog_new_with_buttons("User Guide", (GtkWindow*) window, GTK_DIALOG_MODAL, ("OK"), GTK_RESPONSE_ACCEPT, NULL);
    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // The help message on the User Guide
    std::string helpMessage = "Welcome to Googer Map! \n\n";
    helpMessage = helpMessage + "When the switch on the right is on, the program is in path finding mode, the user can choose two intersections"
            + "\neither using search bar or mouse clicking and press find path button to find the shortest path. \n\n" +
            "When the switch is off on the right, the program is in single searching mode. Users can search desired locations. \n\n" +
            "When the switch on the left is off, it is in searching bar mode. Users can set the starting point and destination by "
            + "\ntyping in the search bar and press [set from] and [set destination]. \n\n" +
            "Users can also press enter in the search bar to set starting point and destination and the user can choose the "
            + "\nintersection in the drop down list to set two points. \n\n" +
            "When the switch on the left is on, the program is in pin point mode. User can set a starting point and destination "
            + "\nby mouse clicking an intersection on the map and press the [set from] or [set destination] button. \n\n" +
            "Note: two modes searching bar mode and pin point mode can be in mixed usage meaning users can search the \nstarting point "
            + "using searching bar mode and then set the destination using pin point mode.";
    
    label = gtk_label_new(&helpMessage[0]);
    gtk_container_add(GTK_CONTAINER(content_area), label);

    // The main purpose of this is to show dialog??s child widget, label
    gtk_widget_show_all(dialog);

    // Connecting the "response" signal from the user to the associated callback function
    g_signal_connect(GTK_DIALOG(dialog), "response", G_CALLBACK(onDialogResponse), NULL);
    // END: CODE FOR SHOWING DIALOG
}

// Dialog response (OK button on popup or close icon) is clicked
void onDialogResponse(GtkDialog *dialog){

    /*This will cause the dialog to be destroyed*/
    gtk_widget_destroy(GTK_WIDGET (dialog));
}