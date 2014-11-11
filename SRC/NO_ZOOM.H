/* this is included in one source file to prevent the VIEW_ZM object
   from being linked in, when it is not used.  */

bool view_is_zoomed= FALSE;
void view_zoom(void) {}

#define NO_ZOOM 1