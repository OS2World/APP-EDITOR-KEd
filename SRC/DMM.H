#ifndef DMM_H
#define DMM_H

typedef USHORT dmm_offset;
USHORT *dmm_configure(USHORT n, USHORT *temp);
USHORT dmm_alloc (USHORT *Base, USHORT size);
void dmm_free (USHORT *Base, USHORT p);


inline USHORT *dmm_configure(USHORT n, USHORT *temp)
{               /* returns a pointer to a block temp of size n integers */
temp[0]= 1;     /* 1st location is freelist pointer, we start w/ 1 block */
temp[1]= n-1;   /* size of the block, set up as one big free block */
temp[2]= 0;     /* 1st data location - 0 means it's available */
return temp;
}

#endif