
// DumpLib.c
// Dump of COFF ARCHIVE (*.lib) file
#include	"dump4.h"
#ifndef USE_PEDUMP_CODE // FIX20080507

#include <time.h>
#include <Winsock2.h>
extern void ProcessHex( PBYTE pb, DWORD len );

#define   ADDEXCEPTION

static   BYTE chLibSignature[] = IMAGE_ARCHIVE_START;   // = "!<arch>\n";
static   BYTE chLibEnd[]       = IMAGE_ARCHIVE_END;     // = "'\n";
//7.3. First Linker Member
static BYTE chLnkMemb[] = IMAGE_ARCHIVE_LINKER_MEMBER;

#define  chkmelib sprtf

typedef struct tagPTRLIST {
   LE list;
   void * vp;
   size_t len;
}PTRLIST, * PPTRLIST;

LIST_ENTRY  sPtrList = { &sPtrList, &sPtrList };
LIST_ENTRY  sDoneList = { &sDoneList, &sDoneList };

void Add_2_Ptr_List( void * vp, size_t len )
{
   PPTRLIST pl = dMALLOC(sizeof(PTRLIST));
   if(pl) {
      pl->vp = vp;
      pl->len = len;
      InsertTailList( &sPtrList, (PLE)pl );
   }
}

void Add_2_Done_List( void * vp, size_t len )
{
   PPTRLIST pl = dMALLOC(sizeof(PTRLIST));
   if(pl) {
      pl->vp = vp;
      pl->len = len;
      InsertTailList( &sDoneList, (PLE)pl );
   }
}

void Free_Ptr_List( void )
{
   KillLList( &sPtrList );
   KillLList( &sDoneList );
}

int In_List( PBYTE pb, PLE ph )
{
   PLE pn;
   Traverse_List( ph, pn )
   {
      PPTRLIST pl = (PPTRLIST)pn;
      PBYTE pbb = (PBYTE)pl->vp;
      PBYTE pbe = pbb + pl->len;
      if( ( pb >= pbb ) && ( pb <= pbe ) )
         return 1;
   }
   return 0;
}

int In_Ptr_List( PBYTE pb )
{
   PLE ph = &sPtrList;
   return( In_List( pb, ph ));
}

int In_Done_List( PBYTE pb )
{
   PLE ph = &sDoneList;
   return( In_List( pb, ph ));
}


BOOL IsLibraryFile(LPTSTR lpf, PBYTE lpImage, INT iFileSize)
{
   PIMAGE_ARCHIVE_MEMBER_HEADER  pia, pia2;
   INT      i, j;
   PBYTE    pb;
   PBYTE    ps;
   PVOID    pend;
   DWORD    dwc, dwi, dws, dwo, dwc2, dwo2;
   INT      ics;     // cummulative size of inspection of buffer
   PDWORD   pdw, pdw2;
   PWORD    pw;
   PBYTE    pbuf = &g_cBuf[0];
   LONG     lg;

   pb = (PBYTE)lpImage;
   pb += iFileSize;
   pend = (PVOID)pb;    // set an END OF FILE pointer

   pb = lpImage;
   ics = IMAGE_ARCHIVE_START_SIZE;     // start with the START size
   if( iFileSize <  ics )
   {
      chkmelib( "File size %d is too small!", iFileSize );
      return FALSE;
   }

   // The first eight bytes of an archive consist of the file signature.
   for( i = 0; i < IMAGE_ARCHIVE_START_SIZE; i++ )
   {
      if( chLibSignature[i] != pb[i] )
         break;
   }
   if( i != IMAGE_ARCHIVE_START_SIZE )
   {
      chkmelib( "Does not contain achive header of '!<arch>\\n'!" );
      return FALSE;
   }

   dws = IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR; // 7.2 set size Archive Member Headers
   pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)( lpImage + IMAGE_ARCHIVE_START_SIZE );
   ics += dws;
   if( iFileSize <  ics )
   {
      chkmelib( "Contains achive header but too small (%d) to have first member!",
         iFileSize );
      return FALSE;
   }
   pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
