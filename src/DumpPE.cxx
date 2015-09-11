// DumpPE.c - from PEDUMP ...
// ;# see : http://msdn.microsoft.com/en-us/windows/hardware/gg463119.aspx
/* =====================================================================================
    20140614:
    from : http://msdn.microsoft.com/library/windows/hardware/gg463119.aspx
    DWN: 14/06/2014  17:43 217,765 pecoff_v83.docx
    14 Archive (Library) File Format
    The first 8 bytes of an archive consist of the file signature.
    The rest of the archive consists of a series of archive members, as follows:
    The first and second members are linker members. Each of these members has its own format as described in section 8.3, Import Name Type.
    the general structure of an archive.
    * Signature :!<arch>\n
    * Header
    * 1st Linker Member
    * Header
    * 2nd Linker Member
    * Header
    * Longnames Member
    * Header
    * Contents of OBJ File 1 (COFF format)
    * Header
    * Contents of OBJ File 2 (COFF format)
    * Header
    * Contents of OBJ File N (COFF format)
    The Header
    Off Len Field Desc
    0   16  Name  The name of the archive member, with a slash (/) appended to terminate the name. 
                    If the first character is a slash, the name has a special interpretation, as described 
                    in the following table.
    16  12  Date  The date and time that the archive member was created: This is the ASCII decimal 
                    representation of the number of seconds since 1/1/1970 UCT.
    28  6   User ID An ASCII decimal representation of the user ID. This field does not contain a meaningful 
                    value on Windows platforms because Microsoft tools emit all blanks.
    34  6   Group ID An ASCII decimal representation of the group ID. This field does not contain a meaningful 
                    value on Windows platforms because Microsoft tools emit all blanks.
    40  8   Mode  An ASCII octal representation of the member’s file mode. This is the ST_MODE value from the 
                    C run-time function _wstat.
    48  10  Size  An ASCII decimal representation of the total size of the archive member, not including the 
                    size of the header.
    58  2   End of Header The two bytes in the C string “‘\n” (0x60 0x0A).
    ====================================================================================
    See also: https://msdn.microsoft.com/en-us/library/windows/hardware/gg463119.aspx
    DWN: 14/06/2014  16:43           217,765 pecoff_v83.docx

   ===================================================================================== */
#ifndef _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES
#endif 
#include "Dump4.h"
#include <math.h>

#ifdef   USE_PEDUMP_CODE // FIX20080507

#include <time.h>
#include "DumpCOFF.h"
#ifdef WIN32
#include <Dbghelp.h>
#endif
#include <algorithm> // for std::sort
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <ostream>

using namespace std;

#include "DumpArch.h"

#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif


#if defined(IMAGE_SIZEOF_ROM_OPTIONAL_HEADER)
#define ADD_DUMP_ROM_IMAGE
#else
#undef ADD_DUMP_ROM_IMAGE
#endif

using std::vector;

// forward refs
PSTR GetMachineTypeName( WORD wMachineType );


typedef std::vector<WORD> vWORD;    // added 20140614

typedef vector<PBYTE>   vPTRLIST;
typedef vector<std::string> vSTR;   // FIX20130225
typedef vSTR::iterator vSTRi;

vPTRLIST    vPtrList;
vSTR vDllList;  // FIX20130225
vSTR vExports;  // FIX20130807
vSTR vObjects;
vWORD vMachineTypes;    // FIX20140614

static int skipped_objects = 0;
void add_to_vector_str( std::string &data, vSTR *pv )
{
    std::string s;
    for ( vSTRi ii = pv->begin(); ii != pv->end(); ii++ ) {
        s = *ii;
        if (strcmp( s.c_str(), data.c_str() ) == 0)
            return;
    }
    pv->push_back(data);
}

void add_to_objects( std::string &data ) { add_to_vector_str( data, &vObjects ); }

void add_to_exports( std::string &data )
{
    std::string s;
    for ( vSTRi ii = vExports.begin(); ii != vExports.end(); ii++ ) {
        s = *ii;
        if (strcmp( s.c_str(), data.c_str() ) == 0)
            return;
    }
    vExports.push_back(data);
}
void kill_export_list() { vExports.clear(); }

void str_to_upper( std::string &data )
{
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
}

bool in_dll_list( std::string sDll )  // FIX20130225
{
    size_t max = vDllList.size();
    size_t ii;
    std::string su(sDll);
    str_to_upper(su);
    std::string s;
    for (ii = 0; ii < max; ii++) {
        s = vDllList[ii];
        str_to_upper(s);
        if ( s.compare(su) == 0 )
            return true;
    }
    return false;
}

void add_2_dll_list( char * pDLL )  // FIX20130225
{
    std::string s(pDLL);
    if (!in_dll_list(s))
        vDllList.push_back(s);
}

void kill_dll_list()  // FIX20130225
{
    vDllList.clear();
}

//int simp_comp_stg(const std::string & str1, const std::string &str2) { return str1.compare(str2); } 

bool stringCompare( const std::string &left, const std::string &right )
{
    for( std::string::const_iterator lit = left.begin(), rit = right.begin(); 
        lit != left.end() && rit != right.end(); ++lit, ++rit ) {
      if( tolower( *lit ) < tolower( *rit ) )
         return true;
      else if( tolower( *lit ) > tolower( *rit ) )
         return false;
    }
    if( left.size() < right.size() )
        return true;
    return false;
}

void show_sorted_stg_list( vSTR *pv )
{
    size_t max = pv->size();
    if (!max) return;

    size_t mwid = 100;
    size_t ii, len, sz;
    std::string s;
    std::sort(pv->begin(), pv->end(), stringCompare);
    len = 0;
    if (VERB5) {
        // each on a single line
        for (ii = 0; ii < max; ii++) {
            s = pv->at(ii);
            sprtf("%s\n",s.c_str()); // on its own line
        }
    } else {
        // output in lines
        for (ii = 0; ii < max; ii++) {
            s = pv->at(ii);
            sz = s.size();
            if (sz > mwid) {
                if (len)
                    sprtf("\n"); // close and previous
                sprtf("%s\n",s.c_str()); // and this
                len = 0;
            } else if ((len + sz) > mwid) {
                if (len)
                    sprtf("\n");  // close and previous
                sprtf("%s ",s.c_str()); // out this
                len = sz;   // and start size counter
            } else {
                sprtf("%s ",s.c_str()); // add this
                len += sz;  // and add to width
            }
        }
        if (max) {
            if (len)
                sprtf("\n\n"); // close current
            else
                sprtf("\n");
        }
    }
}

void show_dll_list()  // FIX20130225
{
    size_t max = vDllList.size();
    size_t mwid = 100;
    size_t ii, len, sz;
    std::string s;
    if (max) {
        sprtf("Total imports: %d, in case insensitive alphabetic order...\n", (int)max );
        std::sort(vDllList.begin(), vDllList.end(), stringCompare);
    }
    len = 0;

    for (ii = 0; ii < max; ii++) {
        s = vDllList[ii];
        sz = s.size();
        if (sz > mwid) {
            if (len)
                sprtf("\n"); // close and previous
            sprtf("%s\n",s.c_str()); // and this
            len = 0;
        } else if ((len + sz) > mwid) {
            if (len)
                sprtf("\n");  // close and previous
            sprtf("%s ",s.c_str()); // out this
            len = sz;   // and start size counter
        } else {
            sprtf("%s ",s.c_str()); // add this
            len += sz;  // and add to width
        }
    }
    if (max) {
        if (len)
            sprtf("\n\n"); // close current
        else
            sprtf("\n");
    }
    kill_dll_list();
}

void show_export_list()
{
    size_t mwid = 100;
    size_t ii, len, sz, off;
    std::string s;
    size_t max = vExports.size();
    std::string skip("??_");
    int skipped = 0;
    if (max) {
        sprtf("Total exports: %d, in case insensitive alphabetic order...\n", (int)max );
        std::sort(vExports.begin(), vExports.end(), stringCompare);
    }
    len = 0;
    for (ii = 0; ii < max; ii++) {
        s = vExports[ii];
        if (!VERB5) {
            off = s.find(skip);
            if (off != std::string::npos) {
                skipped++;
                continue;
            }
        }
        sz = s.size();
        if (sz > mwid) {
            if (len)
                sprtf("\n"); // close and previous
            sprtf("%s\n",s.c_str()); // and this
            len = 0;
        } else if ((len + sz) > mwid) {
            if (len)
                sprtf("\n");  // close and previous
            sprtf("%s ",s.c_str()); // out this
            len = sz;   // and start size counter
        } else {
            sprtf("%s ",s.c_str()); // add this
            len += sz;  // and add to width
        }
    }

    if (max && len) sprtf("\n"); // close current

    if (skipped)
        sprtf("Skipped showing %d exports of form '??_'. Use -v5 to show all\n", skipped);

    if (max) sprtf("\n");

    kill_export_list();
}

void show_obj_list()
{
    size_t max = vObjects.size();
    if (!max) return;
    sprtf("Total objects: %d, in case insensitive alphabetic order...(v=%d)\n", (int)max, giVerbose );
    show_sorted_stg_list( &vObjects );
    if (skipped_objects && !VERB9) 
        sprtf("Skipped %d objects. Use -v9 to view ALL\n", skipped_objects);
    vObjects.clear();

}


int add_2_ptrlist(PBYTE pb)
{
    size_t len = vPtrList.size();
    size_t i;
    PBYTE pt, ptp, ptm;
    for (i = 0; i < len; i++) {
        pt = vPtrList[i];
        ptp = pt + 8;
        ptm = pt - 8;
        if ((pb >= ptm)&&(pb <= ptp))
            return 1;
    }
    vPtrList.push_back(pb);
    return 0;
}

void kill_ptrlist() { vPtrList.clear(); }

PSTR g_ActPEFile = NULL;

LE defliblist = { &defliblist, &defliblist };
void add_2_lib_list(char * cp) {
    size_t len = strlen(cp);
    if (len) {
        PLE ph = &defliblist;
        PLE pn;
        char * pd;
        Traverse_List(ph,pn) {
            pd = (char *)pn;
            pd += sizeof(LE);
            if (strcmp(pd,cp) == 0)
                return; // already GOT IT
        }
        pn = (PLE)dMALLOC(sizeof(LE) + len + 2);
        pd = (char *)pn;
        pd += sizeof(LE);
        CHKMEM(pn);
        strcpy(pd,cp);
        InsertTailList(ph,pn);
    }
}

void show_lib_list() {
    PLE ph = &defliblist;
    int len;
    ListCount2(ph,&len);
    if (len) {
        PLE pn;
        char * pd;
        sprtf("Note: Found %d /DEFAULTLIB directives...\n",len);
        Traverse_List(ph,pn) {
            pd = (char *)pn;
            pd += sizeof(LE);
            sprtf("%s\n", pd );
        }
    }
}

void kill_lib_list()
{
    PLE ph = &defliblist;
    KillLList(ph);
}

// FIX20140614 - as development of x64 apps and libraries increases, is important to know the machine type
// =======================================================================================================
void add_2_machine_list( WORD mch )
{
    size_t max = vMachineTypes.size();
    size_t ii;
    WORD wd;
    for (ii = 0; ii < max; ii++) {
        wd = vMachineTypes[ii];
        if (mch == wd) {
            return;
        }
    }
    vMachineTypes.push_back(mch);
}
// FIX20140614
void show_machine_list()
{
    size_t max = vMachineTypes.size();
    size_t ii;
    WORD wd;
    if (max)
        sprtf("Machine Type: %d: ", (int)max);

    for (ii = 0; ii < max; ii++) {
        wd = vMachineTypes[ii];
        sprtf("%4X (%s) ", wd, GetMachineTypeName(wd));
    }
    if (max)
        sprtf("\n");
    vMachineTypes.clear();
}

// =========================================================
// DUMP EXE
//==================================
// PEDUMP - Matt Pietrek 1997
// FILE: EXEDUMP.C
//==================================
#ifdef DUMP4
#include "DumpPE.h"
#else 
#include "pedump.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#pragma hdrstop
#include "common.h"
#include "symboltablesupport.h"
#include "COFFSymbolTable.h"
#include "resdump.h"
#include "extrnvar.h"
#endif // #ifndef DUMP4

LE pDllList = { &pDllList, &pDllList };
PLE GetDllList(void) { return &pDllList; }

typedef struct tagDLLLIST {
    LE link;
    int done;
    CHAR name[260];
}DLLLIST, * PDLLLIST;

// general ADD TO LIST
void Add_2_list( PLE ph, PDLLLIST pdll, PSTR pnm )
{
    strcpy(pdll->name, pnm);
    pdll->done = 0;
    InsertTailList(ph,(PLE)pdll);  // add to LIST
}

void KillDllList(void) { KillLList(GetDllList()); }

PLE Add2DllList( PSTR pnm )
{
    PLE ph,pn;
    PDLLLIST pdll;
    ph = GetDllList();
    Traverse_List(ph,pn)
    {
        pdll = (PDLLLIST)pn;
        if (STRCMPI(pnm, pdll->name) == 0)
            return pn;
    }
    pdll = (PDLLLIST)dMALLOC( sizeof(DLLLIST) );
    pn = (PLE)pdll;
    if (pdll) {
        Add_2_list( ph, pdll, pnm );
    }
    return pn;
}

LE listDoneDll = { &listDoneDll, &listDoneDll };
LE listDoneFound = { &listDoneFound, &listDoneFound };
PLE GetlistDoneDll(void) { return &listDoneDll; }
PLE GetlistDoneFound(void) { return &listDoneFound; };
void KilllistDoneDll(void) { KillLList(GetlistDoneDll()); }
void KilllistDoneFound(void) { KillLList(GetlistDoneFound()); }

LE pPathList = { &pPathList, &pPathList };
PLE GetPathList(void) { return &pPathList; }
void KillPathList(void) { KillLList(GetPathList()); }
PLE Is_in_List( PLE ph, PSTR pnm )
{
    PLE pn;
    PDLLLIST pdll;
    Traverse_List(ph,pn)
    {
        pdll = (PDLLLIST)pn;
        if (STRCMPI(pnm, pdll->name) == 0)
            return pn;
    }
    return NULL;
}

BOOL Is_In_Dll_Done(PSTR pName)
{
    if (Is_in_List(GetlistDoneDll(),pName))
        return TRUE;
    return FALSE;
}

BOOL Is_In_Found_Done(PSTR pName)
{
    if (Is_in_List(GetlistDoneFound(),pName))
        return TRUE;
    return FALSE;
}

PLE Add2PathList( PSTR pnm )
{
    PLE ph,pn;
    PDLLLIST pdll;
    ph = GetPathList();
    Traverse_List(ph,pn)
    {
        pdll = (PDLLLIST)pn;
        if (STRCMPI(pnm, pdll->name) == 0)
            return pn;
    }
    pdll = (PDLLLIST)dMALLOC( sizeof(DLLLIST) );
    pn = (PLE)pdll;
    if (pdll) {
        Add_2_list( ph, pdll, pnm );
    }
    return pn;
}

LE pFoundList = { &pFoundList, &pFoundList };
PLE GetFoundList(void) { return &pFoundList; }
void KillFoundList(void) { KillLList(GetFoundList()); }

PLE Add2FoundList( PSTR pnm )
{
    PLE ph,pn;
    PDLLLIST pdll;
    ph = GetFoundList();
    Traverse_List(ph,pn)
    {
        pdll = (PDLLLIST)pn;
        if (STRCMPI(pnm, pdll->name) == 0)
            return pn;
    }
    pdll = (PDLLLIST)dMALLOC( sizeof(DLLLIST) );
    pn = (PLE)pdll;
    if (pdll) {
        Add_2_list( ph, pdll, pnm );
    }
    return pn;
}

/* ====================================
   By default, displays following blocks from a DLL/EXE
PE File Header:      fDumpFileHeader
Optional Header:
Data Directory:
Section Table: count 4 
Debug Formats in File:
Imports Table:
  libtidy.dll
exports table:
PEDUMP took the following switches
BOOL Set_PEDUMP_A( BOOL flag ); // ALL ON/OFF
BOOL Set_PEDUMP_H( BOOL flag ); // fShowRawSectionData
BOOL Set_PEDUMP_L( BOOL flag ); // fShowLineNumbers
BOOL Set_PEDUMP_P( BOOL flag ); // fShowPDATA
BOOL Set_PEDUMP_B( BOOL flag ); // fShowRelocations
BOOL Set_PEDUMP_S( BOOL flag ); // fShowSymbolTable
BOOL Set_PEDUMP_I( BOOL flag ); // fShowIATentries
BOOL Set_PEDUMP_R( BOOL flag ); // fShowResources
More added later ...
   ==================================== */


#define ADD_COFF_SYMBOL_TABLE

// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
// #define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
#define MakePtr( cast, ptr, addValue ) (cast)( (char *)(ptr) + (DWORD)(addValue))

PIMAGE_DEBUG_MISC g_pMiscDebugInfo = 0;
PDWORD g_pCVHeader = 0;
PIMAGE_COFF_SYMBOLS_HEADER g_pCOFFHeader = 0;
#ifdef ADD_COFF_SYMBOL_TABLE
COFFSymbolTable * g_pCOFFSymbolTable = 0;
#endif // #ifdef ADD_COFF_SYMBOL_TABLE

#define GetImgDirEntryRVA( pNTHdr, IDE ) \
	(pNTHdr->OptionalHeader.DataDirectory[IDE].VirtualAddress)

#define GetImgDirEntrySize( pNTHdr, IDE ) \
	(pNTHdr->OptionalHeader.DataDirectory[IDE].Size)

// Global variables set here, and used in EXEDUMP.C and OBJDUMP.C
BOOL fShowRelocations = FALSE;
BOOL fShowRawSectionData = FALSE;
BOOL fShowSymbolTable = FALSE;
BOOL fShowLineNumbers = FALSE;
BOOL fShowIATentries = FALSE;
BOOL fShowPDATA = FALSE;
BOOL fShowResources = FALSE;
unsigned long dwFileSizeLow = 0;
unsigned long dwFileSizeHigh = 0;
unsigned char * pBaseLoad = 0;
unsigned char * pBaseTop = 0;
PIMAGE_SECTION_HEADER p_rsrc = 0;
BOOL fDumpFileHeader = TRUE;
BOOL fDumpFollowImports = FALSE; // 20100527 - add -exe:F #ifdef ADD_EXE_FOLLOW
// " F = follow import trail, using PATH to find, and dump imported DLLs
BOOL fDumpOptionalHeader = TRUE;
BOOL fDumpDataDirectory = TRUE;
BOOL fDumpSectionTable = TRUE;
BOOL fDumpImportNames = TRUE;
BOOL fDumpDebugDirectory = TRUE;
BOOL fDumpExportsOnly = FALSE;

// ************************************************
#define ADD_EXE_DEBUG_DIRECTORY

//
// Given a section name, look it up in the section table and return a
// pointer to its IMAGE_SECTION_HEADER
//
PIMAGE_SECTION_HEADER GetSectionHeader(PSTR name, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;
    
    for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
    {
        if ( 0 == strncmp((char *)section->Name,name,IMAGE_SIZEOF_SHORT_NAME) )
            return section;
    }
    
    return 0;
}

