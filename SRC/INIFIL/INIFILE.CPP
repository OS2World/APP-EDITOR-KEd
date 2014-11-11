/*--------------------------------------------------------------------*/
/*  File: INIFILE.CPP                                                 */
/*                                                                    */
/*  Purpose: class for reading/writing Windows-style *.INI-files      */
/*                                                                    */
/*  For settings of variables which have to be changed only once      */
/*  in a while, it isn't necessary to design special dialogs          */
/*  for making the adjustments; like in most WINDOWS applications,    */
/*  you can define certain variables which are initialized by         */
/*  an ASCII file which can be edited by any ASCII editor.            */
/*  The file is divided into groups of variables "[GROUP]",           */
/*  and the variables with their appropriate settings.                */
/*  I wrote this class for my own purposes, therefore I only          */
/*  included handling for int's and strings.                          */
/*  Adding other types of variables is easy, though.                  */
/*  Any feedbacks are welcome.                                        */
/*                                                                    */
/*                                                                    */
/*  Variables are defined in INIFILE.VAR; all existing variables      */
/*  can be used, just store pointers to variables in structure        */
/*  "inivar".                                                         */
/*                                                                    */
/*  This code is public domain and can be used freely.                */
/*  There is no guarantee that this code is bug-free!                 */
/*  You use it at your own risk.                                      */
/*                                                                    */
/*  (c)Sagner Software & Systeme                                      */
/*  PO Box 19 05 46                                                   */
/*  D 42705 Solingen , Germany                                        */
/*                                                                    */
/*  CIS 100022,2357                                                   */
/*     
//      SAVE wipes comment lines !!                                                         
/*--------------------------------------------------------------------*/

#define MAIN
#define TESTING

//#include <ttypes.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <stdlib.h>
#include <dir.h>
#include <dos.h>

/*--------------------------------------------------------------------*/

#include "inifile.h"
#include "inifile.var"

/*--------------------------------------------------------------------*/

IniFile::IniFile(void)
{
    sprintf(IniFileName , "PROGRAM.INI");
}

/*--------------------------------------------------------------------*/

IniFile::IniFile(char** argv)
{
    char drive[3] , dir[80] , name[10] , ext[5];
    fnsplit(argv[0] , drive , dir , name , ext);       // default: EXENAME.INI
    fnmerge(IniFileName , drive , dir , name , "INI");
}

/*----------------------------------------------------------------*/
void IniFile::Init(char** argv)
{
                                // sets IniFileName from argv

    char drive[3] , dir[80] , name[10] , ext[5];
//    fnsplit(argv[0] , drive , dir , name , ext);
    fnsplit(argv[1] , drive , dir , name , ext);        // use cmd line arg
    fnmerge(IniFileName , drive , dir , name , "INI");
}

/*--------------------------------------------------------------------*/

IniFile::~IniFile()
{
}
/*--------------------------------------------------------------------*/

int sortstrings(const void *string1 , const void *string2)
{
    return(strcmp(*(char **)string1 , *(char **)string2));
}

/*------------------------------------------------------------------*/

FILE *IniFile::Open(void)
{          
    FILE *handle;
    char TempBuf[100];
    char ConvBuf[10];
    char SavePath[80];
    int VerNbr;
    char drive[3] , dir[80] , name[10] , ext[5];

    strcpy(SavePath , IniFileName);
    _dos_setfileattr(SavePath , _A_NORMAL);
    handle = fopen(SavePath , "rb");         

    if (handle == NULL) {
        struct ffblk fblock;

        memcpy(ConvBuf , &name[strlen(name) - 3]  , 3);
        ConvBuf[3] = '\0';
        VerNbr = 0;

        memcpy(&name[strlen(name) - 3] , "???" , 3);
        fnmerge(SavePath , drive , dir , name , "ini");

        if(findfirst ( SavePath , &fblock, FA_ARCH)==0) {
            do
            {
                fnsplit(fblock.ff_name , drive , dir , name , ext);
                memcpy(ConvBuf , &name[strlen(name) - 3]  , 3);
                ConvBuf[3] = '\0';

                if(atoi(ConvBuf) > VerNbr) {
                    VerNbr = atoi(ConvBuf);
                }
            }
            while(findnext ( &fblock) == 0);

            fnsplit(SavePath , drive , dir , name , ext);
            sprintf(ConvBuf , "%3.3i" , VerNbr);
            memcpy(&name[strlen(name) - 3] , ConvBuf , 3);
            fnmerge(SavePath , drive , dir , name , "ini");
            handle = fopen(SavePath , "rb");
        }

    }   
#ifdef TESTING
  printf("INI File = %s\n", SavePath);
#endif
  return(handle);
}
  
