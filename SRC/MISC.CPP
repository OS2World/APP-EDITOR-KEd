/*****************************************************
File: MISC.C        Copyright 1989 by Dlugosz Software
   miscelaneous functions
*****************************************************/
#define INCL_DOS
#define INCL_NOPMAPI

#include <os2.h>

#include "usual.h"
#include "misc.h"

//#pragma intrinsic(memset)

//                             used to setup tab handling.....
//DBG_WIN2(dout<<endl <<endl);
//DBG_WIN2(dout<<" 1 % 4 = " << (1 % 4) <<endl);
//DBG_WIN2(dout<<" 2 % 4 = " << (2 % 4) <<endl);
//DBG_WIN2(dout<<" 3 % 4 = " << (3 % 4) <<endl);
//DBG_WIN2(dout<<" 4 % 4 = " << (4 % 4) <<endl);
//DBG_WIN2(dout<<" 5 % 4 = " << (5 % 4) <<endl);
//DBG_WIN2(dout<<" 6 % 4 = " << (6 % 4) <<endl);
//DBG_WIN2(dout<<" 7 % 4 = " << (7 % 4) <<endl);
//DBG_WIN2(dout<<" 8 % 4 = " << (8 % 4) <<endl);
//DBG_WIN2(dout<<endl <<endl);

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void beep_ok(void)
{
  DosBeep(1000,100);
  DosBeep(1400,50);
}

#ifdef FUCKUP
//void fillwords (USHORT *dest, USHORT value, USHORT count)
void fillwords (void *dest, USHORT value, USHORT count)
{
  unsigned short* mydest = (unsigned short*)dest;    // works fine
    for(int n = 1; n <= count; n++) {
       *(mydest++) = value;
    }

//  while (count--) 
//      *(dest++) = value;
}
#endif


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//char* num2str (char* s, SHORT n, SHORT len, char fillchar)
char* num2str (char* s, LONG n, SHORT len, char fillchar)
{
SHORT r, neg;
if (n < 0) {
   neg= TRUE;
   n= -n;
   }
else neg= FALSE;
s += len;
*s-- = '\0';
do {
   r= (SHORT)(n % 10);
   n /= 10;
   *s-- = (char)r + (char)'0';
   len--;
   }  while (n);
if (neg) *s--= '-';
if (fillchar) {
   while (len--) *s-- = fillchar;
   }
return s+1;
}


char* num2str_fast (char* s, LONG n, SHORT len)
{                 // assume no negative, no fillchar 
SHORT r;
s += len;                 // point to end of string
*s-- = '\0';              // terminate with 0
do {
   r= (SHORT)(n % 10);
   n /= 10;
   *s-- = (char)r + (char)'0';    // why '0' here?
   len--;
   }  while (n);

return s+1;
}
/*
char* longnum2str_fast (char* s, long n, SHORT len)
{
SHORT r;
s += len;
*s-- = '\0';
do {
   r= (SHORT)(n % 10);
   n /= 10;
   *s-- = (char)r + (char)'0';
   len--;
   }  while (n);

return s+1;
}
*/
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*
char* longnum2str (char* s, long n, SHORT len, char fillchar)
{
SHORT r, neg;
if (n < 0) {
   neg= TRUE;
   n= -n;
   }
else neg= FALSE;
s += len;
*s-- = '\0';
do {
   r= (SHORT)(n % 10);
   n /= 10;
   *s-- = (char)r + (char)'0';
   len--;
   }  while (n);
if (neg) *s--= '-';
if (fillchar) {
   while (len--) *s-- = fillchar;
   }
return s+1;
}
*/
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void left_just (char* s, SHORT len)
{
while (*s && len) {
   s++;
   len--;
   }
while (len --) {
   *s++ = ' ';
}
*s= '\0';
return;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//SHORT upcase (SHORT c)
//{
//  if (c >= 'a' && c <= 'z') return c &=~32;
//  else return c;
//}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT strcmp_ni (char const* s1, char const* s2, SHORT len)
{
SHORT c1, c2, r;
do {
   c1= upcase (*s1++);
   c2= upcase (*s2++);
   len--;
   } while (len && c1 == c2);
r= c1-c2;
if (r < 0) return -1;
if (r > 0) return 1;
return 0;
}
