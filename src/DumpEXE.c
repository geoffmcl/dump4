
// DumpEXE.c
// To DUMP an EXE file
#include "dump4.h"

#ifndef USE_PEDUMP_CODE // FIX20080507
#include "DumpObj.h"
#include <Dbghelp.h>

//#include <winnt.h>
#define  OUTIT(a,b)  sprtf(a, (pdos->b & 0xffff), (pdos->b & 0xffff))

/* ============================================
typedef struct _IMAGE_SECTION_HEADER {
   BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
   union {
      DWORD PhysicalAddress;
      DWORD VirtualSize;  } Misc;
   DWORD VirtualAddress;
   DWORD SizeOfRawData;
   DWORD PointerToRawData;
   DWORD PointerToRelocations;
   DWORD PointerToLinenumbers;
   WORD NumberOfRelocations;
   WORD NumberOfLinenumbers;
   DWORD Characteristics;
} IMAGE_SECTION_HEADER,  *PIMAGE_SECTION_HEADER;

   ============================================ */
PIMAGE_NT_HEADERS32 g_pNTHeaders = 0;
PIMAGE_FILE_HEADER pPEHeader = 0; // &pnt->FileHeader;
DWORD g_dwNumOfSections = 0; // (pifh->NumberOfSections & 0xffff);

DWORD GetSectionNameEXE( PTSTR pd, PIMAGE_SECTION_HEADER pis )
{
   DWORD dwi = 0;
   DWORD dwo = 0;
   if( pis->Name[0] == '/' ) {
      dwo = sprintf(pd, "Name offset %d, into string section.", atoi(&pis->Name[1]) );
   } else {
      for( dwi = 0; dwi < IMAGE_SIZEOF_SHORT_NAME; dwi++ )
      {
         if( pis->Name[dwi] ) {
            pd[dwo++] = pis->Name[dwi];
         } else {
            break;
         }
      }
      pd[dwo] = 0;
   }
   return dwo;
}

void OutSectionStruct( PIMAGE_SECTION_HEADER pis )
{
   LPTSTR lpd = &g_cBuf[0];
   GetSectionNameEXE( lpd, pis );
   sprtf( "Section [%s]"MEOR, lpd );
   sprtf( " PhysicalAddress      %u"MEOR, pis->Misc.PhysicalAddress);
   // DWORD VirtualSize;  } Misc;
   sprtf( " VirtualAddress       %u"MEOR, pis->VirtualAddress );
   sprtf( " SizeOfRawData        %u"MEOR, pis->SizeOfRawData );
   sprtf( " PointerToRawData     %u"MEOR, pis->PointerToRawData );
   sprtf( " PointerToRelocations %u"MEOR, pis->PointerToRelocations );
   sprtf( " PointerToLinenumbers %u"MEOR, pis->PointerToLinenumbers );
   sprtf( " NumberOfRelocations  %u"MEOR, pis->NumberOfRelocations & 0xffff );
   sprtf( " NumberOfLinenumbers  %u"MEOR, pis->NumberOfLinenumbers & 0xffff );
   sprtf( " Characteristics      %#x"MEOR, pis->Characteristics );
   *lpd = 0;
   AppendSectionFlag( lpd, pis->Characteristics );
   sprtf( " Flag = [%s]"MEOR, lpd );
}

VOID  Show_Section_Headers( PIMAGE_SECTION_HEADER pis, LPDFSTR lpdf,
                           PIMAGE_NT_HEADERS32 pnt)
{
   LPTSTR lpd = &g_cBuf[0];
   DWORD dwi;
   DWORD offset, size;
   PBYTE pbase = lpdf->df_pVoid;
   DWORD max   = lpdf->dwmax;
   PBYTE pb;
   DWORD dwh;

   sprtf( "Display of %d Section Headers ...\n", g_dwNumOfSections );
   for( dwi = 0; dwi < g_dwNumOfSections; dwi++ )
   {
      offset = pis->PointerToRawData;
      size   = pis->SizeOfRawData;
      OutSectionStruct( pis );
      if( offset && size ) {
         pb = pbase + offset;
         if( offset < max ) {
            if( (offset + size) >= max ) {
               sprtf( " Offset+Size OUTSIDE address space! (%u on %u)"MEOR, offset+size, max );
               size = max - offset;
            }
            if(VERB5) {
               while( size ) {
                  if( size > 16 )
                     dwh = 16;
                  else
                     dwh = size;
                  *lpd = 0;
                  GetHEXString( lpd, pb, dwh, pbase, TRUE );
                  AddStringandAdjust(lpd);
                  pb   += dwh;    // bump the offset
                  size -= dwh;    // reduce remaining count
               }
            } else {
               sprtf( "Need -v5, or better, to show HEX data ...\n" );
            }
         } else {
            sprtf( " Offset OUTSIDE address space! (%u on %u)"MEOR, offset, max );
         }
      }

      pis++;
   }
   sprtf( "Done %d Section Headers ...\n", g_dwNumOfSections );
}

