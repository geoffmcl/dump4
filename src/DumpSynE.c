

// DumpSynE.c
#include	"Dump4.h"

#ifdef   ADDSYNPROF

#define  MXSLN       72
#define  DBGSYNE1

//0000:01f0 61 6D 65 FF 5F 73 65 6C  66 FF 00 00 00 00 00 00 ame._self.......
//0000:0200 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
//0000:0210 00 00 00 00 00 00 00 00  01 00 22 01 00 22 03 00 ..........".."..
//0000:0220 00 00 01 00 5B 01 00 5D  02 00 00 00 02 00 2F 2F ....[..]......//
//0000:0230 02 00 0D 0A 01 00 00 00  02 00 2F 2A 02 00 2A 2F ........../*..*/
//0000:0240 01 00 00 00 03 00 2F 2A  2A 02 00 2A 2F 01 00 00 ....../**..*/...
//0000:0250 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
//0000:0260 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
//0000:0270 00 00 00 00 00 00 00 FF  FF 00 00 00 00 00 00 00 ................
//0000:028f 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00    ...............

// WOW, this seems to be 15 or 16
typedef  struct   tagBLK {
   WORD  wd[15];
}BLK, * PBLK;

VOID  AddCtrlChr( INT c, LPTSTR lpbuf, PDWORD pik, PINT pd )
{
   LPTSTR   lpd = &gszDiag[0];   // 1K
   DWORD    ik = *pik;
   INT      d;
   DWORD    il, ir;

   if( c == '\r' )
   {
      lpbuf[ik++] = '\\';
      d = 'r';
   }
   else if( c == '\n' )
   {
      lpbuf[ik++] = '\\';
      d = 'n';
   }
   else if( c == '\t' )
   {
      lpbuf[ik++] = '\\';
      d = 't';
   }
   else
   {
      sprintf(lpd, "%#x", (c & 0xff) );
      il = strlen( lpd );
      il--;
      lpbuf[ik++] = '\\';
      for( ir = 0; ir < il; ir++ )
      {
         lpbuf[ik++] = lpd[ir];
      }
      d = lpd[ir];
   }
   *pik = ik;
   *pd  = d;

}


