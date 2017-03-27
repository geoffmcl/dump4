/*\
 * dumpexe.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * Also reviewed : http://www.skynet.ie/~caolan/pub/winresdump/winresdump/doc/pefile.html
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <Windows.h>
#include <Dbghelp.h>
#include <time.h>
#include <iostream>      

using namespace std;

// other includes
#ifndef SPRTF
#define SPRTF printf
#endif
#ifndef MEOR
#ifdef WIN32
#define MEOR "\r\n"
#else
#define MEOR "\n"
#endif
#endif

// forward ref
int test_main();


/*

//////////////////////////////////////////////////////////////////////////////////
typedef struct _IMAGE_NT_HEADERS {
DWORD                 Signature;
IMAGE_FILE_HEADER     FileHeader;
IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct _IMAGE_NT_HEADERS64 {
DWORD Signature;
IMAGE_FILE_HEADER FileHeader;
IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

//
// Optional header format.
//

typedef struct _IMAGE_OPTIONAL_HEADER {
//
// Standard fields.
//

WORD    Magic;
BYTE    MajorLinkerVersion;
BYTE    MinorLinkerVersion;
DWORD   SizeOfCode;
DWORD   SizeOfInitializedData;
DWORD   SizeOfUninitializedData;
DWORD   AddressOfEntryPoint;
DWORD   BaseOfCode;
DWORD   BaseOfData;

//
// NT additional fields.
//

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

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
//
// Standard fields.
//
WORD   Magic;
BYTE   MajorLinkerVersion;
BYTE   MinorLinkerVersion;
DWORD  SizeOfCode;
DWORD  SizeOfInitializedData;
DWORD  SizeOfUninitializedData;
DWORD  AddressOfEntryPoint;
DWORD  BaseOfCode;
//
// NT additional fields.
//
DWORD  BaseOfData;
DWORD  BaseOfBss;
DWORD  GprMask;
DWORD  CprMask[4];
DWORD  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
//
// Standard fields.
//
WORD        Magic;
BYTE        MajorLinkerVersion;
BYTE        MinorLinkerVersion;
DWORD       SizeOfCode;
DWORD       SizeOfInitializedData;
DWORD       SizeOfUninitializedData;
DWORD       AddressOfEntryPoint;
DWORD       BaseOfCode;
//
// NT additional fields.
//
ULONGLONG   ImageBase;
DWORD       SectionAlignment;
DWORD       FileAlignment;
WORD        MajorOperatingSystemVersion;
WORD        MinorOperatingSystemVersion;
WORD        MajorImageVersion;
WORD        MinorImageVersion;
WORD        MajorSubsystemVersion;
WORD        MinorSubsystemVersion;
DWORD       Win32VersionValue;
DWORD       SizeOfImage;
DWORD       SizeOfHeaders;
DWORD       CheckSum;
WORD        Subsystem;
WORD        DllCharacteristics;
ULONGLONG   SizeOfStackReserve;
ULONGLONG   SizeOfStackCommit;
ULONGLONG   SizeOfHeapReserve;
ULONGLONG   SizeOfHeapCommit;
DWORD       LoaderFlags;
DWORD       NumberOfRvaAndSizes;
IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#ifdef _WIN64
typedef IMAGE_OPTIONAL_HEADER64             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef IMAGE_OPTIONAL_HEADER32             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR32_MAGIC
#endif

////////////////////////////////////////////////////////////////////////////////////
typedef struct _IMAGE_SECTION_HEADER {
BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
union {
DWORD PhysicalAddress;
DWORD VirtualSize;
} Misc;
DWORD VirtualAddress;
DWORD SizeOfRawData;
DWORD PointerToRawData;
DWORD PointerToRelocations;
DWORD PointerToLinenumbers;
WORD  NumberOfRelocations;
WORD  NumberOfLinenumbers;
DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

 */

static const char *module = "dumpexe";

static const char *usr_input = 0;

void give_help( char *name )
{
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(0)\n");
    // TODO: More help
}

static HANDLE hFile = 0;
static HANDLE hFileMapping = 0;
static LPVOID lpFileBase = 0;
//static PIMAGE_DOS_HEADER dosHeader;
//static PIMAGE_NT_HEADERS pNTHeader;
//static DWORD64 base;
//static DWORD64 exportsStartRVA, exportsEndRVA;
//static PIMAGE_EXPORT_DIRECTORY exportDir;
static int fDumpFileHeader = 1;

void CloseMappedFile(const char *fileName)
{
    CloseHandle(hFile);
    CloseHandle(hFileMapping);
}

typedef struct tagWORD_FLAG_DESCRIPTIONS {
    WORD    flag;
    PSTR    name;
} WORD_FLAG_DESCRIPTIONS, *PWORD_FLAG_DESCRIPTIONS;

