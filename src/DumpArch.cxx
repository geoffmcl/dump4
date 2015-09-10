// DumpArch.cxx

#include "Dump4.h"
#include "DumpArch.h"
#include <Dbghelp.h>    // for UnDecorateSymbolName()

#define DBGPRT  prt

//#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
#define MakePtr( cast, ptr, addValue ) (cast)( (char *)(ptr) + (DWORD)(addValue))
static PSTR PszLongnames = 0;

static char _s_buffer[256];

char * getArchiveMemberHeader( PIMAGE_ARCHIVE_MEMBER_HEADER pArchHeader,
                               DWORD fileOffset )
{
    char * cp = GetNxtBuf();
    sprintf(cp,"Archive Member Header: At offset %d (%08X)\n", fileOffset, fileOffset);

    sprintf(EndBuf(cp), "  Name:     %.16s", pArchHeader->Name);
    if ( pArchHeader->Name[0] == '/' )
    {
       if( isdigit(pArchHeader->Name[1]) )
       {
           int value = atoi((char *)&pArchHeader->Name[1]);
           if( PszLongnames ) {
               char *pName = PszLongnames + value;
               if (out_of_top_range((unsigned char *)pName)) {
                   sprintf(EndBuf(cp), "  (Offset %d into PszLongnames out of range)", value);
               } else if (*pName) {
                   sprintf(EndBuf(cp), "  (%s)", pName );
               } else {
                   sprintf(EndBuf(cp), "  (Offset %d into PszLongnames)", value);
               }
           } else {
             sprintf(EndBuf(cp), "  (Offset %d into PszLongnames(0))", value);
           }
       } else {
          sprintf(EndBuf(cp), "  (no name, and no offset!)" );
       }
    }

    strcat(cp,"\n");

	char szDateAsLong[64];
	sprintf( szDateAsLong, "%.12s", pArchHeader->Date );
	LONG dateAsLong = atol(szDateAsLong);
	
    sprintf(EndBuf(cp),"  Date:     %.12s %s", pArchHeader->Date, pedump_ctime((time_t *)&dateAsLong) );
    sprintf(EndBuf(cp),"  UserID:   %.6s\n", pArchHeader->UserID);
    sprintf(EndBuf(cp),"  GroupID:  %.6s\n", pArchHeader->GroupID);
    sprintf(EndBuf(cp),"  Mode:     %.8s\n", pArchHeader->Mode);
    sprintf(EndBuf(cp),"  Size:     %.10s\n", pArchHeader->Size);
    return cp;
}

char *getFirstLinkerMember(PVOID p)
{
    char *cp = GetNxtBuf();
    size_t maxbuf = MXLINEB;
    size_t len;
    DWORD offset;
    DWORD cSymbols = *(PDWORD)p;
    PDWORD pMemberOffsets = MakePtr( PDWORD, p, 4 );
    PSTR pSymbolName;
    unsigned i;

    cSymbols = ConvertBigEndian(cSymbols);
    pSymbolName = MakePtr( PSTR, pMemberOffsets, 4 * cSymbols );
    
    sprintf(cp,"Arch First Linker Member:\n");
    sprintf(EndBuf(cp), "  Symbols:  total count %d\n", cSymbols );
    sprintf(EndBuf(cp), "  MbrOffs   Name\n  --------  ----\n" );

    for ( i = 0; i < cSymbols; i++ )
    {
        if (strlen(cp) > maxbuf) {
            return cp;
        }
        offset = ConvertBigEndian( *pMemberOffsets );        
        len = strlen(pSymbolName);
        sprintf(EndBuf(cp),"  %08X  %s", offset, pSymbolName);
        if (VERB2 && (len > 2) && (*pSymbolName == '?')
            && !(pSymbolName[1] == '?')) {
            char * bb = get_my_big_buf();
            DWORD res = UnDecorateSymbolName( pSymbolName,
                bb, MX_BIG_BUF, 0 );
            if ((res > 0) && !(*bb == '?')) {
                sprintf(EndBuf(cp)," [%s]", bb);
            }
        }
        sprintf(EndBuf(cp),"\n");
        pMemberOffsets++;
        pSymbolName += strlen(pSymbolName) + 1;
    }
    return cp;
}

