
// Headers.h
#ifndef  _Headers_HH
#define  _Headers_HH

/*
#define OLDSIG          0x5a4d
#define NEWSIG          0x454e
#define SINGLEDATA      0x0001
#define MULTIPLEDATA    0x0002
#define PMODEONLY       0x0008
#define LIBRARY         0x8000
#define FASTLOAD        0x0008

*/

typedef struct
{
    BYTE    bFlags;
    WORD    wSegOffset;
} FENTRY, *PFENTRY;

typedef struct
{
    BYTE    bFlags;
    WORD    wINT3F;
    BYTE    bSegNumber;
    WORD    wSegOffset;
} MENTRY, *PMENTRY;

#define EXPORTED    0x01
#define SHAREDDATA  0x02


typedef struct
{
    WORD    wSector;
    WORD    wLength;
    WORD    wFlags;
    WORD    wMinAlloc;
} SEGENTRY, *PSEGENTRY;

#define F_DATASEG       0x0001
#define F_MOVEABLE      0x0010
#define F_SHAREABLE     0x0020
#define F_PRELOAD       0x0040
#define F_DISCARDABLE   0x1000

// The RTYPE and RINFO structures are never actually used
// they are just defined for use in the sizeof() macro when
// reading the info off the disk.  The actual data is read
// into the RESTYPE and RESINFO structures that contain these
// structures with some extra information declared at the end.

typedef struct
{
    WORD    wType;
    WORD    wCount;
    LONG    lReserved;
} RTYPE;

typedef struct
{
    WORD    wOffset;
    WORD    wLength;
    WORD    wFlags;
    WORD    wID;
    LONG    lReserved;
} RINFO;

// RESINFO2 is the same structure as RINFO with one modification.
// RESINFO2 structure uses the lower 16 bits of the lReserved from
// RINFO structure to point to a string that represents
// the resource name.  This can be done since the lReserved piece
// of this structure is used for Run-time data.  This use of the
// lReserved portion is done so that all resources of a certain
// type can be read into one allocated array, thus using 1 ALLOC
// and 1 read.  This saves memory and makes the loading faster
// so it's worth the slight confusion that might be introduced.
/*
typedef struct
{
    WORD     wOffset;
    WORD     wLength;
    WORD     wFlags;
    WORD     wID;
    PSTR     pResourceName;
    WORD     wReserved;
} RESINFO2, *PRESINFO;

extern struct tgRESTYPE;
typedef struct tgRESTYPE *PRESTYPE;

typedef struct tgRESTYPE
{
    WORD        wType;              // Resource type
    WORD        wCount;             // Specifies ResInfoArray size
    LONG        lReserved;          // Reserved for runtime use
    PSTR        pResourceType;      // Points to custom type name
    PRESINFO    pResInfoArray;      // First entry in array
    PRESTYPE    pNext;              // Next Resource type
} RESTYPE;

#define GROUP_CURSOR    12
#define GROUP_ICON      14
#define NAMETABLE       15



typedef struct tgNAME
{
    struct tgNAME  *pNext;
    WORD            wOrdinal;
    char            szName[1];      // Text goes here at allocation time
} NAME, *PNAME;

*/


// the following data is extracted from the samples on the PE format

#define SIZE_OF_NT_SIGNATURE	sizeof (DWORD)

/* global macros to define header offsets into file */
/* offset to PE file signature				       */
#define NTSIGNATURE(a) ((LPVOID)((BYTE *)a		     +	\
			((PIMAGE_DOS_HEADER)a)->e_lfanew))

/* DOS header identifies the NT PEFile signature dword
   the PEFILE header exists just after that dword	       */
#define PEFHDROFFSET(a) ((LPVOID)((BYTE *)a		     +	\
			 ((PIMAGE_DOS_HEADER)a)->e_lfanew    +	\
			 SIZE_OF_NT_SIGNATURE))

/* PE optional header is immediately after PEFile header       */
#define OPTHDROFFSET(a) ((LPVOID)((BYTE *)a		     +	\
			 ((PIMAGE_DOS_HEADER)a)->e_lfanew    +	\
			 SIZE_OF_NT_SIGNATURE		     +	\
			 sizeof (IMAGE_FILE_HEADER)))

/* section headers are immediately after PE optional header    */
#define SECHDROFFSET(a) ((LPVOID)((BYTE *)a		     +	\
			 ((PIMAGE_DOS_HEADER)a)->e_lfanew    +	\
			 SIZE_OF_NT_SIGNATURE		     +	\
			 sizeof (IMAGE_FILE_HEADER)	     +	\
			 sizeof (IMAGE_OPTIONAL_HEADER)))


#endif   // #ifndef  _Headers_HH
// eof - Headers.h
