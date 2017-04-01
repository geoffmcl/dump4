
// DumpUtil.c
// Utility Module

#include "Dump4.h"
#include "DumpUtil.h"
#ifdef _WIN32
#include <conio.h>  // got _getch()
#endif // _WIN32

VOID AddASMString( LPTSTR lps, DWORD dwo );
VOID Write2ASMFile(LPTSTR lps, DWORD dwo);

int   wait_keyin( void )
{
   int   chr;
   sprtf( "Any key to continue ..." );
   //chr = toupper(_getch());
   chr = _getch();
   fflush(stdin);
   return chr;
}


VOID  AddStringandAdjust( LPTSTR lpd )
{
   // FIX20061216 - add CRLF, if not there already
   // prt(lpd);
   size_t len = strlen(lpd);
   if(len) {
      len--;
      if(lpd[len] >= ' ')
         sprtf("%s"MEOR, lpd);
      else
         prt(lpd);
   }
}

LPTSTR   GetNxtBuf( VOID )
{
   // (MXLINEB * MXLINES)
   INT   i = giLnBuf;      //     W.ws_iLnBuf
   i++;
   if( i >= MXLINES )
      i = 0;
   giLnBuf = i;
   return( &gszLnBuf[ (MXLINEB2 * i) ] );    // W.ws_szLnBuf
}

BOOL IsStaticBuffer(LPTSTR buf) 
{
    INT i = 0;
    LPTSTR next;
    for (i = 0; i < MXLINES; i++) {
        next = &gszLnBuf[ (MXLINEB2 * i) ];
        if (buf == next)
            return TRUE;
    }
    return FALSE;
}

LPTSTR	GetShortName( LPTSTR lps, DWORD dwmx )
{
   LPTSTR   lpb = GetNxtBuf();
   DWORD    dwi = strlen(lps);
   DWORD    dw2, dwt;
   if( dwi < MXLINEB )
      strcpy( lpb, lps );
   else
   {
      strncpy( lpb, lps, (MXLINEB-1) );
      lpb[ MXLINEB-1 ] = 0;   // ensure teminating NUL char
   }
   if( (dwi + 3) > dwmx )
   {
      dw2 = dwmx / 2;   // get half the maximum
      if( ( dw2 > 2 ) & ( dw2 < (MXLINEB / 2) ) )
      {
         dw2--;
         lpb[dw2] = 0;
         strcat(lpb,"...");
         dwt = dwi - dw2;
         strcat(lpb, &lps[dwt] );
      }
   }
   return lpb;
}