//static BYTE chLnkMemb[] = IMAGE_ARCHIVE_LINKER_MEMBER;
   j = strlen((LPTSTR)chLnkMemb);
   for( i = 0; i < j; i++ )
   {
      if( pb[i] != chLnkMemb[i] )
      {
         break;
      }
   }
   if( i != j )
   {
      chkmelib( "Has achive header but first member is NOT no first linker Header!" );
      return FALSE;
   }
   pb = &pia->EndHeader[0];   // [2]; // String to end header.
   for( i = 0; i < 2; i++ )
   {
      if( chLibEnd[i] != pb[i] )
         break;
   }
   if( i != 2 )
   {
      chkmelib( "Has achive header but first member has no EndHeader!" );
      return FALSE;
   }

   // march thru the file just using the head size member
   pia2 = pia;
   j = strlen(lpf);
   j = iFileSize - IMAGE_ARCHIVE_START_SIZE;
   //sprtf( "Processing file [%s] of %d bytes."MEOR, lpf, iFileSize );
   dwc = 0;
   dwc2 = 0;
   while( j > 0 )
   {
      dwc++;
      pb = &pia2->Size[0];  // [10];   // File member size - decimal.
      dwi = 0;
      while( (dwi < 10) && ( pb[dwi] >= '0' ) && ( pb[dwi] <= '9' ) )
      {
         pbuf[dwi] = pb[dwi];
         dwi++;
      }
      pbuf[dwi] = 0;
      lg = atol((LPTSTR)pbuf);    // convert string to long
      pb = (PBYTE)((PIMAGE_ARCHIVE_MEMBER_HEADER)pia2 + 1);
      pb += lg;   // get to NEXT
      dwc2 += lg;
      //sprtf( "%2d This is %d Cumulative = %d. Remaining %d (calc=%d)"MEOR,
      //   dwc, lg, dwc2, j,
      //   ( iFileSize - ( (dws * dwc) + dwc2 + IMAGE_ARCHIVE_START_SIZE ) ) );
      // NOTE: Each member header starts on the first even address after the 
      // end of the previous archive member.
      if( (DWORD)pb & 1 )
      {
         pb++;    // fix for EVEN offset
         j--;
         dwc2++;
      }
      //48 10 Size ASCII decimal representation of the total size of 
      //the archive member, not including the size of the header. 
      j -= IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;
      j -= lg;  // remove header and size
      if( j > 0 )
      {
         // move on and check the NEXT
         pia2 = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;  // set NEXT ptr
         ps = (PBYTE)pia2;             // keep this pointer
         pb = &pia2->EndHeader[0];   // [2]; // String to end header.
         for( i = 0; i < 2; i++ )
         {
            if( chLibEnd[i] != pb[i] )
               break;
         }
         if( i != 2 )
         {
            break;
         }
      }
   }
   if( j > 0 )
   {
      sprtf( "Processing file [%s] of %d bytes."MEOR, lpf, iFileSize );
      sprtf( "EEK! On the %d member the Size appears invalid! Remains %#x (%d)."MEOR,
         dwc, j, j );
      dwo = 0;
      while(j > 0)
      {
         *pbuf = 0;
         if( j > 16 )
            i = 16;
         else
            i = j;
         GetHEXString( (LPTSTR)pbuf, &ps[dwo], i, lpImage, TRUE );
         strcat((LPTSTR)pbuf, MEOR);
         sprtf((LPTSTR)pbuf);
         j   -= i;
         dwo += i;
      }
      return FALSE;
   }

   // The rest of the archive consists of a series of archive members.
   // 7.3. First Linker Member
   // = header achive member
   pb = &pia->Name[0];  // [16];   // File member name - already CHECKED
   pb = &pia->Date[0];  // [12];   // File member date - decimal.
   lg = atol((LPTSTR)pb);    // convert string to long
   {
      time_t tt = (time_t)lg;
      // like time() runtime function, so
      char * ctm = ctime(&tt);
      sprintf( (LPTSTR)pbuf, "UNIX time and date:%s", ctm );
   }
   pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
   pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
   pb = &pia->Mode[0];  // [8];    // File member mode - octal.
   pb = &pia->Size[0];  // [10];   // File member size - decimal.
   pb = &pia->EndHeader[0];   // [2]; // String to end header. - already CHECKED

   // = move up to the first linker array
   pdw = (PDWORD)((PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1);
   ics += 2;
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for first member array!",
         iFileSize );
      return FALSE;
   }

   // extract count
   // NOTE: This number is stored in big-endian format.
   dwc = ntohl( *pdw );
   if( !dwc )
      return FALSE;

   pdw++;   // get after the count
   ps = (PBYTE) (pdw + dwc);     // set up pointer to strings
   ics += (2 + (sizeof(DWORD) * dwc));
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for first member array!",
         iFileSize );
      return FALSE;
   }

   for( dwi = 0; dwi < dwc; dwi++ )
   {
      //4 4 * n Offsets Array of file offsets to archive member
      // NOTE:unsigned long stored in big-endian format.
      dwo = ntohl( pdw[dwi] );
      pb = lpImage + dwo;
      if( (dwo + dws) > (DWORD)iFileSize )
         break;
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
      pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
      pb = &pia->Date[0];  // [12];   // File member date - decimal.
      pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
      pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
      pb = &pia->Mode[0];  // [8];    // File member mode - octal.
      pb = &pia->Size[0];  // [10];   // File member size - decimal.
      pb = &pia->EndHeader[0];   // [2]; // String to end header.
      for( i = 0; i < 2; i++ )
      {
         if( chLibEnd[i] != pb[i] )
            break;
      }
      if( i != 2 )
         break;

      j = strlen( (LPSTR)ps);
      strcpy( (LPSTR)pbuf, (LPSTR)ps );
      if(j)
      {
         ps += j + 1;
         ics += (j + 1);
         if( iFileSize <  ics )
         {
            chkmelib( "Has achive header but too small (%d) for first member strings!",
               iFileSize );
            break;
         }
      }
      else
         break;
   }

   if( dwi < dwc )
   {
      if( iFileSize > ics )
      {
         chkmelib( "Has achive header but failed in walk thru first member array!" );
      }
      return FALSE;
   }

   if( (DWORD)ps & 1 )
   {
      ps++;
      ics++;
   }

   pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)ps;
   ics += dws;    // add another header
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for 2nd linker member!",
         iFileSize );
      return FALSE;
   }

   // 7.4. Second Linker Member
   // = header achive member
   pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
   pb = &pia->Date[0];  // [12];   // File member date - decimal.
   pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
   pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
   pb = &pia->Mode[0];  // [8];    // File member mode - octal.
   pb = &pia->Size[0];  // [10];   // File member size - decimal.
   pb = &pia->EndHeader[0];   // [2]; // String to end header.
   for( i = 0; i < 2; i++ )
   {
      if( chLibEnd[i] != pb[i] )
         break;
   }
   if( i != 2 )
   {
      chkmelib( "Has achive header but 2nd linker has no EndHeader!" );
      return FALSE;
   }

   // = second linker count and array
   pdw = (PDWORD)((PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1);

   ics += 4;   // about to read from a DWORD
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for 2nd linker count!",
         iFileSize );
      return FALSE;
   }

   // extract count
   // NOTE: This number is stored in little-endian format.
   // 0 4 Number of Members Unsigned long containing the number of 
   //       archive members. 
   // 4 4 * m Offsets Array of file offsets to archive member 
   //       headers, arranged in ascending order. Each offset is an 
   //       unsigned long. The number m is equal to the value of the 
   //       Number of Members field. 
   dwc = *pdw;
   if( !dwc )
   {
      chkmelib( "Has achive header but no archive member count!" );
      return FALSE;
   }

   pdw++;   // get after the count
   // sanity check only - of accumulate length
