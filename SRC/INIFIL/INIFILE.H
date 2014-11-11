/* IniFile header          v 1.0  */
/*  Oct 31, 1993                  */
/*
typedef short bool;
#define TRUE 1
#define FALSE 0
*/
enum Vartype {Int, Char, String, Long , Float, Double};

/*--------------------------------------------------------------------*/

typedef struct IniVar
{
  Vartype type;
  void *value;
  char *Name;
  char *Group;
};


/*--------------------------------------------------------------------*/

class IniFile
{
public:
   IniFile(char **argv);
   IniFile(void);
   ~IniFile();

   void Init(char **argv);
   char IniFileName[80];
   void LoadIni(void);
   void SaveIni(void);
   FILE *Open(void);
   void Show(char *linein);
};

/*--------------------------------------------------------------------*/
