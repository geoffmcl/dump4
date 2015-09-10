
// DumpGif.c
// see bottom for references used ...
/* =========================================================================
 * file2.c
 *
 * ========================================================================= */
//#include	"dv.h"	/* Single, all inclusing include ... */
//#include "wjpeglib.h"	/* ONLY module to include this ... ALL calls from here */
//#include	"WJPEG32\WJPEGLIB.h"
#include	"Dump4.h"
#ifndef  UINT16
#define  UINT16   WORD
#endif   // UINT16
#ifndef  UINT8
#define  UINT8   BYTE
#endif   // UINT8
#ifndef  MLPTR
#define  MLPTR *
#endif   // MLPTR

#define LM_to_uint(a,b)		((((b)&0xFF) << 8) | ((a)&0xFF))
#define BitSet(byte, bit)	((byte) & (bit))
#define INTERLACE       0x40	/* mask for bit signifying interlaced image */
#define COLORMAPFLAG    0x80	/* mask for bit signifying colormap presence */
#define MAXCOLORMAPSIZE	 256	/* max # of colors in a GIF colormap */
#define GNUMCOLORS	      3	/* # of colors */
#define CM_RED		         0	/* color component numbers */
#define CM_GREEN	         1
#define CM_BLUE		      2
#define MAX_LZW_BITS	     12	/* maximum LZW code size */
#define LZW_TABLE_SIZE	(1<<MAX_LZW_BITS) /* # of possible LZW symbols */

DWORD g_dwpalsz = 0;
DWORD g_dwImgColCnt = 0;
BOOL  g_bOutLess = TRUE;
BOOL  g_bWriteBmp = FALSE;  // write out a bmp of the conversion of the data
TCHAR g_szbmpout[264] = { "\0" };
DWORD g_colorcnt = 0;
BYTE  g_colormap[GNUMCOLORS][MAXCOLORMAPSIZE];
//extern void	GetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );
//extern	char    gszDrive[_MAX_DRIVE];          // Drive
//extern	char    gszDir[_MAX_DIR];          // Directory
//extern	char    gszFname[_MAX_FNAME];         // Filename
//extern	char    gszExt[_MAX_EXT];            // Extension

BOOL  g_addDecomp = TRUE;

BOOL	fExitLabelF9;
BOOL	fNetScape;
WORD	wLoopCnt;
WORD	is_interlaced, input_code_size;

typedef struct tagMIO {
   DWORD dwmax;
   DWORD dwoff;
   char * pbuf;
   long image_width;
   long image_height;
}MIO, * PMIO;

MIO   g_sMIO;

GIFIMGEXT _s_sGIE;
GIFHDREXT _s_sGHE;
LPGIFIMGEXT g_psGIE = &_s_sGIE;
LPGIFHDREXT g_psGHE = &_s_sGHE;
//  LPRGBQUAD lprgb ) = #define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]
#define  g_fGotTransp ( g_psGIE->gceBits & gce_TransColr )
 // = lpb[3];	// Transparency Index (if Bit set)
#define  g_iIndTrans g_psGIE->gceIndex
int   g_iWinIndex = -1;
int   g_iBlackInd = -1;
int   g_iWhiteInd = -1;
int   g_iRepeats  = 0;
BOOL  g_bTransTran = FALSE; // translate the TRANSPARENT colour to 'something' command '-gifT[...]


// fwd ref.
void InitLZWCode (void);
int LZWReadByte2 ( PMIO cinfo );
void init_interlace_row( PMIO cinfo );
long get_interlace_row (void);
VOID  ShowExtension( PMIO cinfo );
VOID  ClearExtensions(VOID);
void	chkgerr( void )
{
	int	i;
	i = 0;
}

BOOL IsGIFFile( LPSTR pfn, LPSTR pb, INT max )
{
   if(( pb[0] == 'G' ) &&
      ( pb[1] == 'I' ) &&
      ( pb[2] == 'F' ) )
   {
      return TRUE;
   }

   return FALSE;
}

VOID  OutGIFData( PBYTE lps, DWORD cc )
{
   TCHAR  buff[32];
   DWORD ui, uk, uj;
	LPTSTR lpd = &gszDiag[0];

   *lpd = 0;
   uk = 0;
   for( ui = 0; ui < cc; ui++ )
   {
      uj = lps[ui];
      sprintf(EndBuf(lpd),"%02X ", (uj & 0xff));

      if(uj & 0x80)
         uj = '.';
      else if(uj < ' ')
         uj = '.';

      buff[uk] = (TCHAR)uj;   // store ASCII, or dots for others
      uk++;
      if(uk == 16)
      {
         buff[uk] = 0;
         sprintf(EndBuf(lpd)," %s", buff);
         strcat(lpd,MEOR);
         prt(lpd);
         uk = 0;
      }

   }
   if(uk)
   {
      buff[uk] = 0;
      while(uk < 16)
      {
         strcat(lpd, "   ");
         uk++;
      }
         sprintf(EndBuf(lpd)," %s", buff);
         strcat(lpd,MEOR);
         prt(lpd);
         uk = 0;
   }

}