// ================================================================================================
// see : http://stackoverflow.com/questions/26616379/pimage-export-directory-memory-access-error
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
// ================================================================================================
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva,
                                                PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;
    
    for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
    {
        // Is the RVA within this section?
        if ( (rva >= section->VirtualAddress) && 
             (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }
    
    return 0;
}

char *SzDebugFormats[] = {
"UNKNOWN/BORLAND","COFF","CODEVIEW","FPO","MISC","EXCEPTION","FIXUP",
"OMAP_TO_SRC", "OMAP_FROM_SRC"};

//
// Dump the debug directory array
//
void DumpDebugDirectory(PIMAGE_DEBUG_DIRECTORY debugDir, DWORD size, void *vbase)
{
   DWORD cDebugFormats = size / sizeof(IMAGE_DEBUG_DIRECTORY);
   PSTR szDebugFormat;
   unsigned i;
   char *base = (char *)vbase;

   if ( cDebugFormats == 0 )
     return;
    
   if ( fDumpDebugDirectory )
   {
      sprtf(
       "Debug Formats in File:\n"
       "  Type            Size     Address  FilePtr  Charactr TimeDate Version\n"
       "  --------------- -------- -------- -------- -------- -------- --------\n"
       );
   }

   for ( i=0; i < cDebugFormats; i++ )
   {
      szDebugFormat = (debugDir->Type <= IMAGE_DEBUG_TYPE_OMAP_FROM_SRC )
                        ? SzDebugFormats[debugDir->Type] : "???";

      if ( fDumpDebugDirectory )
      {
         sprtf("  %-15s %08X %08X %08X %08X %08X %u.%02u\n",
               szDebugFormat, debugDir->SizeOfData, debugDir->AddressOfRawData,
               debugDir->PointerToRawData, debugDir->Characteristics,
               debugDir->TimeDateStamp, debugDir->MajorVersion,
               debugDir->MinorVersion);
      }
      switch( debugDir->Type )
		{
      case IMAGE_DEBUG_TYPE_COFF:
         g_pCOFFHeader =
                (PIMAGE_COFF_SYMBOLS_HEADER)(base+ debugDir->PointerToRawData);
         break;

      case IMAGE_DEBUG_TYPE_MISC:
         g_pMiscDebugInfo =
				(PIMAGE_DEBUG_MISC)(base + debugDir->PointerToRawData);
			break;

      case IMAGE_DEBUG_TYPE_CODEVIEW:
         g_pCVHeader = (PDWORD)(base + debugDir->PointerToRawData);
			break;
		}
      debugDir++;
   }

   if ( fDumpDebugDirectory )
      sprtf("\n");

}

#define ADD_EXE_DEBUG_DIRECTORY

#ifdef   ADD_EXE_DEBUG_DIRECTORY
void DumpExeDebugDirectory(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_DEBUG_DIRECTORY debugDir;
    PIMAGE_SECTION_HEADER header;
    DWORD va_debug_dir;
    DWORD size;
    
    va_debug_dir = GetImgDirEntryRVA(pNTHeader, IMAGE_DIRECTORY_ENTRY_DEBUG);

    if ( va_debug_dir == 0 )
        return;

    // If we found a .debug section, and the debug directory is at the
    // beginning of this section, it looks like a Borland file
    header = GetSectionHeader(".debug", pNTHeader);
    if ( header && (header->VirtualAddress == va_debug_dir) )
    {
        debugDir = (PIMAGE_DEBUG_DIRECTORY)(header->PointerToRawData+base);
        size = GetImgDirEntrySize(pNTHeader, IMAGE_DIRECTORY_ENTRY_DEBUG)
                * sizeof(IMAGE_DEBUG_DIRECTORY);
    }
    else    // Look for the debug directory
    {
        header = GetEnclosingSectionHeader( va_debug_dir, pNTHeader );
        if ( !header )
            return;

        size = GetImgDirEntrySize( pNTHeader, IMAGE_DIRECTORY_ENTRY_DEBUG );
    
        debugDir = MakePtr(PIMAGE_DEBUG_DIRECTORY, base,
                            header->PointerToRawData
							+ (va_debug_dir - header->VirtualAddress) );
    }

    DumpDebugDirectory( debugDir, size, base );
}
#else
void DumpExeDebugDirectory(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
}
#endif

#define ADD_DUMP_IMPORT_SECTION

LPVOID GetPtrFromRVA( DWORD rva, PIMAGE_NT_HEADERS pNTHeader, char *imageBase )
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;
		
	pSectionHdr = GetEnclosingSectionHeader( rva, pNTHeader );
	if ( !pSectionHdr )
		return 0;

	delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}

char * pedump_ctime( time_t * ptm )
{
   __time32_t * timer = (__time32_t *)ptm;
   char * ctm = NULL;
   // ctm = ctime(ptm); // this will use 64-bit time
   ctm = _ctime32(timer); // NOTE: Returns buffer with "\n" appended
   if(ctm == NULL)  // if OUT OF RANGE
      ctm = "Out of range"MEOR;

   return ctm;
}

//
// Dump the imports table (the .idata section) of a PE file
//
#ifdef ADD_DUMP_IMPORT_SECTION

int Done_Import_Init = 0;


static char absPEPath[260];
static char absPath[260];
typedef struct tagSPLITPATH {
    char drive[_MAX_DRIVE+2];
    char dir[_MAX_DIR+2];
    char fname[_MAX_FNAME+2];
    char ext[_MAX_EXT+2];
}SPLITPATH, * PSPLITPATH;

static SPLITPATH splitpath;
VOID Do_Import_Init(VOID)
{
    PSPLITPATH psp = &splitpath;
    char * env, * cp;

    ZeroMemory(&splitpath, sizeof(SPLITPATH));

    cp = _fullpath(absPEPath, g_ActPEFile, 256 );

    _splitpath(absPEPath,
        &psp->drive[0],
        &psp->dir[0],
        &psp->fname[0],
        &psp->ext[0]);

    cp = absPath;
    strcpy(cp,psp->drive);
    strcat(cp,psp->dir);
    Add2PathList( cp );

    env = getenv("PATH");
    if (env) {
        char c, pc;
        cp = absPath;
        c = 0;
        while (*env) {
            pc = c;
            c = *env;
            if (c == ';') {
                if (cp > absPath) {
                    if (!((pc == '\\')||(pc == '/'))) {
                        *cp = '\\';
                        cp++;
                    }
                    *cp = 0;
                    cp = absPath;
                    Add2PathList(cp);
                }
            } else {
                if ( c == '/' )
                    c = '\\';
                *cp = c;
                cp++;
            }
            env++;
        }
        if (cp > absPath) {
            if (!((pc == '\\')||(pc == '/'))) {
                *cp = '\\';
                cp++;
            }
            *cp = 0;
            cp = absPath;
            Add2PathList(cp);
        }
    }
    Done_Import_Init = 1;
}


VOID Process_Next_Import(PSTR pName)
{
    PLE ph,pn;
    PDLLLIST pdll;
    char * cp;
    int fnd;
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    if (Is_In_Dll_Done(pName))
        return;
    if (!Done_Import_Init)
        Do_Import_Init();
    sprtf("Processing %s...", pName );
    cp = absPath;
    ph = GetPathList();
    fnd = 0;
    Traverse_List(ph,pn)
    {
        pdll = (PDLLLIST)pn;
        strcpy(cp,pdll->name);  // already ensured it ends in '\'
        strcat(cp,pName);       // so just copy the name
        hFind = FindFirstFile(cp,&fd);
        if (hFind && (hFind != INVALID_HANDLE_VALUE)) {
            fnd = 1;
            FindClose(hFind);
            break;

        }
    }
    if (fnd) {
        sprtf(" found [%s]", cp);
        if (Is_In_Found_Done(cp)) {
            sprtf( "... done above."MEOR );
        } else {
            Add2FoundList(cp);
            if (Is_in_List( GetFoundList(), cp ))
                sprtf("... done..."MEOR);
            else
                sprtf("... added"MEOR);
        } 
    } else {
        sprtf(" NOT FOUND!"MEOR);
    }
}

int Process_Next_PE(  LPDFSTR lpdf )
{
   dwFileSizeLow = lpdf->dwmax;
   dwFileSizeHigh = 0;
   pBaseLoad = (unsigned char *)lpdf->df_pVoid;
   pBaseTop = pBaseLoad + dwFileSizeLow;
   //if(VERB9)
   //   Set_PEDUMP_A(TRUE);
   //ShowCurrPEOpts();
   return DumpMemoryMap( lpdf->df_pVoid, lpdf->fn, lpdf->dwmax );
}


void	Do_PE_File( char * fn, HANDLE hf )
{
    LPDFSTR lpdf;
    DWORD   dwMax;
    DWORD	fsiz;
	LPTSTR  lptmp;
    PBYTE   pb = 0;	
	lptmp = &gszTmpOut[0];

    lpdf = (LPDFSTR)dMALLOC( sizeof(DFSTR) );
    if ( !lpdf )
        return; // forget it
    ZeroMemory(lpdf,sizeof(DFSTR));
    lpdf->fn    = fn;
    lpdf->lptmp = lptmp;
    if( VFH(hf) ) {
        lpdf->hf = (HANDLE)hf;
        fsiz = GetFileSize( (HANDLE)hf, NULL );
		lpdf->qwSize.LowPart = GetFileSize( (HANDLE)hf, &lpdf->qwSize.HighPart );
        if( lpdf->qwSize.HighPart )
            dwMax = (DWORD)-1;
        else
            dwMax = lpdf->qwSize.LowPart;
        lpdf->dwmax = dwMax;
        lpdf->dwrd  = dwMax;
        lpdf->df_hMap = CreateFileMapping( (HANDLE)lpdf->hf,  // handle to file
            NULL, // optional security attributes
            g_flProtext,   // protection for mapping object
            0,          // high-order 32 bits of object size
            0,          // low-order 32 bits of object size
            NULL );       // name of file-mapping object
        if( lpdf->df_hMap ) {
            lpdf->df_pVoid = MapViewOfFile( lpdf->df_hMap, // file-mapping object to map
                g_ReqAccess,   // access mode
                0,    // high-order 32 bits of file offset
                0,    // low-order 32 bits of file offset
                0 );  // number of bytes to map
            if( lpdf->df_pVoid ) {
                lpdf->lpb = (PBYTE)lpdf->df_pVoid;
                if( giVerbose ) {
                    if( giVerbose > 1 ) {
                        sprintf( lptmp,
                            "File [%s], %I64u bytes (map at %#x)."MEOR,
                            fn,
                            lpdf->qwSize,
                            lpdf->df_pVoid );
                    } else {
                        sprintf( lptmp,
                            "File [%s], %I64u bytes."MEOR,
                            fn,
                            lpdf->qwSize );
                    }
                    prt( lptmp );
                }
                Process_Next_PE( lpdf );
                // ProcessDataStr( lpdf );
                if( VERB ) {
                    if(( gdwEndOff ) &&
                       ( gdwEndOff < fsiz ) ) 
                    {
                        sprintf( lptmp, "Done [%s] Ended after %d byte offset."MEOR,
                            fn,
                            gdwEndOff );
                     } else {
                         sprintf( lptmp, "Completed [%s] = %u Bytes."MEOR,
                             fn, fsiz );
                     }
				     prt( lptmp );
                }

                if( lpdf->df_pVoid )
                    UnmapViewOfFile( lpdf->df_pVoid );  // starting address

                if( lpdf->df_hMap )
                    CloseHandle( lpdf->df_hMap );

                lpdf->df_pVoid = NULL;
                lpdf->df_hMap = 0;
            } else {
                if( VERB ) {
                    sprintf( lptmp, "ERROR: Unable to get MAP View of File"MEOR
                        "\t[%s]!"MEOR, fn );
                    prt( lptmp );
                }
                CloseHandle( lpdf->df_hMap );
            }
		} else {
            if( VERB ) {
                sprintf( lptmp, "WARNING: File [%s] is NULL!"MEOR, fn );
				prt( lptmp );
			}
		}
	} else {
        if( giVerbose ) {
            sprintf( lptmp, "ERROR: Unable to OPEN file [%s]!"MEOR, fn );
			prt( lptmp );
		}
	}
    if(pb)
        LocalFree(pb);
    dMFREE(lpdf);
}


VOID Process_Found_Import(PSTR pName)
{
    HANDLE hf;
	hf = grmOpenFile( pName, &hf, OF_READ );
    if( VFH(hf) ) {
        Do_PE_File( pName, hf );
        grmCloseFile( &hf );
    }
}

BOOL Get_List_Copy( PLE pc, PLE ph )
{
    PLE pn;
    BOOL bRet = FALSE;
    PDLLLIST pdll;
    if ( !IsListEmpty(ph) ) {
        while( !IsListEmpty( ph ) )
        {
            pn = RemoveHeadList(ph);    // remove from list
            pdll = (PDLLLIST)pn;
            if (Is_in_List(pc,pdll->name)) {
                dMFREE(pn);
            } else {
                InsertTailList(pc,pn);      // add to LIST
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

static BOOL In_Process_Import_List = FALSE;
VOID Process_Import_List(VOID)
{
    PLE pn1;
    PLE pn2;
    LE  cDll, cFound;
    int notdone = 0;
    PDLLLIST pdll;
    PLE pcDll = &cDll;
    PLE pcFound = &cFound;
    PLE pDoneDll = GetlistDoneDll();
    PLE pDoneFound = GetlistDoneFound();
    PLE phDll = GetDllList();
    PLE phFound = GetFoundList();
    if ( In_Process_Import_List )
        return;
    In_Process_Import_List = TRUE;
    InitLList(pcDll);
    InitLList(pcFound);
    //if ( fDumpFollowImports )
    while (fDumpFollowImports && Get_List_Copy(pcDll,phDll))
    {
        Traverse_List(pcDll,pn1)
        {
            pdll = (PDLLLIST)pn1;
            if (Is_in_List( pDoneDll, pdll->name ))
                continue;
            Process_Next_Import(pdll->name);
        }
        Get_List_Copy(pDoneDll,pcDll);  // add to DONE
        if (Get_List_Copy(pcFound,phFound)) {
            Traverse_List(pcFound,pn2)
            {
               pdll = (PDLLLIST)pn2;
               if (Is_in_List( pDoneFound, pdll->name) )
                   continue;
                pdll->done = 1;
                Process_Found_Import(pdll->name);
            }
        }
        Get_List_Copy(pDoneFound,pcFound);  // add to DONE
    }

    // ALL DONE
    // ========
    KillPathList();
    KillFoundList();
    KillDllList();
    if (fDumpFollowImports) {
        sprtf("Done ");
        Traverse_List(pDoneDll,pn1)
        {
            pdll = (PDLLLIST)pn1;
            sprtf("%s ",pdll->name);
        }
        sprtf(MEOR);
        Traverse_List(pDoneFound,pn2)
        {
            pdll = (PDLLLIST)pn2;
            sprtf("%s"MEOR,pdll->name);
        }
    }
    KilllistDoneDll(); // or KillLList(pDoneDll);
    KilllistDoneFound(); // or KillLList(pDoneFound);
    In_Process_Import_List = FALSE;
}


void DumpImportsSection(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
   PIMAGE_IMPORT_DESCRIPTOR importDesc;
   PIMAGE_SECTION_HEADER pSection;
   PIMAGE_THUNK_DATA thunk, thunkIAT=0;
   PIMAGE_IMPORT_BY_NAME pOrdinalName;
   DWORD importsStartRVA;
   PSTR pszTimeDate;
   PSTR pszDllName;
   int   DoneCnt = 0;

   // Look up where the imports section is (normally in the .idata section)
   // but not necessarily so.  Therefore, grab the RVA from the data dir.
   importsStartRVA = GetImgDirEntryRVA(pNTHeader,IMAGE_DIRECTORY_ENTRY_IMPORT);
   if ( !importsStartRVA )
      return;

   // Get the IMAGE_SECTION_HEADER that contains the imports.  This is
   // usually the .idata section, but doesn't have to be.
   pSection = GetEnclosingSectionHeader( importsStartRVA, pNTHeader );
   if ( !pSection )
      return;

   importDesc = (PIMAGE_IMPORT_DESCRIPTOR)
    						GetPtrFromRVA(importsStartRVA,pNTHeader,base);
	if ( !importDesc )
		return;
            
   sprtf("Imports Table: %s. %s\n",
      (fDumpImportNames ? "with entry name(s)" : "just library name(s)"),
      (fDumpFollowImports ? "(follow)" : ""));
    
   while ( 1 )
   {
      // See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
      if ( (importDesc->TimeDateStamp==0 ) && (importDesc->Name==0) )
         break;

      DoneCnt++;

      // show the library (DLL) where these entries exist
      // sprtf("  %s\n", GetPtrFromRVA(importDesc->Name, pNTHeader, base) );
      pszDllName = (PSTR)GetPtrFromRVA(importDesc->Name, pNTHeader, base);
      sprtf("  %s\n", pszDllName );
      add_2_dll_list(pszDllName);  // FIX20130225

      if ( fDumpFollowImports )
          Add2DllList(pszDllName);  // keep DLL name entry

      if ( fDumpImportNames )
      {
         sprtf("  OrigFirstThunk:  %08X (Unbound IAT)\n",
      			importDesc->Characteristics);

         pszTimeDate = pedump_ctime((time_t *)&importDesc->TimeDateStamp);
         sprtf("  TimeDateStamp:   %08X", importDesc->TimeDateStamp );
         // 20100502 - do NOT show Unix epoc '-> Thu Jan 01 01:00:00 1970' if NULL
         if (importDesc->TimeDateStamp)
             sprtf( pszTimeDate ?  " -> %s" : "\n", pszTimeDate );
         else
             sprtf( " (NULL)\n" );
         sprtf("  ForwarderChain:  %08X\n", importDesc->ForwarderChain);
         sprtf("  First thunk RVA: %08X\n", importDesc->FirstThunk);
    
         thunk = (PIMAGE_THUNK_DATA)importDesc->Characteristics;
         thunkIAT = (PIMAGE_THUNK_DATA)importDesc->FirstThunk;

         if ( thunk == 0 )   // No Characteristics field?
         {
            // Yes! Gotta have a non-zero FirstThunk field then.
            thunk = thunkIAT;
            
            if ( thunk == 0 )   // No FirstThunk field?  Ooops!!!
                goto ImportsExit;
        }
        
        // Adjust the pointer to point where the tables are in the
        // mem mapped file.
        thunk = (PIMAGE_THUNK_DATA)GetPtrFromRVA((DWORD)thunk, pNTHeader, base);
        if (!thunk )
           goto ImportsExit;

        thunkIAT = (PIMAGE_THUNK_DATA)
        			GetPtrFromRVA((DWORD)thunkIAT, pNTHeader, base);
    
        sprtf("  Ordn  Name\n");
        
        while ( 1 ) // Loop forever (or until we break out)
        {
            if ( thunk->u1.AddressOfData == 0 )
                break;

            if ( thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG )
            {
                sprtf( "  %4u", IMAGE_ORDINAL(thunk->u1.Ordinal) );
            }
            else
            {
                pOrdinalName = (PIMAGE_IMPORT_BY_NAME)thunk->u1.AddressOfData;
                pOrdinalName = (PIMAGE_IMPORT_BY_NAME)
                			GetPtrFromRVA((DWORD)pOrdinalName, pNTHeader, base);
                    
                sprtf("  %4u  %s", pOrdinalName->Hint, pOrdinalName->Name);
            }
            
		   	// If the user explicitly asked to see the IAT entries, or
			   // if it looks like the image has been bound, append the address
            if ( fShowIATentries || importDesc->TimeDateStamp )
                sprtf( " (Bound to: %08X)", thunkIAT->u1.Function );

            sprtf( "\n" );

            thunk++;            // Advance to next thunk
            thunkIAT++;         // advance to next thunk
         }

         sprtf("\n");
      }

      importDesc++;   // advance to next IMAGE_IMPORT_DESCRIPTOR

   }

ImportsExit:

   if ( DoneCnt )
      sprtf("\n");

}
#else // !#ifdef ADD_DUMP_IMPORT_SECTION
void DumpImportsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
}
#endif // #ifdef ADD_DUMP_IMPORT_SECTION

#define ADD_EXPORTS_SECTION

//
// Dump the exports table (usually the .edata section) of a PE file
//
#ifdef ADD_EXPORTS_SECTION
void DumpExportsSection(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_EXPORT_DIRECTORY exportDir, e2;
    PIMAGE_SECTION_HEADER header;
    DWORD delta; 
    PSTR filename;
    DWORD i;
    PDWORD functions;
    PWORD ordinals;
    PSTR *name;
    DWORD exportsStartRVA, exportsEndRVA;
    char* headerAddress = base + ((PIMAGE_DOS_HEADER)base)->e_lfanew;
    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)headerAddress;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)headerAddress;    
    PIMAGE_EXPORT_DIRECTORY exports = 0;
    bool is32 = true;

    exportsStartRVA = GetImgDirEntryRVA(pNTHeader,IMAGE_DIRECTORY_ENTRY_EXPORT);
    exportsEndRVA = exportsStartRVA +
	   				GetImgDirEntrySize(pNTHeader, IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    } else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        is32 = false;
    } else
        return;    // Get the IMAGE_SECTION_HEADER that contains the exports.  This is

    // usually the .edata section, but doesn't have to be.
    header = GetEnclosingSectionHeader( exportsStartRVA, pNTHeader );
    if ( !header )
        return;

    delta = (header->VirtualAddress - header->PointerToRawData);
        
    e2 = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)base + exportsStartRVA + 
        header->PointerToRawData - header->VirtualAddress);

    exportDir = MakePtr(PIMAGE_EXPORT_DIRECTORY, base,
                         exportsStartRVA - delta);

    if (e2 != exportDir) {
        return;
    }
    // Does not work!!!!
    //if (exportDir != exports) {
    //    exportDir = exports;
    //}
        
    filename = (PSTR)(exportDir->Name - delta + base);
        
    sprtf("exports table:\n\n");
    sprtf("  Name:            %s\n", filename);
    sprtf("  Characteristics: %08X\n", exportDir->Characteristics);
    sprtf("  TimeDateStamp:   %08X -> %s",
    			exportDir->TimeDateStamp,
    			pedump_ctime((time_t *)&exportDir->TimeDateStamp) );
    sprtf("  Version:         %u.%02u\n", exportDir->MajorVersion,
            exportDir->MinorVersion);
    sprtf("  Ordinal base:    %08X\n", exportDir->Base);
    sprtf("  # of functions:  %08X\n", exportDir->NumberOfFunctions);
    sprtf("  # of Names:      %08X\n", exportDir->NumberOfNames);
    
    // AddressOfNames is a RVA to a list of RVAs to string names not a RVA to a list of strings.
    functions = (PDWORD)((DWORD)exportDir->AddressOfFunctions - delta + base);
    ordinals = (PWORD)((DWORD)exportDir->AddressOfNameOrdinals - delta + base);
    name = (PSTR *)((DWORD)exportDir->AddressOfNames - delta + base);

    sprtf("\n  Entry Pt  Ordn  Name\n");
    for ( i=0; i < exportDir->NumberOfFunctions; i++ )
    {
        DWORD entryPointRVA = functions[i];
        DWORD j;

        if ( entryPointRVA == 0 )   // Skip over gaps in exported function
            continue;               // ordinals (the entrypoint is 0 for
                                    // these functions).

        sprtf("  %08X  %4u", entryPointRVA, i + exportDir->Base );

        // See if this function has an associated name exported for it.
        for ( j=0; j < exportDir->NumberOfNames; j++ ) {
            if ( ordinals[j] == i ) {
                //char *tmp = name[j];
                //tmp -= delta;
                if (is32)
                    sprtf("  %s", name[j] - delta + (DWORD)base);
                //sprtf("  %s", base + tmp);
                //sprtf("  %s", tmp);
                break;
            }
        }
        // Is it a forwarder?  If so, the entry point RVA is inside the
        // .edata section, and is an RVA to the DllName.EntryPointName
        if ( (entryPointRVA >= exportsStartRVA)
             && (entryPointRVA <= exportsEndRVA) )
        {
            sprtf(" (forwarder -> %s)", entryPointRVA - delta + base );
        }
        
        sprtf("\n");
    }
}
#else // !#ifdef ADD_EXPORTS_SECTION
void DumpExportsSection(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
}
#endif // #ifdef ADD_EXPORTS_SECTION

