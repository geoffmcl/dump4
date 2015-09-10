
// Dump4.h

#include "DumpVers.h"   // ONLY contains #defines ...

// A 32-Bit CONSOLE Application to do a HEX Dump of a File
// or group of files.
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef __cplusplus
extern "C" {
#endif

// local includes
#include "DumpStruc.h"
//#define	dMALLOC(a)	malloc(a)
//#define  dMFREE(a)   free(a)
#define	dMALLOC(a)	LocalAlloc(LPTR,a)
#define  dMFREE(a)   LocalFree(a)
#define CHKMEM(a)   if( a == NULL ) { sprtf("ERROR: MEMORY FAILED!"MEOR); wait_keyin(); pgm_exit(-1); }
extern void pgm_exit(int);

typedef  struct tagMWL  {
   LIST_ENTRY              pList;
   PIMAGE_SYMBOL           pSym;    // pointer to the symbol
   PIMAGE_SECTION_HEADER   pSectHdr;   // pointer to section header
   DWORD                   dwFlag;
   DWORD                   dwRes1;
   BOOL                    bIsFunc;    // set as a CODE function
}MWL, * PMWL;

#include "DumpHelp.h"
#include "DumpWork.h"
#include "Dump4Cab.h"
#include "DumpSynE.h"   // SYNEDIT profile file
#include "DumpLib.h"    // Dump ARCHIVE (Library) *.lib file
#include "DumpObj.h"
#include "DumpUtil.h"
#include "DumpList.h"
#include "DumpFile.h"
#include "DumpMk4.h"
#include "Intel.h"
#include "DumpBmp.h" // dump a BMP image file
#include "DumpGif.h" // dump a GIF image file - added July 2003
#include "DumpRGB.h" // dump a RGB image file - 23 July, 2003
#include "DumpLNK.h" // dump a LNK Microsoft LNK (shortcut) file - 13 Nov, 2006
#include "DumpEXE.h" // FIX20080507 - added DumpPE FIX20061216 - dump an EXE image file
#include "DumpOS2.h" // FIX20080507 - added DumpPE FIX20061216 - dump an EXE image file
#include "headers.h" // some structure and macros
#include "DumpCode.h"   // try to show CODE
#include "DumpASCII.h"  // -a[n] dump ASCII, of min length n (def = ASCII_MIN) was 4
#include "DumpHex.h" // the ORIGINAL simple HEX dump
#include "DumpSHP.h" // -shp = dump as shapefile ...
#include "DumpPE.h"  // from PEDUMP
#include "DumpTar.h" // -tar = dump as TAR file ...
#include "DumpM2TS.h" // -m2ts = dump as M2TS movie clip file ...
#include "DumpAVI.h" // -avi = dump as an AVI file ...
#include "DumpSonic.h" // --sonic - dump as Sonic project file, listing items

#define  VFH(a)      ( a && ( a != INVALID_HANDLE_VALUE ) )

#define  ISUPPER(a)     ( ( a >= 'A' ) && ( a <= 'Z' ) )
#define  ISLOWER(a)     ( ( a >= 'a' ) && ( a <= 'z' ) )
#define  ISNUMERIC(a)   ( ( a >= '0' ) && ( a <= '9' ) )

#define  ISNUM(a)    ISNUMERIC(a)
#define  ISUP(a)     ISUPPER(a)
#define  ISLOW(a)    ISLOWER(a)
#define  ISALPHA(a)  ( ISUPPER(a) || ISLOWER(a) )
#define  ISPRT(a)  ( ISNUM(a) || ISALPHA(a) || (a == '_') )
#define  ISSIGCHAR(a)   ( ( a >= ' ' ) && ( a < 0x7f ) )

extern	HANDLE	grmOpenFile( LPTSTR fn, HANDLE * ph, UINT uFlag );
extern	DWORD	   grmReadFile( HANDLE hf, BYTE * lpb, DWORD dwMax );
extern   HANDLE	grmCreateFile( LPTSTR fn );

extern  int   MCDECL sprtf( LPTSTR lpf, ... );
extern int vsprtf( PTSTR ps, va_list arglist );

extern   void	prt( LPTSTR lps );

extern   void  MCDECL chkme(LPTSTR lpf, ...);

extern   void	MyExit0( void );
extern   void	MyExit1( void );

#define g_flProtext     PAGE_READONLY
#define g_ReqAccess     FILE_MAP_READ

#ifdef __cplusplus
}
#endif
// eof - Dump4.h