//DWORD    g_dwpalsz = 0;
//DWORD    g_dwImgColCnt = 0;
VOID  OutGIFColors( PBYTE lps, DWORD cc, BOOL glob )
{
   DWORD ui, uj, uk;
	LPTSTR lpd = &gszDiag[0];
   DWORD r,g,b;
   COLORREF   cr;

   g_dwImgColCnt = cc;
   g_iWinIndex = -1;
   g_iBlackInd = -1;
   g_iWhiteInd = -1;
   g_iRepeats  = 0;

   if(!cc)
      return;

   sprintf(lpd,
			"COLOUR TABLE - Count of RGBs = %d (%d Bytes)"MEOR,
			cc,
			( cc * 3 ) );
   prt(lpd);
   *lpd = 0;
   //prt( "Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
   for( ui = 0; ui < cc; ui++ )
   {
           //prt( "Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
      strcat(lpd, "Ind.  Colour     ");
      if(ui == 3)
         break;
   }
   strcat(lpd,MEOR);
   prt(lpd);

//		lpq = (RGBQUAD *)lps;

	*lpd = 0;
	uk = 0;
   g_dwpalsz = (cc * sizeof(RGBQUAD));  // set the PALETTE size - RGB in 4 byte array
   if(cc > 256)
      g_dwpalsz = (256 * sizeof(RGBQUAD));  // set the PALETTE size - RGB in 4 byte array
	for( ui = 0; ui < cc; ui++ )
	{
//  0(  0,  0,  0)   1(  0,  0,191)   2(  0,191,  0)   3(  0,191,191) 
      uj = (ui * GNUMCOLORS);
      r = lps[ ( uj + 0 ) ]; //  r = lpq->rgbRed;
      g = lps[ ( uj + 1 ) ];  // g = lpq->rgbGreen;
      b = lps[ ( uj + 2 ) ]; //  b = lpq->rgbBlue;
      cr = RGB(r,g,b);  // set windows COLORREF/DWORD/RGBQUAD

//int   g_iWinIndex = -1;
      //if((r == 0)&&(g == 128)&&(b == 128))
      if(cr == RGB(0,128,128))
         g_iWinIndex = ui;
      if(cr == RGB(0,0,0))
         g_iBlackInd = ui;
      if(cr == RGB(255,255,255))
         g_iWhiteInd = ui;
//int   g_iRepeats  = 0;

      sprintf( EndBuf(lpd),
				"%3X(%3d,%3d,%3d) ",
				ui,
            (r & 0xff), // 	(lpq->rgbRed & 0xff),
            (g & 0xff), // 	(lpq->rgbGreen & 0xff),
				(b & 0xff) );  // (lpq->rgbBlue & 0xff) );
//    BYTE    rgbBlue; 
//    BYTE    rgbGreen; 
//    BYTE    rgbRed;
      if( ui < 256 )
      {
//#define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]
//#define gdwColCount W.w_dwColCount // g DWORD [256]
//            gsBmpColour[ui] = *lpq; // keep COPY of colour from FILE TABLE
            gsBmpColour[ui].rgbRed = (unsigned char)(r & 0xff);   // RGB(r,g,b);
            gsBmpColour[ui].rgbGreen = (unsigned char)(g & 0xff);   // RGB(r,g,b);
            gsBmpColour[ui].rgbBlue = (unsigned char)(b & 0xff);   // RGB(r,g,b);
            if(ui)
            {
               DWORD dwi;
               for(dwi = 0; dwi < ui; dwi++)
               {
                  COLORREF cr2 = RGB(gsBmpColour[dwi].rgbRed,
                     gsBmpColour[dwi].rgbGreen,
                     gsBmpColour[dwi].rgbBlue );
                  if( cr == cr2 )
                  {
                     g_iRepeats++;
                  }
               }
            }

            if(glob)
            {
//#define CM_RED		         0	/* color component numbers */
               g_colormap[CM_RED][ui]   = (BYTE)r;
               g_colormap[CM_GREEN][ui] = (BYTE)g;
               g_colormap[CM_BLUE][ui]  = (BYTE)b;
               g_colorcnt++;
            }
      }

		uk++;
		if( uk == 4 )
		{
				strcat(lpd,MEOR);
				prt(lpd);
				*lpd = 0;
				uk = 0;
		}

//  4(191,  0,  0)   5(191,  0,191)   6(191,191,  0)   7(192,192,192) 
//248(128,128,128) 249(  0,  0,255) 250(  0,255,  0) 251(  0,255,255) 
//252(255,  0,  0) 253(255,  0,255) 254(255,255,  0) 255(255,255,255) 
//			lpq++;

   }

	if( uk )
	{
			strcat(lpd,MEOR);
			prt(lpd);
			*lpd = 0;
			uk = 0;
	}
}

void add_color_stg(LPTSTR lpd, UINT ui)
{
   if(ui < g_colorcnt) {
                           BYTE r,g,b;
                           r = g_colormap[CM_RED][ui];
                           g = g_colormap[CM_GREEN][ui];
                           b = g_colormap[CM_BLUE][ui];
                           sprintf(EndBuf(lpd),"(%3d,%3d,%3d)",
                              r, g, b );
           
   } else {
                           sprintf(EndBuf(lpd),"(Err OOR %3d)", ui);
                           chkme("Warning: BAD color index from LZW parser %d, %s"MEOR,
                              ui,
                              lpd );
           
   }
}

void chk_col_wrap( LPTSTR lpd, PDWORD pcols, PBOOL pskip, DWORD gcnt, DWORD cnt )
{
   DWORD cols = *pcols;
   BOOL  skip = *pskip;
   cols++;
   if(cols == cnt)
   {
      cols = 0;
      strcat(lpd,MEOR);

      if( !skip ) prt(lpd);
                  
      *lpd = 0;

      if( g_bDumpGif != 1 )
          skip = TRUE;

      while(strlen(lpd) < gcnt) strcat(lpd," ");
      
   }

   *pcols = cols;
   *pskip = skip;
}

BOOL	IsNetscape( LPSTR lpb, int i )
{
	BOOL	flg = FALSE;
	if(( lpb[i+0] == 'N' ) &&
		( lpb[i+1] == 'E' ) &&
		( lpb[i+2] == 'T' ) &&
		( lpb[i+3] == 'S' ) &&
		( lpb[i+4] == 'C' ) &&
		( lpb[i+5] == 'A' ) &&
		( lpb[i+6] == 'P' ) &&
		( lpb[i+7] == 'E' ) &&
		( lpb[i+8] == '2' ) &&
		( lpb[i+9] == '.' ) &&
		( lpb[i+10] == '0' ) &&
		( lpb[i+11] == 3 ) &&
		( lpb[i+12] == 1 ) )
	{
		flg = TRUE;
	}
	return flg;
}



