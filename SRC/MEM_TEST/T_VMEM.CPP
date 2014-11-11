/*

     TEST VMEM.CPP 

     test VMEM for access via array rather than struct of void * pointers

     original frame_t frame[]  routines = 29 seconds for 10,000 reps

     new void* frame[]         routines = 28.9 seconds for 10,000 reps

*/

#include <time.h>
#include <stdio.h>


#include "..\usual.h"
#include "vmem.h"
#include "dmm.h"



void main(void)
{
    int i;                                       /* loop counter    */
    int reps;
    time_t cstart1, cend1;
    int my_space;
    vptr foo;
    char *my_ptr;



   Mem::vmem_setup();
   my_space = vmem_create_space();

   printf(" size of struct frame_t = %d\n", sizeof(frame_t));
   printf(" size of     void * ptr = %d\n", sizeof(void *));
   printf(" size of     USHORT     = %d\n", sizeof(USHORT));
   printf(" size of     ULONG      = %d\n", sizeof(ULONG));
   printf(" size of     USHORT*    = %d\n", sizeof(USHORT *));
   printf(" size of     ULONG*     = %d\n", sizeof(ULONG *));

    cstart1 = clock();

    for(reps = 0; reps < 10000; reps++) {
       my_ptr = (char *)vmem_alloc(my_space, 80, &foo);
    }

    cend1 = clock();

    printf( "\nvmem struct access :\t\t\t%4.2f seconds \n",
                        ((float)cend1 - cstart1) / CLK_TCK );


}