typedef struct tagIMPLIST {
   LE link;
   TCHAR dllname[264];
   DWORD ordinal;
   TCHAR function[264];
}IMPLIST, * PIMPLIST;

LE implist = { &implist, &implist };
static IMPLIST ilist;
VOID  Add2List( PIMPLIST pil )
{
   PIMPLIST pi = dMALLOC(sizeof(IMPLIST));
   CHKMEM(pi);
   memcpy(pi, pil, sizeof(IMPLIST));
   InsertTailList(&implist,(PLE)pi);
}
VOID  FreeImpList( VOID )
{
   KillLList((PLE)&implist);
}

VOID  ShowImpList( VOID )
{
   int   icnt;
   PLE   ph = &implist;
   PLE   pn;
   ListCount2(ph,&icnt);
   if(icnt) {
      sprtf( "Display of %d entries in IMPORT list ..."MEOR, icnt );
      ilist.dllname[0] = 0;
      Traverse_List(ph,pn)
      {
         PIMPLIST pi = (PIMPLIST)pn;
         if( strcmp( ilist.dllname, pi->dllname ) )
         {
            strcpy( ilist.dllname, pi->dllname );
            sprtf( "From DLL [%s] ..."MEOR, pi->dllname );
         }
         if( pi->function[0] )
            sprtf( " [%s] %d"MEOR, pi->function, pi->ordinal );
         else
            sprtf( " Ordinal %d"MEOR, pi->ordinal );
      }
   } else {
      sprtf( "No entries in IMPORT list ..."MEOR );
   }
}

PBYTE Word2Ascii( PBYTE pb )
{
   static BYTE wordhdr[3];
   int   i;
   for(i = 0; i < 2; i++) {
      wordhdr[i] = pb[i];
   }
   wordhdr[i] = 0;
   return wordhdr;
}

VOID  Show_HEX_Block( PBYTE pb, DWORD len, PBYTE pbase )
{
   LPTSTR lpd = g_cBuf;
   DWORD dwi;
   DWORD dwo = len;
   while( dwo ) {
      if( dwo > 16 )
         dwi = 16;
      else
         dwi = dwo;
      *lpd = 0;
      GetHEXString( lpd, pb, dwi, pbase, TRUE );
      AddStringandAdjust(lpd);
      pb  += dwi;    // bump the offset
      dwo -= dwi;    // reduce remaining count
   }
}

//typedef enum tagProcType {
//   pt_Bo,
//   pt_16,
//   pt_32
//}ProcType;
static MWL  _s_mwl;

VOID  More_DOS_Stuff( PIMAGE_DOS_HEADER pdh, LPDFSTR lpdf )
{
   LPTSTR lpd = g_cBuf;
   DWORD   iMax, iOff, iLen;
   DWORD   iSize = lpdf->dwmax;
   ProcType typ = pt_16;
      // assuming a paragraph is 16 bytes, and
      //    WORD   e_cparhdr; // Size of header in paragraphs
      // iOff = pdh->e_cparhdr * 16; // appears to be 1 para short???

   if( pdh->e_cparhdr ) {
      iOff = (pdh->e_cparhdr + 1) * 16;
   } else {
      iOff = sizeof(IMAGE_DOS_HEADER);
   }

   iLen = pdh->e_lfanew; // hex up to offset to NEW HEADER
   if( iLen ) {
      if( pdh->e_cparhdr ) {
         iOff = pdh->e_cparhdr * 16;
      }
   } else { // if( iLen == 0 ) { // if NO NEW HEADER, then
      iLen = iSize; // use the WHOLE file size
   }
   if( iOff == 0 ) {
      iOff = pdh->e_ip + sizeof(IMAGE_DOS_HEADER);
   }
   if( iOff < iLen ) {
      iMax = iLen - iOff;
   } else if( iOff < iSize) {
      iMax = iSize - iOff;
   } else {
      iMax = 0;
   }
   sprintf(lpd, "; Offset to DOS CODE = %#x (%d), for %d bytes ...",
      iOff, iOff, iMax );
   AddStringandAdjust( lpd );

   ZeroMemory( &_s_mwl, sizeof(MWL) );
   if( iMax ) {
      DumpCode( &_s_mwl, (PBYTE)pdh, 0, (PBYTE)pdh + iOff, iMax, 0, typ );
   }

}