#define ADD_RUNTIME_FUNCTIONS

#ifdef ADD_RUNTIME_FUNCTIONS
void DumpRuntimeFunctions( char *base, PIMAGE_NT_HEADERS pNTHeader )
{
	DWORD rtFnRVA;

	rtFnRVA = GetImgDirEntryRVA( pNTHeader, IMAGE_DIRECTORY_ENTRY_EXCEPTION );
	if ( !rtFnRVA )
		return;

	DWORD cEntries =
		GetImgDirEntrySize( pNTHeader, IMAGE_DIRECTORY_ENTRY_EXCEPTION )
		/ sizeof( IMAGE_RUNTIME_FUNCTION_ENTRY );
	if ( 0 == cEntries )
		return;

	PIMAGE_RUNTIME_FUNCTION_ENTRY pRTFn = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
							GetPtrFromRVA( rtFnRVA, pNTHeader, base );

	if ( !pRTFn )
		return;

	sprtf( "Runtime Function Table (Exception handling)\n" );
	sprtf( "  Begin     End       Handler   HndlData  PrologEnd\n" );
	sprtf( "  --------  --------  --------  --------  --------\n" );

	for ( unsigned i = 0; i < cEntries; i++, pRTFn++ )
	{
#ifdef	ADD_EXCEPTION_HANDLER
		sprtf(	"  %08X  %08X  %08X  %08X  %08X",
			pRTFn->BeginAddress, pRTFn->EndAddress, pRTFn->ExceptionHandler,
			pRTFn->HandlerData, pRTFn->PrologEndAddress );
#else
		sprtf(	"  %08X  %08X other missing?",
			pRTFn->BeginAddress, pRTFn->EndAddress );
#endif
		if ( g_pCOFFSymbolTable )
		{
			PCOFFSymbol pSymbol
				= g_pCOFFSymbolTable->GetNearestSymbolFromRVA(
										pRTFn->BeginAddress
										- pNTHeader->OptionalHeader.ImageBase,
										TRUE );
			if ( pSymbol )
				sprtf( "  %s", pSymbol->GetName() );

			delete pSymbol;
		}

		sprtf( "\n" );
	}
}
#else // #ifdef ADD_RUNTIME_FUNCTIONS
void DumpRuntimeFunctions( char *base, PIMAGE_NT_HEADERS pNTHeader )
{
}
#endif // #ifdef ADD_RUNTIME_FUNCTIONS

// The names of the available base relocations
char *SzRelocTypes[] = {
"ABSOLUTE","HIGH","LOW","HIGHLOW","HIGHADJ","MIPS_JMPADDR",
"SECTION","REL32" };

#define ADD_BASE_RELOCATIONS_SECTION
//
// Dump the base relocation table of a PE file
//
#ifdef ADD_BASE_RELOCATIONS_SECTION
void DumpBaseRelocationsSection(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
	DWORD dwBaseRelocRVA;
    PIMAGE_BASE_RELOCATION baseReloc;

	dwBaseRelocRVA =
		GetImgDirEntryRVA( pNTHeader, IMAGE_DIRECTORY_ENTRY_BASERELOC );
    if ( !dwBaseRelocRVA )
        return;

    baseReloc = (PIMAGE_BASE_RELOCATION)
    				GetPtrFromRVA( dwBaseRelocRVA, pNTHeader, base );
	if ( !baseReloc )
		return;

    sprtf("base relocations:\n\n");

    while ( baseReloc->SizeOfBlock != 0 )
    {
        unsigned i,cEntries;
        PWORD pEntry;
        char *szRelocType;
        WORD relocType;

		// Sanity check to make sure the data looks OK.
		if ( 0 == baseReloc->VirtualAddress )
			break;
		if ( baseReloc->SizeOfBlock < sizeof(*baseReloc) )
			break;
		
        cEntries = (baseReloc->SizeOfBlock-sizeof(*baseReloc))/sizeof(WORD);
        pEntry = MakePtr( PWORD, baseReloc, sizeof(*baseReloc) );
        
        sprtf("Virtual Address: %08X  size: %08X\n",
                baseReloc->VirtualAddress, baseReloc->SizeOfBlock);
            
        for ( i=0; i < cEntries; i++ )
        {
            // Extract the top 4 bits of the relocation entry.  Turn those 4
            // bits into an appropriate descriptive string (szRelocType)
            relocType = (*pEntry & 0xF000) >> 12;
            szRelocType = relocType < 8 ? SzRelocTypes[relocType] : "unknown";
            
            sprtf("  %08X %s",
                    (*pEntry & 0x0FFF) + baseReloc->VirtualAddress,
                    szRelocType);

			if ( IMAGE_REL_BASED_HIGHADJ == relocType )
			{
				pEntry++;
				cEntries--;
				sprtf( " (%X)", *pEntry );
			}

			sprtf( "\n" );
            pEntry++;   // Advance to next relocation entry
        }
        
        baseReloc = MakePtr( PIMAGE_BASE_RELOCATION, baseReloc,
                             baseReloc->SizeOfBlock);
    }
}
#else // #ifdef ADD_BASE_RELOCATIONS_SECTION
void DumpBaseRelocationsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
}
#endif // #ifdef ADD_BASE_RELOCATIONS_SECTION

#define ADD_BOUND_IMPORT_DESCRIPTORS
//
// Dump out the new IMAGE_BOUND_IMPORT_DESCRIPTOR that NT 3.51 added
//
#ifdef ADD_BOUND_IMPORT_DESCRIPTORS
void DumpBoundImportDescriptors( char *base, PIMAGE_NT_HEADERS pNTHeader )
{
    DWORD bidRVA;   // Bound import descriptors RVA
    PIMAGE_BOUND_IMPORT_DESCRIPTOR pibid;

    bidRVA = GetImgDirEntryRVA(pNTHeader, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);
    if ( !bidRVA )
        return;
    
    pibid = MakePtr( PIMAGE_BOUND_IMPORT_DESCRIPTOR, base, bidRVA );
    
    sprtf( "Bound import descriptors:\n\n" );
    sprtf( "  Module        TimeDate\n" );
    sprtf( "  ------------  --------\n" );
    
    while ( pibid->TimeDateStamp )
    {
        unsigned i;
        PIMAGE_BOUND_FORWARDER_REF pibfr;
        
        sprtf( "  %-12s  %08X -> %s",
        		base + bidRVA + pibid->OffsetModuleName,
                pibid->TimeDateStamp,
                pedump_ctime((time_t *)&pibid->TimeDateStamp) );
                            
        pibfr = MakePtr(PIMAGE_BOUND_FORWARDER_REF, pibid,
                            sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR));

        for ( i=0; i < pibid->NumberOfModuleForwarderRefs; i++ )
        {
            sprtf("    forwarder:  %-12s  %08X -> %s", 
                            base + bidRVA + pibfr->OffsetModuleName,
                            pibfr->TimeDateStamp,
                            pedump_ctime((time_t *)&pibfr->TimeDateStamp) );
            pibfr++;    // advance to next forwarder ref
                
            // Keep the outer loop pointer up to date too!
            pibid = MakePtr( PIMAGE_BOUND_IMPORT_DESCRIPTOR, pibid,
                             sizeof( IMAGE_BOUND_FORWARDER_REF ) );
        }

        pibid++;    // Advance to next pibid;
    }
}
#else // #ifdef ADD_BOUND_IMPORT_DESCRIPTORS
void DumpBoundImportDescriptors( char *base, PIMAGE_NT_HEADERS pNTHeader )
{
}
#endif // #ifdef ADD_BOUND_IMPORT_DESCRIPTORS

int not_EXE_Header( PIMAGE_NT_HEADERS pNTHeader, PTSTR *perr )
{
   int iret = 0;
   PTSTR perror = NULL;
   if(perr)
      *perr = perror;

    __try
    {
        if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        {
            perror = "Not a Portable Executable (PE) EXE\n";
            iret = 1;
        }
    }
    __except( TRUE )    // Should only get here if pNTHeader (above) is bogus
    {
        perror = "invalid .EXE\n";
        iret = 1;
    }
   return iret;
}

#define ADD_DUMP_HEADER
// 20140614: Increase Machine Type Names - from WinNT.h
PSTR GetMachineTypeName( WORD wMachineType )
{
    switch (wMachineType)
    {
    case IMAGE_FILE_MACHINE_UNKNOWN: return "Unknown";  // 0
    case IMAGE_FILE_MACHINE_I386: return "Intel 386";   // 0x014c  // Intel 386.
    case IMAGE_FILE_MACHINE_R3000: return "MIPS-LE";    // 0x0162  // MIPS little-endian, 0x160 big-endian
    case IMAGE_FILE_MACHINE_R4000: return "MIPS-LE";    // 0x0166  // MIPS little-endian
    case IMAGE_FILE_MACHINE_R10000: return "MIPS-LE";   // 0x0168  // MIPS little-endian
    case IMAGE_FILE_MACHINE_WCEMIPSV2: return "MIPS-LE_WCE"; // 0x0169  // MIPS little-endian WCE v2
    case IMAGE_FILE_MACHINE_ALPHA: return "Alpha_AXP";  // 0x0184  // Alpha_AXP
    case IMAGE_FILE_MACHINE_SH3: return "SH3-LE";       // 0x01a2  // SH3 little-endian
    case IMAGE_FILE_MACHINE_SH3DSP: return "SH3DSP";    // 0x01a3
    case IMAGE_FILE_MACHINE_SH3E: return "SH3E-LE";     // 0x01a4  // SH3E little-endian
    case IMAGE_FILE_MACHINE_SH4: return "SH4-LE";       // 0x01a6  // SH4 little-endian
    case IMAGE_FILE_MACHINE_SH5: return "SH5";          // 0x01a8  // SH5
    case IMAGE_FILE_MACHINE_ARM: return "ARM-LE";       // 0x01c0  // ARM Little-Endian
    case IMAGE_FILE_MACHINE_THUMB: return "Thumb";      // 0x01c2
    case IMAGE_FILE_MACHINE_AM33: return "ARM33";       // 0x01d3
    case IMAGE_FILE_MACHINE_POWERPC: return "PowerPC-LE"; // 0x01F0  // IBM PowerPC Little-Endian
    case IMAGE_FILE_MACHINE_POWERPCFP: return "PowerPCFP"; // 0x01f1
    case IMAGE_FILE_MACHINE_IA64: return "Intel 64";           // 0x0200  // Intel 64
    case IMAGE_FILE_MACHINE_MIPS16: return "MIPS16";    // 0x0266  // MIPS
    case IMAGE_FILE_MACHINE_ALPHA64: return "Alpha64";  // 0x0284  // ALPHA64
    case IMAGE_FILE_MACHINE_MIPSFPU: return "MIPSFPU";  // 0x0366  // MIPS
    case IMAGE_FILE_MACHINE_MIPSFPU16: return "MIPSFPU16";  // 0x0466  // MIPS
    // case IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
    case IMAGE_FILE_MACHINE_TRICORE: return "infineon"; // 0x0520  // Infineon
    case IMAGE_FILE_MACHINE_CEF: return "CEF";          // 0x0CEF
    case IMAGE_FILE_MACHINE_EBC: return "EFI-BC";       // 0x0EBC  // EFI Byte Code
    case IMAGE_FILE_MACHINE_AMD64: return "AMD64-K8";   // 0x8664  // AMD64 (K8)
    case IMAGE_FILE_MACHINE_M32R: return "M32R-LE";     // 0x9041  // M32R little-endian
    case IMAGE_FILE_MACHINE_CEE: return "CCE";          // 0xC0EE
    }
    return "UNLISTED";
}
PSTR GetMachineTypeName_OLD( WORD wMachineType )
{
    switch( wMachineType )
    {
        case IMAGE_FILE_MACHINE_I386: 	return "i386";
        // case IMAGE_FILE_MACHINE_I860:return "i860";
        case IMAGE_FILE_MACHINE_R3000:  return "R3000";
		case 160:                       return "R3000 big endian";
        case IMAGE_FILE_MACHINE_R4000:  return "R4000";
		case IMAGE_FILE_MACHINE_R10000: return "R10000";
        case IMAGE_FILE_MACHINE_ALPHA:  return "Alpha";
        case IMAGE_FILE_MACHINE_POWERPC:return "PowerPC";
        default:    					return "unknown";
    }
}

//typedef struct tagWORD_FLAG_DESCRIPTIONS {
//    WORD    flag;
//    PSTR    name;
//} WORD_FLAG_DESCRIPTIONS, *PWORD_FLAG_DESCRIPTIONS;
//typedef struct tagDWORD_FLAG_DESCRIPTIONS {
//    DWORD   flag;
//    PSTR    name;
//} DWORD_FLAG_DESCRIPTIONS, *PDWORD_FLAG_DESCRIPTIONS;

// 20140719 UPDATE
// Bitfield values and names for the IMAGE_FILE_HEADER flags
WORD_FLAG_DESCRIPTIONS ImageFileHeaderCharacteristics[] = {
    { IMAGE_FILE_RELOCS_STRIPPED,"RELOCS_STRIPPED"}, // 0x0001  // Relocation info stripped from file.
    { IMAGE_FILE_EXECUTABLE_IMAGE,"EXECUTABLE_IMAGE"}, // 0x0002  // File is executable  (i.e. no unresolved externel references).
    { IMAGE_FILE_LINE_NUMS_STRIPPED,"LINE_NUMS_STRIPPED"}, // 0x0004  // Line nunbers stripped from file.
    { IMAGE_FILE_LOCAL_SYMS_STRIPPED,"LOCAL_SYMS_STRIPPED"}, // 0x0008  // Local symbols stripped from file.
    { IMAGE_FILE_AGGRESIVE_WS_TRIM,"AGGRESIVE_WS_TRIM"}, // 0x0010  // Agressively trim working set
    { IMAGE_FILE_LARGE_ADDRESS_AWARE,"LARGE_ADDRESS_AWARE"}, // 0x0020  // App can handle >2gb addresses
    { IMAGE_FILE_BYTES_REVERSED_LO,"BYTES_REVERSED_LO"}, //  0x0080  // Bytes of machine word are reversed.
    { IMAGE_FILE_32BIT_MACHINE,"32BIT_MACHINE"}, // 0x0100  // 32 bit word machine.
    { IMAGE_FILE_DEBUG_STRIPPED,"DEBUG_STRIPPED"}, // 0x0200  // Debugging info stripped from file in .DBG file
    { IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP,"REMOVABLE_RUN_FROM_SWAP"}, // 0x0400  // If Image is on removable media, copy and run from the swap file.
    { IMAGE_FILE_NET_RUN_FROM_SWAP,"NET_RUN_FROM_SWAP"}, // 0x0800  // If Image is on Net, copy and run from the swap file.
    { IMAGE_FILE_SYSTEM,"SYSTEM"}, //                    0x1000  // System File.
    { IMAGE_FILE_DLL,"DLL"}, //                       0x2000  // File is a DLL.
    { IMAGE_FILE_UP_SYSTEM_ONLY,"UP_SYSTEM_ONLY"}, //            0x4000  // File should only be run on a UP machine
    { IMAGE_FILE_BYTES_REVERSED_HI,"BYTES_REVERSED_HI"} //       0x8000  // Bytes of machine word are reversed.
};

#define NUMBER_IMAGE_HEADER_FLAGS \
    (sizeof(ImageFileHeaderCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))

UINT getImageFlagCount() { return NUMBER_IMAGE_HEADER_FLAGS; }
PWORD_FLAG_DESCRIPTIONS getImageFlagStruct() { return &ImageFileHeaderCharacteristics[0]; }
//
// Dump the IMAGE_FILE_HEADER for a PE file or an OBJ
//
void DumpHeader(PIMAGE_FILE_HEADER pImageFileHeader)
{
   UINT headerFieldWidth = 30;
   UINT i;

   add_2_machine_list(pImageFileHeader->Machine);
   if( fDumpFileHeader )
   {
      sprtf("PE File Header:\n");

      sprtf("  %-*s%04X (%s)\n", headerFieldWidth, "Machine:", 
                pImageFileHeader->Machine,
                GetMachineTypeName(pImageFileHeader->Machine) );
      sprtf("  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
                pImageFileHeader->NumberOfSections);
      sprtf("  %-*s%08X -> %s", headerFieldWidth, "TimeDateStamp:",
                pImageFileHeader->TimeDateStamp,
                pedump_ctime((time_t *)&pImageFileHeader->TimeDateStamp));
      sprtf("  %-*s%08X\n", headerFieldWidth, "PointerToSymbolTable:",
                pImageFileHeader->PointerToSymbolTable);
      sprtf("  %-*s%08X\n", headerFieldWidth, "NumberOfSymbols:",
                pImageFileHeader->NumberOfSymbols);
      sprtf("  %-*s%04X\n", headerFieldWidth, "SizeOfOptionalHeader:",
                pImageFileHeader->SizeOfOptionalHeader);
      sprtf("  %-*s%04X\n", headerFieldWidth, "Characteristics:",
                pImageFileHeader->Characteristics);
      for ( i=0; i < NUMBER_IMAGE_HEADER_FLAGS; i++ )
      {
        if ( pImageFileHeader->Characteristics & 
             ImageFileHeaderCharacteristics[i].flag )
            sprtf( "    %s\n", ImageFileHeaderCharacteristics[i].name );
      }
      sprtf("\n");
   }
}

#define ADD_DUMP_OPTIONAL

#ifndef	IMAGE_DLLCHARACTERISTICS_WDM_DRIVER
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER  0x2000     // Driver uses WDM model
#endif

// Marked as obsolete in MSDN CD 9
// Bitfield values and names for the DllCharacteritics flags
WORD_FLAG_DESCRIPTIONS DllCharacteristics[] = 
{
{ IMAGE_DLLCHARACTERISTICS_WDM_DRIVER, "WDM_DRIVER" },
};
#define NUMBER_DLL_CHARACTERISTICS \
    (sizeof(DllCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))

#if 0
// Marked as obsolete in MSDN CD 9
// Bitfield values and names for the LoaderFlags flags
DWORD_FLAG_DESCRIPTIONS LoaderFlags[] = 
{
{ IMAGE_LOADER_FLAGS_BREAK_ON_LOAD, "BREAK_ON_LOAD" },
{ IMAGE_LOADER_FLAGS_DEBUG_ON_LOAD, "DEBUG_ON_LOAD" }
};
#define NUMBER_LOADER_FLAGS \
    (sizeof(LoaderFlags) / sizeof(DWORD_FLAG_DESCRIPTIONS))
#endif

// Names of the data directory elements that are defined
char *ImageDirectoryNames[] = {
    "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY", "BASERELOC",
    "DEBUG", "COPYRIGHT", "GLOBALPTR", "TLS", "LOAD_CONFIG",
    "BOUND_IMPORT", "IAT",  // These two entries added for NT 3.51
	"DELAY_IMPORT" };		// This entry added in NT 5

#define NUMBER_IMAGE_DIRECTORY_ENTRYS \
    (sizeof(ImageDirectoryNames)/sizeof(char *))

