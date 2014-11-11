/********************************************************************** 
           VMEM module -- virtual memory management 
           rev 12-24-92     first cut at class design.
*********************************************************************/
#ifndef VMEM_H
#define VMEM_H 

//#define VMEM_INLINE      // 7KB is used if vmem_inline

#define VMEM_PAGESIZE 8192
//#define  MAX_FRAMES   4096 /* frames *vmem_pagesize = 32MB */ // mod 10-21-93
//const int MAX_FRAMES = 4096; /* frames *vmem_pagesize = 32MB */ // mod 10-21-93

struct frame_t {
   void *contents;         /* the stuff itself */
   };                     

class Mem {
private:
//static struct frame_t *myframe;     /* global pointer needed for inlines */
//static struct frame_t frame[MAX_FRAMES];       /* uses 6 bytes * 1024 */

public:

//static struct frame_t frame[MAX_FRAMES];       /* uses 4 bytes * MAX_FRAMES */
static void *mylib_allocmem (USHORT nparas); // Memory allocation functions 
static void mylib_freemem (void *block);
static SHORT vmem_setup(void);
static void vmem_shutdown (void);
static  void *vmem_trans(vptr p_in);

};              // end class MEM

                                    /* low level page commands */
vptr vmem_newpage(void);            /* allocate a page */
void vmem_droppage(vptr);           /* deallocate a page */

SHORT vmem_create_space(void);      /* high level allocation commands */
void vmem_delete_space (SHORT);
void *vmem_alloc(SHORT space, USHORT size, vptr* v);
void vmem_free (vptr);
                                 
SHORT vmem_burst_start (SHORT space);        /* burst mode */
void vmem_burst_end(void);
void* vmem_burst_alloc (USHORT size, vptr* v);
                  

#endif    // VMEM_H
