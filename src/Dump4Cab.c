
// Dump4Cab.c
/*
 * dircab.c
 *
 * Began as a COPY of TESTFDI.c which demonstrated how to use the
 * FDI library APIs. It was modified to mainly be a VIEWER of the
 *  contents of a CAB file ...
 *	September, 2000	-	Geoff R. McLane
 *
 */
#ifdef   DUMP4
#include "dump4.h"
#include "dump4fdi.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else // NOT DUMP4 = STANDALONE DIRCAB
// ***********************************
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <sys/stat.h>
#include "..\..\include\fdi.h"
// ***********************************
#endif   // DUMP4 y/n

#undef	MEMSTATS2
#define	MMINNCOL	15
#ifndef  DUMP4
#define	EndBuf(a)	( a + strlen(a) )
#endif   // !DUMP4

#define	PRGNM		"DIRCAB"

/*
 * Function prototypes 
 */
BOOL	test_fdi(char *cabinet_file);
int		get_percentage(unsigned long a, unsigned long b);
char   *return_fdi_error_string(FDIERROR err);
void    Dbl2Stg( LPSTR lps, double factor, int prec );
int	sprintf3( char * cp, char * fp, DWORD siz );
#ifdef	MEMSTATS2
void	ShowMems( void );
#endif
LPTSTR   GetI64Stg2( LARGE_INTEGER li );
LPTSTR   GetDWStg2( DWORD dwi );
BOOL  CheckPrev( void );

typedef int  (DIAMONDAPI * CPROC) ();
typedef BOOL (DIAMONDAPI * FDICOPY) (HFDI, char *, char *, int, PFNFDINOTIFY, PFNFDIDECRYPT, void *);
typedef BOOL (DIAMONDAPI * FDIISCAB) (HFDI, int, PFDICABINETINFO );
typedef BOOL (DIAMONDAPI * FDIDEST) (HFDI);
typedef HFDI (DIAMONDAPI * FDICREAT) (PFNALLOC,PFNFREE,PFNOPEN,PFNREAD,PFNWRITE,PFNCLOSE,PFNSEEK,int,PERF);

char cFCopy[] = "FDICopy";
char cFIs[]   = "FDIIsCabinet";
char cFDest[] = "FDIDestroy";
char cFCreat[]= "FDICreate";

FDICOPY  pfdicopy  = 0;
FDIISCAB pfdiiscab = 0;
FDIDEST  pfdidest  = 0;
FDICREAT pfdicreat = 0;

typedef struct {
   char  *  c_Nm;
   CPROC *  c_F;
}CABLIB, *LPCABLIB;

CABLIB   sCabLib[] = {
   { cFCopy,  &pfdicopy },
   { cFIs,    &pfdiiscab },
   { cFDest,  &pfdidest },
   { cFCreat, (CPROC *)&pfdicreat }
};

HMODULE  hCabLib = 0;

#ifndef  DUMP4

int   giVerbose = 1;

/*
 * Destination directory for extracted files
 */
char	gszdest_dir[256];
char	gcOutBuf[256];

BOOL	gbShowInfo = FALSE;
BOOL  gfCopyAll = FALSE;
BOOL  gfCopyNone = FALSE;

DWORD	gdwCabSize;
DWORD	gdwTotSize;
DWORD	gdwComplete;
DWORD	gdwInPrev;
DWORD	gdwInNext;
DWORD	gdwInfoCnt;
BOOL  gbHasPrev;  // = fdici.hasprev; // TRUE ? "yes" : "no",
BOOL  gbHasNext;  // = fdici.hasnext; // TRUE ? "yes" : "no" );
TCHAR gcPrevCab[264];   // =,pfdin->psz2);
TCHAR gcPrevPath[264];
TCHAR gcPrevDisk[264];  // ,pfdin->psz3);
TCHAR gcPrevFull[264];
TCHAR gcPrevCab2[264];   // =,pfdin->psz2);

TCHAR gcNextCab[264];
TCHAR gcNextDisk[264];
TCHAR gcNextCab2[264];

#define  VERB  ( giVerbose )
#define  VERB1 VERB
#define  VERB2 ( giVerbose > 1 )
#define  VERB3 ( giVerbose > 2 )
#define  VERB4 ( giVerbose > 3 )
#define  VERB5 ( giVerbose > 4 )
#define  VERB6 ( giVerbose > 5 )
#define  VERB7 ( giVerbose > 6 )
#define  VERB8 ( giVerbose > 7 )
#define  VERB9 ( giVerbose > 8 )
#define  VERBMAX  VERB9
#define  MEOR  "\r\n"

int   prt( LPTSTR lps )
{
   return( printf(lps) );
}

#endif   // DUMP4

//typedef  struct {
//   USHORT   ci_Num;
//   USHORT   ci_setID;      // Cabinet set ID
//   USHORT   ci_iCabinet;   // Cabinet number (0-based)
//   USHORT   ci_iFolder;    // Folder number (0-based)
//   TCHAR    ci_cNextCab[264];
//   TCHAR    ci_cNextDisk[264];
//   TCHAR    ci_cPath[264];
//}CABINF, * LPCABINF;
//CABINF   gsCabInf;

char			gc_cabinet_name[264];
char			gc_cabinet_path[264];
char			gc_cabinet_full[264];
ERF			gs_erf;
FDICABINETINFO	gs_fdici;


BOOL  ChkCabLib( void )
{
   BOOL  bRet = FALSE;
   if( hCabLib && pfdicopy && pfdiiscab && pfdidest && pfdicreat )
      bRet = TRUE;
   return bRet;
}

BOOL  LoadCabLib( void )
{
   BOOL  bRet = FALSE;
   int   i = 0;

   if( ChkCabLib() )
      return TRUE;

   hCabLib = LoadLibrary( "CABINET.DLL" );
   if( hCabLib )
   {
      LPCABLIB lpc = &sCabLib[0];
      CPROC *   vp;
      FARPROC  fp;
      for( i = 0; i < 4; i++ )
      {
         vp = lpc->c_F; // extract POINTER to function pointer
         //if( !( lpc->c_F = GetProcAddress( hCabLib,    // handle to DLL module
         //   lpc->c_Nm ) ) ) //   LPCSTR lpProcName   // function name
         fp = GetProcAddress( hCabLib,    // handle to DLL module
            lpc->c_Nm ); //   LPCSTR lpProcName   // function name
         if( !fp )
         {
            break;
         }
         *vp = (CPROC)fp;
         lpc++;
      }
   }
   if( ( i == 4 ) &&
      ( ChkCabLib() ) )
   {
      bRet = TRUE;
   }
   return bRet;
}

BOOL  FreeCabLib( void )
{
   BOOL  bRet = FALSE;
   if( hCabLib )
   {
      FreeLibrary( hCabLib );
      bRet = TRUE;
   }
   hCabLib = 0;
   return bRet;
}


BOOL  ProcessCAB( char * lpcab )
{
   BOOL  bRet = test_fdi(lpcab);
   LPTSTR   lpo = &gcOutBuf[0];
//	if(test_fdi(lpcab) == TRUE)
	if(bRet)
	{
		if( gbShowInfo )
		{
			prt( PRGNM" was successful\n");
		}
		else
		{
//DWORD	gdwCabSize;
//DWORD	gdwTotSize;
//DWORD	gdwComplete;
//DWORD	gdwInPrev;
//DWORD	gdwInNext;
//DWORD	gdwInfoCnt;
			sprintf(lpo,
				"List %d:",
				(gdwComplete + gdwInPrev + gdwInNext) );
			while( strlen(lpo) < MMINNCOL )
				strcat( lpo, " " );
			sprintf3( EndBuf(lpo),
					"%10d",
					gdwTotSize );
			strcat(lpo, "  (");
			Dbl2Stg(EndBuf(lpo),
				(double)(((double)gdwCabSize * 100.0) / (double)gdwTotSize),
				5 );
			strcat(lpo, "%% Compression.");
			strcat(lpo,")\n");
			prt(lpo);
		}

      if(VERB4)
         CheckPrev();

	}
	else
	{
		prt( PRGNM" failed\n");
	}

#ifdef	MEMSTATS2
	ShowMems();
#endif

   return bRet;

}