//
// Dump the IMAGE_OPTIONAL_HEADER from a PE file
//
void DumpOptionalHeader(PIMAGE_OPTIONAL_HEADER optionalHeader)
{
   UINT width = 30;
   char *s;
   UINT i;

   if ( fDumpOptionalHeader )
   {

      sprtf("Optional Header:\n");

      sprtf("  %-*s%04X\n", width, "Magic", optionalHeader->Magic);
      sprtf("  %-*s%u.%02u\n", width, "linker version",
        optionalHeader->MajorLinkerVersion,
        optionalHeader->MinorLinkerVersion);
      sprtf("  %-*s%X\n", width, "size of code", optionalHeader->SizeOfCode);
      sprtf("  %-*s%X\n", width, "size of initialized data",
        optionalHeader->SizeOfInitializedData);
      sprtf("  %-*s%X\n", width, "size of uninitialized data",
        optionalHeader->SizeOfUninitializedData);
      sprtf("  %-*s%X\n", width, "entrypoint RVA",
        optionalHeader->AddressOfEntryPoint);
      sprtf("  %-*s%X\n", width, "base of code", optionalHeader->BaseOfCode);
      // TODO: This could be a dump of a 32-bit PE files, in whihc case need to use DWORD BaseOfData
#if (defined(IS_64BIT_BUILD) || defined(_WIN64))    // base offset is now a ULONGLONG (63-bit)
      sprtf("  %-*s%I64X\n", width, "base of data", optionalHeader->ImageBase);
#else
      sprtf("  %-*s%X\n", width, "base of data", optionalHeader->BaseOfData);
#endif
      sprtf("  %-*s%X\n", width, "image base", optionalHeader->ImageBase);

      sprtf("  %-*s%X\n", width, "section align",
        optionalHeader->SectionAlignment);
      sprtf("  %-*s%X\n", width, "file align", optionalHeader->FileAlignment);
      sprtf("  %-*s%u.%02u\n", width, "required OS version",
        optionalHeader->MajorOperatingSystemVersion,
        optionalHeader->MinorOperatingSystemVersion);
      sprtf("  %-*s%u.%02u\n", width, "image version",
        optionalHeader->MajorImageVersion,
        optionalHeader->MinorImageVersion);
      sprtf("  %-*s%u.%02u\n", width, "subsystem version",
        optionalHeader->MajorSubsystemVersion,
        optionalHeader->MinorSubsystemVersion);
      sprtf("  %-*s%X\n", width, "Win32 Version",
      optionalHeader->Win32VersionValue);
      sprtf("  %-*s%X\n", width, "size of image", optionalHeader->SizeOfImage);
      sprtf("  %-*s%X\n", width, "size of headers",
            optionalHeader->SizeOfHeaders);
      sprtf("  %-*s%X\n", width, "checksum", optionalHeader->CheckSum);
      switch( optionalHeader->Subsystem )
      {
        case IMAGE_SUBSYSTEM_NATIVE: s = "Native"; break;
        case IMAGE_SUBSYSTEM_WINDOWS_GUI: s = "Windows GUI"; break;
        case IMAGE_SUBSYSTEM_WINDOWS_CUI: s = "Windows character"; break;
        case IMAGE_SUBSYSTEM_OS2_CUI: s = "OS/2 character"; break;
        case IMAGE_SUBSYSTEM_POSIX_CUI: s = "Posix character"; break;
        default: s = "unknown";
      }
      sprtf("  %-*s%04X (%s)\n", width, "Subsystem",
            optionalHeader->Subsystem, s);

      // Marked as obsolete in MSDN CD 9
      sprtf("  %-*s%04X\n", width, "DLL flags",
            optionalHeader->DllCharacteristics);
      for ( i=0; i < NUMBER_DLL_CHARACTERISTICS; i++ )
      {
        if ( optionalHeader->DllCharacteristics & 
             DllCharacteristics[i].flag )
            sprtf( "  %-*s%s", width, " ", DllCharacteristics[i].name );
      }
      if ( optionalHeader->DllCharacteristics )
        sprtf("\n");

      sprtf("  %-*s%X\n", width, "stack reserve size",
        optionalHeader->SizeOfStackReserve);
      sprtf("  %-*s%X\n", width, "stack commit size",
        optionalHeader->SizeOfStackCommit);
      sprtf("  %-*s%X\n", width, "heap reserve size",
        optionalHeader->SizeOfHeapReserve);
      sprtf("  %-*s%X\n", width, "heap commit size",
        optionalHeader->SizeOfHeapCommit);

      #if 0
      // Marked as obsolete in MSDN CD 9
      sprtf("  %-*s%08X\n", width, "loader flags",
        optionalHeader->LoaderFlags);

      for ( i=0; i < NUMBER_LOADER_FLAGS; i++ )
      {
        if ( optionalHeader->LoaderFlags & 
             LoaderFlags[i].flag )
            sprtf( "  %s", LoaderFlags[i].name );
      }
      if ( optionalHeader->LoaderFlags )
        sprtf("\n");
      #endif

      sprtf("  %-*s%X\n", width, "RVAs & sizes",
        optionalHeader->NumberOfRvaAndSizes);

      sprtf("\n");
   }

   if ( fDumpDataDirectory )
   {
      sprtf("\nData Directory:\n");
      for ( i=0; i < optionalHeader->NumberOfRvaAndSizes; i++)
      {
        sprtf( "  %-12s rva: %08X  size: %08X\n",
            (i >= NUMBER_IMAGE_DIRECTORY_ENTRYS)
                ? "unused" : ImageDirectoryNames[i], 
            optionalHeader->DataDirectory[i].VirtualAddress,
            optionalHeader->DataDirectory[i].Size);
      }
   }
}

#define ADD_DUMP_SECTION_TABLE

/*----------------------------------------------------------------------------*/
//
// Section related stuff
//
/*----------------------------------------------------------------------------*/

//
// These aren't defined in the NT 4.0 WINNT.H, so we'll define them here
//
#ifndef IMAGE_SCN_TYPE_DSECT
#define IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
#endif
#ifndef IMAGE_SCN_TYPE_NOLOAD
#define IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
#endif
#ifndef IMAGE_SCN_TYPE_GROUP
#define IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#endif
#ifndef IMAGE_SCN_TYPE_COPY
#define IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.
#endif
#ifndef IMAGE_SCN_TYPE_OVER
#define IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#endif
#ifndef IMAGE_SCN_MEM_PROTECTED
#define IMAGE_SCN_MEM_PROTECTED              0x00004000
#endif
#ifndef IMAGE_SCN_MEM_SYSHEAP
#define IMAGE_SCN_MEM_SYSHEAP                0x00010000
#endif

// Bitfield values and names for the IMAGE_SECTION_HEADER flags
DWORD_FLAG_DESCRIPTIONS SectionCharacteristics[] = 
{

{ IMAGE_SCN_TYPE_DSECT, "DSECT" },
{ IMAGE_SCN_TYPE_NOLOAD, "NOLOAD" },
{ IMAGE_SCN_TYPE_GROUP, "GROUP" },
{ IMAGE_SCN_TYPE_NO_PAD, "NO_PAD" },
{ IMAGE_SCN_TYPE_COPY, "COPY" },
{ IMAGE_SCN_CNT_CODE, "CODE" },
{ IMAGE_SCN_CNT_INITIALIZED_DATA, "INITIALIZED_DATA" },
{ IMAGE_SCN_CNT_UNINITIALIZED_DATA, "UNINITIALIZED_DATA" },
{ IMAGE_SCN_LNK_OTHER, "OTHER" },
{ IMAGE_SCN_LNK_INFO, "INFO" },
{ IMAGE_SCN_TYPE_OVER, "OVER" },
{ IMAGE_SCN_LNK_REMOVE, "REMOVE" },
{ IMAGE_SCN_LNK_COMDAT, "COMDAT" },
{ IMAGE_SCN_MEM_PROTECTED, "PROTECTED" },
{ IMAGE_SCN_MEM_FARDATA, "FARDATA" },
{ IMAGE_SCN_MEM_SYSHEAP, "SYSHEAP" },
{ IMAGE_SCN_MEM_PURGEABLE, "PURGEABLE" },
{ IMAGE_SCN_MEM_LOCKED, "LOCKED" },
{ IMAGE_SCN_MEM_PRELOAD, "PRELOAD" },
{ IMAGE_SCN_LNK_NRELOC_OVFL, "NRELOC_OVFL" },
{ IMAGE_SCN_MEM_DISCARDABLE, "DISCARDABLE" },
{ IMAGE_SCN_MEM_NOT_CACHED, "NOT_CACHED" },
{ IMAGE_SCN_MEM_NOT_PAGED, "NOT_PAGED" },
{ IMAGE_SCN_MEM_SHARED, "SHARED" },
{ IMAGE_SCN_MEM_EXECUTE, "EXECUTE" },
{ IMAGE_SCN_MEM_READ, "READ" },
{ IMAGE_SCN_MEM_WRITE, "WRITE" },
};

#define NUMBER_SECTION_CHARACTERISTICS \
    (sizeof(SectionCharacteristics) / sizeof(DWORD_FLAG_DESCRIPTIONS))

//
// Dump the section table from a PE file or an OBJ
//
int Is_Section_Count_Bad( PIMAGE_SECTION_HEADER section,
                      unsigned cSections )
{
   unsigned char * cp = (unsigned char *)section;
   unsigned max = (cSections * sizeof(IMAGE_SECTION_HEADER));
   unsigned char * cptop = (cp + max);

   if( cptop > pBaseTop )
   {
      return 1;
   }

   return 0;
}

int Is_Section_NV_Blank( PIMAGE_SECTION_HEADER section )
{
   if(( section->Name[0] == 0 )&&
      ( section->Misc.VirtualSize == 0 )&&
      ( section->VirtualAddress == 0 ))
   {
      return 1;
   }
   return 0;
}

int Is_Section_Blank( PIMAGE_SECTION_HEADER section )
{
   if(( section->Name[0] == 0 )&&
    ( section->Misc.VirtualSize == 0 )&&
    ( section->VirtualAddress == 0 )&&
    ( section->PointerToRawData == 0 )&&
    ( section->SizeOfRawData == 0 )&&
    ( section->PointerToRelocations == 0 )&&
    ( section->NumberOfRelocations == 0 )&&
    ( section->PointerToLinenumbers == 0 )&&
    ( section->NumberOfLinenumbers == 0 )&&
    ( section->Characteristics == 0 ))
   {
      return 1;
   }

   return 0;
}

int out_of_top_range( unsigned char * ptr )
{
    if( ptr > pBaseTop )
        return 1;
    if( ptr < pBaseLoad )
        return 2;
    return 0;
}


void DumpSectionTable(PIMAGE_SECTION_HEADER section_in,
                      unsigned cSections_in,
                      BOOL IsEXE)
{
   unsigned i, j, cSections;
   PIMAGE_SECTION_HEADER section;
   PSTR pszAlign;
   int   bad_cnt = Is_Section_Count_Bad( section_in, cSections_in );
   int out_of_range = 0;

   section   = section_in;
   cSections = cSections_in;
   if ( fDumpSectionTable && (cSections == 0xffff)) {
       // FIX20120208 - Skip if count 0xFFFF in object
        // this LOOKS like a problem!
       // THIS IS A PROBLEM - ASSUME WHEN THE WORD IS 0XFFFF
       // that there are NO SECTIONS TO DUMP
       out_of_range = 1;
       //for ( i=1; i <= cSections; i++, section++ ) {
       //    if (out_of_top_range((unsigned char *)section)) {
       //         out_of_range = 1;
       //         break;
       //    }
       //     if ( Is_Section_Blank( section ) ) {
       //         break;
       //     }
       //}
       //if ( !out_of_range && (i < cSections)) {
       //   sprtf("Section Table: count %d %s\n", cSections,
       //      ( bad_cnt ? "\nbut will stop on first all null section!" : "") );
       //} else {
          sprtf("Section Table: count %d (0x%F) Aborting listing\n", cSections, cSections);
          //sprtf("Section Table: count %d %s\n", cSections,
          //   ( bad_cnt ? "\nbut NO nul section found before out of range! Aborting listing..." : "") );
          return;
       //}
   }
   section   = section_in;
   if ( fDumpSectionTable )
   {
      sprtf("Section Table: count %d %s\n", cSections,
         ( bad_cnt ? "\nbut will stop on first all null section!" : "") );
       
       for ( i=1; i <= cSections; i++, section++ )
       {
          if ( bad_cnt && Is_Section_Blank( section ) )
          {
             sprtf( "WARNING: Aborting list on fully BLANK entry!\n" );
             break;
          }
          if (strcmp((const char *)&section->Name[0],".rsrc") == 0) {
              sprtf("Got the RESOURCES section pointer.\n");
              p_rsrc = section;
          }
           sprtf( "  %02X %-8.8s  %s: %08X  VirtAddr:  %08X\n",
                   i, section->Name,
                   IsEXE ? "VirtSize" : "PhysAddr",
                   section->Misc.VirtualSize, section->VirtualAddress);
           sprtf( "    raw data offs:   %08X  raw data size: %08X\n",
                   section->PointerToRawData, section->SizeOfRawData );
           sprtf( "    relocation offs: %08X  relocations:   %08X\n",
                   section->PointerToRelocations, section->NumberOfRelocations );
           sprtf( "    line # offs:     %08X  line #'s:      %08X\n",
                   section->PointerToLinenumbers, section->NumberOfLinenumbers );
           sprtf( "    characteristics: %08X\n", section->Characteristics);

           sprtf("    ");
           for ( j=0; j < NUMBER_SECTION_CHARACTERISTICS; j++ )
           {
               if ( section->Characteristics & 
                   SectionCharacteristics[j].flag )
                   sprtf( "  %s", SectionCharacteristics[j].name );
           }
   		
		   switch( section->Characteristics & IMAGE_SCN_ALIGN_64BYTES )
		   {
			   case IMAGE_SCN_ALIGN_1BYTES: pszAlign = "ALIGN_1BYTES"; break;
			   case IMAGE_SCN_ALIGN_2BYTES: pszAlign = "ALIGN_2BYTES"; break;
			   case IMAGE_SCN_ALIGN_4BYTES: pszAlign = "ALIGN_4BYTES"; break;
			   case IMAGE_SCN_ALIGN_8BYTES: pszAlign = "ALIGN_8BYTES"; break;
			   case IMAGE_SCN_ALIGN_16BYTES: pszAlign = "ALIGN_16BYTES"; break;
			   case IMAGE_SCN_ALIGN_32BYTES: pszAlign = "ALIGN_32BYTES"; break;
			   case IMAGE_SCN_ALIGN_64BYTES: pszAlign = "ALIGN_64BYTES"; break;
			   default: pszAlign = "ALIGN_DEFAULT(16)"; break;
		   }
		   sprtf( "  %s", pszAlign );

         sprtf("\n\n");
       }
       sprtf("\n");
   }
}

#define ADD_DUMP_RESOURCE_SECTION

//
// If a resource entry has a string name (rather than an ID), go find
// the string and convert it from unicode to ascii.
//
void GetResourceNameFromId
(
    DWORD id, DWORD resourceBase, PSTR buffer, UINT cBytes
)
{
    PIMAGE_RESOURCE_DIR_STRING_U prdsu;

    // If it's a regular ID, just format it.
    if ( !(id & IMAGE_RESOURCE_NAME_IS_STRING) )
    {
        sprintf(buffer, "%X", id);
        return;
    }
    
    id &= 0x7FFFFFFF;
    prdsu = (PIMAGE_RESOURCE_DIR_STRING_U)(resourceBase + id);
    unsigned char *ucptr = (unsigned char *)prdsu;
    if ((ucptr + 32) >= pBaseTop) {
        sprtf("Ptr %p OOR %p!\n", ucptr, pBaseTop);
        sprintf(buffer, "%X", id);
        return;
    }
    // prdsu->Length is the number of unicode characters
    WideCharToMultiByte(CP_ACP, 0, prdsu->NameString, prdsu->Length,
                        buffer, cBytes, 0, 0);
    buffer[ MIN((cBytes-1),prdsu->Length) ] = 0;  // Null terminate it!!!
}

// The predefined resource types
char *SzResourceTypes[] = {
"???_0",
"CURSOR",
"BITMAP",
"ICON",
"MENU",
"DIALOG",
"STRING",
"FONTDIR",
"FONT",
"ACCELERATORS",
"RCDATA",
"MESSAGETABLE",
"GROUP_CURSOR",
"???_13",
"GROUP_ICON",
"???_15",
"VERSION",
"DLGINCLUDE",
"???_18",
"PLUGPLAY",
"VXD",
"ANICURSOR",
"ANIICON"
};

PIMAGE_RESOURCE_DIRECTORY_ENTRY pStrResEntries = 0;
PIMAGE_RESOURCE_DIRECTORY_ENTRY pDlgResEntries = 0;
PIMAGE_RESOURCE_DIRECTORY_ENTRY pVerResEntries = 0;
DWORD cStrResEntries = 0;
DWORD cDlgResEntries = 0;
DWORD cVerResEntries = 0;

void GetResourceTypeName(DWORD type, PSTR buffer, UINT cBytes);
void DumpResourceEntry( PIMAGE_RESOURCE_DIRECTORY_ENTRY resDirEntry,
    DWORD resourceBase,
    DWORD level,
    DWORD resourceType,
    DWORD baseResType,
    char *base,
    PIMAGE_NT_HEADERS pNTHeader);

//
// Dump the information about one resource directory.
//
void DumpResourceDirectory( PIMAGE_RESOURCE_DIRECTORY resDir, DWORD resourceBase, 
    DWORD level,
    DWORD resourceType,
    DWORD baseResType,
    char *base,
    PIMAGE_NT_HEADERS pNTHeader
)
{
    PIMAGE_RESOURCE_DIRECTORY_ENTRY resDirEntry;
    char szType[64];
    UINT i;

    // Level 1 resources are the resource types
    if ( level == 1 )
    {
		sprtf( "    ---------------------------------------------------"
	            "-----------\n" );

		if ( resourceType & IMAGE_RESOURCE_NAME_IS_STRING )
		{
			GetResourceNameFromId( resourceType, resourceBase,
									szType, sizeof(szType) );
		}
		else
		{
	        GetResourceTypeName( resourceType, szType, sizeof(szType) );
		}
	}
    else    // All other levels, just print out the regular id or name
    {
        GetResourceNameFromId( resourceType, resourceBase, szType,
                               sizeof(szType) );
    }


    // Spit out the spacing for the level indentation
    for ( i=0; i < level; i++ )
        sprtf("    ");

    sprtf(
        "ResDir (%s) Entries:%02X (Named:%02X, ID:%02X) TimeDate:%08X",
        szType, resDir->NumberOfNamedEntries+ resDir->NumberOfIdEntries,
        resDir->NumberOfNamedEntries, resDir->NumberOfIdEntries,
        resDir->TimeDateStamp );
        
	if ( resDir->MajorVersion || resDir->MinorVersion )
		sprtf( " Vers:%u.%02u", resDir->MajorVersion, resDir->MinorVersion );
	if ( resDir->Characteristics)
		sprtf( " Char:%08X", resDir->Characteristics );

	//
	// The "directory entries" immediately follow the directory in memory
	//
    resDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(resDir+1);

	// If it's a stringtable, save off info for future use
	if ( level == 1 && (resourceType == (WORD)RT_STRING))
	{
		pStrResEntries = resDirEntry;
		cStrResEntries = resDir->NumberOfIdEntries;
        sprtf(" S");
	}

	// If it's a stringtable, save off info for future use
	if ( level == 1 && (resourceType == (WORD)RT_DIALOG))
	{
		pDlgResEntries = resDirEntry;
		cDlgResEntries = resDir->NumberOfIdEntries;
        sprtf(" D");
	}

	// If it's a VERSION block, save off info for future use
	if ( level == 1 && (resourceType == (WORD)RT_VERSION))
	{
		pVerResEntries = resDirEntry;
		cVerResEntries = resDir->NumberOfIdEntries;
        sprtf(" V");
	}

	sprtf( "\n" );
	    
    for ( i=0; i < resDir->NumberOfNamedEntries; i++, resDirEntry++ )
        DumpResourceEntry(resDirEntry, resourceBase, level+1, resourceType, baseResType, base, pNTHeader);

    for ( i=0; i < resDir->NumberOfIdEntries; i++, resDirEntry++ ) {
        DumpResourceEntry(resDirEntry, resourceBase, level+1, resourceType, baseResType, base, pNTHeader);
    }

}

