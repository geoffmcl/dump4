// DumpStruc.h
#ifndef _DumpStruc_H_
#define _DumpStruc_H_
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
// ======================================
#define MCDECL _cdecl
// ======================================
#else
// ======================================
// some unix glue
#include <string.h> // for strcat, ...
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define MCDECL
#define TRUE 1
#define FALSE 0

typedef unsigned char BYTE;
typedef BYTE *PBYTE;

typedef void *HANDLE;
typedef void *HPALETTE;
typedef void *HBITMAP;
typedef void *HFONT;

typedef void *PVOID;
typedef void *LPVOID;
typedef unsigned int DWORD;
typedef short SHORT;
typedef unsigned short WORD;
typedef char *LPTSTR;
typedef char *PTSTR;
typedef char *LPSTR;
typedef char *PSTR;
typedef char CHAR;
typedef char TCHAR;
typedef int BOOL;
typedef int INT;
typedef unsigned int UINT;
typedef void VOID;
typedef long LONG;
#define FAR
#define INVALID_HANDLE_VALUE (PVOID)-1

typedef struct _ULARGE_INTEGER {
    DWORD LowPart;
    DWORD HighPart;
} ULARGE_INTEGER;

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _IMAGE_SYMBOL {
    union {
        BYTE ShortName[8];
        struct {
            DWORD Short;
            DWORD Long;
        } Name;
        DWORD LongName[2];
    } N;
    DWORD Value;
    SHORT SectionNumber;
    WORD  Type;
    BYTE  StorageClass;
    BYTE  NumberOfAuxSymbols;
 } IMAGE_SYMBOL, *PIMAGE_SYMBOL;
 
 #define IMAGE_SIZEOF_SYMBOL 18
 
 #define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef long            FXPT2DOT30, FAR *LPFXPT2DOT30;

/* ICM Color Definitions */
// The following two structures are used for defining RGB's in terms of CIEXYZ.

typedef struct tagCIEXYZ
{
        FXPT2DOT30 ciexyzX;
        FXPT2DOT30 ciexyzY;
        FXPT2DOT30 ciexyzZ;
} CIEXYZ;
typedef CIEXYZ  FAR *LPCIEXYZ;

typedef struct tagICEXYZTRIPLE
{
        CIEXYZ  ciexyzRed;
        CIEXYZ  ciexyzGreen;
        CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;
typedef CIEXYZTRIPLE    FAR *LPCIEXYZTRIPLE;

typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} BITMAPV5HEADER, *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;


typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;


typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR   cFileName[ MAX_PATH ];
    CHAR   cAlternateFileName[ 14 ];
#ifdef _MAC
    DWORD dwFileType;
    DWORD dwCreatorType;
    WORD  wFinderFlags;
#endif
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
    WORD   e_res[4];                    // Reserved words
    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    WORD   e_oeminfo;                   // OEM information; e_oemid specific
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;


// ======================================
#endif

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