#ifndef  DUMP4
// ************************************************************************
// standalone DIRCAB

void main(int argc, char **argv)
{
	gdwCabSize = gdwTotSize = 0;	// start with NOTHING
	gdwInfoCnt = gdwComplete = gdwInPrev = gdwInNext = 0;
//	if (argc != 3)
	if (argc < 2)
	{
		prt(
			PRGNM" - Demonstrates how to use the FDI library API calls\n"
			"\n"
			"Usage: "PRGNM" cabinet [dest_dir]\n"
			"\n"       
			"Where <cabinet> is the name of a cabinet file, and <dest_dir>\n"
			"is the destination for the files extracted\n"
			"\n"
			"  e.g. "PRGNM" c:\\test1.cab c:\\\n"
			"\n"
		);

		pgm_exit(1);
	}

   if( !LoadCabLib() )
   {
      prt( "ERROR: Unable to load CABINET.DLL\n" );
      pgm_exit(2);
   }

	if( argc == 3 )
	{
		gbShowInfo = TRUE;
		strcpy(gszdest_dir, argv[2]);	// proceed with extract to here
      if( gszdest_dir[ (strlen(dest_dir) - 1 ) ] != '\\' )
         strcat(gszdest_dir,"\\");
	}
	else
	{
		gbShowInfo = FALSE;
		strcpy(gszdest_dir, ".\\");	// not used
	}

   ProcessCAB( argv[1] );

   FreeCabLib();

	pgm_exit(0);

}

// ************************************************************************
#endif   // DUMP4

// #define	MEMSTATS2

/*
 * Memory allocation function
 */

#ifdef	MEMSTATS2

// collect some statistics
#define	MXMESTATS	256
typedef struct {
	DWORD	 m_sz;
	void * m_vp;
}MMSTS;
typedef MMSTS * LPMMSTS;
int	iAllCnt = 0;
int	iMemErr = 0;
MMSTS	sMems[MXMESTATS];
FNALLOC(mem_alloc)
{
	void * vp;
	vp = malloc(cb);
   if(vp)
   {
      ZeroMemory( vp, cb );
      sMems[iAllCnt].m_sz = cb;
      sMems[iAllCnt].m_vp = vp;
      if(iAllCnt < MXMESTATS)
         iAllCnt++;
   }
	return vp;
}

/*
 * Memory free function
 */
FNFREE(mem_free)
{
	LPMMSTS lpm = &sMems[0];
	int	i;
	free(pv);
	if( iAllCnt )
	{
		for( i = 0; i < iAllCnt; i++ )
		{
			if( lpm->m_vp == pv )
			{
				lpm->m_vp = 0;
				break;
			}
			lpm++;
		}
		if( i == iAllCnt )
			iMemErr++;
	}
	else
		iMemErr++;
}
void	ShowMems( void )
{
	int	i;
	int	iNotF = 0;
	DWORD	dwMin, dwMax;
	LPMMSTS lpm = &sMems[0];
   LPTSTR   lpo = &gcOutBuf[0];

	dwMin = (DWORD)-1;
	dwMax = 0;
	for( i = 0; i < iAllCnt; i++ )
	{
		if( lpm->m_vp )
			iNotF++;
		if( lpm->m_sz > dwMax )
			dwMax = lpm->m_sz;
		if( lpm->m_sz < dwMin )
			dwMin = lpm->m_sz;
		lpm++;
	}
	sprintf(lpo,
		"Memory: %d allocs from %u to %u in size.",
		iAllCnt,
		dwMin,
		dwMax );
	if( iNotF )
		sprintf(EndBuf(lpo), " (%d not FREED)", iNotF);
	if( iMemErr )
		sprintf(EndBuf(lpo), " (%d ERRORS!)", iMemErr);
	strcat(lpo,"\n");
	prt(lpo);		
}

#else	// !MEMSTATS2

FNALLOC(mem_alloc)
{
	void * vp;
	vp = malloc(cb);
   if(vp)
      ZeroMemory(vp, cb);
   return vp;
}


/*
 * Memory free function
 */
FNFREE(mem_free)
{
	free(pv);
}

#endif	// MEMSTATS2 y/n

// file functions - open, read, write, seek, and close
// ***************************************************
#ifdef   WIN32

// NOTE: This function ONLY handles the current known calls
// There are presently only TWO calls
#define  FCALL1 ( _O_BINARY | _O_RDONLY | _O_SEQUENTIAL )
#define  FCALL2 ( _O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL )

FNOPEN(file_open)
{
   int   iRet;
   DWORD dwa, dwc;
   if( oflag & _O_CREAT )
   {
      dwa = GENERIC_READ | GENERIC_WRITE;
      dwc = CREATE_ALWAYS;
   }
   else
   {
      dwa = GENERIC_READ;
      dwc = OPEN_EXISTING;
   }
	//return _open(pszFile, oflag, pmode);
	iRet = (int) CreateFile(
      pszFile,    // file name
      dwa,        // access mode
      FILE_SHARE_READ, // share mode
      0,          // SD
      dwc,        // how to create
      FILE_ATTRIBUTE_NORMAL,  // file attributes
      0 );        // handle to template file

   UNREFERENCED_PARAMETER(pmode);

	return iRet;
}

FNREAD(file_read)
{
   UINT  uRet = 0;   // none yet ...
   DWORD dwr;
//	return _read(hf, pv, cb);
   if( ReadFile( (HANDLE)hf,   // handle to file
      pv, // data buffer
      cb, // number of bytes to read
      &dwr,  // number of bytes read
      0 ) )  // overlapped buffer
   {
      // success
      uRet = dwr; // return READ amount
   }
   return uRet;
}


FNWRITE(file_write)
{
   UINT  uRet = 0;
   DWORD dww;
	//return _write(hf, pv, cb);
   if( WriteFile( (HANDLE)hf, // handle to file
      pv,   // data buffer
      cb,   // number of bytes to write
      &dww, // number of bytes written
      0 ) ) // overlapped buffer
   {
      uRet = dww;
   }
   return uRet;
}

FNCLOSE(file_close)
{
   int   iRet = 0;
	//return _close(hf);
   if( CloseHandle((HANDLE)hf) )
      iRet++;
   return iRet;
}

FNSEEK(file_seek)
{
   DWORD dwi, dwm;
   DWORD dwh = 0;

   if( seektype == SEEK_SET ) // Beginning of file
      dwm = FILE_BEGIN; // The starting point is zero or the beginning of the file.
   else if( seektype == SEEK_CUR )  // Current position of file pointer
      dwm = FILE_CURRENT;  // The starting point is the current value of the file pointer.
   else if( seektype == SEEK_END )  // End of file
      dwm = FILE_END;   // The starting point is the current end-of-file position.
   else
      dwm = FILE_BEGIN; // default to this if NOT above !!!???!!!
   //return _lseek(hf, dist, seektype);
   dwi = SetFilePointer((HANDLE)hf, // handle to file
      dist, // bytes to move pointer
      (PLONG)&dwh, // bytes to move pointer
      dwm );   // starting point
   return dwi;
}

#else // !WIN32

FNOPEN(file_open)
{
	return _open(pszFile, oflag, pmode);
}

FNREAD(file_read)
{
	return _read(hf, pv, cb);
}


FNWRITE(file_write)
{
	return _write(hf, pv, cb);
}

FNCLOSE(file_close)
{
	return _close(hf);
}