VOID  Show_DOS_Header( PIMAGE_DOS_HEADER pdos, LPDFSTR lpdf )
{
// typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
   LPTSTR lpd = g_cBuf;
   PBYTE pb = Word2Ascii( (PBYTE) pdos );
   sprtf( "Magic number:                     %u (%s)"MEOR, (pdos->e_magic & 0xffff), pb);
   OUTIT( "Bytes on last page of file:       %u (%#x)"MEOR, e_cblp);
   OUTIT( "Pages in file:                    %u (%#x)"MEOR, e_cp);
   OUTIT( "Relocations:                      %u (%#x)"MEOR, e_crlc);
   OUTIT( "Size of header in paragraphs:     %u (%#x)"MEOR, e_cparhdr);
   OUTIT( "Minimum extra paragraphs needed:  %u (%#x)"MEOR, e_minalloc);
   OUTIT( "Maximum extra paragraphs needed:  %u (%#x)"MEOR, e_maxalloc);
   OUTIT( "Initial (relative) SS value:      %u (%#x)"MEOR, e_ss );                        
   OUTIT( "Initial SP value:                 %u (%#x)"MEOR, e_sp );
   OUTIT( "Checksum:                         %u (%#x)"MEOR, e_csum );
   OUTIT( "Initial IP value:                 %u (%#x)"MEOR, e_ip );
   OUTIT( "Initial (relative) CS value:      %u (%#x)"MEOR, e_cs );
   OUTIT( "File address of relocation table: %u (%#x)"MEOR, e_lfarlc );
   OUTIT( "Overlay number:                   %u (%#x)"MEOR, e_ovno );
   // Reserved words WORD   e_res[4];                    
   OUTIT( "OEM identifier (for e_oeminfo):   %u (%#x)"MEOR, e_oemid );
   OUTIT( "OEM information; e_oemid specific %u (%#x)"MEOR, e_oeminfo );
   // Reserved words WORD   e_res2[10];                  
   sprtf( "File address of new exe header:   %lu (%#x)"MEOR, pdos->e_lfanew, pdos->e_lfanew );
   if( pdos->e_lfanew && VERB9 ) {
      sprtf("HEX display up to new header ...(%u bytes)"MEOR, pdos->e_lfanew );
      Show_HEX_Block( (PBYTE)pdos, pdos->e_lfanew, (PBYTE)pdos ); 
   }
   if( VERB5 ) {
      More_DOS_Stuff( pdos, lpdf );
   } else {
      sprtf( "Need -v5 or better to see DOS code ..."MEOR );
   }
}

/* ==============================================
typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER {
    // Standard fields.
    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;
    // NT additional fields.
    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;
   ==================================================== */

DWORD GetData(PTSTR lpd, PBYTE pb, DWORD dwo, DWORD max)
{
   DWORD dwi = 0;
   while(*pb && (dwo < max)) {
      lpd[dwi++] = *pb;
      pb++;
      dwo++;
   }
   lpd[dwi] = 0;
   return dwi;
}
DWORD GetData2(PTSTR lpd, PBYTE pb, DWORD dwo, DWORD max)
{
   DWORD dwi = 0;
   PIMAGE_IMPORT_BY_NAME pibn = (PIMAGE_IMPORT_BY_NAME)pb;
   ilist.ordinal = (pibn->Hint & 0xffff);
   if(VERB9) sprtf( "     Ordinal(HINT) %d"MEOR, (pibn->Hint & 0xffff));
   dwi = GetData( lpd, (PBYTE)&pibn->Name, (dwo+2), max );
   if(dwi) {
      if(VERB9) sprtf( "     Name    %s"MEOR, lpd );
      if(dwi < 256) {
         strcpy( ilist.function, lpd );
      } else {
         strncpy( ilist.function, lpd, 256 );
         strcat( ilist.function, "..." );
      }
   } else {
      sprtf( "     NO Name found!"MEOR );
      strcpy( ilist.function, "No NAME Found!" );
   }
   Add2List( &ilist );
   return dwi;
}

