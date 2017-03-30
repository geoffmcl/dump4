//--------------------
// PROGRAM: PEDUMP
// FILE:    PEDUMP.C
// AUTHOR:  Matt Pietrek - 1993
// from : http://msdn.microsoft.com/en-us/library/ms809762.aspx
//--------------------
#include <stdio.h>

#if 0   // 0000000000000000000000000000000000000000000000000000000000000000000000000000000

//#include <windows.h>
#include <Winsock2.h>

#include "objdump.h"
#include "exedump.h"
#include "extrnvar.h"

// Global variables set here, and used in EXEDUMP.C and OBJDUMP.C
BOOL fShowRelocations = FALSE;
BOOL fShowRawSectionData = FALSE;
BOOL fShowSymbolTable = FALSE;
BOOL fShowLineNumbers = FALSE;

char HelpText[] = 
"PEDUMP - Win32/COFF .EXE/.OBJ file dumper - 1993 Matt Pietrek\n\n"
"Syntax: PEDUMP [switches] filename\n\n"
"  /A    include everything in dump\n"
"  /H    include hex dump of sections\n"
"  /L    include line number information\n"
"  /R    show base relocations\n"
"  /S    show symbol table\n";

// Open up a file, memory map it, and call the appropriate dumping routine
int DumpFile(LPSTR filename)
{
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    PIMAGE_DOS_HEADER dosHeader;
    
    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                    
    if ( hFile == INVALID_HANDLE_VALUE )
    {   printf("Couldn't open file with CreateFile()\n");
        return 1; }
    
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if ( hFileMapping == 0 )
    {   CloseHandle(hFile);
        printf("Couldn't open file mapping with CreateFileMapping()\n");
        return 2; }
    
    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if ( lpFileBase == 0 )
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Couldn't map view of file with MapViewOfFile()\n");
        return 3;
    }

    printf("Dump of file %s\n\n", filename);
    
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
    {
        //DumpExeFile( dosHeader );
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Is an EXE file\n");
        return 4;
    }
    else if ( (dosHeader->e_magic == 0x014C)    // Does it look like a i386
              && (dosHeader->e_sp == 0) )        // COFF OBJ file???
    {
        // The two tests above aren't what they look like.  They're
        // really checking for IMAGE_FILE_HEADER.Machine == i386 (0x14C)
        // and IMAGE_FILE_HEADER.SizeOfOptionalHeader == 0;
        //DumpObjFile( (PIMAGE_FILE_HEADER)lpFileBase );
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Is an OBJ file\n");
        return 5;
    }
    else
        printf("unrecognized file format\n");

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
}

// process all the command line arguments and return a pointer to
// the filename argument.
PSTR ProcessCommandLine(int argc, char *argv[])
{
    int i;
    
    for ( i=1; i < argc; i++ )
    {
        strupr(argv[i]);
        
        // Is it a switch character?
        if ( (argv[i][0] == '-') || (argv[i][0] == '/') )
        {
            if ( argv[i][1] == 'A' )
            {   fShowRelocations = TRUE;
                fShowRawSectionData = TRUE;
                fShowSymbolTable = TRUE;
                fShowLineNumbers = TRUE; }
            else if ( argv[i][1] == 'H' )
                fShowRawSectionData = TRUE;
            else if ( argv[i][1] == 'L' )
                fShowLineNumbers = TRUE;
            else if ( argv[i][1] == 'R' )
                fShowRelocations = TRUE;
            else if ( argv[i][1] == 'S' )
                fShowSymbolTable = TRUE;
        }
        else    // Not a switch character.  Must be the filename
        {   return argv[i]; }
    }
}

int pedump_main(int argc, char *argv[])
{
    PSTR filename;
    
    if ( argc == 1 )
    {   printf(    HelpText );
        return 1; }
    
    filename = ProcessCommandLine(argc, argv);
    if ( filename )
        DumpFile( filename );
    return 0;
}

#endif // 0

#if 0

//==================================    
// PEDUMP - Matt Pietrek 1997-2001    
// FILE: COMMON.CPP    
//==================================    
   
//#include <windows.h>    
#include <Winsock2.h>
#include <stdio.h>    
#include <string.h>    
#include <stdlib.h>    
#include <time.h>    
#include "common.h"    
#include "COFFSymbolTable.h"    
#include "extrnvar.h"    
#include "cv_dbg.h"    
#include "cvsymbols.h"    
   
