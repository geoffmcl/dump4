

// Dump4v.c

#include	"Dump4.h"

BOOL	IsVosTab( LPDFSTR lpdf, LPTSTR lpb, DWORD dwRd )
{
	BOOL flg = FALSE;
   LPTSTR lptmp = &gszTmpOut[0];

   if( ( dwRd > 2 ) &&
      ( lpdf->qwSize.HighPart == 0 ) &&
      ( lpdf->qwSize.LowPart == dwRd ) )
   {
      if( ( lpb[ ( dwRd - 2 ) ] == 0x0d ) &&
         (  lpb[ ( dwRd - 1 ) ] == 0x0a ) )
      {
         // we have one at the END at least
         flg = TRUE;
      }
      else if( VERB )
      {
         prt( "NOTE: This file does NOT end in a Cr/Lf pair, thus is NOT a VOS table!"MEOR );
      }
   }
   else
   {
			if( VERB )
			{
				sprintf( lptmp,
               "NOTE: A full file READ must be done for this -DD option!\r\n"
               "      Read = %d vs Size = %d bytes."MEOR,
               dwRd,
               lpdf->qwSize.LowPart );
				prt( lptmp );
			}
   }
	return flg;
}

typedef  struct {
   int   g_iBlockSize;
   int   g_iBlockCnt;
}GBLK, * LPGBLK;

typedef struct {
   int   v_iBlks;
   GBLK  v_sBlks[1];
}VBLKS, * LPVBLKS;


void  ShowBlks( LPVBLKS lpv )
{
   DWORD dwi, dwc, dwk, dwi2, dwk2;
   char *   lpf;
   LPGBLK   lpg, lpg2;
   LPTSTR lptmp = &gszTmpOut[0];
   if( ( dwc = lpv->v_iBlks ) > 1 )
   {
      prt( "Asside from the ubiquitous UNITY block we have ..."MEOR );
      dwk2 = dwk = 0;
      for( dwi = 1; dwi < dwc; dwi++ )
      {
         lpg = &lpv->v_sBlks[dwi];
         if(dwk)
         {
            int   im, ik;

            lpf = MEOR"Count of %3d blocks of %4d bytes ...";
            sprintf(lptmp,
               lpf,
               lpg->g_iBlockCnt,
               lpg->g_iBlockSize );
            ik = 0;
            for( dwi2 = 1; dwi2 < dwc; dwi2++ )
            {
               if( dwi2 < dwi )
               {
                  lpg2 = &lpv->v_sBlks[dwi2];
                  if( !( lpg2->g_iBlockCnt % lpg->g_iBlockCnt ) )
                  {
                     // is multiple of
                     im = ( lpg2->g_iBlockCnt / lpg->g_iBlockCnt );
                     sprintf(EndBuf(lptmp),
                        "(x%d)",
                        im );
                     ik++; // count multiple
                  }
               }
               else
               {
                  break;
               }
            }
            if(ik)
               dwk2++;
         }
         else
         {
            if( dwc > 2 )
               lpf = "Count of %3d blocks of %4d bytes ...";  // FIRST fitting ...
            else
               lpf = "Count of %d blocks of %d bytes ...";  // FIRST fitting ...

            sprintf(lptmp,
               lpf,
               lpg->g_iBlockCnt,
               lpg->g_iBlockSize );
         }
         prt(lptmp);
         dwk++;
      }

      if( dwk > 1 )
      {
         //sprintf(lptmp,
         //   MEOR"But some of these %d can be simple multiples."MEOR,
         //   dwk );
         if( dwk2 )
         {
            sprintf(lptmp,
               MEOR"But %d of these %d can be simple multiples."MEOR,
               dwk2,
               dwk );
         }
         else
         {
            sprintf(lptmp,
               MEOR"But some of these %d can be imposters!"MEOR,
               dwk );
         }
      }
      else
      {
         sprintf(lptmp,
            MEOR"This SINGLE block size seems unambiguous."MEOR,
            dwk );
      }
      prt(lptmp);
   }
}

int	ChkVosTab( LPDFSTR lpdf )
{
   int   iRet = 0;
   HFILE hf;
   LPTSTR fn;
   BYTE * lpb;
   DWORD rd;
//   DWORD fsiz;
   DWORD dwi;
   char  c, d;
   LPTSTR   lpc;
   DWORD    dwmbs;
   DWORD    dwbc = 0;
   LPTSTR lptmp = &gszTmpOut[0];
//   LPTSTR   lpout = &g_Stg[0];
//      ( fsiz = lpdf->qwSize.LowPart ) &&

   if( ( hf = (HFILE)lpdf->hf ) != 0 &&
      ( fn = lpdf->fn ) != 0 &&
      ( lpb = lpdf->lpb ) != 0 &&
      ( rd = lpdf->dwrd ) != 0 &&
      ( IsVosTab(lpdf,(char *)lpb,rd ) ) )
   {
      DWORD dwbgn;
      LPVBLKS lpv;
      LPGBLK   lpg;

      dwmbs = (( rd + 5 ) / 3);
      dwbgn = sizeof(VBLKS) + ( dwmbs * sizeof(GBLK) );
      if( ( lpv = (LPVBLKS)dMALLOC(dwbgn) ) == 0 )
      {
         chkme( "YOW!!! Memory FAILED!!!!!!!" );
         return iRet;
      }

      lpv->v_iBlks = 1;
      lpg = &lpv->v_sBlks[dwbc];
      lpg->g_iBlockCnt  = 1;
      lpg->g_iBlockSize = rd;
      dwbc++;
      lpg = &lpv->v_sBlks[dwbc];

      dwbgn = 0;
      d = 0;
      for( dwi = 0; dwi < rd; dwi++ )
      {
         lpc = (char *)&lpb[dwi];  // point to char
         c = *lpc;  // extract char
         if( ( d == 0x0d ) && // maybe END of a block
            ( c == 0x0a ) )
         {
            dwbgn = dwi + 1;        // get the "stride" size
            if( ( dwbgn < rd ) &&   // ensure last UNITY NOT counted again
               ( ( rd % dwbgn ) == 0 ) )
            {
               DWORD dwc;
               DWORD dwn;
               // whoa - this could be a multiple of the first
               dwc = 1;
               dwn = (dwbgn * dwc);
               lpg->g_iBlockCnt  = 0;
               lpg->g_iBlockSize = dwbgn;
               while( ( dwn < rd ) &&
                  ( lpb[ (dwn - 1) ] == 0x0a ) &&
                  ( lpb[ (dwn - 2) ] == 0x0d ) )
               {
                  dwc++;
                  dwn = (dwbgn * dwc);
               }
               lpg->g_iBlockCnt  = dwc;
               if( dwbc < dwmbs )   // while count is less that allocated max.
               {
                  dwbc++;
                  lpg = &lpv->v_sBlks[dwbc];
               }
            }
         }
         d = c;
      }

      if( dwbc > 1 )
      {
         // success
         lpv->v_iBlks = dwbc;
         ShowBlks(lpv);
         iRet++;
      }
      else
      {
         // VERY DODGY !!!
         strcpy(lptmp, "This file only has ONE block that conforms to VOS table form!"MEOR );
         prt(lptmp);
      }

      dFREE(lpv);
   }

   return iRet;

}


// eof - Dump4v.c
