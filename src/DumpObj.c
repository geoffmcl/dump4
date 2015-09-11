
// DumpObj.c
// Dump of OBJECT (*.obj) file
#include	"Dump4.h"

#ifndef USE_PEDUMP_CODE // FIX20080507
#include <time.h>
#include <Winsock2.h>
// #include <Dbghelp.h> // for ImageNtHeader

extern void ProcessHex( PBYTE pb, DWORD len );

/* from winnt.h
#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_VXD_SIGNATURE                 0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

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

typedef struct _IMAGE_OS2_HEADER {      // OS/2 .EXE header
    WORD   ne_magic;                    // Magic number
    CHAR   ne_ver;                      // Version number
    CHAR   ne_rev;                      // Revision number
    WORD   ne_enttab;                   // Offset of Entry Table
    WORD   ne_cbenttab;                 // Number of bytes in Entry Table
    LONG   ne_crc;                      // Checksum of whole file
    WORD   ne_flags;                    // Flag word
    WORD   ne_autodata;                 // Automatic data segment number
    WORD   ne_heap;                     // Initial heap allocation
    WORD   ne_stack;                    // Initial stack allocation
    LONG   ne_csip;                     // Initial CS:IP setting
    LONG   ne_sssp;                     // Initial SS:SP setting
    WORD   ne_cseg;                     // Count of file segments
    WORD   ne_cmod;                     // Entries in Module Reference Table
    WORD   ne_cbnrestab;                // Size of non-resident name table
    WORD   ne_segtab;                   // Offset of Segment Table
    WORD   ne_rsrctab;                  // Offset of Resource Table
    WORD   ne_restab;                   // Offset of resident name table
    WORD   ne_modtab;                   // Offset of Module Reference Table
    WORD   ne_imptab;                   // Offset of Imported Names Table
    LONG   ne_nrestab;                  // Offset of Non-resident Names Table
    WORD   ne_cmovent;                  // Count of movable entries
    WORD   ne_align;                    // Segment alignment shift count
    WORD   ne_cres;                     // Count of resource segments
    BYTE   ne_exetyp;                   // Target Operating system
    BYTE   ne_flagsothers;              // Other .EXE flags
    WORD   ne_pretthunks;               // offset to return thunks
    WORD   ne_psegrefbytes;             // offset to segment ref. bytes
    WORD   ne_swaparea;                 // Minimum code swap area size
    WORD   ne_expver;                   // Expected Windows version number
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_VXD_HEADER {      // Windows VXD header
    WORD   e32_magic;                   // Magic number
    BYTE   e32_border;                  // The byte ordering for the VXD
    BYTE   e32_worder;                  // The word ordering for the VXD
    DWORD  e32_level;                   // The EXE format level for now = 0
    WORD   e32_cpu;                     // The CPU type
    WORD   e32_os;                      // The OS type
    DWORD  e32_ver;                     // Module version
    DWORD  e32_mflags;                  // Module flags
    DWORD  e32_mpages;                  // Module # pages
    DWORD  e32_startobj;                // Object # for instruction pointer
    DWORD  e32_eip;                     // Extended instruction pointer
    DWORD  e32_stackobj;                // Object # for stack pointer
    DWORD  e32_esp;                     // Extended stack pointer
    DWORD  e32_pagesize;                // VXD page size
    DWORD  e32_lastpagesize;            // Last page size in VXD
    DWORD  e32_fixupsize;               // Fixup section size
    DWORD  e32_fixupsum;                // Fixup section checksum
    DWORD  e32_ldrsize;                 // Loader section size
    DWORD  e32_ldrsum;                  // Loader section checksum
    DWORD  e32_objtab;                  // Object table offset
    DWORD  e32_objcnt;                  // Number of objects in module
    DWORD  e32_objmap;                  // Object page map offset
    DWORD  e32_itermap;                 // Object iterated data map offset
    DWORD  e32_rsrctab;                 // Offset of Resource Table
    DWORD  e32_rsrccnt;                 // Number of resource entries
    DWORD  e32_restab;                  // Offset of resident name table
    DWORD  e32_enttab;                  // Offset of Entry Table
    DWORD  e32_dirtab;                  // Offset of Module Directive Table
    DWORD  e32_dircnt;                  // Number of module directives
    DWORD  e32_fpagetab;                // Offset of Fixup Page Table
    DWORD  e32_frectab;                 // Offset of Fixup Record Table
    DWORD  e32_impmod;                  // Offset of Import Module Name Table
    DWORD  e32_impmodcnt;               // Number of entries in Import Module Name Table
    DWORD  e32_impproc;                 // Offset of Import Procedure Name Table
    DWORD  e32_pagesum;                 // Offset of Per-Page Checksum Table
    DWORD  e32_datapage;                // Offset of Enumerated Data Pages
    DWORD  e32_preload;                 // Number of preload pages
    DWORD  e32_nrestab;                 // Offset of Non-resident Names Table
    DWORD  e32_cbnrestab;               // Size of Non-resident Name Table
    DWORD  e32_nressum;                 // Non-resident Name Table Checksum
    DWORD  e32_autodata;                // Object # for automatic data object
    DWORD  e32_debuginfo;               // Offset of the debugging information
    DWORD  e32_debuglen;                // The length of the debugging info. in bytes
    DWORD  e32_instpreload;             // Number of instance pages in preload section of VXD file
    DWORD  e32_instdemand;              // Number of instance pages in demand load section of VXD file
    DWORD  e32_heapsize;                // Size of heap - for 16-bit apps
    BYTE   e32_res3[12];                // Reserved words
    DWORD  e32_winresoff;
    DWORD  e32_winreslen;
    WORD   e32_devid;                   // Device ID for VxD
    WORD   e32_ddkver;                  // DDK version for VxD
  } IMAGE_VXD_HEADER, *PIMAGE_VXD_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

//
// Directory format.
//
typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
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
    WORD   Magic;
    BYTE   MajorLinkerVersion;
    BYTE   MinorLinkerVersion;
    DWORD  SizeOfCode;
    DWORD  SizeOfInitializedData;
    DWORD  SizeOfUninitializedData;
    DWORD  AddressOfEntryPoint;
    DWORD  BaseOfCode;
    DWORD  BaseOfData;
    DWORD  BaseOfBss;
    DWORD  GprMask;
    DWORD  CprMask[4];
    DWORD  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    WORD        Magic;
    BYTE        MajorLinkerVersion;
    BYTE        MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       SizeOfInitializedData;
    DWORD       SizeOfUninitializedData;
    DWORD       AddressOfEntryPoint;
    DWORD       BaseOfCode;
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

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#ifdef _WIN64
typedef IMAGE_OPTIONAL_HEADER64             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL64_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef IMAGE_OPTIONAL_HEADER32             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL32_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR32_MAGIC
#endif

typedef struct _IMAGE_NT_HEADERS64 {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#ifdef _WIN64
typedef IMAGE_NT_HEADERS64                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS64                 PIMAGE_NT_HEADERS;
#else
typedef IMAGE_NT_HEADERS32                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS32                 PIMAGE_NT_HEADERS;
#endif
// IMAGE_FIRST_SECTION doesn't need 32/64 versions since the file header is the same either way.

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((ULONG_PTR)ntheader +                                              \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))
// Subsystem Values

#define IMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI            7   // image runs in the Posix character subsystem.
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS       8   // image is a native Win9x driver.
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI       9   // Image runs in the Windows CE subsystem.
#define IMAGE_SUBSYSTEM_EFI_APPLICATION      10  //
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11   //
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER   12  //
#define IMAGE_SUBSYSTEM_EFI_ROM              13
#define IMAGE_SUBSYSTEM_XBOX                 14
// DllCharacteristics Entries

//      IMAGE_LIBRARY_PROCESS_INIT           0x0001     // Reserved.
//      IMAGE_LIBRARY_PROCESS_TERM           0x0002     // Reserved.
//      IMAGE_LIBRARY_THREAD_INIT            0x0004     // Reserved.
//      IMAGE_LIBRARY_THREAD_TERM            0x0008     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200    // Image understands isolation and doesn't want it
#define IMAGE_DLLCHARACTERISTICS_NO_SEH      0x0400     // Image does not use SEH.  No SE handler may reside in this image
#define IMAGE_DLLCHARACTERISTICS_NO_BIND     0x0800     // Do not bind this image.
//                                           0x1000     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER  0x2000     // Driver uses WDM model
//                                           0x4000     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

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

//
// Non-COFF Object file header
//

typedef struct ANON_OBJECT_HEADER {
    WORD    Sig1;            // Must be IMAGE_FILE_MACHINE_UNKNOWN
    WORD    Sig2;            // Must be 0xffff
    WORD    Version;         // >= 1 (implies the CLSID field is present)
    WORD    Machine;
    DWORD   TimeDateStamp;
    CLSID   ClassID;         // Used to invoke CoCreateInstance
    DWORD   SizeOfData;      // Size of data that follows the header
} ANON_OBJECT_HEADER;

//
// Section header format.
//

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

#define IMAGE_SIZEOF_SECTION_HEADER          40

//
// Section characteristics.
//
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
#define IMAGE_SCN_GPREL                      0x00008000  // Section content can be accessed relative to GP
#define IMAGE_SCN_MEM_FARDATA                0x00008000
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000  //
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000  //
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000  //
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000  //
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000  //
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000  //
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000  //
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000  //
// Unused                                    0x00F00000
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// TLS Chaacteristic Flags
//
#define IMAGE_SCN_SCALE_INDEX                0x00000001  // Tls index is scaled

//
// Symbol format.
//

typedef struct _IMAGE_SYMBOL {
    union {
        BYTE    ShortName[8];
        struct {
            DWORD   Short;     // if 0, use LongName
            DWORD   Long;      // offset into string table
        } Name;
        DWORD   LongName[2];    // PBYTE [2]
    } N;
    DWORD   Value;
    SHORT   SectionNumber;
    WORD    Type;
    BYTE    StorageClass;
    BYTE    NumberOfAuxSymbols;
} IMAGE_SYMBOL;
typedef IMAGE_SYMBOL UNALIGNED *PIMAGE_SYMBOL;


#define IMAGE_SIZEOF_SYMBOL                  18

//
// Section values.
//
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
//

#define IMAGE_SYM_UNDEFINED           (SHORT)0          // Symbol is undefined or is common.
#define IMAGE_SYM_ABSOLUTE            (SHORT)-1         // Symbol is an absolute value.
#define IMAGE_SYM_DEBUG               (SHORT)-2         // Symbol is a special debug item.
#define IMAGE_SYM_SECTION_MAX         0xFEFF            // Values 0xFF00-0xFFFF are special

//
// Type (fundamental) values.
//

#define IMAGE_SYM_TYPE_NULL                 0x0000  // no type.
#define IMAGE_SYM_TYPE_VOID                 0x0001  //
#define IMAGE_SYM_TYPE_CHAR                 0x0002  // type character.
#define IMAGE_SYM_TYPE_SHORT                0x0003  // type short integer.
#define IMAGE_SYM_TYPE_INT                  0x0004  //
#define IMAGE_SYM_TYPE_LONG                 0x0005  //
#define IMAGE_SYM_TYPE_FLOAT                0x0006  //
#define IMAGE_SYM_TYPE_DOUBLE               0x0007  //
#define IMAGE_SYM_TYPE_STRUCT               0x0008  //
#define IMAGE_SYM_TYPE_UNION                0x0009  //
#define IMAGE_SYM_TYPE_ENUM                 0x000A  // enumeration.
#define IMAGE_SYM_TYPE_MOE                  0x000B  // member of enumeration.
#define IMAGE_SYM_TYPE_BYTE                 0x000C  //
#define IMAGE_SYM_TYPE_WORD                 0x000D  //
#define IMAGE_SYM_TYPE_UINT                 0x000E  //
#define IMAGE_SYM_TYPE_DWORD                0x000F  //
#define IMAGE_SYM_TYPE_PCODE                0x8000  //
//
// Type (derived) values.
//

#define IMAGE_SYM_DTYPE_NULL                0       // no derived type.
#define IMAGE_SYM_DTYPE_POINTER             1       // pointer.
#define IMAGE_SYM_DTYPE_FUNCTION            2       // function.
#define IMAGE_SYM_DTYPE_ARRAY               3       // array.

//
// Storage classes.
//
#define IMAGE_SYM_CLASS_END_OF_FUNCTION     (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                0x0000
#define IMAGE_SYM_CLASS_AUTOMATIC           0x0001
#define IMAGE_SYM_CLASS_EXTERNAL            0x0002
#define IMAGE_SYM_CLASS_STATIC              0x0003
#define IMAGE_SYM_CLASS_REGISTER            0x0004
#define IMAGE_SYM_CLASS_EXTERNAL_DEF        0x0005
#define IMAGE_SYM_CLASS_LABEL               0x0006
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL     0x0007
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT    0x0008
#define IMAGE_SYM_CLASS_ARGUMENT            0x0009
#define IMAGE_SYM_CLASS_STRUCT_TAG          0x000A
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION     0x000B
#define IMAGE_SYM_CLASS_UNION_TAG           0x000C
#define IMAGE_SYM_CLASS_TYPE_DEFINITION     0x000D
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC    0x000E
#define IMAGE_SYM_CLASS_ENUM_TAG            0x000F
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM      0x0010
#define IMAGE_SYM_CLASS_REGISTER_PARAM      0x0011
#define IMAGE_SYM_CLASS_BIT_FIELD           0x0012

#define IMAGE_SYM_CLASS_FAR_EXTERNAL        0x0044  //

#define IMAGE_SYM_CLASS_BLOCK               0x0064
#define IMAGE_SYM_CLASS_FUNCTION            0x0065
#define IMAGE_SYM_CLASS_END_OF_STRUCT       0x0066
#define IMAGE_SYM_CLASS_FILE                0x0067
// new
#define IMAGE_SYM_CLASS_SECTION             0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL       0x0069

#define IMAGE_SYM_CLASS_CLR_TOKEN           0x006B

// type packing constants

#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2
// MACROS
// Basic Type of  x
#define BTYPE(x) ((x) & N_BTMASK)

// Is x a pointer?
#ifndef ISPTR
#define ISPTR(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_POINTER << N_BTSHFT))
#endif

// Is x a function?
#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif

// Is x an array?

#ifndef ISARY
#define ISARY(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_ARRAY << N_BTSHFT))
#endif

// Is x a structure, union, or enumeration TAG?
#ifndef ISTAG
#define ISTAG(x) ((x)==IMAGE_SYM_CLASS_STRUCT_TAG || (x)==IMAGE_SYM_CLASS_UNION_TAG || (x)==IMAGE_SYM_CLASS_ENUM_TAG)
#endif

#ifndef INCREF
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(IMAGE_SYM_DTYPE_POINTER<<N_BTSHFT)|((x)&N_BTMASK))
#endif
#ifndef DECREF
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
#endif

//
// Auxiliary entry format.
//

typedef union _IMAGE_AUX_SYMBOL {
    struct {
        DWORD    TagIndex;                      // struct, union, or enum tag index
        union {
            struct {
                WORD    Linenumber;             // declaration line number
                WORD    Size;                   // size of struct, union, or enum
            } LnSz;
           DWORD    TotalSize;
        } Misc;
        union {
            struct {                            // if ISFCN, tag, or .bb
                DWORD    PointerToLinenumber;
                DWORD    PointerToNextFunction;
            } Function;
            struct {                            // if ISARY, up to 4 dimen.
                WORD     Dimension[4];
            } Array;
        } FcnAry;
        WORD    TvIndex;                        // tv index
    } Sym;
    struct {
        BYTE    Name[IMAGE_SIZEOF_SYMBOL];
    } File;
    struct {
        DWORD   Length;                         // section length
        WORD    NumberOfRelocations;            // number of relocation entries
        WORD    NumberOfLinenumbers;            // number of line numbers
        DWORD   CheckSum;                       // checksum for communal
        SHORT   Number;                         // section number to associate with
        BYTE    Selection;                      // communal selection type
    } Section;
} IMAGE_AUX_SYMBOL;
typedef IMAGE_AUX_SYMBOL UNALIGNED *PIMAGE_AUX_SYMBOL;

#define IMAGE_SIZEOF_AUX_SYMBOL             18

typedef enum IMAGE_AUX_SYMBOL_TYPE {
    IMAGE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} IMAGE_AUX_SYMBOL_TYPE;

typedef struct IMAGE_AUX_SYMBOL_TOKEN_DEF {
    BYTE  bAuxType;                  // IMAGE_AUX_SYMBOL_TYPE
    BYTE  bReserved;                 // Must be 0
    DWORD SymbolTableIndex;
    BYTE  rgbReserved[12];           // Must be 0
} IMAGE_AUX_SYMBOL_TOKEN_DEF;

typedef IMAGE_AUX_SYMBOL_TOKEN_DEF UNALIGNED *PIMAGE_AUX_SYMBOL_TOKEN_DEF;

//
// Communal selection types.
//

#define IMAGE_COMDAT_SELECT_NODUPLICATES    1
#define IMAGE_COMDAT_SELECT_ANY             2
#define IMAGE_COMDAT_SELECT_SAME_SIZE       3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH     4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE     5
#define IMAGE_COMDAT_SELECT_LARGEST         6
#define IMAGE_COMDAT_SELECT_NEWEST          7

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3

//
// Relocation format.
//

typedef struct _IMAGE_RELOCATION {
    union {
        DWORD   VirtualAddress;
        DWORD   RelocCount;             // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
    };
    DWORD   SymbolTableIndex;
    WORD    Type;
} IMAGE_RELOCATION;
typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION         10

//
// I386 relocation types.
//
#define IMAGE_REL_I386_ABSOLUTE         0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_I386_DIR16            0x0001  // Direct 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_REL16            0x0002  // PC-relative 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32            0x0006  // Direct 32-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32NB          0x0007  // Direct 32-bit reference to the symbols virtual address, base not included
#define IMAGE_REL_I386_SEG12            0x0009  // Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define IMAGE_REL_I386_SECTION          0x000A
#define IMAGE_REL_I386_SECREL           0x000B
#define IMAGE_REL_I386_TOKEN            0x000C  // clr token
#define IMAGE_REL_I386_SECREL7          0x000D  // 7 bit offset from base of section containing target
#define IMAGE_REL_I386_REL32            0x0014  // PC-relative 32-bit reference to the symbols virtual address

//
// MIPS relocation types.
//
#define IMAGE_REL_MIPS_ABSOLUTE         0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_MIPS_REFHALF          0x0001
#define IMAGE_REL_MIPS_REFWORD          0x0002
#define IMAGE_REL_MIPS_JMPADDR          0x0003
#define IMAGE_REL_MIPS_REFHI            0x0004
#define IMAGE_REL_MIPS_REFLO            0x0005
#define IMAGE_REL_MIPS_GPREL            0x0006
#define IMAGE_REL_MIPS_LITERAL          0x0007
#define IMAGE_REL_MIPS_SECTION          0x000A
#define IMAGE_REL_MIPS_SECREL           0x000B
#define IMAGE_REL_MIPS_SECRELLO         0x000C  // Low 16-bit section relative referemce (used for >32k TLS)
#define IMAGE_REL_MIPS_SECRELHI         0x000D  // High 16-bit section relative reference (used for >32k TLS)
#define IMAGE_REL_MIPS_TOKEN            0x000E  // clr token
#define IMAGE_REL_MIPS_JMPADDR16        0x0010
#define IMAGE_REL_MIPS_REFWORDNB        0x0022
#define IMAGE_REL_MIPS_PAIR             0x0025

//
// Alpha Relocation types.
//
#define IMAGE_REL_ALPHA_ABSOLUTE        0x0000
#define IMAGE_REL_ALPHA_REFLONG         0x0001
#define IMAGE_REL_ALPHA_REFQUAD         0x0002
#define IMAGE_REL_ALPHA_GPREL32         0x0003
#define IMAGE_REL_ALPHA_LITERAL         0x0004
#define IMAGE_REL_ALPHA_LITUSE          0x0005
#define IMAGE_REL_ALPHA_GPDISP          0x0006
#define IMAGE_REL_ALPHA_BRADDR          0x0007
#define IMAGE_REL_ALPHA_HINT            0x0008
#define IMAGE_REL_ALPHA_INLINE_REFLONG  0x0009
#define IMAGE_REL_ALPHA_REFHI           0x000A
#define IMAGE_REL_ALPHA_REFLO           0x000B
#define IMAGE_REL_ALPHA_PAIR            0x000C
#define IMAGE_REL_ALPHA_MATCH           0x000D
#define IMAGE_REL_ALPHA_SECTION         0x000E
#define IMAGE_REL_ALPHA_SECREL          0x000F
#define IMAGE_REL_ALPHA_REFLONGNB       0x0010
#define IMAGE_REL_ALPHA_SECRELLO        0x0011  // Low 16-bit section relative reference
#define IMAGE_REL_ALPHA_SECRELHI        0x0012  // High 16-bit section relative reference
#define IMAGE_REL_ALPHA_REFQ3           0x0013  // High 16 bits of 48 bit reference
#define IMAGE_REL_ALPHA_REFQ2           0x0014  // Middle 16 bits of 48 bit reference
#define IMAGE_REL_ALPHA_REFQ1           0x0015  // Low 16 bits of 48 bit reference
#define IMAGE_REL_ALPHA_GPRELLO         0x0016  // Low 16-bit GP relative reference
#define IMAGE_REL_ALPHA_GPRELHI         0x0017  // High 16-bit GP relative reference

//
// IBM PowerPC relocation types.
//
#define IMAGE_REL_PPC_ABSOLUTE          0x0000  // NOP
#define IMAGE_REL_PPC_ADDR64            0x0001  // 64-bit address
#define IMAGE_REL_PPC_ADDR32            0x0002  // 32-bit address
#define IMAGE_REL_PPC_ADDR24            0x0003  // 26-bit address, shifted left 2 (branch absolute)
#define IMAGE_REL_PPC_ADDR16            0x0004  // 16-bit address
#define IMAGE_REL_PPC_ADDR14            0x0005  // 16-bit address, shifted left 2 (load doubleword)
#define IMAGE_REL_PPC_REL24             0x0006  // 26-bit PC-relative offset, shifted left 2 (branch relative)
#define IMAGE_REL_PPC_REL14             0x0007  // 16-bit PC-relative offset, shifted left 2 (br cond relative)
#define IMAGE_REL_PPC_TOCREL16          0x0008  // 16-bit offset from TOC base
#define IMAGE_REL_PPC_TOCREL14          0x0009  // 16-bit offset from TOC base, shifted left 2 (load doubleword)

#define IMAGE_REL_PPC_ADDR32NB          0x000A  // 32-bit addr w/o image base
#define IMAGE_REL_PPC_SECREL            0x000B  // va of containing section (as in an image sectionhdr)
#define IMAGE_REL_PPC_SECTION           0x000C  // sectionheader number
#define IMAGE_REL_PPC_IFGLUE            0x000D  // substitute TOC restore instruction iff symbol is glue code
#define IMAGE_REL_PPC_IMGLUE            0x000E  // symbol is glue code; virtual address is TOC restore instruction
#define IMAGE_REL_PPC_SECREL16          0x000F  // va of containing section (limited to 16 bits)
#define IMAGE_REL_PPC_REFHI             0x0010
#define IMAGE_REL_PPC_REFLO             0x0011
#define IMAGE_REL_PPC_PAIR              0x0012
#define IMAGE_REL_PPC_SECRELLO          0x0013  // Low 16-bit section relative reference (used for >32k TLS)
#define IMAGE_REL_PPC_SECRELHI          0x0014  // High 16-bit section relative reference (used for >32k TLS)
#define IMAGE_REL_PPC_GPREL             0x0015
#define IMAGE_REL_PPC_TOKEN             0x0016  // clr token

#define IMAGE_REL_PPC_TYPEMASK          0x00FF  // mask to isolate above values in IMAGE_RELOCATION.Type

// Flag bits in IMAGE_RELOCATION.TYPE

#define IMAGE_REL_PPC_NEG               0x0100  // subtract reloc value rather than adding it
#define IMAGE_REL_PPC_BRTAKEN           0x0200  // fix branch prediction bit to predict branch taken
#define IMAGE_REL_PPC_BRNTAKEN          0x0400  // fix branch prediction bit to predict branch not taken
#define IMAGE_REL_PPC_TOCDEFN           0x0800  // toc slot defined in file (or, data in toc)

//
// Hitachi SH3 relocation types.
//
#define IMAGE_REL_SH3_ABSOLUTE          0x0000  // No relocation
#define IMAGE_REL_SH3_DIRECT16          0x0001  // 16 bit direct
#define IMAGE_REL_SH3_DIRECT32          0x0002  // 32 bit direct
#define IMAGE_REL_SH3_DIRECT8           0x0003  // 8 bit direct, -128..255
#define IMAGE_REL_SH3_DIRECT8_WORD      0x0004  // 8 bit direct .W (0 ext.)
#define IMAGE_REL_SH3_DIRECT8_LONG      0x0005  // 8 bit direct .L (0 ext.)
#define IMAGE_REL_SH3_DIRECT4           0x0006  // 4 bit direct (0 ext.)
#define IMAGE_REL_SH3_DIRECT4_WORD      0x0007  // 4 bit direct .W (0 ext.)
#define IMAGE_REL_SH3_DIRECT4_LONG      0x0008  // 4 bit direct .L (0 ext.)
#define IMAGE_REL_SH3_PCREL8_WORD       0x0009  // 8 bit PC relative .W
#define IMAGE_REL_SH3_PCREL8_LONG       0x000A  // 8 bit PC relative .L
#define IMAGE_REL_SH3_PCREL12_WORD      0x000B  // 12 LSB PC relative .W
#define IMAGE_REL_SH3_STARTOF_SECTION   0x000C  // Start of EXE section
#define IMAGE_REL_SH3_SIZEOF_SECTION    0x000D  // Size of EXE section
#define IMAGE_REL_SH3_SECTION           0x000E  // Section table index
#define IMAGE_REL_SH3_SECREL            0x000F  // Offset within section
#define IMAGE_REL_SH3_DIRECT32_NB       0x0010  // 32 bit direct not based
#define IMAGE_REL_SH3_GPREL4_LONG       0x0011  // GP-relative addressing
#define IMAGE_REL_SH3_TOKEN             0x0012  // clr token

#define IMAGE_REL_ARM_ABSOLUTE          0x0000  // No relocation required
#define IMAGE_REL_ARM_ADDR32            0x0001  // 32 bit address
#define IMAGE_REL_ARM_ADDR32NB          0x0002  // 32 bit address w/o image base
#define IMAGE_REL_ARM_BRANCH24          0x0003  // 24 bit offset << 2 & sign ext.
#define IMAGE_REL_ARM_BRANCH11          0x0004  // Thumb: 2 11 bit offsets
#define IMAGE_REL_ARM_TOKEN             0x0005  // clr token
#define IMAGE_REL_ARM_GPREL12           0x0006  // GP-relative addressing (ARM)
#define IMAGE_REL_ARM_GPREL7            0x0007  // GP-relative addressing (Thumb)
#define IMAGE_REL_ARM_BLX24             0x0008
#define IMAGE_REL_ARM_BLX11             0x0009
#define IMAGE_REL_ARM_SECTION           0x000E  // Section table index
#define IMAGE_REL_ARM_SECREL            0x000F  // Offset within section

#define IMAGE_REL_AM_ABSOLUTE           0x0000
#define IMAGE_REL_AM_ADDR32             0x0001
#define IMAGE_REL_AM_ADDR32NB           0x0002
#define IMAGE_REL_AM_CALL32             0x0003
#define IMAGE_REL_AM_FUNCINFO           0x0004
#define IMAGE_REL_AM_REL32_1            0x0005
#define IMAGE_REL_AM_REL32_2            0x0006
#define IMAGE_REL_AM_SECREL             0x0007
#define IMAGE_REL_AM_SECTION            0x0008
#define IMAGE_REL_AM_TOKEN              0x0009

//
// x64 relocations
//
#define IMAGE_REL_AMD64_ABSOLUTE        0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_AMD64_ADDR64          0x0001  // 64-bit address (VA).
#define IMAGE_REL_AMD64_ADDR32          0x0002  // 32-bit address (VA).
#define IMAGE_REL_AMD64_ADDR32NB        0x0003  // 32-bit address w/o image base (RVA).
#define IMAGE_REL_AMD64_REL32           0x0004  // 32-bit relative address from byte following reloc
#define IMAGE_REL_AMD64_REL32_1         0x0005  // 32-bit relative address from byte distance 1 from reloc
#define IMAGE_REL_AMD64_REL32_2         0x0006  // 32-bit relative address from byte distance 2 from reloc
#define IMAGE_REL_AMD64_REL32_3         0x0007  // 32-bit relative address from byte distance 3 from reloc
#define IMAGE_REL_AMD64_REL32_4         0x0008  // 32-bit relative address from byte distance 4 from reloc
#define IMAGE_REL_AMD64_REL32_5         0x0009  // 32-bit relative address from byte distance 5 from reloc
#define IMAGE_REL_AMD64_SECTION         0x000A  // Section index
#define IMAGE_REL_AMD64_SECREL          0x000B  // 32 bit offset from base of section containing target
#define IMAGE_REL_AMD64_SECREL7         0x000C  // 7 bit unsigned offset from base of section containing target
#define IMAGE_REL_AMD64_TOKEN           0x000D  // 32 bit metadata token
#define IMAGE_REL_AMD64_SREL32          0x000E  // 32 bit signed span-dependent value emitted into object
#define IMAGE_REL_AMD64_PAIR            0x000F
#define IMAGE_REL_AMD64_SSPAN32         0x0010  // 32 bit signed span-dependent value applied at link time

//
// IA64 relocation types.
//
#define IMAGE_REL_IA64_ABSOLUTE         0x0000
#define IMAGE_REL_IA64_IMM14            0x0001
#define IMAGE_REL_IA64_IMM22            0x0002
#define IMAGE_REL_IA64_IMM64            0x0003
#define IMAGE_REL_IA64_DIR32            0x0004
#define IMAGE_REL_IA64_DIR64            0x0005
#define IMAGE_REL_IA64_PCREL21B         0x0006
#define IMAGE_REL_IA64_PCREL21M         0x0007
#define IMAGE_REL_IA64_PCREL21F         0x0008
#define IMAGE_REL_IA64_GPREL22          0x0009
#define IMAGE_REL_IA64_LTOFF22          0x000A
#define IMAGE_REL_IA64_SECTION          0x000B
#define IMAGE_REL_IA64_SECREL22         0x000C
#define IMAGE_REL_IA64_SECREL64I        0x000D
#define IMAGE_REL_IA64_SECREL32         0x000E
// 
#define IMAGE_REL_IA64_DIR32NB          0x0010
#define IMAGE_REL_IA64_SREL14           0x0011
#define IMAGE_REL_IA64_SREL22           0x0012
#define IMAGE_REL_IA64_SREL32           0x0013
#define IMAGE_REL_IA64_UREL32           0x0014
#define IMAGE_REL_IA64_PCREL60X         0x0015  // This is always a BRL and never converted
#define IMAGE_REL_IA64_PCREL60B         0x0016  // If possible, convert to MBB bundle with NOP.B in slot 1
#define IMAGE_REL_IA64_PCREL60F         0x0017  // If possible, convert to MFB bundle with NOP.F in slot 1
#define IMAGE_REL_IA64_PCREL60I         0x0018  // If possible, convert to MIB bundle with NOP.I in slot 1
#define IMAGE_REL_IA64_PCREL60M         0x0019  // If possible, convert to MMB bundle with NOP.M in slot 1
#define IMAGE_REL_IA64_IMMGPREL64       0x001A
#define IMAGE_REL_IA64_TOKEN            0x001B  // clr token
#define IMAGE_REL_IA64_GPREL32          0x001C
#define IMAGE_REL_IA64_ADDEND           0x001F

//
// CEF relocation types.
//
#define IMAGE_REL_CEF_ABSOLUTE          0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_CEF_ADDR32            0x0001  // 32-bit address (VA).
#define IMAGE_REL_CEF_ADDR64            0x0002  // 64-bit address (VA).
#define IMAGE_REL_CEF_ADDR32NB          0x0003  // 32-bit address w/o image base (RVA).
#define IMAGE_REL_CEF_SECTION           0x0004  // Section index
#define IMAGE_REL_CEF_SECREL            0x0005  // 32 bit offset from base of section containing target
#define IMAGE_REL_CEF_TOKEN             0x0006  // 32 bit metadata token

//
// clr relocation types.
//
#define IMAGE_REL_CEE_ABSOLUTE          0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_CEE_ADDR32            0x0001  // 32-bit address (VA).
#define IMAGE_REL_CEE_ADDR64            0x0002  // 64-bit address (VA).
#define IMAGE_REL_CEE_ADDR32NB          0x0003  // 32-bit address w/o image base (RVA).
#define IMAGE_REL_CEE_SECTION           0x0004  // Section index
#define IMAGE_REL_CEE_SECREL            0x0005  // 32 bit offset from base of section containing target
#define IMAGE_REL_CEE_TOKEN             0x0006  // 32 bit metadata token


#define IMAGE_REL_M32R_ABSOLUTE       0x0000   // No relocation required
#define IMAGE_REL_M32R_ADDR32         0x0001   // 32 bit address
#define IMAGE_REL_M32R_ADDR32NB       0x0002   // 32 bit address w/o image base
#define IMAGE_REL_M32R_ADDR24         0x0003   // 24 bit address
#define IMAGE_REL_M32R_GPREL16        0x0004   // GP relative addressing
#define IMAGE_REL_M32R_PCREL24        0x0005   // 24 bit offset << 2 & sign ext.
#define IMAGE_REL_M32R_PCREL16        0x0006   // 16 bit offset << 2 & sign ext.
#define IMAGE_REL_M32R_PCREL8         0x0007   // 8 bit offset << 2 & sign ext.
#define IMAGE_REL_M32R_REFHALF        0x0008   // 16 MSBs
#define IMAGE_REL_M32R_REFHI          0x0009   // 16 MSBs; adj for LSB sign ext.
#define IMAGE_REL_M32R_REFLO          0x000A   // 16 LSBs
#define IMAGE_REL_M32R_PAIR           0x000B   // Link HI and LO
#define IMAGE_REL_M32R_SECTION        0x000C   // Section table index
#define IMAGE_REL_M32R_SECREL32       0x000D   // 32 bit section relative reference
#define IMAGE_REL_M32R_TOKEN          0x000E   // clr token


#define EXT_IMM64(Value, Address, Size, InstPos, ValPos)  *** Intel-IA64-Filler ****           \
    Value |= (((ULONGLONG)((*(Address) >> InstPos) & (((ULONGLONG)1 << Size) - 1))) << ValPos)  // Intel-IA64-Filler

#define INS_IMM64(Value, Address, Size, InstPos, ValPos)  **** Intel-IA64-Filler ****\
    *(PDWORD)Address = (*(PDWORD)Address & ~(((1 << Size) - 1) << InstPos)) | **** Intel-IA64-Filler ****\
          ((DWORD)((((ULONGLONG)Value >> ValPos) & (((ULONGLONG)1 << Size) - 1))) << InstPos)  // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM7B_INST_WORD_X         3  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM7B_SIZE_X              7  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X     4  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM7B_VAL_POS_X           0  // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM9D_INST_WORD_X         3  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM9D_SIZE_X              9  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X     18 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM9D_VAL_POS_X           7  // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM5C_INST_WORD_X         3  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM5C_SIZE_X              5  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X     13 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM5C_VAL_POS_X           16 // Intel-IA64-Filler

#define EMARCH_ENC_I17_IC_INST_WORD_X            3  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IC_SIZE_X                 1  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IC_INST_WORD_POS_X        12 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IC_VAL_POS_X              21 // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM41a_INST_WORD_X        1  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41a_SIZE_X             10 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X    14 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41a_VAL_POS_X          22 // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM41b_INST_WORD_X        1  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41b_SIZE_X             8  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X    24 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41b_VAL_POS_X          32 // Intel-IA64-Filler

#define EMARCH_ENC_I17_IMM41c_INST_WORD_X        2  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41c_SIZE_X             23 // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X    0  // Intel-IA64-Filler
#define EMARCH_ENC_I17_IMM41c_VAL_POS_X          40 // Intel-IA64-Filler

#define EMARCH_ENC_I17_SIGN_INST_WORD_X          3  // Intel-IA64-Filler
#define EMARCH_ENC_I17_SIGN_SIZE_X               1  // Intel-IA64-Filler
#define EMARCH_ENC_I17_SIGN_INST_WORD_POS_X      27 // Intel-IA64-Filler
#define EMARCH_ENC_I17_SIGN_VAL_POS_X            63 // Intel-IA64-Filler

#define X3_OPCODE_INST_WORD_X                    3  // Intel-IA64-Filler
#define X3_OPCODE_SIZE_X                         4  // Intel-IA64-Filler
#define X3_OPCODE_INST_WORD_POS_X                28 // Intel-IA64-Filler
#define X3_OPCODE_SIGN_VAL_POS_X                 0  // Intel-IA64-Filler

#define X3_I_INST_WORD_X                         3  // Intel-IA64-Filler
#define X3_I_SIZE_X                              1  // Intel-IA64-Filler
#define X3_I_INST_WORD_POS_X                     27 // Intel-IA64-Filler
#define X3_I_SIGN_VAL_POS_X                      59 // Intel-IA64-Filler

#define X3_D_WH_INST_WORD_X                      3  // Intel-IA64-Filler
#define X3_D_WH_SIZE_X                           3  // Intel-IA64-Filler
#define X3_D_WH_INST_WORD_POS_X                  24 // Intel-IA64-Filler
#define X3_D_WH_SIGN_VAL_POS_X                   0  // Intel-IA64-Filler

#define X3_IMM20_INST_WORD_X                     3  // Intel-IA64-Filler
#define X3_IMM20_SIZE_X                          20 // Intel-IA64-Filler
#define X3_IMM20_INST_WORD_POS_X                 4  // Intel-IA64-Filler
#define X3_IMM20_SIGN_VAL_POS_X                  0  // Intel-IA64-Filler

#define X3_IMM39_1_INST_WORD_X                   2  // Intel-IA64-Filler
#define X3_IMM39_1_SIZE_X                        23 // Intel-IA64-Filler
#define X3_IMM39_1_INST_WORD_POS_X               0  // Intel-IA64-Filler
#define X3_IMM39_1_SIGN_VAL_POS_X                36 // Intel-IA64-Filler

#define X3_IMM39_2_INST_WORD_X                   1  // Intel-IA64-Filler
#define X3_IMM39_2_SIZE_X                        16 // Intel-IA64-Filler
#define X3_IMM39_2_INST_WORD_POS_X               16 // Intel-IA64-Filler
#define X3_IMM39_2_SIGN_VAL_POS_X                20 // Intel-IA64-Filler

#define X3_P_INST_WORD_X                         3  // Intel-IA64-Filler
#define X3_P_SIZE_X                              4  // Intel-IA64-Filler
#define X3_P_INST_WORD_POS_X                     0  // Intel-IA64-Filler
#define X3_P_SIGN_VAL_POS_X                      0  // Intel-IA64-Filler

#define X3_TMPLT_INST_WORD_X                     0  // Intel-IA64-Filler
#define X3_TMPLT_SIZE_X                          4  // Intel-IA64-Filler
#define X3_TMPLT_INST_WORD_POS_X                 0  // Intel-IA64-Filler
#define X3_TMPLT_SIGN_VAL_POS_X                  0  // Intel-IA64-Filler

#define X3_BTYPE_QP_INST_WORD_X                  2  // Intel-IA64-Filler
#define X3_BTYPE_QP_SIZE_X                       9  // Intel-IA64-Filler
#define X3_BTYPE_QP_INST_WORD_POS_X              23 // Intel-IA64-Filler
#define X3_BTYPE_QP_INST_VAL_POS_X               0  // Intel-IA64-Filler

#define X3_EMPTY_INST_WORD_X                     1  // Intel-IA64-Filler
#define X3_EMPTY_SIZE_X                          2  // Intel-IA64-Filler
#define X3_EMPTY_INST_WORD_POS_X                 14 // Intel-IA64-Filler
#define X3_EMPTY_INST_VAL_POS_X                  0  // Intel-IA64-Filler

//
// Line number format.
//

typedef struct _IMAGE_LINENUMBER {
    union {
        DWORD   SymbolTableIndex;               // Symbol table index of function name if Linenumber is 0.
        DWORD   VirtualAddress;                 // Virtual address of line number.
    } Type;
    WORD    Linenumber;                         // Line number.
} IMAGE_LINENUMBER;
typedef IMAGE_LINENUMBER UNALIGNED *PIMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

//
// Based relocation format.
//

typedef struct _IMAGE_BASE_RELOCATION {
    DWORD   VirtualAddress;
    DWORD   SizeOfBlock;
//  WORD    TypeOffset[1];
} IMAGE_BASE_RELOCATION;
typedef IMAGE_BASE_RELOCATION UNALIGNED * PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

//
// Based relocation types.
//

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_MIPS_JMPADDR16        9
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10


//
// Archive format.
//

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       "//              "

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    BYTE     Name[16];                          // File member name - `/' terminated.
    BYTE     Date[12];                          // File member date - decimal.
    BYTE     UserID[6];                         // File member user id - decimal.
    BYTE     GroupID[6];                        // File member group id - decimal.
    BYTE     Mode[8];                           // File member mode - octal.
    BYTE     Size[10];                          // File member size - decimal.
    BYTE     EndHeader[2];                      // String to end header.
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

//
// DLL support.
//

//
// Export Format
//

typedef struct _IMAGE_EXPORT_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    DWORD   AddressOfFunctions;     // RVA from base of image
    DWORD   AddressOfNames;         // RVA from base of image
    DWORD   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

//
// Import Format
//

typedef struct _IMAGE_IMPORT_BY_NAME {
    WORD    Hint;
    BYTE    Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

#include "pshpack8.h"                       // Use align 8 for the 64-bit IAT.

typedef struct _IMAGE_THUNK_DATA64 {
    union {
        ULONGLONG ForwarderString;  // PBYTE 
        ULONGLONG Function;         // PDWORD
        ULONGLONG Ordinal;
        ULONGLONG AddressOfData;    // PIMAGE_IMPORT_BY_NAME
    } u1;
} IMAGE_THUNK_DATA64;
typedef IMAGE_THUNK_DATA64 * PIMAGE_THUNK_DATA64;

#include "poppack.h"                        // Back to 4 byte packing

typedef struct _IMAGE_THUNK_DATA32 {
    union {
        DWORD ForwarderString;      // PBYTE 
        DWORD Function;             // PDWORD
        DWORD Ordinal;
        DWORD AddressOfData;        // PIMAGE_IMPORT_BY_NAME
    } u1;
} IMAGE_THUNK_DATA32;
typedef IMAGE_THUNK_DATA32 * PIMAGE_THUNK_DATA32;

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)

//
// Thread Local Storage
//

typedef VOID
(NTAPI *PIMAGE_TLS_CALLBACK) (
    PVOID DllHandle,
    DWORD Reason,
    PVOID Reserved
    );

typedef struct _IMAGE_TLS_DIRECTORY64 {
    ULONGLONG   StartAddressOfRawData;
    ULONGLONG   EndAddressOfRawData;
    ULONGLONG   AddressOfIndex;         // PDWORD
    ULONGLONG   AddressOfCallBacks;     // PIMAGE_TLS_CALLBACK *;
    DWORD   SizeOfZeroFill;
    DWORD   Characteristics;
} IMAGE_TLS_DIRECTORY64;
typedef IMAGE_TLS_DIRECTORY64 * PIMAGE_TLS_DIRECTORY64;

typedef struct _IMAGE_TLS_DIRECTORY32 {
    DWORD   StartAddressOfRawData;
    DWORD   EndAddressOfRawData;
    DWORD   AddressOfIndex;             // PDWORD
    DWORD   AddressOfCallBacks;         // PIMAGE_TLS_CALLBACK *
    DWORD   SizeOfZeroFill;
    DWORD   Characteristics;
} IMAGE_TLS_DIRECTORY32;
typedef IMAGE_TLS_DIRECTORY32 * PIMAGE_TLS_DIRECTORY32;

#ifdef _WIN64
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG64
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL64(Ordinal)
typedef IMAGE_THUNK_DATA64              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA64             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL64(Ordinal)
typedef IMAGE_TLS_DIRECTORY64           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY64          PIMAGE_TLS_DIRECTORY;
#else
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG32
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL32(Ordinal)
typedef IMAGE_THUNK_DATA32              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA32             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL32(Ordinal)
typedef IMAGE_TLS_DIRECTORY32           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY32          PIMAGE_TLS_DIRECTORY;
#endif

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    union {
        DWORD   Characteristics;            // 0 for terminating null import descriptor
        DWORD   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
    };
    DWORD   TimeDateStamp;                  // 0 if not bound,
                                            // -1 if bound, and real date\time stamp
                                            //     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
                                            // O.W. date/time stamp of DLL bound to (Old BIND)

    DWORD   ForwarderChain;                 // -1 if no forwarders
    DWORD   Name;
    DWORD   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;

//
// New format import descriptors pointed to by DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ]
//

typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    NumberOfModuleForwarderRefs;
// Array of zero or more IMAGE_BOUND_FORWARDER_REF follows
} IMAGE_BOUND_IMPORT_DESCRIPTOR,  *PIMAGE_BOUND_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BOUND_FORWARDER_REF {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    Reserved;
} IMAGE_BOUND_FORWARDER_REF, *PIMAGE_BOUND_FORWARDER_REF;

//
// Resource Format.
//

//
// Resource directory consists of two counts, following by a variable length
// array of directory entries.  The first count is the number of entries at
// beginning of the array that have actual names associated with each entry.
// The entries are in ascending order, case insensitive strings.  The second
// count is the number of entries that immediately follow the named entries.
// This second count identifies the number of entries that have 16-bit integer
// Ids as their name.  These entries are also sorted in ascending order.
//
// This structure allows fast lookup by either name or number, but for any
// given resource entry only one form of lookup is supported, not both.
// This is consistant with the syntax of the .RC file and the .RES file.
//

typedef struct _IMAGE_RESOURCE_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    WORD    NumberOfNamedEntries;
    WORD    NumberOfIdEntries;
//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

#define IMAGE_RESOURCE_NAME_IS_STRING        0x80000000
#define IMAGE_RESOURCE_DATA_IS_DIRECTORY     0x80000000
//
// Each directory contains the 32-bit Name of the entry and an offset,
// relative to the beginning of the resource directory of the data associated
// with this directory entry.  If the name of the entry is an actual text
// string instead of an integer Id, then the high order bit of the name field
// is set to one and the low order 31-bits are an offset, relative to the
// beginning of the resource directory of the string, which is of type
// IMAGE_RESOURCE_DIRECTORY_STRING.  Otherwise the high bit is clear and the
// low-order 16-bits are the integer Id that identify this resource directory
// entry. If the directory entry is yet another resource directory (i.e. a
// subdirectory), then the high order bit of the offset field will be
// set to indicate this.  Otherwise the high bit is clear and the offset
// field points to a resource data entry.
//

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            DWORD NameOffset:31;
            DWORD NameIsString:1;
        };
        DWORD   Name;
        WORD    Id;
    };
    union {
        DWORD   OffsetToData;
        struct {
            DWORD   OffsetToDirectory:31;
            DWORD   DataIsDirectory:1;
        };
    };
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

//
// For resource directory entries that have actual string names, the Name
// field of the directory entry points to an object of the following type.
// All of these string objects are stored together after the last resource
// directory entry and before the first resource data object.  This minimizes
// the impact of these variable length objects on the alignment of the fixed
// size directory entry objects.
//

typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
    WORD    Length;
    CHAR    NameString[ 1 ];
} IMAGE_RESOURCE_DIRECTORY_STRING, *PIMAGE_RESOURCE_DIRECTORY_STRING;


typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
    WORD    Length;
    WCHAR   NameString[ 1 ];
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;


//
// Each resource data entry describes a leaf node in the resource directory
// tree.  It contains an offset, relative to the beginning of the resource
// directory of the data for the resource, a size field that gives the number
// of bytes of data at that offset, a CodePage that should be used when
// decoding code point values within the resource data.  Typically for new
// applications the code page would be the unicode code page.
//

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage;
    DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

//
// Load Configuration Directory Entry
//

typedef struct {
    DWORD   Size;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   GlobalFlagsClear;
    DWORD   GlobalFlagsSet;
    DWORD   CriticalSectionDefaultTimeout;
    DWORD   DeCommitFreeBlockThreshold;
    DWORD   DeCommitTotalFreeThreshold;
    DWORD   LockPrefixTable;            // VA
    DWORD   MaximumAllocationSize;
    DWORD   VirtualMemoryThreshold;
    DWORD   ProcessHeapFlags;
    DWORD   ProcessAffinityMask;
    WORD    CSDVersion;
    WORD    Reserved1;
    DWORD   EditList;                   // VA
    DWORD   SecurityCookie;             // VA
    DWORD   SEHandlerTable;             // VA
    DWORD   SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY32, *PIMAGE_LOAD_CONFIG_DIRECTORY32;

typedef struct {
    DWORD      Size;
    DWORD      TimeDateStamp;
    WORD       MajorVersion;
    WORD       MinorVersion;
    DWORD      GlobalFlagsClear;
    DWORD      GlobalFlagsSet;
    DWORD      CriticalSectionDefaultTimeout;
    ULONGLONG  DeCommitFreeBlockThreshold;
    ULONGLONG  DeCommitTotalFreeThreshold;
    ULONGLONG  LockPrefixTable;         // VA
    ULONGLONG  MaximumAllocationSize;
    ULONGLONG  VirtualMemoryThreshold;
    ULONGLONG  ProcessAffinityMask;
    DWORD      ProcessHeapFlags;
    WORD       CSDVersion;
    WORD       Reserved1;
    ULONGLONG  EditList;                // VA
    ULONGLONG  SecurityCookie;          // VA
    ULONGLONG  SEHandlerTable;          // VA
    ULONGLONG  SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

#ifdef _WIN64
typedef IMAGE_LOAD_CONFIG_DIRECTORY64     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY64    PIMAGE_LOAD_CONFIG_DIRECTORY;
#else
typedef IMAGE_LOAD_CONFIG_DIRECTORY32     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY32    PIMAGE_LOAD_CONFIG_DIRECTORY;
#endif

//
// WIN CE Exception table format
//

//
// Function table entry format.  Function table is pointed to by the
// IMAGE_DIRECTORY_ENTRY_EXCEPTION directory entry.
//

typedef struct _IMAGE_CE_RUNTIME_FUNCTION_ENTRY {
    DWORD FuncStart;
    DWORD PrologLen : 8;
    DWORD FuncLen : 22;
    DWORD ThirtyTwoBit : 1;
    DWORD ExceptionFlag : 1;
} IMAGE_CE_RUNTIME_FUNCTION_ENTRY, * PIMAGE_CE_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY {
    ULONGLONG BeginAddress;
    ULONGLONG EndAddress;
    ULONGLONG ExceptionHandler;
    ULONGLONG HandlerData;
    ULONGLONG PrologEndAddress;
} IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD ExceptionHandler;
    DWORD HandlerData;
    DWORD PrologEndAddress;
} IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindInfoAddress;
} _IMAGE_RUNTIME_FUNCTION_ENTRY, *_PIMAGE_RUNTIME_FUNCTION_ENTRY;

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_IA64_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY;

#if defined(_AXP64_)

typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#elif defined(_ALPHA_)

typedef  IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#else

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#endif

//
// Debug Format
//

typedef struct _IMAGE_DEBUG_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Type;
    DWORD   SizeOfData;
    DWORD   AddressOfRawData;
    DWORD   PointerToRawData;
} IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

#define IMAGE_DEBUG_TYPE_UNKNOWN          0
#define IMAGE_DEBUG_TYPE_COFF             1
#define IMAGE_DEBUG_TYPE_CODEVIEW         2
#define IMAGE_DEBUG_TYPE_FPO              3
#define IMAGE_DEBUG_TYPE_MISC             4
#define IMAGE_DEBUG_TYPE_EXCEPTION        5
#define IMAGE_DEBUG_TYPE_FIXUP            6
#define IMAGE_DEBUG_TYPE_OMAP_TO_SRC      7
#define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC    8
#define IMAGE_DEBUG_TYPE_BORLAND          9
#define IMAGE_DEBUG_TYPE_RESERVED10       10
#define IMAGE_DEBUG_TYPE_CLSID            11


typedef struct _IMAGE_COFF_SYMBOLS_HEADER {
    DWORD   NumberOfSymbols;
    DWORD   LvaToFirstSymbol;
    DWORD   NumberOfLinenumbers;
    DWORD   LvaToFirstLinenumber;
    DWORD   RvaToFirstByteOfCode;
    DWORD   RvaToLastByteOfCode;
    DWORD   RvaToFirstByteOfData;
    DWORD   RvaToLastByteOfData;
} IMAGE_COFF_SYMBOLS_HEADER, *PIMAGE_COFF_SYMBOLS_HEADER;

#define FRAME_FPO       0
#define FRAME_TRAP      1
#define FRAME_TSS       2
#define FRAME_NONFPO    3

typedef struct _FPO_DATA {
    DWORD       ulOffStart;             // offset 1st byte of function code
    DWORD       cbProcSize;             // # bytes in function
    DWORD       cdwLocals;              // # bytes in locals/4
    WORD        cdwParams;              // # bytes in params/4
    WORD        cbProlog : 8;           // # bytes in prolog
    WORD        cbRegs   : 3;           // # regs saved
    WORD        fHasSEH  : 1;           // TRUE if SEH in func
    WORD        fUseBP   : 1;           // TRUE if EBP has been allocated
    WORD        reserved : 1;           // reserved for future use
    WORD        cbFrame  : 2;           // frame type
} FPO_DATA, *PFPO_DATA;
#define SIZEOF_RFPO_DATA 16


#define IMAGE_DEBUG_MISC_EXENAME    1

typedef struct _IMAGE_DEBUG_MISC {
    DWORD       DataType;               // type of misc data, see defines
    DWORD       Length;                 // total length of record, rounded to four
                                        // byte multiple.
    BOOLEAN     Unicode;                // TRUE if data is unicode string
    BYTE        Reserved[ 3 ];
    BYTE        Data[ 1 ];              // Actual data
} IMAGE_DEBUG_MISC, *PIMAGE_DEBUG_MISC;


//
// Function table extracted from MIPS/ALPHA/IA64 images.  Does not contain
// information needed only for runtime support.  Just those fields for
// each entry needed by a debugger.
//

typedef struct _IMAGE_FUNCTION_ENTRY {
    DWORD   StartingAddress;
    DWORD   EndingAddress;
    DWORD   EndOfPrologue;
} IMAGE_FUNCTION_ENTRY, *PIMAGE_FUNCTION_ENTRY;

typedef struct _IMAGE_FUNCTION_ENTRY64 {
    ULONGLONG   StartingAddress;
    ULONGLONG   EndingAddress;
    union {
        ULONGLONG   EndOfPrologue;
        ULONGLONG   UnwindInfoAddress;
    };
} IMAGE_FUNCTION_ENTRY64, *PIMAGE_FUNCTION_ENTRY64;

//
// Debugging information can be stripped from an image file and placed
// in a separate .DBG file, whose file name part is the same as the
// image file name part (e.g. symbols for CMD.EXE could be stripped
// and placed in CMD.DBG).  This is indicated by the IMAGE_FILE_DEBUG_STRIPPED
// flag in the Characteristics field of the file header.  The beginning of
// the .DBG file contains the following structure which captures certain
// information from the image file.  This allows a debug to proceed even if
// the original image file is not accessable.  This header is followed by
// zero of more IMAGE_SECTION_HEADER structures, followed by zero or more
// IMAGE_DEBUG_DIRECTORY structures.  The latter structures and those in
// the image file contain file offsets relative to the beginning of the
// .DBG file.
//
// If symbols have been stripped from an image, the IMAGE_DEBUG_MISC structure
// is left in the image file, but not mapped.  This allows a debugger to
// compute the name of the .DBG file, from the name of the image in the
// IMAGE_DEBUG_MISC structure.
//

typedef struct _IMAGE_SEPARATE_DEBUG_HEADER {
    WORD        Signature;
    WORD        Flags;
    WORD        Machine;
    WORD        Characteristics;
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       ImageBase;
    DWORD       SizeOfImage;
    DWORD       NumberOfSections;
    DWORD       ExportedNamesSize;
    DWORD       DebugDirectorySize;
    DWORD       SectionAlignment;
    DWORD       Reserved[2];
} IMAGE_SEPARATE_DEBUG_HEADER, *PIMAGE_SEPARATE_DEBUG_HEADER;

typedef struct _NON_PAGED_DEBUG_INFO {
    WORD        Signature;
    WORD        Flags;
    DWORD       Size;
    WORD        Machine;
    WORD        Characteristics;
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       SizeOfImage;
    ULONGLONG   ImageBase;
    //DebugDirectorySize
    //IMAGE_DEBUG_DIRECTORY
} NON_PAGED_DEBUG_INFO, *PNON_PAGED_DEBUG_INFO;

#ifndef _MAC
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4944
#define NON_PAGED_DEBUG_SIGNATURE      0x494E
#else
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4449  // DI
#define NON_PAGED_DEBUG_SIGNATURE      0x4E49  // NI
#endif

#define IMAGE_SEPARATE_DEBUG_FLAGS_MASK 0x8000
#define IMAGE_SEPARATE_DEBUG_MISMATCH   0x8000  // when DBG was updated, the
                                                // old checksum didn't match.

//
//  The .arch section is made up of headers, each describing an amask position/value
//  pointing to an array of IMAGE_ARCHITECTURE_ENTRY's.  Each "array" (both the header
//  and entry arrays) are terminiated by a quadword of 0xffffffffL.
//
//  NOTE: There may be quadwords of 0 sprinkled around and must be skipped.
//

typedef struct _ImageArchitectureHeader {
    unsigned int AmaskValue: 1;                 // 1 -> code section depends on mask bit
                                                // 0 -> new instruction depends on mask bit
    int :7;                                     // MBZ
    unsigned int AmaskShift: 8;                 // Amask bit in question for this fixup
    int :16;                                    // MBZ
    DWORD FirstEntryRVA;                        // RVA into .arch section to array of ARCHITECTURE_ENTRY's
} IMAGE_ARCHITECTURE_HEADER, *PIMAGE_ARCHITECTURE_HEADER;

typedef struct _ImageArchitectureEntry {
    DWORD FixupInstRVA;                         // RVA of instruction to fixup
    DWORD NewInst;                              // fixup instruction (see alphaops.h)
} IMAGE_ARCHITECTURE_ENTRY, *PIMAGE_ARCHITECTURE_ENTRY;

#include "poppack.h"                // Back to the initial value

// The following structure defines the new import object.  Note the values of the first two fields,
// which must be set as stated in order to differentiate old and new import members.
// Following this structure, the linker emits two null-terminated strings used to recreate the
// import at the time of use.  The first string is the import's name, the second is the dll's name.

#define IMPORT_OBJECT_HDR_SIG2  0xffff

typedef struct IMPORT_OBJECT_HEADER {
    WORD    Sig1;                       // Must be IMAGE_FILE_MACHINE_UNKNOWN
    WORD    Sig2;                       // Must be IMPORT_OBJECT_HDR_SIG2.
    WORD    Version;
    WORD    Machine;
    DWORD   TimeDateStamp;              // Time/date stamp
    DWORD   SizeOfData;                 // particularly useful for incremental links

    union {
        WORD    Ordinal;                // if grf & IMPORT_OBJECT_ORDINAL
        WORD    Hint;
    };

    WORD    Type : 2;                   // IMPORT_TYPE
    WORD    NameType : 3;               // IMPORT_NAME_TYPE
    WORD    Reserved : 11;              // Reserved. Must be zero.
} IMPORT_OBJECT_HEADER;

typedef enum IMPORT_OBJECT_TYPE
{
    IMPORT_OBJECT_CODE = 0,
    IMPORT_OBJECT_DATA = 1,
    IMPORT_OBJECT_CONST = 2,
} IMPORT_OBJECT_TYPE;

typedef enum IMPORT_OBJECT_NAME_TYPE
{
    IMPORT_OBJECT_ORDINAL = 0,          // Import by ordinal
    IMPORT_OBJECT_NAME = 1,             // Import name == public symbol name.
    IMPORT_OBJECT_NAME_NO_PREFIX = 2,   // Import name == public symbol name skipping leading ?, @, or optionally _.
    IMPORT_OBJECT_NAME_UNDECORATE = 3,  // Import name == public symbol name skipping leading ?, @, or optionally _
                                        // and truncating at first @
} IMPORT_OBJECT_NAME_TYPE;


#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
// COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

// Version flags for image.
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

// Maximum size of a NativeType descriptor.
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

// #defines for the MIH FLAGS
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

// V-table constants
    COR_VTABLE_32BIT                    =0x01,          // V-table slots are 32-bits in size.   
    COR_VTABLE_64BIT                    =0x02,          // V-table slots are 64-bits in size.   
    COR_VTABLE_FROM_UNMANAGED           =0x04,          // If set, transition from unmanaged.
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,          // Call most derived method described by

// EATJ constants
    IMAGE_COR_EATJ_THUNK_SIZE           =32,            // Size of a jump thunk reserved range.

// Max name lengths    
    //@todo: Change to unlimited name lengths.
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

// COM+ 2.0 header structure.
typedef struct IMAGE_COR20_HEADER
{
    // Header versioning
    DWORD                   cb;              
    WORD                    MajorRuntimeVersion;
    WORD                    MinorRuntimeVersion;
    
    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY    MetaData;        
    DWORD                   Flags;           
    DWORD                   EntryPointToken;
    
    // Binding information
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

    // Regular fixup and binding information
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

    // Precompiled image info (internal use only - set to zero)
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif // __IMAGE_COR20_HEADER_DEFINED__

//
// End Image Format
//

   ============================ */