/////////////////////////////////////////////////////////////////
// 20140719 UPDATE
// Bitfield values and names for the IMAGE_FILE_HEADER flags
WORD_FLAG_DESCRIPTIONS ImageFileHeaderCharacteristics[] = {
    { IMAGE_FILE_RELOCS_STRIPPED,"RELOCS_STRIPPED" }, // 0x0001  // Relocation info stripped from file.
    { IMAGE_FILE_EXECUTABLE_IMAGE,"EXECUTABLE_IMAGE" }, // 0x0002  // File is executable  (i.e. no unresolved externel references).
    { IMAGE_FILE_LINE_NUMS_STRIPPED,"LINE_NUMS_STRIPPED" }, // 0x0004  // Line nunbers stripped from file.
    { IMAGE_FILE_LOCAL_SYMS_STRIPPED,"LOCAL_SYMS_STRIPPED" }, // 0x0008  // Local symbols stripped from file.
    { IMAGE_FILE_AGGRESIVE_WS_TRIM,"AGGRESIVE_WS_TRIM" }, // 0x0010  // Agressively trim working set
    { IMAGE_FILE_LARGE_ADDRESS_AWARE,"LARGE_ADDRESS_AWARE" }, // 0x0020  // App can handle >2gb addresses
    { IMAGE_FILE_BYTES_REVERSED_LO,"BYTES_REVERSED_LO" }, //  0x0080  // Bytes of machine word are reversed.
    { IMAGE_FILE_32BIT_MACHINE,"32BIT_MACHINE" }, // 0x0100  // 32 bit word machine.
    { IMAGE_FILE_DEBUG_STRIPPED,"DEBUG_STRIPPED" }, // 0x0200  // Debugging info stripped from file in .DBG file
    { IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP,"REMOVABLE_RUN_FROM_SWAP" }, // 0x0400  // If Image is on removable media, copy and run from the swap file.
    { IMAGE_FILE_NET_RUN_FROM_SWAP,"NET_RUN_FROM_SWAP" }, // 0x0800  // If Image is on Net, copy and run from the swap file.
    { IMAGE_FILE_SYSTEM,"SYSTEM" }, //                    0x1000  // System File.
    { IMAGE_FILE_DLL,"DLL" }, //                       0x2000  // File is a DLL.
    { IMAGE_FILE_UP_SYSTEM_ONLY,"UP_SYSTEM_ONLY" }, //            0x4000  // File should only be run on a UP machine
    { IMAGE_FILE_BYTES_REVERSED_HI,"BYTES_REVERSED_HI" } //       0x8000  // Bytes of machine word are reversed.
};

#define NUMBER_IMAGE_HEADER_FLAGS \
    (sizeof(ImageFileHeaderCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))


// 20140614: Increase Machine Type Names - from WinNT.h
static PSTR GetMachineTypeName(WORD wMachineType)
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

static char * pedump_ctime(time_t * ptm)
{
    char * ctm = NULL;
#ifdef WIN32
    __time32_t * timer = (__time32_t *)ptm;
    ctm = _ctime32(timer); // NOTE: Returns buffer with "\n" appended
#else
    ctm = ctime(ptm); // this will use 64-bit time
#endif

    if (ctm == NULL)  // if OUT OF RANGE
        ctm = "Out of range" MEOR;

    return ctm;
}

//
// Dump the IMAGE_FILE_HEADER for a PE file or an OBJ
//
void DumpHeader(PIMAGE_FILE_HEADER pImageFileHeader)
{
    UINT headerFieldWidth = 30;
    UINT i;

    //add_2_machine_list(pImageFileHeader->Machine);
    if (fDumpFileHeader)
    {
        SPRTF("PE File Header:\n");

        SPRTF("  %-*s%04X (%s)\n", headerFieldWidth, "Machine:",
            pImageFileHeader->Machine,
            GetMachineTypeName(pImageFileHeader->Machine));
        SPRTF("  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
            pImageFileHeader->NumberOfSections);
        SPRTF("  %-*s%08X -> %s", headerFieldWidth, "TimeDateStamp:",
            pImageFileHeader->TimeDateStamp,
            pedump_ctime((time_t *)&pImageFileHeader->TimeDateStamp));
        SPRTF("  %-*s%08X\n", headerFieldWidth, "PointerToSymbolTable:",
            pImageFileHeader->PointerToSymbolTable);
        SPRTF("  %-*s%08X\n", headerFieldWidth, "NumberOfSymbols:",
            pImageFileHeader->NumberOfSymbols);
        SPRTF("  %-*s%04X\n", headerFieldWidth, "SizeOfOptionalHeader:",
            pImageFileHeader->SizeOfOptionalHeader);
        SPRTF("  %-*s%04X\n", headerFieldWidth, "Characteristics:",
            pImageFileHeader->Characteristics);
        for (i = 0; i < NUMBER_IMAGE_HEADER_FLAGS; i++)
        {
            if (pImageFileHeader->Characteristics &
                ImageFileHeaderCharacteristics[i].flag)
                SPRTF("    %s\n", ImageFileHeaderCharacteristics[i].name);
        }
        SPRTF("\n");
    }
}

static int fDumpOptionalHeader = 1;
static int fDumpDataDirectory = 1;

#ifndef	IMAGE_DLLCHARACTERISTICS_WDM_DRIVER
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER  0x2000     // Driver uses WDM model
#endif