/*-----------------------------------------------------------------------*/
void IniFile::Show(char *LineBuff)
{
  char *p;
  char *comment = 0;
  char CurGroup[80];
  char Name[80];
  
//  printf("Show LineBuf = %s\n", LineBuff);

  comment = strchr(LineBuff, ';');
  if(comment)       // ";" means a comment line
     return;

  p = strstr(LineBuff , "[");       // find GROUPS
  if(p != NULL) {
     strcpy(CurGroup , p + 1);
     p = strstr(CurGroup , "]");
     if(p != NULL) {
          *p = '\0';
     }
     printf("CurGroup = %s\n", CurGroup);
  }

  p = strstr(LineBuff , " =");

  if(p != NULL) {
      *p = '\0';
      strcpy(Name , LineBuff);
  //    printf("Name = %s\n", Name);

      int nn = sizeof(inivar) / sizeof(IniVar);

      for(int i = 0 ; i < nn ; i ++) {
          if(strcmp(inivar[i].Group , CurGroup) == 0) {
              if(strcmp(inivar[i].Name , Name) == 0)  {
                  switch(inivar[i].type) {
                      case Int: {
                          *(int *)inivar[i].value = atoi(p + 2);
                          printf("\tINT %s = %d\n", inivar[i].Name, *(int *)inivar[i].value);
                          break;
                      }
                      case String: {
                          p += 2;
                          while(*p == ' ') {
                              p++;
                          }
                          strcpy((char *)inivar[i].value , p);
                          p = strstr((char *)inivar[i].value , "\r\n");

                          if(p != NULL) {
                              *p = '\0';
                          }
                          printf("\tSTRING  %s = %s\n", inivar[i].Name, inivar[i].value);
                          break;
                      }
                  }
                  break;
              }           // end switch type
          }               // end switch name
      }                   // end switch group
  }                       // end for loop

}

/*--------------------------------------------------------------------*/

void IniFile::SaveIni(void)
{
    FILE *handle;
    char TempBuf[100];

    _dos_setfileattr(IniFileName , _A_NORMAL);
    handle = fopen(IniFileName , "wb");

    if (handle == NULL) {
        perror("cannot open initialisation file");
    }
    else {
        int nn = sizeof(inivar) / sizeof(IniVar);
        char *ptr[800];
        setmem(ptr , sizeof(ptr) , 0);

        for(int i = 0 ; i < nn ; i ++) {
            ptr[i] = (char *)malloc(81);
            char Value[30];

            switch(inivar[i].type) {
                case Int: {
                    sprintf(Value , "%i" , *(int *)inivar[i].value);
                    break;
                }
                case String: {
                    sprintf(Value , "%.25s" , inivar[i].value);
                }
            }
            if(ptr[i] != NULL) {
                sprintf(ptr[i] , "[%.25s]%.25s = %.25s" ,
                    inivar[i].Group , inivar[i].Name , Value);
            }
        }

        qsort((void *)ptr , nn , sizeof(char*) , sortstrings);

        char LineBuff[110];
        char GroupBuf[30];
        char CurGroup[30];

        sprintf(CurGroup , "");

        for(i = 0 ; i < nn ; i ++) {
            char *p = strstr(ptr[i] , "]");
            if(p != NULL) {
                setmem(GroupBuf , 30 , 0);
                memcpy(GroupBuf , ptr[i] , p - ptr[i] + 1);
                if(strcmp(CurGroup , GroupBuf) != 0) {
                    strcpy(CurGroup , GroupBuf);
                    strcpy(LineBuff , GroupBuf);
                    strcat(LineBuff , "\r\n");
                }
                else {
                    sprintf(LineBuff , "");
                }
                strcat(LineBuff , p + 1);
                strcat(LineBuff , "\r\n");
                fputs(LineBuff , handle);
            }
            free(ptr[i]);
        }
    }
    fclose(handle);
}

/*--------------------------------------------------------------------*/
void IniFile::LoadIni(void)
{
  FILE *handle;
  handle = Open();

if(handle != NULL) {
   char LineBuff[100];

  while(fgets(LineBuff , 100 , handle) != 0)
  {
     Show(LineBuff);
    //  printf("LineBuff = %s", LineBuff);
  }                           // end while (fgets)
}                               // end if handle (!=null)
fclose(handle);
}

/*--------------------------------------------------------------------*/

#ifdef MAIN

void usage(void);

IniFile inifile;                                             
                                                              
void main( int argc, char** argv )                            
{    
   if(argc < 2) 
      usage();                                                      

   inifile.Init(argv);                 //defines Ini-filename            
                                                              
   inifile.LoadIni();       // initializes variables          
     
//   mainloop();                                                
                                                              
   inifile.SaveIni();       // if needed: saves (overwrites) INI-file            
                                                              
}                                                             
           
void usage(void)
{
   printf("\n usage: INIFILE <filename>  \n");
   printf("\n retrieves or saves variables in an initialization file\n");
   exit(-2);
}
#endif                                                              
