// DumpStruc.h
#ifndef _DumpStruc_H_
#define _DumpStruc_H_
#include <sys\types.h>
#include <sys\stat.h>

typedef BYTE (*GETBYTE)(void *);

typedef  struct {
   char *   fn;
   HANDLE   hf;
   int      stat_res;
   struct stat stat_buf;
   ULARGE_INTEGER qwSize;
   DWORD    dwmax;   // max. of the mapping buffer == file size
   DWORD    dwrd;
   HANDLE   df_hMap;    // mapping handle

#ifdef USE_64BIT_SIZE
   GETBYTE  gb;
#else // only 32-bit offsets
   PVOID    df_pVoid;   // mapping pointer
   BYTE *   lpb;  // just a re-cast of the VOID to a BYTE pointer
   LPTSTR   lptmp;
#endif // USE_64BIT_SIZE y/n

   // use in PPM - Portable Pixmap (.ppm) P6 format.
   DWORD    df_dwHeight;   // height of pixel MAP
   DWORD    df_dwWidth;    // width of pixel MAP
   DWORD    df_dwMax;      // MAX_UCHAR = 255
   DWORD    df_dwOff;      // begin of mapping
   // =============================================
}DFSTR, * PDFSTR;

typedef DFSTR * LPDFSTR;



#endif // _DumpStruc_H_
// eof - DumpStruc.h