FNSEEK(file_seek)
{
	return _lseek(hf, dist, seektype);
}
#endif   // WIN32 y/n


FNFDINOTIFY(notification_function)
{
	FILETIME    local_filetime;
	FILETIME    datetime;
	SYSTEMTIME	st;
   DWORD			attrs;
	char    		destination[256];
   LPTSTR      lpo = &gcOutBuf[0];
	switch (fdint)
	{
		case fdintCABINET_INFO: // general information about the cabinet
/*  fdintCABINET_INFO:
 *        Called exactly once for each cabinet opened by FDICopy(), including
 *        continuation cabinets opened due to file(s) spanning cabinet
 *        boundaries. Primarily intended to permit EXTRACT.EXE to
 *        automatically select the next cabinet in a cabinet sequence even if
 *        not copying files that span cabinet boundaries.
 *      Entry:
 *          pfdin->psz1     = name of next cabinet
 *          pfdin->psz2     = name of next disk
 *          pfdin->psz3     = cabinet path name
 *          pfdin->setID    = cabinet set ID (a random 16-bit number)
 *          pfdin->iCabinet = Cabinet number within cabinet set (0-based)
 *      Exit-Success:
 *          Return anything but -1
 *      Exit-Failure:
 *          Returns -1 => Abort FDICopy() call
 */
         strcpy(gcNextCab, pfdin->psz1);    // = name of next cabinet
         strcpy(gcNextDisk,pfdin->psz2);   // = name of next disk
			gdwInfoCnt++;
			if( gbShowInfo )
			{
				sprintf(lpo,
					"fdintCABINET_INFO\n"
					"  next cabinet     = %s\n"
					"  next disk        = %s\n"
					"  cabinet path     = %s\n"
					"  cabinet set ID   = %d\n"
					"  cabinet # in set = %d (zero based)\n"
					"\n",
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3,
					pfdin->setID,
					pfdin->iCabinet );
            prt(lpo);
			}
         else if( VERBMAX )
         {
				sprintf(lpo,
					"INFO: next cab=%s disk=%s path=%s ID=%#x Set#=%d.\n",
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3,
					pfdin->setID,
					pfdin->iCabinet );
            prt(lpo);
         }
			return 0;

		case fdintPARTIAL_FILE: // first file(s) in cabinet is continuation
         // but this is the whole file size
         // NOT the partial amount in this cabinet.
         // Really fizzles the % compression since do NOT know amount in
         // this cab or the amount in previous cab
			gdwInPrev++;
			gdwTotSize += pfdin->cb;	// add file size to total
         strcpy(gcPrevCab,pfdin->psz2);
         strcpy(gcPrevDisk,pfdin->psz3);
			if( gbShowInfo )
			{
				sprintf(lpo,
					"fdintPARTIAL_FILE\n"
					"   name of continued file            = %s\n"
					"   name of cabinet where file starts = %s\n"
					"   name of disk where file starts    = %s\n",
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3 );
            prt(lpo);
			}
			else
			{
				strcpy( lpo, pfdin->psz1 );
				while( strlen(lpo) < MMINNCOL )
					strcat( lpo, " " );
				sprintf3( EndBuf(lpo),
					"%10d",
					pfdin->cb );
				strcat( lpo, " <PARTIAL> Begins in " );
				strcat( lpo, pfdin->psz2 );
            if( VERB2 )
            {
               sprintf(EndBuf(lpo),
                  " Disk=[%s]",
                  pfdin->psz3 );
            }
				strcat( lpo, "\n" );
				prt( lpo );
			}
			return 0;

		case fdintCOPY_FILE:	// file to be copied
		{
			int		response = 'N';
			int		handle;
//			char	destination[256];

			gdwComplete++;
			gdwTotSize += pfdin->cb;	// add file size to total
			if( gbShowInfo )
			{
				sprintf(
					destination, 
					"%s%s",
					gszdest_dir,
					pfdin->psz1 );
				sprintf(lpo,
					"fdintCOPY_FILE\n"
					"  File name in cabinet = %s (size = %d)\n"
               "  Output to %s\n",
					pfdin->psz1,
					pfdin->cb,
               destination );
            prt(lpo);
            if( gfCopyAll )
            {
               prt("  Copying ALL ...");
               response = 'A';
            }
            else if( gfCopyNone )
            {
               prt("  Copying NONE ...");
               response = 'X';
            }
            else
            {
               prt("  Copy ALL? This file? None? (a/y/n/x): ");
            }
            if( !gfCopyAll && !gfCopyNone )
            {
				   do
				   {
					   response = getc(stdin);
					   response = toupper(response);
				   }while( (response != 'Y') && (response != 'N') &&
                  (response != 'A') && (response != 'X') );
            }
				prt("\n");
				if( ( response == 'Y' ) || ( response == 'A' ) )
            {
               if( response == 'A' )
                  gfCopyAll = TRUE;
					handle = file_open(
						destination,
						_O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL,
						_S_IREAD | _S_IWRITE );
               if( ( handle ) && ( handle != -1 ) )
               {
                  // success
                  return handle;
               }
               else
               {
                  // file create error
                  sprintf(lpo,
                     "ERROR: Can NOT create [%s]!!!\n",
                     destination );
                  prt(lpo);
                  gfCopyAll = FALSE;   // if this FAILED, ask again
   					return 0; /* skip file */
               }
				}
				else
				{
               if( response == 'X' )
                  gfCopyNone = TRUE;

					return 0; /* skip file */
				}
			}
			else
			{
				strcpy( lpo, pfdin->psz1 );
				while( strlen(lpo) < MMINNCOL )
					strcat( lpo, " " );
				sprintf3( EndBuf(lpo),
					"%10d",
					pfdin->cb );
				strcat( lpo, "  " );
                if( ( DosDateTimeToFileTime(
                    pfdin->date,
                    pfdin->time,
                    &datetime) ) &&
							( LocalFileTimeToFileTime(
                        &datetime,
                        &local_filetime) ) &&
							( FileTimeToSystemTime(
								&local_filetime,	// file time to convert
								&st )	// receives system time
								) )
					{
						sprintf( EndBuf(lpo),
							"%02d/%02d/%02d  %02d:%02d",
							st.wDay,
							st.wMonth,
							(st.wYear % 100),
							st.wHour,
							st.wMinute );
					}
					else
					{
						strcat( lpo, "FILETIME!!!" );
					}
            /*
             * Mask out attribute bits other than readonly,
             * hidden, system, and archive, since the other
             * attribute bits are reserved for use by
             * the cabinet format.
             */
            attrs = pfdin->attribs;
            attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
				destination[0] = 0;
				if( attrs & _A_RDONLY )
					strcat( destination, "R" );
				if( attrs & _A_HIDDEN )
					strcat( destination, "H" );
				if( attrs & _A_SYSTEM )
					strcat( destination, "S" );
				if( attrs & _A_ARCH )
					strcat( destination, "A" );
				if( destination[0] )
					sprintf( EndBuf(lpo),
						"  %s",
						destination );
				strcat( lpo, "\n" );
				prt( lpo );
				return 0; /* skip file */
			}
		}

		case fdintCLOSE_FILE_INFO:	// close the file, set relevant info
        {
            HANDLE  handle;
//            DWORD   attrs;
//            char    destination[256];
 			sprintf(lpo,
				"fdintCLOSE_FILE_INFO\n"
				"   file name in cabinet = %s\n"
				"\n",
				pfdin->psz1 );
         prt(lpo);

         sprintf( destination, 
             "%s%s",
             gszdest_dir,
             pfdin->psz1 );
#ifdef   WIN32
			handle = (HANDLE)pfdin->hf;
#else // !WIN32
			file_close(pfdin->hf);

            /*
             * Set date/time
             *
             * Need Win32 type handle for to set date/time
             */
            handle = CreateFile(
                destination,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );
#endif   // WIN32 y/n
            if( (handle) && (handle != INVALID_HANDLE_VALUE) )
            {
//                FILETIME    datetime;
                if(DosDateTimeToFileTime(
                    pfdin->date,
                    pfdin->time,
                    &datetime))
                {
//                    FILETIME    local_filetime;
                    if(LocalFileTimeToFileTime(
                        &datetime,
                        &local_filetime))
                    {
                        (void) SetFileTime(
                            handle,
                            &local_filetime,
                            NULL,
                            &local_filetime );
                     }
                }
                CloseHandle(handle);
            }
            else
            {
               sprintf(lpo,
                  "ERROR: Handle of file %s is INVALID!"MEOR,
                  pfdin->psz1 );
            }

            /*
             * Mask out attribute bits other than readonly,
             * hidden, system, and archive, since the other
             * attribute bits are reserved for use by
             * the cabinet format.
             */
            attrs = pfdin->attribs;
            attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
            (void) SetFileAttributes( destination, attrs );

			return TRUE;
        }

		case fdintNEXT_CABINET:	// file continued to next cabinet
			gdwInNext++;
			gdwTotSize += pfdin->cb;	// add file size to total
			if( gbShowInfo )
			{
				sprintf(lpo,
					"fdintNEXT_CABINET\n"
					"   name of next cabinet where file continued = %s\n"
               "   name of next disk where file continued    = %s\n"
					"   cabinet path name                         = %s\n"
					"\n",
					pfdin->psz1,
					pfdin->psz2,
					pfdin->psz3 );
            prt(lpo);
			}
			else
			{
				strcpy( lpo, pfdin->psz1 );
				while( strlen(lpo) < MMINNCOL )
					strcat( lpo, " " );
				sprintf3( EndBuf(lpo),
					"%10d",
					pfdin->cb );
				strcat( lpo, " <PARTIAL> Cont. in " );
				strcat( lpo, pfdin->psz2 );
				strcat( lpo, "\n" );
				prt( lpo );
			}
			return 0;

      case fdintENUMERATE:
/*
 *        Called once after a call to FDICopy() starts scanning a CAB's
 *        CFFILE entries, and again when there are no more CFFILE entries.
 *        If CAB spanning occurs, an additional call will occur after the
 *        first spanned file is completed.  If the pfdin->iFolder value is
 *        changed from zero, additional calls will occur next time it reaches
 *        zero.  If iFolder is changed to zero, FDICopy will terminate, as if
 *        there were no more CFFILE entries.  Primarily intended to allow an
 *        application with it's own file list to help FDI advance quickly to
 *        a CFFILE entry of interest.  Can also be used to allow an
 *        application to determine the cb values for each file in the CAB.
 *      Entry:
 *        pfdin->cb        = current CFFILE position
 *        pfdin->iFolder   = number of files remaining
 *        pfdin->setID     = current CAB's setID value
 *      Exit-Don't Care:
 *        Don't change anything.
 *        Return anything but -1.
 */
         if( VERBMAX )
         {
            sprintf(lpo,
               "ENUMERATE: Pos=%s Folder=%d, ID=%#x.\n",
               GetDWStg2(pfdin->cb),
               pfdin->iFolder,
               pfdin->setID );
            prt(lpo);
         }
         return 0;

	}

	return 0;
}