/* 
from : http://www.devsource.com/c/a/Architecture/Resources-From-PE-I/1/
typedef struct _IMAGE_RESOURCE_DIRECTORY 
{
    DWORD   Characteristics; // Not important to resource retrieval.
    DWORD   TimeDateStamp;   // Not important to resource retrieval.
    WORD    MajorVersion;    // Not important to resource retrieval.
    WORD    MinorVersion;    // Not important to resource retrieval.
    WORD    NumberOfNamedEntries;
    WORD    NumberOfIdEntries;
    //  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;
typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY 
{
    DWORD Name;
    DWORD OffsetToData;
}IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY
The Name field stores an integer ID if its high bit is 0, or an offset (in the lower 31 bits) 
to an IMAGE_RESOURCE_DIR_STRING_U structure if its high bit is 1. The offset is relative to 
the start of the resource section, and this structure identifies a Unicode string that 
names a resource instance: 

typedef struct _IMAGE_RESOURCE_DIR_STRING_U 
{
    WORD    Length;        // Number of Unicode characters in string
    WCHAR   NameString[1]; // Length Unicode characters
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;
The OffsetToData field stores an offset to an IMAGE_RESOURCE_DATA_ENTRY structure 
(which I'll discuss later) if its high bit is 0, or an offset to another 
IMAGE_RESOURCE_DIRECTORY structure if its high bit is 1. Both of these offsets are 
relative to the beginning of the resource section. 

For each of the root directory's resource directory entries, Name contains the integer 
ID of a resource type directory -- I've never seen this directory given a string name. 
Also, OffsetToData contains an offset to the resource type directory's 
IMAGE_RESOURCE_DIRECTORY structure. Table 1 lists possible integer IDs for Name. 

16  RT_VERSION 

Resource type directories are like classes in that they describe categories of 
resources (such as bitmaps, dialogs, and menus) that appear in the resource section. 
The resources themselves are described by resource instance directories, which are 
pointed to from the resource type directories. 

For each of a resource type directory's resource directory entries, Name contains 
the integer ID or offset to the string name of a resource instance directory -- 
see Figure 2 for examples. Furthermore, OffsetToData contains an offset to the 
resource instance directory's IMAGE_RESOURCE_DIRECTORY structure. 

Unlike a resource type directory, which can have multiple resource directory entries, 
a resource instance directory has only one entry. Furthermore, this entry's OffsetToData 
field will always have its most-significant bit clear, which causes it to reference 
an IMAGE_RESOURCE_DATA_ENTRY structure: 

typedef struct _IMAGE_RESOURCE_DATA_ENTRY 
{
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage; // Possibly important to resource retrieval.
    DWORD   Reserved; // Not important to resource retrieval.
}IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

The OffsetToData and Size fields specify the location (as a relative virtual address 
within the resource section) and size (in bytes) of the resource data. Although an 
RVA is not the same as a file offset, the equivalent file offset can be calculated 
by subtracting the resource section's RVA from OffsetToData's RVA value, and adding 
the difference to the offset of the root directory. 

RT_BITMAP 
The RT_BITMAP resource type describes the resource data as a single device-independent 
bitmap. The bitmap is stored as a Windows BITMAPINFOHEADER structure or OS/2 BITMAPCOREHEADER 
structure, optionally followed by a color table (a sequence of RGBQUAD structures for 
Windows; a sequence of RGBTRIPLE structures for OS/2), followed by the bitmap image data. 

It doesn't matter to the utility program whether the resource data contains BITMAPINFOHEADER 
and RGBQUAD structures, or BITMAPCOREHEADER and RGBTRIPLE structures. However, the absence 
of a BITMAPFILEHEADER structure is an impediment to the program -- this structure must be 
present at the start of a .bmp file. 

Before writing the resource data to the .bmp file, the utility program creates a 
BITMAPFILEHEADER structure and initializes the structure's fields appropriately 
(add the size of the BITMAPFILEHEADER structure to the size of the resource data, and 
assign the sum to the structure's bfSize field, for example). The program then writes 
this structure to the file. 

RT_GROUP_CURSOR and RT_CURSOR 
A resource script statement for a cursor resource (MyCursor CURSOR "mycursor.cur", 
for example) is described in a PE file via RT_GROUP_CURSOR and RT_CURSOR resource 
types. The former resource type stores a directory in the resource data. This directory 
identifies the various images comprising the cursor resource, and begins with a 
header structure: 

typedef struct
{
    WORD wReserved;  // Always 0
    WORD wResID;     // Always 2
    WORD wNumImages; // Number of cursor images/directory entries
} CURHEADER;
This header, which also appears at the beginning of a .cur file (and is written "as is" 
to a .cur file by the utility program), identifies the number of cursor images that are 
stored elsewhere in the resource data. Each cursor image is described by one of the entries 
in the array of cursor directory entry structures that follows the header: 

typedef struct
{
    WORD  wWidth;
    WORD  wHeight; // Divide by 2 to get the actual height.
    WORD  wPlanes;
    WORD  wBitCount;
    DWORD dwBytesInImage;
    WORD  wID;
} CURDIRENTRY;
Most of the directory entry's fields present values for interpreting the image data. 
The wID field provides an integer ID that the utility program uses to find the 
appropriate RT_CURSOR resource, which stores the actual cursor image in the resource 
data. The Name field in the only IMAGE_RESOURCE_DIRECTORY_ENTRY for this resource's 
instance resource directory stores the cursor image's ID. 

RT_CURSOR describes a single cursor image's resource data as a two-byte x hotspot 
value, followed by a two-byte y hotspot value. A BITMAPINFOHEADER structure followed 
by a color table comes next -- I've yet to encounter a cursor image with 24-bit color. 
The resource data next stores the image's XOR bitmap followed by its AND bitmap. 
These two bitmaps are used together to support transparency. 

To recreate the .cur file, the utility program first writes the values that are stored 
in the CURHEADER structure, located at the beginning of RT_GROUP_CURSOR's resource data, 
to the file. The program then converts each of the CURDIRENTRY structures and associated 
hotspot values to an equivalent CURSORDIRENTRY structure: 

typedef struct
{
    BYTE  bWidth;
    BYTE  bHeight;        // Set to CURDIRENTRY.wHeight/2.
    BYTE  bColorCount;
    BYTE  bReserved;
    WORD  wHotspotX;
    WORD  wHotspotY;
    DWORD dwBytesInImage;
    DWORD dwImageOffset;  // Offset from start of header to the image
} CURSORDIRENTRY;
After calculating the offset to a cursor image and assigning this value to the 
dwImageOffset field in the associated CURSORDIRENTRY structure, the utility program 
writes the CURSORDIRENTRY structure to the .cur file. After writing these structures, 
it writes all of the cursor images' resource data areas (without their hotspot 
fields) to the file. 

RT_GROUP_ICON and RT_ICON 
Icons are organized similarly to cursors. A resource script statement for an icon 
resource (MyIcon ICON "myicon.ico", for example) is described in a PE file via 
RT_GROUP_ICON and RT_ICON resource types. Although there are many similarities 
between icon and cursor resources, there are also differences, beginning with 
the following structures: 

typedef struct
{
    WORD wReserved;  // Always 0
    WORD wResID;     // Always 1
    WORD wNumImages; // Number of icon images/directory entries
} ICOHEADER;

typedef struct
{
    BYTE  bWidth;
    BYTE  bHeight;
    BYTE  bColors;
    BYTE  bReserved;
    WORD  wPlanes;
    WORD  wBitCount;
    DWORD dwBytesInImage;
    WORD  wID;
} ICODIRENTRY;
As with .cur files, the header appears at the beginning of .ico files, but with 
1 as its resource ID. Also, the ICODIRENTRY structure differs from its 
CURDIRENTRY structure counterpart in terms of new fields (bColors and bReserved) 
and field sizes (BYTE instead of WORD for the width and height fields). 

RT_ICON is similar to RT_CURSOR in that it stores a single image, but 
without hotspot information. The resource data typically begins with a 
BITMAPINFOHEADER structure, often followed by a color table, followed by 
XOR and AND structures. However, if the image is specified via the 
Portable Network Graphics format, the image's resource data consists 
entirely of PNG information. 

To recreate the .ico file, the utility program first writes the values 
that are stored in the ICOHEADER structure, located at the beginning of 
RT_GROUP_ICON's resource data, to the file. The program then converts each 
of the ICODIRENTRY structures to an equivalent ICONDIRENTRY structure: 

typedef struct
{
    BYTE  bWidth;
    BYTE  bHeight;
    BYTE  bColorCount;
    BYTE  bReserved;
    WORD  wPlanes;
    WORD  wBitCount;
    DWORD dwBytesInImage;
    DWORD dwImageOffset; // Offset from start of header to the image
} ICONDIRENTRY;
After calculating the offset to an icon image and assigning this value 
to the dwImageOffset field in the associated ICONDIRENTRY structure, 
the utility program writes the ICONDIRENTRY structure to the .ico file. 
After writing these structures, it writes all of the icon images' 
resource data areas to the file. 

Oops, nothing about RT_VERSION - UGH!

*/
void Dump_RT_VERSION( PIMAGE_RESOURCE_DATA_ENTRY pResDataEntry, 
    DWORD resourceBase, unsigned char *cp, DWORD Size )
{
    DWORD i, off, c;
    unsigned char *end;
    DWORD   Offset   = pResDataEntry->OffsetToData;
    //DWORD   Size     = pResDataEntry->Size;
    DWORD   CodePage = pResDataEntry->CodePage; // Possibly important to resource retrieval.
    sprtf("Contents of a RT_VERSION block, RVA %08x, size %u, codepage %d\n",
        Offset, Size, CodePage);
    end = cp + Size;
    off = 0;
    for (i = 0; i < Size; i++) {
        c = cp[i];
        sprtf("%02x ", c);
        off++;
        if (off == 16) {
            off = 0;
            i -= 15;
            for (; i < Size; i++) {
                c = cp[i];
                if ((c < ' ') || (c & 0x80))
                    c = '.';
                sprtf("%c", (char)c);
                off++;
                if (off == 16) {
                    sprtf("\n");
                    break;
                }
            }
            off = 0;
        }
    }
    if (off) {
        i -= off;
        while(off < 16) {
            sprtf("   ");
            off++;
        }
        off = 0;
        for (; i < Size; i++) {
            c = cp[i];
            if ((c < ' ') || (c & 0x80))
                c = '.';
            sprtf("%c", (char)c);
            off++;
            if (off == 16)
                break;
        }
        sprtf("\n");
    }
}

//
// Dump the information about one resource directory entry.  If the
// entry is for a subdirectory, call the directory dumping routine
// instead of printing information in this routine.
//
void DumpResourceEntry
(
    PIMAGE_RESOURCE_DIRECTORY_ENTRY resDirEntry,
    DWORD resourceBase,
    DWORD level,
    DWORD resourceType,
    DWORD baseResType,
    char *base, 
    PIMAGE_NT_HEADERS pNTHeader
)
{
    UINT i;
    char nameBuffer[128];
    PIMAGE_RESOURCE_DATA_ENTRY pResDataEntry;
    DWORD offset = resourceBase + (resDirEntry->OffsetToData & 0x7FFFFFFF);
    unsigned char *ucptr = (unsigned char *)pNTHeader;
    ucptr += offset;
    if ( resDirEntry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY )
    {
        if ((ucptr + sizeof(PIMAGE_RESOURCE_DIRECTORY)) >= pBaseTop) {
            sprtf("POINTER %p out of range %p!\n", ucptr, pBaseTop);
            return;
        }
        DumpResourceDirectory( (PIMAGE_RESOURCE_DIRECTORY)
            ((resDirEntry->OffsetToData & 0x7FFFFFFF) + resourceBase),
            resourceBase, level, resDirEntry->Name, resourceType, base, pNTHeader);
        return;
    }

    // Spit out the spacing for the level indentation
    for ( i=0; i < level; i++ )
        sprtf("    ");

    if ( resDirEntry->Name & IMAGE_RESOURCE_NAME_IS_STRING )
    {
        GetResourceNameFromId(resDirEntry->Name, resourceBase, nameBuffer,
                              sizeof(nameBuffer));
        sprtf("Name: %s  DataEntryOffs: %08X\n",
            nameBuffer, resDirEntry->OffsetToData);
    }
    else
    {
        sprtf("ID: %08X  DataEntryOffs: %08X\n",
                resDirEntry->Name, resDirEntry->OffsetToData);
    }
    
    // the resDirEntry->OffsetToData is a pointer to an
    // IMAGE_RESOURCE_DATA_ENTRY.  Go dump out that information.  First,
    // spit out the proper indentation
    for ( i=0; i < level; i++ )
        sprtf("    ");
    
    pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                    (resourceBase + resDirEntry->OffsetToData);
    ucptr = (unsigned char *)pResDataEntry;
    if (ucptr >= pBaseTop) {
        sprtf("Pointer %p out of range %p!\n", ucptr, pBaseTop);
        return;
    }
    sprtf("DataRVA: %05X  DataSize: %05X  CodePage: %X\n",
            pResDataEntry->OffsetToData, pResDataEntry->Size,
            pResDataEntry->CodePage);
    if (baseResType == (DWORD)RT_VERSION) {
        // Although an RVA is not the same as a file offset, 
        // the equivalent file offset can be calculated 
        // by subtracting the resource section's RVA from OffsetToData's 
        // RVA value, and adding the difference to the offset of the 
        // root directory. 
        unsigned char *cp = 0; // (unsigned char *)pResDataEntry;
        DWORD size = pResDataEntry->Size;
        DWORD resourcesRVA = GetImgDirEntryRVA(pNTHeader, IMAGE_DIRECTORY_ENTRY_RESOURCE);
        if ( resourcesRVA ) {
            PIMAGE_RESOURCE_DIRECTORY pird = (PIMAGE_RESOURCE_DIRECTORY)
                GetPtrFromRVA( resourcesRVA, pNTHeader, base );
            if (pird) {
                cp = (unsigned char *) GetPtrFromRVA( pResDataEntry->OffsetToData, pNTHeader, base );
            }
        }
        if ( !cp ) {
            cp = (unsigned char *)pResDataEntry;
            cp += resDirEntry->OffsetToData;
            size -= resDirEntry->OffsetToData;
        }
        // DWORD offset = pResDataEntry->OffsetToData - resDirEntry->OffsetToData;
        Dump_RT_VERSION(pResDataEntry, resourceBase, cp, size);
    }

}


// Get an ASCII string representing a resource type
void GetResourceTypeName(DWORD type, PSTR buffer, UINT cBytes)
{
    if ( type <= (WORD)RT_ANIICON )
        strncpy(buffer, SzResourceTypes[type], cBytes);
    else
        sprintf(buffer, "%X", type);
}

DWORD GetOffsetToDataFromResEntry( 	char *base,
									DWORD resourceBase,
									PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry )
{
	// The IMAGE_RESOURCE_DIRECTORY_ENTRY is gonna point to a single
	// IMAGE_RESOURCE_DIRECTORY, which in turn will point to the
	// IMAGE_RESOURCE_DIRECTORY_ENTRY, which in turn will point
	// to the IMAGE_RESOURCE_DATA_ENTRY that we're really after.  In
	// other words, traverse down a level.

	PIMAGE_RESOURCE_DIRECTORY pStupidResDir;
	pStupidResDir = (PIMAGE_RESOURCE_DIRECTORY)
                    (resourceBase + pResEntry->OffsetToDirectory);

    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry =
	    	(PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pStupidResDir + 1);// PTR MATH

	PIMAGE_RESOURCE_DATA_ENTRY pResDataEntry =
			(PIMAGE_RESOURCE_DATA_ENTRY)(resourceBase +
										 pResDirEntry->OffsetToData);

	return pResDataEntry->OffsetToData;
}

void DumpStringTable( 	char *base,
						PIMAGE_NT_HEADERS pNTHeader,
						DWORD resourceBase,
						PIMAGE_RESOURCE_DIRECTORY_ENTRY pStrResEntry,
						DWORD cStrResEntries )
{
	for ( unsigned i = 0; i < cStrResEntries; i++, pStrResEntry++ )
	{
		DWORD offsetToData
			= GetOffsetToDataFromResEntry( base, resourceBase, pStrResEntry );
			
 		PWORD pStrEntry = (PWORD)GetPtrFromRVA(	offsetToData,
												pNTHeader, base );
		if ( !pStrEntry)
			break;
		
		unsigned id = (pStrResEntry->Name - 1) << 4;

		for ( unsigned j = 0; j < 16; j++ )
		{
			WORD len = *pStrEntry++;
			if ( len )
			{
				sprtf( "%-5u: ", id + j );

				for ( unsigned k = 0; k < min(len, (WORD)64); k++ )
				{
					char * s;
					char szBuff[20];
					char c = (char)pStrEntry[k];
					switch( c )
					{
						case '\t': s = "\\t"; break;
						case '\r': s = "\\r"; break;
						case '\n': s = "\\n"; break;
						default:
							wsprintf( szBuff, "%c", isprint(c) ? c : '.' );
							s=szBuff;
							break;
					}

					sprtf( s );
				}

				sprtf( "\n" );
			}

			pStrEntry += len;
		}
	}
}

void DumpDialogs( 	char *base,
					PIMAGE_NT_HEADERS pNTHeader,
					DWORD resourceBase,
					PIMAGE_RESOURCE_DIRECTORY_ENTRY pDlgResEntry,
					DWORD cDlgResEntries )
{
	for ( unsigned i = 0; i < cDlgResEntries; i++, pDlgResEntry++ )
	{
		DWORD offsetToData
			= GetOffsetToDataFromResEntry( base, resourceBase, pDlgResEntry );
			
 		PDWORD pDlgStyle = (PDWORD)GetPtrFromRVA(	offsetToData,
													pNTHeader, base );
		if ( !pDlgStyle )
			break;
													
		sprtf( "  ====================\n" );
		if ( HIWORD(*pDlgStyle) != 0xFFFF )
		{
			//	A regular DLGTEMPLATE
			DLGTEMPLATE * pDlgTemplate = ( DLGTEMPLATE * )pDlgStyle;

			sprtf( "  style: %08X\n", pDlgTemplate->style );			
			sprtf( "  extended style: %08X\n", pDlgTemplate->dwExtendedStyle );			

			sprtf( "  controls: %u\n", pDlgTemplate->cdit );
			sprtf( "  (%u,%u) - (%u,%u)\n",
						pDlgTemplate->x, pDlgTemplate->y,
						pDlgTemplate->x + pDlgTemplate->cx,
						pDlgTemplate->y + pDlgTemplate->cy );
			PWORD pMenu = (PWORD)(pDlgTemplate + 1);	// ptr math!

			//
			// First comes the menu
			//
			if ( *pMenu )
			{
				if ( 0xFFFF == *pMenu )
				{
					pMenu++;
					sprtf( "  ordinal menu: %u\n", *pMenu );
				}
				else
				{
					sprtf( "  menu: " );
					while ( *pMenu )
						sprtf( "%c", LOBYTE(*pMenu++) );				

					pMenu++;
					sprtf( "\n" );
				}
			}
			else
				pMenu++;	// Advance past the menu name

			//
			// Next comes the class
			//			
			PWORD pClass = pMenu;
						
			if ( *pClass )
			{
				if ( 0xFFFF == *pClass )
				{
					pClass++;
					sprtf( "  ordinal class: %u\n", *pClass );
				}
				else
				{
					sprtf( "  class: " );
					while ( *pClass )
					{
						sprtf( "%c", LOBYTE(*pClass++) );				
					}		
					pClass++;
					sprtf( "\n" );
				}
			}
			else
				pClass++;	// Advance past the class name
			
			//
			// Finally comes the title
			//

			PWORD pTitle = pClass;
			if ( *pTitle )
			{
				sprtf( "  title: " );

				while ( *pTitle )
					sprtf( "%c", LOBYTE(*pTitle++) );
					
				pTitle++;
			}
			else
				pTitle++;	// Advance past the Title name

			sprtf( "\n" );

			PWORD pFont = pTitle;
						
			if ( pDlgTemplate->style & DS_SETFONT )
			{
				sprtf( "  Font: %u point ",  *pFont++ );
				while ( *pFont )
					sprtf( "%c", LOBYTE(*pFont++) );

				pFont++;
				sprtf( "\n" );
			}
	        else
    	        pFont = pTitle; 

			// DLGITEMPLATE starts on a 4 byte boundary
			LPDLGITEMTEMPLATE pDlgItemTemplate = (LPDLGITEMTEMPLATE)pFont;
			
			for ( unsigned i=0; i < pDlgTemplate->cdit; i++ )
			{
				// Control item header....
				pDlgItemTemplate = (DLGITEMTEMPLATE *)
									(((DWORD)pDlgItemTemplate+3) & ~3);
				
				sprtf( "    style: %08X\n", pDlgItemTemplate->style );			
				sprtf( "    extended style: %08X\n",
						pDlgItemTemplate->dwExtendedStyle );			

				sprtf( "    (%u,%u) - (%u,%u)\n",
							pDlgItemTemplate->x, pDlgItemTemplate->y,
							pDlgItemTemplate->x + pDlgItemTemplate->cx,
							pDlgItemTemplate->y + pDlgItemTemplate->cy );
				sprtf( "    id: %u\n", pDlgItemTemplate->id );
				
				//
				// Next comes the control's class name or ID
				//			
				PWORD pClass = (PWORD)(pDlgItemTemplate + 1);
				if ( *pClass )
				{							
					if ( 0xFFFF == *pClass )
					{
						pClass++;
						sprtf( "    ordinal class: %u", *pClass++ );
					}
					else
					{
						sprtf( "    class: " );
						while ( *pClass )
							sprtf( "%c", LOBYTE(*pClass++) );

						pClass++;
						sprtf( "\n" );
					}
				}
				else
					pClass++;
					
				sprtf( "\n" );			

				//
				// next comes the title
				//

				PWORD pTitle = pClass;
				
				if ( *pTitle )
				{
					sprtf( "    title: " );
					if ( 0xFFFF == *pTitle )
					{
						pTitle++;
						sprtf( "%u\n", *pTitle++ );
					}
					else
					{
						while ( *pTitle )
							sprtf( "%c", LOBYTE(*pTitle++) );
						pTitle++;
						sprtf( "\n" );
					}
				}
				else	
					pTitle++;	// Advance past the Title name

				sprtf( "\n" );
				
				PBYTE pCreationData = (PBYTE)(((DWORD)pTitle + 1) & 0xFFFFFFFE);
				
				if ( *pCreationData )
					pCreationData += *pCreationData;
				else
					pCreationData++;

				pDlgItemTemplate = (DLGITEMTEMPLATE *)pCreationData;	
				
				sprtf( "\n" );
			}
			
			sprtf( "\n" );
		}
		else
		{
			// A DLGTEMPLATEEX		
		}
		
		sprtf( "\n" );
	}
}