// Marked as obsolete in MSDN CD 9
// Bitfield values and names for the DllCharacteritics flags
// 20170324 - Updated per winnt.h
/*
// DllCharacteristics Entries

//      IMAGE_LIBRARY_PROCESS_INIT            0x0001     // Reserved.
//      IMAGE_LIBRARY_PROCESS_TERM            0x0002     // Reserved.
//      IMAGE_LIBRARY_THREAD_INIT             0x0004     // Reserved.
//      IMAGE_LIBRARY_THREAD_TERM             0x0008     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA    0x0020  // Image can handle a high entropy 64-bit virtual address space.
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040     // DLL can move.
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY    0x0080     // Code Integrity Image
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT    0x0100     // Image is NX compatible
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200     // Image understands isolation and doesn't want it
#define IMAGE_DLLCHARACTERISTICS_NO_SEH       0x0400     // Image does not use SEH.  No SE handler may reside in this image
#define IMAGE_DLLCHARACTERISTICS_NO_BIND      0x0800     // Do not bind this image.
#define IMAGE_DLLCHARACTERISTICS_APPCONTAINER 0x1000     // Image should execute in an AppContainer
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER   0x2000     // Driver uses WDM model
#define IMAGE_DLLCHARACTERISTICS_GUARD_CF     0x4000     // Image supports Control Flow Guard.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

*/
WORD_FLAG_DESCRIPTIONS DllCharacteristics[] =
{
    { IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA, "64BIT_VA"},
    { IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE , "DYNAMIC"},
    { IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY, "INTEGRITY" }, //    0x0080     // Code Integrity Image
    { IMAGE_DLLCHARACTERISTICS_NX_COMPAT, "NX_CONPAT" },    //    0x0100     // Image is NX compatible
    { IMAGE_DLLCHARACTERISTICS_NO_ISOLATION, "NO_ISO" },    // 0x0200     // Image understands isolation and doesn't want it
    { IMAGE_DLLCHARACTERISTICS_NO_SEH, "NO_SEH" },  //       0x0400     // Image does not use SEH.  No SE handler may reside in this image
    { IMAGE_DLLCHARACTERISTICS_NO_BIND, "NO_BIND" },    //     0x0800     // Do not bind this image.
    { IMAGE_DLLCHARACTERISTICS_APPCONTAINER, "APP_CONT" },  // 0x1000     // Image should execute in an AppContainer
    { IMAGE_DLLCHARACTERISTICS_WDM_DRIVER, "WDM_DRIVER" },  //   0x2000     // Driver uses WDM model
    { IMAGE_DLLCHARACTERISTICS_GUARD_CF, "GUARD_CF" },  //     0x4000     // Image supports Control Flow Guard.
    { IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE, "TX_AWARE" }  //     0x8000
};
#define NUMBER_DLL_CHARACTERISTICS \
    (sizeof(DllCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))

// Names of the data directory elements that are defined
/* 20170324 - from winnt.h
// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

*/
char *ImageDirectoryNames[] = {
    "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY", "BASERELOC",
    "DEBUG",
    // "COPYRIGHT",
    "ARCHITECTURE",
    "GLOBALPTR", "TLS", "LOAD_CONFIG",
    "BOUND_IMPORT", "IAT",  // These two entries added for NT 3.51
    "DELAY_IMPORT",		// This entry added in NT 5
    "COM_DIR",
    "RESERVED" };

#define NUMBER_IMAGE_DIRECTORY_ENTRYS \
    (sizeof(ImageDirectoryNames)/sizeof(char *))


//
// Dump the IMAGE_OPTIONAL_HEADER from a PE file
//
//void DumpOptionalHeader(PIMAGE_OPTIONAL_HEADER optionalHeader)