void DumpSectionName(PBYTE pHead, INT iOff, INT iLen, INT iNum,
                     PBYTE pStgs, LPTSTR lpfn,
                     PIMAGE_SECTION_HEADER psh);

extern   VOID DumpCodeSection(PMWL pmwl, PBYTE pBegin, PBYTE pStgs,
                               PBYTE pCode, DWORD dwLen, LPTSTR lpfn );
VOID DumpObjSection(PBYTE pHead, INT iOff, INT iLen, INT iNum,
                              PBYTE pStgs, LPTSTR lpfn );

#undef     ADDHEXD2
#define  MX1LINE     16
#define  MXOUTP      (MX1LINE * 8)
#define  ISUPPER(a)  ( ( a >= 'A' ) && ( a <= 'Z' ) )
#define  ISLOWER(a)  ( ( a >= 'a' ) && ( a <= 'z' ) )
#define  ISNUMERIC(a)      ( ( a >= '0' ) && ( a <= '9' ) )
#define  ISNUM(a)    ISNUMERIC(a)

typedef struct tagFLG2STG {
   DWORD    dwFlag;
   LPTSTR   pShort;
   LPTSTR   pVStg;
   LPTSTR   pDesc;
}FLG2STG, * PFLG2STG;

typedef struct tagFLG2STGT {
   DWORD    dwFlag;
   LPTSTR   pShort;
   LPTSTR   pVStg;
   LPTSTR   pDesc;
   PFLG2STG pRel;
}FLG2STGT, * PFLG2STGT;