//
// Top level routine called to dump out the entire resource hierarchy
//
void DumpResourceSection(char *base, PIMAGE_NT_HEADERS pNTHeader)
{
	DWORD resourcesRVA;
    PIMAGE_RESOURCE_DIRECTORY resDir;

	resourcesRVA = GetImgDirEntryRVA(pNTHeader, IMAGE_DIRECTORY_ENTRY_RESOURCE);
	if ( !resourcesRVA )
		return;

    resDir = (PIMAGE_RESOURCE_DIRECTORY)
    		GetPtrFromRVA( resourcesRVA, pNTHeader, base );

	if ( !resDir )
		return;
		
    sprtf("Resources (RVA: %X)\n", resourcesRVA );

    DumpResourceDirectory(resDir, (DWORD)resDir, 0, 0, 0, base, pNTHeader);

	sprtf( "\n" );

	if ( !fShowResources )
		return;
		
	if ( cStrResEntries )
	{
		sprtf( "String Table\n" );

		DumpStringTable( 	base, pNTHeader, (DWORD)resDir,
							pStrResEntries, cStrResEntries );
		sprtf( "\n" );
	}

	if ( cDlgResEntries )
	{
		sprtf( "Dialogs\n" );

		DumpDialogs( 	base, pNTHeader, (DWORD)resDir,
						pDlgResEntries, cDlgResEntries );
		sprtf( "\n" );
	}
}

#define ADD_COFF_HEADER
#define ADD_DUMP_MISC_DEBUG

void DumpMiscDebugInfo( PIMAGE_DEBUG_MISC pMiscDebugInfo )
{
	if ( IMAGE_DEBUG_MISC_EXENAME != pMiscDebugInfo->DataType )
	{
		sprtf( "Unknown Miscellaneous Debug Information type: %u\n", 
				pMiscDebugInfo->DataType );
		return;
	}

	sprtf( "Miscellaneous Debug Information\n" );
	sprtf( "  %s\n", pMiscDebugInfo->Data );
}

#define ADD_DUMP_CV_DEBUG

typedef struct _PDB_INFO {
    CHAR    Signature[4];   // "NBxx"
    ULONG   Offset;         // always zero
    ULONG   sig;
    ULONG   age;
    CHAR    PdbName[_MAX_PATH];
} PDB_INFO, *PPDB_INFO;

void DumpCVDebugInfo( PDWORD pCVHeader )
{
	PPDB_INFO pPDBInfo;

	sprtf( "CodeView Signature: %08X\n", *pCVHeader );

	if ( '01BN' != *pCVHeader )
	{
		sprtf( "Unhandled CodeView Information format %.4s\n", pCVHeader );
		return;
	}

	pPDBInfo = (PPDB_INFO)pCVHeader;

	sprtf( "  Offset: %08X  Signature: %08X  Age: %08X\n",
			pPDBInfo->Offset, pPDBInfo->sig, pPDBInfo->age );
	sprtf( "  File: %s\n", pPDBInfo->PdbName );
}

#define ADD_DUMP_COFF_HEADER

//
// Dump the COFF debug information header
//
void DumpCOFFHeader(PIMAGE_COFF_SYMBOLS_HEADER pDbgInfo)
{
    sprtf("COFF Debug Info Header\n");
    sprtf("  NumberOfSymbols:      %08X\n", pDbgInfo->NumberOfSymbols);
    sprtf("  LvaToFirstSymbol:     %08X\n", pDbgInfo->LvaToFirstSymbol);
    sprtf("  NumberOfLinenumbers:  %08X\n", pDbgInfo->NumberOfLinenumbers);
    sprtf("  LvaToFirstLinenumber: %08X\n", pDbgInfo->LvaToFirstLinenumber);
    sprtf("  RvaToFirstByteOfCode: %08X\n", pDbgInfo->RvaToFirstByteOfCode);
    sprtf("  RvaToLastByteOfCode:  %08X\n", pDbgInfo->RvaToLastByteOfCode);
    sprtf("  RvaToFirstByteOfData: %08X\n", pDbgInfo->RvaToFirstByteOfData);
    sprtf("  RvaToLastByteOfData:  %08X\n", pDbgInfo->RvaToLastByteOfData);
}

#define ADD_DUMP_LINE_NUMBERS

//
// Given a COFF symbol table index, look up its name.  This function assumes
// that the COFFSymbolCount and PCOFFSymbolTable variables have been already
// set up.
//
BOOL LookupSymbolName(DWORD index, PSTR buffer, UINT length)
{
	if ( !g_pCOFFSymbolTable )
		return FALSE;

	PCOFFSymbol pSymbol = g_pCOFFSymbolTable->GetSymbolFromIndex( index );

	if ( !pSymbol )
		return FALSE;
    PSTR pstr = pSymbol->GetName();
    if (!pstr) {
        delete pSymbol;
        return FALSE;
    }
    if (out_of_top_range((unsigned char *)pstr) ||
        out_of_top_range((unsigned char *)(pstr + length))) {
            delete pSymbol;
            return FALSE;
    }

	lstrcpyn( buffer, pstr, length );

	delete pSymbol;

    return TRUE;
}

//
// Dump a range of line numbers from the COFF debug information
//
void DumpLineNumbers(PIMAGE_LINENUMBER pln, DWORD count)
{
    char buffer[64];
    DWORD i;
    PIMAGE_LINENUMBER pln2 = pln + 1;
    if (out_of_top_range((unsigned char *)pln2)) {
        sprtf("WARNING: Line Number pointer starts out of range! Aborting...\n");
        return;
    }
    pln2 = pln;
    for (i = 0; i < count; i++)
    {
        if (out_of_top_range((unsigned char *)pln2)) {
            sprtf("WARNING: Line Number pointer goes out of range on count %d of %d! Aborting...\n",
                i, count);
            return;
        }
        pln2++;
    }

    sprtf("Line Numbers - count %d\n", count);
    
    for (i=0; i < count; i++)
    {
        pln2 = pln + 1;
        if (out_of_top_range((unsigned char *)pln2)) {
            sprtf("Line Number pointer out of range! Aborting...\n");
            return;
        }

        if ( pln->Linenumber == 0 ) // A symbol table index
        {
            buffer[0] = 0;
            if (LookupSymbolName(pln->Type.SymbolTableIndex, buffer,
                sizeof(buffer))) {
                sprtf("SymIndex: %X (%s)\n", pln->Type.SymbolTableIndex,
                                             buffer);
            }
        }
        else
        {
            // A regular line number
            sprtf(" Addr: %05X  Line: %04u\n",
                pln->Type.VirtualAddress, pln->Linenumber);
        }
        pln++;
    }
}

#define ADD_DUMP_SYMBOL_TABLE

//
// Used by the DumpSymbolTable() routine.  It purpose is to filter out
// the non-normal section numbers and give them meaningful names.
//
void GetSectionName(WORD section, PSTR buffer, unsigned cbBuffer)
{
    char tempbuffer[10];
    
    switch ( (SHORT)section )
    {
        case IMAGE_SYM_UNDEFINED: strcpy(tempbuffer, "UNDEF"); break;
        case IMAGE_SYM_ABSOLUTE:  strcpy(tempbuffer, "ABS"); break;
        case IMAGE_SYM_DEBUG:     strcpy(tempbuffer, "DEBUG"); break;
        default: sprintf(tempbuffer, "%X", section);
    }
    
    strncpy(buffer, tempbuffer, cbBuffer-1);
}

//
// Dumps a COFF symbol table from an EXE or OBJ
//
void DumpSymbolTable( PCOFFSymbolTable pSymTab )
{
	PCOFFSymbol pSymbol = pSymTab->GetNextSymbol( 0 );
    if (!pSymbol) {
        if (fShowSymbolTable)
            sprtf("Symbol Table appears out of range! Aborting listing...\n");
        return;
    }

    if (fShowSymbolTable) {
        sprtf( "Symbol Table - %X entries  (* = auxillary symbol)\n",
    		pSymTab->GetNumberOfSymbols() );
        sprtf(
            "Indx Sectn Value    Type  Storage  Name\n"
            "---- ----- -------- ----- -------  --------\n");
    }
    // 0000   ABS 00AA9D1B  0000 STATIC   @comp.id
    // 0002     1 00000000  0000 STATIC   .drectve
    // 0008     3 00001224  0000 STATIC   $SG83924 - can be LOTS OF THESE
    // 0277     4 00000000  0000 STATIC   _language_table - an exported table
    // 027A     4 00000998  0000 STATIC   ?categories@?3??libintl_setlocale@@9@9 - c++ mashing
    // 027D     5 00000000  0020 EXTERNAL _libintl_setlocale - an exported function
    // 027E     5 0000016A  0000 LABEL    $fail$84066 - a label?
    // 027F UNDEF 00000000  0000 EXTERNAL __imp__free - an imported function
    // 0280 UNDEF 00000000  0020 EXTERNAL _strcmp - another imported function, but how to know this
    // 0281 UNDEF 00000000  0020 EXTERNAL _gl_locale_name_default
    // 0293     5 0000028C  0000 STATIC   $LN13
    // 029E UNDEF 00000000  0000 EXTERNAL __imp__abort
    // 029F UNDEF 00000000  0020 EXTERNAL _strcpy
    // 02A0 UNDEF 00000000  0020 EXTERNAL _memcpy
    // 02A1 UNDEF 00000000  0000 EXTERNAL __imp__strchr
    // 02A2 UNDEF 00000000  0020 EXTERNAL _strlen
    // 02A3 UNDEF 00000000  0000 EXTERNAL ___security_cookie
    char *store, *name;
    PSTR tn;
    int value, index, c, count = 0;
    char szSection[24];
	while ( pSymbol )
	{
        count++;
        GetSectionName(pSymbol->GetSectionNumber(),szSection,sizeof(szSection));
        store = pSymbol->GetStorageClassName();
        name  = pSymbol->GetName();
        tn    = pSymbol->GetTypeName();
        value = pSymbol->GetValue();
        index = pSymbol->GetIndex();
        if (fShowSymbolTable)
            sprtf( "%04X %5s %08X  %s %-8s %s\n", index, szSection, value, tn, store, name );
        c = *name;
        if (VERB9 || ((strcmp(name,pBadName) != 0 )&&(c != '$')&&(c != '.'))) {
            if ( VERB9 ||
                (strcmp(store,"EXTERNAL") == 0) ||
                (strcmp(store,"STATIC") == 0) )
            {
                std::string s(name);
                add_to_objects(s);
            } else {
                skipped_objects++;
            }
        } else {
            skipped_objects++;
        }


		if ( fShowSymbolTable && pSymbol->GetNumberOfAuxSymbols() )
		{
			char szAuxSymbol[1024];
			if (pSymbol->GetAuxSymbolAsString(szAuxSymbol,sizeof(szAuxSymbol)))
				sprtf( "     * %s\n", szAuxSymbol );			
		}
		
		pSymbol = pSymTab->GetNextSymbol( pSymbol );

	}
    if (fShowSymbolTable && count)
        sprtf("Shown %d symbol tables...\n", count);
}

#define ADD_DUMP_RAW_SECTION_DATA

void HexDump(PBYTE ptr, DWORD length);

//
// Do a hexadecimal dump of the raw data for all the sections.  You
// could just dump one section by adjusting the PIMAGE_SECTION_HEADER
// and cSections parameters
//
void DumpRawSectionData(PIMAGE_SECTION_HEADER section,
                        PVOID base,
                        unsigned cSections)
{
   unsigned i, j;
   char name[IMAGE_SIZEOF_SHORT_NAME + 1];
   int   bad_cnt = Is_Section_Count_Bad( section, cSections );
   if (cSections == 0xffff) {
       // this looks BAD
       // FIX20120208 - Skip if count 0xFFFF in object
        PIMAGE_SECTION_HEADER psh = section;
        PIMAGE_SECTION_HEADER psh2;
        for ( i=1; i <= cSections; i++, psh++ ) {
            psh2 = psh + 1;
            if (out_of_top_range((unsigned char *)psh2)) {
                sprtf("Section Hex Dumps Sections - count = %d out of range! Abandoned...\n", cSections);
                return;
            }
            memcpy(name, psh->Name, IMAGE_SIZEOF_SHORT_NAME);
            name[IMAGE_SIZEOF_SHORT_NAME] = 0;
            if ( strcmp(name,".drectve") == 0 ) {
                // FIX20111001 - Seek .drectve, and SHOW more
                PBYTE ptr = MakePtr(PBYTE, base, psh->PointerToRawData);
                char * cp = GetNxtBuf();
                int off = 0;
                char c;
                sprtf("Linker Directives\n");
                for (j = 0; j < psh->SizeOfRawData; j++) {
                    c = ptr[j];
                    if (c <= ' ') {
                        if (off) {
                            cp[off] = 0;
                            sprtf("%s\n",cp);
                            off = 0;
                            if (strncmp(cp,"/DEFAULTLIB:",12) == 0) {
                                add_2_lib_list(cp);
                            }
                        }
                    } else {
                        cp[off++] = c;
                    }
                }
                if (off) {
                    cp[off] = 0;
                    sprtf("%s\n",cp);
                    off = 0;
                }
            }
        }
   }
   // FIX20111001 - Seek .drectve, and SHOW move
   sprtf("Section Hex Dumps Sections - count = %d\n", cSections);
    
    for ( i=1; i <= cSections; i++, section++ )
    {
       if ( bad_cnt && Is_Section_Blank( section ) )
       {
           if (VERB9) {  // 20120416: only if VERB9
              sprtf( "WARNING: Aborting on fully BLANK section #%d!\n", i );
           }
         break;
       }
        // Make a copy of the section name so that we can ensure that
        // it's null-terminated
        memcpy(name, section->Name, IMAGE_SIZEOF_SHORT_NAME);
        name[IMAGE_SIZEOF_SHORT_NAME] = 0;

        // Don't dump sections that don't exist in the file!
        if ( section->PointerToRawData == 0 )
            continue;
        
        sprtf( "section %02X (%s)  size: %08X  file offs: %08X\n",
                i, name, section->SizeOfRawData, section->PointerToRawData);

        HexDump( MakePtr(PBYTE, base, section->PointerToRawData),
                 section->SizeOfRawData );
        sprtf("\n");

        if ( strcmp(name,".drectve") == 0 ) {
            // FIX20111001 - Seek .drectve, and SHOW more
            PBYTE ptr = MakePtr(PBYTE, base, section->PointerToRawData);
            char * cp = GetNxtBuf();
            int off = 0;
            char c;
            sprtf("Linker Directives\n");
            for (j = 0; j < section->SizeOfRawData; j++) {
                c = ptr[j];
                if (c <= ' ') {
                    if (off) {
                        cp[off] = 0;
                        sprtf("%s\n",cp);
                        off = 0;
                        if (strncmp(cp,"/DEFAULTLIB:",12) == 0) {
                            add_2_lib_list(cp);
                        }
                    }
                } else {
                    cp[off++] = c;
                }
            }
            if (off) {
                cp[off] = 0;
                sprtf("%s\n",cp);
                off = 0;
            }
        }
    }
}

// as above, but NO hex output - only get /DEFAULTLIB list
void DumpRawSectionData2(PIMAGE_SECTION_HEADER section,
                        PVOID base,
                        unsigned cSections)
{
   unsigned i, j;
   char name[IMAGE_SIZEOF_SHORT_NAME + 1];
   int   bad_cnt = Is_Section_Count_Bad( section, cSections );

   // FIX20111001 - Seek .drectve, and SHOW move
    
    for ( i=1; i <= cSections; i++, section++ )
    {
       if ( bad_cnt && Is_Section_Blank( section ) )
       {
           if (VERB9) { // 20120416: only if VERB9
              sprtf( "WARNING: Aborting on fully BLANK section #%d!\n", i );
           }
         break;
       }
        // Make a copy of the section name so that we can ensure that
        // it's null-terminated
        memcpy(name, section->Name, IMAGE_SIZEOF_SHORT_NAME);
        name[IMAGE_SIZEOF_SHORT_NAME] = 0;

        // Don't dump sections that don't exist in the file!
        if ( section->PointerToRawData == 0 )
            continue;
        
        if ( strcmp(name,".drectve") == 0 ) {
            // FIX20111001 - Seek .drectve, and SHOW move
            PBYTE ptr = MakePtr(PBYTE, base, section->PointerToRawData);
            char * cp = GetNxtBuf();
            int off = 0;
            char c;
            for (j = 0; j < section->SizeOfRawData; j++) {
                c = ptr[j];
                if (c <= ' ') {
                    if (off) {
                        cp[off] = 0;
                        off = 0;
                        if (strncmp(cp,"/DEFAULTLIB:",12) == 0) {
                            add_2_lib_list(cp);
                        }
                    }
                } else {
                    cp[off++] = c;
                }
            }
            if (off) {
                cp[off] = 0;
                sprtf("%s\n",cp);
                off = 0;
            }
        }
    }
}

// Number of hex values displayed per line
#define HEX_DUMP_WIDTH 16

int Try_HD_Width( PBYTE ptr, DWORD len )
{
   BYTE a, b;
   int   iret = 0;
   __try
   {
      a = ptr[0];
      if(len)
         b = ptr[len - 1];
   }
   __except(TRUE)
   {
      iret = 1;
   }
   return iret;
}


//
// Dump a region of memory in a hexadecimal format
//
void HexDump(PBYTE ptr, DWORD length)
{
    char buffer[256];
    PSTR buffPtr, buffPtr2;
    unsigned cOutput, i;
    DWORD bytesToGo=length;

    while ( bytesToGo  )
    {
        cOutput = bytesToGo >= HEX_DUMP_WIDTH ? HEX_DUMP_WIDTH : bytesToGo;

        if( Try_HD_Width( ptr, cOutput ) )
        {
           sprtf( "WARNING: Abandoning HEX DUMP, bad ptr %p, length %d!\n",
               ptr, cOutput );
            return;
        }
        if (add_2_ptrlist(ptr)) {
            bytesToGo -= cOutput;
            ptr += HEX_DUMP_WIDTH;
            continue;
        }

        buffPtr = buffer;
        buffPtr += sprintf(buffPtr, "%08X:  ", length-bytesToGo );
        buffPtr2 = buffPtr + (HEX_DUMP_WIDTH * 3) + 1;
        
        for ( i=0; i < HEX_DUMP_WIDTH; i++ )
        {
            BYTE value = *(ptr+i);

            if ( i >= cOutput )
            {
                // On last line.  Pad with spaces
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
            }
            else
            {
                if ( value < 0x10 )
                {
                    *buffPtr++ = '0';
                    itoa( value, buffPtr++, 16);
                }
                else
                {
                    itoa( value, buffPtr, 16);
                    buffPtr+=2;
                }
 
                *buffPtr++ = ' ';
                *buffPtr2++ = isprint(value) ? value : '.';
                if ( value == '%' )
                   *buffPtr2++ = '%';  // insert another
            }
            
            // Put an extra space between the 1st and 2nd half of the bytes
            // on each line.
            if ( i == (HEX_DUMP_WIDTH/2)-1 )
                *buffPtr++ = ' ';
        }

        *buffPtr2 = 0;  // Null terminate it.
        //puts(buffer);   // Can't use sprtf(), since there may be a '%'
                        // in the string.
        sprtf("%s\n", buffer);   // Have add 2nd % if one found in ASCII

        bytesToGo -= cOutput;
        ptr += HEX_DUMP_WIDTH;
    }
}

#define ADD_COFF_SYMBOL_TABLE