/* ==================================
IMAGE_DATA_DIRECTORY

The IMAGE_DATA_DIRECTORY structure represents the data directory.


typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
Members
VirtualAddress - Relative virtual address of the table. 
Size - Size of the table, in bytes. 
Remarks
The following is a list of the data directories. Offsets are relative to the
beginning of the optional header.

Offset Description 
96 Export table address and size 
104 Import table address and size 
112 Resource table address and size 
120 Exception table address and size 
128 Certificate table address and size 
136 Base relocation table address and size 
144 Debugging information starting address and size 
152 Architecture-specific data address and size 
160 Global pointer register relative virtual address 
168 Thread local storage (TLS) table address and size 
176 Load configuration table address and size 
184 Bound import table address and size 
192 Import address table address and size 
200 Delay import descriptor address and size 
208 Reserved 

DbgHlp Functions
Image Access

The image access functions access the data in an executable image. The
functions provide high-level access to the base of images and very specific
access to the most common parts of an image's data.

GetTimestampForLoadedLibrary
ImageDirectoryEntryToData
ImageDirectoryEntryToDataEx
ImageNtHeader
ImageRvaToSection
ImageRvaToVa

The IMAGE_SECTION_HEADER structure represents the image section header format.

typedef struct _IMAGE_SECTION_HEADER {
   BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
   union {
      DWORD PhysicalAddress;
      DWORD VirtualSize;
   } Misc;
   DWORD VirtualAddress;
   DWORD SizeOfRawData;
   DWORD PointerToRawData;
   DWORD PointerToRelocations;
   DWORD PointerToLinenumbers;
   WORD NumberOfRelocations;
   WORD NumberOfLinenumbers;
   DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;
Members
Name 
An 8-byte, null-filled string. There is no terminating null character if
the string is exactly eight characters long. For longer names, this member
contains a forward slash (/) followed by a decimal number that is an offset
into the string table. Executable images do not use a string table and do
not support section names longer than eight characters. 
Misc 
PhysicalAddress 
File address. 
VirtualSize 
Total size of the section when loaded into memory, in bytes. If this value
is greater than the SizeOfRawData member, the section is filled with zeroes. 
VirtualAddress 
Address of the first byte of the section when loaded into memory, relative
to the image base. 
SizeOfRawData 
Size of the initialized data on disk, in bytes. This value must be a
multiple of the FileAlignment member of the IMAGE_OPTIONAL_HEADER
structure. If this value is less than the VirtualSize member, the
remainder of the section is filled with zeroes. If the section contains
only uninitialized data, the member is zero. 
PointerToRawData 
File pointer to the first page within the COFF file. This value must be
a multiple of the FileAlignment member of the IMAGE_OPTIONAL_HEADER
structure. If a section contains only uninitialized data, this member is zero. 
PointerToRelocations 
File pointer to the beginning of the relocation entries for the section. If
there are no relocations, this value is zero. 
PointerToLinenumbers 
File pointer to the beginning of the line-number entries for the
section. If there are no COFF line numbers, this value is zero. 
NumberOfRelocations 
Number of relocation entries for the section. This value is zero
for executable images. 
NumberOfLinenumbers 
Number of line-number entries for the section. 
Characteristics 
Characteristics of the image. The following values are defined. Flag Meaning 
IMAGE_SCN_TYPE_REG    0x00000000 Reserved. 
IMAGE_SCN_TYPE_DSECT  0x00000001 Reserved. 
IMAGE_SCN_TYPE_NOLOAD 0x00000002 Reserved. 
IMAGE_SCN_TYPE_GROUP  0x00000004 Reserved. 
IMAGE_SCN_TYPE_NO_PAD 0x00000008 Reserved. 
IMAGE_SCN_TYPE_COPY   0x00000010 Reserved. 
IMAGE_SCN_CNT_CODE    0x00000020 Section contains executable code. 
IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040 Section contains initialized data. 
IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 Section contains uninitialized data. 
IMAGE_SCN_LNK_OTHER   0x00000100 Reserved. 
IMAGE_SCN_LNK_INFO    0x00000200 Reserved. 
IMAGE_SCN_TYPE_OVER   0x00000400 Reserved. 
IMAGE_SCN_LNK_COMDAT  0x00001000 Section contains COMDAT data. 
IMAGE_SCN_MEM_FARDATA 0x00008000 Reserved. 
IMAGE_SCN_MEM_PURGEABLE 0x00020000 Reserved. 
IMAGE_SCN_MEM_16BIT   0x00020000 Reserved. 
IMAGE_SCN_MEM_LOCKED  0x00040000 Reserved. 
IMAGE_SCN_MEM_PRELOAD 0x00080000 Reserved. 
IMAGE_SCN_ALIGN_1BYTES 0x00100000 Align data on a 1-byte boundary. 
IMAGE_SCN_ALIGN_2BYTES 0x00200000 Align data on a 2-byte boundary. 
IMAGE_SCN_ALIGN_4BYTES 0x00300000 Align data on a 4-byte boundary. 
IMAGE_SCN_ALIGN_8BYTES 0x00400000 Align data on a 8-byte boundary. 
IMAGE_SCN_ALIGN_16BYTES 0x00500000 Align data on a 16-byte boundary. 
IMAGE_SCN_ALIGN_32BYTES 0x00600000 Align data on a 32-byte boundary. 
IMAGE_SCN_ALIGN_64BYTES 0x00700000 Align data on a 64-byte boundary. 
IMAGE_SCN_ALIGN_128BYTES 0x00800000 Align data on a 128-byte boundary. 
IMAGE_SCN_ALIGN_256BYTES 0x00900000 Align data on a 256-byte boundary. 
IMAGE_SCN_ALIGN_512BYTES 0x00A00000 Align data on a 512-byte boundary. 
IMAGE_SCN_ALIGN_1024BYTES 0x00B00000 Align data on a 1024-byte boundary. 
IMAGE_SCN_ALIGN_2048BYTES 0x00C00000 Align data on a 2048-byte boundary. 
IMAGE_SCN_ALIGN_4096BYTES 0x00D00000 Align data on a 4096-byte boundary. 
IMAGE_SCN_ALIGN_8192BYTES 0x00E00000 Align data on a 8192-byte boundary. 
IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000 Section contains extended relocations. 
IMAGE_SCN_MEM_DISCARDABLE 0x02000000 Section can be discarded as needed. 
IMAGE_SCN_MEM_NOT_CACHED  0x04000000 Section cannot be cached. 
IMAGE_SCN_MEM_NOT_PAGED   0x08000000 Section cannot be paged. 
IMAGE_SCN_MEM_SHARED      0x10000000 Section can be shared in memory. 
IMAGE_SCN_MEM_EXECUTE     0x20000000 Section can be executed as code. 
IMAGE_SCN_MEM_READ        0x40000000 Section can be read. 
IMAGE_SCN_MEM_WRITE       0x80000000 Section can be written to. 

Also see : http://msdn.microsoft.com/en-us/library/ms809762.aspx
PEDUMP [switches] filename
  
   ================================== */