DWORD	Gif_Count( LPSTR lps, DWORD ddSz )
{
	PUINT8	bhp;
	PUINT8	bhp2;
	DWORD	ddOff;
	WORD	GIFwidth, GIFheight;
	WORD	colormaplen, aspectRatio;
	WORD	gmaplen, icnt;
//	WORD	is_interlaced, input_code_size;
	BYTE	c;
	WORD	extlabel;
   DWORD row, col, cols, colw, rown;
	DWORD	glen, gcnt;
	BOOL	fHdExt, fColMap, skip;
	LPTSTR lpd = &gszDiag[0];
//typedef struct tagMIO {
//   DWORD dwmax;
//   DWORD dwoff;
//   char * pbuf;
//}MIO, * PMIO;
   PMIO pmio = &g_sMIO;
//   BITMAPFILEHEADER  w_bmfh;     // g_bmfh
//   BITMAPINFOHEADER  w_bih;      // g_bih
   BITMAPFILEHEADER * phdr = &g_bmfh;  // (BITMAPFILEHEADER *)pdib;
   LPBITMAPINFOHEADER pbi  = (LPBITMAPINFOHEADER)&g_bih;   // (LPBITMAPINFOHEADER)((BITMAPFILEHEADER *) phdr + 1);
   // FIX20150421 - Can be either BITMAPINFOHEADER (40) or BITMAPV5HEADER (124?)
   // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   PBYTE pbits = 0;
   PBYTE pbiti = 0;
   unsigned int ui;
   DWORD    off, dwi;

   pmio->dwmax = ddSz;
   pmio->dwoff = 0;
   pmio->pbuf  = lps;

	icnt = 0;
	fNetScape = FALSE;
	fExitLabelF9 = FALSE;
	fHdExt = FALSE;
	wLoopCnt = 0;
   fColMap = FALSE;
   ClearExtensions();   // start with NO extensions

	if( ddSz && (bhp = ( PINT8 ) lps) )
	{
  /* Read and verify GIF Header */
  // if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
		ddOff = 6;
		if( ddOff > ddSz )
		{
			chkgerr();
			return 0;
		}
		if( (bhp[0] != 'G') || (bhp[1] != 'I') || (bhp[2] != 'F') )
		{
			chkgerr();
			return 0;
		}
  /* Check for expected version numbers.
   * If unknown version, give warning and try to process anyway;
   * this is per recommendation in GIF89a standard.
   */
		if( (bhp[3] != '8' || bhp[4] != '7' || bhp[5] != 'a') &&
			 (bhp[3] != '8' || bhp[4] != '9' || bhp[5] != 'a') )
		{
			chkgerr();
			return 0;
		}

		if( (bhp[3] == '8') && (bhp[4] == '9') && (bhp[5] == 'a') )
			fNetScape++;

      if(fNetScape)
         strcpy(lpd, "File Version is GIF89a (Netscape) " );
      else
         strcpy(lpd, "File Version is GIF87a ");
      
      strcat(lpd, "from 6 byte GIF header."MEOR );
      prt(lpd);

		bhp2 = bhp + ddOff;	/* Past the HEADER of 6 */

		ddOff += 7;				/* and bump offset by next header 7 */
		if( ddOff > ddSz )	/* If out of data ... */
		{
			chkgerr();
			return 0;
		}

		GIFwidth    = LM_to_uint( bhp2[0], bhp2[1] );	/* Get G WIDTH */
		GIFheight   = LM_to_uint( bhp2[2], bhp2[3] );	/* Get G HEIGHT */
		colormaplen = 2 << ( bhp2[4] & 0x07 );	/* Extract Color Map Lenght */
  /* we ignore the color resolution, sort flag, and background color index */
		aspectRatio = bhp2[6] & 0xFF;	/* Extract aspect ratio ... */
		if( aspectRatio != 0 && aspectRatio != 49 )
		{
			chkgerr();
			return 0;
		}

      pmio->image_width  = GIFwidth;
      pmio->image_height = GIFheight;

		gmaplen = 0;
	/* Read global colormap if header indicates it is present */
		//if( BitSet( bhp2[4], COLORMAPFLAG ) )
		fColMap = BitSet( bhp2[4], COLORMAPFLAG );

      sprintf(lpd, "GIF Image: wid=%d, ht=%d, colors=%d, aspect=%d, colmap=%s"MEOR,
         GIFwidth,
         GIFheight,
         colormaplen,
         aspectRatio,
         (fColMap ? "T" : "F") );
      prt(lpd);

		if( fColMap )
		{
			//if( colormaplen > MAXCOLORMAPSIZE )
			//	colormaplen = MAXCOLORMAPSIZE;
			gmaplen = colormaplen;
			//ddOff += (colormaplen * GNUMCOLORS);	/* Bump the OFFSET */
			//if( ddOff > ddSz )
         if( (ddOff + (colormaplen * GNUMCOLORS)) > ddSz )	/* check the OFFSET */
			{
				chkgerr();
				return( icnt );
			}
   		bhp2 = bhp + ddOff;	/* Past the next block of 7 bytes */

         OutGIFColors( bhp2, colormaplen, TRUE );

			ddOff += (colormaplen * GNUMCOLORS);	/* Bump the OFFSET */
		}

		bhp2 = bhp + ddOff;
      /* Scan until we reach start of end of file */
      // ******************************************
		for( ;; )
		{
			c = *bhp2++;	/* Get BYTE and bump HUGE pointer ... */
			ddOff++;			/* and BUMP the Offset ... */
			if( c == ';' )		/* GIF terminator?? */
			{
            sprintf(lpd, "Reached normal GIF terminator at %d of %d bytes."MEOR,
               ddOff,
               ddSz );
            prt(lpd);
				break;	/* That's it folks ... */
			}

			if( c == '!' )
			{
            /* Extension */
            pmio->dwoff = ddOff; // update buffer offset
            ShowExtension( pmio );  // PMIO cinfo )

				/* Read extension label byte */
				extlabel = *bhp2;
				if( !fHdExt )
				{
					fHdExt = TRUE;
					if( fNetScape && ( extlabel == 0xff ) )
					{
						if(( bhp2[1] == 11         ) &&
                     ( IsNetscape( bhp2, 2 ) ) )
                  {
                     // ie
							//( bhp2[2] == 'N' ) &&
							//( bhp2[3] == 'E' ) &&
							//( bhp2[4] == 'T' ) &&
							//( bhp2[5] == 'S' ) &&
							//( bhp2[6] == 'C' ) &&
							//( bhp2[7] == 'A' ) &&
							//( bhp2[8] == 'P' ) &&
							//( bhp2[9] == 'E' ) &&
							//( bhp2[10] == '2' ) &&
							//( bhp2[11] == '.' ) &&
							//( bhp2[12] == '0' ) &&
							//( bhp2[13] == 3 ) &&
							//( bhp2[14] == 1 ) )
							fNetScape++;
							wLoopCnt = (WORD) (bhp2[15] + (bhp2[16] << 8));
						}
					}
					else if( extlabel == 0xf9 )
					{
						fExitLabelF9 = TRUE;
					}
				}

				bhp2++;		// Bump PAST "Exit Label"
				ddOff++;	// and ADD this BYTE ...
				glen = 0;
				while( (gcnt = bhp2[glen]) > 0 )
				{
               /* skip */
               skip = (((gcnt+1) > 16) && (g_bDumpGif != 1));
               if(skip)
               {
                  sprintf(lpd, "Skipping a extension block of %d bytes, showing first 16."MEOR, gcnt );
                  prt(lpd);
                  OutGIFData( &bhp2[glen], 16 );
               }
               else
               {
                  sprintf(lpd, "Skipping a extension block of %d bytes."MEOR, gcnt );
                  prt(lpd);
                  OutGIFData( &bhp2[glen], (gcnt+1) );
               }
					glen += gcnt + 1;	/* Bump by SKIP length PLUS Count byte ... */
					if( (ddOff + glen) > ddSz )
					{
						chkgerr();
						return 0;
					}
				}

				glen++;	/* Skipped LENGTH + final ZERO Count byte ... */
				ddOff += glen;	/* Skip the BLOCK(s) ... */
				if( ddOff > ddSz )
				{
					chkgerr();
					return 0;
				}

				bhp2 = bhp + ddOff;
				continue;
			}
			if( c != ',' )
			{
            /* Not an image separator? */
				continue;	/* Then get NEXT byte ... */
			}

			/* Read and decipher Local Image Descriptor */
			ddOff += 9;	/* Bump offset by 9 bytes ... */
			if( ddOff > ddSz )
			{
				chkgerr();
				return 0;
			}

			/* we ignore top/left position info, also sort flag */
			GIFwidth      = LM_to_uint( bhp2[4], bhp2[5] );
			GIFheight     = LM_to_uint( bhp2[6], bhp2[7]);
			is_interlaced = BitSet( bhp2[8], INTERLACE);
//   BITMAPFILEHEADER  w_bmfh;     // g_bmfh
//   BITMAPINFOHEADER  w_bih;      // g_bih

         pmio->image_width  = GIFwidth;
         pmio->image_height = GIFheight;
         init_interlace_row( pmio );
    /* Read local colormap if header indicates it is present */
    /* Note: if we wanted to support skipping images, */
    /* we'd need to skip rather than read colormap for ignored images */
			//if( BitSet( bhp2[8], COLORMAPFLAG ) ) 
			fColMap = BitSet( bhp2[8], COLORMAPFLAG );

         sprintf(lpd, "Local Image Descriptor(9): wid=%d, ht=%d, inter=%s colmap=%s"MEOR,
            GIFwidth,
            GIFheight,
            (is_interlaced ? "T" : "F"),
            (fColMap ? "T" : "F") );
         prt(lpd);

			if( fColMap ) 
			{
				colormaplen = 2 << ( bhp2[8] & 0x07);
				//if( colormaplen > MAXCOLORMAPSIZE )
				//	colormaplen = MAXCOLORMAPSIZE;
				//ddOff += (colormaplen * GNUMCOLORS) ;
				//if( ddOff > ddSz )
            if( (ddOff + (colormaplen * GNUMCOLORS)) > ddSz )
				{
					chkgerr();
					return 0;
				}
   			bhp2 = bhp + ddOff;
            OutGIFColors( bhp2, colormaplen, FALSE );
				ddOff += (colormaplen * GNUMCOLORS) ;
			}

			bhp2 = bhp + ddOff;
			input_code_size = *bhp2++; /* get minimum-code-size byte */
			ddOff++;

			if( input_code_size < 2 || input_code_size >= MAX_LZW_BITS )
			{
				chkgerr();
				return 0;
			}

         sprintf(lpd, "LZW imput_code_size = %d"MEOR, input_code_size );
         prt(lpd);

			icnt++;	/* Bump to NEXT, and ... */
			glen = 0;
//typedef struct tagMIO {
//   DWORD dwmax;
//   DWORD dwoff;
//   char * pbuf;
//}MIO, * PMIO;
         pmio->dwoff = ddOff; // set offset to beginning of 

			while( (gcnt = bhp2[glen]) > 0 )
			{
            /* skip */
            skip = (((gcnt + 1) > 16) && (g_bDumpGif != 1));
            if(skip) {
               sprintf(lpd, "Skipping a data block of %d bytes, showing only 16."MEOR, gcnt );
               prt(lpd);
               OutGIFData( &bhp2[glen], 16 );
            } else {
               sprintf(lpd, "Skipping a data block of %d bytes."MEOR, gcnt );
               prt(lpd);
            //OutGIFData( &bhp2[ (glen+1) ], gcnt );
               OutGIFData( &bhp2[glen], (gcnt + 1) );
            }

				glen += gcnt + 1;	/* Bump by SKIP length PLUS Count byte ... */
				if( (ddOff + glen) > ddSz )
				{
					chkgerr();
					return 0;
				}
			}

         if( g_addDecomp )
         {
            sprintf(lpd, "Decompressing LZW data to 8-bit color indexes, "
               "for %d rows of %d cols%s"MEOR,
               GIFheight,
               GIFwidth,
               ((g_bDumpGif != 1) ? ", showing only first set." : "." ));
            prt(lpd);

            pmio->image_width  = GIFwidth;
            pmio->image_height = GIFheight;
            init_interlace_row( pmio );
            ZeroMemory(phdr, sizeof(BITMAPFILEHEADER));
            // lpBmInfoHdr->biSizeImage = WIDTHBYTES(dwWidth * iBPP) * dwHeight;
            InitBitmapInfoHeader( pbi,    // LPBITMAPINFOHEADER lpBmInfoHdr,
                           GIFwidth,
                           GIFheight,
                           8 ); // 	  int nBPP )
            phdr->bfType  = DIB_HEADER_MARKER;   // simple "BM" signature
            //phdr->bfSize  = size;   // file size
            phdr->bfReserved1 = 0;
            phdr->bfReserved2 = 0;
            //phdr->bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER) + pbi->biSize + palsz;
            if(g_bOutLess && g_dwImgColCnt) {
               phdr->bfOffBits   = (DWORD)( sizeof(BITMAPFILEHEADER) + pbi->biSize +
                  g_dwpalsz );   // but must now also set
               pbi->biClrUsed = g_dwImgColCnt;  // set the USED value
            } else {
               // use 'standard' 256 colour palette
               phdr->bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER) + pbi->biSize +
                  (sizeof(RGBQUAD)*256);  // not palsz;
            } // g_bOutLess y/n

            phdr->bfSize      = phdr->bfOffBits +
               	pbi->biSizeImage;  // = WIDTHBYTES(dwWidth * iBPP) * dwHeight;

            colw = WIDTHBYTES(GIFwidth * 8);
            if( pbi->biSizeImage != (GIFheight * colw) )
               chkme( "Warning: %d vs %d ???????"MEOR, pbi->biSizeImage, (GIFheight * colw));

            if(g_bWriteBmp)
            {
               pbits = malloc((pbi->biSizeImage*2));
               pbiti = &pbits[pbi->biSizeImage];
            }
            InitLZWCode ();
            *lpd = 0;
            cols = 0;
            if(is_interlaced)
            {
               prt("For interlaced, the first scan is just a decode into rows, like..."MEOR );
               for( row = 0; row < GIFheight; row++ )
               {
                  sprintf(lpd,"Row %3d: ", (row + 1));
                  gcnt = strlen(lpd);
                  cols = 0;
                  skip = FALSE;  // only output first line of row
                  for( col = 0; col < GIFwidth; col++ )
                  {
                     ui = LZWReadByte2 ( pmio );
                     if(pbits)
                     {
                        off = ((row * colw) + col);   // caclulate offset
                        pbiti[off] = ui;  // and save decoded index
                     }
                     sprintf(EndBuf(lpd)," %3d", ui);
                     if( g_colorcnt )
                     {
                        add_color_stg(lpd,ui);
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 4 );
                     }
                     else
                     {
                        strcat(lpd," ");
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 16 );
                     }
                  }
               }

               prt("For interlaced, the second scan is into rows, like..."MEOR );
               // note already DONE init_interlace_row( pmio );
               for( row = 0; row < GIFheight; row++ )
               {
                  rown = get_interlace_row();
                  sprintf(lpd,"Row %3d: ", (rown + 1));
                  gcnt = strlen(lpd);
                  cols = 0;
                  skip = FALSE;  // only output first line of row
                  for( col = 0; col < GIFwidth; col++ )
                  {
                     //unsigned int ui = LZWReadByte2 ( pmio );
                     strcat(lpd," ");
                     if(pbits)
                     {
                        off = (( rown * colw ) + col);
                        ui = pbiti[off];  // access the interlace row
                        // calculate the bitmap row
                        off = (((GIFheight - row - 1) * colw) + col);
                        // convert index too
                        if((g_bTransTran     ) &&
                           (g_iIndTrans == ui) &&
                           (g_fGotTransp     ) )  // ( g_psGIE->gceBits & gce_TransColr )
                        {  // = lpb[3];	// Transparency Index (if Bit set)
                           dwi = ui;
                           if( g_iWinIndex != -1 )
                              dwi = g_iWinIndex;
                           else if( g_iBlackInd != -1 )
                              dwi = g_iBlackInd;
                           else if( g_iWhiteInd != -1 )
                              dwi = g_iWhiteInd;
                           if( ui != dwi ) {
                              sprintf(EndBuf(lpd),"(%d)", ui);
                              ui = dwi;
                           }
                        }

                        pbits[off] = ui;
                     }
                     sprintf(EndBuf(lpd),"%3d", ui);
                     if( g_colorcnt )
                     {
                        add_color_stg(lpd, ui);
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 4 );
                     }
                     else
                     {
                        strcat(lpd," ");
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 16 );
                     }
                  }
               }

            }
            else
            {
               // NOT interlaced - only ONE pass through the data
               prt("For non interlaced, the single scan decodes into rows, like..."MEOR );
               for( row = 0; row < GIFheight; row++ )
               {
                  //if(is_interlaced)
                  //   rown = get_interlace_row();
                  //else
                     rown = row;

                  sprintf(lpd,"Row %3d: ", (rown + 1));
                  gcnt = strlen(lpd);
                  cols = 0;
                  skip = FALSE;  // only output first line of row
                  for( col = 0; col < GIFwidth; col++ )
                  {
                     ui = LZWReadByte2 ( pmio );
                     strcat(lpd," ");
                     if(pbits)
                     {
                        if(GIFheight > rown)
                        {
                           //DWORD off = ( (rown * colw) + col );
                           //DWORD off = (((GIFheight - rown - 1) * colw) + col);
                           off = (((GIFheight - row - 1) * colw) + col);
                           if(off < pbi->biSizeImage)
                           {
                              // convert index too
                              if((g_bTransTran     ) &&
                                 (g_iIndTrans == ui) &&
                                 (g_fGotTransp     ) )  // ( g_psGIE->gceBits & gce_TransColr )
                              {  // = lpb[3];	// Transparency Index (if Bit set)
                                 dwi = ui;
                                 if( g_iWinIndex != -1 )
                                    dwi = g_iWinIndex;
                                 else if( g_iBlackInd != -1 )
                                    dwi = g_iBlackInd;
                                 else if( g_iWhiteInd != -1 )
                                    dwi = g_iWhiteInd;
                                 if( ui != dwi ) {
                                    sprintf(EndBuf(lpd),"(%d)", ui);
                                    ui = dwi;
                                 }
                              }
                              pbits[off] = ui; // insert index
                           }
                           else
                              chkme("Warning: Have offset outside bits buffer %d vs %d!"MEOR, off, pbi->biSizeImage);
                        }
                     }
                     sprintf(EndBuf(lpd),"%3d", ui);
                     if( g_colorcnt )
                     {
                        add_color_stg(lpd,ui);
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 4 );
                     }
                     else
                     {
                        strcat(lpd," ");
                        chk_col_wrap( lpd, &cols, &skip, gcnt, 16 );
                     }
                  }
               }
            }  // interlaced or not
            if(cols)
            {
               strcat(lpd,MEOR);
               if(!skip)
                  prt(lpd);
               *lpd = 0;
            }
         }

			glen++;	/* Skipped LENGTH + final ZERO Count byte ... */
			ddOff += glen;	/* Skip the BLOCK(s) ... */
			if( ddOff > ddSz )
			{
				chkgerr();
				return 0;
			}
