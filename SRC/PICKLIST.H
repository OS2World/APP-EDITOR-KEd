/* picklist.h */

/* this is a general purpose picklist that uses the windows system */

/* this is where in the window the picklist appears.  If the list will scroll,
   it is assumed that the border is set to match, so just the portal scrolls. */
struct picklist_portal {
   SHORT startrow, startcol;
   SHORT rows, columns;
   byte cursor_color, normal_color;
   };

/* this customizes the picklist.  These functions are called to preform the
   various actions.  */
struct picklist_thunk {
   bool (*keyfilter) (SHORT key);
      /* if present, is called after each key is gotten.  If return value is FALSE,
         the picklist will ignore that key and loop.  */
   char const* (*fetch_choice) (SHORT index);
      /* this is called to get the string for choice n.  Called everytime that
         string needs to be displayed, if the list scrolls.  */
   void (*tellme) (SHORT index);
      /* if present, is called before getting a key.  It is given the index number
         of the choice where the bouncebar will appear. */
   };


//SHORT picklist (struct picklist_portal *p, window_t w, struct picklist_thunk *t, SHORT count, SHORT location);
SHORT picklist (struct picklist_portal *p, struct picklist_thunk *t, SHORT count, SHORT location);