#define  SHWW(a,b)   sprtf(a, (poh->b & 0xffff))
#define  SHWB(a,b)   sprtf(a, (poh->b & 0xff))
#define  SHWDW(a,b)  sprtf(a, poh->b)

VOID  Show_Optional_Header(PIMAGE_OPTIONAL_HEADER32 poh, PBYTE pHead, LPDFSTR lpdf,
                           PIMAGE_NT_HEADERS32 pnt)
{
// Directory format.
// typedef struct _IMAGE_DATA_DIRECTORY {
//    DWORD   VirtualAddress;
//    DWORD   Size;
//} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
   PIMAGE_DATA_DIRECTORY pidd;
   int   i, j;
   DWORD off, size;
   LPTSTR lpd = &g_cBuf[0];
   PIMAGE_SECTION_HEADER pis;
   PIMAGE_IMPORT_DESCRIPTOR piid;
   PIMAGE_THUNK_DATA pitd;
   PIMAGE_SECTION_HEADER psh;
   DWORD Ordinal;
   DWORD toff, tdiff;
   PBYTE pbtmp;
   PBYTE pb = (PBYTE)poh;  // pointer to BEGINNING of OPTIONAL HEADERS

   sprtf( "Optional Header: size = %d"MEOR, sizeof(IMAGE_OPTIONAL_HEADER32) );
   SHWW( "Magic:                      %#x"MEOR, Magic );
   SHWB( "MajorLinkerVersion:         %u"MEOR, MajorLinkerVersion );
   SHWB( "MinorLinterVersion:         %u"MEOR, MinorLinkerVersion );
   SHWDW("SizeOfCode:                 %u"MEOR, SizeOfCode );
   SHWDW("SizeOfInitializedData       %u"MEOR, SizeOfInitializedData );
   SHWDW("SizeOfUninitializedData     %u"MEOR, SizeOfUninitializedData );
   SHWDW("AddressOfEntryPoint         %u"MEOR, AddressOfEntryPoint );
   SHWDW("BaseOfCode                  %u"MEOR, BaseOfCode );
   SHWDW("BaseOfData                  %u"MEOR, BaseOfData );
   // NT additional fields.
   SHWDW("ImageBase                   %u"MEOR, ImageBase );
   SHWDW("SectionAlignment            %u"MEOR, SectionAlignment );
   SHWDW("FileAlignment               %u"MEOR, FileAlignment );
   SHWW( "MajorOperatingSystemVersion %u"MEOR, MajorOperatingSystemVersion );
   SHWW( "MinorOperatingSystemVersion %u"MEOR, MinorOperatingSystemVersion );
   SHWW( "MajorImageVersion           %u"MEOR, MajorImageVersion );
   SHWW( "MinorImageVersion           %u"MEOR, MinorImageVersion );
   SHWW( "MajorSubsystemVersion       %u"MEOR, MajorSubsystemVersion );
   SHWW( "MinorSubsystemVersion       %u"MEOR, MinorSubsystemVersion );
   SHWDW("Win32VersionValue           %u"MEOR, Win32VersionValue );
   SHWDW("SizeOfImage                 %u"MEOR, SizeOfImage );
   SHWDW("SizeOfHeaders               %u"MEOR, SizeOfHeaders );
   SHWDW("CheckSum                    %u"MEOR, CheckSum );
   SHWW( "Subsystem                   %u"MEOR, Subsystem );
   SHWW( "DllCharacteristics          %u"MEOR, DllCharacteristics );
   SHWDW("SizeOfStackReserve          %u"MEOR, SizeOfStackReserve );
   SHWDW("SizeOfStackCommit           %u"MEOR, SizeOfStackCommit );
   SHWDW("SizeOfHeapReserve           %u"MEOR, SizeOfHeapReserve );
   SHWDW("SizeOfHeapCommit            %u"MEOR, SizeOfHeapCommit );
   SHWDW("LoaderFlags                 %u"MEOR, LoaderFlags );
   SHWDW("NumberOfRvaAndSizes         %u"MEOR, NumberOfRvaAndSizes );

   sprtf( "Display of %d Section Headers ...\n", g_dwNumOfSections );
   /*    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]; */
   for( i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++ )
   {
      PTSTR pstr = "NOT CASED";
      toff = 0;
      switch(i)
      {
      case 0:
         toff = 96;  // Export table address and size 
         pstr = "EXPORT TABLE";
         break;
      case 1:  // .idata section IMAGE_IMPORT_DESCRIPTORS array
         toff = 104; // Import table address and size 
         pstr = "IMPORT TABLE";
         break;
      case 2:
         toff = 112; // Resource table address and size 
         pstr = "Resource Directory";
         break;
      case 3:
         toff = 120; // Exception table address and size 
         pstr = "Exception Directorys";
         break;
      case 4:
         toff = 128; // Certificate table address and size 
         pstr = "Security Directory";
         break;
      case 5:
         toff = 136; // Base relocation table address and size 
         pstr = "Base Relocation Table";
         break;
      case 6:
         toff = 144; // Debugging information starting address and size 
         pstr = "Debug Directory";
         break;
      case 7:
         toff = 152; // Architecture-specific data address and size 
         pstr = "Copyright String";
         break;
      case 8:
         toff = 160; // Global pointer register relative virtual address 
         pstr = "Global Pointer";
         break;
      case 9:
         toff = 168; // Thread local storage (TLS) table address and size 
         pstr = "TLS Directory";
         break;
      case 10:
         toff = 176; // Load configuration table address and size 
         pstr = "Load Configuration Directory";
         break;
      case 11:
         toff = 184; // Bound import table address and size 
         pstr = "Bound Import Directory (NIU)";
         break;
      case 12:
         toff = 192; // Import address table address and size 
         pstr = "Import Address Table (IAT) (NIU)";
         break;
      case 13:
         toff = 200; // Delay import descriptor address and size 
         pstr = "Delay Import (NIU)";
         break;
      case 14:
         toff = 208; // Reserved 
         pstr = "COM Runtime Descriptor (NIU)";
         break;
      case 15:
         pstr = "Not In Use";
         break;
      }
      pidd = &poh->DataDirectory[i];
      pbtmp = (PBYTE)pidd;
      tdiff = pbtmp - pb;  // get the OFFSET
      if( tdiff == toff ) {
         // this is correct
         tdiff = 0;
      } else {
         toff = tdiff;
      }
      off = pidd->VirtualAddress;
      size = pidd->Size;
      sprtf("%2d: %s: Virtual Address [%#x] Size = %u"MEOR, (i+1),
         pstr,
         off, size );
      // PIMAGE_SECTION_HEADER
      psh = ImageRvaToSection( pnt, // PIMAGE_NT_HEADERS NtHeaders,
         0, // PVOID Base,
         pidd->VirtualAddress );
      if(psh) {
         // got SECTION HEADER
         // Name 
         // An 8-byte, null-filled string. There is no terminating null character if
         // the string is exactly eight characters long. For longer names, this member
         // contains a forward slash (/) followed by a decimal number that is an offset
         // into the string table. Executable images do not use a string table and do
         // not support section names longer than eight characters.
         pbtmp = (PBYTE)&psh->Name;
         if( *pbtmp == '/' ) {
            sprintf(lpd, "Name %d offset into string table.", atoi(&pbtmp[1]) );
         } else {
            for( j = 0; j < IMAGE_SIZEOF_SHORT_NAME; j++ )
            {
               lpd[j] = pbtmp[j];
               if(pbtmp[j] == 0)
                  break;
            }
         }
         sprtf( "%2d: SECTION Name: %s."MEOR, i + 1, lpd );
         OutSectionStruct( psh );
         // PIMAGE_SECTION_HEADER
         pbtmp = ImageRvaToVa( pnt, // PIMAGE_NT_HEADERS NtHeaders,
            pHead, // PVOID Base,
            psh->VirtualAddress,
            NULL );
         if(pbtmp) {
            *lpd = 0;
            GetHEXString( lpd, pbtmp, 16, pHead, TRUE );
            sprtf("%s"MEOR, lpd );
         }
      } else {
         // guess where it is - and this is errant math
         if( off && size && ((off + size) < lpdf->dwmax) ) {
            PBYTE pb = pHead + off;
            DWORD dwi;
            DWORD dwo = size;
            while( dwo ) {
               if( dwo > 16 )
                  dwi = 16;
               else
                  dwi = dwo;
             *lpd = 0;
             GetHEXString( lpd, pb, dwi, pHead, TRUE );
             AddStringandAdjust(lpd);
             pb  += dwi;    // bump the offset
             dwo -= dwi;    // reduce remaining count
            }
            if( i == 1 ) {
               pb = pHead + off;
               piid = (PIMAGE_IMPORT_DESCRIPTOR)pb;
               while( piid->Characteristics ) {
                  if(VERB9) {
                     sprtf( "   Thunk Pointers   %#x (%#x)"MEOR,
                        piid->Characteristics, piid->FirstThunk );
                  }
                  dwo = piid->Name;
                  if( dwo < lpdf->dwmax ) {
                     pb = pHead + dwo;
                     *lpd = 0;
                     dwi = GetData(lpd, pb, dwo, lpdf->dwmax);
                     if(dwi) {
                        lpd[dwi] = 0;
                        if(VERB9) sprtf("   Import from [%s]"MEOR, lpd );
                        if(dwi < 256) {
                           strcpy( ilist.dllname, lpd );
                        } else {
                           strncpy( ilist.dllname, lpd, 256 );
                           strcat( ilist.dllname, "..." );
                        }
                     } else {
                        sprtf("   Did not find a string!"MEOR);
                        strcpy( ilist.dllname, "No NAME!" );
                     }
                  } else {
                     sprtf( "   Name (OUT OF SPACE)%u"MEOR, piid->Name );
                  }
                  dwo = piid->Characteristics;
                  if( dwo < lpdf->dwmax ) {
                     pb = pHead + dwo;
                     pitd = (PIMAGE_THUNK_DATA)pb;
                     dwo = pitd->u1.AddressOfData;
                     while(dwo) {
                        // process ARRAY of THUNK DATA
                        if( dwo & 0x80000000 ) {
                           Ordinal = (dwo & ~(0x80000000));
                           if(VERB9) sprtf( "    Ordinal  %u"MEOR, Ordinal );
                           ilist.ordinal = Ordinal;
                           ilist.function[0] = 0;
                           Add2List( &ilist );
                        } else if( dwo < lpdf->dwmax ) {
                           *lpd = 0;
                           pb = pHead + dwo;
                           dwi = GetData2(lpd, pb, dwo, lpdf->dwmax);
                        } else {
                           sprtf( "    RVA OUT OF SPACE! %u (%u)"MEOR, dwo, lpdf->dwmax );
                        }
                        pitd++;
                        dwo = pitd->u1.AddressOfData;
                     }
                  }
                  dwo = piid->FirstThunk;
                  piid++;
               }
            }
         } else if( off && size ) {
            if( off > lpdf->dwmax ) {
               sprtf("Offset %u appears OUTSIDE address space! (%u)"MEOR, off, lpdf->dwmax );
            } else {
               sprtf("Offset %u in address space, but size &u > max%u! "MEOR, off, size, lpdf->dwmax );
            }
         }
      }

   }
   pis = (PIMAGE_SECTION_HEADER)&poh->DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
   if(VERB9) {
      Show_Section_Headers( pis, lpdf, pnt );
   }
}