// PIMAGE_NT_HEADERS
void DumpOptionalHeader(PIMAGE_NT_HEADERS pNTHeader, char *base)
{
    PIMAGE_OPTIONAL_HEADER optionalHeader = &pNTHeader->OptionalHeader;
    UINT width = 30;
    char *s;
    UINT i, cnt = 0;
    int Is32 = 1;
    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)pNTHeader;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)pNTHeader;
    PIMAGE_OPTIONAL_HEADER64 opt64 = 0;
    PIMAGE_OPTIONAL_HEADER32 opt32 = 0;

    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        // offset = header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        // exports = (PIMAGE_EXPORT_DIRECTORY)(base + offset);
        Is32 = 1;
        opt32 = &header32->OptionalHeader;
    }
    else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        // offset = header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        // exports = (PIMAGE_EXPORT_DIRECTORY)(base + offset);
        opt64 = &header64->OptionalHeader;
        Is32 = 0;
    }
    else {
        SPRTF("Is NOT a PE32 nor PE32+ magic value! Got %x\n", header32->OptionalHeader.Magic);
        return;
    }


    if (fDumpOptionalHeader)
    {
        /*
        DWORD   SizeOfCode;
        DWORD   SizeOfInitializedData;
        DWORD   SizeOfUninitializedData;
        DWORD   AddressOfEntryPoint;
        DWORD   BaseOfCode;
        */
        SPRTF("Optional Header: %s\n",
            (Is32 ? "PE32" : "PE32+") );

        SPRTF("  %-*s%04X\n", width, "Magic", 
            (Is32 ? opt32->Magic : opt64->Magic));

        SPRTF("  %-*s%u.%02u\n", width, "linker version",
            optionalHeader->MajorLinkerVersion,
            optionalHeader->MinorLinkerVersion);
        SPRTF("  %-*s%X\n", width, "size of code", optionalHeader->SizeOfCode);

        SPRTF("  %-*s%X\n", width, "size of initialized data",
            optionalHeader->SizeOfInitializedData);
        SPRTF("  %-*s%X\n", width, "size of uninitialized data",
            optionalHeader->SizeOfUninitializedData);
        SPRTF("  %-*s%X\n", width, "entrypoint RVA",
            optionalHeader->AddressOfEntryPoint);
        SPRTF("  %-*s%X\n", width, "base of code", optionalHeader->BaseOfCode);

        // TODO: This could be a dump of a 32-bit PE files, in which case need to use DWORD BaseOfData
        if (Is32) {
            /*
            DWORD   BaseOfData;
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
            */
            SPRTF("  %-*s%X\n", width, "base of data", opt32->BaseOfData);
            SPRTF("  %-*s%X\n", width, "base of data", opt32->ImageBase);
            SPRTF("  %-*s%X\n", width, "section align", opt32->SectionAlignment);
            SPRTF("  %-*s%X\n", width, "file align", opt32->FileAlignment);
            SPRTF("  %-*s%u.%02u\n", width, "required OS version",
                opt32->MajorOperatingSystemVersion,
                opt32->MinorOperatingSystemVersion);
            SPRTF("  %-*s%u.%02u\n", width, "image version",
                opt32->MajorImageVersion,
                opt32->MinorImageVersion);
            SPRTF("  %-*s%u.%02u\n", width, "subsystem version",
                opt32->MajorSubsystemVersion,
                opt32->MinorSubsystemVersion);
            SPRTF("  %-*s%X\n", width, "Win32 Version",
                opt32->Win32VersionValue);
            SPRTF("  %-*s%X\n", width, "size of image", opt32->SizeOfImage);
            SPRTF("  %-*s%X\n", width, "size of headers",
                opt32->SizeOfHeaders);
            SPRTF("  %-*s%X\n", width, "checksum", opt32->CheckSum);
            switch (opt32->Subsystem)
            {
            case IMAGE_SUBSYSTEM_NATIVE: s = "Native"; break;
            case IMAGE_SUBSYSTEM_WINDOWS_GUI: s = "Windows GUI"; break;
            case IMAGE_SUBSYSTEM_WINDOWS_CUI: s = "Windows character"; break;
            case IMAGE_SUBSYSTEM_OS2_CUI: s = "OS/2 character"; break;
            case IMAGE_SUBSYSTEM_POSIX_CUI: s = "Posix character"; break;
            default: s = "unknown";
            }
            SPRTF("  %-*s%04X (%s)\n", width, "Subsystem",
                opt32->Subsystem, s);
            // Marked as obsolete in MSDN CD 9
            SPRTF("  %-*s%04X\n", width, "DLL flags",
                opt32->DllCharacteristics);
            cnt = 0;
            for (i = 0; i < NUMBER_DLL_CHARACTERISTICS; i++)
            {
                if (opt32->DllCharacteristics & DllCharacteristics[i].flag) {
                    cnt++;
                    SPRTF("  %s", DllCharacteristics[i].name);
                }
            }
            if (cnt)
                SPRTF("\n");

            SPRTF("  %-*s%X\n", width, "stack reserve size",
                opt32->SizeOfStackReserve);
            SPRTF("  %-*s%X\n", width, "stack commit size",
                opt32->SizeOfStackCommit);
            SPRTF("  %-*s%X\n", width, "heap reserve size",
                opt32->SizeOfHeapReserve);
            SPRTF("  %-*s%X\n", width, "heap commit size",
                opt32->SizeOfHeapCommit);
            SPRTF("  %-*s%X\n", width, "loader flag",
                opt32->LoaderFlags);
            SPRTF("  %-*s%X\n", width, "RVAs & sizes",
                opt32->NumberOfRvaAndSizes);
        }
        else {
            /*
            ULONGLONG   ImageBase;
            DWORD       SectionAlignment;
            DWORD       FileAlignment;
            WORD        MajorOperatingSystemVersion;
            WORD        MinorOperatingSystemVersion;
            WORD        MajorImageVersion;
            WORD        MinorImageVersion;
            WORD        MajorSubsystemVersion;
            WORD        MinorSubsystemVersion;
            DWORD       Win32VersionValue;
            DWORD       SizeOfImage;
            DWORD       SizeOfHeaders;
            DWORD       CheckSum;
            WORD        Subsystem;
            WORD        DllCharacteristics;
            //
            ULONGLONG   SizeOfStackReserve;
            ULONGLONG   SizeOfStackCommit;
            ULONGLONG   SizeOfHeapReserve;
            ULONGLONG   SizeOfHeapCommit;
            DWORD       LoaderFlags;
            DWORD       NumberOfRvaAndSizes;
            IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];

            */
            SPRTF("  %-*s%I64X\n", width, "base of data", opt64->ImageBase);
            SPRTF("  %-*s%X\n", width, "file align", opt64->FileAlignment);
            SPRTF("  %-*s%u.%02u\n", width, "required OS version",
                opt64->MajorOperatingSystemVersion,
                opt64->MinorOperatingSystemVersion);
            SPRTF("  %-*s%u.%02u\n", width, "image version",
                opt64->MajorImageVersion,
                opt64->MinorImageVersion);
            SPRTF("  %-*s%u.%02u\n", width, "subsystem version",
                opt64->MajorSubsystemVersion,
                opt64->MinorSubsystemVersion);
            SPRTF("  %-*s%X\n", width, "Win32 Version",
                opt64->Win32VersionValue);
            SPRTF("  %-*s%X\n", width, "size of image", opt64->SizeOfImage);
            SPRTF("  %-*s%X\n", width, "size of headers",
                opt64->SizeOfHeaders);
            SPRTF("  %-*s%X\n", width, "checksum", opt64->CheckSum);
            switch (opt64->Subsystem)
            {
            case IMAGE_SUBSYSTEM_NATIVE: s = "Native"; break;
            case IMAGE_SUBSYSTEM_WINDOWS_GUI: s = "Windows GUI"; break;
            case IMAGE_SUBSYSTEM_WINDOWS_CUI: s = "Windows character"; break;
            case IMAGE_SUBSYSTEM_OS2_CUI: s = "OS/2 character"; break;
            case IMAGE_SUBSYSTEM_POSIX_CUI: s = "Posix character"; break;
            default: s = "unknown";
            }
            SPRTF("  %-*s%04X (%s)\n", width, "Subsystem",
                opt64->Subsystem, s);
            // Marked as obsolete in MSDN CD 9
            SPRTF("  %-*s%04X\n", width, "DLL flags",
                opt64->DllCharacteristics);
            cnt = 0;
            for (i = 0; i < NUMBER_DLL_CHARACTERISTICS; i++)
            {
                if (opt64->DllCharacteristics & DllCharacteristics[i].flag) {
                    cnt++;
                    SPRTF("  %s", DllCharacteristics[i].name);
                }
            }
            if (cnt)
                SPRTF("\n");

            SPRTF("  %-*s%I64X\n", width, "stack reserve size",
                opt64->SizeOfStackReserve);
            SPRTF("  %-*s%I64X\n", width, "stack commit size",
                opt64->SizeOfStackCommit);
            SPRTF("  %-*s%I64X\n", width, "heap reserve size",
                opt64->SizeOfHeapReserve);
            SPRTF("  %-*s%I64X\n", width, "heap commit size",
                opt64->SizeOfHeapCommit);
            SPRTF("  %-*s%X\n", width, "loader flag",
                opt64->LoaderFlags);
            SPRTF("  %-*s%X\n", width, "RVAs & sizes",
                opt64->NumberOfRvaAndSizes);

        }


#if 0
        // Marked as obsolete in MSDN CD 9
        SPRTF("  %-*s%08X\n", width, "loader flags",
            optionalHeader->LoaderFlags);

        for (i = 0; i < NUMBER_LOADER_FLAGS; i++)
        {
            if (optionalHeader->LoaderFlags &
                LoaderFlags[i].flag)
                SPRTF("  %s", LoaderFlags[i].name);
        }
        if (optionalHeader->LoaderFlags)
            SPRTF("\n");
#endif


        SPRTF("\n");
    }

    if (fDumpDataDirectory)
    {
        UINT max = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
        SPRTF("\nData Directory: Count %u (%x)\n", max, max);
        //for (i = 0; i < optionalHeader->NumberOfRvaAndSizes; i++)
        for (i = 0; i < max; i++)
        {
            if (Is32) {
                SPRTF("  %-12s rva: %08X  size: %08X\n",
                    (i >= NUMBER_IMAGE_DIRECTORY_ENTRYS)
                    ? "unused" : ImageDirectoryNames[i],
                    opt32->DataDirectory[i].VirtualAddress,
                    opt32->DataDirectory[i].Size);
            }
            else {
                SPRTF("  %-12s rva: %08X  size: %08X\n",
                    (i >= NUMBER_IMAGE_DIRECTORY_ENTRYS)
                    ? "unused" : ImageDirectoryNames[i],
                    opt64->DataDirectory[i].VirtualAddress,
                    opt64->DataDirectory[i].Size);
            }
        }
    }
}