typedef struct tagFLG2STG1 {
   DWORD    dwFlag;
   LPTSTR   pStg;
}FLG2STG1, * PFLG2STG1;

#define  add_PA      0x00000001
#define  add_VS      0x00000002
#define  add_VA      0x00000004
#define  add_RD      0x00000008
// 0x00000010
#define  add_REL     0x00000020
#define  add_LN      0x00000040
#define  add_CH      0x00000080
#define  add_AL      0x00000100
#define  add_CHF     0x00000200
#define  add_NM      0x00000400

#define  add_ALL     (add_NM|add_PA|add_VS|add_VA|add_RD|add_REL|add_LN|add_CH|add_AL|add_CHF)

#define  ISVALIDSN(a)  ( ( a != IMAGE_SYM_UNDEFINED ) &&\
                         ( a != IMAGE_SYM_ABSOLUTE  ) &&\
                         ( a != IMAGE_SYM_DEBUG     ) )

/////////////////////////////////////////////////////////////////////////////
// CObjView
// Try to show a COFF object file
TCHAR szMch[] = "Target Machine:";
TCHAR szSec[] = "Number of Sections:";
TCHAR szTim[] = "Date/Time Stamp:";
TCHAR szPtr[] = "Pointer to symbols:";
TCHAR szNum[] = "Number of Symbols:";
TCHAR szOHS[] = "Optional Header Size:";
TCHAR szChr[] = "Characteristics:";

#define  MALLOC(a)   LocalAlloc(LPTR,a)
#define  MFREE(a)    LocalFree(a)

LIST_ENTRY  sSym2Sect = { &sSym2Sect, &sSym2Sect };

BOOL     bIsMS = TRUE;

FLG2STG1 sSectAlign1[] = {
   { IMAGE_SCN_ALIGN_1BYTES,  "1B" }, //    0x00100000  //
   { IMAGE_SCN_ALIGN_2BYTES,  "2B" }, //   0x00200000  //
   { IMAGE_SCN_ALIGN_4BYTES,  "4B" }, //    0x00300000  //
   { IMAGE_SCN_ALIGN_8BYTES,  "8B" }, //    0x00400000  //
   { IMAGE_SCN_ALIGN_16BYTES, "16B" },  //  0x00500000  // Default alignment if no others are specified.
   { IMAGE_SCN_ALIGN_32BYTES, "32B" },  //  0x00600000  //
   { IMAGE_SCN_ALIGN_64BYTES, "64B" }, //  0x00700000  //
   { IMAGE_SCN_ALIGN_128BYTES, "128B" },   //  0x00800000  //
   { IMAGE_SCN_ALIGN_256BYTES, "256B" },   //  0x00900000  //
   { IMAGE_SCN_ALIGN_512BYTES, "512B" },   //  0x00A00000  //
   { IMAGE_SCN_ALIGN_1024BYTES, "1024B" }, //   0x00B00000  //
   { IMAGE_SCN_ALIGN_2048BYTES, "2048B" }, //  0x00C00000  //
   { IMAGE_SCN_ALIGN_4096BYTES, "4096B" }, //  0x00D00000  //
   { IMAGE_SCN_ALIGN_8192BYTES, "8192B" }, // 0x00E00000  //
   { 0,                         0 }
};

// Unused                                    0x00F00000
//#define IMAGE_SCN_ALIGN_MASK                 0x00F00000
FLG2STG1 sSectChars1[] = {
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
   { IMAGE_SCN_TYPE_NO_PAD,            "NP" }, //  0x00000008  // Reserved.
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.
   { IMAGE_SCN_CNT_CODE,               "Code" },  //        0x00000020  // Section contains code.
   { IMAGE_SCN_CNT_INITIALIZED_DATA,   "IData" }, //       0x00000040  // Section contains initialized data.
   { IMAGE_SCN_CNT_UNINITIALIZED_DATA, "UData" },   //     0x00000080  // Section contains uninitialized data.
   { IMAGE_SCN_LNK_OTHER,              "Lnk" }, //          0x00000100  // Reserved.
   { IMAGE_SCN_LNK_INFO,               "LInfo" },   //           0x00000200  // Section contains comments or some other type of information.
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
   { IMAGE_SCN_LNK_REMOVE,             "LRem" }, //      0x00000800  // Section contents will not become part of image.
   { IMAGE_SCN_LNK_COMDAT,             "CD" }, //         0x00001000  // Section contents comdat.
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
   { IMAGE_SCN_NO_DEFER_SPEC_EXC,      "RSE" },  //   0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
   { IMAGE_SCN_GPREL,                  "GPREL" }, // 0x00008000  // Section content can be accessed relative to GP
   { IMAGE_SCN_MEM_FARDATA,            "MFData" }, //          0x00008000
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
   { IMAGE_SCN_MEM_PURGEABLE,          "MPurg" }, //      0x00020000
   { IMAGE_SCN_MEM_16BIT,              "M16b" },   //     0x00020000
   { IMAGE_SCN_MEM_LOCKED,             "MLock" }, //    0x00040000
   { IMAGE_SCN_MEM_PRELOAD,            "MRel" }, //  0x00080000
//                                                         0x00500000
   { IMAGE_SCN_LNK_NRELOC_OVFL,        "LNRelO" },  //   0x01000000  // Section contains extended relocations.
   { IMAGE_SCN_MEM_DISCARDABLE,        "MDisc" },  //   0x02000000  // Section can be discarded.
   { IMAGE_SCN_MEM_NOT_CACHED,         "MNC" }, //   0x04000000  // Section is not cachable.
   { IMAGE_SCN_MEM_NOT_PAGED,          "MNP" }, //    0x08000000  // Section is not pageable.
   { IMAGE_SCN_MEM_SHARED,             "MS" }, //    0x10000000  // Section is shareable.
   { IMAGE_SCN_MEM_EXECUTE,            "MEx" }, //   0x20000000  // Section is executable.
   { IMAGE_SCN_MEM_READ,               "MR" }, //        0x40000000  // Section is readable.
   { IMAGE_SCN_MEM_WRITE,              "MW" },  //      0x80000000  // Section is writeable.
   { 0,                    0 }
};

//
// Section characteristics.
FLG2STG sSectChars[] = {
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
   { IMAGE_SCN_TYPE_NO_PAD, "NO_PAD", "0x00000008", "Reserved." },
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

   { IMAGE_SCN_CNT_CODE, "CODE", "0x00000020", "Section contains code." },
   { IMAGE_SCN_CNT_INITIALIZED_DATA, "INIT_DATA", "0x00000040",
     "Section contains initialized data." },
   { IMAGE_SCN_CNT_UNINITIALIZED_DATA, "UNINIT_DATA", "0x00000080",
     "Section contains uninitialized data." },
   { IMAGE_SCN_LNK_OTHER,  "OTHER", "0x00000100", "Reserved." },
   { IMAGE_SCN_LNK_INFO,   "INFO", "0x00000200",
     "Section contains comments or some other type of information." },
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
   { IMAGE_SCN_LNK_REMOVE, "REMOVE", "0x00000800",
     "Section contents will not become part of image." },
   { IMAGE_SCN_LNK_COMDAT, "COMDAT", "0x00001000", "Section contents comdat." },
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
   { IMAGE_SCN_NO_DEFER_SPEC_EXC, "NO_EXC", "0x00004000",
     "Reset speculative exceptions handling bits in the TLB entries for this section." },
   { IMAGE_SCN_GPREL, "GPREL", "0x00008000",
     "Section content can be accessed relative to GP" },
   { IMAGE_SCN_MEM_FARDATA, "FARDATA", "0x00008000", "IMAGE_SCN_MEM_FARDATA" },
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
   { IMAGE_SCN_MEM_PURGEABLE, "PURGEABLE", "0x00020000",
     "IMAGE_SCN_MEM_PURGEABLE" },
   { IMAGE_SCN_MEM_16BIT,   "16BIT", "0x00020000", "IMAGE_SCN_MEM_16BIT" },
   { IMAGE_SCN_MEM_LOCKED,  "LOCKED", "0x00040000", "IMAGE_SCN_MEM_LOCKED" },
   { IMAGE_SCN_MEM_PRELOAD, "PRELOAD", "0x00080000", "IMAGE_SCN_MEM_PRELOAD" },
// 0x00500000

   { IMAGE_SCN_LNK_NRELOC_OVFL, "OVFL", "0x01000000",
     "Section contains extended relocations." },
   { IMAGE_SCN_MEM_DISCARDABLE, "DISCARDABLE", "0x02000000",
     "Section can be discarded." },
   { IMAGE_SCN_MEM_NOT_CACHED, "NOT_CACHED", "0x04000000",
     "Section is not cachable." },
   { IMAGE_SCN_MEM_NOT_PAGED, "NOT_PAGED", "0x08000000",
     "Section is not pageable." },
   { IMAGE_SCN_MEM_SHARED, "SHARED", "0x10000000", "Section is shareable." },
   { IMAGE_SCN_MEM_EXECUTE, "EXECUTE", "0x20000000", "Section is executable." },
   { IMAGE_SCN_MEM_READ, "READ", "0x40000000", "Section is readable." },
   { IMAGE_SCN_MEM_WRITE, "WRITE", "0x80000000", "Section is writeable." },
   { 0,                   0,       0,            0 }
};

FLG2STG sSectAlign[] = {
   { IMAGE_SCN_ALIGN_1BYTES,  "1BYTE", "0x00100000", "IMAGE_SCN_ALIGN_1BYTES" },
   { IMAGE_SCN_ALIGN_2BYTES,  "2BYTES", "0x00200000", "IMAGE_SCN_ALIGN_2BYTES" },
   { IMAGE_SCN_ALIGN_4BYTES,  "4BYTES", "0x00300000", "IMAGE_SCN_ALIGN_4BYTES" },
   { IMAGE_SCN_ALIGN_8BYTES,  "8BYTES", "0x00400000", "IMAGE_SCN_ALIGN_8BYTES" },
   { IMAGE_SCN_ALIGN_16BYTES, "16BYTES", "0x00500000",
     "Default alignment if no others are specified." },
   { IMAGE_SCN_ALIGN_32BYTES, "32BYTES", "0x00600000",
     "IMAGE_SCN_ALIGN_32BYTES" },
   { IMAGE_SCN_ALIGN_64BYTES, "64BYTES", "0x00700000",
     "IMAGE_SCN_ALIGN_64BYTES" },
   { IMAGE_SCN_ALIGN_128BYTES, "128BYTES", "0x00800000",
     "IMAGE_SCN_ALIGN_128BYTES" },
   { IMAGE_SCN_ALIGN_256BYTES, "256BYTES", "0x00900000",
     "IMAGE_SCN_ALIGN_256BYTES" },
   { IMAGE_SCN_ALIGN_512BYTES, "512BYTES", "0x00A00000",
     "IMAGE_SCN_ALIGN_512BYTES" },
   { IMAGE_SCN_ALIGN_1024BYTES, "1024BYTES", "0x00B00000",
     "IMAGE_SCN_ALIGN_1024BYTES" },
   { IMAGE_SCN_ALIGN_2048BYTES, "2048BYTES", "0x00C00000",
     "IMAGE_SCN_ALIGN_2048BYTES" },
   { IMAGE_SCN_ALIGN_4096BYTES, "4096BYTES", "0x00D00000",
     "IMAGE_SCN_ALIGN_4096BYTES" },
   { IMAGE_SCN_ALIGN_8192BYTES, "8192BYTES", "0x00E00000",
     "IMAGE_SCN_ALIGN_8192BYTES" },
// Unused                                    0x00F00000
//   { IMAGE_SCN_ALIGN_MASK, "MASK", "0x00F00000", "IMAGE_SCN_ALIGN_MASK" },
   { 0,                          0,          0,             0 }
};

//
// Communal selection types.
//
FLG2STG  sComSel[] = {
   { IMAGE_COMDAT_SELECT_NODUPLICATES, "NODUPLICATES", "1",
     "IMAGE_COMDAT_SELECT_NODUPLICATES" },
   { IMAGE_COMDAT_SELECT_ANY,          "ANY", "2", "IMAGE_COMDAT_SELECT_ANY" },
   { IMAGE_COMDAT_SELECT_SAME_SIZE,    "SAME_SIZE", "3",
     "IMAGE_COMDAT_SELECT_SAME_SIZE" },
   { IMAGE_COMDAT_SELECT_EXACT_MATCH,  "MATCH", "4",
     "IMAGE_COMDAT_SELECT_EXACT_MATCH" },
   { IMAGE_COMDAT_SELECT_ASSOCIATIVE,  "ASSOCIATIVE", "5",
     "IMAGE_COMDAT_SELECT_ASSOCIATIVE" },
   { IMAGE_COMDAT_SELECT_LARGEST,      "LARGEST", "6",
     "IMAGE_COMDAT_SELECT_LARGEST" },
   { IMAGE_COMDAT_SELECT_NEWEST,       "NEWEST", "7", "IMAGE_COMDAT_SELECT_NEWEST" },
   { 0,                                0,        0,    0 }
};

FLG2STG  sComSrch[] = {
   { IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY, "NOLIBRARY", "1",
     "IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY" },
   { IMAGE_WEAK_EXTERN_SEARCH_LIBRARY,   "LIBRARY", "2",
     "IMAGE_WEAK_EXTERN_SEARCH_LIBRARY" },
   { IMAGE_WEAK_EXTERN_SEARCH_ALIAS,     "ALIAS", "3",
     "IMAGE_WEAK_EXTERN_SEARCH_ALIAS" },
   { 0,                                0,        0,    0 }
};


// Symbol format.
//typedef struct _IMAGE_SYMBOL {
//    union {
//        BYTE    ShortName[8];
//        struct {
//            DWORD   Short;     // if 0, use LongName
//            DWORD   Long;      // offset into string table
//        } Name;
//        DWORD   LongName[2];    // PBYTE [2]
//    } N;
//    DWORD   Value;
//    SHORT   SectionNumber;
//    WORD    Type;
//    BYTE    StorageClass;
//    BYTE    NumberOfAuxSymbols;
//} IMAGE_SYMBOL;
//typedef IMAGE_SYMBOL UNALIGNED *PIMAGE_SYMBOL;
//#define IMAGE_SIZEOF_SYMBOL                  18

//5.6. COFF String Table
//Immediately following the COFF symbol table is the COFF string table. The 
//position of this table is found by taking the symbol table address in the 
//COFF header, and adding the number of symbols multiplied by the size of a 
//symbol.
//At the beginning of the COFF string table are 4 bytes containing the total 
//size (in bytes) of the rest of the string table. This size includes the size 
//field itself, so that the value in this location would be 4 if no strings 
//were present.
//Following the size are null-terminated strings pointed to by symbols in the 
//COFF symbol table.

//
// Type (fundamental) values.
//
FLG2STG  sSymType[] = {
   { IMAGE_SYM_TYPE_NULL,  "NULL", "0x0000", "no type." },
   { IMAGE_SYM_TYPE_VOID,  "VOID", "0x0001", "IMAGE_SYM_TYPE_VOID" },
   { IMAGE_SYM_TYPE_CHAR,  "CHAR", "0x0002", "type character." },
   { IMAGE_SYM_TYPE_SHORT, "SHORT", "0x0003", "type short integer." },
   { IMAGE_SYM_TYPE_INT,   "INT", "0x0004", "IMAGE_SYM_TYPE_INT" },
   { IMAGE_SYM_TYPE_LONG,  "LONG", "0x0005", "IMAGE_SYM_TYPE_LONG" },
   { IMAGE_SYM_TYPE_FLOAT, "FLOAT", "0x0006", "IMAGE_SYM_TYPE_FLOAT" },
   { IMAGE_SYM_TYPE_DOUBLE,"DOUBLE", "0x0007", "IMAGE_SYM_TYPE_DOUBLE" },
   { IMAGE_SYM_TYPE_STRUCT,"STRUCT", "0x0008", "IMAGE_SYM_TYPE_STRUCT" },
   { IMAGE_SYM_TYPE_UNION, "UNION", "0x0009", "IMAGE_SYM_TYPE_UNION" },
   { IMAGE_SYM_TYPE_ENUM,  "ENUM", "0x000A", "enumeration." },
   { IMAGE_SYM_TYPE_MOE,   "MOE", "0x000B", "member of enumeration." },
   { IMAGE_SYM_TYPE_BYTE,  "BYTE", "0x000C", "IMAGE_SYM_TYPE_BYTE" },
   { IMAGE_SYM_TYPE_WORD,  "WORD", "0x000D", "IMAGE_SYM_TYPE_WORD" },
   { IMAGE_SYM_TYPE_UINT,  "UINT", "0x000E", "IMAGE_SYM_TYPE_UINT" },
   { IMAGE_SYM_TYPE_DWORD, "DWORD", "0x000F", "IMAGE_SYM_TYPE_DWORD" },
   { IMAGE_SYM_TYPE_PCODE, "PCODE", "0x8000", "IMAGE_SYM_TYPE_PCODE" },
   { 0,                             0,           0,     0 }
};

//
// Type (derived) values.
//
FLG2STG  sSymType2[] = {
   { IMAGE_SYM_DTYPE_NULL, "NULL", "0", "no derived type." },
   { IMAGE_SYM_DTYPE_POINTER, "POINTER", "1", "pointer." },
   { IMAGE_SYM_DTYPE_FUNCTION, "FUNCTION", "2", "function." },
   { IMAGE_SYM_DTYPE_ARRAY, "ARRAY", "3", "array." },
   { 0,                             0,           0,     0 }
};

//
// Storage classes. (StorageClass)
//
FLG2STG sStorClass[] = {
   { IMAGE_SYM_CLASS_END_OF_FUNCTION, "ENDOFFUNC", "(BYTE)-1", "<eof>" },
   { IMAGE_SYM_CLASS_NULL,            "NULL", "0x0000",        "<nul>" },
   { IMAGE_SYM_CLASS_AUTOMATIC,       "AUTOMATIC", "0x0001",   "<auto>" },
   { IMAGE_SYM_CLASS_EXTERNAL,        "EXTERNAL", "0x0002",    "<ext>" },
   { IMAGE_SYM_CLASS_STATIC,          "STATIC", "0x0003",      "<stat>" },
   { IMAGE_SYM_CLASS_REGISTER,        "REGISTER", "0x0004",    "<reg>" },
   { IMAGE_SYM_CLASS_EXTERNAL_DEF,    "EXTERNAL_DEF", "0x0005","<extd>" },
   { IMAGE_SYM_CLASS_LABEL,           "LABEL", "0x0006",       "<lab>" },
   { IMAGE_SYM_CLASS_UNDEFINED_LABEL, "UNDEFINED_LABEL", "0x0007","<ulab>" },
   { IMAGE_SYM_CLASS_MEMBER_OF_STRUCT, "STRUCT", "0x0008",     "<strm>" },
   { IMAGE_SYM_CLASS_ARGUMENT,        "ARGUMENT", "0x0009",    "<arg>" },
   { IMAGE_SYM_CLASS_STRUCT_TAG,      "TAG", "0x000A",         "<tag>" },
   { IMAGE_SYM_CLASS_MEMBER_OF_UNION, "UNION", "0x000B",       "<union>" },
   { IMAGE_SYM_CLASS_UNION_TAG,       "UNION_TAG", "0x000C",   "<utag>" },
   { IMAGE_SYM_CLASS_TYPE_DEFINITION, "DEFINITION", "0x000D",  "<def>" },
   { IMAGE_SYM_CLASS_UNDEFINED_STATIC, "UNDEFINED_STATIC", "0x000E", "<ustat>" },
   { IMAGE_SYM_CLASS_ENUM_TAG,        "ENUM_TAG", "0x000F",    "<enum>" },
   { IMAGE_SYM_CLASS_MEMBER_OF_ENUM,  "MEM_OF_ENUM", "0x0010", "<enumm>" },
   { IMAGE_SYM_CLASS_REGISTER_PARAM,  "REG_PARAM", "0x0011",   "<regp>" },
   { IMAGE_SYM_CLASS_BIT_FIELD,       "BIT_FIELD", "0x0012",   "<bit>" },
   { IMAGE_SYM_CLASS_FAR_EXTERNAL,    "FAR_EXTERNAL", "0x0044","<fext>" },
   { IMAGE_SYM_CLASS_BLOCK,           "BLOCK", "0x0064",       "<block>" },
   { IMAGE_SYM_CLASS_FUNCTION,        "FUNCTION", "0x0065",    "<func>" },
   { IMAGE_SYM_CLASS_END_OF_STRUCT,   "END_STRUCT", "0x0066",  "<estr>" },
   { IMAGE_SYM_CLASS_FILE,            "FILE", "0x0067",        "<file>" },
// new
   { IMAGE_SYM_CLASS_SECTION,         "SECTION", "0x0068",     "<sect>" },
   { IMAGE_SYM_CLASS_WEAK_EXTERNAL,   "WEAK_EXTERNAL", "0x0069","<wext>" },
   { 0,                                0,               0,       0 }
};

FLG2STG  sHdrChars[] = {
   { IMAGE_FILE_RELOCS_STRIPPED, "RELOCS STRIPPED", "0x0001",
     "Relocation info stripped from file." },
   { IMAGE_FILE_EXECUTABLE_IMAGE, "EXECUTABLE IMAGE", "0x0002",
     "File is executable (i.e. no unresolved externel references)." },
   { IMAGE_FILE_LINE_NUMS_STRIPPED, "NUMS STRIPPED", "0x0004",
     "Line nunbers stripped from file." },
   { IMAGE_FILE_LOCAL_SYMS_STRIPPED, "SYMS STRIPPED", "0x0008",
     "Local symbols stripped from file." },
   { IMAGE_FILE_AGGRESIVE_WS_TRIM, "WS TRIM", "0x0010",
     "Agressively trim working set" },
   { IMAGE_FILE_LARGE_ADDRESS_AWARE, "ADDRESS AWARE", "0x0020",
     "App can handle >2gb addresses" },
   { 0x0040, "16BIT MACHINE" "0x0040", "16-bit machine (flag reserved)" },
   { IMAGE_FILE_BYTES_REVERSED_LO, "REVERSED LO", "0x0080",
     "Bytes of machine word are reversed." },
   { IMAGE_FILE_32BIT_MACHINE, "32BIT MACHINE", "0x0100", "32 bit word machine." },
   { IMAGE_FILE_DEBUG_STRIPPED, "DEBUG STRIPPED", "0x0200",
     "Debugging info stripped from file in .DBG file" },
   { IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, "REMOVABLE SWAP", "0x0400",
     "If Image is on removable media, copy and run from the swap file." },
   { IMAGE_FILE_NET_RUN_FROM_SWAP, "NET SWAP", "0x0800",
     "If Image is on Net, copy and run from the swap file." },
   { IMAGE_FILE_SYSTEM, "FILE SYSTEM", "0x1000", "System File." },
   { IMAGE_FILE_DLL, "DLL", "0x2000", "File is a DLL." },
   { IMAGE_FILE_UP_SYSTEM_ONLY, "UP SYSTEM ONLY", "0x4000",
     "File should only be run on a UP machine" },
   { IMAGE_FILE_BYTES_REVERSED_HI, "REVERSE HI", "0x8000",
     "Bytes of machine word are reversed." },
   { 0,                             0,           0,     0 }
};

