
// DumpOS2.c

#include "dump4.h"

#ifndef USE_PEDUMP_CODE // FIX20080507
/* ==============================================
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

   ============================================== */
#define  SHOS2(a,b,c) Show_OS2_Entry( a, &pos2->b, c );

// "Offset of Non-resident Names Table"
//  123456789012345678901234567890123456
#define  MMIN_STR 36


VOID  Show_OS2_Entry( PTSTR ptype, PVOID pv, PTSTR pdesc )
{
   PTSTR lpd = g_cBuf;
   DWORD dwv = 0;

   strcpy(lpd,pdesc);
   strcat(lpd,":");
   while(strlen(lpd) < MMIN_STR) strcat(lpd," ");
   if( strcmp(ptype, "WORD") == 0 ) {
      PWORD pw = (PWORD)pv;
      dwv = (*pw & 0xffff);
   } else if( strcmp(ptype, "CHAR") == 0 ) {
      PCHAR pc = (PCHAR)pv;
      dwv = (*pc & 0xff);
   } else if( strcmp(ptype, "LONG") == 0 ) {
      PLONG pl = (PLONG)pv;
      dwv = *pl;
   } else if( strcmp(ptype, "BYTE") == 0 ) {
      PBYTE pb = (PBYTE)pv;
      dwv = (*pb & 0xff);
   } else {
      sprintf(EndBuf(lpd), "ERROR: Uncased output for ", ptype );
   }
   sprintf(EndBuf(lpd), "%u", dwv );
   sprintf(EndBuf(lpd), " (%#x)", dwv );
   sprintf(EndBuf(lpd), " [%s]", ptype);
   sprtf( "%s"MEOR, lpd );
}

VOID  Show_OS2_Header( PIMAGE_OS2_HEADER pos2 )
{
   //SHOS2( "WORD", ne_magic,  );
   PBYTE pb = Word2Ascii((PBYTE)pos2);
   DWORD dwv = 0;
   PTSTR lpd = g_cBuf;
   strcpy(lpd,"Magic number");
   strcat(lpd,":");
   while(strlen(lpd) < MMIN_STR) strcat(lpd," ");
   dwv = (pos2->ne_magic & 0xffff);
   sprintf(EndBuf(lpd), "%u", dwv );
   sprintf(EndBuf(lpd), " (%#x)", dwv );
   sprintf(EndBuf(lpd), " [%s]", pb );
   sprintf(EndBuf(lpd), " [%s]", "WORD");
   sprtf( "%s"MEOR, lpd );

   SHOS2( "CHAR", ne_ver, "Version number" );
   SHOS2( "CHAR", ne_rev, "Revision number" );
   SHOS2( "WORD", ne_enttab, "Offset of Entry Table" );
   SHOS2( "WORD", ne_cbenttab, "Number of bytes in Entry Table" );
   SHOS2( "LONG", ne_crc, "Checksum of whole file" );
   SHOS2( "WORD", ne_flags, "Flag word" );
   SHOS2( "WORD", ne_autodata, "Automatic data segment number" );
   SHOS2( "WORD", ne_heap, "Initial heap allocation" );
   SHOS2( "WORD", ne_stack, "Initial stack allocation" );
   SHOS2( "LONG", ne_csip, "Initial CS:IP setting" );
   SHOS2( "LONG", ne_sssp, "Initial SS:SP setting" );
   SHOS2( "WORD", ne_cseg, "Count of file segments" );
   SHOS2( "WORD", ne_cmod, "Entries in Module Reference Table" );
   SHOS2( "WORD", ne_cbnrestab, "Size of non-resident name table" );
   SHOS2( "WORD", ne_segtab, "Offset of Segment Table" );
   SHOS2( "WORD", ne_rsrctab, "Offset of Resource Table" );
   SHOS2( "WORD", ne_restab, "Offset of resident name table" );
   SHOS2( "WORD", ne_modtab, "Offset of Module Reference Table" );
   SHOS2( "WORD", ne_imptab, "Offset of Imported Names Table" );
   SHOS2( "LONG", ne_nrestab,    "Offset of Non-resident Names Table" );
   SHOS2( "WORD", ne_cmovent, "Count of movable entries" );
   SHOS2( "WORD", ne_align, "Segment alignment shift count" );
   SHOS2( "WORD", ne_cres, "Count of resource segments" );
   SHOS2( "BYTE", ne_exetyp, "Target Operating system" );
   SHOS2( "BYTE", ne_flagsothers, "Other .EXE flags" );
   SHOS2( "WORD", ne_pretthunks, "offset to return thunks" );
   SHOS2( "WORD", ne_psegrefbytes, "offset to segment ref. bytes" );
   SHOS2( "WORD", ne_swaparea, "Minimum code swap area size" );
   SHOS2( "WORD", ne_expver, "Expected Windows version number" );
}