BOOL	ChkSynEdit( LPDFSTR lpdf )
{
	BOOL	   bRet = FALSE;
	BOOL	   bRet2 = FALSE;
   HFILE    hf;
   PBYTE    pb, pb1;
   DWORD    ir;
   DWORD    ik, il;
   DWORD    rd, icnt1, icnt2, icnt3, icnt4, itot, icntb;
   INT      c;
   INT      d;
   LPTSTR   lpo = &gcOutBuf[0];  // = W.w_cOutBuf - [16K]
   LPTSTR   lpbuf = &gcOutBuf2[0];  // another 16K
   LPTSTR   pfn;
   PWORD    pw;
   DWORD    dwi, dwj;
   PBLK     pblk, pblk1;

   hf  = (HFILE)lpdf->hf;
   pfn = lpdf->fn;
   pb  = lpdf->lpb;
   rd  = lpdf->dwrd;
   icnt1 = icnt2 = icnt3 = icnt4 = itot = icntb = 0;
   if( ( VFHO(hf) ) &&
       ( pb       ) &&
       ( rd > 2   ) )
   {
      pw = (PWORD)pb;
      dwi = 0;
      dwj = *pw;
      while( (dwi+dwj+2) < rd )
      {
         icnt1++;
#ifdef   DBGSYNE1
         sprintf(lpo, "Lead word %d has value of %d (%#x). (Offset %#x [%d])"MEOR,
            icnt1, dwj, dwj,
            (dwi + dwj + 2),
            (dwi + dwj + 2) );
         prt(lpo);
#endif   // #ifdef   DBGSYNE1
         dwi += (dwj+2);
         pw = (PWORD)&pb[dwi];
         dwj = *pw;
         if( dwj == 0 )
            break;
      }

      pblk = pblk1 = (PBLK)pw;
// ***********************
      if( ( icnt1 > 0 ) && (dwi < rd) )
      {
         pw = (PWORD)((PBLK)pblk + 1);
         dwj = *pw;  // we jumped 15,
         while( ( dwj == 0 ) && ( dwi < rd ) )
         {
            // try 16, 17, etc
            pw++;
            dwi += 2;
            dwj = *pw;
         }
         *lpo = 0;
         while( (dwj) && ((dwi + dwj) < rd))
         {
            pw++;
            dwi += 2;
            pb1 = (PBYTE)pw;
            dwi += dwj;
            if( dwi < rd )
            {
               sprintf(EndBuf(lpo), "Begin l=%d [", dwj );
               ik = 0;
               for(ir = 0; ir < dwj; ir++ )
               {
                  c = pb1[ir];
                  if( ( c >= ' ' ) && ( c < 0x7f ) )
                  {
                     d = (TCHAR)c;
                  }
                  else
                  {
                     AddCtrlChr( c, lpbuf, &ik, &d );
                  }
                  lpbuf[ik++] = (TCHAR)d;
               }
               lpbuf[ik] = 0;
               il = strlen(lpbuf);
               strcat(lpo, lpbuf);
               strcat(lpo, "] ");
   
               pw = (PWORD)((PBYTE)pb1 + dwj);
               dwj = *pw;
               pw++;
               dwi += 2;
               pb1 = (PBYTE)pw;
               dwi += dwj;
               if( dwi < rd )
               {
                  sprintf(EndBuf(lpo), "End l=%d [", dwj );
                  ik = 0;
                  for(ir = 0; ir < dwj; ir++ )
                  {
                     c = pb1[ir];
                     if( ( c >= ' ' ) && ( c < 0x7f ) )
                     {
                        d = (TCHAR)c;
                     }
                     else
                     {
                        AddCtrlChr( c, lpbuf, &ik, &d );
                     }
                     lpbuf[ik++] = (TCHAR)d;
                  }
                  lpbuf[ik] = 0;
                  il = strlen(lpbuf);
                  strcat(lpo, lpbuf);
                  strcat(lpo, "] ");
                  pw = (PWORD)((PBYTE)pb1 + dwj);
                  sprintf(EndBuf(lpo), "Type %d", *pw );
                  pw++;
                  sprintf(EndBuf(lpo), "(val? %#x)", *pw );
                  pw++;
                  dwi += 4;
                  icnt3++;
                  icntb++;
                  if( dwj < rd )
                     dwj = *pw;
                  else
                     dwj = 0;
               }
               else
               {
                  dwj = 0;
               }
            }
            else
            {
               dwj = 0;
            }
            if( *lpo )
            {
               strcat(lpo, MEOR );
               //prt(lpo);
               *lpo = 0;
            }
         }  // while we have braces
      }
// ***********************
      dwi = 0;
      pw = (PWORD)pb;
      dwi = 0;
      dwj = *pw;
      *lpo = 0;
      icnt3 = 0;
      // OUTPUT PHASE
      if( ( icnt1 > 0 ) && (icntb > 0) )
      {
         while( (dwi+dwj+2) < rd )
         {
            icnt2++;
            pb1 = (PBYTE)((PWORD)pw+1);
            dwi += (dwj+2);
            if(dwj >= 2)
               dwj -= 2;
            else
               dwj = 0;
            ik = 0;
            for(ir = 0; ir < dwj; ir++ )
            {
               c = pb1[ir];
               if( ( c >= ' ' ) && ( c < 0x7f ) )
               {
                  lpbuf[ik++] = (TCHAR)c;
               }
               else if( c == (BYTE)-1 )
               {
                  icnt4++;
                  lpbuf[ik] = 0;
                  strcat(lpo, lpbuf);
                  strcat(lpo, " ");
                  ik = 0;
                  il = strlen(lpo);
                  if( il > MXSLN )
                  {
                     if( icnt3 == 0 )
                     {
                        sprintf(lpbuf, MEOR"Processing set %d."MEOR, icnt2 );
                        prt(lpbuf);
                        icnt3 = 1;
                     }
                     strcat(lpo, MEOR);
                     prt(lpo);
                     *lpo = 0;
                     il = 0;
                  }
               }
               else
                  break;
            }  // for the length of this block

            if( il ) // if some to output
            {
               if( icnt3 == 0 )
               {
                  sprintf(lpbuf, MEOR"Processing set %d."MEOR, icnt2 );
                  prt(lpbuf);
                  icnt3 = 1;
               }
               strcat(lpo, MEOR);
               prt(lpo);
               *lpo = 0;
               il = 0;
            }

            if( icnt4 )
            {
               // show the count
               sprintf(lpbuf, "Processed set %d with %d items."MEOR, icnt2, icnt4 );
               prt(lpbuf);
            }

            // accumulate total items
            itot += icnt4;

            // reset for the next set of items
            icnt3 = icnt4 = 0;

            pw = (PWORD)&pb[dwi];
            dwj = *pw;
            if( dwj == 0 )
               break;
         }  // while within the read in block

         if( icnt1 && itot )
         {
            sprintf(lpo, MEOR"Completed %d sets with a total of %d items."MEOR,
               icnt1, itot );
            prt(lpo);
         }
   
         icnt3 = icnt4 = 0;
   
         //if( icntb )
         {
            sprintf(lpo, MEOR"Processing %d sets of brace items."MEOR,
               icntb );
            prt(lpo);
         }

         // this is a GUESS at the structure that follows
         //pblk = (PBLK)pw;
         pblk = pblk1;  // get BLOCK pointer back
         pw = (PWORD)((PBLK)pblk + 1);
         dwj = *pw;
         //if( dwj == 0 )
         while( ( dwj == 0 ) && ( dwi < rd ) )
         {
            // try 16!!!
            pw++;
            dwi += 2;
            dwj = *pw;
         }
         *lpo = 0;
         while( (dwj) && ((dwi + dwj) < rd))
         {
            pw++;
            dwi += 2;
            pb1 = (PBYTE)pw;
            dwi += dwj;
            if( dwi < rd )
            {
               sprintf(EndBuf(lpo), "Begin l=%d [", dwj );
               ik = 0;
               for(ir = 0; ir < dwj; ir++ )
               {
                  c = pb1[ir];
                  if( ( c >= ' ' ) && ( c < 0x7f ) )
                  {
                     d = (TCHAR)c;
                  }
                  else
                  {
                     AddCtrlChr( c, lpbuf, &ik, &d );
                  }
                  lpbuf[ik++] = (TCHAR)d;
               }
               lpbuf[ik] = 0;
               il = strlen(lpbuf);
               strcat(lpo, lpbuf);
               strcat(lpo, "] ");
   
               pw = (PWORD)((PBYTE)pb1 + dwj);
               dwj = *pw;
               pw++;
               dwi += 2;
               pb1 = (PBYTE)pw;
               dwi += dwj;
               if( dwi < rd )
               {
                  sprintf(EndBuf(lpo), "End l=%d [", dwj );
                  ik = 0;
                  for(ir = 0; ir < dwj; ir++ )
                  {
                     c = pb1[ir];
                     if( ( c >= ' ' ) && ( c < 0x7f ) )
                     {
                        d = (TCHAR)c;
                     }
                     else
                     {
                        AddCtrlChr( c, lpbuf, &ik, &d );
                     }
                     lpbuf[ik++] = (TCHAR)d;
                  }
                  lpbuf[ik] = 0;
                  il = strlen(lpbuf);
                  strcat(lpo, lpbuf);
                  strcat(lpo, "] ");
                  pw = (PWORD)((PBYTE)pb1 + dwj);
                  sprintf(EndBuf(lpo), "Type %d", *pw );
                  pw++;
                  sprintf(EndBuf(lpo), "(val? %#x)", *pw );
                  pw++;
                  dwi += 4;
                  icnt3++;
                  if( dwj < rd )
                     dwj = *pw;
                  else
                     dwj = 0;
               }
               else
               {
                  dwj = 0;
               }
            }
            else
            {
               dwj = 0;
            }
            if( *lpo )
            {
               strcat(lpo, MEOR );
               prt(lpo);
               *lpo = 0;
            }
         }  // while we have braces
      }
   }

   if( icnt1 && itot && icnt3 && icntb )
      bRet2 = TRUE;

   if( bRet2 )
   {
      sprintf(lpo, MEOR"File appears to be a SYNEDIT profile file."MEOR
         "and what follows is a HEX dump of the contents."MEOR
         MEOR );
      prt(lpo);
   }
   else
   {
      if( pfn && *pfn )
      {
         sprintf(lpo, "WARNING: File [%s]"MEOR
            "\tdoes NOT appear to be a SYNEDIT profile!"MEOR,
            pfn );
      }
      else
      {
         sprintf(lpo, "WARNING: Loaded File does NOT appear to be a SYNEDIT profile!" MEOR);
      }
      prt(lpo);
   }

   return bRet;

}

#endif   // #ifdef   ADDSYNPROF

// eof - DumpSynE.c