LPVOID GetFileMap(const char *szFileName)
{
    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "failed to open file " << szFileName << endl;
        return 0;
    }
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0)
    {
        cerr << "failed to map file " << szFileName << endl;
        CloseHandle(hFile);
        return 0;
    }
    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0)
    {
        cerr << "failed to get map view of file " << szFileName << endl;
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 0;
    }
    return lpFileBase;
}

void CloseFileMap()
{
    if (lpFileBase)
        UnmapViewOfFile(lpFileBase);
    if (hFileMapping)
        CloseHandle(hFileMapping);
    if (hFile && (hFile != INVALID_HANDLE_VALUE))
        CloseHandle(hFile);
    hFile = 0;
    hFileMapping = 0;
    lpFileBase = 0;

}


bool GetDLLFileExports(char *szFileName, unsigned int *nNoOfExports, char **&pszFunctions)
{
    PIMAGE_DOS_HEADER pImg_DOS_Header;
    PIMAGE_NT_HEADERS pImg_NT_Header;
    PIMAGE_EXPORT_DIRECTORY pImg_Export_Dir;

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("failed to open file '%s'\n", szFileName);
        return false;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0)
    {
        printf("failed to map file '%s'\n", szFileName);
        CloseHandle(hFile);
        return false;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0)
    {
        printf("failed to view file '%s'\n", szFileName);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return false;
    }

    pImg_DOS_Header = (PIMAGE_DOS_HEADER)lpFileBase;
    char *base = (char *)lpFileBase;
    DWORD64 offset = 0;
    pImg_NT_Header = (PIMAGE_NT_HEADERS)((char *)pImg_DOS_Header + (LONG)pImg_DOS_Header->e_lfanew);


    if (IsBadReadPtr(pImg_NT_Header, sizeof(IMAGE_NT_HEADERS)) || pImg_NT_Header->Signature != IMAGE_NT_SIGNATURE)
    {
        printf("failed to read or get correct signature of file '%s'\n", szFileName);
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return false;
    }

    DumpHeader((PIMAGE_FILE_HEADER)&pImg_NT_Header->FileHeader);
    DumpOptionalHeader(pImg_NT_Header, base);
    //DumpOptionalHeader((PIMAGE_OPTIONAL_HEADER)&pImg_NT_Header->OptionalHeader);


    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)pImg_NT_Header;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)pImg_NT_Header;
    PIMAGE_EXPORT_DIRECTORY exports = NULL;
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pImg_NT_Header);
    bool Is32 = true;

    pImg_Export_Dir = (PIMAGE_EXPORT_DIRECTORY)pImg_NT_Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        Is32 = true;
        section = IMAGE_FIRST_SECTION(header32);
        offset = header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + offset);
    }
    else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        Is32 = false;
        offset = header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + offset);
        section = IMAGE_FIRST_SECTION(header64);
    }
    else {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return false;
    }
    UINT i;
    if (Is32) {
        for (i = 0; i < header32->FileHeader.NumberOfSections; i++, section++)
        {

        }
    }
    else {
        for (i = 0; i < header64->FileHeader.NumberOfSections; i++, section++)
        {

        }
    }


    if (!pImg_Export_Dir)
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return false;
    }

    pImg_Export_Dir = (PIMAGE_EXPORT_DIRECTORY)ImageRvaToVa(pImg_NT_Header, pImg_DOS_Header, (DWORD)pImg_Export_Dir, 0);

    DWORD cnt = pImg_Export_Dir->NumberOfNames;
    DWORD **ppdwNames = (DWORD **)pImg_Export_Dir->AddressOfNames;

    ppdwNames = (PDWORD*)ImageRvaToVa(pImg_NT_Header, pImg_DOS_Header, (DWORD)ppdwNames, 0);
    if (!ppdwNames || !cnt)
    {
        printf("no address of 'name' found!\n");
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return false;
    }

    *nNoOfExports = cnt; // pImg_Export_Dir->NumberOfNames;

    pszFunctions = new char*[cnt + 1];

    for (unsigned i = 0; i < *nNoOfExports; i++)
    {
        char *szFunc = (PSTR)ImageRvaToVa(pImg_NT_Header, pImg_DOS_Header, (DWORD)*ppdwNames, 0);
        if (szFunc && strlen(szFunc)) {
            pszFunctions[i] = new char[strlen(szFunc) + 1];
            strcpy(pszFunctions[i], szFunc);
        }
        else {
            pszFunctions[i] = 0;
        }

        ppdwNames++;
    }
    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    return true;
}