FNFDINOTIFY(note_function2)
{
	switch (fdint)
	{
		case fdintCABINET_INFO: // general information about the cabinet
/*  fdintCABINET_INFO:
 *        Called exactly once for each cabinet opened by FDICopy(), including
 *        continuation cabinets opened due to file(s) spanning cabinet
 *        boundaries. Primarily intended to permit EXTRACT.EXE to
 *        automatically select the next cabinet in a cabinet sequence even if
 *        not copying files that span cabinet boundaries.
 *      Entry:
 *          pfdin->psz1     = name of next cabinet
 *          pfdin->psz2     = name of next disk
 *          pfdin->psz3     = cabinet path name
 *          pfdin->setID    = cabinet set ID (a random 16-bit number)
 *          pfdin->iCabinet = Cabinet number within cabinet set (0-based)
 *      Exit-Success:
 *          Return anything but -1
 *      Exit-Failure:
 *          Returns -1 => Abort FDICopy() call
 */
         strcpy(gcNextCab2,pfdin->psz1);
			return 0;

		case fdintPARTIAL_FILE: // first file(s) in cabinet is continuation
/*  fdintPARTIAL_FILE:
 *        Called for files at the front of the cabinet that are CONTINUED
 *        from a previous cabinet.  This callback occurs only when FDICopy is
 *        started on second or subsequent cabinet in a series that has files
 *        continued from a previous cabinet.
 *      Entry:
 *          pfdin->psz1 = file name of file CONTINUED from a PREVIOUS cabinet
 *          pfdin->psz2 = name of cabinet where file starts
 *          pfdin->psz3 = name of disk where file starts
 *      Exit-Success:
 *          Return anything other than -1; enumeration continues
 *      Exit-Failure:
 *          Returns -1 => Abort FDICopy() call
 */
         strcpy( gcPrevCab2, pfdin->psz2 );
			return 0;

		case fdintCOPY_FILE:	// file to be copied
/*  fdintCOPY_FILE:
 *        Called for each file that *starts* in the current cabinet, giving
 *        the client the opportunity to request that the file be copied or
 *        skipped.
 *      Entry:
 *          pfdin->psz1    = file name in cabinet
 *          pfdin->cb      = uncompressed size of file
 *          pfdin->date    = file date
 *          pfdin->time    = file time
 *          pfdin->attribs = file attributes
 *          pfdin->iFolder = file's folder index
 *      Exit-Success:
 *          Return non-zero file handle for destination file; FDI writes
 *          data to this file use the PFNWRITE function supplied to FDICreate,
 *          and then calls fdintCLOSE_FILE_INFO to close the file and set
 *          the date, time, and attributes.  NOTE: This file handle returned
 *          must also be closeable by the PFNCLOSE function supplied to
 *          FDICreate, since if an error occurs while writing to this handle,
 *          FDI will use the PFNCLOSE function to close the file so that the
 *          client may delete it.
 *      Exit-Failure:
 *          Returns 0  => Skip file, do not copy
 *          Returns -1 => Abort FDICopy() call
 */
			return 0; /* skip file */

		case fdintCLOSE_FILE_INFO:	// close the file, set relevant info
/*  fdintCLOSE_FILE_INFO:
 *        Called after all of the data has been written to a target file.
 *        This function must close the file and set the file date, time,
 *        and attributes.
 *      Entry:
 *          pfdin->psz1    = file name in cabinet
 *          pfdin->hf      = file handle
 *          pfdin->date    = file date
 *          pfdin->time    = file time
 *          pfdin->attribs = file attributes
 *          pfdin->iFolder = file's folder index
 *          pfdin->cb      = Run After Extract (0 - don't run, 1 Run)
 *      Exit-Success:
 *          Returns TRUE
 *      Exit-Failure:
 *          Returns FALSE, or -1 to abort;
 *
 *              IMPORTANT NOTE IMPORTANT:
 *                  pfdin->cb is overloaded to no longer be the size of
 *                  the file but to be a binary indicated run or not
 *
 *              IMPORTANT NOTE:
 *                  FDI assumes that the target file was closed, even if this
 *                  callback returns failure.  FDI will NOT attempt to use
 *                  the PFNCLOSE function supplied on FDICreate() to close
 *                  the file!
 */
			return TRUE;

		case fdintNEXT_CABINET:	// file continued to next cabinet
/*  fdintNEXT_CABINET:
 *        This function is *only* called when fdintCOPY_FILE was told to copy
 *        a file in the current cabinet that is continued to a subsequent
 *        cabinet file.  It is important that the cabinet path name (psz3)
 *        be validated before returning!  This function should ensure that
 *        the cabinet exists and is readable before returning.  So, this
 *        is the function that should, for example, issue a disk change
 *        prompt and make sure the cabinet file exists.
 *
 *        When this function returns to FDI, FDI will check that the setID
 *        and iCabinet match the expected values for the next cabinet.
 *        If not, FDI will continue to call this function until the correct
 *        cabinet file is specified, or until this function returns -1 to
 *        abort the FDICopy() function.  pfdin->fdie is set to
 *        FDIERROR_WRONG_CABINET to indicate this case.
 *
 *        If you *haven't* ensured that the cabinet file is present and
 *        readable, or the cabinet file has been damaged, pfdin->fdie will
 *        receive other appropriate error codes:
 *
 *              FDIERROR_CABINET_NOT_FOUND
 *              FDIERROR_NOT_A_CABINET
 *              FDIERROR_UNKNOWN_CABINET_VERSION
 *              FDIERROR_CORRUPT_CABINET
 *              FDIERROR_BAD_COMPR_TYPE
 *              FDIERROR_RESERVE_MISMATCH
 *              FDIERROR_WRONG_CABINET
 *
 *      Entry:
 *          pfdin->psz1 = name of next cabinet where current file is continued
 *          pfdin->psz2 = name of next disk where current file is continued
 *          pfdin->psz3 = cabinet path name; FDI concatenates psz3 with psz1
 *                          to produce the fully-qualified path for the cabinet
 *                          file.  The 256-byte buffer pointed at by psz3 may
 *                          be modified, but psz1 may not!
 *          pfdin->fdie = FDIERROR_WRONG_CABINET if the previous call to
 *                        fdintNEXT_CABINET specified a cabinet file that
 *                        did not match the setID/iCabinet that was expected.
 *      Exit-Success:
 *          Return anything but -1
 *      Exit-Failure:
 *          Returns -1 => Abort FDICopy() call
 *      Notes:
 *          This call is almost always made when a target file is open and
 *          being written to, and the next cabinet is needed to get more
 *          data for the file.
 */
			return 0;

      case fdintENUMERATE:
/*
 *        Called once after a call to FDICopy() starts scanning a CAB's
 *        CFFILE entries, and again when there are no more CFFILE entries.
 *        If CAB spanning occurs, an additional call will occur after the
 *        first spanned file is completed.  If the pfdin->iFolder value is
 *        changed from zero, additional calls will occur next time it reaches
 *        zero.  If iFolder is changed to zero, FDICopy will terminate, as if
 *        there were no more CFFILE entries.  Primarily intended to allow an
 *        application with it's own file list to help FDI advance quickly to
 *        a CFFILE entry of interest.  Can also be used to allow an
 *        application to determine the cb values for each file in the CAB.
 *      Entry:
 *        pfdin->cb        = current CFFILE position
 *        pfdin->iFolder   = number of files remaining
 *        pfdin->setID     = current CAB's setID value
 *      Exit-Don't Care:
 *        Don't change anything.
 *        Return anything but -1.
 */
         return 0;

	}

	return 0;
}