// BOOL  g_bWriteBmp = command "-gif[n]:"  // write out a bmp of the conversion of the data
// TCHAR g_szbmpout[264] = { "\0" }; command "-gif[n]:outfile.bmp
         if(pbits)
         {
            static TCHAR _s_szbmp[264];
            LPTSTR   pbmp = _s_szbmp;
            FILE * fp;

            if( g_szbmpout[0] )
            {
               if(icnt == 1) {
                  strcpy(pbmp,g_szbmpout);   // use USER's given name "-gif[n]:outname.bmp
               } else {
                  // we have already written the user's named output, so, add number
                  sprintf(pbmp, "%s.[%d]", g_szbmpout, icnt);
               }

            }
            else
            {
               // just us a numeric temp set
               sprintf(pbmp,"TEMP%04d.bmp", icnt);
            }
            fp = fopen(pbmp, "wb");
            if(fp)
            {
      //   BITMAPFILEHEADER * phdr = &g_bmfh;  // (BITMAPFILEHEADER *)pdib;
      //   LPBITMAPINFOHEADER pbi  = &g_bih;   // (LPBITMAPINFOHEADER)((BITMAPFILEHEADER *) phdr + 1);
               gcnt = 0;
               gcnt += fwrite( phdr, 1, sizeof(BITMAPFILEHEADER), fp );
               gcnt += fwrite( pbi,  1, sizeof(BITMAPINFOHEADER), fp );
               //if(g_dwpalsz) {
               if(g_bOutLess && g_dwImgColCnt && g_dwpalsz) {
                  //fwrite( &gsBmpColour[0], 1, palsz, fp );
                  gcnt += fwrite( &gsBmpColour[0], 1, g_dwpalsz,            fp );
               } else {
                  gcnt += fwrite( &gsBmpColour[0], 1, (sizeof(RGBQUAD)*256), fp );
               } 
               gcnt += fwrite( pbits, 1, pbi->biSizeImage, fp );
               fclose(fp);
               sprintf(lpd,"Have written BMP to [%s] of %d bytes."MEOR, pbmp, gcnt);
               prt(lpd);
            }
            free(pbits);
            pbits = 0;
         }

			bhp2 = bhp + ddOff;

		}	/* Forever - until break or returned flag FALSE if error */
	}

   return icnt;

}

