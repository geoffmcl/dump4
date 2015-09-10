
// DumpHex.c

#include	"Dump4.h"

#define  DO_OUT   DO_HEX_OUT( foff, lph, lpa )

#define swabi4(i4) (((i4) >> 24) + (((i4) >> 8) & 65280) + (((i4) & 65280) << 8) + (((i4) & 255) << 24))

typedef void (*OUTHEX)(PTSTR, DWORD, DWORD);

void DO_HEX_OUT( DWORD foff, PTSTR lph, PTSTR lpa )
{
	PTSTR lpo   = &gszTmpOut[0];
	BOOL  bAddH = gfAddHex;
   BOOL  bAddA = gfAddAscii;
   BOOL  bAddO = gfAddOffset;

   *lpo = 0;
   if(bAddO) {
      if( g_iNumbOffset )   // 4294967295
         sprintf( lpo, "%010u: ", foff );
      else
         sprintf( lpo, "%04x:%04x ", ((foff & 0xffff0000) >> 16), (foff & 0x0000ffff) );
   }
	if(bAddH)
      strcat( lpo, lph );
	if(bAddA)
      strcat( lpo, lpa );
   if(*lpo){
      strcat(lpo,MEOR);
      prt(lpo);
   }
}

void	OutputHex_ORG( LPTSTR lpb, DWORD dwBgn, DWORD dwEnd )
{
   char		c, d;
	LPTSTR	lph = &gszHex[0];
   LPTSTR   lpa = &gszAscii[0];
	LPTSTR 	lpo = &gszTmpOut[0];
	DWORD		foff, ir;
	int		fix, ia, iaw;
	BOOL     bAddH = gfAddHex;
   BOOL     bAddA = gfAddAscii;
   BOOL     bPar  = gfRemPar;
   BOOL     bPrt  = gw_fUsePrintf;
   BOOL     bAddO = gfAddOffset;
   PDWORD pdw = (PDWORD)&gdwordbuf[0];
   DWORD dwordcnt;

	foff = dwBgn;
	*lph = 0;
	fix = 0;
	iaw = ia = 0;
   dwordcnt = 0;
   for( ir = dwBgn; ir < dwEnd; ir++ )
	{
		c = lpb[ir];   // get character
		if( bPar )
			c = (char)(c & 0x7f);
		if( c < ' ' )
			d = '.';
		else if( c >= 0x7f )
			d = '.';
		else
			d = c;
      // =============================================
  		lpa[ia+fix] = d;	// Fill in the ASCII
      if( bPrt && ( d == '%' ) )	{ // Care using PRINTF
		   lpa[ia+fix] = d;	// Put in 2 for PRINTF
		   fix++;
	   }
   	ia++;    // bump ascii pointer

      // =============================================
      if( g_DoAsDWORDS ) {
         gdwordbuf[dwordcnt++] = c;
         if( dwordcnt == 4 ) {
            if(g_SwapBytes)
               *pdw = swabi4(*pdw);
   	      sprintf( EndBuf(lph), " %08X  ", *pdw );
            dwordcnt = 0;
         }
      } else {
	      sprintf( EndBuf(lph), "%02X ", (c & 0xff) );
  		   if( ia == 8 )
            strcat( lph, " " );
      }

      // =============================================
		if( ia == 16 ) {
			lpa[ia+fix] = 0;
  			DO_OUT;
			ia = 0;
			fix = 0;
			*lph = 0;
		}

      iaw++;
      if( iaw == 16 )
      {
         foff += iaw;
         iaw   = 0;
      }
	}  // for size of REQUEST = for( ir = dwBEGIN; ir < dwEND; ir++ )

	if( ia )	// Remaining data
	{
		foff += ia;
		lpa[ia+fix] = 0;
      if( g_DoAsDWORDS ) {
         if(dwordcnt) {
            while(dwordcnt < 4) {
               gdwordbuf[dwordcnt++] = 0;
            }
            if(g_SwapBytes)
               *pdw = swabi4(*pdw);
   	      sprintf( EndBuf(lph), " %08X  ", *pdw );
         }
		   while( ia < 16 ) {
			   strcat( lph, "   " );
			   ia++;
		   }
      } else {
		   if( ia < 8 )
			   strcat( lph, " " );
		   while( ia < 16 ) {
			   strcat( lph, "   " );
			   ia++;
		   }
      }
		DO_OUT;
		ia = 0;
		fix = 0;
		*lph = 0;
	}
}