//   if( (DWORD)ics != (DWORD)((DWORD)pdw - (DWORD)lpImage) )
//   {
//      chkmelib( "Sanity check! Values are %d and %d!", ics,
//         (DWORD)((DWORD)pdw - (DWORD)lpImage) );
//   }

   pdw2 = pdw + dwc;     // set up pointer to Indices Array count
   // * 4 Number of Symbols Unsigned long containing the number of 
   //       symbols indexed. Each object-file member typically defines one 
   //       or more external symbols. 
   // * 2 * n Indices Array of 1-based indices (unsigned short) 
   //       which map symbol names to archive member offsets. The number n 
   //       is equal to Number of Symbols. For each symbol named in the 
   //       String Table, the corresponding element in the Indices array 
   //       gives an index into the Offsets array. The Offsets array, in 
   //       turn, gives the location of the archive member that contains 
   //       the symbol. 
   ics += (sizeof(DWORD) * dwc);
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for Symbols Index Array!",
         iFileSize );
      return FALSE;
   }

   ics += 4;   // about to read from another DWORD
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for Indices Array Count!",
         iFileSize );
      return FALSE;
   }

   dwc2 = *pdw2;  // extract count
   pdw2++;
   if( !dwc2 )
   {
      chkmelib( "Has achive header but ZERO Indices Array count!" );
      return FALSE;
   }

   pw = (PWORD)pdw2; // set pointer to WORD Indicies Array
   // * * String Table Series of null-terminated strings that name 
   ps = (PBYTE)( pw + dwc2 );  // get pointer to strings
   ics += (dwc2 * sizeof(WORD));
   if( iFileSize <  ics )
   {
      chkmelib( "Has achive header but too small (%d) for Indices Array!",
         iFileSize );
      return FALSE;
   }

   for( dwi = 0; dwi < dwc; dwi++ )
   {
      //4 4 * n Offsets Array of file offsets to archive member
      // NOTE:unsigned long stored (in little-endian format).
      dwo = pdw[dwi];
      pb = lpImage + dwo;
      if( (dwo + dws) > (DWORD)iFileSize )
         break;
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
      pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
      pb = &pia->Date[0];  // [12];   // File member date - decimal.
      pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
      pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
      pb = &pia->Mode[0];  // [8];    // File member mode - octal.
      pb = &pia->Size[0];  // [10];   // File member size - decimal.
      pb = &pia->EndHeader[0];   // [2]; // String to end header.
      for( i = 0; i < 2; i++ )
      {
         if( chLibEnd[i] != pb[i] )
            break;
      }
      if( i != 2 )
         break;
   }

   if( dwi < dwc )
   {
      chkmelib( "Has achive header but FAILED walk thru 2nd linker offsets to Archive members!" );
      return FALSE;
   }

   for( dwi = 0; dwi < dwc2; dwi++ )
   {
      //* 2 * n Indices Array of 1-based indices (unsigned short) 
      dwo2 = (DWORD) pw[dwi];  // extract the index
      if( dwo2 )
         dwo2--;
      else
         break;

      if( dwo2 >= dwc )         // check index value
         break;

      dwo = pdw[dwo2];        // extract offset for this index
      if( (dwo + dws) > (DWORD)iFileSize )   // check over-run of size
         break;

      pb = lpImage + dwo;     // get pointer to member
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
      pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
      pb = &pia->Date[0];  // [12];   // File member date - decimal.
      pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
      pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
      pb = &pia->Mode[0];  // [8];    // File member mode - octal.
      pb = &pia->Size[0];  // [10];   // File member size - decimal.
      pb = &pia->EndHeader[0];   // [2]; // String to end header.
      for( i = 0; i < 2; i++ )
      {
         if( chLibEnd[i] != pb[i] ) // verify the end header
            break;
      }
      if( i != 2 )
         break;

      j = strlen( (LPSTR)ps);       // get legnth of string
      strcpy( (LPSTR)pbuf, (LPSTR)ps );
      if(j)
      {
         ps += j + 1;               // bump to next string
         ics += (j + 1);
         if( iFileSize < ics )
            break;
      }
      else
         break;

   }

   if( dwi < dwc2 )
   {
      chkmelib( "Has !<arch> but FAILED walk of 2nd linker indicies to Archive members!" );
      return FALSE;
   }
   // 7.5. Longnames Member
   if( (DWORD)ps & 1 )
   {
      ps++;
      ics++;
   }

   pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)ps;
   // sanity check only - of accumulate length
