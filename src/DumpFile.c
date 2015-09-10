
// DumpFile.c
#include "Dump4.h"
//#include	<windows.h>

#define	MXIOB		512
// File Name and Handle
TCHAR    szDbgFile[264];
TCHAR	   szDefDbg[] = "TEMPDUMP.TXT";
HANDLE   hDbgFile = 0;

// forward declaration
void	WriteDiagFile( LPTSTR lpd );

VOID  prt_init( VOID )
{
	if( !gw_fUsePrintf && !gw_fDnUseP )
	{
		gw_hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
      if( !VFH(gw_hStdOut) )
		{
			gw_hStdOut = 0;
			gw_fUsePrintf = TRUE;
			printf( "\nERROR: Get STD_OUTPUT_HANDLE FAILED!!!"MEOR );
		}
		gw_fDnUseP = TRUE;
	}
}

BOOL  outstg( HANDLE h, LPTSTR lpb, DWORD dwl )
{
   DWORD dwWtn;
   BOOL  flg = WriteFile( h,	// handle to file to write to 
						lpb,	// pointer to data to write to file
						dwl,	// number of bytes to write
						&dwWtn,	// pointer to number of bytes written
						NULL );	// pointer to structure needed for overlapped I/O
   if( flg && (dwl != dwWtn) )
      flg = FALSE;
   return flg;
}