VOID Show_NT_Header( PIMAGE_NT_HEADERS32 pnt, PBYTE pHead, LPDFSTR lpdf )
{
//typedef struct _IMAGE_NT_HEADERS {
//    DWORD Signature;
//    IMAGE_FILE_HEADER FileHeader;
//    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
//} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;
   PIMAGE_FILE_HEADER pifh = &pnt->FileHeader;
   PIMAGE_OPTIONAL_HEADER32 poh = &pnt->OptionalHeader;
   PBYTE pSym = 0;
   PTSTR ptmp = g_cBuf;

   pPEHeader = pifh; // make this globally available
   pSym = (PBYTE)&pnt->Signature;
   ptmp[0] = pSym[0];
   ptmp[1] = pSym[1];
   ptmp[2] = 0;
   pSym = 0;
   sprtf( "NT Signature:       %#x (%s)"MEOR, pnt->Signature, ptmp );
   Show_Target_Machine( pifh->Machine & 0xffff );
   g_dwNumOfSections = (pifh->NumberOfSections & 0xffff);
   Show_Number_Sections( g_dwNumOfSections );
   Show_DateTime_Stamp( pifh->TimeDateStamp );
   Show_Symbols_Count( pifh->NumberOfSymbols, pifh->PointerToSymbolTable, pHead, &pSym );
   Show_OH_Size( pifh->SizeOfOptionalHeader & 0xffff );
   Show_Characteristics( pifh->Characteristics & 0xffff );
   Show_Optional_Header(poh, pHead, lpdf, pnt);
}