//    
// Dump the COFF debug information header    
//    
void DumpCOFFHeader(PIMAGE_COFF_SYMBOLS_HEADER pDbgInfo)   
{   
    printf("COFF Debug Info Header\n");   
    printf("  NumberOfSymbols:      %08X\n", pDbgInfo->NumberOfSymbols);   
    printf("  LvaToFirstSymbol:     %08X\n", pDbgInfo->LvaToFirstSymbol);   
    printf("  NumberOfLinenumbers:  %08X\n", pDbgInfo->NumberOfLinenumbers);   
    printf("  LvaToFirstLinenumber: %08X\n", pDbgInfo->LvaToFirstLinenumber);   
    printf("  RvaToFirstByteOfCode: %08X\n", pDbgInfo->RvaToFirstByteOfCode);   
    printf("  RvaToLastByteOfCode:  %08X\n", pDbgInfo->RvaToLastByteOfCode);   
    printf("  RvaToFirstByteOfData: %08X\n", pDbgInfo->RvaToFirstByteOfData);   
    printf("  RvaToLastByteOfData:  %08X\n", pDbgInfo->RvaToLastByteOfData);   
}   
   
   
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
   
    lstrcpyn( buffer, pSymbol->GetName(), length );   
   
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
       
    printf("Line Numbers\n");   
       
    for (i=0; i  count; i++)   
    {   
        if ( pln->Linenumber == 0 ) // A symbol table index    
        {   
            buffer[0] = 0;   
            LookupSymbolName(pln->Type.SymbolTableIndex, buffer,   
                            sizeof(buffer));   
            printf("SymIndex: %X (%s)\n", pln->Type.SymbolTableIndex,   
                                             buffer);   
        }   
        else        // A regular line number    
            printf(" Addr: %05X  Line: %04u\n",   
                pln->Type.VirtualAddress, pln->Linenumber);   
        pln++;   
    }   
}   
   
   
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
void DumpCOFFSymbolTable( PCOFFSymbolTable pSymTab )   
{   
    printf( "Symbol Table - %X entries  (* = auxillary symbol)\n",   
            pSymTab->GetNumberOfSymbols() );   
   
   
    printf(   
    "Indx Sectn Value    Type  Storage  Name\n"   
    "---- ----- -------- ----- -------  --------\n");   
   
    PCOFFSymbol pSymbol = pSymTab->GetNextSymbol( 0 );   
   
    while ( pSymbol )   
    {   
        char szSection[10];   
        GetSectionName(pSymbol->GetSectionNumber(),szSection,sizeof(szSection));   
   
        printf( "%04X %5s %08X  %s %-8s %s\n",   
                pSymbol->GetIndex(), szSection, pSymbol->GetValue(),   
                pSymbol->GetTypeName(), pSymbol->GetStorageClassName(),   
                pSymbol->GetName() );   
   
        if ( pSymbol->GetNumberOfAuxSymbols() )   
        {   
            char szAuxSymbol[1024];   
            if (pSymbol->GetAuxSymbolAsString(szAuxSymbol,sizeof(szAuxSymbol)))   
                printf( "     * %s\n", szAuxSymbol );              
        }   
           
        pSymbol = pSymTab->GetNextSymbol( pSymbol );   
   
    }   
}   
   
void DumpMiscDebugInfo( PIMAGE_DEBUG_MISC pMiscDebugInfo )   
{   
    if ( IMAGE_DEBUG_MISC_EXENAME != pMiscDebugInfo->DataType )   
    {   
        printf( "Unknown Miscellaneous Debug Information type: %u\n",    
                pMiscDebugInfo->DataType );   
        return;   
    }   
   
    printf( "Miscellaneous Debug Information\n" );   
    printf( "  %s\n", pMiscDebugInfo->Data );   
}   
   
void DumpCVDebugInfoRecord( PDWORD pCVHeader )   
{   
    PPDB_INFO pPDBInfo;   
   
    printf( "CodeView Signature: %08X\n", *pCVHeader );   
   
    if ( 'SDSR' == (UINT32)*pCVHeader )   
    {   
        printf( "New format Visual Studio .NET (or later) debug information\n" );   
        return;   
    }   
   
    if ( 'BN' != (unsigned short)*pCVHeader )   
    {   
        printf( "Unhandled CodeView Information format %.4s\n", pCVHeader );   
        return;   
    }   
   
    if ( '01BN' == (unsigned short)*pCVHeader ) // NB10 == PDB file info    
    {   
        pPDBInfo = (PPDB_INFO)pCVHeader;   
   
        printf( "  Offset: %08X  Signature: %08X  Age: %08X\n",   
                pPDBInfo->Offset, pPDBInfo->sig, pPDBInfo->age );   
        printf( "  File: %s\n", pPDBInfo->PdbName );   
    }   
    else    // Should be NB09 or NB11    
    {   
        DumpCVSymbolTable( (PBYTE)pCVHeader, g_pMappedFileBase );   
    }   
}   
   


#endif // 0000000000000000000000000000000000000000000000000000000000000000000000000

// eof