void oi( LPTSTR lps )
{
	DWORD dwl = strlen(lps);
   if( dwl && VFH(gw_hStdOut) )
      outstg( gw_hStdOut, lps, dwl );

#ifndef  NDEBUG
   WriteDiagFile(lps);
#endif   // !NDEBUG

   if( dwl && VFH( g_hOutFile ) )
   {
      if( !outstg(g_hOutFile, lps, dwl) )
      {
         CloseHandle(g_hOutFile);
         g_hOutFile = INVALID_HANDLE_VALUE;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : doutput
// Return type: VOID 
// Argument   : LPTSTR lps
// Description: DIRECT output to the various channels
//              WITHOUT end-of-line checking.
///////////////////////////////////////////////////////////////////////////////
VOID  doutput( LPTSTR lps )
{
   DWORD dwl = strlen(lps);
   outstg( gw_hStdOut, lps, dwl );
#ifndef  NDEBUG
	if( VFH(hDbgFile) )
      outstg( hDbgFile, lps, dwl );
#endif   // !NDEBUG
   if( VFH( g_hOutFile ) )
      outstg( g_hOutFile, lps, dwl );
}

#define  MAXIO 256

void prt( LPTSTR lps )
{
   TCHAR tbuf[MAXIO+8];
	DWORD dwl = strlen(lps);
   DWORD off = 0;
   DWORD dwi;
   PTSTR pb = tbuf;
   TCHAR c, d;

   d = 0;
   for(dwi = 0; dwi < dwl; dwi++)
   {
      c = lps[dwi];
      if( c == '\n' ) {
         if( d != '\r' )
            pb[off++] = '\r';
      } else if( c == '\r' ) {
         if( lps[dwi+1] != '\n' ) {
            pb[off++] = c;
            c = '\n';
         }
      }
      pb[off++] = c;
      if(off >= MAXIO) {
         pb[off] = 0;
         oi(pb);
         off = 0;
      }
      d = c;
   }
   if(off) {
      pb[off] = 0;
      oi(pb);
   }
}

void prt_OK_maybe( LPTSTR lps )
{
	BOOL	   flg;
	DWORD	   dwLen	= strlen(lps);
   DWORD    dwi, dwk;
	static TCHAR   _s_buf[MXIOB+264];
	char	   c, d;
	LPTSTR	lpout;

	if( gw_fUsePrintf )
	{
		//printf( lps );
		printf( lps, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
	}
	else
	{
		if( dwLen )
		{
			lpout = &_s_buf[0];
			dwk = 0;
			d = 0;
			for( dwi = 0; dwi < dwLen; dwi++ )
			{
				c = lps[dwi];
				if( c == 0x0d )
				{
					if( (dwi+1) < dwLen )
					{
						if( lps[dwi+1] != 0x0a )
						{
							lpout[dwk++] = c;
							c = 0x0a;
						}
					}
					else
					{
						lpout[dwk++] = c;
						c = 0x0a;
					}
				}
				else if( c == 0x0a )
				{
					if( d != 0x0d )
					{
						lpout[dwk++] = 0x0d;
					}
				}
				lpout[dwk++] = c;
				d = c;
				if( dwk >= MXIOB )
				{
					flg = outstg( gw_hStdOut, lpout, dwk );
					dwk = 0;
               if( !flg )
                  break;
				}
			}
			if( dwk )
			{
            outstg( gw_hStdOut, lpout, dwk );
			}
		}
	}

#ifndef  NDEBUG
   WriteDiagFile(lps);
#endif   // !NDEBUG

   if( dwLen && VFH( g_hOutFile ) )
   {
      if( outstg(g_hOutFile, lps, dwLen) )
      {
         if( lps[dwLen-1] >= ' ' )
            outstg( g_hOutFile, MEOR, 2 );
      }
      else
      {
         CloseHandle(g_hOutFile);
         g_hOutFile = INVALID_HANDLE_VALUE;
      }
   }
}


BOOL  grmWriteFile( HANDLE * ph, LPTSTR lpb )
{
   HANDLE   h = 0;
   DWORD    dwi, dww;

   dwi = dww = 0;

   if( ph && lpb )
   {
      dwi = strlen(lpb);
      h = *ph;
   }
   if( ( VH(h) ) &&
      ( dwi ) )
   {
      if( ( WriteFile(h,lpb,dwi,&dww,NULL) ) &&
         ( dwi == dww ) )
      {
         // success
      }
      else
      {
         CloseHandle(h);
         h = (HANDLE)-1;
         *ph = h;
         dww = 0;
      }
   }
   return( (BOOL)dww );
}

BOOL  grmCloseFile( HANDLE * ph )
{
   HANDLE   h = 0;
   BOOL  bRet = FALSE;
   if( ph )
      h = *ph;

   if( VH(h) )
   {
      if( CloseHandle(h) )
         bRet = TRUE;
   }

   h = 0;
   if( ph )
      *ph = h;

   return bRet;
}

BOOL  OpenOut( LPTSTR lpf, HANDLE * pHand, BOOL bAppend )
{
   BOOL  bRet = FALSE;
   HANDLE   h;
   if( bAppend )
   {
      h = grmOpenFile( lpf, pHand, 1 );
      if( VFH(h) )
      {
         LONG  lg = 0;
         SetFilePointer(h, 0, &lg, FILE_END );
         bRet = TRUE;
      }
   }
   else
   {
      h = grmCreateFile( lpf );
      if( VFH(h) )
      {
         *pHand = h;
         bRet = TRUE;
      }
   }
   return bRet;
}

void	OpenDiagFile( LPTSTR lpf, HANDLE * lpHF, BOOL bApd )
{
   OpenOut( lpf, lpHF, bApd );
}

void	CloseDiagFile( HANDLE * lpHF )
{
   HANDLE h = *lpHF;
   if( VFH(h) )
      CloseHandle(h);
   *lpHF = 0;
}

#define MX_SPRTF_BUF (1024 * 8)
int vsprtf( PTSTR ps, va_list arglist )
{
   static TCHAR _s_vsprtfbuf[MX_SPRTF_BUF+16];
   LPTSTR   lpb = &_s_vsprtfbuf[0];
   int   i;
   //i = vsprintf( lpb, ps, arglist );
   // 2011-03-30 - change to more secure version
   // AND USING A GET LENGTH FUNCTION ;=)) _vscprintf();
   int len = _vscprintf( ps, arglist ) // _vscprintf doesn't count
                               + 1; // terminating '\0'
   if (len > MX_SPRTF_BUF) {
       char * buffer = (char *)malloc( len * sizeof(char) );
       if (buffer) {
           i = vsprintf_s( buffer, len, ps, arglist );
           if (i <= 0) {
               prt("ERROR:DUMP4:1: vsprintf_s FAILED!\n");
           } else {
               prt(buffer);
           }
           free(buffer);
       } else {
           sprintf(lpb,
               "ERROR:DUMP4: Allocation of %d bytes FAILED!. No sprtf() output ;=((\n",
               len);
           prt(lpb);
       }
   } else {
       i = vsprintf_s( lpb, MX_SPRTF_BUF, ps, arglist );
       if (i <= 0) {
           prt("ERROR:DUMP4:2: vsprintf_s FAILED!\n");
       } else {
           prt(lpb);
       }
   }
   return i;
}

int   _cdecl sprtf( LPTSTR lpf, ... )
{
    int res;
    va_list arglist;
    va_start(arglist, lpf);
    res = vsprtf( lpf, arglist );
    va_end(arglist);
    return res;
}

#if 0
int   _cdecl sprtf_org( LPTSTR lpf, ... )
{
   static TCHAR _s_sprtfbuf[1024];
   LPTSTR   lpb = &_s_sprtfbuf[0];
   int   i;
   va_list arglist;
   va_start(arglist, lpf);
   i = vsprintf( lpb, lpf, arglist );
   va_end(arglist);
   prt(lpb);
   return i;
}
#endif


void  _cdecl chkme( LPTSTR lpf, ... )
{
   static TCHAR _s_chkbuf[1024];
   LPTSTR   lpb = &_s_chkbuf[0];
   int   i;
   va_list arglist;
   va_start(arglist, lpf);
   i = vsprintf( lpb, lpf, arglist );
   va_end(arglist);
   sprtf("%s"MEOR, lpb);
}

BOOL  Chk4Debug( LPTSTR lpd )
{
   BOOL     bret = FALSE;
   LPTSTR ptmp = gszDiag;
   LPTSTR   p;
   DWORD  dwi;

   strcpy(ptmp, lpd);
   dwi = strlen(ptmp);
   if(dwi)
   {
      dwi--;
      if(ptmp[dwi] == '\\')
      {
         ptmp[dwi] = 0;
         p = strrchr(ptmp, '\\');
         if(p)
         {
            p++;
            if( strcmpi(p, "DEBUG") == 0 )
            {
               *p = 0;
               strcpy(lpd,ptmp);    // use this
               bret = TRUE;
            }
         }
      }
   }
   return bret;
}

VOID  GetModulePath( LPTSTR lpb )
{
   LPTSTR   p;
   GetModuleFileName( NULL, lpb, 256 );
   p = strrchr( lpb, '\\' );
   if( p )
      p++;
   else
      p = lpb;
   *p = 0;
#ifndef  NDEBUG
   Chk4Debug( lpb );
#endif   // !NDEBUG

}

void	WriteDiagFile( LPTSTR lpd )
{
	int		i = strlen(lpd);
	if( i && ( hDbgFile == 0 ) )
   {
      LPTSTR   pfn = szDbgFile;
      GetModulePath(pfn);
      strcat(pfn, szDefDbg);
		OpenDiagFile( pfn, &hDbgFile, FALSE );   // TRUE );
   }

   if( i && VFH(hDbgFile) )
   {
      if( !outstg( hDbgFile, lpd, i ) )
	  {
         CloseHandle( hDbgFile );
	     hDbgFile = INVALID_HANDLE_VALUE;
      }
	}
}

void	WriteDiagFile_OK_but_why( LPTSTR lpd )
{
	int		i, j, k;
	char	   buf[260];
	char	   c, d;
	LPTSTR	lpb;

   i = 0;
	if( lpd )
      i = lstrlen( lpd );
   if(i)
	{
		if( hDbgFile == 0 )
      {
         GetModuleFileName(NULL, szDbgFile, 256);
         lpb = strrchr( szDbgFile, '\\' );
         if(lpb)
            lpb++;
         else
            lpb = szDbgFile;
         strcpy(lpb, szDefDbg);
			OpenDiagFile( &szDbgFile[0], &hDbgFile, FALSE );   // TRUE );
      }

		if( VFH(hDbgFile) )
		{
			lpb = &buf[0];
			k = 0;
			d = c = 0;
			for( j = 0; j < i; j++ )
			{
				c = lpd[j];
				if( c == 0x0a )	// A LF
				{
					if( d != 0x0d )
					{
						lpb[k++] = 0x0d;
					}
				}
				else if( c == 0x0d )	// A CR
				{
					if( (j+1) < i )	// If more
					{
						if( lpd[j+1] != 0x0a )
						{
							lpb[k++] = c;
							c = 0x0a;
						}
					}
					else
					{
						lpb[k++] = c;
						c = 0x0a;
					}
				}
				lpb[k++] = c;
				d = c;	// Keep previous put
			}
			if( k )
			{
				if( c != 0x0a )
				{
					lpb[k++] = 0x0d;
					lpb[k++] = 0x0a;
				}
				if( !outstg( hDbgFile, lpb, k ) )
				{
					CloseHandle( hDbgFile );
					hDbgFile = INVALID_HANDLE_VALUE;
				}
			}
		}
	}
}


// eof - DiagFile.c
