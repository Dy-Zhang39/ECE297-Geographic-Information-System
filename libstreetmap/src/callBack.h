

/* 
 * store all the call back function used in ezgl
 */
#ifndef CALLBACK_H
#define CALLBACK_H

//call back function for search button
gboolean searchButtonIsClicked(GtkWidget *, gpointer data);

//call back function for text field
gboolean textEntryPressedEnter(GtkWidget *, gpointer data);

//call back for text entry change
gboolean textEntryChanges(GtkWidget *, gpointer data);

//change the map when user switch to different Map
gboolean changeMap(GtkWidget *, gpointer data);

//zoom in to the intersection when user choose a location from the bar
gboolean possibleLocationIsChosen(GtkWidget*, gpointer data);


//callback function for POI selection buttons
gboolean toggleAllPOI(GtkWidget *, gpointer data);
gboolean toggleEducationPOI(GtkWidget *, gpointer data);
gboolean toggleFoodPOI(GtkWidget *, gpointer data);
gboolean toggleMedicalPOI(GtkWidget *, gpointer data);
gboolean toggleTransportPOI(GtkWidget *, gpointer data);
gboolean toggleRecreationPOI(GtkWidget *, gpointer data);
gboolean toggleFinancePOI(GtkWidget *, gpointer data);
gboolean toggleGovPOI(GtkWidget *, gpointer data);
gboolean toggleOtherPOI(GtkWidget *, gpointer data);
gboolean toggleHidePOI(GtkWidget *, gpointer data);

//call back function for check box used to show subway
gboolean toggleSubway(GtkWidget *, gpointer data);

//call back function for check box used to night mode
gboolean toggleNightMode(GtkWidget *, gpointer data);

//call back function when the switch change position
gboolean toggleSwitch (GtkWidget *, gboolean state, gpointer data);


void initialSetUp(ezgl::application *application, bool new_window);
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);
#endif /* CALLBACK_H */

