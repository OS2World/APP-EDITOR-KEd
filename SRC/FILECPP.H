/**********************************************************************

     fileclass.h

     Other classes deal with Directory, FileNames/PathNames

     rev .001   2/21/96
            portable(?) handle-based class for files.
            open
            close
            create
            delete
            get_ (size, date, etc)
  
  
       
*************************************************************************/             

#include "usual.h"


class File {
   private:
           String _path;
           int    _size;
           int    _action;
           int    _mode;
           int    _handle;
           int    _lasterr;
   public:
           File();
           File( const char &path, int action, int mode);
           ~virtual File();

           virtual bool open();
           virtual bool open(const char &path, int action, int mode);
           void close();

           virtual bool create();
           virtual bool create(const char &path, int mode);
   
           virtual bool read(void *buf, size_t amount, size_t *bytes_read);
           virtual bool write(void *buf, size_t amount, size_t *bytes_written);

           virtual size_t size() const { return _size; }


};