int  IsEXEFile( LPDFSTR lpdf )
{
   int  bRet = 0;
   PBYTE pb = lpdf->df_pVoid;
   DWORD msz = lpdf->dwmax;
   WORD * pw;
   if( pb && (msz > sizeof(IMAGE_DOS_HEADER)) ) {
      PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)pb;
      if(( pdos->e_magic == IMAGE_DOS_SIGNATURE ) &&
         ( pdos->e_lfanew + sizeof(IMAGE_NT_HEADERS32) < msz )) {
         PIMAGE_NT_HEADERS32 pnt = (PIMAGE_NT_HEADERS32)(pb + pdos->e_lfanew);
         PIMAGE_OS2_HEADER pos2 = (PIMAGE_OS2_HEADER)pnt;
         pw = (WORD *)pnt;
         if(VERB9) sprtf( "Found DOS signature, checking NT (%#x) ..."MEOR, (pnt->Signature & 0xffff) );
         if(( pnt->Signature == IMAGE_NT_SIGNATURE )||
            ( *pw == IMAGE_NT_SIGNATURE ) ) {
            PIMAGE_FILE_HEADER pifh = &pnt->FileHeader;
            PIMAGE_OPTIONAL_HEADER32 poh = &pnt->OptionalHeader;
            bRet = 1;
         } else if(( pos2->ne_magic == IMAGE_OS2_SIGNATURE )||
                   ( *pw == IMAGE_OS2_SIGNATURE ) ) {
            sprtf("Has OS/2 (NE) signature ...(%#x)"MEOR, (pos2->ne_magic & 0xffff) );
            bRet = 2;

         }
      }
   }
   return bRet;
}

