
// Dump4.c

// A CONSOLE Application - main action is ProcessFile( LPTSTR pFileName )
// If the name is successfully openned for READING, then
// void	DoFile( char * fn, HANDLE hf ) will be CALLED to action it.
// Here it will be mapped by CreateFileMapping() and then
// MapViewOfFile() and the stucture is passed to
// ProcessDataStr( LPDFSTR lpdf ) where the various COMMAND switches are
// checked. If NONE then just a HEX DUMP as normal.

// Apr 2001 - Add -lib for processing an ARCHIVE (library *lib) file
// Feb 2001 - Add -prof for SYNEDIT profile dump in DumpSynE
// Sep 2000 - Add CAB directory listing
#include	"Dump4.h"
extern BOOL  ProcessWAV( LPDFSTR lpdf );
extern BOOL  ProcessDFS( LPDFSTR lpdf );
extern   LPTSTR   GetDWStg2( DWORD dwi );
extern   VOID     prt_init( VOID );

//extern   void  chkcis( BYTE * lpb, DWORD rd );
extern   int   chkcis( LPDFSTR lpdf );
extern   int   DoGeoff( LPDFSTR lpdf ); // -g gbDoGeoff = TRUE
extern   int	ChkVosTab( LPDFSTR lpdf ); // -dd do VOS table file
extern   BOOL  ProcessPPM( LPDFSTR lpdf );

extern   BOOL  grmCloseFile( HANDLE * ph );

#ifndef  NDEBUG
extern   void	WriteDiagFile( LPSTR lpd );
#endif   // !NDEBUG

// external
#ifdef   ADDCISD4    /* add CIS compuserve file support */

extern	UINT	IsCisFile( HANDLE hf, LPSTR lpf, LPSTR lpb,
						  DWORD len, DWORD fsiz );
extern	void	SetFileType( UINT typ, LPSTR lpb );
#endif   /* ADDCISD4    == add CIS compuserve file support */

extern   void  DeleteLineMem( void );
extern   void  OutLineMem( void );

// Forward reference
void	ProcessFile( char * fn );
void	ProcessDataStr( LPDFSTR lpdf );
void	ProcessData( HANDLE hf, LPTSTR fn, LPTSTR lpb, DWORD len,
				  DWORD fsiz );
extern	int	ProcessBMP( HANDLE hf, LPTSTR fn, LPTSTR lpb, DWORD len,
				  DWORD fsiz );

// data items
PWB   pWB = NULL;

VOID  FreeWB( VOID )
{
   if( pWB ) {
      if(g_pHexBlock) dMFREE(g_pHexBlock);
      g_pHexBlock = NULL;
      free(pWB);
   }
   pWB = NULL;
}

VOID pgm_exit( int i )
{
   FreeWB();
   kill_ptrlist();
   exit(i);
}

void	MyExit0( void )
{
	pgm_exit( 0 );
}

void	MyExit1( void )
{
	pgm_exit( 1 );
}

/* ==================================================================
   void	DoOutput( DWORD fOff )

   Purpose: Combine the OFFSET, the HEXADECIMAL and the ASCII display
      into a line and add MEOR (Cr/Lf)

   Controlled by three (3) switches -
   gfAddOffset - Add the offset into the file
   gfAddHex    - Add the HEXADECIMAL
   gfAddAscii  - Add the ASCII output
   Fix 4 July 2000 or F20000704
   If JUST gfAddAscii, then try to break lines ONLY when there is a
   "break" in the data. That is <= " " (less than or equal to a space)

   ================================================================== */
void	DoOutput( DWORD fOff )
{
	int		io;
	LPTSTR 	lpo = &gszTmpOut[0];
   LPTSTR   lpa = &gszALine[0];  // buffer of MXALINE chars

	io = 0;	// No output
   *lpo = 0;
	if( ( fOff >= gdwBgnOff ) &&
		 ( fOff <= gdwEndOff ) )
	{
		if( gfAddOffset )
		{
			sprintf( &gszOffset[0],
				"%04x:%04x ",
				((fOff & 0xffff0000) >> 16),
				(fOff & 0x0000ffff) );
			strcat( lpo, &gszOffset[0] );
			io++;
		}
		if( gfAddHex )
		{
			strcat( lpo, &gszHex[0] );
			io++;
		}
		if( gfAddAscii )
		{
#ifdef   F20000704
         int   il, ia, j;
         TCHAR c;
         if( io )
         {
			   strcat( lpo, &gszAscii[0] );
			   io++;
         }
         else
         {
            // this is the ONLY output
            il = lstrlen( &gszAscii[0] );
            ia = lstrlen( lpa );
            if( ia )
            {
               for( j = 0; j < il; j++ )
               {
                  c = gszAscii[j];
                  //if( ( c <= ' ' ) || ( c == '_' ) || ( c == '.') || (ia >= MXALINE) )
                  if( ( ( ia > 16 ) && ( ( c <= ' ' ) || ( c == '_' ) || ( c == '.') ) ) ||
                     (ia >= MXALINE) )
                  {
                     lpa[ia++] = c;
                     lpa[ia] = 0;
                     strcat(lpo,lpa);
                     io++;
                     *lpa = 0;
                     ia = 0;
                     j++;
                     if( j < il )
                     {
                        strcpy(lpa, &gszAscii[j] );
                     }
                     break;
                  }
                  else
                  {
                     lpa[ia++] = c;
                  }
               }
            }
            else
            {
               // this is the first
               strcpy(lpa, &gszAscii[0]);
            }
         }
#else /* !F20000704 */
			strcat( lpo, &gszAscii[0] );
			io++;
#endif   /* F20000704 */

		}
	}
	if( io )	// If we HAVE output
	{
		// Append a LF
		strcat( lpo, "" MEOR );
		prt( lpo );	// Out it
	}
}