DWORD ProcessFile(const char *fileName)
{
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    DWORD64 base;
    DWORD64 exportsStartRVA, exportsEndRVA;
    PIMAGE_EXPORT_DIRECTORY exportDir;
    DWORD err = 0;
    hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        return err;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0)
    {
        err = GetLastError();
        CloseHandle(hFile);
        return err;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0)
    {
        err = GetLastError();
        CloseHandle(hFile);
        CloseHandle(hFileMapping);
        return err;
    }
    //printf("loaded and mapped file '%s'\n", fileName);

    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if (dosHeader->e_magic != 0x5a4d) {
        printf("Does not have PE magic!\n");
        CloseMappedFile(fileName);
        return 1;
    }

    pNTHeader = (PIMAGE_NT_HEADERS)((BYTE*)dosHeader + dosHeader->e_lfanew);
    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)pNTHeader;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)pNTHeader;
    base = (DWORD64)dosHeader;
    PIMAGE_EXPORT_DIRECTORY exports = NULL;
    bool Is32 = true;

    exportsStartRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    exportsEndRVA = exportsStartRVA + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        exportsStartRVA = header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        exportsEndRVA = exportsStartRVA + header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    }
    else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        exports = (PIMAGE_EXPORT_DIRECTORY)(base + header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        exportsStartRVA = header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        exportsEndRVA = exportsStartRVA + header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        Is32 = false;
    }
    else {
        printf("File '%s' does not have PE HDR32 or HDR64 magic!\n", fileName);
        CloseMappedFile(fileName);
        return 1;
    }
    exportDir = exports;
    printf("Dump of '%s'\n", fileName);

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;
    PIMAGE_SECTION_HEADER header = 0;
    if (Is32)
        section = IMAGE_FIRST_SECTION(header32);
    else
        section = IMAGE_FIRST_SECTION(header64);
    for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
    {
        // Is the RVA within this section?
        if ((exportsStartRVA >= section->VirtualAddress) &&
            (exportsStartRVA < (section->VirtualAddress + section->Misc.VirtualSize))) {
            header = section;
            break;
        }
    }
    if (!header) {
        CloseMappedFile(fileName);
        return 1;
    }


    // AddressOfNames is a RVA to a list of RVAs to string names not a RVA to a list of strings.
    // exportDir = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)base + exportsStartRVA + section->PointerToRawData - section->VirtualAddress);
    DWORD64 delta1 = header->PointerToRawData - header->VirtualAddress;
    DWORD64 delta2 = header->VirtualAddress - header->PointerToRawData;

    //PDWORD pfunctions = (PDWORD)((PBYTE)base + (DWORD64)exportDir->AddressOfFunctions + header->PointerToRawData - header->VirtualAddress);
    //PDWORD ordinals = (PDWORD)((PBYTE)base + (DWORD64)exportDir->AddressOfNameOrdinals + header->PointerToRawData - header->VirtualAddress);
    //PSTR* name = (PSTR*)((PBYTE)base + (DWORD64)exportDir->AddressOfNames + header->PointerToRawData - header->VirtualAddress);
    PDWORD pfunctions = (PDWORD)((PBYTE)base + (DWORD64)exportDir->AddressOfFunctions + delta2);
    PDWORD ordinals = (PDWORD)((PBYTE)base + (DWORD64)exportDir->AddressOfNameOrdinals + delta2);
    PSTR* name = (PSTR*)((PBYTE)base + (DWORD64)exportDir->AddressOfNames + delta2);
    PDWORD nameRVA = (PDWORD)name;
    DWORD funcRVA;
    PSTR funcName;


    for (unsigned i = 0; i < (DWORD64)exportDir->NumberOfNames; i++)
    {
        funcRVA = nameRVA[i];
        funcName = name[i];
    }

    CloseMappedFile(fileName);
    return err;
}