INT   GIFFrame( LPDFSTR lpdf )
{
   return( Gif_Count( lpdf->lpb, lpdf->dwmax ) );
}

BOOL  ProcessGIF( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   if( IsGIFFile( lpdf->fn, lpdf->lpb, (INT)lpdf->dwmax ) )
   {
      bRet = GIFFrame( lpdf );
      if( !bRet )
         prt( "Warning02: Does NOT appear to be in known GIF format!"MEOR );
   }
   else
   {
      prt( "Warning01: Does NOT appear to be in known GIF format!"MEOR );
   }
   return bRet;
}

// see refernces at the 'real' bottom of the file
// eof - DumpGif.c

// rdgif - 
#define	_NOWARNMESS
/* Static vars for GetCode and LZWReadByte */
#define	MAX_LZW_BITS	12	/* maximum LZW code size (4096 symbols) */
//#define LZW_TABLE_SIZE	(1 << MAX_LZW_BITS)

static char code_buf[256+4];	/* current input data block */
static int last_byte;		/* # of bytes in code_buf */
static int last_bit;		/* # of bits in code_buf */
static int cur_bit;		/* next bit index to read */
static BOOL out_of_blocks = FALSE;	/* TRUE if hit terminator data block */

// static int input_code_size;	/* codesize given in GIF file */
static int clear_code,end_code; /* values for Clear and End codes */