BOOL  CheckPrev( void )
{
	HFDI			hfdi;
	int			hf;
   int         iPCnt, iNCnt;
   LPTSTR      lpo = &gcOutBuf[0];
   if( VERB5 )
   {
      // see if we can construct the FULL chain of CABINET files
      prt( "Moment ... attempting to trace CAB CHAIN ..."MEOR );
      if( ( gbHasPrev ) &&
         ( gcPrevCab[0] ) )
      {
         // lets use this PREVIOUS
         iPCnt = 0;
Next_Cycle:
         sprintf( gcPrevFull, "%s%s", gcPrevPath, gcPrevCab );
      	hfdi = pfdicreat( mem_alloc, mem_free,
            file_open, file_read, file_write, file_close, file_seek,
            cpu80386,
            &gs_erf );
         if(hfdi == 0 )
            return FALSE;
      	hf = file_open( gcPrevFull, (_O_BINARY | _O_RDONLY | _O_SEQUENTIAL), 0 );
      	if( ( hf == -1 ) || ( hf == 0 ) )
         {
      		pfdidest(hfdi);
		      sprintf(lpo,"ERROR: Unable to open [%s]!\n", gcPrevFull);
            prt(lpo);
		      return FALSE;
	      }

         ZeroMemory( &gs_fdici, sizeof(FDICABINETINFO) );
      	if( pfdiiscab(	hfdi, hf, &gs_fdici) )
         {
      		file_close(hf);
            // ok, this is a CAB
            if( VERBMAX )
            {
   			   sprintf(lpo,
	   			   "CAB: %s has %3d file(s) (BT=%s ID=%#x CAB#=%d) ",
                  gcPrevCab,
		   		   gs_fdici.cFiles,
                  GetDWStg2(gs_fdici.cbCabinet),
                  gs_fdici.setID,
                  gs_fdici.iCabinet );
			      if( gs_fdici.hasprev )	// == TRUE ? "yes" : "no",
				      strcat(lpo, "P");
			      if( gs_fdici.hasnext )	//== TRUE ? "yes" : "no" );
				      strcat(lpo, "N");
			      strcat(lpo,"\n");
			      prt(lpo);
            }
         }
         else
	      {
		      /* ************************ *
		       * No, it's not a cabinet!  *
		       ************************** */
      		file_close(hf);
		      sprintf(lpo,
			   "FDIIsCabinet() failed: [%s] is NOT a cabinet!\n",
			   gcPrevFull );
            prt(lpo);
            pfdidest(hfdi);
            return FALSE;
	      }

         gcPrevCab2[0] = 0;   // set no previous
      	if( pfdicopy( hfdi,
            gcPrevCab, gcPrevPath,
		      0,
		      note_function2, NULL, NULL) )
         {
            // success
            iPCnt++;
         }
         else
         {
            // FAILED!!!
            sprintf(lpo,
			   "FDICopy() failed: code %d [%s]\n",
			   gs_erf.erfOper, return_fdi_error_string(gs_erf.erfOper) );
            prt(lpo);
      		pfdidest(hfdi);
		      return FALSE;
	      }

         // we are at the end of this CAB
     		if( FALSE == pfdidest(hfdi) )
         {
      		pfdidest(hfdi);
		      return FALSE;
         }

         // do we have more previous CABS
         if( ( gs_fdici.hasprev ) &&
            ( gcPrevCab2[0] ) &&
            ( strcmpi(gcPrevCab,gcPrevCab2) ) )
         {
            strcpy(gcPrevCab,gcPrevCab2);
            goto Next_Cycle;
         }

         sprintf(lpo,
            "Appears [%s] is first CAB in series ..."MEOR,
            gcPrevCab );
         prt(lpo);
      }
      else
      {
         sprintf(lpo,
            "Appears [%s] is first CAB in series ..."MEOR,
            gc_cabinet_name );
         prt(lpo);
      }
      if( ( gbHasNext ) &&
         ( gcNextCab[0] ) )
      {
         // lets use this PREVIOUS
         iNCnt = 0;
Next_Cycle2:
         sprintf( gcPrevFull, "%s%s", gcPrevPath, gcNextCab );
      	hfdi = pfdicreat( mem_alloc, mem_free,
            file_open, file_read, file_write, file_close, file_seek,
            cpu80386,
            &gs_erf );
         if(hfdi == 0 )
            return FALSE;
      	hf = file_open( gcPrevFull, (_O_BINARY | _O_RDONLY | _O_SEQUENTIAL), 0 );
      	if( ( hf == -1 ) || ( hf == 0 ) )
         {
      		pfdidest(hfdi);
		      sprintf(lpo,"ERROR: Unable to open [%s]!\n", gcPrevFull);
            prt(lpo);
		      return FALSE;
	      }

         ZeroMemory( &gs_fdici, sizeof(FDICABINETINFO) );
      	if( pfdiiscab(	hfdi, hf, &gs_fdici) )
         {
      		file_close(hf);
            // ok, this is a CAB
            if( VERBMAX )
            {
   			   sprintf(lpo,
	   			   "CAB: %s has %3d file(s) (BT=%s ID=%#x CAB#=%d) ",
                  gcNextCab,
		   		   gs_fdici.cFiles,
                  GetDWStg2(gs_fdici.cbCabinet),
                  gs_fdici.setID,
                  gs_fdici.iCabinet );
			      if( gs_fdici.hasprev )	// == TRUE ? "yes" : "no",
				      strcat(lpo, "P");
			      if( gs_fdici.hasnext )	//== TRUE ? "yes" : "no" );
				      strcat(lpo, "N");
			      strcat(lpo,"\n");
			      prt(lpo);
            }
         }
         else
	      {
		      /* ************************ *
		       * No, it's not a cabinet!  *
		       ************************** */
      		file_close(hf);
		      sprintf(lpo,
			   "FDIIsCabinet() failed: [%s] is NOT a cabinet!\n",
			   gcPrevFull );
            prt(lpo);
            pfdidest(hfdi);
            return FALSE;
	      }

         gcNextCab2[0] = 0;   // set no next
      	if( pfdicopy( hfdi,
            gcNextCab, gcPrevPath,
		      0,
		      note_function2, NULL, NULL) )
         {
            // success
            iNCnt++;
         }
         else
         {
            // FAILED!!!
            sprintf(lpo,
			   "FDICopy() failed: code %d [%s]\n",
			   gs_erf.erfOper, return_fdi_error_string(gs_erf.erfOper) );
            prt(lpo);
      		pfdidest(hfdi);
		      return FALSE;
	      }

         // we are at the end of this CAB
     		if( FALSE == pfdidest(hfdi) )
         {
      		pfdidest(hfdi);
		      return FALSE;
         }

         // do we have more previous CABS
         if( ( gs_fdici.hasnext ) &&
            ( gcNextCab2[0] ) &&
            ( strcmpi(gcNextCab,gcNextCab2) ) )
         {
            strcpy(gcNextCab,gcNextCab2);
            goto Next_Cycle2;
         }

         sprintf(lpo,
            "Appears [%s] is last CAB in series ..."MEOR,
            gcNextCab );
         prt(lpo);
      }
      else
      {
         sprintf(lpo,
            "Appears [%s] is last CAB in series ..."MEOR,
            gc_cabinet_name );
         prt(lpo);
      }
   }
   return TRUE;
}