int parse_args( int argc, char **argv )
{
    int i,i2,c;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c) {
            case 'h':
            case '?':
                give_help(argv[0]);
                return 2;
                break;
            // TODO: Other arguments
            default:
                printf("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument
            if (usr_input) {
                printf("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = _strdup(arg);
        }
    }
#ifndef NDEBUG
    if (!usr_input) {
        //usr_input = "D:\\Tidy\\tidy-html5\\build\\tempmt\\Release\\tidy.exe";
        //usr_input = "D:\\Tidy\\tidy-html5\\build\\tempmt\\Release\\tidy.dll";
        //usr_input = "C:\\Windows\\system32\\kernel32.dll";
        usr_input = "F:\\Projects\\tidy-html5\\build\\win32-mt\\Release\\tidy.exe";
    }
#endif
    if (!usr_input) {
        printf("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

// main() OS entry
int old_main( int argc, char **argv )
{
    int iret = 0;
    iret = parse_args(argc,argv);
    if (iret) {
        if (iret == 2)
            iret = 0;
        return iret;
    }
    unsigned int i, cnt = 0;
    char **pszFunctions = 0;
    int showDLLExports = 1;

    if (showDLLExports) {

        if (GetDLLFileExports((char *)usr_input, &cnt, pszFunctions)) {
            if (pszFunctions) {
                printf("found %u functions exported...\n", cnt);
                for (i = 0; i < cnt; i++) {
                    if (pszFunctions[i]) {
                        printf("%d: %s\n", (i + 1), pszFunctions[i]);
                        delete pszFunctions[i];
                    }
                }
                delete pszFunctions;
            }
        }
        else {
            iret |= 1;
        }
    }

    // iret |= ProcessFile(usr_input);  // actions of app

    return iret;
}

// from : https://www.experts-exchange.com/questions/21066562/Listing-the-Export-Address-Table-EAT-IMAGE-EXPORT-DIRECTORY.html
// #define USE_GETMODULE_HANDLE
#define RVAToPtr(BASE,RVA) ( ((LPBYTE)BASE) + ((DWORD)(RVA)) )

// ================================================================================================
// see : http://stackoverflow.com/questions/26616379/pimage-export-directory-memory-access-error
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
// ================================================================================================
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva,
    PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)pNTHeader;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)pNTHeader;
    unsigned i, max;
    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        section = IMAGE_FIRST_SECTION(header32);
        max = header32->FileHeader.NumberOfSections;
    }
    else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        section = IMAGE_FIRST_SECTION(header64);
        max = header64->FileHeader.NumberOfSections;
    }
    else
        return 0;

    for (i = 0; i < max; i++, section++)
    {
        // Is the RVA within this section?
        if ((rva >= section->VirtualAddress) &&
            (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }

    return 0;
}

LPVOID GetPtrFromRVA(DWORD rva, PIMAGE_NT_HEADERS pNTHeader, char *imageBase)
{
    PIMAGE_SECTION_HEADER pSectionHdr;
    UINT delta;

    pSectionHdr = GetEnclosingSectionHeader(rva, pNTHeader);
    if (!pSectionHdr)
        return 0;

    delta = (UINT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
    return (PVOID)(imageBase + (rva - delta));
}


int main(int argc, char **argv)
{
    IMAGE_DOS_HEADER *dosHeader;
#ifdef USE_GETMODULE_HANDLE
    HINSTANCE hInstance;
    hInstance = GetModuleHandle("kernel32.dll");
    dosHeader = (IMAGE_DOS_HEADER *)hInstance;
#else
    dosHeader = (PIMAGE_DOS_HEADER)GetFileMap("C:\\Windows\\System32\\kernel32.dll");
    if (!dosHeader)
    {
        cerr << "Failed to open file!" << endl;
        return 1;
    }
#endif

    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        cerr << "Failed to get DOS signature!" << endl;
        //MessageBox(0, "1", "", 0);
#ifndef USE_GETMODULE_HANDLE
        CloseFileMap();
#endif
        return 1;
    }
    char *base = (char *)dosHeader;
    IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *)(((BYTE *)dosHeader) + dosHeader->e_lfanew);
    PIMAGE_NT_HEADERS32 header32 = (PIMAGE_NT_HEADERS32)ntHeaders;
    PIMAGE_NT_HEADERS64 header64 = (PIMAGE_NT_HEADERS64)ntHeaders;

    if (ntHeaders->Signature != 0x00004550)
    {
        cerr << "Failed to get NT signature!" << endl;
        //MessageBox(0, "2", "", 0);
#ifndef USE_GETMODULE_HANDLE
        CloseFileMap();
#endif
        return 1;
    }
    bool is32 = true;
    IMAGE_OPTIONAL_HEADER *optionalHeader = &ntHeaders->OptionalHeader;
    IMAGE_DATA_DIRECTORY *dataDirectory = &optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    IMAGE_EXPORT_DIRECTORY *Exp;
    DWORD Rva = dataDirectory->VirtualAddress;
    Exp = (IMAGE_EXPORT_DIRECTORY *)((DWORD)dosHeader + dataDirectory->VirtualAddress);

    if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) { // PE32
        Rva = header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        Exp = (PIMAGE_EXPORT_DIRECTORY)(base + header32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    }
    else if (header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) { // PE32+
        Rva = header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        Exp = (PIMAGE_EXPORT_DIRECTORY)(base + header64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        is32 = false;
    }
    else
    {
        cerr << "Not HDR32 or HDR64!" << endl;
#ifndef USE_GETMODULE_HANDLE
        CloseFileMap();
#endif
        return 1;    // Get the IMAGE_SECTION_HEADER that contains the exports.  This is
    }

    int count = 0;
#ifndef USE_GETMODULE_HANDLE
    Exp = (PIMAGE_EXPORT_DIRECTORY)GetPtrFromRVA(Rva, ntHeaders, base);
#endif
    cout << "Exp.Dir offset  " << ((char *)Exp - base) << endl;
    cout << "Exp.Addr offset " << Exp->AddressOfNames << endl;

#ifdef USE_GETMODULE_HANDLE
    ULONG * addressofnames = (ULONG*)((BYTE*)hInstance + Exp->AddressOfNames);
#else
    ULONG * addressofnames = (ULONG*)((BYTE*)base + Exp->AddressOfNames);
#endif
    cout << "Address offset " << ((char *)addressofnames - base) << endl;
    cout << "List of " << Exp->NumberOfNames << " Export Names, kernel32.dll..." << endl;
    for (count = 0; count < Exp->NumberOfNames; count++)
    {
#ifdef USE_GETMODULE_HANDLE
        char*functionname = (char*)((BYTE*)hInstance + addressofnames[count]);
#else
        Rva = addressofnames[count];
        char *functionname = (char*)((BYTE*)base + addressofnames[count]);
        functionname = (char *)GetPtrFromRVA(Rva, ntHeaders, base);
        if (!functionname)
            break;
#endif
        cout << (count + 1) << ": " << functionname << endl;
    }
#ifndef USE_GETMODULE_HANDLE
    CloseFileMap();
#endif
    return 0;
}


// eof = dumpexe.cxx