//   if( (DWORD)ics != (DWORD)((DWORD)pia - (DWORD)lpImage) )
//   {
//      chkmelib( "Sanity check! Values are %d and %d!", ics,
//         (DWORD)((DWORD)pia - (DWORD)lpImage) );
//   }
   ics += dws;    // add another header
   if( iFileSize <  ics )
   {
      chkmelib( "Has !<arch> but too small (%d) for Longnames member!",
         iFileSize );
      return FALSE;
   }

   // = header achive member
   pb = &pia->Name[0];  // [16];   // File member name
   pb = &pia->Date[0];  // [12];   // File member date - decimal.
   pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
   pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
   pb = &pia->Mode[0];  // [8];    // File member mode - octal.
   pb = &pia->Size[0];  // [10];   // File member size - decimal.
   pb = &pia->EndHeader[0];   // [2]; // String to end header.
   for( i = 0; i < 2; i++ )
   {
      if( chLibEnd[i] != pb[i] )
         break;
   }
   if( i != 2 )
      return FALSE;

   ps = (PBYTE)( (PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1 );  // get pointer to strings
   // sanity check only - of accumulate length
   if( (DWORD)ics != (DWORD)((DWORD)ps - (DWORD)lpImage) )
   {
      chkmelib( "Sanity check! Values are %d and %d!", ics,
         (DWORD)((DWORD)ps - (DWORD)lpImage) );
   }


   return TRUE;
}

TCHAR szLibHdr[] = "Archive (LIB) Header: [";
TCHAR szLnkNm1[] = "First Linker Name:    [";
TCHAR szHdrNm[]  = "Header Name:          [";
TCHAR szTmDate[] = "UNIX time/data:       [";
TCHAR szUserID[] = "User ID:              [";
TCHAR szGroupI[] = "Group ID:             [";
TCHAR szMode[]   = "Mode: (octal)         [";
TCHAR szSize[]   = "Member Size:          [";
TCHAR szEndHdr[] = "End Header:           [";


