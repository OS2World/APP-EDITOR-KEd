/*****************************************************
File: ASCII.C          Copyright 1989 Dlugosz Software
   ASCII character chart
*****************************************************/
#include "usual.h"
#include "keycodes.h"
#include "misc.h"
#include "getkey.h"
#include "setting.h"
#include <string.h>

#include "basewin.h"

#define colors settings.colors[ASCIIBOX_COLORS]
#define pos(n) settings.location[ASCIIBOX_LOCATION].n

SHORT ascii_chart (SHORT code, char *str)
/* returns 1 for decimal, 2 for 'H'ex, 3 for 'h'ex, 4 for char, -1 if aborted */
{
SHORT x, y;
static SHORT oldtype= 4;
static char *mode[]= {"Dec","Hex","hex","Chr"};
SHORT h,v,key,type,nibble,toupper_value;
//char s[32];                              // mod 7-27-94
byte s[32];

basewin asciiw (10, 34,pos(y),pos(x));
asciiw.clearwin(colors[NORMAL_COLOR], ' ');
asciiw.border(window_core::Double, colors[BORDER_COLOR]);    
asciiw.title(TCenter," ASCII Box ");
asciiw.show();

for (v= 0; v < 8; v++) {
   for (h= 0; h < 32; h++) {
      if (h > 15) 
//         s[h]= (char)(128+v*16+h-16);
         s[h]= (byte)(128+v*16+h-16);
      else 
//         s[h]= (char)(v*16+h);
         s[h]= (byte)(v*16+h);
      }
   if (v == 0) {
      s[0]= 255;  /* keep putstring from terminating at the '\0' */
   }
   asciiw.put(v+1, 1, s, 32);
   }

asciiw.highlight(9,14,colors[HIGHLIGHT_COLOR],6);
asciiw.title(BLeft,mode[oldtype-1]);
asciiw.highlight(9,2,colors[HIGHLIGHT_COLOR],3);

x= code%16;
y= code/16;
if (code >= 128) {
   y-= 8;
   x+= 16;
   }
//while (1) {
for (;;) {
   s[0]= (x > 15) ? (char)(8+y) : (char)(y);
   s[1]= (x > 15) ? (char)(x-16) :(char)(x);
   code= s[0]*16+s[1];
   num2str (s+2,code,4,' ');
   s[0]+= (s[0] > 9) ? 55 : 48;
   s[1]+= (s[1] > 9) ? 55 : 48;
   asciiw.title(BCenter,s);
   asciiw.highlight(y+1, x+1, colors[HIGHLIGHT_COLOR],1);
   key= getkey ();
   asciiw.highlight(y+1, x+1, colors[NORMAL_COLOR],1);
   switch (key) {
      case kUp:
         if (--y < 0) y= 7;
         break;
      case kDown:
         if (++y > 7) y= 0;
         break;
      case kLeft:
         if (--x < 0) x= 31;
         break;
      case kRight:
         if (++x > 31) x= 0;
         break;
      case kHOME:
         x= 0; break;
      case kEND:
         x= 31; break;
      case PageUp:
         y= 0; break;
      case PageDown:
         y= 7; break;
      case 'd':
      case 'D':
         type= 1; goto exit;
      case 'H':
         type= 2; goto exit;
      case 'h':
         type= 3; goto exit;
      case 'c':
      case 'C':
         type= 4; goto exit;
      case GREY_Enter:
      case Enter:             
         type= oldtype; goto exit;
      default:
         type= -1; goto exit;
      }                      /* switch */
   }                         /* while */
exit:
switch (type) {
   case -1:
      return -1;
   case 1:
      strcpy (str, num2str (s,code,3,'\0'));
      break;
   case 2:
   case 3:
      toupper_value= (type == 2) ? 55 : 87;
      nibble= (code >> 4) % 16;
      *str++= (char)(nibble + ((nibble>9) ? toupper_value:48));
      nibble= (code & 15) % 16;
      *str++= (char)(nibble + ((nibble>9) ? toupper_value:48));
      *str= '\0';
      break;
   case 4:
      *str= (char)code;
      str[1]= '\0';
   }
oldtype= type;
return type;             // basewin asciiw is automatically destroyed
}
