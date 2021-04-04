

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

//call back function when the switch change between single searching mode and path finding mode
gboolean changeSearchingMode (GtkWidget *, gboolean state, gpointer data);

//call back function when switching between searching bar mode and pinpoint mode
gboolean changeSelectingMode(GtkWidget *, gboolean state, gpointer data);

void initialSetUp(ezgl::application *application, bool new_window);
void actOnMouseClick(ezgl::application* app, GdkEventButton* event, double x, double y);

//call back functions for setting path
void setFromBtnClicked(GtkWidget *, gpointer data);
void setToBtnClicked(GtkWidget *, gpointer data);

//call back functions for user guide
void helpBtnClicked(GtkWidget *widget, ezgl::application *application);
void onDialogResponse(GtkDialog *dialog);

void clearRouteBtnClicked(GtkWidget *, gpointer data);
#endif /* CALLBACK_H */