VOID DumpLibMemberHdr(LPTSTR pTitle, PIMAGE_ARCHIVE_MEMBER_HEADER pia)
{
   LPTSTR   lpd = (LPTSTR)&g_bBuf[0];
   INT      i, k;
   PBYTE    pb;
   LONG     lg;

   if( In_Done_List( (PBYTE)pia ) )
      return;

   Add_2_Done_List( pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
   if( VERB5 ) {
      ProcessHex( (PBYTE) pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
   }

   strcpy(lpd, pTitle);
   if( !pia )
   {
      strcat( lpd, " IS NULL!" );
      AddStringandAdjust(lpd);
      return;      
   }
   pb = &pia->EndHeader[0];   // [2]; // String to end header.
   for( i = 0; i < 2; i++ )
   {
      if( chLibEnd[i] != pb[i] )
         break;
   }
   if( i == 2 )
      strcat(lpd," is Ok");
   else
      strcat(lpd, " is ERRANT!");

   AddStringandAdjust(lpd);

   // = header achive member
//typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
//    BYTE     Name[16];   // File member name - `/' terminated.
//    BYTE     Date[12];   // File member date - decimal.
//    BYTE     UserID[6];  // File member user id - decimal.
//    BYTE     GroupID[6]; // File member group id - decimal.
//    BYTE     Mode[8];    // File member mode - octal.
//    BYTE     Size[10];   // File member size - decimal.
//    BYTE     EndHeader[2]; // String to end header.
//} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;
//#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60
   pb = &pia->Name[0];  // [16];   // File member name - `/' terminated.
//static BYTE chLnkMemb[] = IMAGE_ARCHIVE_LINKER_MEMBER;
   strcpy(lpd, szHdrNm);
   k = AppendASCII(lpd, pb, 16);
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szTmDate);
   pb = &pia->Date[0];  // [12];   // File member date - decimal.
   lg = atol((LPTSTR)pb);    // convert string to long
   // like time() runtime function, so
   {
      time_t tt = (time_t)lg;
      char * ctm = ctime( &tt );
      strcat( lpd, ctm );
   }
   k = strlen(lpd);
   while( k-- )
   {
      if( lpd[k] > ' ' )
         break;
      lpd[k] = 0;
   }
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szUserID);
   pb = &pia->UserID[0];   // [6];  // File member user id - decimal.
   lg = atol((LPTSTR)pb);    // convert string to long
   sprintf(EndBuf(lpd), "%d", lg );
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szGroupI);
   pb = &pia->GroupID[0];  // [6]; // File member group id - decimal.
   lg = atol((LPTSTR)pb);    // convert string to long
   sprintf(EndBuf(lpd), "%d", lg );
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szMode  );
   pb = &pia->Mode[0];  // [8];    // File member mode - octal.
   k = AppendASCII(lpd, pb, 8);
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szSize  );
   pb = &pia->Size[0];  // [10];   // File member size - decimal.
   lg = atol((LPTSTR)pb);    // convert string to long
   sprintf(EndBuf(lpd), "%d", lg );
   strcat(lpd, "]");
   AddStringandAdjust(lpd);

   strcpy(lpd, szEndHdr);
   pb = &pia->EndHeader[0];   // [2]; // String to end header.
   k = AppendASCII(lpd, pb, 2);
   strcat(lpd, "]");
   for( i = 0; i < 2; i++ )
   {
      if( chLibEnd[i] != pb[i] )
         break;
   }
   if( i == 2 )
      strcat(lpd," Ends Ok");
   else
      strcat(lpd, " Error Ending!");

   AddStringandAdjust(lpd);

   Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );

}


VOID DumpLibHeader(PBYTE pHead, LPDFSTR lpdf)
{
   LPTSTR   lpd = (LPTSTR)&g_bBuf[0];
   PIMAGE_ARCHIVE_MEMBER_HEADER  pia;

   sprintf(lpd, "The LIBRARY starts with (%d):-", sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
   AddStringandAdjust(lpd);
   ProcessHex( pHead, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );

   strcpy(lpd, szLibHdr);
   if( !pHead )
   {
      strcat( lpd, "IS NULL!]" );
      AddStringandAdjust(lpd);
      return;      
   }

   AppendASCII(lpd, pHead, IMAGE_ARCHIVE_START_SIZE );

   strcat(lpd, "]");
   AddStringandAdjust(lpd);
   Add_2_Ptr_List( (void *)pHead, IMAGE_ARCHIVE_START_SIZE );

   pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)( pHead + IMAGE_ARCHIVE_START_SIZE );

   // 7.3. First Linker Member
   DumpLibMemberHdr( "Lib First Linker Member", pia );

}

#define  MX1LINE     40
#define  MXSTGLN     256