//Intel 386
//The following relocation type indicators are defined for Intel386 and 
//compatible processors:
//Constant Value Description 
//IMAGE_REL_I386_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_I386_DIR16 0x0001 Not supported. 
//IMAGE_REL_I386_REL16 0x0002 Not supported. 
//IMAGE_REL_I386_DIR32 0x0006 The target's 32-bit virtual address. 
//IMAGE_REL_I386_DIR32NB 0x0007 The target's 32-bit relative virtual address. 
//IMAGE_REL_I386_SEG12 0x0009 Not supported. 
//IMAGE_REL_I386_SECTION 0x000A The 16-bit-section index of the section 
//containing the target. This is used to support debugging information. 
//IMAGE_REL_I386_SECREL 0x000B The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//relative displacement to the target. This supports the x86 relative branch 
//and call instructions. 
//IMAGE_REL_I386_REL32 0x0014 The 32-bit 
//
// I386 relocation types.
//
FLG2STG  sRelI386[] = {
   { IMAGE_REL_I386_ABSOLUTE, "ABSOLUTE", "0x0000", "Reference is absolute, no relocation is necessary" },
   { IMAGE_REL_I386_DIR16,    "DIR16   ", "0x0001", "Direct 16-bit reference to the symbols virtual address" },
   { IMAGE_REL_I386_REL16,    "REL16   ", "0x0002", "PC-relative 16-bit reference to the symbols virtual address" },
   { IMAGE_REL_I386_DIR32,    "DIR32   ", "0x0006", "Direct 32-bit reference to the symbols virtual address" },
   { IMAGE_REL_I386_DIR32NB,  "DIR32NB ", "0x0007", "Direct 32-bit reference to the symbols virtual address, base not included" },
   { IMAGE_REL_I386_SEG12,    "SEG12   ", "0x0009", "Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address" },
   { IMAGE_REL_I386_SECTION,  "SECTION ", "0x000A",
     "The 16-bit-section index of the section containing the target. This is used to support debugging information." },
   { IMAGE_REL_I386_SECREL,   "SECREL  ", "0x000B",
     "The 32-bit offset of the target from the beginning of its section. This is used to support debugging information as well as static thread local storage." },
   { IMAGE_REL_I386_REL32,    "REL32   ", "0x0014", "PC-relative 32-bit reference to the symbols virtual address" },
   { 0,                    0,        0,        0 }
};

//MIPS Processors
//The following relocation type indicators are defined for MIPS processors:
//Constant Value Description 
//IMAGE_REL_MIPS_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_MIPS_REFHALF 0x0001 The high 16 bits of the target's 32-bit virtual address. 
//IMAGE_REL_MIPS_REFWORD 0x0002 The target's 32-bit virtual address. 
//IMAGE_REL_MIPS_JMPADDR 0x0003 The low 26 bits of the target's virtual 
//address. This supports the MIPS J and JAL instructions. 
//IMAGE_REL_MIPS_REFHI 0x0004 The high 16 bits of the target's 32-bit virtual 
//address. Used for the first instruction in a two-instruction sequence that 
//loads a full address. This relocation must be immediately followed by a PAIR 
//relocations whose SymbolTableIndex contains a signed 16-bit displacement 
//which is added to the upper 16 bits taken from the location being relocated. 
//IMAGE_REL_MIPS_REFLO 0x0005 The low 16 bits of the target's virtual address. 
//IMAGE_REL_MIPS_GPREL 0x0006 16-bit signed displacement of the target relative 
//to the Global Pointer (GP) register. 
//IMAGE_REL_MIPS_LITERAL 0x0007 Same as IMAGE_REL_MIPS_GPREL. 
//IMAGE_REL_MIPS_SECTION 0x000A The 16-bit section index of the section 
//containing the target. This is used to support debugging information. 
//IMAGE_REL_MIPS_SECREL 0x000B The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//16 bits of the 32-bit offset of the target from the beginning of its section. 
//
//IMAGE_REL_MIPS_SECRELLO 0x000C The low 
//IMAGE_REL_MIPS_SECRELHI 0x000D The high 16 bits of the 32-bit offset of the 
//target from the beginning of its section. A PAIR relocation must immediately 
//follow this on. The SymbolTableIndex of the PAIR relocation contains a signed 
//16-bit displacement, which is added to the upper 16 bits taken from the 
//location being relocated. 
//IMAGE_REL_MIPS_JMPADDR16 0x0010 The low 26 bits of the target's virtual 
//address. This supports the MIPS16 JAL instruction. 
//IMAGE_REL_MIPS_REFWORDNB 0x0022 The target's 32-bit relative virtual address. 
//IMAGE_REL_MIPS_PAIR 0x0025 This relocation is only valid when it immediately 
//follows a REFHI or SECRELHI relocation. Its SymbolTableIndex contains a 
//displacement and not an index into the symbol table. 

FLG2STG  sRelMIPS[] = {
   { IMAGE_REL_MIPS_ABSOLUTE, "ABSOLUTE", "0x0000",
    "Reference is absolute, no relocation is necessary" },
   { IMAGE_REL_MIPS_REFHALF,  "REFHALF ", "0x0001", "IMAGE_REL_MIPS_REFHALF" },
   { IMAGE_REL_MIPS_REFWORD,  "REFWORD ", "0x0002", "IMAGE_REL_MIPS_REFWORD" },
   { IMAGE_REL_MIPS_JMPADDR,  "JMPADDR ", "0x0003", "IMAGE_REL_MIPS_JMPADDR" },
   { IMAGE_REL_MIPS_REFHI,    "REFHI   ", "0x0004", "IMAGE_REL_MIPS_REFHI" },
   { IMAGE_REL_MIPS_REFLO,    "REFLO   ", "0x0005", "IMAGE_REL_MIPS_REFLO" },
   { IMAGE_REL_MIPS_GPREL,    "GPREL   ", "0x0006", "IMAGE_REL_MIPS_GPREL" },
   { IMAGE_REL_MIPS_LITERAL,  "LITERAL ", "0x0007", "IMAGE_REL_MIPS_LITERAL" },
   { IMAGE_REL_MIPS_SECTION,  "SECTION ", "0x000A", "IMAGE_REL_MIPS_SECTION" },
   { IMAGE_REL_MIPS_SECREL,   "SECREL  ", "0x000B", "IMAGE_REL_MIPS_SECREL" },
   { IMAGE_REL_MIPS_SECRELLO, "SECRELLO", "0x000C",
    "Low 16-bit section relative referemce (used for >32k TLS)" },
   { IMAGE_REL_MIPS_SECRELHI, "SECRELHI", "0x000D",
    "High 16-bit section relative reference (used for >32k TLS)" },
   { IMAGE_REL_MIPS_JMPADDR16,"JMPADR16", "0x0010",
    "IMAGE_REL_MIPS_JMPADDR16" },
   { IMAGE_REL_MIPS_REFWORDNB, "REFWDNB", "0x0022",
    "IMAGE_REL_MIPS_REFWORDNB" },
   { IMAGE_REL_MIPS_PAIR,      "PAIR   ", "0x0025", "IMAGE_REL_MIPS_PAIR" },
   { 0,                     0,     0,         0 }
};

//Alpha Processors
//The following relocation Type indicators are defined for Alpha processors:
//Constant Value Description 
//IMAGE_REL_ALPHA_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_ALPHA_REFLONG 0x0001 The target's 32-bit virtual address. This 
//fixup is illegal in a PE32+ image unless the image has been sandboxed by 
//clearing the IMAGE_FILE_LARGE_ADDRESS_AWARE bit in the File Header. 
//IMAGE_REL_ALPHA_REFQUAD 0x0002 The target's 64-bit virtual address. 
//IMAGE_REL_ALPHA_GPREL32 0x0003 32-bit signed displacement of the target 
//relative to the Global Pointer (GP) register. 
//IMAGE_REL_ALPHA_LITERAL 0x0004 16-bit signed displacement of the target 
//relative to the Global Pointer (GP) register. 
//IMAGE_REL_ALPHA_LITUSE 0x0005 Reserved for future use. 
//IMAGE_REL_ALPHA_GPDISP 0x0006 Reserved for future use. 
//IMAGE_REL_ALPHA_BRADDR 0x0007 The 21-bit relative displacement to the target. 
//This supports the Alpha relative branch instructions. 
//IMAGE_REL_ALPHA_HINT 0x0008 14-bit hints to the processor for the target of 
//an Alpha jump instruction. 
//IMAGE_REL_ALPHA_INLINE_REFLONG 0x0009 The target's 32-bit virtual address 
//split into high and low 16-bit parts. Either an ABSOLUTE or MATCH relocation 
//must immediately follow this relocation. The high 16 bits of the target 
//address are stored in the location identified by the INLINE_REFLONG 
//relocation. The low 16 bits are stored four bytes later if the following 
//relocation is of type ABSOLUTE or at a signed displacement given in the 
//SymbolTableIndex if the following relocation is of type MATCH. 
//IMAGE_REL_ALPHA_REFHI 0x000A The high 16 bits of the target's 32-bit virtual 
//address. Used for the first instruction in a two-instruction sequence that 
//loads a full address. This relocation must be immediately followed by a PAIR 
//relocations whose SymbolTableIndex contains a signed 16-bit displacement 
//which is added to the upper 16 bits taken from the location being relocated. 
//IMAGE_REL_ALPHA_REFLO 0x000B The low 16 bits of the target's virtual address. 
//IMAGE_REL_ALPHA_PAIR 0x000C This relocation is only valid when it immediately 
//follows a REFHI , REFQ3, REFQ2, or SECRELHI relocation. Its SymbolTableIndex 
//contains a displacement and not an index into the symbol table. 
//IMAGE_REL_ALPHA_MATCH 0x000D This relocation is only valid when it 
//immediately follows INLINE_REFLONG relocation. Its SymbolTableIndex contains 
//the displacement in bytes of the location for the matching low address and 
//not an index into the symbol table. 
//IMAGE_REL_ALPHA_SECTION 0x000E The 16-bit section index of the section 
//containing the target.  This is used to support debugging information. 
//IMAGE_REL_ALPHA_SECREL 0x000F The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//target's 32-bit relative virtual address. 
//IMAGE_REL_ALPHA_REFLONGNB 0x0010 The 
//IMAGE_REL_ALPHA_SECRELLO 0x0011 The low 16 bits of the 32-bit offset of the 
//target from the beginning of its section. 
//IMAGE_REL_ALPHA_SECRELHI 0x0012 The high 16 bits of the 32-bit offset of the 
//target from the beginning of its section. A PAIR relocation must immediately 
//follow this on. The SymbolTableIndex of the PAIR relocation contains a signed 
//16-bit displacement which is added to the upper 16 bits taken from the 
//location being relocated. 
//IMAGE_REL_ALPHA_REFQ3 0x0013 The low 16 bits of the high 32 bits of the 
//target's 64-bit virtual address. This relocation must be immediately followed 
//by a PAIR relocations whose SymbolTableIndex contains a signed 32-bit 
//displacement which is added to the 16 bits taken from the location being 
//relocated. The 16 bits in the relocated location are shifted left by 32 
//before this addition. 
//IMAGE_REL_ALPHA_REFQ2 0x0014 The high 16 bits of the low 32 bits of the 
//target's 64-bit virtual address. This relocation must be immediately followed 
//by a PAIR relocations whose SymbolTableIndex contains a signed 16-bit 
//displacement which is added to the upper 16 bits taken from the location 
//being relocated. 
//IMAGE_REL_ALPHA_REFQ1 0x0015 The low 16 bits of the target's 64-bit virtual address. 
//IMAGE_REL_ALPHA_GPRELLO 0x0016 The low 16 bits of the 32-bit signed 
//displacement of the target relative to the Global Pointer (GP) register. 
//IMAGE_REL_ALPHA_GPRELHI 0x0017 The high 16 bits of the 32-bit signed 
//displacement of the target relative to the Global Pointer (GP) register. 
//
// Alpha Relocation types.
//
FLG2STG  sRelALPHA[] = {
   { IMAGE_REL_ALPHA_ABSOLUTE, "ABSOLUTE", "0x0000",
    "IMAGE_REL_ALPHA_ABSOLUTE" },
   { IMAGE_REL_ALPHA_REFLONG,  "REFLONG ", "0x0001", "IMAGE_REL_ALPHA_REFLONG" },
   { IMAGE_REL_ALPHA_REFQUAD,  "REFQUAD ", "0x0002", "IMAGE_REL_ALPHA_REFQUAD" },
   { IMAGE_REL_ALPHA_GPREL32,  "GPREL32 ", "0x0003", "IMAGE_REL_ALPHA_GPREL32" },
   { IMAGE_REL_ALPHA_LITERAL,  "LITERAL ", "0x0004", "IMAGE_REL_ALPHA_LITERAL" },
   { IMAGE_REL_ALPHA_LITUSE,   "LITUSE  ", "0x0005", "IMAGE_REL_ALPHA_LITUSE" },
   { IMAGE_REL_ALPHA_GPDISP,   "GPDISP  ", "0x0006", "IMAGE_REL_ALPHA_GPDISP" },
   { IMAGE_REL_ALPHA_BRADDR,   "BRADDR  ", "0x0007", "IMAGE_REL_ALPHA_BRADDR" },
   { IMAGE_REL_ALPHA_HINT,     "HINT    ", "0x0008", "IMAGE_REL_ALPHA_HINT" },
   { IMAGE_REL_ALPHA_INLINE_REFLONG, "REFLONG ", "0x0009",
    "IMAGE_REL_ALPHA_INLINE_REFLONG" },
   { IMAGE_REL_ALPHA_REFHI,    "REFHI   ", "0x000A", "IMAGE_REL_ALPHA_REFHI" },
   { IMAGE_REL_ALPHA_REFLO,    "REFLO   ", "0x000B", "IMAGE_REL_ALPHA_REFLO" },
   { IMAGE_REL_ALPHA_PAIR,     "PAIR    ", "0x000C", "IMAGE_REL_ALPHA_PAIR" },
   { IMAGE_REL_ALPHA_MATCH,    "MATCH   ", "0x000D", "IMAGE_REL_ALPHA_MATCH" },
   { IMAGE_REL_ALPHA_SECTION,  "SECTION ", "0x000E", "IMAGE_REL_ALPHA_SECTION" },
   { IMAGE_REL_ALPHA_SECREL,   "SECREL  ", "0x000F", "IMAGE_REL_ALPHA_SECREL" },
   { IMAGE_REL_ALPHA_REFLONGNB,"REFLNGNB", "0x0010",
    "IMAGE_REL_ALPHA_REFLONGNB" },
   { IMAGE_REL_ALPHA_SECRELLO, "SECRELLO", "0x0011",
    "Low 16-bit section relative reference" },
   { IMAGE_REL_ALPHA_SECRELHI, "SECRELHI", "0x0012",
    "High 16-bit section relative reference" },
   { IMAGE_REL_ALPHA_REFQ3,    "REFQ3   ", "0x0013",
    "High 16 bits of 48 bit reference" },
   { IMAGE_REL_ALPHA_REFQ2,    "REFQ2   ", "0x0014",
    "Middle 16 bits of 48 bit reference" },
   { IMAGE_REL_ALPHA_REFQ1,    "REFQ1   ", "0x0015",
    "Low 16 bits of 48 bit reference" },
   { IMAGE_REL_ALPHA_GPRELLO,  "GPRELLO ", "0x0016",
    "Low 16-bit GP relative reference" },
   { IMAGE_REL_ALPHA_GPRELHI,  "GPRELHI ", "0x0017",
    "High 16-bit GP relative reference" },
   { 0,                     0,     0,         0 }
};
   

//IBM PowerPC Processors
//The following relocation Type indicators are defined for PowerPC processors:
//Constant Value Description 
//IMAGE_REL_PPC_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_PPC_ADDR64 0x0001 The target's 64-bit virtual address. 
//IMAGE_REL_PPC_ADDR32 0x0002 The target's 32-bit virtual address. 
//IMAGE_REL_PPC_ADDR24 0x0003 The low 24 bits of the target's virtual address. 
//This is only valid when the target symbol is absolute and can be sign 
//extended to its original value. 
//IMAGE_REL_PPC_ADDR16 0x0004 The low 16 bits of the target's virtual address. 
//IMAGE_REL_PPC_ADDR14 0x0005 The low 14 bits of the target's virtual address. 
//This is only valid when the target symbol is absolute and can be sign 
//extended to its original value. 
//IMAGE_REL_PPC_REL24 0x0006 A 24-bit PC-relative offset to the symbol's location. 
//IMAGE_REL_PPC_REL14 0x0007 A 14-bit PC-relative offset to the symbol's location. 
//IMAGE_REL_PPC_ADDR32NB 0x000A The target's 32-bit relative virtual address. 
//IMAGE_REL_PPC_SECREL 0x000B The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//IMAGE_REL_PPC_SECTION 0x000C The 16-bit section index of the section 
//containing the target. This is used to support debugging information. 
//IMAGE_REL_PPC_SECREL16 0x000F The 16-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//bits of the target's 32-bit virtual address. Used for the first instruction 
//in a two-instruction sequence that loads a full address. This relocation must 
//be immediately followed by a PAIR relocations whose SymbolTableIndex contains 
//a signed 16-bit displacement which is added to the upper 16 bits taken from 
//the location being relocated. 
//IMAGE_REL_PPC_REFHI 0x0010 The high 16 
//IMAGE_REL_PPC_REFLO 0x0011 The low 16 bits of the target's virtual address. 
//IMAGE_REL_PPC_PAIR 0x0012 This relocation is only valid when it immediately 
//follows a REFHI or SECRELHI relocation. Its SymbolTableIndex contains a 
//displacement and not an index into the symbol table. 
//IMAGE_REL_PPC_SECRELLO 0x0013 The low 16 bits of the 32-bit offset of the 
//target from the beginning of its section. 
//IMAGE_REL_PPC_SECRELHI 0x0014 The high 16 bits of the 32-bit offset of the 
//target from the beginning of its section. A PAIR relocation must immediately 
//follow this on. The SymbolTableIndex of the PAIR relocation contains a signed 
//16-bit displacement which is added to the upper 16 bits taken from the 
//location being relocated. 
//IMAGE_REL_PPC_GPREL 0x0015 16-bit signed displacement of the target relative 
//to the Global Pointer (GP) register. 

//
// IBM PowerPC relocation types.
//

FLG2STG  sRelPPC[] = {
   { IMAGE_REL_PPC_ABSOLUTE, "ABSOLUTE", "0x0000", "NOP" },
   { IMAGE_REL_PPC_ADDR64,   "ADDR64  ", "0x0001", "64-bit address" },
   { IMAGE_REL_PPC_ADDR32,   "ADDR32  ", "0x0002", "32-bit address" },
   { IMAGE_REL_PPC_ADDR24,   "ADDR24  ", "0x0003",
     "26-bit address, shifted left 2 (branch absolute)" },
   { IMAGE_REL_PPC_ADDR16,   "ADDR16  ", "0x0004", "16-bit address" },
   { IMAGE_REL_PPC_ADDR14,   "ADDR14  ", "0x0005",
     "16-bit address, shifted left 2 (load doubleword)" },
   { IMAGE_REL_PPC_REL24,    "REL24   ", "0x0006",
     "26-bit PC-relative offset, shifted left 2 (branch relative)" },
   { IMAGE_REL_PPC_REL14,    "REL14   ", "0x0007",
     "16-bit PC-relative offset, shifted left 2 (br cond relative)" },
   { IMAGE_REL_PPC_TOCREL16, "TOCREL16", "0x0008",
     "16-bit offset from TOC base" },
   { IMAGE_REL_PPC_TOCREL14, "TOCREL14", "0x0009",
     "16-bit offset from TOC base, shifted left 2 (load doubleword)" },
   { IMAGE_REL_PPC_ADDR32NB, "ADDR32NB", "0x000A",
     "32-bit addr w/o image base" },
   { IMAGE_REL_PPC_SECREL,   "SECREL  ", "0x000B",
     "va of containing section (as in an image sectionhdr)" },
   { IMAGE_REL_PPC_SECTION,  "SECTION ", "0x000C", "sectionheader number" },
   { IMAGE_REL_PPC_IFGLUE,   "IFGLUE  ", "0x000D",
     "substitute TOC restore instruction iff symbol is glue code" },
   { IMAGE_REL_PPC_IMGLUE,   "IMGLUE  ", "0x000E",
     "symbol is glue code; virtual address is TOC restore instruction" },
   { IMAGE_REL_PPC_SECREL16, "SECREL16", "0x000F",
     "va of containing section (limited to 16 bits)" },
   { IMAGE_REL_PPC_REFHI,    "REFHI   ", "0x0010", "IMAGE_REL_PPC_REFHI" },
   { IMAGE_REL_PPC_REFLO,    "REFLO   ", "0x0011", "IMAGE_REL_PPC_REFLO" },
   { IMAGE_REL_PPC_PAIR,     "PAIR    ", "0x0012", "IMAGE_REL_PPC_PAIR" },
   { IMAGE_REL_PPC_SECRELLO, "SECRELLO", "0x0013",
     "Low 16-bit section relative reference (used for >32k TLS)" },
   { IMAGE_REL_PPC_SECRELHI, "SECRELHI", "0x0014",
     "High 16-bit section relative reference (used for >32k TLS)" },
   { IMAGE_REL_PPC_GPREL,    "GPREL   ", "0x0015", "IMAGE_REL_PPC_GPREL" },
   { 0,                     0,     0,         0 }
};

//#define IMAGE_REL_PPC_TYPEMASK          0x00FF  // mask to isolate above values in IMAGE_RELOCATION.Type
// Flag bits in IMAGE_RELOCATION.TYPE

FLG2STG  sRelPPCF[] = {
   { IMAGE_REL_PPC_NEG,     "NEG", "0x0100",
     "subtract reloc value rather than adding it" },
   { IMAGE_REL_PPC_BRTAKEN, "BRTAKEN", "0x0200",
     "fix branch prediction bit to predict branch taken" },
   { IMAGE_REL_PPC_BRNTAKEN,"BRNTAKEN", "0x0400",
     "fix branch prediction bit to predict branch not taken" },
   { IMAGE_REL_PPC_TOCDEFN, "TOCDEFN", "0x0800",
     "toc slot defined in file (or, data in toc)" },
   { 0,                     0,     0,         0 }
};


//Hitachi SuperH Processors
//The following relocation type indicators are defined for SH3 and SH4 processors:
//Constant Value Description 
//IMAGE_REL_SH3_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_SH3_DIRECT16 0x0001 Reference to the 16-bit location that contains 
//the virtual address of the target symbol. 
//IMAGE_REL_SH3_DIRECT32 0x0002 The target's 32-bit virtual address. 
//IMAGE_REL_SH3_DIRECT8 0x0003 Reference to the 8-bit location that contains 
//the virtual address of the target symbol. 
//IMAGE_REL_SH3_DIRECT8_WORD 0x0004 Reference to the 8-bit instruction that 
//contains the effective 16-bit virtual address of the target symbol. 
//IMAGE_REL_SH3_DIRECT8_LONG 0x0005 Reference to the 8-bit instruction that 
//contains the effective 32-bit virtual address of the target symbol. 
//IMAGE_REL_SH3_DIRECT4 0x0006 Reference to the 8-bit location whose low 4 bits 
//contain the virtual address of the target symbol. 
//IMAGE_REL_SH3_DIRECT4_WORD 0x0007 Reference to the 8-bit instruction whose 
//low 4 bits contain the effective 16-bit virtual address of the target symbol. 
//
//IMAGE_REL_SH3_DIRECT4_LONG 0x0008 Reference to the 8-bit instruction whose 
//low 4 bits contain the effective 32-bit virtual address of the target symbol. 
//
//IMAGE_REL_SH3_PCREL8_WORD 0x0009 Reference to the 8-bit instruction which 
//contains the effective 16-bit relative offset of the target symbol. 
//IMAGE_REL_SH3_PCREL8_LONG 0x000A Reference to the 8-bit instruction which 
//contains the effective 32-bit relative offset of the target symbol. 
//IMAGE_REL_SH3_PCREL12_WORD 0x000B Reference to the 16-bit instruction whose 
//low 12 bits contain the effective 16-bit relative offset of the target 
//symbol. 
//IMAGE_REL_SH3_STARTOF_SECTION 0x000C Reference to a 32-bit location that is 
//the virtual address of the symbol's section. 
//IMAGE_REL_SH3_SIZEOF_SECTION 0x000D Reference to the 32-bit location that is 
//the size of the symbol's section. 
//IMAGE_REL_SH3_SECTION 0x000E The 16-bit section index of the section 
//containing the target. This is used to support debugging information. 
//IMAGE_REL_SH3_SECREL 0x000F The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
//IMAGE_REL_SH3_DIRECT32_NB 0x0010 The target's 32-bit relative virtual address. 

//
// Hitachi SH3 relocation types.
//
FLG2STG  sRelSH3[] = {
   { IMAGE_REL_SH3_ABSOLUTE, "ABSOLUTE", "0x0000", "No relocation" },
   { IMAGE_REL_SH3_DIRECT16, "DIRECT16", "0x0001", "16 bit direct" },
   { IMAGE_REL_SH3_DIRECT32, "DIRECT32", "0x0002", "32 bit direct" },
   { IMAGE_REL_SH3_DIRECT8,  "DIRECT8 ", "0x0003", "8 bit direct, -128..255" },
   { IMAGE_REL_SH3_DIRECT8_WORD, "WORD8   ", "0x0004", "8 bit direct .W (0 ext.)" },
   { IMAGE_REL_SH3_DIRECT8_LONG, "LONG8   ", "0x0005", "8 bit direct .L (0 ext.)" },
   { IMAGE_REL_SH3_DIRECT4,  "DIRECT4 ", "0x0006", "4 bit direct (0 ext.)" },
   { IMAGE_REL_SH3_DIRECT4_WORD, "WORD5  ", "0x0007", "4 bit direct .W (0 ext.)" },
   { IMAGE_REL_SH3_DIRECT4_LONG, "LONG4  ", "0x0008", "4 bit direct .L (0 ext.)" },
   { IMAGE_REL_SH3_PCREL8_WORD, "RWORD8  ", "0x0009", "8 bit PC relative .W" },
   { IMAGE_REL_SH3_PCREL8_LONG, "RLONG8  ", "0x000A", "8 bit PC relative .L" },
   { IMAGE_REL_SH3_PCREL12_WORD,"WORD12  ", "0x000B", "12 LSB PC relative .W" },
   { IMAGE_REL_SH3_STARTOF_SECTION, "SSECTION", "0x000C",
     "Start of EXE section" },
   { IMAGE_REL_SH3_SIZEOF_SECTION, "ZSECTION", "0x000D", "Size of EXE section" },
   { IMAGE_REL_SH3_SECTION,   "SECTION ", "0x000E", "Section table index" },
   { IMAGE_REL_SH3_SECREL,    "SECREL  ", "0x000F", "Offset within section" },
   { IMAGE_REL_SH3_DIRECT32_NB, "NB      ", "0x0010", "32 bit direct not based" },
   { 0,                     0,     0,         0 }
};

//
// IA64 relocation types.
//