//void CNEView::FillInEntryTable(unsigned char *lpEntryTable)
PBYTE _FillInEntryTable(PBYTE lpEntryTable, PBYTE pbase)
{
   PTSTR m_szBuf = g_cBuf;
	WORD  wIndex=1;
	WORD  wI;
	BYTE  bBundles;
	BYTE  bFlags;
   PBYTE tb = lpEntryTable;
   DWORD offset = (lpEntryTable - pbase);

   sprintf(m_szBuf, "Entry table: beginning offset %u (%#x)", offset, lpEntryTable );
	AddStringandAdjust(m_szBuf);

	sprintf(m_szBuf,"Dummy");

	while( TRUE )
	{
		bBundles = (BYTE)*lpEntryTable++;
		if( bBundles == 0 )
			break;    // End of the table
		bFlags = (BYTE)*lpEntryTable++;
		switch( bFlags )
		{
		case 0x00:      // Placeholders
			if( bBundles == 1 )
				sprintf( m_szBuf, "%d Placeholder", wIndex );
			else
				sprintf( m_szBuf, "%d-%d Placeholders", wIndex, wIndex + bBundles - 1 );

			AddStringandAdjust(m_szBuf);

			wIndex += bBundles;
			break;

		case 0xFF:      // MOVEABLE segments
         sprintf( m_szBuf, "Set of MOVEABLE Segments = %u ..."MEOR, (bBundles & 0xff));
			AddStringandAdjust( m_szBuf );
			for( wI=0; wI < (WORD)bBundles; wI++ )
			{
				PMENTRY pe = (PMENTRY)lpEntryTable;
				WORD    wS = ((WORD)pe->bFlags)>>2;

				sprintf( m_szBuf,
					"%d %#04x     %#04x   %#04x        MOVEABLE  ",
					wIndex,
					pe->bSegNumber,
					pe->wSegOffset,
					wS );

				if( pe->bFlags & EXPORTED )
					strcat( m_szBuf, "EXPORTED " );
				else
					strcat( m_szBuf, "         " );

				if( pe->bFlags & SHAREDDATA )
					strcat( m_szBuf, "SHARED_DATA" );

				AddStringandAdjust( m_szBuf );

				wIndex++;
				lpEntryTable += sizeof( MENTRY );
			}
			break;

		default:        // FIXED Segments
         sprintf( m_szBuf, "Set of FIXED Segments = %u ..."MEOR, (bBundles & 0xff));
			AddStringandAdjust( m_szBuf );
			for( wI = 0; wI < (WORD)bBundles; wI++ )
			{
				PFENTRY pe = (PFENTRY)lpEntryTable;
				WORD    wS = ((WORD)pe->bFlags)>>2;

				sprintf( m_szBuf,
					"%d %#04x     %#04x   %#04x        FIXED     ",
					wIndex,
					bFlags,
					pe->wSegOffset,
					wS );
				if( pe->bFlags & EXPORTED )
					strcat( m_szBuf, "EXPORTED  " );
				else
					strcat( m_szBuf, "         " );

				if( pe->bFlags & SHAREDDATA )
					strcat( m_szBuf, "SHARED_DATA" );

				AddStringandAdjust(m_szBuf);

				wIndex++;

				lpEntryTable += sizeof( FENTRY );

			}
			break;

		}
	}

   offset = (lpEntryTable - pbase);
   sprintf(m_szBuf, "Entry table: ending offset %u (%#x) size %u",
      offset, lpEntryTable, lpEntryTable - tb );
	AddStringandAdjust(m_szBuf);

   return lpEntryTable;
}