BOOL test_fdi(char *cabinet_fullpath)
{
	HFDI			hfdi;
	ERF				erf;
	FDICABINETINFO	fdici;
	int				hf;
	char			*p;
   LPTSTR      lpo = &gcOutBuf[0];
//	char			cabinet_name[256];
//	char			cabinet_path[256];

   gc_cabinet_name[0] = 0;
   gc_cabinet_path[0] = 0;
   gcPrevCab[0] = 0;
   gcPrevDisk[0] = 0;
   gcNextCab[0] = 0;

//	hfdi = FDICreate(
	hfdi = pfdicreat(
		mem_alloc,
		mem_free,
		file_open,
		file_read,
		file_write,
		file_close,
		file_seek,
		cpu80386,
		&erf );
	if( hfdi == NULL )
	{
		sprintf(lpo,
         "FDICreate() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper) );
      prt(lpo);
		return FALSE;
	}
   else if( VERBMAX )
   {
		sprintf(lpo,
         "FDICreate() returned handle: 0x%x (%d)\n",
         hfdi, hfdi );
      prt(lpo);
   }


	/*
	 * Is this file really a cabinet?
	 */
	hf = file_open(
		cabinet_fullpath,
		_O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
		0 );

	if( ( hf == -1 ) || ( hf == 0 ) )
   {
		//(void) FDIDestroy(hfdi);
		pfdidest(hfdi);
		sprintf(lpo,"ERROR: Unable to open [%s]!\n", cabinet_fullpath);
      prt(lpo);
		return FALSE;
	}
   else if( VERBMAX )
   {
      LARGE_INTEGER li;
      li.LowPart = GetFileSize((HANDLE) hf,  // handle to file
         (PULONG)&li.HighPart );   // high-order word of file size
		sprintf(lpo,"Openned with handle=%#x (%d) Size=%s bytes.\n",
         hf, hf,
         GetI64Stg2(li) );
      prt(lpo);
   }

//	if (FALSE == FDIIsCabinet(
//			hfdi,
//			hf,
//			&fdici))
	if( FALSE == pfdiiscab(	hfdi, hf, &fdici) )
	{
		/* ************************ *
		 * No, it's not a cabinet!  *
		 ************************** */
		file_close(hf);
		sprintf(lpo,
			"FDIIsCabinet() failed: [%s] is NOT a cabinet!\n",
			cabinet_fullpath );
      prt(lpo);
		//(void) FDIDestroy(hfdi);
		(pfdidest) (hfdi);
		return FALSE;
	}
	else
	{
		file_close(hf);
		gdwCabSize = fdici.cbCabinet;
      gbHasPrev = fdici.hasprev; // TRUE ? "yes" : "no",
      gbHasNext = fdici.hasnext; // TRUE ? "yes" : "no" );
		if( gbShowInfo )
		{
			sprintf(lpo,
				"Information on cabinet file '%s'\n"
				"   Total length of cabinet file : %d\n"
				"   Number of folders in cabinet : %d\n"
				"   Number of files in cabinet   : %d\n"
				"   Cabinet set ID               : %d\n"
				"   Cabinet number in set        : %d\n"
				"   RESERVE area in cabinet?     : %s\n"
				"   Chained to prev cabinet?     : %s\n"
				"   Chained to next cabinet?     : %s\n"
				"\n",
				cabinet_fullpath,
				fdici.cbCabinet,
				fdici.cFolders,
				fdici.cFiles,
				fdici.setID,
				fdici.iCabinet,
				fdici.fReserve == TRUE ? "yes" : "no",
				fdici.hasprev == TRUE ? "yes" : "no",
				fdici.hasnext == TRUE ? "yes" : "no" );
         prt(lpo);
		}
		else
		{
			sprintf(lpo,
				"List of %d File(s) (BT=%s ID=%#x CAB#=%d) ",
				fdici.cFiles,
            GetDWStg2(fdici.cbCabinet),
            fdici.setID,
            fdici.iCabinet );
			if( fdici.hasprev )	// == TRUE ? "yes" : "no",
				strcat(lpo, "P");
			if( fdici.hasnext )	//== TRUE ? "yes" : "no" );
				strcat(lpo, "N");
			strcat(lpo,"\n");
			prt(lpo);
		}
	}

	p = strrchr(cabinet_fullpath, '\\');
	if (p == NULL)
	{
		strcpy(gc_cabinet_name, cabinet_fullpath);
		strcpy(gc_cabinet_path, "");
	}
	else
	{
		strcpy(gc_cabinet_name, p+1);
		strncpy(gc_cabinet_path, cabinet_fullpath, (int) (p-cabinet_fullpath)+1);
		gc_cabinet_path[ (int) (p-cabinet_fullpath)+1 ] = 0;
	}

   strcpy(gcPrevPath,gc_cabinet_path);
//	if (TRUE != FDICopy(
//		hfdi,
//		cabinet_name,
//		cabinet_path,
//		0,
//		notification_function,
//		NULL,
//		NULL))

	if( TRUE != pfdicopy(
		hfdi,
		gc_cabinet_name,
		gc_cabinet_path,
		0,
		notification_function,
		NULL,
		NULL) )
	{
		sprintf(lpo,
			"FDICopy() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper) );
      prt(lpo);
		//(void) FDIDestroy(hfdi);
		pfdidest(hfdi);
		return FALSE;
	}

//	if (FDIDestroy(hfdi) != TRUE)
	if( (	pfdidest(hfdi) ) != TRUE )
	{
		sprintf(lpo,
			"FDIDestroy() failed: code %d [%s]\n",
			erf.erfOper, return_fdi_error_string(erf.erfOper) );
      prt(lpo);
		return FALSE;
	}

//   CheckPrev();

	return TRUE;
}


char *return_fdi_error_string(FDIERROR err)
{
	switch (err)
	{
		case FDIERROR_NONE:
			return "No error";

		case FDIERROR_CABINET_NOT_FOUND:
			return "Cabinet not found";
			
		case FDIERROR_NOT_A_CABINET:
			return "Not a cabinet";
			
		case FDIERROR_UNKNOWN_CABINET_VERSION:
			return "Unknown cabinet version";
			
		case FDIERROR_CORRUPT_CABINET:
			return "Corrupt cabinet";
			
		case FDIERROR_ALLOC_FAIL:
			return "Memory allocation failed";
			
		case FDIERROR_BAD_COMPR_TYPE:
			return "Unknown compression type";
			
		case FDIERROR_MDI_FAIL:
			return "Failure decompressing data";
			
		case FDIERROR_TARGET_FILE:
			return "Failure writing to target file";
			
		case FDIERROR_RESERVE_MISMATCH:
			return "Cabinets in set have different RESERVE sizes";
			
		case FDIERROR_WRONG_CABINET:
			return "Cabinet returned on fdintNEXT_CABINET is incorrect";
			
		case FDIERROR_USER_ABORT:
			return "User aborted";
			
		default:
			return "Unknown error";
	}
}


// extracted from grmout.c - sept 2000
/* Oct 99 update - retreived from DDBData.c */

// ===========================================================
// void Buffer2Stg( LPSTR lps, LPSTR lpb, int decimal,
//				 int sign, int precision )
//
// Purpose: Convert the string of digits from the _ecvt
//			function to a nice human readbale form.
//
// 1999 Sept 7 - Case of removing ?.?0000 the zeros
//
// ===========================================================
void Buffer2Stg( LPSTR lps, LPSTR lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;

	k = 0;					// Start at output beginning
	cad = 0;				// Count AFTER the decimal
	j = lstrlen( lpb );		// Get LENGTH of buffer digits

	if( sign )				// if the SIGN flag is ON
		lps[k++] = '-';		// Fill in the negative

	l = decimal;
	if( l < 0 )
	{
		// A NEGATIVE decimal position
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		// Possible shortened use of the digit string
		// ie possible LOSS of DIGITS to fit the precision requested.
		if( decimal == 0 )
		{
			if( ( precision - 1 ) < j )
			{
				//chkme();
				j = precision - 1;
			}
		}
		else
		{
			if( precision < j )
			{
//				chkme();
				j = precision;
			}
		}
	}

	sig = 0;	// Significant character counter
	// Process each digit of the digit list in the buffer
	// or LESS than the list if precision is LESS!!!
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];		// Get a DIGIT
		if( i == decimal )	// Have we reached the DECIMAL POINT?
		{
			// At the DECIMAL point
			if( i == 0 )	
			{
				// if no other digits BEFORE the decimal
				lps[k++] = '0';	// then plonk in a zero now
			}
			lps[k++] = '.';	// and ADD the decimal point
			cad++;
		}
		// Check for adding a comma for the THOUSANDS
		if( ( decimal > 0 ) &&
			( sig ) &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';	// Add in a comma
		}
		lps[k++] = c;	// Add this digit to the output
		if( sig )		// If we have HAD a significant char
		{
			sig++;		// Then just count another, and another etc
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}	// while processing the digit list

	// FIX980509 - If digit length is LESS than decimal position
	// =========================================================
	if( ( decimal > 0 ) &&
		( i < decimal ) )
	{
		c = '0';
		while( i < decimal )
		{
			if( ( decimal > 0 ) &&
				( sig ) &&
				( i < decimal ) )
			{
				m = decimal - i;
				if( (m % 3) == 0 )
					lps[k++] = ',';	// Add in a comma
			}
			lps[k++] = c;	// Add this digit to the output
			i++;
		}
	}
	// =========================================================
	if( cad )
		cad--;
	lps[k] = 0;		// zero terminate the output
	// FIX990907 - Remove unsightly ZEROs after decimal point
    for( i = 0; i < k; i++ )
    {
        if( lps[i] == '.' )
            break;
    }
    if( ( i < k ) &&
        ( lps[i] == '.' ) )
    {
        i++;
        if( lps[i] == '0' )
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
            if( k > i )
            {
                // we have backed to a not '0' value so STOP
            }
            else
            {
                // we backed all the way, so remove the DECIMAL also
                i--;
                lps[i] = 0;
            }
        }
        else
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
        }
    }

}