static int code_size;		/* current actual code size */
static int limit_code;		/* 2^code_size */
static int max_code;		/* first unused code value */
static BOOL first_time;	/* flags first call to LZWReadByte */

/* LZW decompression tables:
 *   symbol_head[K] = prefix symbol of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 *   symbol_tail[K] = suffix byte   of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 * Note that entries 0..end_code of the above tables are not used,
 * since those symbols represent raw bytes or special codes.
 *
 * The stack represents the not-yet-used expansion of the last LZW symbol.
 * In the worst case, a symbol could expand to as many bytes as there are
 * LZW symbols, so we allocate LZW_TABLE_SIZE bytes for the stack.
 * (This is conservative since that number includes the raw-byte symbols.)
 *
 * Here, the tables are statically allocated in the EXE, thus into
 * system page memory assigned to this process.
 */

// static table buffers - 4096 * 4 = 16K
static WORD symprefix[ (LZW_TABLE_SIZE * sizeof(WORD)) ];
static BYTE symsuffix[ (LZW_TABLE_SIZE * sizeof(BYTE)) ];
static BYTE symexpans[ (LZW_TABLE_SIZE * sizeof(BYTE)) ];

static UINT16 MLPTR symbol_head  = symprefix; /* => table of prefix symbols */
static UINT8  MLPTR symbol_tail  = symsuffix; /* => table of suffix bytes */
static UINT8  MLPTR symbol_stack = symexpans; /* stack for symbol expansions */
static UINT8  MLPTR sp;		/* stack pointer */


/* (Re)initialize LZW state; shared code for startup
   and Clear processing */
void ReInitLZW (void)
{
	code_size = input_code_size+1;
	limit_code = clear_code << 1;	/* 2^code_size */
	max_code = clear_code + 2;	/* first unused code value */
	sp = symbol_stack;		/* init stack to empty */
}


/* Initialize for a series of LZWReadByte
   (and hence GetCode) calls */
void InitLZWCode (void)
{
	/* GetCode initialization */
	last_byte = 2;		/* make safe to "recopy last two bytes" */
	last_bit = 0;			/* nothing in the buffer */
	cur_bit = 0;			/* force buffer load on first call */
	out_of_blocks = FALSE;

	/* LZWReadByte initialization */
	clear_code = 1 << input_code_size; /* compute special code values */
	end_code = clear_code + 1;	/* note that these do not change */
	first_time = TRUE;
	ReInitLZW();
}

//typedef struct tagMIO {
//   DWORD dwmax;
//   DWORD dwoff;
//   char * pbuf;
//}MIO, * PMIO;

int ReadByte2( PMIO cinfo )
{
   int   i;
   if( cinfo->dwoff >= cinfo->dwmax )
   {
      chkme("Warning: ReadByte2 is OUT OF DATA = END OF FILE"MEOR );
      return -1;
   }
   // get the char, and
   i = (cinfo->pbuf[cinfo->dwoff] & 0xff);

   cinfo->dwoff++;   // bump the INPUT pointer

   return i;
}

BOOL  ReadOK2( PMIO cinfo, char * pbuf, int count )
{
   int   i = 0;
   int   c;
   while(count--)
   {
      c = ReadByte2(cinfo);
      if(c == -1)
      {
         chkme("Warning: ReadOK2 is OUT OF DATA = END OF FILE"MEOR );
         return FALSE;
      }

      if(pbuf)
         pbuf[i++] = (char)c;

   }

   return TRUE;
}

/* Read a GIF data block, which has a leading count byte */
/* A zero-length block marks the end of a data block sequence */
//int GetDataBlock( compress_info_ptr cinfo, char MLPTR buf )
int GetDataBlock( PMIO cinfo, char * pbuf )
{
	int count = ReadByte2(cinfo); // get the GIF length byte/value
	if( count > 0 )   // and if NOT zero, then
	{
		//if( !ReadOK(cinfo->input_file, buf, count) )
		if( !ReadOK2(cinfo, pbuf, count) )  // read data into buffer
		{
         count = 0;  // or -1;
		}
  }
  return( count );
}

int   SkipDataBlocks( PMIO cinfo )
{
   int   skip = 1;
   int   skips = 0;
   while(skip)
   {
      int i = GetDataBlock( cinfo, 0 );
      if(i == -1)
         skip = 0;
      skips++;
   }

   return skips;
}

/* Fetch the next code_size bits from the GIF data */
/* We assume code_size is less than 16 */
//int GetCode (compress_info_ptr cinfo)
int GetCode ( PMIO cinfo )
{
  register INT32 accum;
  int offs, ret, count;

  if ( (cur_bit+code_size) > last_bit) {
    /* Time to reload the buffer */
    if( out_of_blocks )
	 {
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Ran out of GIF bits");
#endif
      return end_code;		/* fake something useful */
    }
    /* preserve last two bytes of what we have -- assume code_size <= 16 */
    code_buf[0] = code_buf[last_byte-2];
    code_buf[1] = code_buf[last_byte-1];
    /* Load more bytes; set flag if we reach the terminator block */
    if( (count = GetDataBlock(cinfo, &code_buf[2])) == 0 )
	 {
      out_of_blocks = TRUE;
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Ran out of GIF bits");
#endif
      return end_code;		/* fake something useful */
    }
    /* Reset counters */
    cur_bit = (cur_bit - last_bit) + 16;
    last_byte = 2 + count;
    last_bit = last_byte * 8;
  }

  /* Form up next 24 bits in accum */
  offs = cur_bit >> 3;		/* byte containing cur_bit */
#ifdef CHAR_IS_UNSIGNED
  accum = code_buf[offs+2];
  accum <<= 8;
  accum |= code_buf[offs+1];
  accum <<= 8;
  accum |= code_buf[offs];
#else
  accum = code_buf[offs+2] & 0xFF;
  accum <<= 8;
  accum |= code_buf[offs+1] & 0xFF;
  accum <<= 8;
  accum |= code_buf[offs] & 0xFF;
#endif

  /* Right-align cur_bit in accum, then mask off desired number of bits */
  accum >>= (cur_bit & 7);
  ret = ((int) accum) & ((1 << code_size) - 1);
  
  cur_bit += code_size;
  return ret;
}

