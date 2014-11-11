/* HPFS.C   holds parsing routines for OS/2 and LongFileNames

      rev. 1.0     2-1-93
*/

#define INCL_DOS
#include "usual.h"
#include "vmem.h"

#include <ctype.h>
#include <string.h>
#include <direct.h>   /* getdcwd */
#include <stdlib.h>   /* _MAX_PATH constant  */
#include <stdio.h>    /* perror func  */

char cwd[FNAME_LEN];
char progname[FNAME_LEN];




void parse_filename(UCHAR *s)
{
 UCHAR *tempstr;
 USHORT counter = 0;




}