/*****************************************************
file DMM.C          Copyright 1989 by Dlugosz Software
  dynamic memory management for small blocks
*****************************************************/

// #define MEMDEBUG 

#define INCL_DOS
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "dmm.h"

/*  DMM deals with INTEGER size (16 bit) blocks  ONLY  */

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef FUCKUP                                  // moved to dmm.h inline
USHORT *dmm_configure(USHORT n, USHORT *temp)
{               /* returns a pointer to a block temp of size n integers */
temp[0]= 1;     /* 1st location is freelist pointer, we start w/ 1 block */
temp[1]= n-1;   /* size of the block, set up as one big free block */
temp[2]= 0;     /* 1st data location - 0 means it's available */
return temp;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

USHORT dmm_alloc (USHORT *Base, USHORT size)
{
 USHORT prev, attempt, p, piece;
 prev = 0;
 attempt = Base[0];

if (attempt == 0)                /* empty freelist = no free blocks */
   return 0;            

size++;                     /* need one location for header */
while (Base[attempt] < size) {  /*   */
   prev = attempt+1;             /* address of pointer */
   attempt= Base[attempt+1];
   if (attempt == 0)             /* error */
       return 0; 
}                          
piece= Base[attempt] - size;
if (piece <= 1) {           /* size is larger than remaining block */
   Base[prev]= Base[attempt+1]; // take the whole block
   p= attempt+1; 
}
else {                      /* take latter part only */
   Base[attempt+piece]= size;
   Base[attempt] = piece;
   p= attempt+piece+1; 
}
return p;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

inline static void  check_merge (USHORT *Base, USHORT p)
{
USHORT q= 0;

 if (p < 1 || (p > 65534))
   return;

 q = Base[p+1];          // size of allocated block
 if (q == p+Base[p])  {
   Base[p] += Base[q];
   Base[p+1]= Base[q+1];  
 }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
void dmm_free (USHORT *Base, USHORT p)
{
USHORT previous= 0;        /* pointer to previous pointer */

p --;                         // Base[0] is num of alloc'ed blocks
while ((Base[previous] != 0) && (Base[previous] < p))
   previous= Base[previous]+1;

Base[p+1] = Base[previous];
Base[previous]= p;
check_merge (Base, p);
check_merge (Base, previous-1);      
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

/************************************************************************/
/*     this stuff is only for debugging DMM functions -
          I probably won't ever use it......
*/

#ifdef MEMDEBUG
extern void* malloc();

#define NULL 0L

struct ref_link {
   USHORT ref;
   USHORT blk_size;
   struct ref_link *next_ref; };

struct base_link {
   USHORT *base_address;
   struct base_link *next_base;
   struct ref_link *ref_list; } *base_root= NULL, *active_base; };

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  store_dmmbase (Base)
USHORT *Base;
{
struct base_link *temp;

temp= malloc (sizeof (struct base_link));
temp->base_address= Base; /* base stored */
temp->ref_list= NULL; /* no ref's yet */
temp->next_base= base_root;
base_root= temp;  /* record pushed */
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  check_base (Base)
USHORT *Base;
{
active_base= base_root;
while (active_base && active_base->base_address != Base)
   active_base= active_base->next_base;
if (! active_base) puts
   ("error in dmm-- attempt to access a Base that doesn't exist.");
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  store_allocation (p)
USHORT p;
{
struct ref_link *newref;

if (!p) 
   puts ("error in DMM-- dmm_alloc cannot get memory.");
newref= malloc(sizeof(struct ref_link));
newref->ref= p;
newref->blk_size= active_base->base_address [p-1];
newref->next_ref= active_base->ref_list;
active_base->ref_list= newref;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  delete_ref (block)
USHORT block;
{
struct ref_link *p, *q;

q= NULL;
p= active_base->ref_list;
while (p && p->ref != block) {
   q= p;
   p= p->next_ref; }
if (!p) puts 
 ("error in DMM-- attempt to free a block not allocated");
if (p->blk_size != active_base->base_address [block-1])
   report_error ("error in DMM-- header word has been corrupted.");
if (q) q->next_ref= p->next_ref;
else active_base->ref_list= p->next_ref;
free (p);
}

#endif                                       /* end DEBUG stuff  */