void	OutputBYTE( PTSTR lpb, DWORD dwBgn, DWORD dwEnd )
{
   char		c, d;
	LPTSTR	lph = &gszHex[0];
   LPTSTR   lpa = &gszAscii[0];
	LPTSTR 	lpo = &gszTmpOut[0];
	DWORD		foff, ir;
	int		fix, ia, iaw;
	BOOL     bAddH = gfAddHex;
   BOOL     bAddA = gfAddAscii;
   BOOL     bPar  = gfRemPar;
   BOOL     bPrt  = gw_fUsePrintf;
   BOOL     bAddO = gfAddOffset;

	foff = dwBgn;
	*lph = 0;
	fix = 0;
	iaw = ia = 0;
   for( ir = dwBgn; ir < dwEnd; ir++ )
	{
		c = lpb[ir];   // get character
		if( bPar )
			c = (char)(c & 0x7f);
		if( c < ' ' )
			d = '.';
		else if( c >= 0x7f )
			d = '.';
		else
			d = c;
      // =============================================
  		lpa[ia+fix] = d;	// Fill in the ASCII
      if( bPrt && ( d == '%' ) )	{ // Care using PRINTF
		   lpa[ia+fix] = d;	// Put in 2 for PRINTF
		   fix++;
	   }
   	ia++;    // bump ascii pointer

      // =============================================
      sprintf( EndBuf(lph), "%02X ", (c & 0xff) );
	   if( ia == 8 )
         strcat( lph, " " );

      // =============================================
		if( ia == 16 ) {
			lpa[ia+fix] = 0;
  			DO_OUT;
			ia = 0;
			fix = 0;
			*lph = 0;
		}

      iaw++;
      if( iaw == 16 )
      {
         foff += iaw;
         iaw   = 0;
      }
	}  // for size of REQUEST = for( ir = dwBEGIN; ir < dwEND; ir++ )

	if( ia )	// Remaining data
	{
		foff += ia;
		lpa[ia+fix] = 0;
	   if( ia < 8 )
		   strcat( lph, " " );
	   while( ia < 16 ) {
		   strcat( lph, "   " );
		   ia++;
	   }
		DO_OUT;
		ia = 0;
		fix = 0;
		*lph = 0;
	}
}

void	OutputDWORD( PTSTR lpb, DWORD dwBgn, DWORD dwEnd )
{
   char		c, d;
	LPTSTR	lph = &gszHex[0];
   LPTSTR   lpa = &gszAscii[0];
	LPTSTR 	lpo = &gszTmpOut[0];
	DWORD		foff, ir;
	int		fix, ia, iaw;
	BOOL     bAddH = gfAddHex;
   BOOL     bAddA = gfAddAscii;
   BOOL     bPar  = gfRemPar;
   BOOL     bPrt  = gw_fUsePrintf;
   BOOL     bAddO = gfAddOffset;
   PDWORD pdw = (PDWORD)&gdwordbuf[0];
   DWORD dwordcnt;

	foff = dwBgn;
	*lph = 0;
	fix = 0;
	iaw = ia = 0;
   dwordcnt = 0;
   for( ir = dwBgn; ir < dwEnd; ir++ )
	{
		c = lpb[ir];   // get character
		if( bPar )
			c = (char)(c & 0x7f);
		if( c < ' ' )
			d = '.';
		else if( c >= 0x7f )
			d = '.';
		else
			d = c;
      // =============================================
  		lpa[ia+fix] = d;	// Fill in the ASCII
      if( bPrt && ( d == '%' ) )	{ // Care using PRINTF
		   lpa[ia+fix] = d;	// Put in 2 for PRINTF
		   fix++;
	   }
   	ia++;    // bump ascii pointer

      // =============================================
      gdwordbuf[dwordcnt++] = c;
      if( dwordcnt == 4 ) {
         if(g_SwapBytes)
            *pdw = swabi4(*pdw);
	      sprintf( EndBuf(lph), " %08X  ", *pdw );
         dwordcnt = 0;
      }

      // =============================================
		if( ia == 16 ) {
			lpa[ia+fix] = 0;
  			DO_OUT;
			ia = 0;
			fix = 0;
			*lph = 0;
		}

      iaw++;
      if( iaw == 16 )
      {
         foff += iaw;
         iaw   = 0;
      }
	}  // for size of REQUEST = for( ir = dwBEGIN; ir < dwEND; ir++ )

	if( ia )	// Remaining data
	{
		foff += ia;
		lpa[ia+fix] = 0;  // close the ASCII string
      if(dwordcnt) {
         while(dwordcnt < 4) {
            gdwordbuf[dwordcnt++] = 0;
         }
         if(g_SwapBytes)
            *pdw = swabi4(*pdw);
	      sprintf( EndBuf(lph), " %08X  ", *pdw );
         dwordcnt = 0;
      }
	   while( ia < 16 ) {
         gdwordbuf[dwordcnt++] = 0;
         if( dwordcnt == 4 ) {
	         strcat( lph, "           " );
            dwordcnt = 0;
         }
		   ia++;
	   }
		DO_OUT;
		ia = 0;
		fix = 0;
		*lph = 0;
	}
}


/* ==============================================================
   void	ProcessData( HANDLE hf, LPSTR fn, LPSTR lpb, DWORD len,
				  DWORD fsiz )

   Purpose: To process the data already read into the buffer as lines
      of OFFSET: HEXADECIMAL DIGITS "ascii char" Cr/Lf

   Inputs:
   HANDLE hf    = handle of the open file
   LPSTR fn    = name of the open file
   LPSTR lpb   = buffer where data was read in,
   DWORD len   = length of data actuall in buffer
   DWORD fsiz  = the full file length
      
   ============================================================== */