VOID DumpLibStringList(LPTSTR pTitle, LPTSTR pStrings, INT iMax)
{
   LPTSTR   lpd = (LPTSTR)&g_bBuf[0];
   LPTSTR   lps;
   INT      i, imax, itot, icnt, icnt2;
   DWORD    dwmin;
   DWORD    totlen = 0;

   strcpy(lpd, pTitle);
   lps = pStrings;
   i = strlen(lps);
   if( !i )
   {
      strcat( lpd, " NOTE: String list is empty!" );
      AddStringandAdjust(lpd);
      return;
   }

   icnt = 0;
   imax = itot = 0;
   dwmin = (DWORD)-1;
   while( (i = strlen(lps)) > 0 )
   {
      icnt++;
      if(i > imax)
         imax = i;
      if((DWORD)i < dwmin)
         dwmin = i;
      itot += i;
      lps += (i + 1);
      if( icnt >= iMax )
         break;
   }

   sprintf( EndBuf(lpd), " (List of %d strings)", icnt );
   AddStringandAdjust(lpd);
   sprintf( lpd,
      "Total of %u bytes. Minimum = %u. Maximum = %u",
      itot, dwmin, imax );
   AddStringandAdjust(lpd);

   lps = pStrings;
   *lpd = 0;
   icnt2 = icnt;
   icnt  = 0;     // restart the counter
   while( (i = strlen(lps)) > 0 )
   {
      icnt++;
      if( i < MXSTGLN )
         sprintf( lpd, "%4d [%s]%d", icnt, lps, i );
      else
      {
         sprintf( lpd, "%4d [", icnt );
         itot = strlen(lpd);
         for( imax = 0; imax < MXSTGLN; imax++ )
         {
            lpd[itot++] = lps[imax];
         }
         lpd[itot] = 0;
         sprintf(EndBuf(lpd), "...]%d", i);
      }

      AddStringandAdjust(lpd);

      gdwBgnOff += (i + 1);

      lps += (i + 1);
      totlen += (i + 1);
      if( ( icnt >= iMax  ) ||
          ( icnt >= icnt2 ) )
         break;
   }

   sprintf(lpd, "End of list of %d strings", icnt );
   AddStringandAdjust(lpd);
   Add_2_Ptr_List( pStrings, totlen );

}


//typedef struct _IMAGE_EXPORT_DIRECTORY {
//    DWORD   Characteristics;
//    DWORD   TimeDateStamp;
//    WORD    MajorVersion;
//    WORD    MinorVersion;
//    DWORD   Name;
//    DWORD   Base;
//    DWORD   NumberOfFunctions;
//    DWORD   NumberOfNames;
//    DWORD   AddressOfFunctions;     // RVA from base of image
//    DWORD   AddressOfNames;         // RVA from base of image
//    DWORD   AddressOfNameOrdinals;  // RVA from base of image
//} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

void Out_Export_Directory ( PIMAGE_EXPORT_DIRECTORY pxd )
{
   LPTSTR      lpd = &g_cBuf[0];
   sprintf(lpd, "IMAGE_EXPORT_DIRECTORY follows (%d):", sizeof(IMAGE_EXPORT_DIRECTORY) );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Characteristics       (DWORD) : %#0X", pxd->Characteristics );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Timestamp             (DWORD) : %#0X", pxd->TimeDateStamp );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Version           (WORD:WORD) : %u.%u", pxd->MajorVersion & 0xffff, pxd->MinorVersion & 0xffff );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Name                  (DWORD) : %#0X", pxd->Name );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Base                  (DWORD) : %#0X", pxd->Base );
   AddStringandAdjust(lpd);
   sprintf(lpd, "NumberOfFunctions     (DWORD) : %d", pxd->NumberOfFunctions );
   AddStringandAdjust(lpd);
   sprintf(lpd, "NumberOfNames         (DWORD) : %d", pxd->NumberOfNames );
   AddStringandAdjust(lpd);
   sprintf(lpd, "AddressOfFunctions    (DWORD) : %#0X", pxd->AddressOfFunctions );
   AddStringandAdjust(lpd);
   sprintf(lpd, "AddressOfNames        (DWORD) : %#0X", pxd->AddressOfNames );
   AddStringandAdjust(lpd);
   sprintf(lpd, "AddressOfNameOrdinals (DWORD) : %#0X", pxd->AddressOfNameOrdinals );
   AddStringandAdjust(lpd);
}

//typedef struct _IMAGE_THUNK_DATA32 {
//    union {
//        DWORD ForwarderString;      // PBYTE 
//        DWORD Function;             // PDWORD
//        DWORD Ordinal;
//        DWORD AddressOfData;        // PIMAGE_IMPORT_BY_NAME
//    } u1;
//} IMAGE_THUNK_DATA32;
//typedef IMAGE_THUNK_DATA32 * PIMAGE_THUNK_DATA32;