FLG2STG  sRelIA64[] = {
   { IMAGE_REL_IA64_ABSOLUTE, "ABSOLUTE", "0x0000", "IMAGE_REL_IA64_ABSOLUTE" },
   { IMAGE_REL_IA64_IMM14,    "IMM14   ", "0x0001", "IMAGE_REL_IA64_IMM14" },
   { IMAGE_REL_IA64_IMM22,    "IMM22   ", "0x0002", "IMAGE_REL_IA64_IMM22" },
   { IMAGE_REL_IA64_IMM64,    "IMM64   ", "0x0003", "IMAGE_REL_IA64_IMM64" },
   { IMAGE_REL_IA64_DIR32,    "DIR32   ", "0x0004", "IMAGE_REL_IA64_DIR32" },
   { IMAGE_REL_IA64_DIR64,    "DIR64   ", "0x0005", "IMAGE_REL_IA64_DIR64" },
   { IMAGE_REL_IA64_PCREL21B, "PCREL21B", "0x0006", "IMAGE_REL_IA64_PCREL21B" },
   { IMAGE_REL_IA64_PCREL21M, "PCREL21M", "0x0007", "IMAGE_REL_IA64_PCREL21M" },
   { IMAGE_REL_IA64_PCREL21F, "PCREL21F", "0x0008", "IMAGE_REL_IA64_PCREL21F" },
   { IMAGE_REL_IA64_GPREL22,  "GPREL22 ", "0x0009", "IMAGE_REL_IA64_GPREL22" },
   { IMAGE_REL_IA64_LTOFF22,  "LTOFF22 ", "0x000A", "IMAGE_REL_IA64_LTOFF22" },
   { IMAGE_REL_IA64_SECTION,  "SECTION ", "0x000B", "IMAGE_REL_IA64_SECTION" },
   { IMAGE_REL_IA64_SECREL22, "SECREL22", "0x000C", "IMAGE_REL_IA64_SECREL22" },
   { IMAGE_REL_IA64_SECREL64I,"SCREL64I", "0x000D", "IMAGE_REL_IA64_SECREL64I" },
   { IMAGE_REL_IA64_SECREL32, "SECREL32", "0x000E", "IMAGE_REL_IA64_SECREL32" },
//   { IMAGE_REL_IA64_LTOFF64,  "LTOFF64 ", "0x000F", "IMAGE_REL_IA64_LTOFF64" },
   { 0x00ff,                  "LTOFF64 ", "0x000F", "IMAGE_REL_IA64_LTOFF64" },
   { IMAGE_REL_IA64_DIR32NB,  "DIR32NB ", "0x0010", "IMAGE_REL_IA64_DIR32NB" },
   { IMAGE_REL_IA64_SREL22,   "SREL22  ", "0x0011", "IMAGE_REL_IA64_SREL22" },
// { IMAGE_REL_IA64_UREL22,   "UREL22  ", "0x0012", "IMAGE_REL_IA64_UREL22" },
   { 0x0012,                  "UREL22  ", "0x0012", "IMAGE_REL_IA64_UREL22" },
   { IMAGE_REL_IA64_SREL32,   "SREL32  ", "0x0013", "IMAGE_REL_IA64_SREL32" },
   { IMAGE_REL_IA64_UREL32,   "UREL32  ", "0x0014", "IMAGE_REL_IA64_UREL32" },
   { IMAGE_REL_IA64_ADDEND,   "ADDEND  ", "0x001F", "IMAGE_REL_IA64_ADDEND" },
   { 0,                     0,     0,         0 }
};

//ARM Processors
//The following relocation Type indicators are defined for ARM processors:
//Constant Value Description 
//IMAGE_REL_ARM_ABSOLUTE 0x0000 This relocation is ignored. 
//IMAGE_REL_ARM_ADDR32 0x0001 The target's 32-bit virtual address. 
//IMAGE_REL_ARM_ADDR32NB 0x0002 The target's 32-bit relative virtual address. 
//IMAGE_REL_ARM_BRANCH24 0x0003 The 24-bit relative displacement to the target.  
//IMAGE_REL_ARM_BRANCH11 0x0004 Reference to a subroutine call, consisting of 
//two 16-bit instructions with 11-bit offsets. 
//IMAGE_REL_ARM_SECTION 0x000E The 16-bit section index of the section 
//containing the target. This is used to support debugging information. 
//IMAGE_REL_ARM_SECREL 0x000F The 32-bit offset of the target from the 
//beginning of its section. This is used to support debugging information as 
//well as static thread local storage. 
FLG2STG  sRelARM[] = {
   { IMAGE_REL_ARM_ABSOLUTE, "ABSOLUTE", "0x0000", "No relocation required" },
   { IMAGE_REL_ARM_ADDR32,   "ADDR32  ", "0x0001", "32 bit address" },
   { IMAGE_REL_ARM_ADDR32NB, "ADDR32NB", "0x0002",
     "32 bit address w/o image base" },
   { IMAGE_REL_ARM_BRANCH24, "BRANCH24", "0x0003",
     "24 bit offset << 2 & sign ext." },
   { IMAGE_REL_ARM_BRANCH11, "BRANCH11", "0x0004", "Thumb: 2 11 bit offsets" },
   { IMAGE_REL_ARM_GPREL12,  "GPREL12 ", "0x0006",
     "GP-relative addressing (ARM)" },
   { IMAGE_REL_ARM_GPREL7,   "GPREL7  ", "0x0007",
     "GP-relative addressing (Thumb)" },
   { IMAGE_REL_ARM_SECTION,  "SECTION", "0x000E", "Section table index" },
   { IMAGE_REL_ARM_SECREL,   "SECREL ", "0x000F", "Offset within section" },
   { 0,                     0,     0,         0 }
};

//
// CEF relocation types.
//

FLG2STG  sRelCEF[] = {
   { IMAGE_REL_CEF_ABSOLUTE, "ABSOLUTE", "0x0000",
     "Reference is absolute, no relocation is necessary" },
   { IMAGE_REL_CEF_ADDR32, "ADDR32", "0x0001", "32-bit address (VA)." },
   { IMAGE_REL_CEF_ADDR64, "ADDR64", "0x0002", "64-bit address (VA)." },
   { IMAGE_REL_CEF_ADDR32NB, "ADDR32NB", "0x0003",
     "32-bit address w/o image base (RVA)." },
   { IMAGE_REL_CEF_SECTION, "SECTION", "0x0004", "Section index" },
   { IMAGE_REL_CEF_SECREL, "SECREL", "0x0005",
     "32 bit offset from base of section containing target" },
   { IMAGE_REL_CEF_TOKEN, "TOKEN", "0x0006", "32 bit metadata token" },
   { 0,                     0,     0,         0 }
};


FLG2STGT  sMachRel[] = {
   { IMAGE_FILE_MACHINE_UNKNOWN, "UNKNOWN", "0x0", "Unknown", 0 }, 
   { IMAGE_FILE_MACHINE_I386,    "I386", "0x014c", "Intel 386", sRelI386 },
   { IMAGE_FILE_MACHINE_R3000,   "R3000", "0x0162", "MIPS little-endian", sRelMIPS },
   { 0x160,                      "R30BE", "0x0160", "MIPS big-endian", sRelMIPS },
   { IMAGE_FILE_MACHINE_R4000,   "R4000", "0x0166", "MIPS little-endian", sRelMIPS },
   { IMAGE_FILE_MACHINE_R10000,  "R10000", "0x0168", "MIPS little-endian", sRelMIPS },
   { IMAGE_FILE_MACHINE_WCEMIPSV2, "WCEv2", "0x0169", "MIPS little-endian WCE v2", sRelMIPS },
   { IMAGE_FILE_MACHINE_ALPHA,   "ALPHA", "0x0184", "Alpha_AXP", sRelALPHA },
   { IMAGE_FILE_MACHINE_POWERPC, "POWERPC", "0x01F0", "IBM PowerPC Little-Endian", sRelPPC },
   { IMAGE_FILE_MACHINE_SH3,     "SH3", "0x01a2", "SH3 little-endian", sRelSH3 },
   { IMAGE_FILE_MACHINE_SH3E,    "SH3E", "0x01a4", "SH3E little-endian", sRelSH3 },
   { IMAGE_FILE_MACHINE_SH4,     "SH4",  "0x01a6", "SH4 little-endian", sRelSH3 },
   { IMAGE_FILE_MACHINE_ARM,     "ARM", "0x01c0", "ARM Little-Endian", sRelARM },
   { IMAGE_FILE_MACHINE_THUMB,   "THUMB", "0x01c2", "Thumb", sRelARM },
   { IMAGE_FILE_MACHINE_IA64,    "IA64", "0x0200", "Intel 64", sRelIA64 },
   { IMAGE_FILE_MACHINE_MIPS16,  "MIPS16", "0x0266", "MIPS", sRelMIPS },
   { IMAGE_FILE_MACHINE_MIPSFPU, "MIPSFPU", "0x0366", "MIPS", sRelMIPS },
   { IMAGE_FILE_MACHINE_MIPSFPU16, "MIPSFPU16", "0x0466", "MIPS", sRelMIPS },
   { IMAGE_FILE_MACHINE_ALPHA64, "ALPHA64", "0x0284", "ALPHA64", sRelALPHA },
   { IMAGE_FILE_MACHINE_AXP64,   "AXP64", "0x???", "AP64", sRelALPHA },
   { IMAGE_FILE_MACHINE_CEF,     "CEF", "0xC0EF", "CEF", sRelCEF },
   { 0,                       0,     0,       0, 0 }
};

// can also be ANON
//typedef struct ANON_OBJECT_HEADER {
//    WORD    Sig1;            // Must be IMAGE_FILE_MACHINE_UNKNOWN
//    WORD    Sig2;            // Must be 0xffff
//    WORD    Version;         // >= 1 (implies the CLSID field is present)
//    WORD    Machine;
//    DWORD   TimeDateStamp;
//    CLSID   ClassID;         // Used to invoke CoCreateInstance
//    DWORD   SizeOfData;      // Size of data that follows the header
//} ANON_OBJECT_HEADER;

char * Get_CLSID_Stg( CLSID * pid )
{
   static char _s_clsid[64];
   char * cp = _s_clsid;
   int i;
   sprintf(cp, "{%08X-%04X-%04X-%02X%02X-",
      pid->Data1, // LONG
      (pid->Data2 & 0xffff), // short
      (pid->Data3 & 0xffff), // short
      (pid->Data4[0] & 0xff),
      (pid->Data4[1] & 0xff) );

   for(i = 2; i < 8; i++) {
      sprintf(EndBuf(cp), "%02X", (pid->Data4[i] & 0xff) );
   }
   strcat(cp,"}");
   return cp;
}

int Is_ANON_OBJECT_HEADER( void * pv )
{
   ANON_OBJECT_HEADER * panon = (ANON_OBJECT_HEADER *)pv;
   if(( panon->Sig1 == IMAGE_FILE_MACHINE_UNKNOWN ) &&
      ( panon->Sig2 == 0xffff ) ) {
         // char * cp = Get_CLSID_Stg( &panon->ClassID );
         return 1;
   }
   return 0;
}

//typedef struct tagUNKBLOCK {
//   char ln1[16];
//   char ln2[4];
//}UNKBLOCK, * PUNKBLOCK;

// test if it is an OBJ file
BOOL IsObjectFile( LPTSTR lpf, BYTE *lpImage, INT iFileSize )
{
   static TCHAR _s_szBuf[1024];
   PIMAGE_FILE_HEADER poh = (PIMAGE_FILE_HEADER)lpImage;
   ANON_OBJECT_HEADER * panon = (ANON_OBJECT_HEADER *)lpImage;
   //POBJHDR  poh = (POBJHDR)lpImage;
   BOOL        flg = FALSE;
   //PFILETIME   pft = &poh->oh_DT;
   PFILETIME   pft = (PFILETIME)&poh->TimeDateStamp; // seconds since midnight on January 1, 1970
   SYSTEMTIME  st;
   FILETIME    ft;
   PVOID       pv, pv2;
   DWORD       dws, dwi, dwc;
   PIMAGE_SECTION_HEADER   psh;
   PBYTE       pb;
   LPTSTR      lpb = _s_szBuf;
   TCHAR       buf[64];
   INT         i;
   int         isanon =
      (iFileSize > sizeof(ANON_OBJECT_HEADER)) ? Is_ANON_OBJECT_HEADER(lpImage) : 0;

//   pv = poh->oh_SymPtr;
   if( VERB9 ) {
      ProcessHex( lpImage,
         (isanon ? sizeof(ANON_OBJECT_HEADER) : sizeof(IMAGE_FILE_HEADER) ) );
   }
   if( isanon ) {
      time_t tt = (time_t)panon->TimeDateStamp;
      // like time() runtime function, so
      char * ctm = ctime(&tt);
      i = sprintf(lpb, "Object UNIX time and date:%s", ctm );
      if(VERB9) {
         while(i--) {
            if( lpb[i] > ' ' )
               break;
            lpb[i] = 0;
         }
         AddStringandAdjust(lpb);
      }
      dws = panon->SizeOfData;
      if(( iFileSize > sizeof(ANON_OBJECT_HEADER ) )&&
         ( FileTimeToLocalFileTime( (const FILETIME *)&tt, &ft ) ) && // UTC file time converted
         ( FileTimeToSystemTime( &ft, &st)       ) &&
         (  dws                                  ) &&
         ( dws < (DWORD)iFileSize                ) ) {
         PIMAGE_FILE_HEADER pifh = (PIMAGE_FILE_HEADER)((ANON_OBJECT_HEADER *)panon + 1);
         pv2 = (PVOID)((BYTE *) lpImage + iFileSize);
         dwc = pifh->NumberOfSections;
         // looking good - hop through the data checking
         psh = (PIMAGE_SECTION_HEADER)((PIMAGE_FILE_HEADER)pifh + 1);
         for( dwi = 0; dwi < dwc ; dwi++ )
         {
            sprintf(lpb, "SECTION HEADER #%#X", (dwi + 1));
            //AddStringandAdjust(lpb);
            //ProcessHex( (PBYTE) psh, sizeof(IMAGE_SECTION_HEADER) );
            pb = (PBYTE)&psh->Name;
            for( i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++ )
            {
               if( pb[i] )
                  buf[i] = pb[i];
               else
                  break;
            }
            buf[i] = 0;
            if( i == 0 )
               break;
            *lpb = 0;
            while( i < 8 )
            {
               strcat(lpb, " ");
               i++;
            }
            strcat(lpb,buf);
            strcat(lpb, " ");
            strcat(lpb, "name");
            pb = (PBYTE)psh->PointerToRawData;
            dws = psh->SizeOfRawData;
            if(pb && dws)
            {
               pb += dws;
               if( (DWORD)pb > (DWORD)pv2 )
               {
                  break;
               }
            }
            psh++;
         }
         if( dwi < dwc ) {
            // FAILED to get through
            if(VERB5) {
               AddStringandAdjust( "Failed Object file test on Sections count!" );
            }
            return FALSE;
         }
         return TRUE;
      } else {
         if(VERB5) {
            AddStringandAdjust( "Failed Object file test on size or internal dates!" );
         }
      }
      return FALSE;
   }
   pv = (PVOID)poh->PointerToSymbolTable;
   dws = (DWORD)poh->NumberOfSections;
   sprintf(lpb, "Checking Object: Size = %u, No of Sec. = %d, Offset = %p",
      iFileSize, dws, pv );
   if(VERB5) {
      AddStringandAdjust(lpb);
   }
   if( ( iFileSize > sizeof(IMAGE_FILE_HEADER) ) &&
       ( FileTimeToLocalFileTime( pft, &ft )   ) && // UTC file time converted
       ( FileTimeToSystemTime( &ft, &st)       ) &&
       ( dws ) )
   {
      sprintf(lpb,
         "File [%s] has stamp %02d/%02d/%02d %02d:%02d",
         GetShortName(lpf,40),
         (st.wDay & 0xffff),
         (st.wMonth & 0xffff),
         (st.wYear % 100),
         (st.wHour & 0xffff),
         (st.wMinute & 0xffff) );
      if( VERB5 ) {
         AddStringandAdjust(lpb);
      }
      *lpb = 0;
      if( pv )
      {
         pv2 = (PVOID)((BYTE *) lpImage + iFileSize);
         if( (DWORD)pv < (DWORD)pv2 )
         {
            // looking good - hop through the data checking
            psh = (PIMAGE_SECTION_HEADER)((PIMAGE_FILE_HEADER)poh + 1);

            for( dwi = 0; dwi < dws; dwi++ )
            {
               sprintf(lpb, "SECTION HEADER #%#X", (dwi + 1));

               pb = (PBYTE)&psh->Name;
               for( i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++ )
               {
                  if( pb[i] )
                     buf[i] = pb[i];
                  else
                     break;
               }
               buf[i] = 0;
               if( i == 0 )
                  break;
               *lpb = 0;
               while( i < 8 )
               {
                  strcat(lpb, " ");
                  i++;
               }
               strcat(lpb,buf);
               strcat(lpb, " ");
               strcat(lpb, "name");

               pb = (PBYTE)psh->PointerToRawData;
               dwc = psh->SizeOfRawData;
               if(pb)
               {
                  pb += dwc;
                  if( (DWORD)pb > (DWORD)pv2 )
                  {
                     break;
                  }
               }
               psh++;
            }

            if( dwi == dws )
            {
               flg = TRUE;
            }
         } else {
            sprintf(lpb, "Failed Object test due to poh->PointerToSymbolTable GT FileSize!" );
            if(VERB5) {
               AddStringandAdjust(lpb);
            }
         }
      }
   } else {
      if(VERB5) {
         AddStringandAdjust( "Failed Object file test on size or internal dates!" );
      }
   }
   return flg;
}


#define  it_FunBgn   1
#define  it_FunLines 2
#define  it_FunEnd   3
#define  it_Source   4
#define  it_WeakExt  5
#define  it_SecDef   6


#define  ISEXTERN(a) (\
   ( a == IMAGE_SYM_CLASS_EXTERNAL ) ||\
   ( a == IMAGE_SYM_CLASS_EXTERNAL_DEF ) ||\
   ( a == IMAGE_SYM_CLASS_WEAK_EXTERNAL ) ||\
   ( a == IMAGE_SYM_CLASS_FAR_EXTERNAL ) )

VOID AppendFStgEq(LPTSTR lpd, PFLG2STG pf2s, DWORD dwf, BOOL bShort )
{
   BOOL  bFnd = FALSE;
   while( pf2s->pShort )
   {
      if( pf2s->dwFlag == dwf )
      {
         if( bShort )
            strcat(lpd, pf2s->pShort);
         else
            strcat(lpd, pf2s->pDesc);
         bFnd = TRUE;
         break;
      }
      pf2s++;
   }

   if( !bFnd )
      sprintf(EndBuf(lpd), "(V?)%#x", dwf );
}

VOID AppendFStgEq1(LPTSTR lpd, PFLG2STG1 pf2s1, DWORD dwf )
{
   BOOL  bFnd = FALSE;
   while( pf2s1->pStg )
   {
      if( pf2s1->dwFlag == dwf )
      {
         strcat(lpd, pf2s1->pStg);
         bFnd = TRUE;
         break;
      }
      pf2s1++;
   }

   if( !bFnd ) {
      if( dwf )
         sprintf(EndBuf(lpd), "(V?)%#x", dwf );
      else
         strcat(lpd, "None");
   }

}

INT AppendFStgsOr(LPTSTR lpd, PFLG2STG pf2s, DWORD dwf)
{
   INT   i = 0;
   while( dwf && pf2s->pShort )
   {
      if( pf2s->dwFlag & dwf )
      {
         if(i)
            strcat(lpd,"|");
         strcat(lpd, pf2s->pShort);
         dwf &= ~(pf2s->dwFlag);
         i += strlen(pf2s->pShort);
      }
      pf2s++;
   }

   if( dwf )
   {
      if(i)
         strcat(lpd,"|");
      sprintf(EndBuf(lpd), "(R?)%#x", dwf );
   }

   return i;

}

INT AppendFStgsOr1(LPTSTR lpd, PFLG2STG1 pf2s1, DWORD dwf)
{
   INT   i = 0;
   while( dwf && pf2s1->pStg )
   {
      if( pf2s1->dwFlag & dwf )
      {
         if(i)
            strcat(lpd,"|");
         strcat(lpd, pf2s1->pStg);
         dwf &= ~(pf2s1->dwFlag);
         i += strlen(pf2s1->pStg);
      }
      pf2s1++;
   }

   if( dwf )
   {
      if(i)
         strcat(lpd,"|");
      sprintf(EndBuf(lpd), "(R?)%#x", dwf );
   }

   return i;

}

#define  MXLONGNAME     256
#define  MXSHORTNAME    8

INT GetSymName( LPTSTR lpb, PIMAGE_SYMBOL pSym, PBYTE pStgs, BOOL bAll )
{
   DWORD    dwo;
   PDWORD   pdw;
   INT      i    = 0;   // no length
   PBYTE    pb   = 0;   // no buffer pointer
   INT      imax = MXSHORTNAME;  // maximum of 8

   if( pSym == 0 ) {
      i = sprintf( lpb, "NONAME" );
      return i;
   }

   dwo = pSym->N.Name.Short;
   if( dwo == 0 )
   {
      dwo = pSym->N.Name.Long;
      // offset into String Table
      if(pStgs)
      {
         pdw = (PDWORD)pStgs;
         if( (dwo > 3) && ( dwo < *pdw ) )
         {
            pb = pStgs + dwo;
            if( *pb )
            {
               // maybe ok
               imax = MXLONGNAME;   // but set like a 256 byte (arbitrary) MAXIMUM length
            }
            else
            {
               sprintf(lpb, "(W?)StgOff=0x%x??", dwo );
               pb = 0;
            }
         }
         else
         {
            sprintf(lpb, "(W?)StgOff=%#x?", dwo );
         }
      }
      else
      {
            strcpy(lpb, "(W?)pStgs=NULL?");
      }
   }
   else
   {
      // BYTE    ShortName[8]; note imax defaults to MXSHORTNAME = 8
       pb = (PBYTE) &pSym->N.ShortName[0];   // simple pointer to SHORT name
   }

   if(pb)
   {
      INT   j;
      while( *pb )
      {
         j = *pb;
         if( isprint( j ) )
         {
            lpb[i++] = *pb;
         }
         else if( bAll )
         {
            if( j & 0x80 )
            {
               lpb[i++] = '@';
               j &= ~(0x80);
            }
            lpb[i++] = '^';
            lpb[i++] = (TCHAR)(j + 0x40);   // bump into ASCII (+'@')
         }
         else
         {
            break;
         }

         if( i >= imax )
            break;

         pb++;
            
      }
      lpb[i] = 0;
   }
   return i;

}

#define  DBGRELNM

#ifdef   ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

PIMAGE_RELOCATION GetRelName( LPTSTR lpb, PBYTE pRel, DWORD dwLen, PBYTE pStgs,
                                       DWORD dwo, DWORD dwFlag )
{
   PIMAGE_RELOCATION bRet = 0;
   DWORD             dwva;
   if( pRel && dwLen && m_pSymTbl && m_dwSymCnt )
   {
      DWORD             dwn;
      PIMAGE_RELOCATION pIRel = (PIMAGE_RELOCATION)pRel;
      *lpb = 0;
      for( dwn = 0; dwn < dwLen; dwn++ )
      {
         dwva = pIRel->VirtualAddress;
         if( dwva == dwo )
         {
            dwn = pIRel->SymbolTableIndex;
            if( dwn < m_dwSymCnt )
            {
               GetSymName( lpb, (PIMAGE_SYMBOL)((LPTSTR)m_pSymTbl + dwn), pStgs, FALSE );
               if( *lpb )
               {
                  bRet = pIRel;
               }
               else
               {
                  sprtf( "WARNING: Failed to get symbol string for %d."MEOR, dwo );
               }
            }
            else
            {
               sprtf( "WARNING: SybolTableIndex %d greater than count %d."MEOR,
                  dwn, m_dwSymCnt );
            }
            break;
         }
#ifdef  GETSLOPSYM
         if( dwFlag )
         {
            DWORD dwx;
            dwx = 1;
            for( dwn = 0; dwn < dwFlag; dwn++ )
            {
               if( dwva == (dwo + dwx) )
               {
                  sprtf( "WARNING: Obtaining symbol for %d at plus %d."MEOR, dwo, dwx );
                  dwn = pIRel->SymbolTableIndex;
                  if( dwn < m_dwSymCnt )
                  {
                     GetSymName( lpb, (m_pSymTbl + dwn), pStgs, FALSE );
                     if( *lpb )
                        bRet = pIRel;
                     else
                        sprtf( "WARNING: Failed to get symbol string for %d."MEOR, dwo );
                     break;
                  }
                  else
                  {
                     sprtf( "WARNING: SybolTableIndex %d greater than count %d."MEOR,
                        dwn, m_dwSymCnt );
                  }
                  //break;
               }
               else if( dwva == (dwo - dwx) )
               {
                  sprtf( "WARNING: Obtaining symbol for %d at minus %d."MEOR, dwo, dwx );
                  dwn = pIRel->SymbolTableIndex;
                  if( dwn < m_dwSymCnt )
                  {
                     GetSymName( lpb, (m_pSymTbl + dwn), pStgs, FALSE );
                     if( *lpb )
                        bRet = pIRel;
                     else
                        sprtf( "WARNING: Failed to get symbol string for %d."MEOR, dwo );
                     break;
                  }
                  else
                  {
                     sprtf( "WARNING: SybolTableIndex %d greater than count %d."MEOR,
                        dwn, m_dwSymCnt );
                  }
                  //break;
               }
               dwx++;
            }
         }

#ifdef   DBGRELNM
         if(( dwFlag == 0 ) &&
            ( ( pIRel->VirtualAddress == (dwo + 1) ) ||
              ( pIRel->VirtualAddress == (dwo - 1) ) ))
         {
            DWORD dw1 = (dwo + 1);
            if( pIRel->VirtualAddress == (dwo - 1) )
               dw1 = (dwo - 1);
            dwn = pIRel->SymbolTableIndex;
            if( dwn < m_dwSymCnt )
            {
               GetSymName( lpb, (m_pSymTbl + dwn), pStgs, FALSE );
               if( *lpb )
               {
                  chkme( "HEY! Appears wrong value %x passed to GetRelName should be %x!"MEOR
                     "Missed name [%s]!"MEOR,
                     dwo,
                     dw1,
                     lpb );
               }
               else
                  goto No_Name;
            }
            else
            {
No_Name:
               chkme( "HEY! Appears wrong value %x passed to GetRelName should be %x!"MEOR
                  "But no name anyway!!!"MEOR, dwo, dw1 );
            }
         }
#endif   // DBGRELNM
#else // #ifdef  GETSLOPSYM
#ifdef   DBGRELNM
         if(( ( pIRel->VirtualAddress == (dwo + 1) ) ||
              ( pIRel->VirtualAddress == (dwo - 1) ) ))
         {
            DWORD dw1 = (dwo + 1);
            if( pIRel->VirtualAddress == (dwo - 1) )
               dw1 = (dwo - 1);
            dwn = pIRel->SymbolTableIndex;
            if( dwn < m_dwSymCnt )
            {
               GetSymName( lpb, (PIMAGE_SYMBOL)((LPTSTR)m_pSymTbl + dwn), pStgs, FALSE );
               if( *lpb )
               {
                  sprtf( "WARNING: Using symbol at %d instead of %d"MEOR, dw1, dwo );

//                  chkme( "HEY! Appears wrong value %x passed to GetRelName should be %x!"MEOR
//                     "Missed name [%s]!"MEOR,
//                     dwo,
//                     dw1,
//                     lpb );
                  bRet = pIRel;
                  break;
               }
               else
                  goto No_Name;
            }
            else
            {
No_Name:
               sprtf( "WARNING: HEY! Appears wrong value %x passed to GetRelName should be %x!"MEOR
                  "But no name anyway!!!"MEOR, dwo, dw1 );
            }
         }
#endif   // DBGRELNM
#endif   // #ifdef  GETSLOPSYM y/n

         pIRel++; // bump to NEXT relocation item
      }
   }
   return bRet;
}

#else // !#ifdef   ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

PIMAGE_RELOCATION GetRelName( LPTSTR lpb, PBYTE pRel, DWORD dwLen, PBYTE pStgs, DWORD dwo )
{
   PIMAGE_RELOCATION  bRet = 0;
   if( pRel && dwLen && g_pSymTbl && g_dwSymCnt )
   {
      DWORD             dwn;
      PIMAGE_RELOCATION pIRel = (PIMAGE_RELOCATION)pRel;
      *lpb = 0;
      for( dwn = 0; dwn < dwLen; dwn++ )
      {
         if( pIRel->VirtualAddress == dwo )
         {
            dwn = pIRel->SymbolTableIndex;
            if( dwn < g_dwSymCnt )
            {
               GetSymName( lpb, (PIMAGE_SYMBOL)((PIMAGE_SYMBOL)g_pSymTbl + dwn), pStgs, FALSE );
               if( *lpb )
               {
                  bRet = pIRel;
               }
            }
            break;
         }
#ifdef   DBGRELNM
         if( ( pIRel->VirtualAddress == (dwo + 1) ) ||
            ( pIRel->VirtualAddress == (dwo - 1) ) )
         {
            if( pIRel->VirtualAddress == (dwo + 1) )
               chkme( "HEY! Appears wrong value passed to GetRelName %x vs %x!", dwo, (dwo+1) );
            else
               chkme( "HEY! Appears wrong value passed to GetRelName %x vs %x!", dwo, (dwo-1) );
         }
#endif   // DBGRELNM
         pIRel++; // bump to NEXT relocation item
      }
   }
   return bRet;
}