char    GMUpper( char c )
{
    char    d;
    if( (c >= 'a') && (c <= 'z') )
        d = (char)(c & 0x5f);
    else
        d = c;
    return( d );
}

int		GMInStr( LPSTR lpsrc, LPSTR lpfind )
{
	int		iAt, islen, iflen, i, j, k;
	char	c, d, b;

	iAt = 0;	// NOT FOOUND yet
   islen = iflen = 0;
	if( lpsrc && lpfind )
   {
      islen = lstrlen( lpsrc );
      iflen = lstrlen( lpfind );
   }
   if( islen && iflen )
	{
		d = GMUpper( lpfind[0] );
		for( i = 0; i < islen; i++ )
		{
			c = GMUpper( lpsrc[i] );
			if( c == d )
			{
				if( iflen == 1 )
				{
					// The FIND location ***PLUS*** ONE (1)!!!
					iAt = i+1;
					break;
				}
				else
				{
					if( (islen - i) >= iflen )
					{
						// ok, we have the length
						k = i + 1;	// Get to NEXT char
						for( j = 1; j < iflen; j++ )
						{
							c = GMUpper( lpsrc[k] );	// Get next
							b = GMUpper( lpfind[j] );
							if( c != b )
								break;
							k++;
						}
						if( j == iflen )
						{
							// FIX981106 - This should be
							//iAt = k + 1;
							// The FIRST char FIND location
							// ***PLUS*** ONE (1)!!!
							iAt = i + 1;
							break;
						}
					}
					else
					{
						// not enough length left
						break;
					}
				}
			}
		}
	}
	return iAt;
}


void	RTrimDecimal1( LPSTR lpr )
{
	int		i, j, k, dlen;
	char	c, d;
	LPSTR	lpend;

	c = 0;
   i = j = k = 0;
	if( lpr )
   {
      i = lstrlen( lpr );
      k = GMInStr( lpr, "." );
		j = ( i - k );
   }
   if( ( i && k ) &&
		( i < 128 ) &&
		( j > 2 ) )
	{
		// Also look for runs, especially say 0.799999997
		dlen = j - 1;

		c = lpr[i - 1];	// Get last char of string
		if( ( c >= '0' ) &&
			( c <= '9' ) )
		{

			lpend = &lpr[ i - 1 ];
			*lpend = 0;		// kill end
			lpend--;
			//k = i - k;
			if( c >= '5' )
			{
				//if( k )
				//	k--;
				d = c;
				c = *lpend;
				if( ( c >= '0' ) &&
					( c <= '9' ) )
				{
					if( c < '9' )
					{
						c++;
						*lpend = c;
					}
					else
					{
				//		if( k )
				//			k--;
				//		while( k-- )
						while( dlen-- )
						{
							*lpend = '0';
							lpend--;
							d = c;
							c = *lpend;
							if( ( c >= '0' ) &&
								( c <= '9' ) )
							{
								if( c < '9' )
								{
									c++;
									*lpend = c;
									break;
								}
								else
								{
									if( k )
									{
										*lpend = '0';
										lpend--;
									}
								}
							}
							else
							{
								break;
							}
						}	// backing up
					}
				}	// back one is a number
			}	// we are removing a 5 or more
		}
	}
}