//typedef struct _IMAGE_IMPORT_DESCRIPTOR {
//    union {
//        DWORD   Characteristics;            // 0 for terminating null import descriptor
//        DWORD   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
//    };
//    DWORD   TimeDateStamp;                  // 0 if not bound,
//                                            // -1 if bound, and real date\time stamp
//                                            //     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
//                                            // O.W. date/time stamp of DLL bound to (Old BIND)
//
//    DWORD   ForwarderChain;                 // -1 if no forwarders
//    DWORD   Name;
//    DWORD   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
//} IMAGE_IMPORT_DESCRIPTOR;
//typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;
void Out_Import_Descriptor ( void * vp )
{
   PIMAGE_IMPORT_DESCRIPTOR pid = (PIMAGE_IMPORT_DESCRIPTOR)vp;
   LPTSTR      lpd = &g_cBuf[0];
   sprintf(lpd, "IMAGE_IMPORT_DESCRIPTOR follows (%d):", sizeof(IMAGE_IMPORT_DESCRIPTOR) );
   AddStringandAdjust(lpd);
   sprintf(lpd, "Characteristics       (DWORD) : %#0X", pid->Characteristics );
   AddStringandAdjust(lpd);
   sprintf(lpd, "TimeStamp             (DWORD) : %#0X", pid->TimeDateStamp );
   AddStringandAdjust(lpd);
   sprintf(lpd, "ForwarderChain        (DWORD) : %#0X (%s)", pid->ForwarderChain,
      (pid->ForwarderChain == -1) ? "None" : "Chain");
   AddStringandAdjust(lpd);
   sprintf(lpd, "Name                  (DWORD) : %#0X", pid->Name );
   AddStringandAdjust(lpd);
   sprintf(lpd, "FirstThunk            (DWORD) : %#0X", pid->FirstThunk );
   AddStringandAdjust(lpd);
}


