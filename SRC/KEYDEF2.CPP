/*************************************************************************
       KEYDEF2.CPP    declares key values and names and function 
                      pointers so users can build their own key mappings

          rev 1.0    Sept 5, 1993
          rev 1.1    March 1994
                     added function pointers and a string for the 
                     function name to the table. See INITKEYS.C for
                     a description of the strategy
          rev 2.0    July 10, 1994
                     this version splits up the former 1 large KeyTable
                     into several pieces. This is needed so I can map
                     Shift-Arrow keys. 
*************************************************************************/
#include "usual.h"
#include "initkeys.h"
#include "vmem.h"        // we don't need this, but sheet.h does.....
#include "sheet.h"       
#include "view.h"
#include "edset.h"
#include "version.h"
#include "misc.h"       // beep_ok
#include "mymenu.h"
#include "block.h"     // mark_block
#include "ascii.h"     // show_ascii_chart
#include "shell.h"     // shell_menu
#include "tool.h"      // load_prob_file
#include "help.h"      // do_help 
#include "ed_cmd.h"    // function prototypes

//   MISC unmapped KEYS
/*     {""      ,  NULL                      ,0x0000 }, 
     {"BS"    ,  NULL                        ,0x0008 }, 
     {"TAB"   ,  NULL                        ,0x0009 }, 
     {"sF"    ,  NULL                        ,0x0009 }, 
     {"CR"    ,  NULL                        ,0x000A }, 
     {"ESC"   ,  NULL                        ,0x001B }, 
     {"BACKTAB" ,NULL                        ,0x010a }, 
     {"cBS"   ,  NULL                        ,0x0127 }, 
*/


//    struct has:   ascii str  = key name
//                  fptr       = pointer to default function for that key
//                  ushort num = scan code of key

//   string_name  func ptr          func str        ushort keyval
//=====================================================================
struct kname_val AltKeyTable[] = {
     {"AltQ"  ,  file_quit          ,"file_quit"    ,0x1000 },  // 0
     {"AltW"  ,  window_move        ,"window_move"  ,0x1100 }, 
     {"AltE"  ,  NULL               ,""             ,0x1200 }, 
     {"AltR"  ,  NULL               ,""             ,0x1300 }, 
     {"AltT"  ,  NULL               ,""             ,0x1400 }, 
     {"AltY"  ,  NULL               ,""             ,0x1500 }, 
     {"AltU"  ,  block_cut          ,"block_cut"    ,0x1600 }, 
     {"AltI"  ,  NULL               ,""             ,0x1700 }, 
     {"AltO"  ,  NULL               ,""             ,0x1800 }, 
     {"AltP"  ,  block_paste        ,"block_paste"  ,0x1900 }, 
     {""      ,  NULL               ,""             ,0x1A00 },    // 10
     {""      ,  NULL               ,""             ,0x1B00 }, 
     {""      ,  NULL               ,""             ,0x1C00 }, 
     {""      ,  NULL               ,""             ,0x1D00 }, 
     {"AltA"  ,  ascii_chart        ,"ascii_chart"  ,0x1E00 }, 
     {"AltS"  ,  file_save          ,"file_save"    ,0x1F00 }, 
     {"AltD"  ,  show_os_screen     ,"show_os_screen",0x2000 }, 
     {"AltF"  ,  menu_file          ,"menu_file"    ,0x2100 }, 
     {"AltG"  ,  NULL               ,""             ,0x2200 }, 
     {"AltH"  ,  NULL               ,""             ,0x2300 }, 
     {"AltJ"  ,  NULL               ,""             ,0x2400 },  // 20
     {"AltK"  ,  NULL               ,""             ,0x2500 }, 
     {"AltL"  ,  NULL               ,""             ,0x2600 }, 
     {""      ,  NULL               ,""             ,0x2700 }, 
     {""      ,  NULL               ,""             ,0x2800 }, 
     {""      ,  NULL               ,""             ,0x2900 }, 
     {""      ,  NULL               ,""             ,0x2A00 }, 
     {""      ,  NULL               ,""             ,0x2B00 }, 
     {"AltZ"  ,  NULL               ,""             ,0x2C00 }, 
     {"AltX"  ,  ked_exit           ,"ked_exit"     ,0x2D00 }, 
     {"AltC"  ,  block_copy         ,"block_copy"   ,0x2E00 }, // 30
     {"AltV"  ,  show_version       ,"show_version" ,0x2F00 }, 
     {"AltB"  ,  NULL               ,""             ,0x3000 }, 
     {"AltN"  ,  file_next          ,"file_next"    ,0x3100 }, 
     {"AltM"  ,  file_prev          ,"file_prev"    ,0x3200 }, 
     {""      ,  NULL               ,""             ,0x3300 }, 
     {""      ,  NULL               ,""             ,0x3400 }, 
     {""      ,  NULL               ,""             ,0x3500 }, 
     {""      ,  NULL               ,""             ,0x3600 }, 
     {""      ,  NULL               ,""             ,0x3700 }, 
     {""      ,  NULL               ,""             ,0x3800 }, // 40
     {""      ,  NULL               ,""             ,0x3900 }, 
     {""      ,  NULL               ,""             ,0x3A00 }, 
     {"AltF1" ,  NULL               ,""             ,0x6800 },
     {"AltF2" ,  NULL               ,""             ,0x6900 },     
     {"AltF3" ,  NULL               ,""             ,0x6A00 },     // 90
     {"AltF4" ,  NULL               ,""             ,0x6B00 },
     {"AltF5" ,  NULL               ,""             ,0x6C00 },     
     {"AltF6" ,  NULL               ,""             ,0x6D00 },
     {"AltF7" ,  NULL               ,""             ,0x6E00 },
     {"AltF8" ,  NULL               ,""             ,0x6F00 },
     {"AltF9" ,  NULL               ,""             ,0x7000 },
     {"AltF10",  NULL               ,""             ,0x7100 },
     {"",        NULL               ,""             ,0x7200 },
     {"AltUp"   ,  NULL             ,""             ,0x9800 },  // 
     {"AltDown",  NULL              ,""             ,0xA000 },  // 
     {"AltLeft" ,  NULL             ,""             ,0x9B00 },  // 
     {"AltRight",  NULL             ,""             ,0x9D00 },  // 
     {""      ,  NULL               ,""             ,0x0000 },
     };