//typedef  struct {
//   char *   fn;
//   HANDLE   hf;
//   ULARGE_INTEGER qwSize;
//   BYTE *   lpb;
//   DWORD    dwmax;
//   DWORD    dwrd;
//   LPTSTR   lptmp;
//   HANDLE   df_hMap;    // mapping handle
//   PVOID    df_pVoid;   // mapping pointer
//}DFSTR;
//typedef DFSTR * LPDFSTR;

//#define g_flProtext     PAGE_READONLY
//#define g_ReqAccess     FILE_MAP_READ

//DFSTR gsDoFil;

#ifdef WIN32
////////////////////////////////////////////////////////////////////
void	DoFile( char * fn, HANDLE hf )
{
	//int			rd;
   LPDFSTR     lpdf = &gsDoFil;
   DWORD       dwMax;
//	int			ir;
	//char	*	lpb;
//	char		c, d;
//	DWORD		foff;
	DWORD		   fsiz;
//	int			ia, fix;
//	char	*	lph;
//	UINT		ft;
	LPTSTR	   lptmp;
   PBYTE       pb = 0;	
   // solid memory - allocated as part of file size
//	lpb = &gcFilBuf[0];  // of MXFIO

	//lpb  = (char *)&g_bfilbuf[0];  // of MMXFIO
   //dwMax = MMXFIO;
	lptmp = &gszTmpOut[0];

   lpdf->fn    = fn;
   lpdf->lptmp = lptmp;
   //lpdf->lpb   = (PBYTE)lpb;
   //lpdf->dwmax = dwMax;
   if( VFH(hf) )
	{

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
      if( lpdf->df_hMap )
      {
         lpdf->df_pVoid = MapViewOfFile( lpdf->df_hMap, // file-mapping object to map
               g_ReqAccess,   // access mode
               0,    // high-order 32 bits of file offset
               0,    // low-order 32 bits of file offset
               0 );  // number of bytes to map
         if( lpdf->df_pVoid )
         {
            lpdf->lpb = (PBYTE)lpdf->df_pVoid;
            pBaseLoad = lpdf->lpb;
            pBaseTop = pBaseLoad + dwMax;

//		foff = 0;
//		lph = &gszHex[0];
//		*lph = 0;
//		fix = 0;
//   			if( giVerbose > 1 )

   			if( giVerbose )
   			{
//               sprintf( lptmp,
//   						"Total Size is %I64u bytes (mapped to %#x)." MEOR,
//                     lpdf->qwSize,
//                     lpdf->df_pVoid );
       			if( giVerbose > 1 )
               {
                  sprintf( lptmp,
   						"File [%s], %I64u bytes (map at %#x)." MEOR,
                     fn,
                     lpdf->qwSize,
                     lpdf->df_pVoid );
               }
               else
               {
                  sprintf( lptmp,
   						"File [%s], %I64u bytes." MEOR,
                     fn,
                     lpdf->qwSize );
               }
					prt( lptmp );
   			}

      /* NOW NOT NEEDED ====================================
      if( ( lpdf->qwSize.HighPart == 0   ) &&
         ( lpdf->qwSize.LowPart > MMXFIO ) )
      {
         // consider buffer allocation greater than 4 K, maybe up to 16 K today
         if( lpdf->qwSize.LowPart < (MMXALLOC-256) )
         {
            DWORD dwm = lpdf->qwSize.LowPart + 256;
            pb = LocalAlloc( LPTR, dwm );
            if( pb )
            {
               lpb = (char *)pb;
               dwMax = lpdf->qwSize.LowPart;
               lpdf->lpb = pb;
               lpdf->dwmax = dwMax;
            }
            if(!pb)
               chkme( "reallocation failed !!!" );
         }
      }
      
		// read a block of the file
//		if( rd = _lread( hf, lpb, MXFIO ) )
//		if( rd = grmReadFile( (HANDLE)hf, lpb, MXFIO ) )
		rd = grmReadFile( (HANDLE)hf, (PBYTE)lpb, dwMax );
		if( rd )
		{
         lpdf->dwrd = rd;
			if( giVerbose > 1 )
			{
				if( fsiz != (DWORD)-1 )
				{
					sprintf( lptmp,
						"Total Size is %s (0x%04X) bytes. Read %d to buffer" MEOR,
						GetDWStg2(fsiz), fsiz, rd );
					prt( lptmp );
				}
			}
      =========================================== */
            ProcessDataStr( lpdf );

   			if( VERB )
	   		{
                if(( gdwEndOff ) && ( gdwEndOff < fsiz ) )
                {
                    sprintf( lptmp, "Done [%s] Ended after %s byte offset. ",
                        fn,
                        GetDWStg2(gdwEndOff) );
                } else {
                    sprintf( lptmp, "Completed [%s] = %u Bytes. ",
                        fn, fsiz );
                }
                if ( lpdf->stat_res == 0 )
                {
                    char timebuf[32];
                    int err = ctime_s(timebuf, 26, &lpdf->stat_buf.st_mtime);
                    if (err == 0)
                    {
                        err = (int)strlen(timebuf);
                        while (err--) {
                            if (timebuf[err] > ' ')
                                break;
                            timebuf[err] = 0;
                        }
                        if (err) {
                            sprintf(EndBuf(lptmp), " %s", timebuf);
                        }
                    }
                }
                strcat(lptmp,MEOR); // terminate the string
                prt( lptmp );
            }

            if( lpdf->df_pVoid )
               UnmapViewOfFile( lpdf->df_pVoid );  // starting address

            if( lpdf->df_hMap )
               CloseHandle( lpdf->df_hMap );

            lpdf->df_pVoid = NULL;
            lpdf->df_hMap = 0;
         }
         else
         {
            if( VERB )
   			{
   				sprintf( lptmp, "ERROR: Unable to get MAP View of File" MEOR
                  "\t[%s]!" MEOR, fn );
   				prt( lptmp );
   			}
            CloseHandle( lpdf->df_hMap );
         }
		}
		else
		{
#ifndef  USEMAPPING
         if( VERB )
			{
				sprintf( lptmp, "ERROR: Unable to MAP File" MEOR
               "\t[%s]!" MEOR, fn );
				prt( lptmp );
			}
#else // !USEMAPPING
         if( VERB )
			{
				sprintf( lptmp, "WARNING: File [%s] is NULL!" MEOR, fn );
				prt( lptmp );
			}
#endif   // #ifndef  USEMAPPING
		}
	}
	else
	{
		if( giVerbose )
		{
			sprintf( lptmp, "ERROR: Unable to OPEN file [%s]!" MEOR, fn );
			prt( lptmp );
		}
	}


   if(pb)
     LocalFree(pb);

}
////////////////////////////////////////////////////////////////////
#else // !#ifdef WIN32
////////////////////////////////////////////////////////////////////
void	DoFile( char * fn, HANDLE hf )
{
    LPTSTR	   lptmp = &gszTmpOut[0];
	sprintf( lptmp, "ERROR: This service has to be ported to unix [%s]!" MEOR, fn );
	prt( lptmp );
}
////////////////////////////////////////////////////////////////////
#endif // #ifdef WIN32 y/n