#endif   // #ifdef   ADDOBJSW y/n   // FIX20010731 - add -obj to dump as a COFF object file

INT GetSectionName(LPTSTR lpb, PIMAGE_SECTION_HEADER psh, PBYTE pStgs)
{
   LPTSTR   lps;
   INT      i, j;
   DWORD    dwo;

   i = 0;
   lps = (LPTSTR)&psh->Name[0];
   if( pStgs && ( lps[0] == '/' ) ) //&&
   {
      // ( (lps[1] >= '0') && (lps[1] <= '9') ) )
      // if contains a slash (/)
      // followed by ASCII representation of a decimal number:
      // this number is an offset into the string table
      dwo = 0;
      for( i = 1; i < (IMAGE_SIZEOF_SHORT_NAME - 1); i++ )
      {
         j = lps[i];
         if( ISNUMERIC(j) )
            dwo = (dwo * 10) + (j - '0');
         else
            break;
      }
      lps = (LPTSTR) (pStgs + dwo);
      if( *lps )
      {
         strcpy(lpb, lps);
         i = strlen(lpb);
         lps = 0;
      }
      else
         lps = (LPTSTR)&psh->Name[0];
   }

   if(lps)
   {
      for( i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++ )
      {
         if( lps[i] )
            lpb[i] = lps[i];
         else
            break;
      }
   }

   lpb[i] = 0;

   return i;

}


INT GetObjSection(LPTSTR lpb, PIMAGE_SECTION_HEADER psh, PBYTE pStgs,
                            DWORD dwf )
{
   DWORD    dwo;
   INT      i, j;

   *lpb = 0;
   if( dwf & add_NM )
   {
      GetSectionName( lpb, psh, pStgs );
      strcat(lpb, " ");
   }

   if( dwf & add_PA )
   {
      dwo = psh->Misc.PhysicalAddress; 
      if(dwo)
      {
         strcat(lpb, "PA=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }

   if( dwf & add_VS )
   {
      dwo = psh->Misc.VirtualSize; 
      if(dwo)
      {
         strcat(lpb, "VS=");
         sprintf(EndBuf(lpb), "%d ", dwo );
      }
   }

   if( dwf & add_VA )
   {
      dwo = psh->VirtualAddress;
      if(dwo)
      {
         strcat(lpb, "VA=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }

   if( dwf & add_RD )
   {
      dwo = psh->SizeOfRawData;
      if(dwo)
      {
         strcat(lpb, "RD=");
         sprintf(EndBuf(lpb), "%d ", dwo );
      }
      dwo = psh->PointerToRawData;
      if(dwo)
      {
         strcat(lpb, "PRD=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }

   if( dwf & add_REL )
   {
      dwo = psh->NumberOfRelocations;
      if(dwo)
      {
         strcat(lpb, "RC=");
         sprintf(EndBuf(lpb), "%d ", dwo );
      }
   
      dwo = psh->PointerToRelocations;
      if(dwo)
      {
         strcat(lpb, "PR=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }

   if( dwf & add_LN )
   {
      dwo = psh->NumberOfLinenumbers;
      if(dwo)
      {
         strcat(lpb, "LNC=");
         sprintf(EndBuf(lpb), "%d ", dwo );
      }
   
      dwo = psh->PointerToLinenumbers;
      if(dwo)
      {
         strcat(lpb, "PLN=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }

   if( dwf & add_CH )
   {
      dwo = psh->Characteristics;
      if(dwo)
      {
         strcat(lpb, "CH=");
         sprintf(EndBuf(lpb), "%#x ", dwo );
      }
   }


   if( dwf & add_AL )
   {
      strcat(lpb, "A=");
      dwo = (psh->Characteristics & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
      if(dwo)
      {
         PFLG2STG pf2s = &sSectAlign[0];
         while( dwo && pf2s->pShort )
         {
            if( dwo == pf2s->dwFlag )
            {
               strcat(lpb, pf2s->pShort);
               dwo = 0;
               break;
            }
            pf2s++;
         }
         if( dwo )
         {
            sprintf(EndBuf(lpb), "=%#x?", dwo);
         }
         strcat(lpb, " ");
      }
      else
      {
         strcat(lpb, "16BYTES(def) ");
      }
   }

   if( dwf & add_CHF )
   {

      dwo = (psh->Characteristics & ~(IMAGE_SCN_ALIGN_MASK) );   //  0x00F00000
      if(dwo)
      {
         PFLG2STG pf2s = &sSectChars[0];
         strcat(lpb, "C=");
         j = 0;
         while( dwo && pf2s->pShort )
         {
            if( dwo & pf2s->dwFlag )
            {
               if(j)
                  strcat(lpb,"|");
               strcat(lpb, pf2s->pShort);
               j += strlen(pf2s->pShort);
               dwo &= ~(pf2s->dwFlag);
            }
            pf2s++;
         }
         if( dwo )
         {
            sprintf(EndBuf(lpb), "%#x", dwo );
         }
         strcat(lpb, " ");
      }
   }
   
   i = strlen(lpb);
   while( i )
   {
      i--;
      if( lpb[i] <= ' ' )
      {
         lpb[i] = 0;
      }
      else
      {
         i++;
         break;
      }
   }

   return i;
}

VOID DeleteWorkList(VOID)
{
   KillLList(&sSym2Sect);
}


VOID DumpAuxEntry(PIMAGE_SYMBOL pis, INT iIsFunc, INT iType,
                            DWORD dwstcls, DWORD dwc, PIMAGE_SECTION_HEADER psh2,
                            DWORD dwFlag, DWORD  dwSCnt, PIMAGE_SECTION_HEADER psh )
{
   PFLG2STG                pf2s2;
   DWORD                   dwo;
   PBYTE                   pb;
   PIMAGE_SECTION_HEADER   psh3;
   LPTSTR                  lpd = &g_cBuf[0];
   PIMAGE_AUX_SYMBOL       pas = (PIMAGE_AUX_SYMBOL)pis;

   //sprintf(lpd, "Symbol %3d Aux:", dwc );
   //SetMinLen(lpd,MINHDSZ);
   sprintf(lpd, "%3d Aux:", dwc );
   SetMinLen(lpd,MINSHDSZ);
   if( iIsFunc == 3 )
   {
      // 5.5.1. Auxiliary Format 1: Function Definitions
      //A symbol table record marks the beginning of a function definition if all of 
      //the following are true: it has storage class EXTERNAL (2), a Type value 
      //indicating it is a function (0x20), and a section number greater than zero. 
      sprintf(EndBuf(lpd),
         "Function: TId=%d",
         pas->Sym.TagIndex );
      sprintf(EndBuf(lpd),
         " Sze=%d",
         pas->Sym.Misc.TotalSize );
      sprintf(EndBuf(lpd),
         " Lne=%#x",
         pas->Sym.FcnAry.Function.PointerToLinenumber );
      sprintf(EndBuf(lpd),
         " Nxt=%#x",
         pas->Sym.FcnAry.Function.PointerToNextFunction );
   }
   else if( (iType == it_FunBgn) ||
      (iType == it_FunLines) ||
      (iType == it_FunEnd) )
   {
      //else if( (strcmp(lpn, ".bf") == 0) ||
      //(strcmp(lpn, ".ef") == 0) )
      // 5.5.2. Auxiliary Format 2: .bf and .ef Symbols
      if( (iType == it_FunBgn) ||
         (iType == it_FunEnd) )
      {
         sprintf(EndBuf(lpd),
            "Line #%d",
            pas->Sym.Misc.LnSz.Linenumber );
         if(iType == it_FunBgn)  // strcmp(lpn, ".bf") == 0)
            sprintf(EndBuf(lpd), " nf=0x%x", pas->Sym.FcnAry.Function.PointerToNextFunction );
      }
      else
      {
         strcat(lpd, " Function Lines");
      }
   }
   else if( iType == it_WeakExt )
   {
      // 5.5.3. Auxiliary Format 3: Weak Externals
      sprintf(EndBuf(lpd), " SymInd=%d ", pas->Sym.TagIndex );
      dwo = pas->Sym.Misc.TotalSize;   // characteristics
      pf2s2 = &sComSrch[0];
      AppendFStgEq(lpd, pf2s2, dwo, TRUE);
   }
   else if( iType == it_Source )
   {
      // 5.5.4. Auxiliary Format 4: Files
      if( dwstcls == IMAGE_SYM_CLASS_FILE )  // 0x067 (103)
         strcat(lpd, "File=");
      else
         strcat(lpd, "F???=");
      pb = (PBYTE)pas;
      strcat(lpd, (LPTSTR)pb);
   }
   else if( iType == it_SecDef )
   {
      if( dwstcls == IMAGE_SYM_CLASS_STATIC ) // 3
      {
         sprintf(EndBuf(lpd),
            "Len %d",
            pas->Section.Length );
         sprintf(EndBuf(lpd),
            " #Rel %d",
            pas->Section.NumberOfRelocations );
         //sprintf(EndBuf(lpd), " CS 0x%x", pas->Section.CheckSum );
         if( psh2 && ( dwFlag & IMAGE_SCN_LNK_COMDAT ) )
         {
            //AddStringandAdjust(lpd);
            //*lpd = 0;
            //SetMinLen(lpd,MINSHDSZ);
            dwo = pas->Section.Selection;
            if(dwo)
            {
               strcat(lpd, "Sel=");
               pf2s2 = &sComSel[0];
               AppendFStgEq(lpd, pf2s2, dwo, TRUE);
            }
            if( dwo == 5 ) // IMAGE_COMDAT_SELECT_ASSOCIATIVE
            {
               dwo = pas->Section.Number;
               if( (dwo > 0) && (dwo <= dwSCnt) )
               {
                  psh3 = psh + (dwo - 1);
                  sprintf(EndBuf(lpd), " COMDAT Ind=%d", (dwo - 1));
               }
               else
               {
                  sprintf(EndBuf(lpd), " WARNING: Secl #%d INVALID! (mx=%d)",
                     dwo, dwSCnt );
               }
            }
         }
      }
      else
      {
         strcat(lpd, "WARNING: SecDef Not STATIC??");
      }
   }
   else
   {
      //strcat(lpd, "Is auxillary entry only");
      strcat(lpd, "Not defined" );  // "No auxillary format defined?");
   }

   AddStringandAdjust(lpd);

}

VOID  AppendSectionFlag( PTSTR ptmp, DWORD dwFlag )
{
   DWORD dwalign = (dwFlag & IMAGE_SCN_ALIGN_MASK);
   AppendFStgsOr1(ptmp, &sSectChars1[0], (dwFlag & ~(IMAGE_SCN_ALIGN_MASK) ));
   strcat(ptmp, "|"); // add separator
   //AppendFStgEq(ptmp, &sSectAlign[0], (dwFlag & IMAGE_SCN_ALIGN_MASK), TRUE);
   if( dwalign ) {
      AppendFStgEq1(ptmp, &sSectAlign1[0], dwalign );
   } else {
      strcat(ptmp, "Align16(def)");
   }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : CObjView::DumpObjSymbols
// Return type: VOID 
// Arguments  : PIMAGE_SYMBOL pis
//            : DWORD dwi
//            : PIMAGE_SECTION_HEADER psh
//            : DWORD dwSCnt
//            : PBYTE pStg
// Description: 
//5.4.1. Symbol Name Representation
//The Name field in a symbol table consists of eight bytes that contain the 
//name itself, if not too long, or else give an offset into the String Table. 
//To determine whether the name itself or an offset is given, test the first 
//four bytes for equality to zero.
//Offset Size Field Description 
//0 8 Short Name An array of eight bytes. This array is padded with nulls on 
//the right if the name is less than eight bytes long. 
//0 4 Zeroes Set to all zeros if the name is longer than eight bytes. 
//4 4 Offset Offset into the String Table. 
//8 4 Value Value associated with the symbol. The interpretation of this field 
//depends on Section Number and Storage Class. A typical meaning is the 
//relocatable address. 
//5.4.2. Section Number Values
//Normally, the Section Value field in a symbol table entry is a one-based 
//index into the Section Table. However, this field is a signed integer and may 
//take negative values. The following values, less than one, have special 
//meanings:
//Constant Value Description 
//IMAGE_SYM_UNDEFINED 0 Symbol record is not yet assigned a section. If the 
//value is 0 this indicates a references to an external symbol defined 
//elsewhere. If the value is non-zero this is a common symbol with a size 
//specified by the value. 
//IMAGE_SYM_ABSOLUTE -1 The symbol has an absolute (non-relocatable) value and 
//is not an address. 
//IMAGE_SYM_DEBUG -2 The symbol provides general type or debugging information 
//but does not correspond to a section. Microsoft tools use this setting along 
//with .file records (storage class FILE). 
//    SHORT   SectionNumber;
//5.4.3. Type Representation
//The Type field of a symbol table entry contains two bytes, each byte 
//representing type information. The least-significant byte represents simple 
//(base) data type, and the most-significant byte represents complex type, if 
//any:
//    WORD    Type;
//5.4.4. Storage Class
//The Storage Class field of the Symbol Table indicates what kind of definition 
//a symbol represents. The following table shows possible values. Note that the 
//Storage Class field is an unsigned one-byte integer. The special value -1 
//should therefore be taken to mean its unsigned equivalent, 0xFF.
//         if( ISEXTERN(dwstcls) )  // IMAGE_SYM_CLASS_EXTERNAL | IMAGE_SYM_CLASS_EXTERNAL_DEF | etc
            // Weak externals are represented by a Symbol Table record with
            // EXTERNAL storage class, UNDEF section number, and a value of 0.
//17 1 NumberOfAuxSymbols Number of auxiliary symbol table entries that follow 
//this record. 
///////////////////////////////////////////////////////////////////////////////
VOID DumpObjSymbols( PIMAGE_SYMBOL pis, DWORD dwi, PIMAGE_SECTION_HEADER psh,
                             DWORD  dwSCnt, PBYTE pStg)
{
   if( pis && dwi )
   {
      LPTSTR               lpd = &g_cBuf[0];
      LPTSTR               lpn = &g_cBuf2[0];
      LPTSTR               lpb = &g_cBuf3[0];
      LPTSTR               ptmp = &g_cTmp[0];
      INT                  iIsFunc, iType;
      DWORD                dwstcls, dwFlag, dwType;
      DWORD                dwc, dwaux, dwo;
      PBYTE                pb;
      PIMAGE_SECTION_HEADER psh2;
      PMWL                 pmwl;
      BOOL                 bHvSect = TRUE;   // have section header and count
      BOOL                 bNewLn;
      INT                  iFunCnt, iFunCnt2;

      // after the Symbol Table comes the String Table
      // beginning with a DWORD giving its LENGTH (in bytes)
      // pdw = (PDWORD)((PIMAGE_SYMBOL)pis + dwi);
      // dwssz = *pdw;
      // pStg = (PBYTE)pdw;   // the pointer passed is to the DWORD header
      sprintf(lpd, "SYMBOL TABLE: Dump of %d symbol table entries.",
         dwi );
      AddStringandAdjust(lpd);

      if( !psh || (dwSCnt == 0) )
      {
         AddStringandAdjust( "WARNING: Not passed section header or count. Limiting!" );
         bHvSect = FALSE;
      }

      dwc = 0;
      dwaux = 0;
      //dwi = poh->NumberOfSymbols;
      iIsFunc = 0;
      *lpn = 0;   // no name yet
      dwstcls = 0;
      iType = 0;  // no type
      dwFlag = 0;
      iFunCnt  = 0;
      iFunCnt2 = 0;
      pis--;   // decrement to allow for first bump
      // =====================================================================
      while(dwi--)
      {
         dwc++;
         pis++;   // bump to next Symbol Table entry
         // ======== DEAL WITH AN AUXILIARY ENTRY ========
         if( dwaux )
         {
            DumpAuxEntry( pis, iIsFunc, iType, dwstcls, dwc, psh2,
                            dwFlag, dwSCnt, psh );
            dwaux--;
            continue;
         }
         // ======== DEAL WITH AN AUXILIARY ENTRY ========

         iIsFunc = 0;   // start off NOT a FUNCTION
         *lpn = 0;
         iType = 0;     // no type
         pb = (PBYTE) &pis->StorageClass;
         dwstcls = (DWORD)*pb;
         dwFlag = 0;
         psh2 = 0;
         dwaux = (DWORD)pis->NumberOfAuxSymbols;
         dwo = (DWORD)pis->SectionNumber;    // extract SECTION number

         pb = (PBYTE) &pis->Type;
         dwType = (DWORD) *pb;      // get BASE type
         bNewLn = FALSE;

         //sprintf(lpd, "Symbol %3d Name:", dwc );
         //SetMinLen(lpd,MINHDSZ);
         sprintf(lpd, "%3d Name:", dwc );
         SetMinLen(lpd,MINSHDSZ);
         GetSymName( lpn, pis, pStg, FALSE );
         strcat(lpd,lpn);
         if( strlen(lpn) > MMXNAME )
         {
            AddStringandAdjust(lpd);
            *lpd = 0;
            SetMinLen(lpd,MINSHDSZ);
            bNewLn = TRUE;
         }
         else
            strcat(lpd, " ");

         *lpb = 0;
         if( ISVALIDSN(dwo) )
         {
            if( bHvSect )
            {
               dwo--;
               if( dwo < dwSCnt )
               {
                  psh2 = psh + dwo;    // get Section Header pointer
                  dwFlag = psh2->Characteristics;
//typedef struct _IMAGE_SECTION_HEADER {
//    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
//    union {
//            DWORD   PhysicalAddress;
//            DWORD   VirtualSize;
//    } Misc;
//    DWORD   VirtualAddress;
//    DWORD   SizeOfRawData;
//    DWORD   PointerToRawData;
//    DWORD   PointerToRelocations;
//    DWORD   PointerToLinenumbers;
//    WORD    NumberOfRelocations;
//    WORD    NumberOfLinenumbers;
//    DWORD   Characteristics;
//} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;
//#define IMAGE_SIZEOF_SECTION_HEADER          40
                  GetSectionName(lpb, psh2, pStg);

                  sprintf( ptmp, ";%3d %s %s %d ", dwo, lpn, lpb, psh2->SizeOfRawData );
                  if( ( strcmp(lpn,  lpb) == 0 ) ||
                     ( strcmp(lpn, ".bf") == 0 ) ||
                     ( strcmp(lpn, ".lf") == 0 ) ||
                     ( strcmp(lpn, ".ef") == 0 ) )
                  {
                     //*ptmp = 0;
                  }

                  if( *ptmp )
                  {
                     AppendFStgEq(ptmp, &sStorClass[0], dwstcls, FALSE);
                     strcat(ptmp, " "); // add separator
                     //AppendFStgsOr(ptmp, &sSectChars[0], (dwFlag & ~(IMAGE_SCN_ALIGN_MASK) ));
                     AppendFStgsOr1(ptmp, &sSectChars1[0], (dwFlag & ~(IMAGE_SCN_ALIGN_MASK) ));
                     strcat(ptmp, "|"); // add separator
                     //AppendFStgEq(ptmp, &sSectAlign[0], (dwFlag & IMAGE_SCN_ALIGN_MASK), TRUE);
                     AppendFStgEq1(ptmp, &sSectAlign1[0], (dwFlag & IMAGE_SCN_ALIGN_MASK) );
                     if( (strcmp(lpb, ".data") == 0) || (strcmp(lpb, ".bss") == 0) )
                     {
                        if( psh2->Misc.PhysicalAddress )
                           sprintf(EndBuf(ptmp), " PA=%x", psh2->Misc.PhysicalAddress );
                        if( psh2->Misc.VirtualSize )
                           sprintf(EndBuf(ptmp), " VS=%d", psh2->Misc.VirtualSize );
                        if( psh2->VirtualAddress )
                           sprintf(EndBuf(ptmp), " VA=%x", psh2->VirtualAddress );
                     }
                     strcat(ptmp, MEOR); // add termination
                     WriteASMFile(ptmp);
                  }
                  //pmwl = new MWL;
                  //if(pmwl)
                  //{
                  //   ZeroMemory(pmwl, sizeof(MWL));
                  //   pmwl->pSectHdr = psh2;
                  //   pmwl->pSym     = pis;
                  //   InsertTailList( &sSym2Sect, (PLE)pmwl );  // append
                  //}
               }
               else
               {
                  strcat(lpb, "OUT_OF_RNG! " );
               }
            }
            else
               strcat(lpb, "NO_SECT!" );
         }
         else
         {
            if( dwo == IMAGE_SYM_UNDEFINED )
            {
               strcat(lpb, "NYDEF");
            }
            else if( dwo == IMAGE_SYM_ABSOLUTE )
            {
               strcat(lpb, "ABSVAL"); // "absolute (non-relocatable) value" );
            }
            else if( dwo == IMAGE_SYM_DEBUG )
            {
               strcat(lpb, "DBGINF"); // "general type or debugging information" );
            }
         }

         if( lpn[0] == '.' )
         {
            // DO THE DOT PEOPLE
            if( strcmp(lpn, ".bf") == 0 )
            {
               iType = it_FunBgn;
               sprintf(EndBuf(lpd), "= begin function [%s] ", lpb );
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );
               AddStringandAdjust(lpd);
               continue;
            }
            else if( strcmp(lpn, ".lf") == 0 )
            {
               iType = it_FunLines;
               sprintf(EndBuf(lpd), "= function lines [%s] ", lpb );
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );
               AddStringandAdjust(lpd);
               continue;
            }
            else if( strcmp(lpn, ".ef") == 0 )
            {
               iType = it_FunEnd;
               sprintf(EndBuf(lpd), "= end function [%s] ", lpb );
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );
               AddStringandAdjust(lpd);
               continue;
            }
            else if( strcmp(lpn, ".file") == 0 )
            {
               iType = it_Source;
               sprintf(EndBuf(lpd), "source file [%s] ", lpb);
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );
               AddStringandAdjust(lpd);
               continue;

            }
            else if( ( strcmp(lpn, ".text") == 0 ) ||
               ( strcmp(lpn, ".drectve") == 0 ) )
            {
               iType = it_SecDef;
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               sprintf(EndBuf(lpd), " [%s] ", lpb);
               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );
               AddStringandAdjust(lpd);
               continue;
            }
            else if( ( strbgn(lpn, ".debug") == 0 ) ||
               ( strcmp(lpn, ".bss" ) == 0 ) ||
               ( strcmp(lpn, ".data" ) == 0 ) )
            {
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);
               sprintf(EndBuf(lpd), " [%s] ", lpb);

               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );

               AddStringandAdjust(lpd);
               continue;
            }
            else if( strcmp( lpn, ".rdata" ) == 0 )
            {
               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);

               sprintf(EndBuf(lpd), " [%s] ", lpb);

               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );

               AddStringandAdjust(lpd);
               continue;

            }
            // any other DOT "." people !!!!
            // *****************************
         }
         else  // if( lpn[0] != '.' )
         {
            // NOT a DOT
            pmwl = MALLOC( sizeof(MWL) );
            if(pmwl)
            {
               ZeroMemory(pmwl, sizeof(MWL));
               pmwl->pSectHdr = psh2;
               pmwl->pSym     = pis;
               InsertTailList( &sSym2Sect, (PLE)pmwl );  // append
            }

            if( strcmp(lpb, ".text") == 0 )
            {

               // FIX20010731 - remove this additional test
               // ( dwFlag & IMAGE_SCN_LNK_COMDAT  ) &&
               if( ( dwFlag & IMAGE_SCN_CNT_CODE    ) &&
                   ( dwFlag & IMAGE_SCN_MEM_EXECUTE ) )
               {
                  iFunCnt++;
                  if(pmwl)
                     pmwl->bIsFunc = TRUE;   // set it as a CODE FUNCTION
               }
               else
               {
                  iFunCnt2++; // this would be an INTERNAL Object file error!!!
               }

               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);

               if( ISEXTERN(dwstcls) )
               {
                  if( dwType == 0x020 )
                     iIsFunc = 3;   // set as a FUNCTION for any AUX following
                  strcat(lpd, " (Global function Object)");
               }
               else
               {
                  strcat(lpd, " (Local Function Object)");
               }

               if( psh2 && psh2->SizeOfRawData )
                  sprintf(EndBuf(lpd), " %d bytes", psh2->SizeOfRawData );

               // out this line, and start a NEW one
               AddStringandAdjust(lpd);
               *lpd = 0;
               SetMinLen(lpd,MINSHDSZ);
               // ==================================

               sprintf(EndBuf(lpd), "[%s] ", lpb);

               dwo = (dwFlag & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
               AppendFStgEq(lpd, &sSectAlign[0], dwo, TRUE);

               strcat(lpd, " "); // add separator

               dwo = (dwFlag & ~(IMAGE_SCN_ALIGN_MASK) );   //  0x00F00000
               AppendFStgsOr(lpd, &sSectChars[0], dwo);

               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );

               AddStringandAdjust(lpd);

               continue;

            }
            else  // NOT "associated" with a .text section
            {

               AppendFStgEq(lpd, &sStorClass[0], dwstcls, TRUE);

               if( ISEXTERN(dwstcls) )
                  strcat(lpd, " (Global Class)");
               else
                  strcat(lpd, " (Local Class)");

               if( psh2 && psh2->SizeOfRawData )
                  sprintf(EndBuf(lpd), " %d bytes", psh2->SizeOfRawData );

               // out this line, and start a NEW one
               AddStringandAdjust(lpd);
               *lpd = 0;
               SetMinLen(lpd,MINSHDSZ);
               // ==================================

               sprintf(EndBuf(lpd), "[%s] ", lpb);

               dwo = (dwFlag & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
               AppendFStgEq(lpd, &sSectAlign[0], dwo, TRUE);

               strcat(lpd, " "); // add separator

               dwo = (dwFlag & ~(IMAGE_SCN_ALIGN_MASK) );   //  0x00F00000
               AppendFStgsOr(lpd, &sSectChars[0], dwo);

               if( dwaux )
                  sprintf(EndBuf(lpd), " Aux=%d", dwaux );

               AddStringandAdjust(lpd);

               continue;

            }
            // done ALL the labels

         }

         // it appear NOTHING falls thru here!!!
         // ************************************
         strcat(lpd,lpb);
         AddStringandAdjust(lpd);
         chkme( "SYMNOTE: CHECKME ON THIS FALL THRU!!!"MEOR
            "\tLine=[%s]"MEOR, lpd );
         AddStringandAdjust( "SYMNOTE: CHECKME ON THIS FALL THRU!!!" );

         // done this Symbol Entry
      }  // for dwi SYMBOL TABLE count
      // =====================================================================

      if( iFunCnt )
      {
         if( iFunCnt2 )
         {
            sprintf( lpd, "There appears to be %d FUNCTION OBJECTS defined", iFunCnt );
            AddStringandAdjust(lpd);
            sprintf( lpd, "and some other %d .text without full function class",
               iFunCnt2 );
         }
         else
         {
            sprintf( lpd, "There are %d FUNCTION OBJECTS defined.", iFunCnt );
         }
         AddStringandAdjust(lpd);
      }
   }  // pSym = got pointer to SYMBOL TABLE
}

VOID  Show_Target_Machine( DWORD dwo )
{
   LPTSTR     lpd  = &g_cBuf[0];
   PFLG2STGT  pf2s = &sMachRel[0];
   strcpy(lpd, szMch);  // "Target Machine:";
   SetMinLen(lpd,MINHDSZ);
   while( pf2s->pShort )
   {
      if( pf2s->dwFlag == dwo )
         break;
      pf2s++;
   }
   if( !pf2s->pShort )
      pf2s = &sMachRel[0];

   g_pMchRel = pf2s;          // P1 - save the TYPE pointer for later use
   strcat(lpd, pf2s->pShort);
   sprintf(EndBuf(lpd), " (0x%x)", dwo );
   AddStringandAdjust(lpd);
}

VOID Show_Number_Sections( DWORD dwo )
{
   LPTSTR     lpd  = &g_cBuf[0];
   LPTSTR     lpn = &g_cBuf2[0];
   strcpy(lpd, szSec);  // "Number of Sections:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);
}

// FIX20061216 - avoid a DEBUG assert, by checking the TIME before
#define M_MAX__TIME64_T     0x100000000000i64    /* number of seconds from
                                                   00:00:00, 01/01/1970 UTC to
                                                   23:59:59. 12/31/2999 UTC */

VOID Show_DateTime_Stamp( DWORD dwi )
{
   time_t     lt;
   time_t *   plt = &lt;
   LPTSTR     lpd  = &g_cBuf[0];
   LPTSTR     lpn = &g_cBuf2[0];
   lt = dwi;                  // make into 64-bit value
   strcpy(lpd, szTim);  // "Date/Time Stamp:";
   SetMinLen(lpd,MINHDSZ);
   if( *plt >= M_MAX__TIME64_T ) { // extracted from DWORD, thus MUST be LESS
      strcpy(lpn, "OUT OF RANGE!"); // so this should NEVER happen again
      sprintf(EndBuf(lpn), "(Mx=%I64u)", M_MAX__TIME64_T);
   } else {
      strcpy(lpn, ctime(plt) ); // DWORD always LESS than MAX value
   }
   dwi = strlen(lpn);
   while(dwi--)
   {
      if(lpn[dwi] > ' ')
         break;
      lpn[dwi] = 0;
   }
   strcat(lpd, lpn);
   sprintf(EndBuf(lpd), " (%I64u s. 01/01/70)", *plt );
   AddStringandAdjust(lpd);
}

VOID  Show_Symbols_Count( DWORD dwi, DWORD dwo, PBYTE pHead,
                         PBYTE * ppSym )
{
   LPTSTR        lpd  = &g_cBuf[0];
   PBYTE         pb, pSym;
   PBYTE         pStg = 0;
   PDWORD        pdw;
   PIMAGE_SYMBOL pis;
   DWORD         dwssz, dwc, dwscnt;

   pSym = 0;
   if(dwi && dwo)
   {
      pSym = pHead + dwi;
      *ppSym = pSym;
      pis = (PIMAGE_SYMBOL)pSym;
      // store information in BOTH views
      g_pSymTbl = pis;          // P1 - save the SYMBOL TABLE pointer for later use
      g_dwSymCnt = dwo;          // P1 - save the SYMBOL TABLE count for later use

      // at end of SYMBOL TABLE is the STRING TABLE
      // ==========================================
      pStg = (PBYTE)((PIMAGE_SYMBOL)pis + dwo);
      pdw = (PDWORD)pStg;
      dwssz = *pdw;
      if( dwssz > 4 )
         dwc = dwssz - sizeof(DWORD);   // 4
      else
         dwc = 0;
      pb = pStg + 4;
      dwscnt = 0;
      while(dwc--)
      {
         if( *pb == 0 )
            dwscnt++;
         pb++;
      }
   }

   // symbol table
   strcpy(lpd, szPtr);  // "Pointer to symbols:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%#x", dwi );
   AddStringandAdjust(lpd);

   strcpy(lpd, szNum);  // "Number of Symbols:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);

}

VOID Show_OH_Size( DWORD dwo )
{
   LPTSTR        lpd  = &g_cBuf[0];
   strcpy(lpd, szOHS);  // "Size of Optional Header:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d", dwo );
   if(dwo)
      sprintf(EndBuf(lpd), " (0x%x)", dwo );
   AddStringandAdjust(lpd);
}

VOID Show_Characteristics( DWORD dwi )
{
   LPTSTR        lpd   = &g_cBuf[0];
   PFLG2STG      pf2s1 = &sHdrChars[0];

   strcpy(lpd, szChr);  // "Characteristics:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%08x", dwi );
   AddStringandAdjust(lpd);
   while( dwi && pf2s1->pShort )
   {
      if( dwi & pf2s1->dwFlag )
      {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         //strcat(lpd, pf2s1->pShort);
         strcat(lpd, pf2s1->pDesc);
         AddStringandAdjust(lpd);
         dwi &= ~(pf2s1->dwFlag);
      }
      pf2s1++;
   }
   if( dwi )
   {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         sprintf(EndBuf(lpd), "0x%x remains!", dwi );
         AddStringandAdjust(lpd);
   }
}

PBYTE DumpObjHeader(PBYTE pHead, PIMAGE_SECTION_HEADER psh,
                             DWORD  dwSCnt )
{
   LPTSTR               lpd = &g_cBuf[0];
   LPTSTR               lpn = &g_cBuf2[0];
   LPTSTR               lpb = &g_cBuf3[0];
   PIMAGE_FILE_HEADER   poh = (PIMAGE_FILE_HEADER)pHead;
   //PFLG2STGT            pf2s; // = &sMachRel[0];
   //time_t               lt;
   //time_t *             plt = &lt;
   //PFLG2STG1            pf2s1 = &sCharacter[0];
   //PFLG2STG             pf2s1 = &sHdrChars[0];
   PDWORD               pdw;
   DWORD                dwi, dwo, dwc;
   DWORD                dwssz, dwscnt;    // string SIZE and COUNT
   PBYTE                pb, pSym;
   PBYTE                pStg = 0;
   PIMAGE_SYMBOL        pis;

   g_pSymTbl = 0;          // P1 - save the SYMBOL TABLE pointer for later use
   g_dwSymCnt = 0;          // P1 - save the SYMBOL TABLE count for later use

   // KillLList(&sSym2Sect);
   DeleteWorkList();

   strcpy(lpd, "Object Image Header:");
   if( !pHead )
   {
      strcat( lpd, " POINTER is NULL!" );
      AddStringandAdjust(lpd);
      return NULL;
   }

   AddStringandAdjust(lpd);

   // Target Machine
   dwo = (DWORD)poh->Machine;
   Show_Target_Machine( dwo );

   // Number of Sections
   dwo = poh->NumberOfSections;
   Show_Number_Sections( dwo );

   // Date/Time Stamp = Seconds since midnight on January 1, 1970
   // like (UNIX) time() - returns seconds since midnight (00:00:00), January 1, 1970
   // FIX20061216 - Time stamp is only a DWORD, NOT a 64-bit entity
   //plt = (time_t *)&poh->TimeDateStamp;
   dwi = poh->TimeDateStamp;  // extract the DWORD
   Show_DateTime_Stamp(dwi);

   strcpy(lpd, "; ");
   strcat(lpd, szTim);  // "Date/Time Stamp:";
   strcat(lpd, " ");
   strcat(lpd, lpn);
   strcat(lpd, MEOR);
   WriteASMFile(lpd);

   // Symbol Table
   dwo = poh->NumberOfSymbols;
   dwi = poh->PointerToSymbolTable;
   if(dwi && dwo)
   {
      pSym = pHead + dwi;
      pis = (PIMAGE_SYMBOL)pSym;
      // store information in BOTH views
      g_pSymTbl = pis;          // P1 - save the SYMBOL TABLE pointer for later use
      g_dwSymCnt = dwo;          // P1 - save the SYMBOL TABLE count for later use

      // at end of SYMBOL TABLE is the STRING TABLE
      // ==========================================
      pStg = (PBYTE)((PIMAGE_SYMBOL)pis + dwo);
      pdw = (PDWORD)pStg;
      dwssz = *pdw;
      if( dwssz > 4 )
         dwc = dwssz - sizeof(DWORD);   // 4
      else
         dwc = 0;
      pb = pStg + 4;
      dwscnt = 0;
      while(dwc--)
      {
         if( *pb == 0 )
            dwscnt++;
         pb++;
      }
   }

   // symbol table
   strcpy(lpd, szPtr);  // "Pointer to symbols:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%#x", dwi );
   AddStringandAdjust(lpd);

   strcpy(lpd, szNum);  // "Number of Symbols:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);

   // Size of Optional Header
   dwo = poh->SizeOfOptionalHeader;
   Show_OH_Size( dwo );

   dwi = (DWORD) poh->Characteristics;
   Show_Characteristics( dwi );

   if( pSym )
   {
      sprintf(lpd, "STRING TABLE: Contains %d strings. Total length of %d bytes.",
         dwscnt, dwssz );
      AddStringandAdjust(lpd);
      // ===========================================
      dwi = poh->NumberOfSymbols;
      DumpObjSymbols( pis, dwi, psh, dwSCnt, pStg );
   }  // pSym = got pointer to SYMBOL TABLE
   else
   {
      AddStringandAdjust( "EEK: Appears to be NO SYMBOL TABLE, thus NO STRING TABLE!!!");
      pSym = 0;
   }

   return pStg;
}

//typedef struct ANON_OBJECT_HEADER {
//    WORD    Sig1;            // Must be IMAGE_FILE_MACHINE_UNKNOWN
//    WORD    Sig2;            // Must be 0xffff
//    WORD    Version;         // >= 1 (implies the CLSID field is present)
//    WORD    Machine;
//    DWORD   TimeDateStamp;
//    CLSID   ClassID;         // Used to invoke CoCreateInstance
//    DWORD   SizeOfData;      // Size of data that follows the header
//} ANON_OBJECT_HEADER;
// SEEMS FOLLOWED BY
//typedef struct _IMAGE_FILE_HEADER {
//    WORD    Machine;
//    WORD    NumberOfSections;
//    DWORD   TimeDateStamp;
//    DWORD   PointerToSymbolTable;
//    DWORD   NumberOfSymbols;
//    WORD    SizeOfOptionalHeader;
//    WORD    Characteristics;
//} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;


PBYTE DumpAnonObjHeader( LPDFSTR lpdf )
{
   PTSTR                  lpd = &g_cBuf[0];
   PBYTE pHead                = lpdf->df_pVoid;
   ANON_OBJECT_HEADER * panon = (ANON_OBJECT_HEADER *)pHead;
   PIMAGE_FILE_HEADER pifh    = (PIMAGE_FILE_HEADER)((ANON_OBJECT_HEADER *)panon + 1);
   PIMAGE_SECTION_HEADER psh  = (PIMAGE_SECTION_HEADER)(pifh + 1);   
   PTSTR    pStrings = NULL;
   DWORD dwo, dwi, dws, dwc;
   WORD  vers, opts;

   g_pSymTbl = 0;          // P1 - save the SYMBOL TABLE pointer for later use
   g_dwSymCnt = 0;          // P1 - save the SYMBOL TABLE count for later use

   // KillLList(&sSym2Sect);
   DeleteWorkList();

   sprintf(lpd, "ANON Object Image Header (%d):", sizeof(ANON_OBJECT_HEADER) );
   if( !pHead )
   {
      strcat( lpd, " POINTER is NULL!" );
      AddStringandAdjust(lpd);
      return NULL;
   }
   AddStringandAdjust(lpd);

   strcpy(lpd, "Sig1:Sig2:" );
   SetMinLen(lpd,MINHDSZ);
   strcat(lpd, "0000:FFFF");
   AddStringandAdjust(lpd);

   vers = panon->Version;  // extract VERSION
   strcpy(lpd, "Version:");  // "Target Machine:";
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (%#0X)", vers, vers);
   AddStringandAdjust(lpd);

   // Target Machine
   dwo = (DWORD)panon->Machine;
   Show_Target_Machine( dwo );

   // Time stamp
   dwi = panon->TimeDateStamp;  // extract the DWORD
   Show_DateTime_Stamp(dwi);

   strcpy(lpd,"CLSID:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%s (If version >= 1)", Get_CLSID_Stg( &panon->ClassID ) );
   AddStringandAdjust(lpd);

   dws = panon->SizeOfData;
   strcpy(lpd,"SizeOfData:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (%#0X)", dws, dws );
   AddStringandAdjust(lpd);

   //pifh = (PIMAGE_FILE_HEADER)((ANON_OBJECT_HEADER *)panon + 1);

   sprintf(lpd, "Object Image File Header: (%d)", sizeof(IMAGE_FILE_HEADER) );
   AddStringandAdjust(lpd);
   if( VERB9 ) {
      ProcessHex( (PBYTE)pifh, sizeof(IMAGE_FILE_HEADER) );
   }
   dwo = pifh->Machine;
   Show_Target_Machine( dwo );

   dwc = pifh->NumberOfSections;
   sprintf(lpd, "NumberOfSections:"); // WORD
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd),"%d", dwc );
   AddStringandAdjust(lpd);

   // Time stamp
   dwi = pifh->TimeDateStamp;  // extract the DWORD
   Show_DateTime_Stamp(dwi);

   dwi = pifh->PointerToSymbolTable;
   strcpy(lpd, "PointerToSymbolTable:" ); // (DWORD)
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd),"%d (%#0X)", dwi, dwi );
   AddStringandAdjust(lpd);

   dwi = pifh->NumberOfSymbols;
   strcpy(lpd, "NumberOfSymbols:"); //  (DWORD):
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd),"%d (%#0X)", dwi, dwi );
   AddStringandAdjust(lpd);

   opts = pifh->SizeOfOptionalHeader;
   strcpy(lpd, "SizeOfOptionalHeader:"); //  (WORD):
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd),"%d (%#0X)", opts, opts );
   AddStringandAdjust(lpd);

   dwi = pifh->Characteristics;
   Show_Characteristics( dwi );

   // ProcessHex( (PBYTE)psh, sizeof(IMAGE_SECTION_HEADER) );
   // get POINTER to SYMBOL FILE!!!
   // pSym = pHead + pifh->PointerToSymbolTable;
   // *** BUT THIS DOES NOT APPEAR RIGHT FOR ANON OBJECT FILES ***
   if( pifh->PointerToSymbolTable && pifh->NumberOfSymbols )
   {
      // Symbol Table
      PIMAGE_SYMBOL pis;
      PBYTE pSym, pStg, pb;
      PDWORD   pdw;
      DWORD    dwssz, dwscnt;
      dwo = pifh->NumberOfSymbols;
      dwi = pifh->PointerToSymbolTable;
      pSym = pHead + dwi;
      pis = (PIMAGE_SYMBOL)pSym;
      // store information in BOTH views
      //g_pSymTbl = pis;          // P1 - save the SYMBOL TABLE pointer for later use
      //g_dwSymCnt = dwo;          // P1 - save the SYMBOL TABLE count for later use

      // at end of SYMBOL TABLE is the STRING TABLE
      // ==========================================
      // APPARENTLY _NOT_ FOR ANONO OBJECT FILES
      pStg = (PBYTE)((PIMAGE_SYMBOL)pis + dwo);
      pdw = (PDWORD)pStg;
      dwssz = *pdw;
      if( dwssz > 4 )
         dwc = dwssz - sizeof(DWORD);   // 4
      else
         dwc = 0;
      pb = pStg + 4;
      dwscnt = 0;
      //while(dwc--)
      //{
      //   if( *pb == 0 )
      //      dwscnt++;
      //   pb++;
      //}
   }

   dwc = pifh->NumberOfSections;
   // psh = (PIMAGE_SECTION_HEADER)(pifh + 1);   
   if(dwc) {
      for(dwi = 0; dwi < dwc; dwi++) {
         psh++;
      }
      pStrings = (PBYTE)psh;
   }

   return pStrings;
}

BOOL ObjFrame( LPDFSTR lpdf )
{
   PBYTE                pHead;
   PIMAGE_FILE_HEADER   poh;
   DWORD                dwi, dwc;
   PIMAGE_SECTION_HEADER   psh;
   LPTSTR               lpd = &g_cBuf[0];
   PBYTE                pb, pStgs;
   INT                  iLen, iOff;
   LPTSTR               lpfn = &lpdf->fn[0];
   int                  isanon;
   _try
	{
      pHead = (PBYTE) lpdf->df_pVoid;
      iLen  = lpdf->dwmax;
      isanon = Is_ANON_OBJECT_HEADER(pHead);

      strcpy( lpd, "; Processing file [");
      strcat( lpd, lpdf->fn );
      sprintf(EndBuf(lpd), "] of length %d"MEOR, iLen);
      WriteASMFile(lpd);

      iOff = 0;
      poh = (PIMAGE_FILE_HEADER)pHead;
      dwc = 0;
      psh = 0;
      if( isanon ) {
         ANON_OBJECT_HEADER * panon = (ANON_OBJECT_HEADER *)pHead;
         poh = (PIMAGE_FILE_HEADER)((ANON_OBJECT_HEADER *)panon + 1);
         psh = (PIMAGE_SECTION_HEADER)((PIMAGE_FILE_HEADER)poh + 1);
         iOff += sizeof(ANON_OBJECT_HEADER);
         iOff += sizeof(IMAGE_FILE_HEADER);
         // dump header and SYMBOL TABLE
         pStgs = DumpAnonObjHeader( lpdf );
         // strings does NOT appear valid for ANON
         pStgs = 0;
         dwc = poh->NumberOfSections;
         if(dwc)
         {
            sprintf( lpd, "Total of %d (%#x) Sections, each %d", dwc, dwc,
               sizeof(IMAGE_SECTION_HEADER) );
            AddStringandAdjust(lpd);
            for( dwi = 0; dwi < dwc; dwi++ )
            {
               sprintf(lpd, "SECTION HEADER #%x (%d)",
                  (dwi + 1),
                  (dwi + 1));
               AddStringandAdjust(lpd);

               DumpSectionName(pHead, iOff, iLen, (dwi + 1),
                  pStgs, lpfn, psh);

               psh++;   // bump to NEXT SECTION HEADER
               iOff += sizeof(IMAGE_SECTION_HEADER);  // and bump OFFSET into image
            }
         }
      } else {
         if( poh ) {
            dwc = poh->NumberOfSections;
            if(dwc) {
               psh = (PIMAGE_SECTION_HEADER)((PIMAGE_FILE_HEADER)poh + 1);
               iOff += sizeof(IMAGE_FILE_HEADER);
               if( poh->SizeOfOptionalHeader ) {
                  pb    = (PBYTE)psh;
                  pb   += poh->SizeOfOptionalHeader;
                  iOff += poh->SizeOfOptionalHeader;
                  psh   = (PIMAGE_SECTION_HEADER)pb;
               }
            }
         }

         // dump header and SYMBOL TABLE
         pStgs = DumpObjHeader( pHead, psh, dwc );

         iOff = 0;
         poh = (PIMAGE_FILE_HEADER)pHead;
         if( poh )
         {
            dwc = poh->NumberOfSections;
            if(dwc)
            {
               sprintf( lpd, "Total of %d (%#x) Sections", dwc, dwc );
               AddStringandAdjust(lpd);

               psh = (PIMAGE_SECTION_HEADER)((PIMAGE_FILE_HEADER)poh + 1);
               iOff += sizeof(IMAGE_FILE_HEADER);

               if( poh->SizeOfOptionalHeader )
               {
                  pb    = (PBYTE)psh;
                  pb   += poh->SizeOfOptionalHeader;
                  iOff += poh->SizeOfOptionalHeader;
                  psh   = (PIMAGE_SECTION_HEADER)pb;
               }
               for( dwi = 0; dwi < dwc; dwi++ )
               {
                  sprintf(lpd, "SECTION HEADER #%x (%d)",
                     (dwi + 1),
                     (dwi + 1));

                  AddStringandAdjust(lpd);

                  DumpObjSection( (PBYTE)psh, iOff, iLen, (dwi + 1), pStgs, lpfn );

                  psh++;   // bump to NEXT SECTION HEADER
                  iOff += sizeof(IMAGE_SECTION_HEADER);  // and bump OFFSET into image
               }
            }
         }
      }
	}
	_except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
         EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		MessageBox( NULL, "ERROR: EXCEPTION_ACCESS_VIOLATION!"MEOR
         "CORRUPTED FILE: Can not display all information!",
         "ACCESS VIOLATION",
         MB_OK );
		sprtf( "ERROR: EXCEPTION_ACCESS_VIOLATION!"MEOR
         "CORRUPTED FILE: Can not display all information!"MEOR );

   }

   DeleteWorkList();
   // and if ever I move to NOT global, then
   //cfMyP2->DeleteWorkList();

	return TRUE;
}