struct kname_val MiscKeyTable[] = {
     {"F1"    ,  help               ,"help"         ,0x3B00 }, // HELP
     {"F2"    ,  menu_print         ,"menu_print"   ,0x3C00 }, 
     {"F3"    ,  menu_file          ,"menu_file"    ,0x3D00 },  
     {"F4"    ,  menu_settings      ,"menu_settings",0x3E00 }, 
     {"F5"    ,  block_start        ,"block_start"  ,0x3F00 }, 
     {"F6"    ,  block_end          ,"block_end"    ,0x4000 },  
     {"F7"    ,  menu_block         ,"menu_block"   ,0x4100 }, 
     {"F8"    ,  menu_search        ,"menu_search"  ,0x4200 }, // 50
     {"F9"    ,  NULL               ,""             ,0x4300 }, 
     {"F10"   ,  show_files         ,"show_files"   ,0x4400 }, // select_view
     {""      ,  NULL               ,""             ,0x4500 }, 
     {""      ,  NULL               ,""             ,0x4600 }, 
     {"HOME"  ,  go_bol             ,"go_bol"       ,0x4700 }, 
     {"UP"    ,  go_up              ,"go_up"        ,0x4800 }, 
     {"PGUP"  ,  go_pgup            ,"go_pgup"      ,0x4900 }, 
     {""      ,  NULL               ,""             ,0x4a00 }, 
     {"LEFT"  ,  go_left            ,"go_left"      ,0x4b00 }, 
     {""      ,  NULL               ,""             ,0x4c00 },    // 60
     {"RIGHT" ,  go_right           ,"go_right"     ,0x4d00 }, 
     {""      ,  NULL               ,""             ,0x4e00 }, 
     {"END"   ,  go_eol             ,"go_eol"       ,0x4f00 },      
     {"DOWN"  ,  go_down            ,"go_down"      ,0x5000 }, 
     {"PGDN"  ,  go_pgdown          ,"go_pgdown"    ,0x5100 }, 
     {"INS"   ,  NULL               ,""             ,0x5200 }, 
     {"DEL"   ,  del_char           ,"del_char"     ,0x5300 },
     {""      ,  NULL               ,""             ,0x0000 },      
     };

struct kname_val sKeyTable[] = {
     {"sF1"   ,  NULL               ,""             ,0x5400 }, 
     {"sF2"   ,  ked_shell          ,"ked_shell"    ,0x5500 },   
     {"sF3"   ,  ked_shell_run      ,"ked_shell_run",0x5600 },    // 70
     {"sF4"   ,  menu_shell         ,"menu_shell"   ,0x5700 }, 
     {"sF5"   ,  go_block_start     ,"go_block_start",0x5800 },    
     {"sF6"   ,  go_block_end       ,"go_block_end" ,0x5900 }, 
     {"sF7"   ,  NULL               ,""             ,0x5A00 }, 
     {"sF8"   ,  NULL               ,""             ,0x5B00 }, 
     {"sF9"   ,  NULL               ,""             ,0x5C00 }, 
     {"sF10"  ,  NULL               ,""             ,0x5D00 }, 
     {""      ,  NULL               ,""             ,0x0000 },
     };

