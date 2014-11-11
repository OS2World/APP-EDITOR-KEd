typedef ULONG mylib_file_t;
//typedef HFILE mylib_file_t;

//extern char file_EOF_char;
extern bool file_write_eof;

extern SHORT crit_error;  /* error code of last failed dos call */
  /* >> document the error codes */

enum fflags {READONLY, WRITEONLY, READWRITE };

enum seek_flags {SEEK_SET, SEEK_CUR, SEEK_END };

/*************************************************************************
   The basic File class (for Sheet use) will have these methods:
        Create  Open  Close 

   I will also need a "FileMgr" class just for the Directory boxes.
   This will just provide "find first" services.

   Started work on ReadLine() function, can't use fgets() if I use OS/2
   calls to open file.

   I am going to make the basic File Class buffered, and not bother
   with an separate buffer class, or 2 File Classes, 1 buffered and 1 not.

   NOPE: 3/10/96
   The bufferred file stuff was really messing up the regular file
   class, so I'm gonna derive ABufdFile from File.

   Allocate buffer on Open(), and dealloc on Close().
   Do not alloc on Create, since we don't need to buffer an empty file.

   to do:
   Need some kind of flag to indicate binary mode on open and read

   =================================================================
   YOU KNOW, I'm jumping through hoops here, because fgets() returns
   CR and LF in the buf, and KED doesn't want these. Instead of writing
   a whole File Class, why not just have the KED display routines drop
   the CR or LF ???

   So, should I have Sheet drop the control chars, or View ??
   View, obviously, since Sheet MUST return what's in the buf without 
   any changes.

   Well, yeah
*************************************************************************/

class TheFile 
{
private:
  mylib_file_t _File;
  char _name[FNAME_LEN];
  bool _is_loaded;
  SHORT critical_err;
//  ULONG _size;
  bool _binary_mode;
  ULONG pos;
public:
         TheFile();
         TheFile(const char* filename);
         virtual ~TheFile() {}
  bool   Create (const char *filename);
  bool   Delete (const char *filename);
  bool   Open (const char *filename, SHORT access);
  bool   Close ();
  bool   Write(UCHAR *buf,ULONG size);
  USHORT Read(UCHAR* buf,ULONG size);
  bool   RandomRead(ULONG filepos, void* buf, USHORT size);
  UCHAR* ReadLine(UCHAR* str, int maxlen);
  bool   is_loaded() const { return _is_loaded; }
  bool   get_handle() const { return _File; }
  bool   binary_mode() const { return _binary_mode; }
  ULONG  Tell();
  ULONG  Size();
  bool   Seek(ULONG filepos,SHORT whence);
  bool   Rename (CHAR *old, CHAR *my_new);
  bool   Copy (const char *old, const char *my_new);
};

class ABufdFile : public TheFile        // bufferred File
{
private:
  UCHAR* _internal_buf;
  int    _level;      // points to un-gotten data in buf
  UCHAR* _curp;       // internal ptr to current pos in file buf
  enum {_BUF_SIZE = 32767};
public:
  ABufdFile() { TheFile(); }

};


#ifdef FUCKUP       // future classes
  bool   Random_write (mylib_file_t file, ULONG filepos, void* buf, USHORT size);
  bool   Makedir (CHAR *dir);
  bool   Removedir (CHAR *dir);
  bool   Setdate (mylib_file_t file,USHORT date,USHORT time);
  void   Write_to_screen (char *string);
#endif
/************************************************************************/

bool examine_error(ULONG error_code, const char *name);
                                            /* function definitions */

bool mylib_create (char const *filename, mylib_file_t *file);
bool mylib_open (char const *filename, mylib_file_t *file,SHORT access);
         /* "access" is from the following, or'ed together if necessary */
#define MYLIB_READONLY 1
#define MYLIB_WRITEONLY 2
#define MYLIB_READWRITE (MYLIB_READONLY|MYLIB_WRITEONLY)
bool mylib_close (mylib_file_t);
bool mylib_seek (mylib_file_t file, ULONG filepos,SHORT whence);
                            /* "whence" may be one of the following: */
#define MYLIB_SEEK_SET 0
#define MYLIB_SEEK_CUR 1
#define MYLIB_SEEK_END 2
bool mylib_random_write (mylib_file_t file, ULONG filepos, void* buf, USHORT size);
bool mylib_random_read (mylib_file_t file, ULONG filepos, void* buf, USHORT size);
USHORT mylib_read_text (mylib_file_t file,UCHAR* buf,ULONG size);
bool mylib_write_text (mylib_file_t file, UCHAR *buf,ULONG size);
ULONG mylib_filesize (mylib_file_t file); /* can mess up position pointer */
bool mylib_delete (char const *filename);
ULONG mylib_tell (mylib_file_t file);
bool mylib_rename (CHAR *old, CHAR *my_new);
bool mylib_copy (const char *old, const char *my_new);
bool mylib_makedir (CHAR *dir);
bool mylib_removedir (CHAR *dir);
bool mylib_setdate (mylib_file_t file,USHORT date,USHORT time);
void mylib_write_to_screen (char *string);

                                              /* orginally in UTIL.ASM    */
                                              /* now in MYLIB.C           */
                                              /* now in dirlist.cpp    */
//USHORT find_num_files(byte const* name);
//SHORT assembly_findfirst (struct dos_buffer_type * DTA, CHAR const* name,USHORT dirflags);
//SHORT assembly_findnext (struct dos_buffer_type * DTA);
ULONG get_freespace(char drive);
SHORT set_file_attributes(char const *file,USHORT attrib);

bool filesys_init(void);    // query OS2 for FAT or HPFS info