char *getSecondLinkerMember(PVOID p)
{
    DWORD cArchiveMembers = *(PDWORD)p;
    PDWORD pMemberOffsets = MakePtr( PDWORD, p, 4 );
    DWORD cSymbols;
    PSTR pSymbolName;
    PWORD pIndices;
    unsigned i;
    char *cp = GetNxtBuf();

    cArchiveMembers = cArchiveMembers;

    // The number of symbols is in the DWORD right past the end of the
    // member offset array.
    cSymbols = pMemberOffsets[cArchiveMembers];

    pIndices = MakePtr( PWORD, p, 4 + cArchiveMembers * sizeof(DWORD) + 4 );

    pSymbolName = MakePtr( PSTR, pIndices, cSymbols * sizeof(WORD) );
    
    //if ( fDumpExportsOnly )
    //    return;

    sprintf(cp,"Arch Second Linker Member:\n");
    
    sprintf(EndBuf(cp), "  Archive Members: %08X\n", cArchiveMembers );
    sprintf(EndBuf(cp), "  Symbols: total count %d\n", cSymbols );
    sprintf(EndBuf(cp), "  MbrOffs   Name\n  --------  ----\n" );

    for ( i = 0; i < cSymbols; i++ )
    {
        if (strlen(cp) > MXLINEB)
            return cp;

        sprintf(EndBuf(cp),"  %08X  %s\n", pMemberOffsets[pIndices[i] - 1], pSymbolName);
        pSymbolName += strlen(pSymbolName) + 1;
    }
    return cp;
}

char * getLongnamesMember(PVOID p, DWORD len)
{
    char *cp = GetNxtBuf();
    PSTR pszName = (PSTR)p;
    DWORD offset = 0;

    PszLongnames = (PSTR)p;     // Save off pointer for use when dumping
                                // out OBJ member names

    //if ( fDumpExportsOnly )
    //    return;

    sprintf(cp,"Longnames:\n");
    
    // The longnames member is a series of null-terminated string.  Print
    // out the offset of each string (in decimal), followed by the string.
    while ( offset < len )
    {

        unsigned cbString = lstrlen( pszName )+1;
        if ((strlen(cp)+cbString) > MXLINEB)
            return cp;
        sprintf(EndBuf(cp),"  %05u: %s\n", offset, pszName);
        offset += cbString;
        pszName += cbString;
    }
    return cp;
}

char *getHeader(PIMAGE_FILE_HEADER pImageFileHeader)
{
   
   UINT headerFieldWidth = 30;
   UINT i;
   char *cp = _s_buffer;    // GetNxtBuf();
   *cp = 0;
   //if( fDumpFileHeader )
   //{
      sprintf(EndBuf(cp), "Arch File Header:\n");

      sprintf(EndBuf(cp),"  %-*s%04X (%s)\n", headerFieldWidth, "Machine:", 
                pImageFileHeader->Machine,
                GetMachineTypeName(pImageFileHeader->Machine) );
      sprintf(EndBuf(cp),"  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
                pImageFileHeader->NumberOfSections);
      sprintf(EndBuf(cp),"  %-*s%08X -> %s", headerFieldWidth, "TimeDateStamp:",
                pImageFileHeader->TimeDateStamp,
                pedump_ctime((time_t *)&pImageFileHeader->TimeDateStamp));
      sprintf(EndBuf(cp),"  %-*s%08X\n", headerFieldWidth, "PointerToSymbolTable:",
                pImageFileHeader->PointerToSymbolTable);
      sprintf(EndBuf(cp),"  %-*s%08X\n", headerFieldWidth, "NumberOfSymbols:",
                pImageFileHeader->NumberOfSymbols);
      sprintf(EndBuf(cp),"  %-*s%04X\n", headerFieldWidth, "SizeOfOptionalHeader:",
                pImageFileHeader->SizeOfOptionalHeader);
      sprintf(EndBuf(cp),"  %-*s%04X\n", headerFieldWidth, "Characteristics:",
                pImageFileHeader->Characteristics);
      PWORD_FLAG_DESCRIPTIONS pwfd = getImageFlagStruct();
      UINT max = getImageFlagCount();
      for ( i = 0; i < max; i++ )
      {
        if ( pImageFileHeader->Characteristics & pwfd[i].flag )
            sprintf(EndBuf(cp), "    %s\n", pwfd[i].name );
      }
      //sprintf(EndBuf(cp),"\n");
   //}
    return cp;
}

int isSectionNVBlank( PIMAGE_SECTION_HEADER section )
{
   if(( section->Name[0] == 0 )&&
      ( section->Misc.VirtualSize == 0 )&&
      ( section->VirtualAddress == 0 ))
   {
      return 1;
   }
   return 0;
}

char *getSectionTable(UINT i, PIMAGE_SECTION_HEADER section)
{
    static char _s_buf[256];
    char *cp = _s_buf;
    BOOL IsEXE = FALSE;
    sprintf(cp, "  %02X %-8.8s  %s: %08X  VirtAddr:  %08X\n",
                   i, section->Name,
                   IsEXE ? "VirtSize" : "PhysAddr",
                   section->Misc.VirtualSize, section->VirtualAddress);
    sprintf(EndBuf(cp), "    raw data offs:   %08X  raw data size: %08X\n",
                   section->PointerToRawData, section->SizeOfRawData );
    sprintf(EndBuf(cp), "    relocation offs: %08X  relocations:   %08X\n",
                   section->PointerToRelocations, section->NumberOfRelocations );
    sprintf(EndBuf(cp), "    line # offs:     %08X  line #'s:      %08X\n",
                   section->PointerToLinenumbers, section->NumberOfLinenumbers );
    sprintf(EndBuf(cp), "    characteristics: %08X\n", section->Characteristics);

    return cp;
}