void	ProcessData( HANDLE hf, LPTSTR fn, LPTSTR lpb, DWORD rd, DWORD fsiz )
{
   DWORD    dwBgn = gdwBgnOff;
   DWORD    dwEnd = rd;
   DWORD dwi, dwn, dwb, dwe, dwc, dwd, dwbb;
	LPTSTR 	lpo = &gszTmpOut[0];
   OUTHEX   OutputHex = OutputBYTE;

   if( g_DoAsDWORDS )
      OutputHex = OutputDWORD;

   if( gdwEndOff ) {
      if(gdwEndOff < dwEnd)
         dwEnd = gdwEndOff;
   }
   if( g_pHexBlock ) {
      dwb = dwBgn;
      dwe = dwEnd;
      dwd = 0;
      dwbb = dwb;
      //sprintf(lpo,"Size of int = %d bytes, size of DWORD = %d bytes ..."MEOR,
      //   sizeof(int), sizeof(DWORD) );
      //prt(lpo);

      for( dwi = 0; ; dwi++ ) {
         dwn = g_pHexBlock[dwi].num;
         dwc = g_pHexBlock[dwi].cnt;
         if((dwn == 0)||(dwc == 0))
            break;
         if(VERB2) {
            sprintf(lpo,"Block output -blocks:%u x %u ..."MEOR, dwn, dwc );
            prt(lpo);
         }
         while(dwc--) {
            dwd++;
            dwe = dwb + dwn;
            if( dwe > dwEnd )
               dwe = dwEnd;
            if(VERB2) {
               sprintf(lpo,"Block %u ..."MEOR, dwd );
               prt(lpo);
            }
            OutputHex( lpb, dwb, dwe );
            dwb = dwe;
         }
      }
      OutputHex = OutputBYTE;
      if( dwb < dwEnd )
         OutputHex( lpb, dwb, dwEnd );
   } else {
      OutputHex( lpb, dwBgn, dwEnd );
   }

   UNREFERENCED_PARAMETER(hf);
   UNREFERENCED_PARAMETER(fsiz);
   UNREFERENCED_PARAMETER(fn);

}

void	ProcessData_ORGINAL( HANDLE hf, LPTSTR fn, LPTSTR lpb, DWORD rd, DWORD fsiz )
{
   char		c, d;
	LPTSTR	lph = &gszHex[0];
   LPTSTR   lpa = &gszAscii[0];
	LPTSTR 	lpo = &gszTmpOut[0];
	DWORD		foff, ir;
	int		fix, ia, iaw;
	BOOL     bAddH = gfAddHex;
   BOOL     bAddA = gfAddAscii;
   BOOL     bPar  = gfRemPar;
   BOOL     bPrt  = gw_fUsePrintf;
   BOOL     bAddO = gfAddOffset;
   DWORD    dwBgn = gdwBgnOff;
   DWORD    dwEnd = gdwEndOff;

	foff = 0;
	*lph = 0;
	fix = 0;
	iaw = ia = 0;
   if((dwBgn > 15) && (dwBgn & 0x0f))
   {
      dwBgn -= 15;
      dwBgn = ((dwBgn / 16) * 16);
   }
   for( ir = 0; ir < rd; ir++ )
	{
   	if( ( foff >= dwBgn ) &&
	   	 ( foff <= dwEnd ) )
      {
   		c = lpb[ir];   // get character
   		if( bPar )
   			c = (char)(c & 0x7f);
   		if( c < ' ' )
   			d = '.';
   		else if( c >= 0x7f )
   			d = '.';
   		else
   			d = c;
   		if( bAddA )
   		{
      		lpa[ia+fix] = d;	// Fill in the ASCII
   		   if( bPrt &&
   			   ( d == '%' ) )	// Care using PRINTF
   		   {
   			   lpa[ia+fix] = d;	// Put in 2 for PRINTF
   			   fix++;
   		   }
         }

  	   	ia++;    // bump ascii pointer

         if( bAddH )
         {
   		   sprintf( EndBuf(lph),
   			   "%02X ",
   			   (c & 0xff) );
     		   if( ia == 8 )
   			   strcat( lph, " " );
         }

   		if( ia == 16 )
   		{
   			lpa[ia+fix] = 0;
     			DO_OUT;
//     			foff += ia;
   			ia = 0;
   			fix = 0;
   			*lph = 0;
   		}
      }  // if within the given RANGE

      iaw++;
      if( iaw == 16 )
      {
         foff += iaw;
         iaw   = 0;
      }
	}  // for size of READ = for( ir = 0; ir < rd; ir++ )

	if( ia )	// Remaining data
	{
		foff += ia;
		lpa[ia+fix] = 0;
		if( ia < 8 )
			strcat( lph, " " );
		while( ia < 16 )
		{
			strcat( lph, "   " );
			ia++;
		}

		DO_OUT;

		ia = 0;
		fix = 0;
		*lph = 0;
	}

   UNREFERENCED_PARAMETER(fsiz);
   UNREFERENCED_PARAMETER(fn);

}



// eof - DumpHex.c