LPTSTR GetHEXOffset( DWORD dwo )
{
    LPTSTR ps = GetNxtBuf();
    *ps = 0;
    sprintf( EndBuf(ps), "%4.4X", ((dwo & 0xffff0000) >> 16) );	// Put in offset
    strcat( ps,":");
    sprintf( EndBuf(ps), "%4.4X ", (dwo & 0x0000ffff) );
    return ps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GetHEXString
// Return type: INT 
// Arguments  : LPTSTR lpd - Destination buffer
//            : PBYTE pb   - Source data buffer
//            : INT iLen   - Length of bytes to process
//            : PBYTE pBgn - Pointer to beginning address to get relative offset
// Description: Prepare a HEX string into the given destination buffer,
//              for the length given.
// If length is greater than 16, then a Cr/Lf pair will be added after each
// line of HEX data.
///////////////////////////////////////////////////////////////////////////////
INT   GetHEXString( LPTSTR lpd, PBYTE pb, INT iLen, PBYTE pBgn, BOOL fill )
{
   PBYTE p;
   DWORD dwo, dwi;
   INT   i, iCount;

   dwo = 0;
   while( iLen > 0 )
   {
      if(dwo)
         strcat(lpd, MEOR);
      // if given the BEGIN address, show a relative offset
      dwi = (DWORD)(pb - pBgn) + dwo;
      sprintf( EndBuf(lpd), "%4.4X", ((dwi & 0xffff0000) >> 16) );	// Put in offset
      strcat( lpd,":");
      sprintf( EndBuf(lpd), "%4.4X ", (dwi & 0x0000ffff) );

      p = pb + dwo;	// Get moving pointer
      iCount = iLen;	// and counter
      for( i = 0; i < 16; i++ ) {
         if( i == 8 )
            strcat(lpd, " ");

         if( iCount > 0 ) {
				sprintf( EndBuf(lpd),
               "%2.2X ",
               *p );
		 } else {
            // fill in as blank
             if(fill)
                 strcat( lpd, "   " );
		 }
	     iCount--;	// Reduce remainder count
		 p++;		// bump to next byte
      }

      // add space the ASCII
      strcat( lpd, " " );

      p = pb + dwo;	// Get the pointer again
      iCount = iLen;	// and reset the count
		for( i = 0; i < 16; i++ )
		{
			if( ( iCount > 0 ) &&
				isprint(*p) )
			{
				sprintf( EndBuf(lpd),
						"%c",
						*p );
			}
			else if( iCount > 0 )
			{
				// NOT printable
				strcat( lpd, "." );	// insert a dot
			}
			else
			{
				// end of file = add space
                if( fill )
                    strcat( lpd, " " );
			}

			iCount--;	// reduce remainder
			p++;		// bump pointer
		}	// for 16 bytes

      dwo  += 16;
      iLen -= 16;
   }

   return( strlen(lpd) );

}


INT   AppendASCII( LPTSTR lpd, PBYTE pb, INT iLen )
{
   INT   k = strlen(lpd);
   INT   i, c;

   k = strlen(lpd);
   for( i = 0; i < iLen; i++ )
   {
      c = pb[i];
      if( c < ' ' )
      {
         c += 0x40;
         lpd[k++] = '^';
      }
      else if( c >= 0x7f )
      {
         lpd[k++] = '@';
         c |= ~(0x80);
         if( c < ' ' )
         {
            lpd[k++] = '^';
            c += 0x40;
         }
      }
      lpd[k++] = (TCHAR)c;
   }

   lpd[k] = 0;

   return k;
}


// ***********************
// Output to an ASM file
#ifdef   DUMP4
#define  DEF_ASM_FILE   "TEMPDUMP.ASM"
#else    // !DUMP4
#define  DEF_ASM_FILE   "TEMPYAHU.ASM"
#endif   // DUMP4 y/n

BOOL     bDnAInit = FALSE;
BOOL     bAppdASM = FALSE;
#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
HANDLE   hASMFile = 0;
#else // !_WIN32
FILE * fpASMFile = 0;
#endif

TCHAR    szASMDef[] = DEF_ASM_FILE;
TCHAR    szASMFile[264];

VOID  InitASMFile( VOID )
{
   LPTSTR   p1 = szASMFile;
   if( !bDnAInit )
   {
      bDnAInit = TRUE;
      szASMFile[0] = 0;
#ifdef _WIN32
      GetModuleFileName(NULL, szASMFile, 256);
      p1 = strrchr( szASMFile, '\\' );
      if(p1)
      {
#ifdef   NDEBUG
         p1++;
#else // for the DEBUG version
         LPTSTR   p2;
         p2 = p1;
         *p1 = 0;
         p1 = strrchr( szASMFile, '\\' );
         if(p1)
            p1++;
         else
         {
            *p2 = '\\';
            p2++;
            p1 = p2;
         }
#endif   // NDEBUG y/n
      }
      else
         p1 = szASMFile;

      strcpy(p1, szASMDef);
#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
      hASMFile = 0;
#else
      fpASMFile = 0;
#endif
#else
      strcpy(p1, szASMDef);
      fpASMFile = 0;
#endif

   }
}

VOID  CloseASMFile(VOID)
{
#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
    if( VFH(hASMFile) )
      CloseHandle(hASMFile);
   hASMFile = 0;
#else
    if (fpASMFile)
        fclose(fpASMFile);
    fpASMFile = 0;
#endif
}

VOID  CreateASMFile( VOID )
{
   if( !bDnAInit )
      InitASMFile();

#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
   if( bAppdASM )
   {
      hASMFile = CreateFile( szASMFile, // file name
         (GENERIC_READ | GENERIC_WRITE),     // access mode
         (FILE_SHARE_READ | FILE_SHARE_WRITE),  // share mode
         0, // SD
         OPEN_EXISTING, // how to create
         FILE_ATTRIBUTE_NORMAL,  // file attributes
         0 ); // handle to template file
      if( VFH(hASMFile) )
      {
         LONG  lg = 0;
         SetFilePointer( hASMFile,   // handle to file
            0, // bytes to move pointer
            &lg,  // bytes to move pointer
            FILE_END ); // starting point
      }
      else
      {
         hASMFile = CreateFile( szASMFile,   // file name
            (GENERIC_READ | GENERIC_WRITE),     // access mode
            (FILE_SHARE_READ | FILE_SHARE_WRITE),  // share mode
            0, // SD
            CREATE_ALWAYS,                // how to create
            FILE_ATTRIBUTE_NORMAL,                 // file attributes
            0 );        // handle to template file
      }
   }
   else
   {
         hASMFile = CreateFile( szASMFile,   // file name
            (GENERIC_READ | GENERIC_WRITE),     // access mode
            (FILE_SHARE_READ | FILE_SHARE_WRITE),  // share mode
            0, // SD
            CREATE_ALWAYS,                // how to create
            FILE_ATTRIBUTE_NORMAL,                 // file attributes
            0 );        // handle to template file
   }
#else
   if (bAppdASM)
   {
       fpASMFile = fopen(szASMFile, "a");
   }
   else
   {
       fpASMFile = fopen(szASMFile, "w");
   }

#endif
}

VOID WriteASMFile( LPTSTR lps )
{
   DWORD    dwl, dww;
#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
   if( hASMFile == 0 )
      CreateASMFile();     // one time create

   dwl = strlen( lps );
   if( VFH(hASMFile) )
   {
      if( dwl )
      {
         dww = 0; // zero written length
         if( ( WriteFile( hASMFile, lps, dwl, &dww, NULL ) ) &&
            ( dwl == dww ) )
         {
            // success
            if( lps[ (dwl - 1) ] >= ' ' )
            {
               WriteASMFile( MEOR );
            }
         }
         else
         {
            CloseHandle( hASMFile );
            hASMFile = INVALID_HANDLE_VALUE;
         }
      }
      else
      {
               WriteASMFile( MEOR );
      }
   }
#else
   if (fpASMFile == 0)
       CreateASMFile();     // one time create

   dwl = strlen(lps);
   if (VFH(fpASMFile))
   {
       if (dwl)
       {
           dww = fwrite(lps, 1, dwl, fpASMFile);
           if (dww == dwl)
           {
               // success
               if (lps[(dwl - 1)] >= ' ')
               {
                   WriteASMFile(MEOR);
               }
           }
           else
           {
               fclose(fpASMFile);
               fpASMFile = INVALID_HANDLE_VALUE;
           }
       }
       else
       {
           WriteASMFile(MEOR);
       }
   }
#endif
}

INT      strbgn( LPTSTR lpb, LPTSTR lps )
{
   LPTSTR lpd = GetNxtBuf();
   INT      i = -1;
   INT      j = strlen(lps);
   INT      k = strlen(lpb);
   if( *lpb != *lps )
      i = ( *lpb - *lps );
   else if( j && ( j <= k ) )
   {
      strncpy(lpd, lpb, j);
      lpd[j] = 0;
      i = strcmp(lpd,lps);
   }
   return i;
}

INT      strbgni( LPTSTR lpb, LPTSTR lps )
{
   LPTSTR lpd = GetNxtBuf();
   INT      i = -1;
   INT      j = strlen(lps);
   INT      k = strlen(lpb);
   if( toupper(*lpb) != toupper(*lps) )
      i = ( toupper(*lpb) - toupper(*lps) );
   else if( j && ( j <= k ) )
   {
      strncpy(lpd, lpb, j);
      lpd[j] = 0;
      i = STRCMPI(lpd,lps);
   }
   return i;
}

VOID AddASMString( LPTSTR lps, DWORD dwo )
{

   AddStringandAdjust(lps);   // which does
	// ComputeNewHorizontalExtent( m_iFontWidth * strlen(lps) );
   // and the MAIN action is to ADD THE STRING TO THE LIST BOX
	// m_LB.AddString( lps );
   // sprtf( "%s"MEOR, lps ); and to diagnostic out
   Write2ASMFile( lps, dwo );

}

VOID Write2ASMFile(LPTSTR lps, DWORD dwo)
{
   DWORD    dwl, dww;
   LPTSTR   lpo = &gszDiag[0];
#if (defined(_WIN32) && defined(USE_NATIVE_FIO))
   if( hASMFile == 0 )
      CreateASMFile();     // one time create
   dwl = strlen( lps );
   if( VFH(hASMFile) )
   {
      if( ( *lps == '_' ) || ( *lps == '?' ) )
         strcpy( lpo, MEOR );
      else
         strcpy( lpo, "   " );

      strcat( lpo, &lps[dwo] );  // get the TAIL
      if( dwl )
      {
         if( lps[dwl-1] >= ' ' )
            strcat(lpo, MEOR );  // add Cr/Lf at end
      }
      else
         strcpy(lpo, MEOR);   // just out a Cr/Lf

      dwl = strlen(lpo);   // get lenght
      dww = 0; // zero written length
      if( ( WriteFile( hASMFile, lpo, dwl, &dww, NULL ) ) &&
         ( dwl == dww ) )
      {
         // success
      }
      else
      {
         CloseHandle( hASMFile );
         hASMFile = INVALID_HANDLE_VALUE;
      }
   }
#else
   if (fpASMFile == 0)
       CreateASMFile();     // one time create
   dwl = strlen(lps);
   if (VFH(fpASMFile))
   {
       if ((*lps == '_') || (*lps == '?'))
           strcpy(lpo, MEOR);
       else
           strcpy(lpo, "   ");

       strcat(lpo, &lps[dwo]);  // get the TAIL
       if (dwl)
       {
           if (lps[dwl - 1] >= ' ')
               strcat(lpo, MEOR);  // add Cr/Lf at end
       }
       else
           strcpy(lpo, MEOR);   // just out a Cr/Lf

       dwl = strlen(lpo);   // get lenght
       dww = 0; // zero written length
       dww = fwrite(lpo, 1, dwl, fpASMFile);
       if (dwl == dww)
       {
           // success
       }
       else
       {
           fclose(fpASMFile);
           fpASMFile = INVALID_HANDLE_VALUE;
       }
   }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : InStr
// Return type: INT 
// Arguments  : LPTSTR lpb
//            : LPTSTR lps
// Description: Return the position of the FIRST instance of the string in lps
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
INT   InStr( LPTSTR lpb, LPTSTR lps )
{
   INT   iRet = 0;
   INT   i, j, k, l, m;
   TCHAR    c;
   i = strlen(lpb);
   j = strlen(lps);
   if( i && j && ( i >= j ) )
   {
      c = *lps;   // get the first we are looking for
      l = i - ( j - 1 );   // get the maximum length to search
      for( k = 0; k < l; k++ )
      {
         if( lpb[k] == c )
         {
            // found the FIRST char so check until end of compare string
            for( m = 1; m < j; m++ )
            {
               if( lpb[k+m] != lps[m] )   // on first NOT equal
                  break;   // out of here
            }
            if( m == j )   // if we reached the end of the search string
            {
               iRet = k + 1;  // return NUMERIC position (that is LOGICAL + 1)
               break;   // and out of the outer search loop
            }
         }
      }  // for the search length
   }
   return iRet;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Left
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl
//            : DWORD dwi
// Description: Return the LEFT prortion of a string
//              Emulates the Visual Basic function
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Left( LPTSTR lpl, DWORD dwi )
{
   LPTSTR   lps = GetNxtBuf();
   DWORD    dwk;
   for( dwk = 0; dwk < dwi; dwk++ )
      lps[dwk] = lpl[dwk];
   lps[dwk] = 0;
   return lps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Right
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl
//            : DWORD dwl
// Description: Returns a buffer containing the RIGHT postion of a string
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Right( LPTSTR lpl, DWORD dwl )
{
   LPTSTR   lps = GetNxtBuf();
   DWORD    dwk = strlen(lpl);
   DWORD    dwi;
   *lps = 0;
   if( ( dwl ) &&
      ( dwk ) &&
      ( dwl <= dwk ) )
   {
      if( dwl == dwk )  // is it right ALL
         dwi = 0;
      else
         dwi = dwk - dwl;
      strcpy(lps, &lpl[dwi] );
   }

   return lps;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Mid
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl
//            : DWORD dwb
//            : DWORD dwl
// Description: Returns a buffer containing the MIDDLE portion of a string.
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Mid( LPTSTR lpl, DWORD dwb, DWORD dwl )
{
   LPTSTR   lps = GetNxtBuf();
   DWORD    dwk = strlen(lpl);
   DWORD    dwi, dwr;
   //LPTSTR pt;
   *lps = 0;
   if( ( dwl ) && 
      ( dwb ) &&
      ( dwl ) &&
      ( dwb <= dwk ) &&
      ( dwl <= (dwk - (dwb - 1)) ) )
   {
      dwr = 0;
      for(dwi = (dwb - 1); (dwi < dwk), (dwr < dwl); dwi++ )
      {
//         pt = &lpl[dwi];
         lps[dwr++] = lpl[dwi];
      }
      lps[dwr] = 0;
   }
   return lps;
}

double get_percent2( DWORD max, DWORD amt )
{
    double d1, d2;
    int i;
    if(max == 0)
        return 100.0;

    d1 = (double)(amt * 100) / (double)max;
    i = (int)(d1 * 100.0);
    d2 = ((double)i / 100.0);
    if( d2 < 0.0000001 ) {
        i = (int)(d1 * 10000.0);
        d2 = ((double)i / 10000.0);
        if( d2 < 0.0000001 ) {
            i = (int)(d1 * 1000000.0);
            d2 = ((double)i / 1000000.0);
        }
    }
    return d2;
}

#ifdef WIN32
BOOL Get_FD_File_Time_Stg( PTSTR pDest, WIN32_FIND_DATA * pfd, BOOL bLocal )
{
    SYSTEMTIME  st;
    FILETIME    ft;
    BOOL        flg1, flg2;

    flg1 = flg2 = FALSE;
    if( bLocal ) {
        if( FileTimeToLocalFileTime(
            &pfd->ftCreationTime, // UTC file time to convert
            &ft ) )  // converted file time to LOCAL time
        {
            flg2 = TRUE;   // flag NOT UTC
        }
        else
        {
            // we have NO conversion
            flg1 = TRUE;
            // just move the UTC
            ft.dwHighDateTime = pfd->ftCreationTime.dwHighDateTime;
            ft.dwLowDateTime  = pfd->ftCreationTime.dwLowDateTime;
            flg2 = FALSE;
        }
    }
    else
    {
       // just move the UTC
       ft.dwHighDateTime = pfd->ftCreationTime.dwHighDateTime;
       ft.dwLowDateTime  = pfd->ftCreationTime.dwLowDateTime;
       flg2 = FALSE;
   }

   if( FileTimeToSystemTime( &ft, &st ) )  // receives system time
   {
       sprintf( EndBuf(pDest),
           " %02d/%02d/%02d %02d:%02d",
           (st.wDay & 0xffff),
           (st.wMonth & 0xffff),
           (st.wYear % 100),
           (st.wHour & 0xffff),
           (st.wMinute & 0xffff) );
         // ========================
         if( !flg2 )
            strcat(pDest, " UTC");
   }
   return (flg1 | flg2);
}
#endif

// return nice number - with comas
PTSTR My_NiceNumber( PTSTR lpn )
{
   size_t i, j, k;
   PTSTR lpr = GetNxtBuf();
   *lpr = 0;
   i = strlen( lpn );
	if( i ) {
      if( i > 3 ) {
			k = 0;
			for( j = 0; j < i; j++ ) {
            // FIX20070923 - avoid adding FIRST comma
				if( k && ( ( (i - j) % 3 ) == 0 ) )
					lpr[k++] = ',';
				lpr[k++] = lpn[j];
			}
			lpr[k] = 0;
		} else {
			strcpy( lpr, lpn );
		}
   }
   return lpr;
}

// given a DWORD number, get nice number in buffer
PTSTR My_NiceNumberStg( DWORD num )
{
   PTSTR lpr = GetNxtBuf();
   sprintf(lpr, "%u", num);
   return (My_NiceNumber(lpr));
}

// eof - DumpUtil.c