char * getObjFile( PIMAGE_FILE_HEADER pImageFileHeader )
{
    char *cp = GetNxtBuf();
    unsigned int i;
    unsigned int max;
    WORD    wmax;
    PIMAGE_SECTION_HEADER pSections;
    PIMAGE_SECTION_HEADER section;
    int out_of_range = 0;
    char *cp2;

    *cp = 0;
    wmax = pImageFileHeader->NumberOfSections;
    max = wmax;
    cp2 = getHeader(pImageFileHeader);
    DBGPRT(cp2);
    if (wmax == 0xffff) { // FIX20120208 - Skip if count 0xFFFF in object
        sprtf("Section count out of range! Skipping...(0x%X)\n",max);
        return cp;
    }
    // move up to 'sections'
    pSections = MakePtr(PIMAGE_SECTION_HEADER, (pImageFileHeader+1),
                            pImageFileHeader->SizeOfOptionalHeader);

    for ( i = 0; i < max; i++ ) {
        section = &pSections[i];
        if (out_of_top_range((unsigned char *)(section+1))) {
             out_of_range = 1;
             break;
        }
        if (isSectionNVBlank( section )) {
            break;
        }
        cp2 = getSectionTable((i+1),section);
        DBGPRT(cp2);
    }

    return cp;
}

int is_archive_format(unsigned char *base, int size, char * name)
{
    PIMAGE_ARCHIVE_MEMBER_HEADER pah;
    DWORD thisMemberSize, lnSize;
    BOOL fBreak = FALSE;
    char *msg;
    int linkmemcnt = 0;
    char *cp;
    DWORD   dwFileOffset;
    unsigned char*end;
    int iret = 0;
    if (size > sizeof(ARCH)) {
        if ( strncmp((char *)base,IMAGE_ARCHIVE_START,IMAGE_ARCHIVE_START_SIZE ) == 0)
        {
            pah = (PIMAGE_ARCHIVE_MEMBER_HEADER)base;
            pah = MakePtr(PIMAGE_ARCHIVE_MEMBER_HEADER, base, IMAGE_ARCHIVE_START_SIZE);
            while (pah) {
                iret = 1;
                dwFileOffset = (PBYTE)pah - (PBYTE) base;
                // Calculate how big this member is (it's originally stored as 
                // as ASCII string.
                thisMemberSize = atoi((char *)pah->Size)
                        + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;
                thisMemberSize = (thisMemberSize+1) & ~1;   // Round up
                end = base + thisMemberSize;
                if (out_of_top_range(end)) {
                    sprtf("Base plus size %d, out of range!\n",thisMemberSize);
                    break;
                }
                cp = getArchiveMemberHeader(pah,dwFileOffset);
                DBGPRT(cp);
                if ( !strncmp( 	(char *)pah->Name,
        				IMAGE_ARCHIVE_LINKER_MEMBER, 16) )
                {
                    // got linker member
                    msg = "linker member";
                    if (linkmemcnt == 0)
                        cp = getFirstLinkerMember((PVOID)(pah + 1));
                    else if (linkmemcnt == 1)
                        cp = getSecondLinkerMember((PVOID)(pah + 1));
                    else {
                        msg = "WHAT IS THIS?";
                    }
                    linkmemcnt++;
                }
                else if( !strncmp(	(char *)pah->Name,
        					IMAGE_ARCHIVE_LONGNAMES_MEMBER, 16) )
                {
                    // long names member
                    msg = "long names member";
                    lnSize = atoi((char *)pah->Size);
                    cp = getLongnamesMember( (PVOID)(pah + 1),
                                 lnSize );
                }
                else
                {
                    // an object
                    msg = "object member";
                    cp = getObjFile(  (PIMAGE_FILE_HEADER)(pah + 1) );

                }
                // Get a pointer to the next archive member
                pah = MakePtr(PIMAGE_ARCHIVE_MEMBER_HEADER, pah,
                                thisMemberSize);
                // Bail out if we don't see the EndHeader signature in the next record
                __try
                {
                    if (strncmp( (char *)pah->EndHeader, IMAGE_ARCHIVE_END, 2))
                        break;
                    else
                        msg = "no archive end yet";
                }
                __except( TRUE )    // Should only get here if pArchHeader is bogus
                {
                    fBreak = TRUE;  // Ideally, we could just put a "break;" here,
                                    // but BC++ doesn't like it.
                    msg = "BAD end";
                    iret = 0;
                }
        
                if ( fBreak )   // work around BC++ problem.
                    break;

            }
        }
    }
    return iret;
}