BOOL  ProcessOBJ( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   // PIMAGE_NT_HEADERS pnth = ImageNtHeader( lpdf->df_pVoid );

   if( IsObjectFile( lpdf->fn, lpdf->lpb, (INT)lpdf->dwmax ) ) {
      bRet = ObjFrame( lpdf );
   } else {
      int i = IsEXEFile( lpdf );
      if( i == 1 ) {
         bRet = DumpEXEFile( lpdf );
      } else if( i == 2 ) {
         bRet = DumpOS2File( lpdf );
      } else {
         sprtf("WARNING: Failed IsObjectFile and IsEXEFile test ... reverting to DUMP ..."MEOR);
      }
   }

   return bRet;
}

void DumpSectionName(PBYTE pHead, INT iOff, INT iLen, INT iNum,
                     PBYTE pStgs, LPTSTR lpfn,
                     PIMAGE_SECTION_HEADER psh)
{
   PTSTR lpd = &g_cBuf[0];
   PTSTR lpb = &g_cBuf2[0];
   PTSTR lpname = &g_cBuf3[0];
   size_t   k;
   PIMAGE_SYMBOL           pSymTbl, pis;
   DWORD    dwo, dws, dwi, dwr, dwFlag;
   PBYTE    pb, pRaw, pRel;
   PBYTE    pb2 = pHead;
   PFLG2STG pf2s1 = &sSectChars[0];
   PFLG2STG pf2sa = &sSectAlign[0];
   BOOL  bIsFunc = FALSE;
   PMWL  pmwl = 0;
   DWORD dwmax, dwmin, dwSymCnt;
   int   i;
   PFLG2STGT pf2st = g_pMchRel;   // P2 - get previous assignment
   PFLG2STG  pf2s;

   pSymTbl = 0;
   pis = 0;
   if( VERB9 ) {
      ProcessHex( (PBYTE)psh, sizeof(IMAGE_SECTION_HEADER) );
   }
   // Section NAME
   // NOTE: If .text, then is CODE
   strcpy(lpd, "Section Name:");
   SetMinLen(lpd,MINHDSZ);
   k = strlen(lpd);
   *lpname = 0;
   GetSectionName( lpname, psh, pStgs );
   strcat(lpd, lpname);

   *lpb = 0;
   if(pis)
   {
      GetSymName( lpb, pis, pStgs, FALSE );
      if( *lpb )
      {
         strcat(lpd, " (Sym:");
         strcat(lpd, lpb);
         strcat(lpd, ")");
      }
   }

#ifdef   ADDOFF1
   strcat(lpd, " ");
   sprintf(EndBuf(lpd), "(pHead=%#x)", (pHead - pb2));
#endif   // ADDOFF1
   AddStringandAdjust(lpd);

   dwo = psh->Misc.PhysicalAddress; 
   if(dwo)
   {
      strcpy(lpd, "Physical Address:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%#x", dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->Misc.VirtualSize; 
   if(dwo)
   {
      strcpy(lpd, "Virtual Size:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->VirtualAddress;
   strcpy(lpd, "Virtual Address:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%#x", dwo );
   AddStringandAdjust(lpd);

   dws = psh->SizeOfRawData;
   strcpy(lpd, "Size of Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dws, dws );
   AddStringandAdjust(lpd);

   //20 4 PointerToRawData File pointer to section's first page within the COFF 
   //file.
   dwi = psh->PointerToRawData;
   if( dwi )
      pb = pHead + dwi;
   else
      pb = 0;
   pRaw = pb;  // keep RAW DATA pointer

   strcpy(lpd, "Ptr to Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwr = psh->NumberOfRelocations;
   dwi = psh->PointerToRelocations;
   if(dwi)
      pRel = pHead + dwi;
   else
      pRel = 0;

   strcpy(lpd, "Ptr to Reloactions:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwo = psh->PointerToLinenumbers;
   strcpy(lpd, "Ptr to Line Numbers:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwo );
   AddStringandAdjust(lpd);

   strcpy(lpd, "Relocation Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwr, dwr );
   AddStringandAdjust(lpd);

   dwo = psh->NumberOfLinenumbers;
   strcpy(lpd, "Line Number Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);

   dwFlag = dwi = psh->Characteristics;
   strcpy(lpd, "Characteristics:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%08x", dwi );
   AddStringandAdjust(lpd);

   dwo = (dwi & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
   dwi &= ~(dwo);
   if(dwo)
   {
      while( dwo && pf2sa->pShort )
      {
         if( dwo == pf2sa->dwFlag )
         {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            strcat(lpd, pf2sa->pShort);
            AddStringandAdjust(lpd);
            dwo = 0;
            break;
         }
         pf2sa++;
      }
      if( dwo )
      {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            sprintf(EndBuf(lpd), "Note: %#x alignment!", dwo);
            AddStringandAdjust(lpd);
      }
   }
   else
   {
      *lpd = 0;
      SetMinLen(lpd,MINHDSZ);
      strcat(lpd, "Align 16 bytes (default)");
      AddStringandAdjust(lpd);
   }


   while( dwi && pf2s1->pShort )
   {
      //dwo = (dwi & pf2s1->dwFlag);
      if( dwi & pf2s1->dwFlag )
      {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         strcat(lpd, pf2s1->pShort);
         AddStringandAdjust(lpd);
         dwi &= ~(pf2s1->dwFlag);
      }
      pf2s1++;
   }
   if( dwi )
   {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         sprintf(EndBuf(lpd), "Note: %#X remains!", dwi );
         AddStringandAdjust(lpd);
   }

   if( pRaw && dws )
   {
      if( pis )   // do we have a SYMBOL pointer
      {
         *lpb = 0;
         GetSymName( lpb, pis, pStgs, FALSE );
         // This mean there is an associated LABEL
         // It is either a FUNCTION, if .text
         // or other program data
         //if( strcmp( lpname, ".text" ) == 0 )
         if( bIsFunc )
         {
            sprintf(lpd, "SECTION #%d (%#x) CODE for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) CODE DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            DumpCodeSection( pmwl, pb2, pStgs, pRaw, dws, lpfn );
         }
         else if( *lpb != '.' )
         {
            sprintf(lpd, "SECTION #%d (%#x) DATA for %s (Len=%d)",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);

            *lpd = 0;
            GetSymName( lpd, pis, pStgs, FALSE );

            if( *lpd != '_' )
               Write2ASMFile( szNul, 0 );  // add extra line to FILE only

            sprintf(EndBuf(lpd), " LABEL BYTE ; %d", dws);
            AddASMString( lpd, 0 );

            {
               BOOL  bs = FALSE;
               INT   i;
               pb = pRaw;
               dwo = dws;
               *lpb = 0;
               while( dwo-- )
               {
                  i = (INT)*pb;
                  if( ( i != '\'' ) && ( isprint(i) ) )
                  {
                     if(bs)   // if in string
                        sprintf(EndBuf(lpb), "%c", i );
                     else
                     {
                        if( *lpb )
                           strcat(lpb, ",");
                        sprintf(EndBuf(lpb), "'%c", i );
                        bs = TRUE;
                     }
                  }
                  else  // NOT printable, or is a "'" character
                  {
                     if(bs)
                     {
                        bs = FALSE;          // off the string
                        strcat(lpb, "',");   // close and add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                     else
                     {
                        if( *lpb )     // if previous
                           strcat(lpb, ","); // add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                  }
                  pb++;

                  if( strlen(lpb) > 65 )
                  {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

                     *lpb = 0;

                  }
               }  // while there is data

               if( *lpb )
               {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

               }
            }
         }
         else
         {
            sprintf(lpd, "SECTION #%d (%#x) OTHER for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            pb = pRaw;
            dwo = dws;
            while( dwo )
            {
               if( dwo > 16 )
                  dwi = 16;
               else
                  dwi = dwo;
               *lpd = 0;
               GetHEXString( lpd, pb, dwi, pb2, TRUE );
               AddStringandAdjust(lpd);
               pb  += dwi;    // bump the offset
               dwo -= dwi;    // reduce remaining count
            }
         }
      }
      else
      {
         i = 0;
         sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %s (Len=%d).",
            iNum, iNum, lpname, dws );
         //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
         //   iNum, iNum, dws, dws,
         //   (pRaw - pb2) );
         AddStringandAdjust(lpd);
         pb = pRaw;
         dwo = dws;  // this is the TOTAL size
         // FIX20061216 - put it ALL out if VERB9
         dwmax = 0; // start with NO LIMITS
         dwmin = 0;

         if( !VERB9 && (dwo > (MXOUTP * 2)) )
         {
            dwmin = MXOUTP;
            dwmax = dwo - MXOUTP;
         }
         while( dwo )
         {
            if( dwo > MX1LINE )
               dwi = MX1LINE;
            else
               dwi = dwo;
            if( dwmax )
            {
               if( (dwo >= dwmax) ||
                  (dwo <= dwmin)  )
                  *lpd = 0;
               else
                  *lpd = ' ';
            }
            else
               *lpd = 0;
            if( *lpd == 0 )
            {
               GetHEXString( lpd, pb, dwi, pb2, TRUE );
               AddStringandAdjust(lpd);
            }
            else if( i == 0 )
            {
               sprintf( lpd, "... *** Excluded %d bytes of DUMP data! *** ...",
                  (dws - (MXOUTP * 2)) );
               AddStringandAdjust(lpd);
               i++;
            }
            pb  += dwi;    // bump the offset
            dwo -= dwi;    // reduce remaining count
         }
      }

      if( dwFlag & IMAGE_SCN_LNK_INFO )
      {
         pb = pRaw;
         sprintf(lpd, "SECTION #%d (%#x) Linker Information (%d bytes).",
            iNum, iNum, dws );
         AddStringandAdjust(lpd);
         dwi = dws;
         *lpb = 0;
         dwo = 0;
         while(dwi)
         {
            if( *pb > ' ' )
               lpb[dwo++] = *pb;
            else if(dwo)
            {
               lpb[dwo] = 0;
               *lpd = 0;
               SetMinLen(lpd,MINHDSZ);
               strcat(lpd,lpb);
               AddStringandAdjust(lpd);
               dwo = 0;
            }
            pb++;
            dwi--;
         }
      }

   }

   // relocations
   if( dwr && pRel )
   {
      PIMAGE_RELOCATION pIRel = (PIMAGE_RELOCATION)pRel;
      sprintf(lpd, "SECTION #%d (%#x) RELOCATIONS for %d count.(p=%#x)",
         iNum, iNum, dwr,
         (pRel - pb2) );
      AddStringandAdjust(lpd);
      dwi = dwr;
      //Offset Index Type
//typedef struct _IMAGE_RELOCATION {
//    union {
//    DWORD   VirtualAddress;
//    DWORD   RelocCount; // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL
//    };
//    DWORD   SymbolTableIndex;
//    WORD    Type;
//} IMAGE_RELOCATION;
//typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;
// SymbolTableIndex - A zero-based index into the symbol table.
// This symbol gives the address to be used for the relocation.
// If the specified symbol has section storage class,
// then the symbol's address is the address with the first section
// of the same name. 
      //                 "12345678 12345678 12345678
      AddStringandAdjust("  Offset    Index Type     Name");
      while(dwi--)
      {
         sprintf(lpd, "%8x %8x ",
            pIRel->VirtualAddress,
            pIRel->SymbolTableIndex );
         dwo = pIRel->Type;
         pf2s = 0;
         if( pf2st )
            pf2s = pf2st->pRel;  // pf2st = m_pMchRel;   // get previous assignment
         if( pf2s )
         {
            AppendFStgEq( lpd, pf2s, dwo, TRUE );
            strcat(lpd," ");
         }
         else
         {
            sprintf(EndBuf(lpd), "NRT but Flag=%#x ", dwo );
         }

         dwo = pIRel->SymbolTableIndex;
         i = 0;
         if( g_pSymTbl && g_dwSymCnt && (dwo < g_dwSymCnt) )
         {
            pSymTbl  = (PIMAGE_SYMBOL)((LPTSTR)g_pSymTbl + dwo);
            dwSymCnt = g_dwSymCnt;
            *lpb = 0;
            i = GetSymName(lpb, pSymTbl, pStgs, FALSE );
            strcat(lpd, lpb);
         }
         else
         {
            if( !g_pSymTbl || !g_dwSymCnt )
            {
               strcat(lpd, "NO SymTbl or SymCnt?" );
            }
            else
            {
               sprintf(EndBuf(lpd), "Index %d into 0x%x of %d cnt ERRANT!",
                  dwo,
                  g_pSymTbl,
                  g_dwSymCnt );
            }
         }
         AddStringandAdjust(lpd);
         pIRel++;    // bump to NEXT
      }
   }

   // ***TBD*** Show the other items, linenumbers, etc
}

// ======== NEW CODE IMPORTED AND PORTED FROM YAHU PROJECT =======
//VOID DumpObjSection(PBYTE pHead, INT iOff, INT iLen, INT iNum,
//                              PBYTE pStgs )

VOID DumpObjSection(PBYTE pHead, INT iOff, INT iLen, INT iNum,
                              PBYTE pStgs, LPTSTR lpfn )
{
   LPTSTR                  lpd = &g_cBuf[0];
   LPTSTR                  lpb = &g_cBuf2[0];
   LPTSTR                  lpname = &g_cBuf3[0];
   PIMAGE_SECTION_HEADER   psh = (PIMAGE_SECTION_HEADER)pHead;
   INT                     i, k;
   //LPTSTR                  lps;
//   PFLG2STG1               pf2s1 = &sSecChars[0];
//   PFLG2STG1               pf2sa = &sSecAlign[0];
   PFLG2STG                pf2s1 = &sSectChars[0];
   PFLG2STG                pf2sa = &sSectAlign[0];
   DWORD                   dwi, dwo, dwFlag;
   PBYTE                   pb, pRaw, pRel, pb2;
   DWORD                   dws, dwr;
   PFLG2STGT               pf2st = g_pMchRel;   // P2 - get previous assignment
   PFLG2STG                pf2s;
   PIMAGE_SYMBOL           pSymTbl, pis;
   DWORD                   dwSymCnt;
   PMWL                    pmwl;
   BOOL                    bIsFunc;
   DWORD                   dwmax, dwmin;  // limit output (in some cases)

   pb2 = pHead - iOff;  // pointer to file beginning
   pmwl = 0;
   pis = 0;
   bIsFunc = FALSE;
   if( !IsListEmpty( &sSym2Sect ) )
   {
      PLE   pHead = &sSym2Sect;
      PLE   pNext;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->pSectHdr == psh )
         {
            pis     = pmwl->pSym;      // get SYMBOL TABLE entry for this section
            bIsFunc = pmwl->bIsFunc;   // and extract previous type checking
            break;
         }
      }
   }

   // Section NAME
   // NOTE: If .text, then is CODE
   strcpy(lpd, "Section Name:");
   SetMinLen(lpd,MINHDSZ);
   k = strlen(lpd);
   *lpname = 0;
   GetSectionName( lpname, psh, pStgs );
   strcat(lpd, lpname);

   *lpb = 0;
   if(pis)
   {
      GetSymName( lpb, pis, pStgs, FALSE );
      if( *lpb )
      {
         strcat(lpd, " (Sym:");
         strcat(lpd, lpb);
         strcat(lpd, ")");
      }
   }

#ifdef   ADDOFF1
   strcat(lpd, " ");
   sprintf(EndBuf(lpd), "(pHead=%#x)", (pHead - pb2));
#endif   // ADDOFF1
   AddStringandAdjust(lpd);

   dwo = psh->Misc.PhysicalAddress; 
   if(dwo)
   {
      strcpy(lpd, "Physical Address:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%#x", dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->Misc.VirtualSize; 
   if(dwo)
   {
      strcpy(lpd, "Virtual Size:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->VirtualAddress;
   strcpy(lpd, "Virtual Address:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%#x", dwo );
   AddStringandAdjust(lpd);

   dws = psh->SizeOfRawData;
   strcpy(lpd, "Size of Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dws, dws );
   AddStringandAdjust(lpd);

   //20 4 PointerToRawData File pointer to section's first page within the COFF 
   //file.
   dwi = psh->PointerToRawData;
   if( dwi )
      pb = pHead - iOff + dwi;
   else
      pb = 0;
   pRaw = pb;  // keep RAW DATA pointer

   strcpy(lpd, "Ptr to Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwr = psh->NumberOfRelocations;
   dwi = psh->PointerToRelocations;
   if(dwi)
      pRel = pHead - iOff + dwi;
   else
      pRel = 0;

   strcpy(lpd, "Ptr to Reloactions:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwo = psh->PointerToLinenumbers;
   strcpy(lpd, "Ptr to Line Numbers:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwo );
   AddStringandAdjust(lpd);

   strcpy(lpd, "Relocation Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwr, dwr );
   AddStringandAdjust(lpd);

   dwo = psh->NumberOfLinenumbers;
   strcpy(lpd, "Line Number Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);

   dwFlag = dwi = psh->Characteristics;
   strcpy(lpd, "Characteristics:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%08x", dwi );
   AddStringandAdjust(lpd);

   dwo = (dwi & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
   dwi &= ~(dwo);
   if(dwo)
   {
      while( dwo && pf2sa->pShort )
      {
         if( dwo == pf2sa->dwFlag )
         {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            strcat(lpd, pf2sa->pShort);
            AddStringandAdjust(lpd);
            dwo = 0;
            break;
         }
         pf2sa++;
      }
      if( dwo )
      {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            sprintf(EndBuf(lpd), "Note: %#x alignment!", dwo);
            AddStringandAdjust(lpd);
      }
   }
   else
   {
      *lpd = 0;
      SetMinLen(lpd,MINHDSZ);
      strcat(lpd, "Align 16 bytes (default)");
      AddStringandAdjust(lpd);
   }


   while( dwi && pf2s1->pShort )
   {
      //dwo = (dwi & pf2s1->dwFlag);
      if( dwi & pf2s1->dwFlag )
      {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         strcat(lpd, pf2s1->pShort);
         AddStringandAdjust(lpd);
         dwi &= ~(pf2s1->dwFlag);
      }
      pf2s1++;
   }
   if( dwi )
   {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         sprintf(EndBuf(lpd), "Note: %#X remains!", dwi );
         AddStringandAdjust(lpd);
   }

   if( pRaw && dws )
   {
      if( pis )   // do we have a SYMBOL pointer
      {
         *lpb = 0;
         GetSymName( lpb, pis, pStgs, FALSE );
         // This mean there is an associated LABEL
         // It is either a FUNCTION, if .text
         // or other program data
         //if( strcmp( lpname, ".text" ) == 0 )
         if( bIsFunc )
         {
            sprintf(lpd, "SECTION #%d (%#x) CODE for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) CODE DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            DumpCodeSection( pmwl, pb2, pStgs, pRaw, dws, lpfn );
         }
         else if( *lpb != '.' )
         {
            sprintf(lpd, "SECTION #%d (%#x) DATA for %s (Len=%d)",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);

            *lpd = 0;
            GetSymName( lpd, pis, pStgs, FALSE );

            if( *lpd != '_' )
               Write2ASMFile( szNul, 0 );  // add extra line to FILE only

            sprintf(EndBuf(lpd), " LABEL BYTE ; %d", dws);
            AddASMString( lpd, 0 );

            {
               BOOL  bs = FALSE;
               INT   i;
               pb = pRaw;
               dwo = dws;
               *lpb = 0;
               while( dwo-- )
               {
                  i = (INT)*pb;
                  if( ( i != '\'' ) && ( isprint(i) ) )
                  {
                     if(bs)   // if in string
                        sprintf(EndBuf(lpb), "%c", i );
                     else
                     {
                        if( *lpb )
                           strcat(lpb, ",");
                        sprintf(EndBuf(lpb), "'%c", i );
                        bs = TRUE;
                     }
                  }
                  else  // NOT printable, or is a "'" character
                  {
                     if(bs)
                     {
                        bs = FALSE;          // off the string
                        strcat(lpb, "',");   // close and add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                     else
                     {
                        if( *lpb )     // if previous
                           strcat(lpb, ","); // add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                  }
                  pb++;

                  if( strlen(lpb) > 65 )
                  {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

                     *lpb = 0;

                  }
               }  // while there is data

               if( *lpb )
               {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

               }
            }
         }
         else
         {
            sprintf(lpd, "SECTION #%d (%#x) OTHER for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            pb = pRaw;
            dwo = dws;
            while( dwo )
            {
               if( dwo > 16 )
                  dwi = 16;
               else
                  dwi = dwo;
               *lpd = 0;
               GetHEXString( lpd, pb, dwi, pb2, TRUE );
               AddStringandAdjust(lpd);
               pb  += dwi;    // bump the offset
               dwo -= dwi;    // reduce remaining count
            }
         }
      }
      else
      {
         i = 0;
         sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %s (Len=%d).",
            iNum, iNum, lpname, dws );
         //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
         //   iNum, iNum, dws, dws,
         //   (pRaw - pb2) );
         AddStringandAdjust(lpd);
         pb = pRaw;
         dwo = dws;  // this is the TOTAL size
         // FIX20061216 - put it ALL out if VERB9
         dwmax = 0; // start with NO LIMITS
         dwmin = 0;

         if( !VERB9 && (dwo > (MXOUTP * 2)) )
         {
            dwmin = MXOUTP;
            dwmax = dwo - MXOUTP;
         }
         while( dwo )
         {
            if( dwo > MX1LINE )
               dwi = MX1LINE;
            else
               dwi = dwo;
            if( dwmax )
            {
               if( (dwo >= dwmax) ||
                  (dwo <= dwmin)  )
                  *lpd = 0;
               else
                  *lpd = ' ';
            }
            else
               *lpd = 0;
            if( *lpd == 0 )
            {
               GetHEXString( lpd, pb, dwi, pb2, TRUE );
               AddStringandAdjust(lpd);
            }
            else if( i == 0 )
            {
               sprintf( lpd, "... *** Excluded %d bytes of DUMP data! *** ...",
                  (dws - (MXOUTP * 2)) );
               AddStringandAdjust(lpd);
               i++;
            }
            pb  += dwi;    // bump the offset
            dwo -= dwi;    // reduce remaining count
         }
      }

      if( dwFlag & IMAGE_SCN_LNK_INFO )
      {
         pb = pRaw;
         sprintf(lpd, "SECTION #%d (%#x) Linker Information (%d bytes).",
            iNum, iNum, dws );
         AddStringandAdjust(lpd);
         dwi = dws;
         *lpb = 0;
         dwo = 0;
         while(dwi)
         {
            if( *pb > ' ' )
               lpb[dwo++] = *pb;
            else if(dwo)
            {
               lpb[dwo] = 0;
               *lpd = 0;
               SetMinLen(lpd,MINHDSZ);
               strcat(lpd,lpb);
               AddStringandAdjust(lpd);
               dwo = 0;
            }
            pb++;
            dwi--;
         }
      }

   }

   // relocations
   if( dwr && pRel )
   {
      PIMAGE_RELOCATION pIRel = (PIMAGE_RELOCATION)pRel;
      sprintf(lpd, "SECTION #%d (%#x) RELOCATIONS for %d count.(p=%#x)",
         iNum, iNum, dwr,
         (pRel - pb2) );
      AddStringandAdjust(lpd);
      dwi = dwr;
      //Offset Index Type
//typedef struct _IMAGE_RELOCATION {
//    union {
//    DWORD   VirtualAddress;
//    DWORD   RelocCount; // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL
//    };
//    DWORD   SymbolTableIndex;
//    WORD    Type;
//} IMAGE_RELOCATION;
//typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;
// SymbolTableIndex - A zero-based index into the symbol table.
// This symbol gives the address to be used for the relocation.
// If the specified symbol has section storage class,
// then the symbol's address is the address with the first section
// of the same name. 
      //                 "12345678 12345678 12345678
      AddStringandAdjust("  Offset    Index Type     Name");
      while(dwi--)
      {
         sprintf(lpd, "%8x %8x ",
            pIRel->VirtualAddress,
            pIRel->SymbolTableIndex );
         dwo = pIRel->Type;
         pf2s = 0;
         if( pf2st )
            pf2s = pf2st->pRel;  // pf2st = m_pMchRel;   // get previous assignment
         if( pf2s )
         {
            AppendFStgEq( lpd, pf2s, dwo, TRUE );
            strcat(lpd," ");
         }
         else
         {
            sprintf(EndBuf(lpd), "NRT but Flag=%#x ", dwo );
         }

         dwo = pIRel->SymbolTableIndex;
         i = 0;
         if( g_pSymTbl && g_dwSymCnt && (dwo < g_dwSymCnt) )
         {
            pSymTbl  = (PIMAGE_SYMBOL)((LPTSTR)g_pSymTbl + dwo);
            dwSymCnt = g_dwSymCnt;
            *lpb = 0;
            i = GetSymName(lpb, pSymTbl, pStgs, FALSE );
            strcat(lpd, lpb);
         }
         else
         {
            if( !g_pSymTbl || !g_dwSymCnt )
            {
               strcat(lpd, "NO SymTbl or SymCnt?" );
            }
            else
            {
               sprintf(EndBuf(lpd), "Index %d into 0x%x of %d cnt ERRANT!",
                  dwo,
                  g_pSymTbl,
                  g_dwSymCnt );
            }
         }
         AddStringandAdjust(lpd);
         pIRel++;    // bump to NEXT
      }
   }

   // ***TBD*** Show the other items, linenumbers, etc

}

#ifndef  ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

VOID DumpObjSection(PBYTE pHead, INT iOff, INT iLen, INT iNum,
                              PBYTE pStgs, LPTSTR lpfn )
{
   LPTSTR                  lpd = &g_cBuf[0];
   LPTSTR                  lpb = &g_cBuf2[0];
   LPTSTR                  lpname = &g_cBuf3[0];
   PIMAGE_SECTION_HEADER   psh = (PIMAGE_SECTION_HEADER)pHead;
   INT                     i, k;
   //LPTSTR                  lps;
//   PFLG2STG1               pf2s1 = &sSecChars[0];
//   PFLG2STG1               pf2sa = &sSecAlign[0];
   PFLG2STG                pf2s1 = &sSectChars[0];
   PFLG2STG                pf2sa = &sSectAlign[0];
   DWORD                   dwi, dwo, dwFlag;
   PBYTE                   pb, pRaw, pRel, pb2;
   DWORD                   dws, dwr;
   PFLG2STGT               pf2st = (PFLG2STGT)g_pMchRel;   // P2 - get previous assignment
   PFLG2STG                pf2s;
   PIMAGE_SYMBOL           pSymTbl, pis;
   DWORD                   dwSymCnt;
   PMWL                    pmwl;
   BOOL                    bIsFunc;

   pb2 = pHead - iOff;  // pointer to file beginning
   pmwl = 0;
   pis = 0;
   bIsFunc = FALSE;
   if( !IsListEmpty( &sSym2Sect ) )
   {
      PLE   pHead = &sSym2Sect;
      PLE   pNext;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->pSectHdr == psh )
         {
            pis     = pmwl->pSym;      // get SYMBOL TABLE entry for this section
            bIsFunc = pmwl->bIsFunc;   // and extract previous type checking
            break;
         }
      }
   }

   // Section NAME
   // NOTE: If .text, then is CODE
   strcpy(lpd, "Section Name:");
   SetMinLen(lpd,MINHDSZ);
   k = strlen(lpd);
   *lpname = 0;
   GetSectionName( lpname, psh, pStgs );
   strcat(lpd, lpname);

   *lpb = 0;
   if(pis)
   {
      GetSymName( lpb, pis, pStgs, FALSE );
      if( *lpb )
      {
         strcat(lpd, " (Sym:");
         strcat(lpd, lpb);
         strcat(lpd, ")");
      }
   }

#ifdef   ADDOFF1
   strcat(lpd, " ");
   sprintf(EndBuf(lpd), "(pHead=%#x)", (pHead - pb2));
#endif   // ADDOFF1
   AddStringandAdjust(lpd);

   dwo = psh->Misc.PhysicalAddress; 
   if(dwo)
   {
      strcpy(lpd, "Physical Address:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%#x", dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->Misc.VirtualSize; 
   if(dwo)
   {
      strcpy(lpd, "Virtual Size:");
      SetMinLen(lpd,MINHDSZ);
      sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
      AddStringandAdjust(lpd);
   }

   dwo = psh->VirtualAddress;
   strcpy(lpd, "Virtual Address:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%#x", dwo );
   AddStringandAdjust(lpd);

   dws = psh->SizeOfRawData;
   strcpy(lpd, "Size of Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dws, dws );
   AddStringandAdjust(lpd);

   //20 4 PointerToRawData File pointer to section's first page within the COFF 
   //file.
   dwi = psh->PointerToRawData;
   if( dwi )
      pb = pHead - iOff + dwi;
   else
      pb = 0;
   pRaw = pb;  // keep RAW DATA pointer

   strcpy(lpd, "Ptr to Raw Data:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwr = psh->NumberOfRelocations;
   dwi = psh->PointerToRelocations;
   if(dwi)
      pRel = pHead - iOff + dwi;
   else
      pRel = 0;

   strcpy(lpd, "Ptr to Reloactions:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwi );
   AddStringandAdjust(lpd);

   dwo = psh->PointerToLinenumbers;
   strcpy(lpd, "Ptr to Line Numbers:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%x", dwo );
   AddStringandAdjust(lpd);

   strcpy(lpd, "Relocation Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwr, dwr );
   AddStringandAdjust(lpd);

   dwo = psh->NumberOfLinenumbers;
   strcpy(lpd, "Line Number Count:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "%d (0x%x)", dwo, dwo );
   AddStringandAdjust(lpd);

   dwFlag = dwi = psh->Characteristics;
   strcpy(lpd, "Characteristics:");
   SetMinLen(lpd,MINHDSZ);
   sprintf(EndBuf(lpd), "0x%08x", dwi );
   AddStringandAdjust(lpd);

   dwo = (dwi & IMAGE_SCN_ALIGN_MASK);   //  0x00F00000
   dwi &= ~(dwo);
   if(dwo)
   {
      while( dwo && pf2sa->pShort )
      {
         if( dwo == pf2sa->dwFlag )
         {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            strcat(lpd, pf2sa->pShort);
            AddStringandAdjust(lpd);
            dwo = 0;
            break;
         }
         pf2sa++;
      }
      if( dwo )
      {
            *lpd = 0;
            SetMinLen(lpd,MINHDSZ);
            sprintf(EndBuf(lpd), "Note: %#x alignment!", dwo);
            AddStringandAdjust(lpd);
      }
   }
   else
   {
      *lpd = 0;
      SetMinLen(lpd,MINHDSZ);
      strcat(lpd, "Align 16 bytes (default)");
      AddStringandAdjust(lpd);
   }


   while( dwi && pf2s1->pShort )
   {
      //dwo = (dwi & pf2s1->dwFlag);
      if( dwi & pf2s1->dwFlag )
      {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         strcat(lpd, pf2s1->pShort);
         AddStringandAdjust(lpd);
         dwi &= ~(pf2s1->dwFlag);
      }
      pf2s1++;
   }
   if( dwi )
   {
         *lpd = 0;
         SetMinLen(lpd,MINHDSZ);
         sprintf(EndBuf(lpd), "Note: %#X remains!", dwi );
         AddStringandAdjust(lpd);
   }

   if( pRaw && dws )
   {
      if( pis )   // do we have a SYMBOL pointer
      {
         *lpb = 0;
         GetSymName( lpb, pis, pStgs, FALSE );
         // This mean there is an associated LABEL
         // It is either a FUNCTION, if .text
         // or other program data
         //if( strcmp( lpname, ".text" ) == 0 )
         if( bIsFunc )
         {
            sprintf(lpd, "SECTION #%d (%#x) CODE for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) CODE DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            DumpCodeSection( pmwl, pb2, pStgs, pRaw, dws, lpfn );
         }
         else if( *lpb != '.' )
         {
            sprintf(lpd, "SECTION #%d (%#x) DATA for %s (Len=%d)",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);

            *lpd = 0;
            GetSymName( lpd, pis, pStgs, FALSE );

            if( *lpd != '_' )
               Write2ASMFile( szNul, 0 );  // add extra line to FILE only

            sprintf(EndBuf(lpd), " LABEL BYTE ; %d", dws);
            AddASMString( lpd, 0 );

            {
               BOOL  bs = FALSE;
               INT   i;
               pb = pRaw;
               dwo = dws;
               *lpb = 0;
               while( dwo-- )
               {
                  i = (INT)*pb;
                  if( ( i != '\'' ) && ( isprint(i) ) )
                  {
                     if(bs)   // if in string
                        sprintf(EndBuf(lpb), "%c", i );
                     else
                     {
                        if( *lpb )
                           strcat(lpb, ",");
                        sprintf(EndBuf(lpb), "'%c", i );
                        bs = TRUE;
                     }
                  }
                  else  // NOT printable, or is a "'" character
                  {
                     if(bs)
                     {
                        bs = FALSE;          // off the string
                        strcat(lpb, "',");   // close and add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                     else
                     {
                        if( *lpb )     // if previous
                           strcat(lpb, ","); // add comma
                        if( i == 0 )
                           strcat(lpb,"0");
                        else if( i & 0x0f0 )
                           sprintf(EndBuf(lpb), "0x0%02x", i ); // and the hex
                        else
                           sprintf(EndBuf(lpb), "0x%02x", i ); // and the hex
                     }
                  }
                  pb++;

                  if( strlen(lpb) > 65 )
                  {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

                     *lpb = 0;

                  }
               }  // while there is data

               if( *lpb )
               {
                     if(bs)
                        strcat(lpb, "'");   // close
                     bs = FALSE;

                     sprintf( lpd, "DB %s", lpb );

                     AddASMString( lpd, 0 );

               }
            }
         }
         else
         {
            sprintf(lpd, "SECTION #%d (%#x) OTHER for %s (Len=%d).",
               iNum, iNum, lpb, dws );
            //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
            //   iNum, iNum, dws, dws,
            //   (pRaw - pb2) );
            AddStringandAdjust(lpd);
            pb = pRaw;
            dwo = dws;
            while( dwo )
            {
               if( dwo > 16 )
                  dwi = 16;
               else
                  dwi = dwo;
               *lpd = 0;
               GetHEXString( lpd, pb, dwi, pb2, TRUE );
               AddStringandAdjust(lpd);
               pb  += dwi;    // bump the offset
               dwo -= dwi;    // reduce remaining count
            }
         }
      }
      else
      {
         sprintf(lpd, "SECTION #%d (%#x) RAW DATA for %s (Len=%d).",
            iNum, iNum, lpname, dws );
         //sprintf(lpd, "SECTION #%d (%#x) OTHER DATA for %d (%#x) p=%#x.",
         //   iNum, iNum, dws, dws,
         //   (pRaw - pb2) );
         AddStringandAdjust(lpd);
         pb = pRaw;
         dwo = dws;
         while( dwo )
         {
            if( dwo > 16 )
               dwi = 16;
            else
               dwi = dwo;
            *lpd = 0;
            GetHEXString( lpd, pb, dwi, pb2, TRUE );
            AddStringandAdjust(lpd);
            pb  += dwi;    // bump the offset
            dwo -= dwi;    // reduce remaining count
         }
      }

      if( dwFlag & IMAGE_SCN_LNK_INFO )
      {
         pb = pRaw;
         sprintf(lpd, "SECTION #%d (%#x) Linker Information (%d bytes).",
            iNum, iNum, dws );
         AddStringandAdjust(lpd);
         dwi = dws;
         *lpb = 0;
         dwo = 0;
         while(dwi)
         {
            if( *pb > ' ' )
               lpb[dwo++] = *pb;
            else if(dwo)
            {
               lpb[dwo] = 0;
               *lpd = 0;
               SetMinLen(lpd,MINHDSZ);
               strcat(lpd,lpb);
               AddStringandAdjust(lpd);
               dwo = 0;
            }
            pb++;
            dwi--;
         }
      }

   }

   // relocations
   if( dwr && pRel )
   {
      PIMAGE_RELOCATION pIRel = (PIMAGE_RELOCATION)pRel;
      sprintf(lpd, "SECTION #%d (%#x) RELOCATIONS for %d count.(p=%#x)",
         iNum, iNum, dwr,
         (pRel - pb2) );
      AddStringandAdjust(lpd);
      dwi = dwr;
      //Offset Index Type
//typedef struct _IMAGE_RELOCATION {
//    union {
//    DWORD   VirtualAddress;
//    DWORD   RelocCount; // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL
//    };
//    DWORD   SymbolTableIndex;
//    WORD    Type;
//} IMAGE_RELOCATION;
//typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;
// SymbolTableIndex - A zero-based index into the symbol table.
// This symbol gives the address to be used for the relocation.
// If the specified symbol has section storage class,
// then the symbol's address is the address with the first section
// of the same name. 
      //                 "12345678 12345678 12345678
      AddStringandAdjust("  Offset    Index Type     Name");
      while(dwi--)
      {
         sprintf(lpd, "%8x %8x ",
            pIRel->VirtualAddress,
            pIRel->SymbolTableIndex );
         dwo = pIRel->Type;
         pf2s = 0;
         if( pf2st )
            pf2s = pf2st->pRel;  // pf2st = m_pMchRel;   // get previous assignment
         if( pf2s )
         {
            AppendFStgEq( lpd, pf2s, dwo, TRUE );
            strcat(lpd," ");
         }
         else
         {
            sprintf(EndBuf(lpd), "NRT but Flag=%#x ", dwo );
         }

         dwo = pIRel->SymbolTableIndex;
         i = 0;
         if( g_pSymTbl && g_dwSymCnt && (dwo < g_dwSymCnt) )
         {
            pSymTbl  = (PIMAGE_SYMBOL)((PIMAGE_SYMBOL)g_pSymTbl + dwo);
            dwSymCnt = g_dwSymCnt;
            *lpb = 0;
            i = GetSymName(lpb, pSymTbl, pStgs, FALSE );
            strcat(lpd, lpb);
         }
         else
         {
            if( !g_pSymTbl || !g_dwSymCnt )
            {
               strcat(lpd, "NO SymTbl or SymCnt?" );
            }
            else
            {
               sprintf(EndBuf(lpd), "Index %d into 0x%x of %d cnt ERRANT!",
                  dwo,
                  g_pSymTbl,
                  g_dwSymCnt );
            }
         }
         AddStringandAdjust(lpd);
         pIRel++;    // bump to NEXT
      }
   }

   // ***TBD*** Show the other items, linenumbers, etc

}

#endif   // #ifndef  ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

#endif // #ifndef USE_PEDUMP_CODE // FIX20080507
// ===============================================================

