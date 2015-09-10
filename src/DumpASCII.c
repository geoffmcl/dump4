
// DumpASCII.c
// Part of Dump4.c
// A 32-Bit CONSOLE Application
#include	"Dump4.h"
// FIX20070723 - add -a[n] to DUMP ASCII, n is min ASCII length
// FIX20091112 - add -aa to DUMP aplhanumeric
#define ISMYCHAR(a) ( gfDoASCII2 ? ( ISALPHA(a) || ISNUM(a) ) : ISSIGCHAR(a) )

DWORD giKeepInLine = 65;

static TCHAR _s_buf[MXDIAGBUF] = {"\0"};

static void out_buf( LPTSTR lpb )
{
    strcat(lpb,MEOR);
    prt(lpb);
    *lpb = 0;
}

static void end_prt(void)
{
    LPTSTR lpb = _s_buf;
    if(strlen(lpb)) out_buf(lpb);
    *lpb = 0;
}

static void prta( LPTSTR lpd )
{
    LPTSTR lpb = _s_buf;
    DWORD len = strlen(lpd);
    if( len > giKeepInLine ) {
        end_prt();
        strcat(lpd,MEOR);
        prt(lpd);
        return;
    } else if(( strlen(lpb) + len ) > giKeepInLine ) {
        out_buf(lpb);
    }
    if(*lpb) strcat(lpb," ");
    strcat(lpb,lpd);
}

void  ProcessASCII( LPDFSTR lpdf )
{
   PBYTE    pb = lpdf->lpb;
   DWORD    max = lpdf->dwmax;
   DWORD    dwi, len, tot, shtr, cnt;
   BYTE     b;
   PBYTE    pbgn = pb;
  	LPTSTR   lpd = &gszDiag[0];   // = MXDIAGBUF
   len = tot = shtr = cnt = 0;
   for( dwi = 0; dwi < max; dwi++ )
   {
      b = pb[dwi];
      if(len) {
         if( ISMYCHAR(b) ) {
            if( len >= (MXDIAGBUF - 24) ) {
               lpd[len] = 0;
               //strcat(lpd, MEOR);
               prta( lpd );
               tot += len;
               len = 0;
            }
            lpd[len] = (TCHAR)b;
            len++;
         }
         else
         {
            // END OF ASCII
            if( len >= (DWORD) giMinASCII ) {
                lpd[len] = 0;
                //strcat(lpd, MEOR);
                prta( lpd );
                tot += len;
                cnt++;
            } else {
               shtr++;
            }
            len = 0;    // start again
         }
      } else {
         if( ISMYCHAR(b) ) {
            lpd[len] = (TCHAR)b;
            len = 1;
            pbgn = &pb[dwi];
         }
      }
   }
   // out of buffer LOOP
   if( len >= (DWORD) giMinASCII ) {
      lpd[len] = 0;
      //strcat(lpd, MEOR);
      prt( lpd );
      tot += len;
      len = 0;
      cnt++;
   } else {
      shtr++;
   }
   end_prt();
   sprintf(lpd, "Output %u ASCII strings, %u characters, %u shorter than %u skipped."MEOR,
      cnt, tot, shtr, giMinASCII );
   prt(lpd);
}

// eof - DumpASCII.c