int LZWReadByte2 ( PMIO cinfo )
/* Read an LZW-compressed byte */
{
  static int oldcode;		/* previous LZW symbol */
  static int firstcode;		/* first byte of oldcode's expansion */
  register int code;		/* current working code */
  int incode;			/* saves actual input code */

  /* First time, just eat the expected Clear code(s) and return next code, */
  /* which is expected to be a raw byte. */
  if (first_time) {
    first_time = FALSE;
    code = clear_code;		/* enables sharing code with Clear case */
  } else {

    /* If any codes are stacked from a previously read symbol, return them */
    if (sp > symbol_stack)
      return (int) *(--sp);

    /* Time to read a new symbol */
    code = GetCode(cinfo);

  }

  if (code == clear_code) {
    /* Reinit static state, swallow any extra Clear codes, and */
    /* return next code, which is expected to be a raw byte. */
    ReInitLZW();
    do {
      code = GetCode(cinfo);
    } while (code == clear_code);
    if (code > clear_code) 
	 {	/* make sure it is a raw byte */
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Corrupt data in GIF file");
#endif
      code = 0;			/* use something valid */
    }
    firstcode = oldcode = code;	/* make firstcode, oldcode valid! */
    return code;
  }

	if( code == end_code )
	{
    /* Skip the rest of the image, unless GetCode already read terminator */
    if (! out_of_blocks) {
      SkipDataBlocks(cinfo);
      out_of_blocks = TRUE;
    }
    /* Complain that there's not enough data */
#ifndef	_NOWARNMESS
    WARNMS(cinfo->emethods, "Premature end of GIF image");
#endif
    /* Pad data with 0's */
    return 0;			/* fake something usable */
  }

  /* Got normal raw byte or LZW symbol */
  incode = code;		/* save for a moment */
  
  if (code >= max_code) {	/* special case for not-yet-defined symbol */
    /* code == max_code is OK; anything bigger is bad data */
    if (code > max_code) 
	 {
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Corrupt data in GIF file");
#endif
      incode = 0;		/* prevent creation of loops in symbol table */
    }
    *sp++ = (UINT8) firstcode;	/* it will be defined as oldcode/firstcode */
    code = oldcode;
  }

  /* If it's a symbol, expand it into the stack */
  while (code >= clear_code) {
    *sp++ = symbol_tail[code];	/* tail of symbol: a simple byte value */
    code = symbol_head[code];	/* head of symbol: another LZW symbol */
  }
  /* At this point code just represents a raw byte */
  firstcode = code;		/* save for possible future use */

  /* If there's room in table, */
  if ((code = max_code) < LZW_TABLE_SIZE) {
    /* Define a new symbol = prev sym + head of this sym's expansion */
    symbol_head[code] = oldcode;
    symbol_tail[code] = (UINT8) firstcode;
    max_code++;
    /* Is it time to increase code_size? */
    if ((max_code >= limit_code) && (code_size < MAX_LZW_BITS)) {
      code_size++;
      limit_code <<= 1;		/* keep equal to 2^code_size */
    }
  }
  
  oldcode = incode;		/* save last input symbol for future use */
  return firstcode;		/* return first byte of symbol's expansion */
}


/*
 * Read one row of pixels.
 * This version is used for interlaced GIF images:
 * we read from the big in-memory image.
 */

//METHODDEF void
//get_interlaced_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
static long cur_row_number;	/* need to know actual row number */
static long pass2_offset;	/* # of pixel rows in pass 1 */
static long pass3_offset;	/* # of pixel rows in passes 1&2 */
static long pass4_offset;	/* # of pixel rows in passes 1,2,3 */
void init_interlace_row( PMIO cinfo )
{
   
	/* Initialize for get_interlaced_row, and perform first call on it. */
	cur_row_number = 0;
	pass2_offset = (cinfo->image_height + 7L) / 8L;
	pass3_offset = pass2_offset + (cinfo->image_height + 3L) / 8L;
	pass4_offset = pass3_offset + (cinfo->image_height + 1L) / 4L;

}

// this returns the row number next to access out of the
// array built on the first pass through the data
// 
long get_interlace_row (void)
{
	long irow;
   int   pass;
	/* Figure out which row of interlaced image is needed,
	   and access it. */
	//switch( (int)( cur_row_number & 7L ) )
	pass = ( cur_row_number & 7L );
	switch(pass)
	{
	case 0:			/* first-pass row */
		irow = cur_row_number >> 3;
		break;

	case 4:			/* second-pass row */
		irow = (cur_row_number >> 3) + pass2_offset;
		break;

	case 2:			/* third-pass row */
	case 6:
		irow = (cur_row_number >> 2) + pass3_offset;
		break;

	default:			/* fourth-pass row */
		irow = (cur_row_number >> 1) + pass4_offset;
		break;

	}

	cur_row_number++;		/* for next time */

   return irow;
}


// eof - LZW decode

// full extension decode

