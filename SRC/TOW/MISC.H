#ifndef MISC_H
#define MISC_H

#if !defined(___DEFS_H)
#include <_defs.h>
#endif

void beep_ok(void);

//char* num2str (char* s, SHORT n, SHORT len, char fillchar);
char* num2str (char* s, LONG n, SHORT len, char fillchar);
char* longnum2str (char* s, long n, SHORT len, char fillchar);

char* num2str_fast (char* s, LONG n, SHORT len);
char* longnum2str_fast (char* s, long n, SHORT len);

void left_just (char* s, SHORT len);
SHORT strcmp_ni (char const* s1, char const* s2, SHORT len);

//#define fillwords(dest,val,count) memset(dest,val,(2*count)) //no work!
//void fillwords (USHORT* dest, USHORT val, USHORT count);
//void fillwords (void *dest, USHORT value, USHORT count);


#ifdef MEMCPY_C     
                                          // in tbasewin.cpp
void memcpy_even (byte* dest, byte const* source, USHORT count);
                                          //cpp code in misc.cpp
void fillwords (void *dest, USHORT value, USHORT count);
#else
#ifdef __cplusplus
extern "C" {
#endif
                                                     // asm code in fillw.asm
void * _RTLENTRYF _EXPFUNC fillwords(void * __s, int __c, unsigned __n);
                                                   // asm code in tb5.asm
//void * _RTLENTRYF _EXPFUNC memcpy_even(byte* dest, byte const* source, USHORT count);
void * _RTLENTRYF _EXPFUNC memcpy_even(USHORT* dest, byte const* source, USHORT count);
#ifdef __cplusplus
}
#endif
#endif

//SHORT upcase(SHORT);  /* works on extended key codes */
inline SHORT upcase(SHORT c)
{
  if (c >= 'a' && c <= 'z') 
    return c &=~32;
  else 
    return c;
}

#endif     //MISC_H