//void CFileView::FillInFlatStructures(PHEADERTEMPLATE pTemplate, unsigned char* pPointers,
PBYTE _FillInFlatStructures(PTSTR pTitle, unsigned char* pPointers,
                                     PBYTE pbase)
{
   PTSTR m_szBuf = g_cBuf;
	BYTE iCurrentSize;
   PBYTE pb = pPointers;

   strcpy( m_szBuf, pTitle );
   wsprintf( EndBuf(m_szBuf), " offset %u address %#x",
      (pPointers - pbase), pPointers );

	AddStringandAdjust( m_szBuf );
	do
	{
		iCurrentSize = pPointers[0];
		if( !iCurrentSize )
			break;

		strncpy( m_szBuf,
			(const char *)&pPointers[1],
			iCurrentSize);

		m_szBuf[iCurrentSize]='\0';

		wsprintf( m_szBuf, 
			"%s @ ordinal %u", // pTemplate->pszTemplate,
			m_szBuf,
			((WORD *)(&pPointers[iCurrentSize+1]))[0] );

		AddStringandAdjust(m_szBuf);

		pPointers += iCurrentSize;
		pPointers += sizeof(WORD) + sizeof(BYTE);

	}while( TRUE ); // the break statement bails us out here...

   wsprintf( m_szBuf, "End %s - Offset %u, address %#x",
      pTitle, // pTemplate->pszHeading,
      pPointers - pbase,
      pPointers );
	AddStringandAdjust(m_szBuf);

   return pPointers;
}

//int CFileView::FillInSizedString(BYTE *lpTarget, BYTE *lpSource)
int _FillInSizedString(BYTE *lpTarget, BYTE *lpSource)
{
	BYTE	bSize;

	bSize = *lpSource++;	// Get first byte length

	strncpy( (char *)lpTarget, (char *)lpSource, bSize );

	lpTarget[bSize]='\0';

	return (int) bSize;
}

//void CNEView::FillInResourceTable(BYTE *lpResourceTable, PBYTE pbase)
PBYTE _FillInResourceTable(BYTE *lpResourceTable, PBYTE pbase)
{
	static char *rc_types[] = {
		"<unknown>",
		"CURSOR",
		"BITMAP",
		"ICON",
		"MENU",
		"DIALOG",
		"STRING",
		"FONTDIR",
		"FONT",
		"ACCELERATOR",
		"RCDATA",
		"<unknown>",
		"GROUP CURSOR",
		"<unknown>",
		"GROUP ICON",
		"NAME TABLE",
		"<unknown>"
	};
	static char _s_szFlags[64];
	static BYTE _s_szNameBuf[264];   // change this!!!
	static BYTE _s_szNameBuff[264];
   PTSTR m_szBuf = g_cBuf;
	BYTE *lpMovingPointer = lpResourceTable;
	RTYPE *prt;
	WORD      wResSize;
	WORD wShiftCount=((WORD *)lpMovingPointer)[0];
	int iInnerLoop;

	lpMovingPointer += sizeof(WORD);

   wsprintf( m_szBuf, "Resource Table: offset %u, address %#x",
      lpResourceTable - pbase, lpResourceTable );
	AddStringandAdjust(m_szBuf);

	// Read all the resource types
	while( TRUE )
	{

		prt = (RTYPE *)lpMovingPointer;
		lpMovingPointer += sizeof(RTYPE);
		if( prt->wType == 0 )
			break;   // end of table

		if( !( prt->wType & 0x8000 ) )	// this is a  custom resource type
		{
			_FillInSizedString( _s_szNameBuf,
				lpResourceTable+prt->wType );
		}
		else
		{
			WORD wType = prt->wType & 0x7fff;
			if( wType > 15 )
				wType = 16;

			lstrcpy( (char *)_s_szNameBuf, rc_types[wType] );
		};
		
		// Allocate buffer for 'Count' resources of this type
		wResSize = prt->wCount * sizeof( RINFO );

		for( iInnerLoop = 0; iInnerLoop < prt->wCount; iInnerLoop++ )
		{
			RINFO *pCurrentResInfo = (RINFO *)lpMovingPointer;

			// figure out what to make of the ID
			if( !(pCurrentResInfo->wID & 0x8000) )
			{
				_FillInSizedString( _s_szNameBuff,
					lpResourceTable+pCurrentResInfo->wID );
			}
			else
			{
				wsprintf( (char *)_s_szNameBuff,
					"%#x",
					pCurrentResInfo->wID );
				wsprintf( _s_szFlags," " );

				if( pCurrentResInfo->wFlags & F_MOVEABLE )
					lstrcat( _s_szFlags,"<moveable> " );

				if( pCurrentResInfo->wFlags & F_SHAREABLE )
					lstrcat( _s_szFlags,"<pure> ");

				if( pCurrentResInfo->wFlags & F_PRELOAD )
					lstrcat( _s_szFlags,"<preload> ");

				wsprintf( m_szBuf,
					"%s [%s] @%#x; length: %#x; %s",
					_s_szNameBuff,
					_s_szNameBuf,
					pCurrentResInfo->wOffset<<wShiftCount,
					pCurrentResInfo->wLength<<wShiftCount,
					_s_szFlags );

				AddStringandAdjust(m_szBuf);

				lpMovingPointer += sizeof(RINFO);
			};
		}
	};	// while(TRUE)

   wsprintf( m_szBuf, "End Resource Table: offset %u, address %#x",
      lpMovingPointer - pbase, lpMovingPointer );
	AddStringandAdjust(m_szBuf);

   /*
    // Now that the resources are read, read the names
    prt = pExeInfo->pResTable;

    while (prt)
    {
        if (prt->wType & 0x8000)        // Pre-defined type
            prt->pResourceType = NULL;

        // Now do Resource Names for this type
        pri = prt->pResInfoArray;

        wI = 0;
        while ( wI < prt->wCount )
        {
            if (pri->wID & 0x8000)  // Integer resource
                pri->pResourceName = NULL;
            else                    // Named resource
            {
                // wID is offset from beginning of Resource Table
                _llseek( fFile, lResTable + pri->wID, 0 );

                wResSize = 0;
                // Read string size
                if (_lread(fFile, (LPSTR)&wResSize, 1)!=1)
                    return LERR_READINGFILE;

                // +1 for the null terminator
                pri->pResourceName = (PSTR)LocalAlloc(LPTR, wResSize+1);
                if (!pri->pResourceName)
                    return LERR_MEMALLOC;

                // Read string
                if (_lread(fFile, (LPSTR)pri->pResourceName, wResSize)!=wResSize)
                    return LERR_READINGFILE;
                pri->pResourceName[ wResSize ] = 0;   // Null terminate string;
            }
            pri++;
            wI++;
        }
        prt = prt->pNext;
    }
*/

   return lpMovingPointer;
}