//int	DoExtensionExt( compress_info_ptr cinfo, LPGIFIMGEXT lpGIE,
//				   LPGIFHDREXT lpGHE, LPRGBQUAD lprgb )
int	DoExtensionExt2( PMIO cinfo, LPGIFIMGEXT lpGIE,
				   LPGIFHDREXT lpGHE, LPRGBQUAD lprgb )
{
	int extlabel;
	static BYTE buf[264];
	int count, cnt2;
	int	i, j;
	PBYTE	lps, lpb;
	LPTSTR lpd = gszDiag;

	/* Read extension label byte */
	extlabel = ReadByte2( cinfo );
   sprintf(lpd,"Ex=%02X ", extlabel);

	lpGIE->gceLabel = (BYTE)extlabel;	// Graphic Control Extension = 0xf9
	//lpGIE->gceSize  = 0;	// Block Size (4 for TEXT, Bit for TEXT)
	lpb = &code_buf[0];		// Use STATIC buffer for FIRST
	/* Process the FIRST data block associated with the extension */
	count = ReadByte2( cinfo ); 
	if(count > 0)
	{
		lpGIE->gceSize += (count+1);	// Accumlate DATA SIZE
		//if( !ReadOK(cinfo->input_file, lpb, count) )
		if( !ReadOK2(cinfo, lpb, count) )
		{
         sprintf(lpd,"Fails on first count of %d"MEOR, count);
         prt(lpd);
         return -1;
//			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		}
		if( ( count    == 4 ) &&
			( extlabel == GIF_CtrlExt ) )
		{
			// NOTE: Offsets a MINUS 1, since we have read the count
			lpGIE->gceFlag |=	gie_GCE;	// Graphic Control Extension
			lpGIE->gceBits = lpb[0];	// packed field
//	DWORD	gceColr;	// COLORREF (if SET)
			lpGIE->gceDelay = LM_to_uint(lpb[1],lpb[2]);	// 1/100 secs to wait
			lpGIE->gceIndex = lpb[3];	// Transparency Index (if Bit set)
			lpGIE->gceColr = 0;	// Just ZERO will do.
         sprintf(EndBuf(lpd), "GCE b=%02X d=%d i=%d ",
            lpGIE->gceBits,
            lpGIE->gceDelay,
            lpGIE->gceIndex );
			if( ( lpGIE->gceBits & gce_TransColr ) &&
				lprgb )
			{
            BYTE  r,g,b;
				int	idx;
				idx = lpGIE->gceIndex & 0xff;

				r = lprgb[idx].rgbRed;
            g = lprgb[idx].rgbGreen;
            b = lprgb[idx].rgbBlue;
				//lpGIE->gceColr = RGB( lprgb[idx].rgbRed,
				//						lprgb[idx].rgbGreen,
				//						lprgb[idx].rgbBlue );
            lpGIE->gceColr = RGB(r,g,b);  // set colour
            sprintf(EndBuf(lpd),"TrClr=%d(%03u,%03u,%03u) ",
               idx,
               r,g,b );

				//if( lpGIE->gceColr ){
				//	lpGIE->gceColr = RGB( lprgb[idx].rgbRed,
				//						lprgb[idx].rgbGreen,
				//						lprgb[idx].rgbBlue );}
			}
		}
		else if( ( count == 11 ) &&
			( extlabel == GIF_AppExt ) )
		{
			lpGIE->gceFlag |= gie_APP;	// Application Extension
			if( IsNetscape( lpb, 0 ) )
			{
				lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[13],lpb[14]);	// Loop count
				lpGIE->gceFlag |= gie_Netscape;	// FLAG Netscape
            sprintf(EndBuf(lpd), "Got Netscape - loop=%d ",
               lpGIE->gceRes1 );
			}
         else
         {
            strcat(lpd,"Unknown APP extension! *TBD* ");
         }
		}
		else if( ( count == 12 ) &&
			( extlabel == GIF_PTxtExt ) )
		{
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
//     +---------------+
			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
			lpGIE->gceRes1 = (DWORD)(LM_to_uint(lpb[8],lpb[9]) & 0xffff); // Char CELL
			lpGIE->gceRes2 = (DWORD)(lpb[10] & 0xff);	// Foreground index
			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
			lpGIE->gceSize += sizeof( PTEHDR );	// Room for 2 x DWORD header
         sprintf(EndBuf(lpd), "PTE x=%d y=%d, cx=%d, cy=%d i=%d ",
   			lpGIE->giLeft, // = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
	   		lpGIE->giTop,  //  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
		   	lpGIE->giGI.giWidth, // = LM_to_uint(lpb[4],lpb[5]);
			   lpGIE->giGI.giHeight,// = LM_to_uint(lpb[6],lpb[7]);
			   //lpGIE->gceRes1 = (DWORD)(LM_to_uint(lpb[8],lpb[9]) & 0xffff); // Char CELL
			   lpGIE->gceRes2 ); // = (DWORD)(lpb[10] & 0xff);	// Foreground index
			   //lpGIE->gceIndex = lpb[11];	// Backgound INDEX
			   //lpGIE->gceSize += sizeof( PTEHDR );	// Room for 2 x DWORD header
		}
		else if( extlabel == GIF_CommExt )
		{
         cnt2 = count;
			lpGIE->gceFlag |=	gie_COM;	// Had Comment
			for( i = 0; i < count; i++ )
			{
            j = (lpb[i] & 0xff);
            if(j & 0x80)
               j = '.';
            else if( j < ' ' )
               j = '.';

				buf[i] = (char)j;

			}
         buf[i] = 0;

			if( cnt2 > 8 )
				cnt2 = 8;
			lps = (LPSTR) &lpGIE->gceRes1;
			for( i = 0; i < cnt2; i++ )
			{
				lps[i] = lpb[i];
			}
         sprintf(EndBuf(lpd),"Comment=%s ", buf );

		}
		else
		{
         cnt2 = count;
			lpGIE->gceFlag |= gie_UNK;	// Undefined Extension
			if( cnt2 > 8 )
				cnt2 = 8;
			lps = (LPSTR) &lpGIE->gceRes1;
			for( i = 0; i < cnt2; i++ )
			{
				lps[i] = lpb[i];
			}
			for( i = 0; i < count; i++ )
			{
            j = (lpb[i] & 0xff);
            if(j & 0x80)
               j = '.';
            else if( j < ' ' )
               j = '.';
				buf[i] = (char)j; // lpb[i];
			}
         buf[i] = 0;
         sprintf(lpd,"UNKNOWN/UNLISTED with data [%s]", buf);

		}
	}

	// Now process any subsequent BLOCKS, if any
	if( count )	// IFF there WAS a FIRST BLOCK
	{	// Probably an ERROR if NO FIRST BLOCK!!!
		// DISCARD into BUFFER (on stack)
		while( count = ReadByte2( cinfo ) )
		{
			lpGIE->gceSize += ( count + 1 );	// Keep accumulating SIZE if say TEXT
			//if( !ReadOK(cinfo->input_file, buf, count) )
			if( !ReadOK2(cinfo, buf, count) )
			{
            sprintf(lpd,"Fails on subsequent count of %d"MEOR, count);
            prt(lpd);
            return -1;
			//	ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			}
		}
	}

	lpGIE->gceSize++;	// Bump 1 for NUL termination

	
   switch( extlabel )
			{
			case GIF_AppExt:	//              0xFF
				lpGHE->gheFlag |= ghf_AppExt;	// Had App Extension
				break;
			case GIF_CommExt:	//             0xFE
				lpGHE->gheFlag |= ghf_CommExt;	// Had Comment Extension
				break;
			case GIF_CtrlExt:	//             0xF9
				lpGHE->gheFlag |= ghf_CtrlExt;	// Had Graphic Control Extension
				break;
			case GIF_PTxtExt:	//             0x01
				lpGHE->gheFlag |= ghf_PTxtExt;	// Had plain text extension
				break;
			default:
				lpGHE->gheFlag |= ghf_UnknExt;	// Had an UNKNONW extension
				break;
			}

   strcat(lpd,MEOR);
   prt(lpd);

	return	extlabel;
}

//GIFIMGEXT _s_sGIE;
//GIFHDREXT _s_sGHE;
//LPGIFIMGEXT g_psGIE = &_s_sGIE;
//LPGIFHDREXT g_psGHE = &_s_sGHE;
//  LPRGBQUAD lprgb ) = #define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]
VOID  ShowExtension( PMIO cinfo )
{
   int c = DoExtensionExt2( cinfo,
      &_s_sGIE,   // LPGIFIMGEXT lpGIE,
      &_s_sGHE,   // LPGIFHDREXT lpGHE,
      gsBmpColour );   // LPRGBQUAD lprgb )

}
VOID  ClearExtensions(VOID)
{
   //LPGIFIMGEXT
   ZeroMemory(g_psGIE, sizeof(GIFIMGEXT));   // = &_s_sGIE;
   //LPGIFHDREXT
   ZeroMemory(g_psGHE, sizeof(GIFHDREXT));   // = _s_sGHE;
//  LPRGBQUAD lprgb ) = #define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]

}

// eof - full extension decode
// references

/*
 * wrdgif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to read input images in GIF format.
 *
 */

/*
 * This code is loosely based on giftoppm from the PBMPLUS distribution
 * of Feb. 1991.  That file contains the following copyright notice:
 * +-------------------------------------------------------------------+
 * | Copyright 1990, David Koblas.                                     |
 * |   Permission to use, copy, modify, and distribute this software   |
 * |   and its documentation for any purpose and without fee is hereby |
 * |   granted, provided that the above copyright notice appear in all |
 * |   copies and that both that copyright notice and this permission  |
 * |   notice appear in supporting documentation.  This software is    |
 * |   provided "as is" without express or implied warranty.           |
 * +-------------------------------------------------------------------+
 *
 * We are also required to state that
 *    "The Graphics Interchange Format(c) is the Copyright property of
 *    CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *    CompuServe Incorporated."
 */

// eof - DumpGif.c