void	RTrimDecimal( LPSTR lpr )
{
	int		i, j, k, l;
	char	c;
	char	szless[128];
	LPSTR	lpl;

   i = 0;
	if( lpr )
      i = lstrlen( lpr );
   if(i)
	{
		k = GMInStr( lpr, "." );
		if(k)
		{
			// Returns LOCATION of DECIMAL
			k--;	// back to char BEFORE decimal
			for( j = (i - 1); j >= k; j-- )
			{
				c = lpr[j];		// Get END (after DECIMAL POINT!)
				if( c == '0' )
				{
					lpr[j] = 0;
				}
				else
				{
					if( c == '.' )
						lpr[j] = 0;
					break;
				}
			}
		}	// if it contains a DECIMAL point

		// ==============================
		// 2nd processing ==============
		// ==============================
		i = lstrlen( lpr );
		k = GMInStr( lpr, "." );
		if( ( i ) &&
			( k ) &&
			( i < 128 ) &&
			( ( i - k ) > 2 ) )
		{
			// Also look for runs, especially say 0.799999997
			j = ( k + 1 );
			k = i - j;	// length of decimal number
			lpl = &szless[0];
			strcpy( lpl, lpr );
			RTrimDecimal1( lpl );
			if( ( lstrlen( lpl ) ) &&
				( lstrlen( lpl ) < i ) &&
				( GMInStr( lpl, "." ) ) )
			{
				i = lstrlen( lpl );
				k = GMInStr( lpl, "." );
				for( j = (i - 1); j > k; j-- )
				{
					c = lpl[j];
					if( c == '0' )
					{
						lpl[j] = 0;
					}
					else
					{
						break;
					}
				}
				if( (lstrlen(lpl) + 2 ) < lstrlen(lpr) )
				{
					strcpy( lpr, lpl );
				}
			}
			j = ( GMInStr( lpr, "." ) + 1 );

		}
		// ==============================
		// 3rd processing ==============
		// ==============================
		i = lstrlen( lpr );
      k = GMInStr( lpr, "." );
		if( ( i ) &&
			( k ) &&
			( i < 128 ) &&
			( ( i - k ) > 6 ) )
		{
			// Ok, I specifically want to avoid such things as
			// 48.000000082 and 47.999999992, etc
			// Returns LOCATION of DECIMAL
			k++;	// to char AFTER decimal
			lpl = &szless[0];
			j = k;
			l = 0;		// Repeat counter
			*lpl = 0;
			for( ; j < i; j++ )
			{
				c = lpr[j];		// Get chars (after DECIMAL POINT)
				if( c == *lpl )
				{
					l++;		// Count SAMENESS
				}
				else
				{
					if( l > 5 )
					{
						// we have HAD 5 of these chars
						// ============================
						l++;	// Bump for FIRST of these
						lpl = &lpr[ ( j - l ) ];	// Get FIRST
						c = *lpl;	// Get FIRST
						*lpl = 0;
						lpl--;		// Back one more
						l++;		// count one more back before the zero created
						if( *lpl == '.' )
						{
							l++;	// Count one more
							*lpl = 0;
							lpl--;
						}
						if( c >= '5' )
						{
							for( j = (j - l); j >= 0; j-- )
							{
								if( ( j ) && ( lpr[j] == '.' ) )
								{
									// backed up to the DECIMAL
									lpr[j] = 0;		// Kill it
									j--;			// back one
									if( lpr[j] < '9' )
									{
										c = lpr[j];
										c++;
										lpr[j] = c;
										break;		// all done here
									}
									else if( lpr[j] == '9' )
									{
										lpr[j] = '0';	// bump to next
									}
									else
									{
										break;	// a NON-NUMBER or "."!!!
									}
								}
								else if( lpr[j] < '9' )
								{
									c = lpr[j];
									c++;
									lpr[j] = c;
									break;	// All done here
								}
								else if( lpr[j] == '9' )
								{
									lpr[j] = '0';	// bump to next
								}
								else
								{
									break;
								}
							}	// for a BACKWARDS count
							// SPECIAL CASE
							// ============
							if( j < 0 )
							{
								// Ok, we must INSERT a "1"
								lpl = &szless[0];
								strcpy( lpl, lpr );
								strcpy( lpr, "1" );
								strcat( lpr, lpl );
							}
						}	// if ROUNDING-UP required.
						break;
					}
					l = 0;
				}
				*lpl = c;
			}

		}	// if it contains ".", and is greater than 6 places
	}
}

// Dbl2Str Dbl2Stg DbltoStg
void	Double2Stg( LPSTR lps, double factor )
{
	int		decimal, sign, precision;
	char *	buffer;

	precision = 16;
	buffer = _ecvt( factor, precision, &decimal, &sign );
	Buffer2Stg( lps, buffer, decimal, sign, precision );
}

/*	=======================================================
	void	Dbl2Stg( LPSTR lps, double factor, int prec )
	if( prec )
		precision = prec;
	else
		precision = 16;
	buffer = _ecvt( factor, precision, &decimal, &sign );
	Buffer2Stg( lps, buffer, decimal, sign, precision );
	======================================================= */

void    Dbl2Stg( LPSTR lps, double factor, int prec )
{
    int             decimal, sign, precision;
    char *  buffer;

    if( prec )
        precision = prec;
    else
        precision = 16;

    buffer = _ecvt( factor, precision, &decimal, &sign );

    Buffer2Stg( lps, buffer, decimal, sign, precision );
}


int	sprintf3( char * cp, char * fp, DWORD siz )
{
	int	ilen, i, j, k;
	char buf[64];

	ilen = sprintf( buf, fp, siz );	// format it to a buffer
	strcpy( cp, buf );
	i = strlen(buf);
	if(i)
	{
		j = 0;
		k = i - 1;
		while( ( i >= 0 ) && ( k >= 0 ) )
		{
			i--;
			if( buf[i] <= ' ' )
				break;
			if( j == 3 )
			{
				cp[k--] = ',';
				j = 0;
			}
			cp[k--] = buf[i];
			j++;
		}
	}
	return ilen;
}

LPTSTR   GetI64Stg2( LARGE_INTEGER li )
{
   static   TCHAR _s_buf[ (64 + 16) ];
   static   _s_ii = 0;
   LPTSTR   lps;
   TCHAR    buf[32];
   int      i,j,k;
   sprintf(buf,
      "%I64d",
      li );
   if( _s_ii )
   {
      lps = &_s_buf[(32+8)];
      _s_ii =0;
   }
   else
   {
      lps = &_s_buf[0];
      _s_ii =1;
   }
   *lps = 0;   // clear any previous
   i = strlen(buf);  // get its length
   if( i )  // get its length
   {
      k = 32;
      j = 0;
      lps[k+1] = 0;  // esure ZERO termination
      while( ( i > 0 ) && ( k >= 0 ) )
      {
         i--;     // back up one
         if( j == 3 )   // have we had a set of 3?
         {
            lps[k--] = ',';   // ok, add a comma
            j = 0;            // and restart count of digits
         }
         lps[k--] = buf[i];   // move the buffer digit
         j++;
      }
      k++;  // back to LAST position
      lps = &lps[k]; // pointer to beginning of 'nice" number
   }
   return lps;
}

LPTSTR   GetDWStg2( DWORD dwi )
{
   LARGE_INTEGER li;
   li.LowPart = dwi;
   li.HighPart = 0;
   return( GetI64Stg2(li) );
}

// eof - dircab.c
// eof - Dump4Cab.c