//void CNEView::FillInSegmentTable(PSEGENTRY lpSegmentTable,int iEntries, WORD wAlign,
//                                 PBYTE pbase)
PBYTE _FillInSegmentTable(PSEGENTRY lpSegmentTable,int iEntries, WORD wAlign,
                                 PBYTE pbase)
{
	int       i=0;
   PBYTE pb = (PBYTE)lpSegmentTable;
   PSEGENTRY pE;
   PTSTR m_szBuf = g_cBuf;

	if( wAlign == 0 )
		wAlign = 9;

   wsprintf( m_szBuf, "Segment table: offset %u (%#x) - count %u", (pb - pbase),
      pb, iEntries );
   AddStringandAdjust(m_szBuf);
	// AddStringandAdjust("Segment table:");

	for( i = 0; i < iEntries; i++ )
	{
		pE = &lpSegmentTable[i];
		wsprintf( m_szBuf,
			" %s seg: Selector(<<align): %#08lx ;Length: %#04x ;Memory: %#04x ",
			pE->wFlags & F_DATASEG ? "DATA" : "CODE",
			((LONG)pE->wSector)<<wAlign,pE->wLength,
			pE->wMinAlloc );

		if( pE->wFlags & F_PRELOAD )
			lstrcat( m_szBuf, "PRELOAD " );

		if( pE->wFlags & F_MOVEABLE )
			lstrcat( m_szBuf, "(moveable) " );

		if( pE->wFlags & F_DISCARDABLE )
			lstrcat( m_szBuf, "(discardable)" );

		AddStringandAdjust(m_szBuf);

	}

   wsprintf( m_szBuf, "END Segment table: offset %u (%#x)",
      ((PBYTE)&lpSegmentTable[i] - pbase),
      &lpSegmentTable[i] );
   AddStringandAdjust(m_szBuf);

   return (PBYTE)&lpSegmentTable[iEntries];
}