//
// top level routine called from PEDUMP.C to dump the components of a PE file
//
void DumpExeFile( PIMAGE_DOS_HEADER dosHeader )
{
   PTSTR perror = NULL;
    PIMAGE_NT_HEADERS pNTHeader;
    char *base = (char *)dosHeader;
       
    pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
                                dosHeader->e_lfanew );

    // First, verify that the e_lfanew field gave us a reasonable
    // pointer, then verify the PE signature.
    if ( not_EXE_Header( pNTHeader, &perror ) ) {
       if(perror)
          sprtf( perror );
       return;
    }
#ifdef ADD_DUMP_HEADER    
    DumpHeader((PIMAGE_FILE_HEADER)&pNTHeader->FileHeader);
#endif // #ifdef ADD_DUMP_HEADER    

#ifdef ADD_DUMP_OPTIONAL
    DumpOptionalHeader((PIMAGE_OPTIONAL_HEADER)&pNTHeader->OptionalHeader);
#endif // #ifdef ADD_DUMP_OPTIONAL

#ifdef ADD_DUMP_SECTION_TABLE
    DumpSectionTable( IMAGE_FIRST_SECTION(pNTHeader), 
                        pNTHeader->FileHeader.NumberOfSections, TRUE);
    //sprtf("\n");
#endif // #ifdef ADD_DUMP_SECTION_TABLE

    DumpExeDebugDirectory(base, pNTHeader);

    if ( pNTHeader->FileHeader.PointerToSymbolTable == 0 )
        g_pCOFFHeader = 0; // Doesn't really exist!

#ifdef ADD_DUMP_RESOURCE_SECTION
    DumpResourceSection(base, pNTHeader);
    sprtf("\n");
#endif // #ifdef ADD_DUMP_RESOURCE_SECTION

    DumpImportsSection(base, pNTHeader);
    
    if ( GetImgDirEntryRVA( pNTHeader, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT) )
    {
        DumpBoundImportDescriptors( base, pNTHeader );
        sprtf( "\n" );
    }
    
    DumpExportsSection(base, pNTHeader);
    sprtf("\n");

	//=========================================================================
	//
	// If we have COFF symbols, create a symbol table now
	//
	//=========================================================================
#ifdef ADD_COFF_HEADER
	if ( g_pCOFFHeader )	// Did we see a COFF symbols header while looking
	{						// through the debug directory?
		g_pCOFFSymbolTable = new COFFSymbolTable(
				(PVOID)(base+ pNTHeader->FileHeader.PointerToSymbolTable),
				pNTHeader->FileHeader.NumberOfSymbols );
	}
#endif // #ifdef ADD_COFF_HEADER

	if ( fShowPDATA )
	{
		DumpRuntimeFunctions( base, pNTHeader );
		sprtf( "\n" );
	}

    if ( fShowRelocations )
    {
        DumpBaseRelocationsSection(base, pNTHeader);
        sprtf("\n");
    } 

	if ( fShowSymbolTable && g_pMiscDebugInfo )
	{
#ifdef   ADD_DUMP_MISC_DEBUG
		DumpMiscDebugInfo( g_pMiscDebugInfo );
		sprtf( "\n" );
#endif // #ifdef   ADD_DUMP_MISC_DEBUG
	}

	if ( fShowSymbolTable && g_pCVHeader )
	{
#ifdef ADD_DUMP_CV_DEBUG
		DumpCVDebugInfo( g_pCVHeader );
		sprtf( "\n" );
#endif // #ifdef ADD_DUMP_CV_DEBUG
	}

    if ( fShowSymbolTable && g_pCOFFHeader )
    {
#ifdef ADD_DUMP_COFF_HEADER
        DumpCOFFHeader( g_pCOFFHeader );
        sprtf("\n");
#endif // #ifdef ADD_DUMP_COFF_HEADER
    }
    
    if ( fShowLineNumbers && g_pCOFFHeader )
    {
#ifdef ADD_DUMP_LINE_NUMBERS
        DumpLineNumbers( MakePtr(PIMAGE_LINENUMBER, g_pCOFFHeader,
                            g_pCOFFHeader->LvaToFirstLinenumber),
                            g_pCOFFHeader->NumberOfLinenumbers);
        sprtf("\n");
#endif // #ifdef ADD_DUMP_LINE_NUMBERS
    }

    // if ( fShowSymbolTable )
    {
#ifdef ADD_DUMP_SYMBOL_TABLE
        if ( pNTHeader->FileHeader.NumberOfSymbols 
            && pNTHeader->FileHeader.PointerToSymbolTable
			&& g_pCOFFSymbolTable )
        {
            DumpSymbolTable( g_pCOFFSymbolTable );
            sprtf("\n");
        }
#endif // #ifdef ADD_DUMP_SYMBOL_TABLE
    }
    
    if ( fShowRawSectionData )
    {
#ifdef ADD_DUMP_RAW_SECTION_DATA
        DumpRawSectionData( (PIMAGE_SECTION_HEADER)(pNTHeader+1),
                            dosHeader,
                            pNTHeader->FileHeader.NumberOfSections);
#endif // #ifdef ADD_DUMP_RAW_SECTION_DATA
    }
    else
    {
#ifdef ADD_DUMP_RAW_SECTION_DATA
        // as above, but NO dump - only get /DEFAULTLIB, if any
        DumpRawSectionData2( (PIMAGE_SECTION_HEADER)(pNTHeader+1),
                            dosHeader,
                            pNTHeader->FileHeader.NumberOfSections);
#endif // #ifdef ADD_DUMP_RAW_SECTION_DATA
    }

#ifdef ADD_COFF_SYMBOL_TABLE
	if ( g_pCOFFSymbolTable )
		delete g_pCOFFSymbolTable;
#endif // #ifdef ADD_COFF_SYMBOL_TABLE

    show_dll_list();  // FIX20130225

    //if ( fDumpFollowImports )
    Process_Import_List();
}

#define ADD_DUMP_DBG_FILE

void DumpImageDbgHeader(PIMAGE_SEPARATE_DEBUG_HEADER pImageSepDbgHeader)
{
    UINT headerFieldWidth = 30;

    sprtf("  %-*s%04X\n", headerFieldWidth, "Flags:",
                pImageSepDbgHeader->Flags);
    sprtf("  %-*s%04X %s\n", headerFieldWidth, "Machine:",
                pImageSepDbgHeader->Machine,
                GetMachineTypeName(pImageSepDbgHeader->Machine));
    sprtf("  %-*s%04X\n", headerFieldWidth, "Characteristics:",
                pImageSepDbgHeader->Characteristics);
    sprtf("  %-*s%08X -> %s", headerFieldWidth, "TimeDateStamp:",
                pImageSepDbgHeader->TimeDateStamp,
                pedump_ctime((time_t *)&pImageSepDbgHeader->TimeDateStamp) );
    sprtf("  %-*s%08X\n", headerFieldWidth, "CheckSum:",
                pImageSepDbgHeader->CheckSum);
    sprtf("  %-*s%08X\n", headerFieldWidth, "ImageBase:",
                pImageSepDbgHeader->ImageBase);
    sprtf("  %-*s%08X\n", headerFieldWidth, "Size of Image:",
                pImageSepDbgHeader->SizeOfImage);
    sprtf("  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
                pImageSepDbgHeader->NumberOfSections);
    sprtf("  %-*s%04X\n", headerFieldWidth, "ExportedNamesSize:",
                pImageSepDbgHeader->ExportedNamesSize);
    sprtf("  %-*s%08X\n", headerFieldWidth, "DebugDirectorySize:",
                pImageSepDbgHeader->DebugDirectorySize);
    sprtf("  %-*s%08X\n", headerFieldWidth, "SectionAlignment:",
                pImageSepDbgHeader->SectionAlignment);
}

void DumpDbgFile( PIMAGE_SEPARATE_DEBUG_HEADER pImageSepDbgHeader )
{
    DumpImageDbgHeader(pImageSepDbgHeader);
    sprtf("\n");
    
    DumpSectionTable( (PIMAGE_SECTION_HEADER)(pImageSepDbgHeader+1),
                        pImageSepDbgHeader->NumberOfSections, TRUE);
                    
    DumpDebugDirectory(
        MakePtr(PIMAGE_DEBUG_DIRECTORY,
        pImageSepDbgHeader, sizeof(IMAGE_SEPARATE_DEBUG_HEADER) +
        (pImageSepDbgHeader->NumberOfSections * sizeof(IMAGE_SECTION_HEADER))
        + pImageSepDbgHeader->ExportedNamesSize),
        pImageSepDbgHeader->DebugDirectorySize,
        (char *)pImageSepDbgHeader);
    
   if ( g_pCOFFHeader )
	{
      DumpCOFFHeader( g_pCOFFHeader );
    
		sprtf("\n");

		g_pCOFFSymbolTable = new COFFSymbolTable(
			MakePtr( PVOID, g_pCOFFHeader, g_pCOFFHeader->LvaToFirstSymbol),
			g_pCOFFHeader->NumberOfSymbols );

		DumpSymbolTable( g_pCOFFSymbolTable );

		delete g_pCOFFSymbolTable;
	}
}

#define ADD_DUMP_OBJ_FILE

typedef struct _i386RelocTypes
{
    WORD type;
    PSTR name;
} i386RelocTypes;

// ASCII names for the various relocations used in i386 COFF OBJs
i386RelocTypes i386Relocations[] = 
{
{ IMAGE_REL_I386_ABSOLUTE, "ABSOLUTE" },
{ IMAGE_REL_I386_DIR16, "DIR16" },
{ IMAGE_REL_I386_REL16, "REL16" },
{ IMAGE_REL_I386_DIR32, "DIR32" },
{ IMAGE_REL_I386_DIR32NB, "DIR32NB" },
{ IMAGE_REL_I386_SEG12, "SEG12" },
{ IMAGE_REL_I386_SECTION, "SECTION" },
{ IMAGE_REL_I386_SECREL, "SECREL" },
{ IMAGE_REL_I386_REL32, "REL32" }
};
#define I386RELOCTYPECOUNT (sizeof(i386Relocations) / sizeof(i386RelocTypes))

//
// Given an i386 OBJ relocation type, return its ASCII name in a buffer
//
void GetObjRelocationName(WORD type, PSTR buffer, DWORD cBytes)
{
    DWORD i;
    
    for ( i=0; i < I386RELOCTYPECOUNT; i++ )
        if ( type == i386Relocations[i].type )
        {
            strncpy(buffer, i386Relocations[i].name, cBytes);
            return;
        }
        
    sprintf( buffer, "???_%X", type);
}


//
// Dump the relocation table for one COFF section
//
void DumpObjRelocations(PIMAGE_RELOCATION pRelocs, DWORD count)
{
    DWORD i;
    char szTypeName[32];
    PIMAGE_RELOCATION pR = pRelocs;
    for ( i=0; i < count; i++ ) {
        if (out_of_top_range((unsigned char *)pR)) {
            sprtf("With count of %d (0x%x), have out of range value! Aborting Relocations...\n",
                count, count );
            return;
        }
        pR++;
    }

    for ( i=0; i < count; i++ )
    {
        GetObjRelocationName(pRelocs->Type, szTypeName, sizeof(szTypeName));
        sprtf("  Address: %08X  SymIndex: %08X  Type: %s\n",
                pRelocs->VirtualAddress, pRelocs->SymbolTableIndex,
                szTypeName);
        pRelocs++;
    }
}

//
// top level routine called from PEDUMP.C to dump the components of a
// COFF OBJ file.
//
void DumpObjFile( PIMAGE_FILE_HEADER pImageFileHeader )
{
    unsigned i;
    PIMAGE_SECTION_HEADER pSections;
    PIMAGE_SECTION_HEADER section;
    int  bad_cnt;
    int out_of_range = 0;
    WORD    max = pImageFileHeader->NumberOfSections;

    DumpHeader(pImageFileHeader);

    pSections = MakePtr(PIMAGE_SECTION_HEADER, (pImageFileHeader+1),
                            pImageFileHeader->SizeOfOptionalHeader);

    if ( !(max == 0xffff) ) { // FIX20120208 - Skip if count 0xFFFF in object
        DumpSectionTable(pSections, pImageFileHeader->NumberOfSections, FALSE);
    //sprtf("\n");
    }

    i = pImageFileHeader->NumberOfSections;
    bad_cnt = Is_Section_Count_Bad( pSections, i );
    if ( fShowRelocations && ((i == 0xffff)||(max == 0xffff)) ) {
        // look like TROUBLE
        // NO IT IS TROUBLE
        // FIX20120208 - Skip if count 0xFFFF in object
        out_of_range = 1;
        //for ( i=0; i < pImageFileHeader->NumberOfSections; i++ )
        //{
        //   section = &pSections[i];
        //   if (out_of_top_range((unsigned char *)section)) {
        //        out_of_range = 1;
        //        break;
        //   }
        //   if (Is_Section_NV_Blank( section )) {
        //       break;
        //   }
        //}
        if (out_of_range) {
            sprtf("Skipping Show Relocations due to out of range...\n");
            return;
        }
    }
    if ( fShowRelocations && !out_of_range)
    {
        for ( i=0; i < pImageFileHeader->NumberOfSections; i++ )
        {
           section = &pSections[i];
           if( bad_cnt && Is_Section_NV_Blank( section ) )
           {
               sprtf("Section %02X Aborting as Name, Virtual null!\n", i);
               break;
           }
            if ( pSections[i].PointerToRelocations == 0 )
                continue;

        
            sprtf("Section %02X (%.8s) relocations\n", i, pSections[i].Name);
            DumpObjRelocations( MakePtr(PIMAGE_RELOCATION, pImageFileHeader,
                                    pSections[i].PointerToRelocations),
                                pSections[i].NumberOfRelocations );
            sprtf("\n");
        }
    }
     
    if ( pImageFileHeader->PointerToSymbolTable )
    {
		g_pCOFFSymbolTable = new COFFSymbolTable(
					MakePtr(PVOID, pImageFileHeader, 
							pImageFileHeader->PointerToSymbolTable),
					pImageFileHeader->NumberOfSymbols );

        DumpSymbolTable( g_pCOFFSymbolTable );
        if (fShowSymbolTable) sprtf("\n");
    }

    if ( fShowLineNumbers && !out_of_range)
    {
        // Walk through the section table...
        for (i=0; i < pImageFileHeader->NumberOfSections; i++)
        {
            // if there's any line numbers for this section, dump'em
            if (out_of_top_range((unsigned char *)pSections)) {
                  sprtf("WARNING: pSection out of range! Aborting show line numbers...(%d)\n", i+1);
                  break;
            }
           section = pSections;
           if (out_of_top_range((unsigned char *)section)) {
                  sprtf("WARNING: section out of range! Aborting show line numbers...(%d)\n", i+1);
                  break;
           }
           if( bad_cnt && Is_Section_NV_Blank( section ) )
           {
              sprtf("WARNING: Section %02X Aborting as Name, Virtual,... BLANK!\n", i+1);
               break;
           }
            if ( pSections->NumberOfLinenumbers )
            {
                DumpLineNumbers( MakePtr(PIMAGE_LINENUMBER, pImageFileHeader,
                                         pSections->PointerToLinenumbers),
                                 pSections->NumberOfLinenumbers );
                sprtf("\n");
            }
            pSections++;
        }
    }
    
    if ( fShowRawSectionData )
    {
        DumpRawSectionData( (PIMAGE_SECTION_HEADER)(pImageFileHeader+1),
                            pImageFileHeader,
                            pImageFileHeader->NumberOfSections);
    }
    else
    {
        DumpRawSectionData2( (PIMAGE_SECTION_HEADER)(pImageFileHeader+1),
                            pImageFileHeader,
                            pImageFileHeader->NumberOfSections);
    }

	delete g_pCOFFSymbolTable;
}

void DumpROMOptionalHeader( PIMAGE_ROM_OPTIONAL_HEADER pROMOptHdr )
{
    UINT width = 30;

    sprtf("Optional Header\n");
    
    sprtf("  %-*s%04X\n", width, "Magic", pROMOptHdr->Magic);
    sprtf("  %-*s%u.%02u\n", width, "linker version",
        pROMOptHdr->MajorLinkerVersion,
        pROMOptHdr->MinorLinkerVersion);
    sprtf("  %-*s%X\n", width, "size of code", pROMOptHdr->SizeOfCode);
    sprtf("  %-*s%X\n", width, "size of initialized data",
        pROMOptHdr->SizeOfInitializedData);
    sprtf("  %-*s%X\n", width, "size of uninitialized data",
        pROMOptHdr->SizeOfUninitializedData);
    sprtf("  %-*s%X\n", width, "entrypoint RVA",
        pROMOptHdr->AddressOfEntryPoint);
    sprtf("  %-*s%X\n", width, "base of code", pROMOptHdr->BaseOfCode);
    sprtf("  %-*s%X\n", width, "base of Bss", pROMOptHdr->BaseOfBss);
    sprtf("  %-*s%X\n", width, "GprMask", pROMOptHdr->GprMask);

	sprtf("  %-*s%X\n", width, "CprMask[0]", pROMOptHdr->CprMask[0] );
	sprtf("  %-*s%X\n", width, "CprMask[1]", pROMOptHdr->CprMask[1] );
	sprtf("  %-*s%X\n", width, "CprMask[2]", pROMOptHdr->CprMask[2] );
	sprtf("  %-*s%X\n", width, "CprMask[3]", pROMOptHdr->CprMask[3] );

    sprtf("  %-*s%X\n", width, "GpValue", pROMOptHdr->GpValue);
}

// VARIATION on the IMAGE_FIRST_SECTION macro from WINNT.H

#define IMAGE_FIRST_ROM_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((DWORD)ntheader +                                                  \
     FIELD_OFFSET( IMAGE_ROM_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_ROM_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))

void DumpROMImage( PIMAGE_ROM_HEADERS pROMHeader )
{
    DumpHeader(&pROMHeader->FileHeader);

    DumpROMOptionalHeader(&pROMHeader->OptionalHeader);
    sprtf("\n");

    DumpSectionTable( IMAGE_FIRST_ROM_SECTION(pROMHeader), 
                        pROMHeader->FileHeader.NumberOfSections, TRUE);
    //sprtf("\n");

	// Dump COFF symbols out here.  Get offsets from the header
}

#define ADD_DUMP_LIB_FILE

PSTR PszLongnames = 0;

// Routine to convert from big endian to little endian
DWORD ConvertBigEndian(DWORD bigEndian)
{
	DWORD temp = 0;

	// sprtf( "bigEndian: %08X\n", bigEndian );

	temp |= bigEndian >> 24;
	temp |= ((bigEndian & 0x00FF0000) >> 8);
	temp |= ((bigEndian & 0x0000FF00) << 8);
	temp |= ((bigEndian & 0x000000FF) << 24);

	return temp;
}

void DisplayArchiveMemberHeader(
    PIMAGE_ARCHIVE_MEMBER_HEADER pArchHeader,
    DWORD fileOffset )
{
    if ( fDumpExportsOnly )
        return;

    sprtf("Archive Member Header: At offset %d (%08X)\n", fileOffset, fileOffset);

    sprtf("  Name:     %.16s", pArchHeader->Name);
    if ( pArchHeader->Name[0] == '/' )
    {
       if( isdigit(pArchHeader->Name[1]) )
       {
          if( PszLongnames )
             sprtf( "  (%s)", PszLongnames + atoi((char *)pArchHeader->Name+1) );
          else
             sprtf( "  (Offset %d into PszLongnames(0))", atoi((char *)pArchHeader->Name+1));
       } else {
          sprtf( "  (no name, and no offset!)" );
       }
    }

    sprtf("\n");

	char szDateAsLong[64];
	sprintf( szDateAsLong, "%.12s", pArchHeader->Date );
	LONG dateAsLong = atol(szDateAsLong);
	
    sprtf("  Date:     %.12s %s", pArchHeader->Date, pedump_ctime((time_t *)&dateAsLong) );
    sprtf("  UserID:   %.6s\n", pArchHeader->UserID);
    sprtf("  GroupID:  %.6s\n", pArchHeader->GroupID);
    sprtf("  Mode:     %.8s\n", pArchHeader->Mode);
    sprtf("  Size:     %.10s\n", pArchHeader->Size);
}

// from : http://en.wikipedia.org/wiki/Name_mangling
// In software compiler engineering, name mangling (also called name decoration)
// is a technique used to solve various problems caused by the need to resolve 
// unique names for programming entities in many modern programming languages.
// Basic Structure
// All mangled C++ names start with ? (question mark). Because all mangled 
// C names start with alphanumeric characters, @ (at-sign) and _ (underscore),
// C++ names can be distinguished from C names.
// The structure of mangled names looks like this:
// Prefix ? - Optional: Prefix @? - Qualified name - Type information
// For example, we assume the following prototype.
// void __cdecl abc<def<int>,void*>::xyz(void);
// So the mangled name for this function is ?xyz@?$abc@V?$def@H@@PAX@@YAXXZ.
// NOTE: There is a windows FUNCTION - 
/* ====================
DWORD WINAPI UnDecorateSymbolName(
  __in   PCTSTR DecoratedName,
  __out  PTSTR UnDecoratedName,
  __in   DWORD UndecoratedLength,
  __in   DWORD Flags
);

DecoratedName: Always begins with a question mark (?)
UnDecoratedName: BUffer to receive result
UndecoratedLength: Length of buffer
Flags: 0 to do full decode - various other partial flags
Return Value
If the function succeeds, the return value is the number of characters 
in the UnDecoratedName buffer, not including the NULL terminator.
If the function fails, the return value is zero. To retrieve extended 
error information, call GetLastError.
Header:  Dbghelp.h Library: Dbghelp.lib 

   ===================== */