// Value Meaning 
typedef struct tagHDRTYPES {
   USHORT   uType;
   PTSTR  pszDesc;
}HDRTYPES, * PHDRTYPES;

HDRTYPES sTypeList[] = {
   { IMAGE_DIRECTORY_ENTRY_ARCHITECTURE, "7 Architecture-specific data" },
   { IMAGE_DIRECTORY_ENTRY_BASERELOC, "5 Base relocation table" },
   { IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, "11 Bound import directory" },
   { IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, "14 COM descriptor table" },
   { IMAGE_DIRECTORY_ENTRY_DEBUG, "6 Debug directory" },
   { IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, "13 Delay import table" },
   { IMAGE_DIRECTORY_ENTRY_EXCEPTION, "3 Exception directory" },
   { IMAGE_DIRECTORY_ENTRY_EXPORT, "0 Export directory" },
   { IMAGE_DIRECTORY_ENTRY_GLOBALPTR, "8 Relative virtual address of global pointer" },
   { IMAGE_DIRECTORY_ENTRY_IAT, "12 Import address table" },
   { IMAGE_DIRECTORY_ENTRY_IMPORT, "1 Import directory" },
   { IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, "10 Load configuration directory" },
   { IMAGE_DIRECTORY_ENTRY_RESOURCE, "2 Resource directory" },
   { IMAGE_DIRECTORY_ENTRY_SECURITY, "4 Security directory" },
   { IMAGE_DIRECTORY_ENTRY_TLS, "9 Thread local storage directory" },
   { 0, 0 }
};

void New_Dbg_Helper( LPDFSTR lpdf )
{
   PBYTE pb;
   PHDRTYPES pht = &sTypeList[0];
   PIMAGE_SECTION_HEADER psh;
   ULONG Size;
   DWORD dwi;
   LPTSTR lpd = &g_cBuf[0];
   PBYTE pHead = (PBYTE)lpdf->df_pVoid;
   PBYTE pbtmp;
   while( pht->pszDesc )
   {
      Size = 0;
      psh = NULL;
      pb = ImageDirectoryEntryToDataEx(
         lpdf->df_pVoid,   // PVOID Base,
         FALSE,            // BOOLEAN MappedAsImage,
         pht->uType,       // USHORT DirectoryEntry,
         &Size,            // PULONG Size,
         &psh );           // PIMAGE_SECTION_HEADER* FoundHeader
      if(pb) {
         sprtf( "FOUND %s ... Size = %d, %s"MEOR, pht->pszDesc, Size,
            (psh ? "with section header." : "No section header") );
         if(psh) {
            OutSectionStruct( psh );
         }
         pbtmp = pb;
         while(Size)
         {
            if(Size < 16)
               dwi = Size;
            else
               dwi = 16;
            *lpd = 0;
            GetHEXString( lpd, pbtmp, dwi, pHead, TRUE );
            sprtf("%s"MEOR, lpd);
            Size -= dwi;
            pbtmp = &pbtmp[dwi];
         }
      } else {
         sprtf( "No %s ..."MEOR, pht->pszDesc );
      }

      pht++;
   }
}

BOOL  DumpEXEFile( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   PBYTE pb = lpdf->df_pVoid;
   DWORD msz = lpdf->dwmax;
   PIMAGE_NT_HEADERS pnth = ImageNtHeader(lpdf->df_pVoid);
   if( pb && (msz > sizeof(IMAGE_DOS_HEADER)) ) {
      PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)pb;
      PIMAGE_NT_HEADERS32 pnt = (PIMAGE_NT_HEADERS32)(pb + pdos->e_lfanew);
      PIMAGE_FILE_HEADER pifh = &pnt->FileHeader;
      PIMAGE_OPTIONAL_HEADER32 poh = &pnt->OptionalHeader;
      sprtf("Found DOS header ..."MEOR);
      Show_DOS_Header( pdos, lpdf );
      sprtf("Found NT header ...Offset %d ..."MEOR,
         (pnth ? ((PBYTE)pnth - pb) : pdos->e_lfanew) );
      g_pNTHeaders = pnt;  // make globally available
      Show_NT_Header( pnt, pb, lpdf );
      ShowImpList();
      New_Dbg_Helper(lpdf);
      bRet = TRUE;
   }

   FreeImpList();

   return bRet;
}

#endif // #ifndef USE_PEDUMP_CODE // FIX20080507
// EOF - DumpEXE.c