BOOL  DumpOS2File( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   PBYTE pbase = lpdf->df_pVoid;
   DWORD msz = lpdf->dwmax;
   PBYTE pb;
   PTSTR lpd = g_cBuf;
   DWORD dwi, size;
   DWORD dwmin, dwmax, dwo, dwv;
   DWORD dwmin2, dwmax2;
   PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)pbase;
   int ine_segtab = 0; //, "Offset of Segment Table" );
   int ine_rsrctab = 0; //, "Offset of Resource Table" );
   int ine_restab = 0;  //, "Offset of resident name table" );
   int ine_modtab = 0;  //, "Offset of Module Reference Table" );
   int ine_imptab = 0;  //, "Offset of Imported Names Table" );
   int ine_nrestab = 0; //,    "Offset of Non-resident Names Table" );
   int ine_pretthunks = 0; //, "offset to return thunks" );
   int ine_psegrefbytes = 0; //, "offset to segment ref. bytes" );
   dwmin2 = dwmax2 = 0; // set IMPORT TABLE to zero
   if( pbase && (msz > sizeof(IMAGE_DOS_HEADER)) ) {
      PIMAGE_OS2_HEADER pos2 = (PIMAGE_OS2_HEADER)(pbase + pdos->e_lfanew);
      if(( pdos->e_magic == IMAGE_DOS_SIGNATURE )&&
         ( pos2->ne_magic == IMAGE_OS2_SIGNATURE))
      {
         sprtf("Found DOS header ..."MEOR);
         Show_DOS_Header( pdos, lpdf );
         sprtf("Showing OS2 (DLL) header ..."MEOR);
         Show_OS2_Header( pos2 );
         //   SHOS2( "WORD", ne_enttab, "Offset of Entry Table" );
         //   SHOS2( "WORD", ne_cbenttab, "Number of bytes in Entry Table" );
         if( (DWORD)(pos2->ne_enttab+pdos->e_lfanew) < lpdf->dwmax ) {
            dwmin = pos2->ne_enttab;
            pb = ((PBYTE)pos2 + dwmin);
            dwmin = (pb - pbase);
            size = msz - (pb - pbase);
            if( pos2->ne_cbenttab < size )
               size = pos2->ne_cbenttab;
            dwmax = dwmin + size;
            sprtf("HEX Display of ENTRY TABLE ... %u bytes ...(%u - %u)"MEOR, size, dwmin, dwmax);
            while(size)
            {
               *lpd = 0;
               if(size > 16)
                  dwi = 16;
               else
                  dwi = size;
               GetHEXString( lpd, pb, dwi, pbase, TRUE );
               sprtf( "%s"MEOR, lpd );
               pb   += dwi;
               size -= dwi;
            }
            dwmin = pos2->ne_enttab;
            pb = ((PBYTE)pos2 + dwmin);
            pb = _FillInEntryTable(pb, pbase);
            dwmax = (pb - pbase);

         }
         if( pos2->ne_imptab && pos2->ne_modtab ) {
            if( (DWORD)(pos2->ne_imptab+pdos->e_lfanew) < lpdf->dwmax ) {
               PWORD pImpTable;
               PBYTE pModTable;
               PBYTE pDetails;
               pb = (PBYTE)pos2;
               pb += pos2->ne_imptab;
               dwv = pdos->e_lfanew;
               dwv += pos2->ne_imptab; // offset to modules
               pModTable = (pbase + dwv); // Module Table (BYTE len + string)
               dwv = pos2->ne_modtab; // offset to IMPORT TABLE
               pImpTable = (PWORD)((PBYTE)pos2 + dwv); // IMPORT Table (WORD * list)
               dwo = pos2->ne_cmod; // module entries
               sprtf( "Import Table (ne_imptab) %u (%#x)... Offsets %u and %u"MEOR,
                  pos2->ne_imptab, pos2->ne_imptab,
                  ((PBYTE)pImpTable - pbase),
                  ((PBYTE)pModTable - pbase) );
               sprtf("Module Count %u ..."MEOR, dwo );
               for( dwv = 0; dwv < dwo; dwv++ ) {
                  pDetails = pModTable + pImpTable[dwv];
                  strncpy( lpd, (const char *)&pDetails[1], pDetails[0] );
                  lpd[pDetails[0]] = 0;
                  sprtf("%s"MEOR, lpd);
               }
            } else {
               sprtf("IMPORT TABLE OUTSIDE IMAGE! (%u)%#x ..."MEOR,
                  pos2->ne_imptab, pos2->ne_imptab );
            }
         }
         if( pos2->ne_restab ) {
            pb = (PBYTE)pos2 + pos2->ne_restab;
            pb = _FillInFlatStructures( "Resident Names:", pb, pbase );
         }
         if( pos2->ne_nrestab ) {
            pb = (PBYTE)pbase + pos2->ne_nrestab;
            pb = _FillInFlatStructures( "Non-resident Names:", pb, pbase );
         }
         if( pos2->ne_rsrctab ) {
            pb = (PBYTE)pos2 + pos2->ne_rsrctab;
            pb = _FillInResourceTable(pb, pbase);

         }
         if( pos2->ne_segtab ) {
            pb = _FillInSegmentTable((PSEGENTRY) (PBYTE)pos2 + pos2->ne_segtab,
               pos2->ne_cseg,
               pos2->ne_align,
               pbase);
         }

         pb = (PBYTE)((PIMAGE_OS2_HEADER) pos2 + 1);
         size = msz - (pb - pbase);
         sprtf("Display of balance ... %u bytes ... (excl %u - %u)"MEOR, size, dwmin, dwmax);
         while(size)
         {
            *lpd = 0;
            if(size > 16)
               dwi = 16;
            else
               dwi = size;

            dwo = (pb - pbase);
            if( dwmin && dwmax ) {
               if(( dwo < dwmin )||
                  ( dwo > dwmax ))
               {
                  GetHEXString( lpd, pb, dwi, pbase, TRUE );
                  sprtf( "%s"MEOR, lpd );
               }
            } else {
               GetHEXString( lpd, pb, dwi, pbase, TRUE );
               sprtf( "%s"MEOR, lpd );
            }
            pb   += dwi;
            size -= dwi;
            dwo = (pb - pbase);
            if(( ine_segtab == 0) &&
               ( pos2->ne_segtab) &&
               ( dwo > (DWORD)(pos2->ne_segtab+pdos->e_lfanew)) ) {
                  dwv = pos2->ne_segtab + pdos->e_lfanew;
                  sprtf("Offset of Segment Table (%u)%#x"MEOR, dwv, dwv );
               ine_segtab = 1;
            }
            if(( ine_rsrctab == 0 )&&
               ( pos2->ne_rsrctab )&&
               ( dwo > (DWORD)(pos2->ne_rsrctab+pdos->e_lfanew) )) {
                  dwv = pos2->ne_rsrctab+pdos->e_lfanew;
                  sprtf("Offset of Resource Table (%u)%#x"MEOR, dwv, dwv );
               ine_rsrctab = 1;
            }
            if((ine_restab == 0) &&
               ( pos2->ne_restab )&&
               ( dwo > (DWORD)(pos2->ne_restab+pdos->e_lfanew) )){
                  dwv = (pos2->ne_restab+pdos->e_lfanew);
                  sprtf("Offset of resident name table (%u)%#x"MEOR, dwv, dwv );
               ine_restab = 1;
            }
            if(( ine_modtab == 0) &&
               ( pos2->ne_modtab ) &&
               ( dwo > (DWORD)(pos2->ne_modtab+pdos->e_lfanew))){
                  dwv = (pos2->ne_modtab+pdos->e_lfanew);
                  sprtf("Offset of Module Reference Table (%u)%#x"MEOR, dwv, dwv );
               ine_modtab = 1;
            }
            if(( ine_imptab == 0) &&
               ( pos2->ne_imptab) &&
               ( dwo > (DWORD)(pos2->ne_imptab+pdos->e_lfanew))) {
                  dwv = (pos2->ne_imptab+pdos->e_lfanew);
                  sprtf("Offset of Imported Names Table (%u)%#x"MEOR, dwv, dwv );
               ine_imptab = 1;
            }
            if(( ine_nrestab == 0 )&&
               ( pos2->ne_nrestab )&&
               ( dwo > (DWORD)pos2->ne_nrestab )) {
                  dwv = pos2->ne_nrestab;
                  sprtf("Offset of Non-resident Names Table (%u)%#x"MEOR, dwv, dwv );
               ine_nrestab = 1;
            }
            if(( ine_pretthunks == 0 )&&
               ( pos2->ne_pretthunks )&&
               ( dwo > (DWORD)(pos2->ne_pretthunks+pdos->e_lfanew)) ) {
                  dwv = (pos2->ne_pretthunks+pdos->e_lfanew);
                  sprtf("offset to return thunks (%u)%#x"MEOR, dwv, dwv );
               ine_pretthunks = 1;
            }
            if(( ine_psegrefbytes == 0) &&
               ( pos2->ne_psegrefbytes )&&
               ( dwo > (DWORD)(pos2->ne_psegrefbytes+pdos->e_lfanew) )){
                  dwv = (pos2->ne_psegrefbytes+pdos->e_lfanew);
                  sprtf("offset to segment ref. bytes (%u)%#x"MEOR, dwv, dwv );
               ine_psegrefbytes = 1;
            }
         }
         bRet = TRUE;
      }
   }
   return bRet;
}

#endif // #ifndef USE_PEDUMP_CODE // FIX20080507
// eof - DumpOS2.c