struct kname_val cKeyTable[] = {
     {"cF1"   ,  NULL               ,""             ,0x5E00 },
     {"cF2"   ,  NULL               ,""             ,0x5F00 },     
     {"cF3"   ,  NULL               ,""             ,0x6000 },      // 80
     {"cF4"   ,  NULL               ,""             ,0x6100 },
     {"cF5"   ,  NULL               ,""             ,0x6200 },     
     {"cF6"   ,  NULL               ,""             ,0x6300 },
     {"cF7"   ,  NULL               ,""             ,0x6400 },
     {"cF8"   ,  NULL               ,""             ,0x6500 },
     {"cF9"   ,  NULL               ,""             ,0x6600 },
     {"cF10"  ,  NULL               ,""             ,0x6700 },
     {"cLEFT" ,  go_prev_word       ,"go_prev_word" ,0x7300 },    
     {"cRIGHT" , go_next_word       ,"go_next_word" ,0x7400 },    // 100
     {"cEnd",    NULL               ,""             ,0x7500 },
     {"cPGDN" ,  go_eof             ,"go_eof"       ,0x7600 },      
     {"cHome",   NULL               ,""             ,0x7700 }, 
     {"",        NULL               ,""             ,0x7800 },
     {"",        NULL               ,""             ,0x7900 },
     {"",        NULL               ,""             ,0x7A00 },
     {"",        NULL               ,""             ,0x7B00 },
     {"",        NULL               ,""             ,0x7C00 },
     {"",        NULL               ,""             ,0x7D00 },  
     {"",        NULL               ,""             ,0x7E00 },  // 110
     {"",        NULL               ,""             ,0x7F00 },
     {"",        NULL               ,""             ,0x8000 },  
     {"",        NULL               ,""             ,0x8100 },
     {"",        NULL               ,""             ,0x8200 },
     {"",        NULL               ,""             ,0x8300 },     
     {"cPGUP" ,  go_bof             ,"go_bof"       ,0x8400 }, 
     {"cA"    ,  NULL               ,""             ,0x1E01 },  // ctrl A
     {"cB"    ,  del_bol            ,"del_bol"      ,0x3002 },  // ctrl B
     {"cC"    ,  NULL               ,""             ,0x2E03 },  // ctrl C  
     {"cD"    ,  NULL               ,""             ,0x2004 },  // ctrl D  120
     {"cE"    ,  del_eol            ,"del_eol"      ,0x1205 },  // ctrl E
     {"cF"    ,  NULL               ,""             ,0x2106 },  // ctrl F  
     {"cG"    ,  NULL               ,""             ,0x2207 },  // ctrl G
     {"cH"    ,  NULL               ,""             ,0x2308 },  // ctrl H
     {"cI"    ,  NULL               ,""             ,0x1709 },  // ctrl I
     {"cJ"    ,  NULL               ,""             ,0x240A },  // ctrl J
//     {"cK"    ,  wordstar_block     ,"wordstar_block",0x250B },  // ctrl K
     {"cL"    ,  NULL               ,""             ,0x260C },  // ctrl L
     {"cM"    ,  search_prev        ,"search_prev"  ,0x320D },  // ctrl M  
     {"cN"    ,  search_next        ,"search_next"  ,0x310E },  // ctrl N 130
     {"cO"    ,  NULL               ,""             ,0x180F },  // ctrl O
     {"cP"    ,  NULL               ,""             ,0x1910 },  // ctrl P   
     {"cQ"    ,  NULL               ,""             ,0x1011 },  // ctrl Q
     {"cR"    ,  del_word_left      ,"del_word_left",0x1312 },  // ctrl R
     {"cS"    ,  NULL               ,""             ,0x1F13 },  // ctrl S
     {"cT"    ,  del_word_right     ,"del_word_right",0x1414 },  // ctrl T
     {"cU"    ,  NULL               ,""             ,0x1615 },  // ctrl U
     {"cV"    ,  NULL               ,""             ,0x2F16 },  // ctrl V
     {"cW"    ,  window_size        ,"window_size"  ,0x1117 },  // ctrl W  
     {"cX"    ,  NULL               ,""             ,0x2D18 },  // ctrl X  140
     {"cY"    ,  del_line           ,"del_line"     ,0x1519 },  // ctrl Y
     {"cZ"    ,  NULL               ,""             ,0x2C1A },  // ctrl Z
     {""      ,  NULL               ,""             ,0x0000 },
     };                                            
                                              
                                              
                                              
                                              
                                          
                                          
                                          
                                          
                                          