int	GetDir( LPSTR lpDest, LPSTR lpSrc )
{
	int	i, j;
	char	c;
   int   k = 0;
	if( lpDest && lpSrc )
	{
		strcpy( lpDest, lpSrc );
		i = lstrlen( lpDest );
		if( i )
		{
			for( j = (i - 1); j > 0; j-- )
			{
				c = lpDest[j];
				//if( ( c == ':' )
            if( (  c == '/' ) ||
					(  c == '\\' ) )
				{
               k = j + 1;
//					lpDest[j+1] = 0;
					break;
				}
			}
		}
      lpDest[k] = 0;
	}
   return k;
}

BOOL	GotWild( LPSTR lpf )
{
	BOOL	flg;
	BOOL	flg2;
	int		i, j;
	char	c;

	flg = FALSE;
	flg2 = FALSE;
   i = 0;
	if( lpf )
      i = lstrlen( lpf );
   if(i)
	{
		for( j = 0; j < i; j++ )
		{
			c = lpf[j];
			if( ( c == '?' ) || ( c == '*' ) )
			{
				flg = TRUE;
				break;
			}
			if( c == '.' )
				flg2 = TRUE;
		}
	}
	return flg;
}

void	ProcessFile( LPTSTR fn )
{
    HANDLE		hf;
	LPTSTR		lpb;
	LPTSTR		lpf, lpd;
	DWORD			pCnt;
	WIN32_FIND_DATA	fd;
	HANDLE			hFind;
    LPDFSTR     lpdf = &gsDoFil;

	lpb = &gszDiag[0];
	pCnt = 0;
#if defined(USE_FIND_FIRST) || defined(_WIN32)
    hFind = FindFirstFile( fn, &fd );
    if( VFH(hFind) ) {
        if( giVerbose ) {
            ULARGE_INTEGER ul;
            ul.LowPart  = fd.nFileSizeLow;
            ul.HighPart = fd.nFileSizeHigh;
            sprintf( lpb, "Processing File [%s], %I64d bytes..." MEOR,
                fn,
                ul );
        }
        FindClose( hFind );
    } else {
        if( giVerbose ) {
            sprintf( lpb, "Processing File [%s]..." MEOR, fn );
            prt( lpb );
        }
    }
    lpdf->stat_res = stat(fn, &lpdf->stat_buf);
#else
    lpdf->stat_res = stat(fn, &lpdf->stat_buf);
    if (lpdf->stat_res)
    {
        if (giVerbose) {
            sprintf(lpb, "stat FAILED on File [%s]..." MEOR, fn);
            prt(lpb);
        }
    } 
    else
    {
        sprintf(lpb, "Processing File [%s], %zd bytes..." MEOR,
            fn,
            lpdf->stat_buf.st_size);
        if (giVerbose) {
            prt(lpb);
        }
    }
#endif

	hf = grmOpenFile( fn, &hf, OF_READ );
    if( VFH(hf) ) {
        pCnt = 1;
        if( gfDoCABFile ) {
            grmCloseFile( &hf );
            ProcessCAB( fn );
        }
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
        else if( gfDoSHPFile ) { //   "  -shp    Dump as SHAPEFILE ..." MEOR
            grmCloseFile( &hf );
            if( ProcessSHP( fn ) )
                return;  // all done
      	    hf = grmOpenFile( fn, &hf, OF_READ );
            if(VFH(hf)) {
                goto Do_FILE;
            } else {
                goto Try_WILD;
            }
        }
#endif // #ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
        else
        {
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
Do_FILE:
#endif
		    DoFile( fn, hf );
            grmCloseFile( &hf );
        }
	} else {
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
Try_WILD:
#endif
        gbGotWild = GotWild(fn);
        if(( gbGotWild ) && ( VERB ) ) {
            sprintf( lpb, "ADVICE: Using [%s] for searching ..." MEOR, fn );
			prt( lpb );
        } else if(VERB) {
            sprintf( lpb, "WARNING: Open file [%s] FAILED!" MEOR, fn );
			prt( lpb );
#if defined(USE_FIND_FIRST) || defined(_WIN32)
            hFind = FindFirstFile( fn, &fd );
            if( VH(hFind) ) {
                prt( "This CAN not happen! Is file LOCKED by another process?" MEOR );
                FindClose(hFind);
             } else {
                 LPTSTR   pbuf = &g_cBuf[0];
                 LPTSTR   p;
                 strcpy(pbuf,fn);
                 p = strrchr(pbuf, '\\');
                 while(p)
                 {
                     *p = 0;
                     hFind = FindFirstFile( pbuf, &fd );
                     if( VH(hFind) )
                     {
                         sprintf( lpb,
                             "ADVICE: Did find [%s]!" MEOR
                             "   Appears an error in [%s]?" MEOR,
                             pbuf,
                             &fn[ (strlen(pbuf) + 1) ] );
                         prt( lpb );
                         FindClose(hFind);
                         break;
                     }
                     p = strrchr(pbuf, '\\');
                 }
            }
#endif
        }

//typedef struct _WIN32_FIND_DATA { // wfd  
//  DWORD dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    DWORD    nFileSizeHigh; 
//    DWORD    nFileSizeLow; 
//    DWORD    dwReserved0; 
//    DWORD    dwReserved1; 
//    TCHAR    cFileName[ MAX_PATH ]; 
//    TCHAR    cAlternateFileName[ 14 ]; 
//} WIN32_FIND_DATA;
//		if( GotWild( fn ) )
		if( gbGotWild )
		{
#ifdef _WIN32
			lpf = &gszNxtFile[0];
			lpd = &gszCurDir[0];
			GetDir( lpd, fn );
			if( giVerbose > 8 )
			{
				strcpy( lpb, "ADVICE: Trying FindFirstFile..." MEOR );
				prt( lpb );
			}
			hFind = FindFirstFile( fn, &fd );
         if( VH(hFind) )
			{
				if( giVerbose > 8 )
				{
					sprintf( lpb, "ADVICE: Got FIND Handle %X..." MEOR,
						hFind );
					prt( lpb );
				}
				//if( (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) ||
				//	(fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) )
				// while( FindNextFile( hFind, &fd ) );
            do
            {
					if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						strcpy( lpf, lpd );
						strcat( lpf, &fd.cFileName[0] );
						ProcessFile( lpf );
						pCnt++;
					}
				}	while( FindNextFile( hFind, &fd ) );

            // no more FOUND

				FindClose( hFind );
				if(( pCnt == 0 ) &&
					( VERB      ) )  // giVerbose > 8 ) )
				{
					sprintf( lpb, "WARNING: Unable to FIND any file(s)\nlike [%s]!" MEOR,
						fn );
					prt( lpb );
				}
			}
			else
			{
				if( giVerbose > 8 )
				{
					sprintf( lpb, "WARNING: Unable to OPEN/FIND file(s)\nlike [%s]!" MEOR, fn );
					prt( lpb );
				}
			}
#else // !_WIN32
            sprintf(lpb, "ERROR: Wildcard NOT yet suuported in NOT WIN32! '%s" MEOR, fn);
            prt(lpb);
#endif // _WIN32 y/n

		}
		else
		{
			if( giVerbose )
			{
				sprintf( lpb, "WARNING: Unable to OPEN file [%s]!" MEOR, fn );
				prt( lpb );
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : grmOpenFile
// Return type: HANDLE 
// Arguments  : LPTSTR fn
//            : HANDLE * lpv
//            : UINT uFlag
// Description: Open an EXISTING file, and
//              return the HANDLE
///////////////////////////////////////////////////////////////////////////////
HANDLE	grmOpenFile( LPTSTR fn, HANDLE * ph, UINT uFlag )
{
	HANDLE	h = 0;
#ifdef WIN32
	DWORD	dwa = GENERIC_READ;
	if( uFlag )
		dwa |= GENERIC_WRITE;
	if( ( fn  ) &&
		( *fn ) )
	{
		h = CreateFile( fn,		// pointer to name of the file
			dwa,	// access (read-write) mode
			0,		// share mode
			NULL,	// pointer to security attributes
			OPEN_EXISTING,	// how to create
			FILE_ATTRIBUTE_NORMAL,	// file attributes
			NULL );	// handle to file with attributes to copy
      *ph = h;
	}
#else
    FILE *fp = 0;
    char *mode = "r";
    if (uFlag)
        mode = "w";
    if ((fn) && (*fn)) {
        fp = fopen(fn,mode);
        if (fp) {
            h = (HANDLE)fp;
            *ph = h;
        }
    }
#endif
	return h;
}

HANDLE	grmCreateFile( LPTSTR fn )
{
	HANDLE	h = 0;
#ifdef WIN32
	DWORD	dwa = (GENERIC_READ|GENERIC_WRITE);
	if( ( fn  ) &&
		( *fn ) )
	{
		h = CreateFile( fn,		// pointer to name of the file
			dwa,	// access (read-write) mode
			0,		// share mode
			NULL,	// pointer to security attributes
			CREATE_ALWAYS,	// how to create
			FILE_ATTRIBUTE_NORMAL,	// file attributes
			NULL );	// handle to file with attributes to copy
	}
#else
	if( ( fn  ) &&
		( *fn ) )
	{
        h = (HANDLE)fopen(fn,"w");	
	}
#endif
	return h;
}
 

DWORD	grmReadFile( HANDLE hf, BYTE * lpb, DWORD dwMax )
{
	DWORD	dwr = 0;
	if( ( hf               ) &&
		( hf != (HANDLE)-1 ) &&
		( lpb              ) &&
		( dwMax            ) )
	{
#ifdef WIN32	
		ReadFile( hf,	// handle of file to read
			lpb,		// address of buffer that receives data
			dwMax,		// number of bytes to read
			&dwr,		// address of number of bytes read
			NULL );		// address of structure for data
#else
        dwr = fread(lpb,1,dwMax,hf);
#endif			
	}
	return dwr;
}

#define  DO_OUT \
{\
   *lpo = 0;\
	if(bAddO) sprintf( lpo, "%04x:%04x ", ((foff & 0xffff0000) >> 16), (foff & 0x0000ffff) );\
	if(bAddH) strcat( lpo, lph );\
	if(bAddA) strcat( lpo, lpa );\
   if(*lpo)  strcat(lpo,MEOR); prt(lpo);\
}

// show hex outhex out hex hex out hex show process hex
void ProcessHex( PBYTE pb, DWORD len )
{
   char c, d;
	LPTSTR	lph = &gszHex[0];
   LPTSTR   lpa = &gszAscii[0];
	LPTSTR 	lpo = &gszTmpOut[0];
   BOOL     bPrt  = gw_fUsePrintf;
	BOOL     bAddH = gfAddHex;
   BOOL     bAddA = gfAddAscii;
   BOOL     bAddO = gfAddOffset;
   DWORD    dwi;
   DWORD    ia, fix;
   DWORD64  foff = (DWORD64)pb;

   ia = fix = 0;
   *lph = 0;
   for( dwi = 0; dwi < len; dwi++ )
   {
      c = pb[dwi];   // get character
  		if( c < ' ' )
  			d = '.';
  		else if( c >= 0x7f )
  			d = '.';
  		else
  			d = c;
      lpa[ia+fix] = d;	// Fill in the ASCII
   	if( bPrt && ( d == '%' ) )	// Care using PRINTF
   	{
         lpa[ia+fix] = d;	// Put in 2 for PRINTF
         fix++;
   	}
   	ia++;    // bump ascii pointer
	   sprintf( EndBuf(lph), "%02X ", (c & 0xff) );
      if( ia == 8 )
         strcat( lph, " " );
      if( ia == 16 )
      {
         *lpo = 0;
  			lpa[ia+fix] = 0;
         if(bAddO) sprintf( lpo, "%04x:%04x ", ((foff & 0xffff0000) >> 16), (foff & 0x0000ffff) );
         if(bAddH) strcat( lpo, lph );
         if(bAddA) strcat( lpo, lpa );
         if(*lpo) {
            strcat(lpo,MEOR);
            prt(lpo);
         }
         foff += ia;
         ia = fix = 0;
         *lph = 0;
      }
   }
   if(ia)
   {
		lpa[ia+fix] = 0;
      while(ia < 16)
      {
         strcat(lph,"   ");
      	ia++;    // bump ascii pointer
         if( ia == 8 )
            strcat( lph, " " );
      }
      *lpo = 0;
		lpa[ia+fix] = 0;
      if(bAddO) sprintf( lpo, "%04x:%04x ", ((foff & 0xffff0000) >> 16), (foff & 0x0000ffff) );
      if(bAddH) strcat( lpo, lph );
      if(bAddA) strcat( lpo, lpa );
      if(*lpo) {
         strcat(lpo,MEOR);
         prt(lpo);
      }
   }
}


/* ==============
#define  ISNUM(a)    ( ( a >= '0' ) && ( a <= '9' ) )
#define  ISUP(a)     ( ( a >= 'A' ) && ( a <= 'Z' ) )
#define  ISLOW(a)    ( ( a >= 'a' ) && ( a <= 'z' ) )
#define  ISPRT(a)  ( ISNUM(a) || ISUP(a) || ISLOW(a) || (a == '_') )
#define  ISSIGCHAR(a)   ( ( a >= ' ' ) && ( a < 0x7f ) )
  ================ */

#define  BGNCHK   60
#define  CHKTWO   70
#define  CHKEND   78

VOID  stophere(VOID)
{
   int i;
   i = 0;
}

VOID	ProcessAscii( LPTSTR lpb, DWORD rd )
{
   char		c, d, pc;
	LPTSTR 	lpo = &gszTmpOut[0];
	DWORD		ir, ia, foff, dwi, ip;
   DWORD    dwBgn = gdwBgnOff;
   DWORD    dwEnd = gdwEndOff;
   LPTSTR   pstr;
   BOOL     bUni = gfUnicode;  // be UNICODE aware

	foff = 0;
	ip = ia = 0;
   pc = (char)-1;        // no previous
   for( ir = 0; ir < rd; ir++ )
	{
   	if( ( foff >= dwBgn ) &&
	   	 ( foff <= dwEnd ) )
      {
   		c = lpb[ir];   // get character (BYTE)

         if( c == 'g' )
            stophere();

   		if( c < ' ' )
         {
   			d = '.';
            if( bUni )
            {
               if( ISSIGCHAR(pc) )
               {
                  pc = c;
                  continue;
               }
            }
         }
   		else if( c >= 0x7f )
         {
   			d = '.';
         }
   		else
         {
            // is within printable character range
            //if( bUni )
            //{
            //   d = c;
            //}
            //else
            //{
               d = c;
            //}
         }

         ip = ia;       // before the increment
    		lpo[ia++] = d;	// Fill in the ASCII

         if( ia > BGNCHK )
         {
            if( !ISPRT(c) )
            {
               lpo[ia] = 0;
               strcat(lpo,MEOR);
               prt(lpo);
               ia = 0;
            }
            else if( ia > CHKEND )
            {
               // first thing is to go BACKWARD looking for a good break
               dwi = ia;
               lpo[ia] = 0;
               while(dwi--)
               {
                  c = lpo[dwi];
                  if( !ISPRT(c) )
                  {
                     dwi++;
                     pstr = Right(lpo, (ia - dwi));
                     lpo[dwi] = 0;
                     strcat(lpo,MEOR);
                     prt(lpo);
                     strcpy(lpo,pstr);
                     ia = strlen(lpo);
                     break;
                  }
               }
               if(dwi == (DWORD)-1)
               {
                  strcat(lpo,MEOR);
                  prt(lpo);
               }
            }
         }

         pc = c;  // keep the PREVIOUS

      }  // if within the given RANGE
	}  // for size of READ = for( ir = 0; ir < rd; ir++ )

	if( ia )	// Remaining data
	{
		lpo[ia] = 0;
      strcat(lpo,MEOR);
      prt(lpo);
	}
}


void	ProcessData_ORG( HANDLE hf, LPTSTR fn, LPTSTR lpb, DWORD len, DWORD fsiz )
{
	char		c, d;
	DWORD		rd = len;
	LPTSTR	lph;
	DWORD		foff, ir;
	int		fix, ia;
	
	foff = 0;
	lph = &gszHex[0];
	*lph = 0;
	fix = 0;
//	ft = IsCisFile( hf, fn, lpb, rd, fsiz );
//	SetFileType( ft, lptmp );

	ia = 0;

			for( ir = 0; ir < rd; ir++ )
			{
				c = lpb[ir];   // get character
				if( gfRemPar )
				{
					c = (char)(c & 0x7f);
				}
				if( c < ' ' )
					d = '.';
				else if( c >= 0x7f )
					d = '.';
				else
					d = c;

				gszAscii[ia+fix] = d;	// Fill in the ASCII
				ia++;
				if( gw_fUsePrintf &&
					( d == '%' ) )	// Care using PRINTF
				{
					gszAscii[ia+fix] = d;	// Put in 2 for PRINTF
					fix++;
				}

				sprintf( EndBuf(lph),
					"%02X ",
					(c & 0xff) );

				if( ia == 8 )
					strcat( lph, " " );

				if( ia == 16 )
				{
					gszAscii[ia+fix] = 0;

					DoOutput( foff );

					foff += ia;
					ia = 0;
					fix = 0;
					*lph = 0;
				}

			}  // for size of READ = for( ir = 0; ir < rd; ir++ )

#ifndef  USEMAPPING
			if( rd == MXFIO )
			{
//				while( rd = _lread( hf, lpb, MXFIO ) )
				rd = grmReadFile( (HANDLE)hf, (PBYTE)lpb, MXFIO );
				while( rd )
				{
					for( ir = 0; ir < rd; ir++ )
					{
						c = lpb[ir];
						if( gfRemPar )
						{
							c = (char)(c & 0x7f);
						}
						if( c < ' ' )
							d = '.';
						else if( c >= 0x7f )
							d = '.';
						else
							d = c;

						gszAscii[ia+fix] = d;	// Fill in the ASCII
						ia++;
						if( d == '%' )	// Care using PRINTF
						{
							gszAscii[ia+fix] = d;
							fix++;
						}

						sprintf( (lph + lstrlen( lph )),
							"%02X ",
							(c & 0xff) );

						if( ia == 8 )
							strcat( lph, " " );

						if( ia == 16 )
						{
							gszAscii[ia+fix] = 0;
							DoOutput( foff );
							foff += ia;
							ia = 0;
							fix = 0;
							*lph = 0;
                     if( ( gdwEndOff ) &&
                        ( foff > gdwEndOff ) )
                     {
                        // all done with processing
                        break;
                     }
						}
					}	// for( ir = 0; ir < rd; ir++ ) process the block
               // =================================================
               if( ( gdwEndOff ) &&
                  ( foff > gdwEndOff ) )
               {
                  // all done with processing
                  break;
               }
               // =================================================
   				rd = grmReadFile( (HANDLE)hf, (PBYTE)lpb, MXFIO );
				}	// while read data
			}	// if MAX read

#endif   //#ifndef  USEMAPPING

			if( ia )	// Remaining data
			{
				foff += ia;
				gszAscii[ia+fix] = 0;
				if( ia < 8 )
					strcat( lph, " " );
				while( ia < 16 )
				{
					strcat( lph, "   " );
					ia++;
				}

				DoOutput( foff );

				ia = 0;
				fix = 0;
				*lph = 0;
			}

   UNREFERENCED_PARAMETER(fsiz);
   UNREFERENCED_PARAMETER(fn);

}


//extern   void  chkcis( BYTE * lpb, DWORD rd );
//extern   int  chkcis( LPDFSTR lpdf );
//extern   int  DoGeoff( LPDFSTR lpdf ); // -g gbDoGeoff = TRUE
//               else if( c == 'X' )
//               {
//                  cp++;
//               }

// FIX20080507 - Use DumpPE (from PEDUMP) to process binary LIB/EXE/DLL stuff
// -lib = g_bDumpLib -exe, -obj, -dll = g_bDumpObj
int ProcessPE(  LPDFSTR lpdf )
{
   dwFileSizeLow = lpdf->dwmax;
   dwFileSizeHigh = 0;
   pBaseLoad = (unsigned char *)lpdf->df_pVoid;
   pBaseTop = pBaseLoad + dwFileSizeLow;
#ifdef   USE_PEDUMP_CODE
   if(VERB9)
      Set_PEDUMP_A(TRUE);
   ShowCurrPEOpts();
   return DumpMemoryMap( lpdf->df_pVoid, lpdf->fn, lpdf->dwmax );
#else
   sprtf("PE Dump NOT supported\n");
   return 0;
#endif
}

void	ProcessDataStr( LPDFSTR lpdf )
{
   HANDLE     hf;
   LPTSTR    fn;
   BYTE *    lpb;
   DWORD     rd;
   DWORD     fsiz, bsiz;

   hf   = (HANDLE)lpdf->hf;
   fn   = lpdf->fn;
   lpb  = lpdf->lpb;
   rd   = lpdf->dwrd;
   bsiz = lpdf->dwmax;  // get the READ buffer full size
   fsiz = lpdf->qwSize.LowPart;

#ifdef   USEMAPPING
   if( bsiz == (DWORD)-1 )
   {
      // BIG BIG FILE CHECK
      chkme( "Code is NOT yet fixed for such a LARGE file!!!" );
   }
#else // !USEMAPPING
   if(( lpdf->qwSize.HighPart ) ||
      ( fsiz > bsiz ) )
   {
      // BIG BIG FILE CHECK
      // ( fsiz > MXFIO ) )
      chkme( "Seems we should allocate memory for the read buffer!!!" );
   }
#endif   // #ifdef   USEMAPPING

	if( gfDoASCII )
   {
	   ProcessASCII( lpdf );
      return;
   }
   if( gfDoCISAddr )
   {
      if( chkcis( lpdf ) == 0 )
      {
         // all done
         return;
      }
   }

   if( gbDoGeoff )
   {
      if( DoGeoff( lpdf ) == 0 )
      {
         // all done
         return;
      }
   }

   if( gfDoVosTab )
   {
      if( ( ChkVosTab(lpdf) ) ||
         ( gbGotWild ) ||
         ( gfDoVosOnly ) )
      {
         // then finish here
         return;
      }
   }

//   if( gfDoCABFile )
//   {
//      if( ProcessCAB( fn ) )
//         return;
//   }
   if( gbDoSynEdit )
   {
      if( ChkSynEdit(lpdf) )
         return;
   }

	if( gfDoBMP )
   {
	   if( ProcessBMP( hf, fn, (char *)lpb, rd, fsiz ) )
         return; // all DONE
   }

   if( g_bDumpLib )
   {
#ifdef USE_PEDUMP_CODE // FIX20080507
      if( ProcessPE( lpdf ) ) // FIX20080507 - Use DumpPE (PEDUMP)
#else // !#ifdef USE_PEDUMP_CODE // FIX20080507
      if( ProcessLIB( lpdf ) )
#endif // #ifdef USE_PEDUMP_CODE // FIX20080507 y/n
         return;  // all DONE
   }
#ifdef  ADDLNK // added Nov 2006
   if( g_bDumpLNK )
   {
      if( ProcessLNK( lpdf ) )
         return;  // all DONE
   }
#endif  // ADDLNK

   if( g_bDumpMK4 )
   {
      if( ProcessMK4( lpdf ) )
         return;  // all DONE
   }

   if( g_bDoPPM )
   {
      if( ProcessPPM( lpdf ) )
         return;  // all done
   }
   if( g_bDoM2TS )
   {
      if( DumpM2TS( lpdf ) )
         return;  // all done
   }
#ifdef ADD_AVI_FILE
    // BOOL w_bDoAVI; // g_bDoAVI - process as an AVI file
   if( g_bDoAVI )
   {
      if( DumpAVI( lpdf ) )
         return;  // all done
   }
#endif // #ifdef ADD_AVI_FILE

#ifdef   ADDOBJSW
   if( g_bDumpObj )
   {
#ifdef USE_PEDUMP_CODE // FIX20080507
      if( ProcessPE( lpdf ) ) // FIX20080507 - Use DumpPE (PEDUMP)
#else // !#ifdef USE_PEDUMP_CODE // FIX20080507
      if( ProcessOBJ( lpdf ) )
#endif // #ifdef USE_PEDUMP_CODE // FIX20080507 y/n
         return;  // all done
   }
#endif   // #ifdef   ADDOBJSW

   if( g_bDumpGif )
   {
      if( ProcessGIF( lpdf ) )
         return; // all done
   }

#ifdef   DUMP_RGB2
   if( g_bDumpRGB ) {
      if( ProcessRGB( lpdf ) )
         return;  // all done
   }
#endif   // #ifdef   DUMP_RGB2

   if( gfDoDFS ) {
      if( ProcessDFS( lpdf ) )
         return;  // all done
   }

#ifdef ADD_TAR_FILE  // FIX20080908 - Add -tar - dump TAR format files
   if( gfDoTARFile ) { // "  -tar    Dump as TAR file ...
      if( ProcessTAR( lpdf ) )
         return;
   }
#endif // #ifdef ADD_TAR_FILE
#ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file
    if ( gfDoSonic ) {
        if ( ProcessSONIC( lpdf ) )
            return;
    }
#endif // #ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file

   if( gfDoWAV ) { // "  -wav    Dump as WAVE file ...
      if( ProcessWAV( lpdf ) )
         return;
   }
   // ELSE just do it as a HEX DUMP ***********
   if( gfAddAscii && !gfAddHex && !gfRemPar && !gw_fUsePrintf && !gfAddOffset )
      ProcessAscii( (LPTSTR)lpb, (DWORD)rd );
   else
      ProcessData( hf, fn, (char *)lpb, rd, fsiz );
   // ==========================================
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : main
// Return type: int 
// Arguments  : int argc
//            : char *argv[]
// Description: standard OS entry
//              
///////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
	int     retv;
	int		i;
	char *	fn;
	LPSTR	lpb, lpc;

   pWB = malloc( sizeof(WB) );
   if( !pWB )
   {
      printf( "ERROR: Memory allocation FAILED!" );
      return( (int)-1 );
   }

   ZeroMemory( pWB, sizeof(WB) );
   prt_init();

   // sprtf( "Begin Dump4 (CONSOLE APP.) work %d bytes." MEOR, sizeof(WB) );

	retv = 0;

   // some pointer to self
   glpOut = &gcOutBuf[0];
   glpOut2 = &gcOutBuf2[0];
   gdwOutSz = MXOUTBUF;

   gfAddOffset = TRUE;
   gfAddHex    = TRUE;  // = W.w_fAddHex    // add the HEX
   gfAddAscii  = TRUE;  // = W.w_fAddAscii  // add the ASCII
	giVerbose   = DEF_VERB;
   giMinASCII  = ASCII_MIN;   // minimum ASCII length

	gdwBgnOff   = 0;
	gdwEndOff   = (DWORD)-1;
   gszALine[0] = 0;  // buffer of MXALINE chars
   gbDoExtra   = TRUE;  // -gx[...]
   gbChkPWD    = TRUE;  // -gp[0|1|+|-]
   gbIgnoreMNU = TRUE;  // -gi[...] ignore 123ABC

	lpb = &gw_szcwd[0];
	_getcwd( lpb, 256 );

   GetModuleFileName( NULL, gszModule, 256 );

	//lpb = &gcFilBuf[0];
   lpb = &gszDiag[0];

	if( argc < 2 )
		Usage0();

	//lpc = malloc( (MXCMDBUF+260) );
	lpc = &gw_szCmd[0];
	if( lpc )
		strcpy( lpc, "Command:[" );

	ProcessCommand( argc, argv, lpc );

	if( VERB )  // ie giVerbose > 1
	{
		if( ( lpc ) &&
			( VERB4 ) )    // = giVerbose >= 9 
		{
			strcat( lpc, "]" MEOR );
			prt( lpc );
		}

      *lpb = 0;
		if( giVerbose > 2 )
		{
			//_getcwd( lpb, 256 );
			sprintf( lpb,
				"Current Work Directory =[%s]." MEOR,
				&gw_szcwd[0] );
			prt(lpb);
         *lpb = 0;
		}
      if( gfDoASCII )
      {
			sprintf( EndBuf(lpb), "Extract %s only (Min. length %d)." MEOR,
                (gfDoASCII2 ? "alphanumeric" : "ASCII"),
                giMinASCII );
		   sprintf( EndBuf(lpb), "Begin at %u ", gdwBgnOff );
			if( gdwEndOff != (DWORD)-1 )
            sprintf( EndBuf(lpb), "End at %u ", gdwEndOff );
         else
            strcat( lpb, "End at END " );
         strcat( lpb, MEOR );
      }
      else if( gfDoBMP )
		{
			strcpy( lpb, "Display as a BITMAP only." MEOR );
		}
		else
		{
         if( gbDoGeoff )
         {
            strcpy( lpb, "Telephone List -g" );

         }
         else if( gfDoVosTab )
         {
            if( gfDoVosOnly )
               strcpy( lpb, "View as a VOS Table file only -DDO" MEOR );
            else
               strcpy( lpb, "Try first as a VOS Table file -DD" MEOR );

         }
         else if(gfDoCABFile)
         {
            strcpy( lpb, "List files in MS CABINET file (-CAB)" MEOR );
         }
#ifdef   ADDSYNPROF  // #define  ADDSYNPROF
         else if( gbDoSynEdit )
         {
            strcpy( lpb, "Process as a SYNEDIT profile file (-prof)" MEOR );
         }
#endif   // #ifdef   ADDSYNPROF  // #define  ADDSYNPROF
         else if( g_bDumpGif )
         {
            if(g_bDumpGif == 1)
               strcpy( lpb, "Process as a GIF image file (-gif)" MEOR );
            else
               sprintf( lpb, "Process as a GIF image file (-gif%d)" MEOR, g_bDumpGif );
         }
         else
         {
            lpc = 0;
            strcpy( lpb, "Display: " );
			   if( !gfAddOffset && !gfAddHex && !gfAddAscii )
				   strcat( lpb, "NOTHING! " );

            if( gfAddOffset ) {
				   strcat( lpb, "Offset" );
               lpc = ", ";
            }
            if( g_iNumbOffset )
				   strcat( lpb, " as Number" );

            if( gfAddHex ) {
               if(lpc)
                  strcat( lpb, lpc );
				   strcat( lpb, "Hex" );
               lpc = ", ";
            }

            if( g_DoAsDWORDS ) {
				   strcat( lpb, " as DWORDS" );
               if( g_SwapBytes )
   				   strcat( lpb, " swap endian" );
            }

            if( gfAddAscii ) {
               if(lpc)
                  strcat( lpb, lpc );
				   strcat( lpb, "Ascii" );
               lpc = ", ";
            }
            if(lpc)
               strcat( lpb, lpc );
			   sprintf( (lpb + lstrlen(lpb)), "Begin at %u", gdwBgnOff );

            strcat( lpb, ", " );
			   if( gdwEndOff != (DWORD)-1 )
				   sprintf( (lpb + lstrlen( lpb )), "End at %u ", gdwEndOff );
			   else
				   strcat( lpb, "End at END " );
            if( g_pHexBlock )
				   strcat( lpb, "in Blocks" );

			   strcat( lpb, MEOR );
         }
		}
      // if(VERB)
      prt(lpb);
	}


	//if( lpc )
	//	free( lpc );

	if( giInCount == 0 )
	{
		prt( "ERROR: No input file[s]!" MEOR );
		UsageX();
	}

	for( i = 0; i < giInCount; i++ )
	{
		fn = gpFilNames[i];
		ProcessFile( fn );
		free( fn );
	}

// Exit_main:

   FreeCabLib();

//   DeleteLineMem();
   OutLineMem();

   if( VFH(g_hOutFile) )
      CloseHandle(g_hOutFile);

   pgm_exit(retv);

	return( retv );
}  // end - int main( int argc, char *argv[] )



// eof - Dump4.c