// #define MX_BIG_BUF 16384
static char my_big_buf[MX_BIG_BUF];
char * get_my_big_buf(void) { return my_big_buf; }

vSTR space_split( std::string &str )
{
    vSTR v;
    std::string::size_type len = str.length();
    std::string::size_type i = 0;
    std::string::size_type j;
    while (i < len) {
        while (i < len && isspace((unsigned char)str[i])) i++;  // eat spaces
        j = i;  // first non-space char
        while (i < len && !isspace((unsigned char)str[i])) i++; // move over non-spaces
        if (j < i) v.push_back(  str.substr(j, i-j) );
    }
    return v;
}


bool MyStgSort( std::string s1, std::string s2 )
{
    // strings are of the form "  00003C50  _areadlink [opt]"
    vSTR v1 = space_split(s1);
    vSTR v2 = space_split(s2);
    std::string s21;
    std::string s22;
    if ((v1.size() >= 2) && (v2.size() >= 2)) {
        s21 = v1[1];
        s22 = v2[1];
        str_to_upper( s21 );
        str_to_upper( s22 );
    }
    int res = strcmp( s21.c_str(), s22.c_str() ); 
    return ( res < 0 );
}

void DumpFirstLinkerMember(PVOID p)
{
    size_t len;
    DWORD offset;
    DWORD cSymbols = *(PDWORD)p;
    PDWORD pMemberOffsets = MakePtr( PDWORD, p, 4 );
    PSTR pSymbolName;
    unsigned int i;
    size_t skipped = 0;
    char *cp = GetNxtBuf();
    vSTR vs;
    std::string s;
    bool skip;

    cSymbols = ConvertBigEndian(cSymbols);
    pSymbolName = MakePtr( PSTR, pMemberOffsets, 4 * cSymbols );
    
    sprtf("PE First Linker Member:\n");
    sprtf( "  Symbols:  total count %d\n", cSymbols );
    sprtf( "  MbrOffs   Name\n  --------  ----\n" );

    for ( i = 0; i < cSymbols; i++ )
    {
        offset = ConvertBigEndian( *pMemberOffsets );        
        len = strlen(pSymbolName);
        s = pSymbolName;
        add_to_exports(s);
        skip = (strncmp(pSymbolName,"??_",3) == 0) ? true : false;
        if (!VERB5 && (len > 3) && skip) {
            skipped++;
        } else {
            sprintf(cp,"  %08X  %s", offset, pSymbolName);
            if (VERB2 && (len > 2) && (*pSymbolName == '?')
                && !(pSymbolName[1] == '?')) {
                char * bb = get_my_big_buf();
                DWORD res = UnDecorateSymbolName( pSymbolName,
                    bb, MX_BIG_BUF, 0 );
                if ((res > 0) && !(*bb == '?')) {
                    sprintf(EndBuf(cp)," [%s]", bb);
                }
            }
            //sprtf("\n");
            s = cp;
            vs.push_back(s);
        }
        pMemberOffsets++;
        pSymbolName += strlen(pSymbolName) + 1;
    }
    std::sort(vs.begin(), vs.end(), MyStgSort);
    for ( vSTRi ii = vs.begin(); ii != vs.end(); ii++ ) {
        s = *ii;
        sprtf("%s\n",s.c_str());
    }

    if (skipped)
        sprtf("Skipped %d symbols of the form '??_'. User -v5 to view.\n", skipped);
}

void DumpSecondLinkerMember(PVOID p)
{
    DWORD cArchiveMembers = *(PDWORD)p;
    PDWORD pMemberOffsets = MakePtr( PDWORD, p, 4 );
    DWORD cSymbols;
    PSTR pSymbolName;
    PWORD pIndices;
    unsigned i;
    char *cp = GetNxtBuf();
    vSTR vs;
    std::string s;

    cArchiveMembers = cArchiveMembers;

    // The number of symbols is in the DWORD right past the end of the
    // member offset array.
    cSymbols = pMemberOffsets[cArchiveMembers];

    pIndices = MakePtr( PWORD, p, 4 + cArchiveMembers * sizeof(DWORD) + 4 );

    pSymbolName = MakePtr( PSTR, pIndices, cSymbols * sizeof(WORD) );
    
    if ( fDumpExportsOnly )
        return;

    sprtf("PE Second Linker Member:\n");
    
    sprtf( "  Archive Members: %08X\n", cArchiveMembers );
    sprtf( "  Symbols: total count %d\n", cSymbols );
    sprtf( "  MbrOffs   Name\n  --------  ----\n" );

    for ( i = 0; i < cSymbols; i++ )
    {
        s = pSymbolName;
        add_to_exports(s);
        sprintf(cp,"  %08X  %s", pMemberOffsets[pIndices[i] - 1], pSymbolName);
        s = cp;
        vs.push_back(s);
        pSymbolName += strlen(pSymbolName) + 1;
    }
    std::sort(vs.begin(), vs.end(), MyStgSort);
    for ( vSTRi ii = vs.begin(); ii != vs.end(); ii++ ) {
        s = *ii;
        sprtf("%s\n",s.c_str());
    }
}

void DumpLongnamesMember(PVOID p, DWORD len)
{
    PSTR pszName = (PSTR)p;
    DWORD offset = 0;

    PszLongnames = (PSTR)p;     // Save off pointer for use when dumping
                                // out OBJ member names

    if ( fDumpExportsOnly )
        return;

    sprtf("Longnames:\n");
    
    // The longnames member is a series of null-terminated string.  Print
    // out the offset of each string (in decimal), followed by the string.
    while ( offset < len )
    {
        unsigned cbString = lstrlen( pszName )+1;

        sprtf("  %05u: %s\n", offset, pszName);
        offset += cbString;
        pszName += cbString;
    }
}

void DumpLibFile( LPVOID lpFileBase )
{
    PIMAGE_ARCHIVE_MEMBER_HEADER pArchHeader;
    BOOL fSawFirstLinkerMember = FALSE;
    BOOL fSawSecondLinkerMember = FALSE;
    BOOL fBreak = FALSE;

    if ( strncmp((char *)lpFileBase,IMAGE_ARCHIVE_START,
                            		IMAGE_ARCHIVE_START_SIZE ) )
    {
        sprtf("Not a valid .LIB file - signature not found\n");
        return;
    }
    
    pArchHeader = MakePtr(PIMAGE_ARCHIVE_MEMBER_HEADER, lpFileBase,
                            IMAGE_ARCHIVE_START_SIZE);

    while ( pArchHeader )
    {
        DWORD thisMemberSize;
        
        DisplayArchiveMemberHeader( pArchHeader,
                                    (PBYTE)pArchHeader - (PBYTE) lpFileBase );

        if ( !fDumpExportsOnly )
            sprtf("\n");

        if ( !strncmp( 	(char *)pArchHeader->Name,
        				IMAGE_ARCHIVE_LINKER_MEMBER, 16) )
        {
            if ( !fSawFirstLinkerMember )
            {
                DumpFirstLinkerMember( (PVOID)(pArchHeader + 1) );
                sprtf("\n");
                fSawFirstLinkerMember = TRUE;
            }
            else if ( !fSawSecondLinkerMember )
            {
                DumpSecondLinkerMember( (PVOID)(pArchHeader + 1) );
                if ( !fDumpExportsOnly )
                    sprtf("\n");
                fSawSecondLinkerMember = TRUE;
            }
        }
        else if( !strncmp(	(char *)pArchHeader->Name,
        					IMAGE_ARCHIVE_LONGNAMES_MEMBER, 16) )
        {
            DumpLongnamesMember( (PVOID)(pArchHeader + 1),
                                 atoi((char *)pArchHeader->Size) );
            if ( !fDumpExportsOnly )
                sprtf("\n");
        }
        else    // It's an OBJ file
        {
            DumpObjFile( (PIMAGE_FILE_HEADER)(pArchHeader + 1) );
        }

        // Calculate how big this member is (it's originally stored as 
        // as ASCII string.
        thisMemberSize = atoi((char *)pArchHeader->Size)
                        + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;

        thisMemberSize = (thisMemberSize+1) & ~1;   // Round up

        // Get a pointer to the next archive member
        pArchHeader = MakePtr(PIMAGE_ARCHIVE_MEMBER_HEADER, pArchHeader,
                                thisMemberSize);

        // Bail out if we don't see the EndHeader signature in the next record
        __try
        {
            if (strncmp( (char *)pArchHeader->EndHeader, IMAGE_ARCHIVE_END, 2))
                break;
        }
        __except( TRUE )    // Should only get here if pArchHeader is bogus
        {
            fBreak = TRUE;  // Ideally, we could just put a "break;" here,
        }                   // but BC++ doesn't like it.
        
        if ( fBreak )   // work around BC++ problem.
            break;
    }
    show_export_list();
}


// =========================================================
//#ifdef __cplusplus
//extern "C" {
//#endif
extern int IsELFFile(char *buf, size_t nbytes);
extern int DumpELFFile(char *buf, size_t nbytes);
//#ifdef __cplusplus
//}
//#endif
int is_listed_machine_type( WORD wMachineType )
{
    PSTR ps = GetMachineTypeName(wMachineType);
    if (strcmp(ps,"UNLISTED") == 0)
        return 0;
    if (strcmp(ps,"Unknown") == 0)
        return 0;
    return 1;
}

int DumpMemoryMap( LPVOID lpFileBase, PSTR filename, size_t nbytes )
{
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
	PIMAGE_FILE_HEADER pImgFileHdr = (PIMAGE_FILE_HEADER)lpFileBase;
    int listed = is_listed_machine_type( pImgFileHdr->Machine ); // do we have a list machine type
    // of course this may only mean the TABLE needs to be EXTENDED for a NEW machine type

    g_ActPEFile = filename;

   if ( 0 == strncmp((char *)lpFileBase, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE ) )
   {
#ifdef ADD_DUMP_LIB_FILE
     DumpLibFile( lpFileBase );
#endif // #ifdef ADD_DUMP_LIB_FILE
   } else
   if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
   {
     DumpExeFile( dosHeader );
   }
   else if ( dosHeader->e_magic == IMAGE_SEPARATE_DEBUG_SIGNATURE )
   {
#ifdef ADD_DUMP_DBG_FILE
     DumpDbgFile( (PIMAGE_SEPARATE_DEBUG_HEADER)lpFileBase );
#endif // #ifdef ADD_DUMP_DBG_FILE
   }
   else if ( listed && ( 0 == pImgFileHdr->SizeOfOptionalHeader ) ) 
   {	// 0 optional header
#ifdef ADD_DUMP_OBJ_FILE
           DumpObjFile( pImgFileHdr );					// means it's an OBJ
#endif // #ifdef ADD_DUMP_OBJ_FILE
#ifdef ADD_DUMP_ROM_IMAGE
      else if ( 	pImgFileHdr->SizeOfOptionalHeader
			      == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER )
      {
	      DumpROMImage( (PIMAGE_ROM_HEADERS)pImgFileHdr );
      }
#endif // #ifdef ADD_DUMP_ROM_IMAGE
   }
   else
   {
#ifdef ADD_ELF_SUPPORT
       if (IsELFFile((char *)lpFileBase, nbytes)) {
           //sprtf("Dump of an ELF files not yet completed!\n");
           if( DumpELFFile((char *)lpFileBase, nbytes) ) {
               sprtf("Dump of an ELF file FAILED!\n");
               return 0;
           } else {
               return 1; // it has been dumped
           }
       }
#endif // #undef ADD_ELF_SUPPORT
      sprtf("WARNING: Unrecognized file format for %s\n", filename );
      return 0;
   }
   show_lib_list();
   kill_lib_list();
   show_obj_list();
   show_machine_list();
   return 1;   // has been DUMPED
}

BOOL Set_PEDUMP_D( BOOL flag )  // fDumpFileHeader
{
   // else if ( argv[i][1] == 'D' )
   BOOL curr = fDumpFileHeader;
   fDumpFileHeader = flag;
   return curr;
}

#ifdef ADD_EXE_FOLLOW
//   { Set_PEDUMP_F, 'F', &fDumpFollowImports },
BOOL Set_PEDUMP_F( BOOL flag )  // fDumpFollowImports
{
   BOOL curr = fDumpFollowImports;
   fDumpFollowImports = flag;
   return curr;
}
#endif

BOOL Set_PEDUMP_O( BOOL flag )  // fDumpOptionalHeader
{
   // else if ( argv[i][1] == 'D' )
   BOOL curr = fDumpOptionalHeader;
   fDumpOptionalHeader = flag;
   return curr;
}

BOOL Set_PEDUMP_H( BOOL flag )
{
   // } else if ( argv[i][1] == 'H' )
   BOOL curr = fShowRawSectionData;
   fShowRawSectionData = flag;
   return curr;
}
BOOL Set_PEDUMP_L( BOOL flag )
{
   // else if ( argv[i][1] == 'L' )
   BOOL curr = fShowLineNumbers;
   fShowLineNumbers = flag;
   return curr;
}
BOOL Set_PEDUMP_P( BOOL flag )
{
   // else if ( argv[i][1] == 'P' )
   BOOL curr = fShowPDATA;
   fShowPDATA = flag;
   return curr;
}
BOOL Set_PEDUMP_B( BOOL flag )
{
   // else if ( argv[i][1] == 'B' )
   BOOL curr = fShowRelocations;
   fShowRelocations = flag;
   return curr;
}

BOOL Set_PEDUMP_S( BOOL flag )  // fShowSymbolTable
{
   // else if ( argv[i][1] == 'S' )
   BOOL curr = fShowSymbolTable;
   fShowSymbolTable = flag;
   return curr;
}

BOOL Set_PEDUMP_I( BOOL flag )  // fShowIATentries
{
   //else if ( argv[i][1] == 'I' )
   BOOL curr = fShowIATentries;
   fShowIATentries = flag;
   return curr;
}

BOOL Set_PEDUMP_R( BOOL flag )  // fShowResources
{
   // else if ( argv[i][1] == 'R' )
   BOOL curr = fShowResources;
   fShowResources = flag;
   return curr;
}

// fDumpDataDirectory
BOOL Set_PEDUMP_T( BOOL flag )  // fDumpDataDirectory
{
   BOOL curr = fDumpDataDirectory;
   fDumpDataDirectory = flag;
   return curr;
}

// fDumpSectionTable
BOOL Set_PEDUMP_C( BOOL flag )  // fDumpSectionTable
{
   BOOL curr = fDumpSectionTable;
   fDumpSectionTable = flag;
   return curr;
}

// fDumpImportNames
BOOL Set_PEDUMP_M( BOOL flag )  // fDumpImportNames
{
   BOOL curr = fDumpImportNames;
   fDumpImportNames = flag;
   return curr;
}

// BOOL fDumpDebugDirectory = TRUE;
BOOL Set_PEDUMP_G( BOOL flag )  // fDumpDebugDirectory
{
   BOOL curr = fDumpDebugDirectory;
   fDumpDebugDirectory = flag;
   return curr;
}

// void Set_All_ON(void)
BOOL Set_PEDUMP_A( BOOL flag )  // set ALL ON
{
   Set_PEDUMP_H( flag ); // fShowRawSectionData
   Set_PEDUMP_L( flag ); // fShowLineNumbers
   Set_PEDUMP_P( flag ); // fShowPDATA
   Set_PEDUMP_B( flag ); // fShowRelocations
   Set_PEDUMP_S( flag ); // fShowSymbolTable
   Set_PEDUMP_I( flag ); // fShowIATentries
   Set_PEDUMP_R( flag ); // fShowResources
   return flag;
}

// fDumpExportsOnly
BOOL Set_PEDUMP_X( BOOL flag )  // fDumpExportsOnly
{
   BOOL curr = fDumpExportsOnly;
   fDumpExportsOnly = flag;
   return curr;
}

void Set_Imports_Only( void )
{
   Set_PEDUMP_A( FALSE ); // set ALL ON/OFF
   Set_PEDUMP_B( FALSE ); // fShowRelocations
   Set_PEDUMP_C( FALSE ); // fDumpSectionTable
   Set_PEDUMP_D( FALSE ); // fDumpFileHeader
   Set_PEDUMP_G( FALSE ); // fDumpDebugDirectory
   Set_PEDUMP_H( FALSE ); // fShowRawSectionData
   Set_PEDUMP_I( FALSE ); // fShowIATentries
   Set_PEDUMP_L( FALSE ); // fShowLineNumbers
   Set_PEDUMP_M( TRUE ); // fDumpImportNames
   Set_PEDUMP_O( FALSE ); // fDumpOptionalHeader
   Set_PEDUMP_P( FALSE ); // fShowPDATA
   Set_PEDUMP_R( FALSE ); // fShowResources
   Set_PEDUMP_S( FALSE ); // fShowSymbolTable
   Set_PEDUMP_T( FALSE ); // fDumpDataDirectory
   Set_PEDUMP_X( FALSE ); // fDumpExportsOnly
}

void Set_Exports_Only( void )
{
   Set_PEDUMP_A( FALSE ); // set ALL ON/OFF
   Set_PEDUMP_B( FALSE ); // fShowRelocations
   Set_PEDUMP_C( FALSE ); // fDumpSectionTable
   Set_PEDUMP_D( FALSE ); // fDumpFileHeader
   Set_PEDUMP_G( FALSE ); // fDumpDebugDirectory
   Set_PEDUMP_H( FALSE ); // fShowRawSectionData
   Set_PEDUMP_I( FALSE ); // fShowIATentries
   Set_PEDUMP_L( FALSE ); // fShowLineNumbers
   Set_PEDUMP_M( FALSE ); // fDumpImportNames
   Set_PEDUMP_O( FALSE ); // fDumpOptionalHeader
   Set_PEDUMP_P( FALSE ); // fShowPDATA
   Set_PEDUMP_R( FALSE ); // fShowResources
   Set_PEDUMP_S( FALSE ); // fShowSymbolTable
   Set_PEDUMP_T( FALSE ); // fDumpDataDirectory
   Set_PEDUMP_X( TRUE  ); // fDumpExportsOnly
}

typedef struct tagCHAR2FUNC {
   SETFUNC Func;
   char    Chr;
   PBOOL   Flag;
}CHAR2FUNC, * PCHAR2FUNC;

CHAR2FUNC sCharFunc[] = {
   { Set_PEDUMP_A, 'A', 0  }, // set ALL ON/OFF
   { Set_PEDUMP_B, 'B', &fShowRelocations },
   { Set_PEDUMP_C, 'C', &fDumpSectionTable },
   { Set_PEDUMP_D, 'D', &fDumpFileHeader },
// 20100527 - add -exe:F
// " F = follow import trail, using PATH to find, and dump imported DLLs: def = OFF."MEOR
#ifdef ADD_EXE_FOLLOW
   { Set_PEDUMP_F, 'F', &fDumpFollowImports },
#endif
   { Set_PEDUMP_G, 'G', &fDumpDebugDirectory },
   { Set_PEDUMP_H, 'H', &fShowRawSectionData },
   { Set_PEDUMP_I, 'I', &fShowIATentries },
   { Set_PEDUMP_L, 'L', &fShowLineNumbers },
   { Set_PEDUMP_M, 'M', &fDumpImportNames },
   { Set_PEDUMP_O, 'O', &fDumpOptionalHeader },
   { Set_PEDUMP_P, 'P', &fShowPDATA },
   { Set_PEDUMP_R, 'R', &fShowResources },
   { Set_PEDUMP_S, 'S', &fShowSymbolTable },
   { Set_PEDUMP_T, 'T', &fDumpDataDirectory },
   { Set_PEDUMP_X, 'X', &fDumpExportsOnly },
   { 0, 0, 0 }
};

char * Get_Current_Opts( void ) {
   static char _s_curropts[256];
   char * pco = _s_curropts;
   PCHAR2FUNC pcf = &sCharFunc[0];

   *pco = 0;
   while( pcf->Func )
   {
      PBOOL pb = pcf->Flag;
      if(pb)
      {
         sprintf( pco + strlen(pco), "%c", pcf->Chr );
         if ( ! *pb )
            strcat(pco,"-");
      }
      pcf++;
   }
   return pco;
}

#endif // #ifdef   USE_PEDUMP_CODE - FIX20080507
// eof - DumpPE.c

