  //=============================================================
  // NOEXCEPT.CPP - can be compiled as C or C++
  //=============================================================

  #ifdef __cplusplus
  extern "C"
  #endif // __cplusplus
  void _ExceptInit(void) {}

#ifdef __cplusplus
  #include <alloc.h>
  #include <new.h>

  new_handler _new_handler = NULL;

  new_handler set_new_handler(new_handler p)
  {
      new_handler t = _new_handler;
      _new_handler = p;
      return t;
  }

  void *operator new( size_t size )
  {
      void * p = 0;

      size = size ? size : 1;
      while ( (p = malloc(size)) == NULL &&
              _new_handler != NULL)
          _new_handler();
   }

#endif     // __cplusplus