BOOL LibFrame( LPDFSTR lpdf )
{
   PBYTE       pHead, pb, ps;
   PDWORD      pdw, pdw2;
   PWORD       pw;
   DWORD       dwi, dwc, dwo, dwc2, dwo2;
   PIMAGE_ARCHIVE_MEMBER_HEADER  pia;
//   PIMAGE_EXPORT_DIRECTORY pxd;
   INT         j;
   LPTSTR      lpd = &g_cBuf[0];

#ifdef   ADDEXCEPTION
   _try
	{
#endif   // #ifdef   ADDEXCEPTION
      pHead = (PBYTE)lpdf->df_pVoid;
      pb = pHead;

      DumpLibHeader( pb, lpdf );

      gdwBgnOff = IMAGE_ARCHIVE_START_SIZE;

      // = first linker member
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER) (pb + IMAGE_ARCHIVE_START_SIZE);

      // = first linker array = first linker member plus 1
      pdw = (PDWORD)((PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1);

      gdwBgnOff += sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
      // extract count
      // NOTE: This number is stored in big-endian format.
      dwc = ntohl( *pdw );
      Add_2_Ptr_List( (void *)pdw, sizeof(DWORD) );

      pdw++;   // get after the count
      gdwBgnOff += sizeof(DWORD);
      ps = (PBYTE) (pdw + dwc);     // set up pointer to strings
      gdwBgnOff += (sizeof(DWORD) * dwc);
      Add_2_Ptr_List( (void *)ps, (sizeof(DWORD) * dwc) );

      DumpLibStringList( "First Linker Array", (LPTSTR)ps, dwc );

      for( dwi = 0; dwi < dwc; dwi++ )
      {
         //4 4 * n Offsets Array of file offsets to archive member
         // NOTE:unsigned long stored in big-endian format.
         dwo = ntohl( pdw[dwi] );
         pb = pHead + dwo;
         pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
         sprintf(lpd, "%4d From 1st Linker Array", (dwi + 1));
         DumpLibMemberHdr( lpd, pia );
         Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
         j = strlen( (LPSTR)ps);
         if(j)
            ps += j + 1;
      }

      if( (DWORD)ps & 1 )
         ps++;

      // 7.4. Second Linker Member
      // = header achive member
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)ps;
      DumpLibMemberHdr( "Lib Second Linker Member", pia );
      Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
      // = second linker count and array
      pdw = (PDWORD)((PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1);
      // extract count
      // NOTE: This number is stored in little-endian format.
      // 0 4 Number of Members Unsigned long containing the number of 
      //       archive members. 
      // 4 4 * m Offsets Array of file offsets to archive member 
      //       headers, arranged in ascending order. Each offset is an 
      //       unsigned long. The number m is equal to the value of the 
      //       Number of Members field. 
      dwc = *pdw;
      Add_2_Ptr_List( (void *)pdw, sizeof(DWORD) );
      pdw++;   // get after the count
      pdw2 = pdw + dwc;     // set up pointer to Indices Array count
      // * 4 Number of Symbols Unsigned long containing the number of 
      //       symbols indexed. Each object-file member typically defines one 
      //       or more external symbols. 
      // * 2 * n Indices Array of 1-based indices (unsigned short) 
      //       which map symbol names to archive member offsets. The number n 
      //       is equal to Number of Symbols. For each symbol named in the 
      //       String Table, the corresponding element in the Indices array 
      //       gives an index into the Offsets array. The Offsets array, in 
      //       turn, gives the location of the archive member that contains 
      //       the symbol. 
      dwc2 = *pdw2;  // extract count
      Add_2_Ptr_List( (void *)pdw2, sizeof(DWORD) );
      pdw2++;
      pw = (PWORD)pdw2; // set pointer to WORD Indicies Array
      // * * String Table Series of null-terminated strings that name 
      ps = (PBYTE)( pw + dwc2 );  // get pointer to strings
      DumpLibStringList( "Second Linker Array", (LPTSTR)ps, dwc2 );
      Add_2_Ptr_List( (void *)ps, (sizeof(DWORD) * dwc2) );
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         //4 4 * n Offsets Array of file offsets to archive member
         // NOTE:unsigned long stored (in little-endian format).
         dwo = pdw[dwi];
         pb = pHead + dwo;
         pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
      }
      for( dwi = 0; dwi < dwc2; dwi++ )
      {
         //* 2 * n Indices Array of 1-based indices (unsigned short) 
         dwo2 = (DWORD) pw[dwi];  // extract the index
         if( dwo2 )
            dwo2--;
         if( dwo2 < dwc )         // check index value
         {
            dwo = pdw[dwo2];        // extract offset for this index
            pb = pHead + dwo;     // get pointer to member
            pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)pb;
            sprintf( lpd, "%4d From 2nd Linker Array", (dwi + 1));
            DumpLibMemberHdr( lpd, pia );
            Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
            j = strlen( (LPSTR)ps);       // get legnth of string
            ps += j + 1;               // bump to next string
         }
      }

      // 7.5. Longnames Member
      if( (DWORD)ps & 1 )
         ps++;
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)ps;
      // = header achive member
      DumpLibMemberHdr( "Longnames Member", pia );
      Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );

      ps = (PBYTE)( (PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1 );  // get pointer to strings
      Add_2_Ptr_List( (void *)ps, strlen(ps) + 1 );
      sprintf(lpd, "String of length %d ", strlen(ps) );
      AddStringandAdjust(lpd);
      strcpy(lpd, ps);
      AddStringandAdjust(lpd);
      // NEXT is ???
      ps += strlen(ps);
      ps++;
      pia = (PIMAGE_ARCHIVE_MEMBER_HEADER)ps;
      DumpLibMemberHdr( "Another Member", pia );
      Add_2_Ptr_List( (void *)pia, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) );
      // get pointer to ???
      //ps = (PBYTE) ( (PIMAGE_ARCHIVE_MEMBER_HEADER)pia + 1 );
      //ProcessHex( ps, 32 );
      //Out_Import_Descriptor ( (void *)ps );
      //ps += 2; // ??????????????????????
      //pxd = (PIMAGE_EXPORT_DIRECTORY)ps;
      //Out_Export_Directory ( pxd );

#ifdef   ADDEXCEPTION
	}
	_except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
         EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		MessageBox(NULL, "CORRUPTED FILE: Can not display all information!",
         "ERROR IN DATA",
         MB_OK );
      return FALSE;
	}
#endif   // #ifdef   ADDEXCEPTION

   return TRUE;
}

BOOL  ProcessLIB( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   Free_Ptr_List();
   if( IsLibraryFile( lpdf->fn, lpdf->lpb, (INT)lpdf->dwmax ) )
   {
      bRet = LibFrame( lpdf );
      if(bRet) {
         // success - show data NOT covered in pointer array
         PBYTE pb = lpdf->df_pVoid;
         DWORD mx = lpdf->dwmax;
         DWORD dwi, dwb, dwd;
         for(dwi = 0; dwi < mx; dwi++)
         {
            if( !In_Ptr_List( &pb[dwi] ) ) {
               dwb = dwi;
               dwi++;
               while( (dwi < mx) && ( !In_Ptr_List( &pb[dwi] ) ) ) {
                  dwi++;
               }
               dwd = dwi - dwb;
               ProcessHex( &pb[dwb], dwd );
            }
         }
      }
   }

   Free_Ptr_List();
   return bRet;
}

#endif // #ifndef USE_PEDUMP_CODE // FIX20080507
// eof - DumpLib.c
