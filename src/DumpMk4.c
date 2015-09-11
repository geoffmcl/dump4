


// Dumpmk4.c
#include	"Dump4.h"


short kStor = 0x4C4A; // b0 = 'J', b1 = <4C> (on Intel)
short kRev  = 0x4A4C; // b0 = <4C>, b1 = 'J'

//class c4_FileHeader
typedef struct tagMK4HDR {
   short  _format;            // defines format and byte-order (4C4A/4A4C)
   BYTE   _spare;             // always 0x1A (CTRL-Z)
   BYTE   _flags;             // always 0x80 (allows future LoadValue)
   DWORD  _start;         // file offset, always low byte first
}MK4HDR, * PMK4HDR;

#define  ISPRT2(a)      ( ( a >= ' ' ) && ( a < 0x7f ) )

BOOL  IsMK4File( LPTSTR lpf, LPTSTR lpb, DWORD dwmax )
{
   BOOL  bRet = FALSE;
   LPTSTR   pb;
   PMK4HDR  pmk4 = (PMK4HDR)lpb;
   if( ( dwmax > sizeof(MK4HDR) ) &&
       ( ( pmk4->_format == kStor ) || ( pmk4->_format == kRev ) ) &&
       ( pmk4->_spare == 0x1a ) )
   {
      DWORD dwo = pmk4->_start;
      if( dwo < dwmax )
      {
         DWORD   dwl;
         PWORD   pw;

         pw = (PWORD) &lpb[dwo];
         pb = (LPTSTR)((PWORD)pw + 1);
         dwl = (DWORD)*pw;
         dwl = 0;
         while(1)
         {
            if( !ISPRT2( (*pb & 0xff) ) )
               break;
            pb++;
            dwl++;
         }
         if(dwl)
            bRet = TRUE;
      }
   }
   return bRet;
}
BOOL  DumpMK4( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   return bRet;
}

BOOL  ProcessMK4( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   if( IsMK4File( lpdf->fn, lpdf->lpb, (INT)lpdf->dwmax ) )
   {
      bRet = DumpMK4( lpdf );
   }

   return bRet;
}

