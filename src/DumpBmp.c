
// DumpBmp.c
// Part of Dump4.c
// A 32-Bit CONSOLE Application
#ifndef _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES
#endif 
#include	"Dump4.h"
#include <math.h>
// FIX20070228 - add -bmp2 to DUMP the bitmap (used) COLORS
// FIX20150421 - add BITMAPV5HEADER
/* ============================================
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct {
  // *** SAME AS THE ABOVE ***
  DWORD        bV5Size;
  LONG         bV5Width;
  LONG         bV5Height;
  WORD         bV5Planes;
  WORD         bV5BitCount;
  DWORD        bV5Compression;
  DWORD        bV5SizeImage;
  LONG         bV5XPelsPerMeter;
  LONG         bV5YPelsPerMeter;
  DWORD        bV5ClrUsed;
  DWORD        bV5ClrImportant;
  // ============================
  DWORD        bV5RedMask;
  DWORD        bV5GreenMask;
  DWORD        bV5BlueMask;
  DWORD        bV5AlphaMask;
  DWORD        bV5CSType;
  CIEXYZTRIPLE bV5Endpoints;
  DWORD        bV5GammaRed;
  DWORD        bV5GammaGreen;
  DWORD        bV5GammaBlue;
  DWORD        bV5Intent;
  DWORD        bV5ProfileData;
  DWORD        bV5ProfileSize;
  DWORD        bV5Reserved;
} BITMAPV5HEADER, *PBITMAPV5HEADER;
   ======================================= */

extern	void	DoOutput( DWORD fOff );

#define		MINSIZE		(sizeof(BTIMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))

int do_vertical_strip = 0; // special DEBUG - get colours in VERTICAL strips
int add_row_col = 0;
int print_decimal = 0; // printf color as (  0,  0,  0) (255,255,255) (... TODO: Not yet right
DWORD def_wrap_size = 16;     // set to WRAP at this SIZE (if not other preferred)
DWORD def_wrap_size24 = 18;   // wrap at a convenient value

// Value  Description
typedef struct tagNSS {
	UINT	num;
	LPSTR	sznm;
	LPSTR	szdesc;
}NSS;

DWORD  SortRGBQ( LPTSTR lps,  // given a 256 RGBQUAD array, return 'different' colour cnt
                BOOL upd,   // and update the array if TRUE
                DWORD count );  // count COPIED to table

//From July 2000 MSDN
//Value Description 
//BI_RGB An uncompressed format. 
//BI_RLE8 A run-length encoded (RLE) format for bitmaps with 8
//bpp. The compression format is a 2-byte format consisting of a
//count byte followed by a byte containing a color index. For more
//information, see Bitmap Compression.  
//BI_RLE4 An RLE format for bitmaps with 4 bpp. The compression
//format is a 2-byte format consisting of a count byte followed by
//two word-length color indexes. For more information, see Bitmap
//Compression. 
//BI_BITFIELDS Specifies that the bitmap is not compressed and
//that the color table consists of three DWORD color masks that
//specify the red, green, and blue components, respectively, of
//each pixel. This is valid when used with 16- and 32-bpp bitmaps.
//BI_JPEG Windows 98, Windows 2000: Indicates that the image is a
//JPEG image. 
//BI_PNG Windows 98, Windows 2000: Indicates that the image is a
//PNG image. 

NSS		sComps[] = {
	{ BI_RGB,  "BI_RGB",
      "An uncompressed format." },
	{ BI_RLE8, "BI_RLE8",
      "A run-length encoded (RLE) format for bitmaps with 8 bits per pixel. "
      "The compression format is a two-byte format consisting of a count byte "
      "followed by a byte containing a color index." },
	{ BI_RLE4, "BI_RLE4",
      "An RLE format for bitmaps with 4 bits per pixel. The compression format "
      "is a two-byte format consisting of a count byte followed by two word-length "
      "color indices." },
	{ BI_BITFIELDS, "BI_BITFIELDS",
      "Specifies that the bitmap is not compressed and "
      "that the color table consists of three doubleword color "
      "masks that specify the red, green, and blue components, respectively, "
      "of each pixel. This is valid when used with 16- and 32-bits-per-pixel bitmaps." },
      // added Sep 2000 from July 2000 MSDN help
   { BI_JPEG, "BI_JPEG",
   "Windows 98, Windows 2000: Indicates that the image is a JPEG image." },
   { BI_PNG,  "BI_PNG",
   "Windows 98, Windows 2000: Indicates that the image is a PNG image." },

   // more to add ???
	{ (UINT)-1, "UNKNOWN", "???" }
};
 

//typedef struct tagNSS {
//	UINT	num;
//	LPSTR	sznm;
//	LPSTR	szdesc;
//}NSS;
//NSS		sComps[] = {
//		0 BI_RGB 


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GetCompStg
// Return type: LPSTR 
// Argument   : UINT ui
// Description: Convert the "compression" type to a string
//              from the above table.
///////////////////////////////////////////////////////////////////////////////
LPSTR	GetCompStg( UINT ui )
{
	NSS *	lpnss = &sComps[0];
	UINT	nu = 1;
	LPSTR	lps = "UNKNOWN";
	while( nu )
	{
		if( lpnss->num == ui )
		{
			lps = lpnss->sznm;
			break;
		}
		lpnss++;
		if( lpnss->num == (UINT)-1 )
			break;
	}

	return lps;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ValidBitCount
// Return type: BOOL 
// Argument   : int i
// Description: Verify the bit count value is one of
//              1, 4, 8, 16, 24, 32.
///////////////////////////////////////////////////////////////////////////////
BOOL	ValidBitCount( int i )
{
	BOOL	flg = FALSE;
	switch(i)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
		flg = TRUE;
		break;
	}
	return flg;
}

// #define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ValidPerRow
// Return type: int 
// Argument   : BITMAPINFOHEADER * lpbih
// Description: Per the bit count in the BITMAPINFOHEADER
//              return the number of BYTES in a ROW
///////////////////////////////////////////////////////////////////////////////
int	ValidPerRow( BITMAPINFOHEADER * lpbih )
{
	// lpbih->biWidth and
	// lpbih->biBitCount
	int		i = lpbih->biBitCount;
	int		j, k, l;
	k = lpbih->biWidth;
	switch(i)
	{
	case 1:
		j = k / 8;	/* fit 8 bits per byte */
		if( k % 8 )	/* fit 8 bits per byte */
			j++;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 4:
		j = k / 2;	/* fit 2 nibbles per byte */
		if( k % 2 )
			j++;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 8:
		j = k;		/* use byte index to 256 color table */
		l = WIDTHBYTES( (j * 8) );
		break;

	case 16:
		j = k * 2;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 24:
		j = k * 3;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 32:
		j = k * 4;
		l = WIDTHBYTES( (j * 8) );
		break;

	}

	return j;
}

int	BytesPerRow2( BITMAPINFOHEADER * lpbih )
{
	return( WIDTHBYTES( (ValidPerRow(lpbih) * 8) ) );
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : BytesPerRow
// Return type: int 
// Argument   : BITMAPINFOHEADER * lpbih
// Description: Per the bit count in the BITMAPINFOHEADER
//              return the number of BYTES in a ROW
// INCLUDING padding to 32-bit boundary.
///////////////////////////////////////////////////////////////////////////////
int	BytesPerRow( BITMAPINFOHEADER * lpbih )
{
	// lpbih->biWidth and
	// lpbih->biBitCount
	int		i = lpbih->biBitCount;
	int		j, k, l;
	k = lpbih->biWidth;
	switch(i)
	{
	case 1:
		j = k / 8;	/* fit 8 bits per byte */
		if( k % 8 )	/* fit 8 bits per byte */
			j++;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 4:
		j = k / 2;	/* fit 2 nibbles per byte */
		if( k % 2 )
			j++;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 8:
		j = k;		/* use byte index to 256 color table */
		l = WIDTHBYTES( (j * 8) );
		break;

	case 16:
		j = k * 2;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 24:
		j = k * 3;
		l = WIDTHBYTES( (j * 8) );
		break;
	case 32:
		j = k * 4;
		l = WIDTHBYTES( (j * 8) );
		break;

	}

	return l;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GetColorCount
// Return type: int 
// Argument   : int bits
// Description: Convert the "bit" count to the number of colors
//              NOTE: 16,24,32 of course return ZERO
///////////////////////////////////////////////////////////////////////////////
int	GetColorCount( int bits )
{
	int		cc = 0;
	switch(bits)
	{
	case 1:
		cc = 2;
		break;
	case 4:
		cc = 16;
		break;
	case 8:
		cc = 256;
		break;
	case 16:
	case 24:
	case 32:
		break;
	}
	return cc;

}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GetColorStg
// Return type: LPSTR 
// Argument   : int bits
// Description: 
// Return type of bitmap - per colour count
// 13 September 2000
//              
///////////////////////////////////////////////////////////////////////////////
LPSTR	GetColorStg( int bits )
{
	LPSTR	lpd = 0;
	switch(bits)
	{
      // MSDN July 2000 Help suggests
   case 0:
      // Windows 98, Windows 2000:
      // The number of bits-per-pixel is specified or is implied by the JPEG or PNG format. 
      lpd = " ie Bits per pixel specified or implied by JPEG or PNG compression";
      break;
	case 1:
// The bitmap is monochrome,
// and the bmiColors member of BITMAPINFO contains two entries.
// Each bit in the bitmap array represents a pixel. If the bit is clear,
// the pixel is displayed with the color of the first entry in the bmiColors
// table; if the bit is set, the pixel has the color of the
// second entry in the table.
		lpd = " ie Monochrome Max. 2-Colours";
		break;
	case 4:
//The bitmap has a maximum of 16 colors, and the bmiColors member
//of BITMAPINFO contains up to 16 entries. Each pixel in the
//bitmap is represented by a 4-bit index into the color table. For
//example, if the first byte in the bitmap is 0x1F, the byte
//represents two pixels. The first pixel contains the color in the
//second table entry, and the second pixel contains the color in
//the sixteenth table entry.
		lpd = " ie Max. 16-Colours (4-bit Index)";
		break;
	case 8:
//The bitmap has a maximum of 256 colors, and the bmiColors member
//of BITMAPINFO contains up to 256 entries. In this case, each
//byte in the array represents a single pixel.
		lpd = " ie Max. 256-Colours (8-bit Index)";
		break;
	case 16:
//The bitmap has a maximum of 2^16 colors. If the biCompression
//member of the BITMAPINFOHEADER is BI_RGB, the bmiColors member
//of BITMAPINFO is NULL. Each WORD in the bitmap array represents
//a single pixel. The relative intensities of red, green, and blue
//are represented with five bits for each color component. The
//value for blue is in the least significant five bits, followed
//by five bits each for green and red. The most significant bit is
//not used. The bmiColors color table is used for optimizing
//colors used on palette-based devices, and must contain the
//number of entries specified by the biClrUsed member of the
//BITMAPINFOHEADER. 
//If the biCompression member of the BITMAPINFOHEADER is
//BI_BITFIELDS, the bmiColors member contains three DWORD color
//masks that specify the red, green, and blue components,
//respectively, of each pixel. Each WORD in the bitmap array
//represents a single pixel.
//Windows NT/Windows 2000: When the biCompression member is
//BI_BITFIELDS, bits set in each DWORD mask must be contiguous and
//should not overlap the bits of another mask. All the bits in the
//pixel do not have to be used. 
//Windows 95/98: When the biCompression member is BI_BITFIELDS,
//the system supports only the following 16bpp color masks: A
//5-5-5 16-bit image, where the blue mask is 0x001F, the green
//mask is 0x03E0, and the red mask is 0x7C00; and a 5-6-5 16-bit
//image, where the blue mask is 0x001F, the green mask is 0x07E0,
//and the red mask is 0xF800.
		lpd = " ie Max. 65,536 Colours (Word per color)";
		break;
	case 24:
//The bitmap has a maximum of 2^24 colors, and the bmiColors
//member of BITMAPINFO is NULL. Each 3-byte triplet in the bitmap
//array represents the relative intensities of blue, green, and
//red, respectively, for a pixel. The bmiColors color table is
//used for optimizing colors used on palette-based devices, and
//must contain the number of entries specified by the biClrUsed
//member of the BITMAPINFOHEADER.
		lpd = " ie Max. 16.7 Million Colours (24-bits)";
		break;
	case 32:
//The bitmap has a maximum of 2^32 colors. If the biCompression
//member of the BITMAPINFOHEADER is BI_RGB, the bmiColors member
//of BITMAPINFO is NULL. Each DWORD in the bitmap array represents
//the relative intensities of blue, green, and red, respectively,
//for a pixel. The high byte in each DWORD is not used. The
//bmiColors color table is used for optimizing colors used on
//palette-based devices, and must contain the number of entries
//specified by the biClrUsed member of the BITMAPINFOHEADER. 
//If the biCompression member of the BITMAPINFOHEADER is
//BI_BITFIELDS, the bmiColors member contains three DWORD color
//masks that specify the red, green, and blue components,
//respectively, of each pixel. Each DWORD in the bitmap array
//represents a single pixel.
//Windows NT/ 2000: When the biCompression member is BI_BITFIELDS,
//bits set in each DWORD mask must be contiguous and should not
//overlap the bits of another mask. All the bits in the pixel do
//not need to be used.
//Windows 95/98: When the biCompression member is BI_BITFIELDS,
//the system supports only the following 32-bpp color mask: The
//blue mask is 0x000000FF, the green mask is 0x0000FF00, and the
//red mask is 0x00FF0000. 
		lpd = " ie Max. 4,295 Million Colours (32-bits)";
		break;
   default:
      lpd = " Un-recognised BitCount value!";
      break;
	}
	return lpd;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : chkit
// Return type: void 
// Argument   : void
// Description: JUST A DEBUG STOP
//              
///////////////////////////////////////////////////////////////////////////////
void	chkit( void )
{

}
#pragma pack(1)
// force byte alignment
typedef struct tagRGB24 {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
} RGB24, * PRGB24;
#pragma pack()

DWORD rgb24cnt = 0;
DWORD rgb24max = 256;
PRGB24 prgb24 = NULL;
RGB24 rgb24;

// frgb=511 frbg=0 fgrb=0 fgbr=0 fbrg=0 fbgr=2494 miss=0 ...
// WARNING: 2494 NOT FOUND ...
void  set_RGB_color( DWORD lc, int c )
{
   int icoff = lc % 3;
   switch(icoff)
   {
   case 0:
      //rgb24.rgbRed = (BYTE)c;
      rgb24.rgbBlue = (BYTE)c;
      break;
   case 1:
      rgb24.rgbGreen = (BYTE)c;
      break;
   case 2:
      //rgb24.rgbBlue = (BYTE)c;
      rgb24.rgbRed = (BYTE)c;
      break;
   default:
      sprtf( "ERRANT CODING!"MEOR );
      wait_keyin();
      break;
   }
}

void  Compare_RGB_Colors( PRGB24 tmp, UINT tmpcnt, PRGB24 p24, UINT rgb24cnt )
{
   UINT  i, j, missed;
   BYTE  r, g, b;
   UINT  frgb, frbg, fgrb, fgbr, fbrg, fbgr, miss;

   frgb = frbg = fgrb = fgbr = fbrg = fbgr = miss = 0;
   missed = 0;
   if( tmpcnt != rgb24cnt )
      sprtf( "WARNING: Different counts - %d vs %d ..."MEOR, tmpcnt, rgb24cnt );

   for( i = 0; i < tmpcnt; i++ ) {
      r = tmp[i].rgbRed;
      g = tmp[i].rgbGreen;
      b = tmp[i].rgbBlue;
      for( j = 0; j < rgb24cnt; j++ ) {
         if(( r == p24[j].rgbRed ) &&
            ( g == p24[j].rgbGreen ) &&
            ( b == p24[j].rgbBlue ) )
            break;
      }
      if( j < rgb24cnt ) {
         frgb++;
      } else {
         missed++;
         sprtf( "%d WARNING: rgb(%3d,%3d,%3d) NOT FOUND!"MEOR, missed, r, g, b );
         for( j = 0; j < rgb24cnt; j++ ) {
            if(( b == p24[j].rgbRed ) &&
               ( g == p24[j].rgbGreen ) &&
               ( r == p24[j].rgbBlue ) )
               break;
         }
         if( j < rgb24cnt ) {
            sprtf( "But bgr(%3d,%3d,%3d) was FOUND!"MEOR, r, g, b );
            fbgr++;
         } else {
            for( j = 0; j < rgb24cnt; j++ ) {
               if(( r == p24[j].rgbRed ) &&
                  ( b == p24[j].rgbGreen ) &&
                  ( g == p24[j].rgbBlue ) )
                  break;
            }
            if( j < rgb24cnt ) {
               sprtf( "But rbg(%3d,%3d,%3d) was FOUND!"MEOR, r, g, b );
               frbg++;
            } else {
               for( j = 0; j < rgb24cnt; j++ ) {
                  if(( g == p24[j].rgbRed ) &&
                     ( r == p24[j].rgbGreen ) &&
                     ( b == p24[j].rgbBlue ) )
                     break;
               }
               if( j < rgb24cnt ) {
                  sprtf( "But grb(%3d,%3d,%3d) was FOUND!"MEOR, r, g, b );
                  fgrb++;
               } else {
                  for( j = 0; j < rgb24cnt; j++ ) {
                     if(( g == p24[j].rgbRed ) &&
                        ( b == p24[j].rgbGreen ) &&
                        ( r == p24[j].rgbBlue ) )
                        break;
                  }
                  if( j < rgb24cnt ) {
                     sprtf( "But gbr(%3d,%3d,%3d) was FOUND!"MEOR, r, g, b );
                     fgbr++;
                  } else {
                     for( j = 0; j < rgb24cnt; j++ ) {
                        if(( b == p24[j].rgbRed ) &&
                           ( r == p24[j].rgbGreen ) &&
                           ( g == p24[j].rgbBlue ) )
                           break;
                     }
                     if( j < rgb24cnt ) {
                        sprtf( "But brg(%3d,%3d,%3d) was FOUND!"MEOR, r, g, b );
                        fbrg++;
                     } else {
                        miss++;
                     }
                  }
               }
            }
         }
      }
   }
   if(missed) {
      sprtf( "frgb=%d frbg=%d fgrb=%d fgbr=%d fbrg=%d fbgr=%d miss=%d ..."MEOR,
         frgb, frbg, fgrb, fgbr, fbrg, fbgr, miss );
      sprtf( "WARNING: %d NOT FOUND ..."MEOR, missed );
   } else {
      sprtf( "Compare of %d COLORS show 100%% EXACTLY the SAME ..."MEOR, tmpcnt );
   }
}


void  add_RGB_color( BYTE r, BYTE g, BYTE b )
{
   if( prgb24 ) {
      DWORD cnt;
      for( cnt = 0; cnt < rgb24cnt; cnt++ ) {
         if(( prgb24[cnt].rgbRed == r ) &&
            ( prgb24[cnt].rgbGreen == g ) &&
            ( prgb24[cnt].rgbBlue == b ) )
            break;
      }
      if( cnt == rgb24cnt ) {
         // NOT FOUND
         if( rgb24cnt >= rgb24max ) {
            // must reallocate
            PRGB24 tmp;
            rgb24max *= 2; // double the count
            tmp = dMALLOC( (sizeof(RGB24) * rgb24max) );
            CHKMEM(tmp);
            memcpy(tmp, prgb24, (sizeof(RGB24) * rgb24cnt));
            dMFREE(prgb24);
            prgb24 = tmp;
         }
         prgb24[cnt].rgbRed = r;
         prgb24[cnt].rgbGreen = g;
         prgb24[cnt].rgbBlue = b;
         rgb24cnt++;
      }
   }
}
void  add_COLOR_rbg24( void )
{
   add_RGB_color( rgb24.rgbRed, rgb24.rgbGreen, rgb24.rgbBlue );
}

void  Show_RGB_Colors( void )
{
   if( prgb24 ) {
      DWORD cnt;
      BYTE r, g, b;
      sprtf( "BYTE rbg_%d_colors24[] = {"MEOR, rgb24cnt );
      for( cnt = 0; cnt < rgb24cnt; cnt++ ) {
         r = prgb24[cnt].rgbRed;
         g = prgb24[cnt].rgbGreen;
         b = prgb24[cnt].rgbBlue;
         sprtf( "   %3d, %3d, %3d, // #%2.2X%2.2X%2.2X"MEOR,
            r, g, b, r, g, b );
      }
      sprtf( "};"MEOR );
   }
}

double sqr( int dif )
{
   double d = (double)dif;
   return ( d * d );
}

void  Show_RGB_Colors2( void )
{
   if( prgb24 ) {
      DWORD cnt;
      BYTE r, g, b;
      BYTE pr, pg, pb;
      double diff = 0.0;
      sprtf( "BYTE rbg_%d_colors24[] = {"MEOR, rgb24cnt );
      cnt = 0;
      pr = prgb24[cnt].rgbRed;
      pg = prgb24[cnt].rgbGreen;
      pb = prgb24[cnt].rgbBlue;
      for( cnt = 0; cnt < rgb24cnt; cnt++ ) {
         r = prgb24[cnt].rgbRed;
         g = prgb24[cnt].rgbGreen;
         b = prgb24[cnt].rgbBlue;
         diff = sqrt( sqr(r - pr) + sqr(g - pg) + sqr(b - pb) ); 
         sprtf( "   %3d, %3d, %3d, // %4d #%2.2X%2.2X%2.2X (d=%2.4f)"MEOR,
            r, g, b, cnt, r, g, b, diff );
         pr = r;
         pg = g;
         pb = b;
      }
      sprtf( "};"MEOR );
   }
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : FormRowData
// Return type: void 
// Arguments  : int c
//            : LPTSTR lpd
//            : DWORD iCol
//            : DWORD dwBytes
//            : LPDWORD pui
//            : LPDWORD piRow
//            : LPDWORD piC
//            : LPDWORD pia
// Description: Build a HEX DUMP of a ROW from the BITMAP
//              Added option to ONLY output the first line of each ROW
///////////////////////////////////////////////////////////////////////////////
//BOOL  g_bOneRow = TRUE;
#define  g_bOneRow   (giBmpVerb == 1) ? 1 : 0   // verbosity of BITMAP output
static INT _s_inewrow = 0;
DWORD g_dwRows, g_dwCols;
DWORD logcolm = 0;

// // g_BmpBgnEnd[4]
//typedef enum {
//   bgn_row,
//   bgn_col,
//   end_row,
//   end_col,
int   row_in_range( int irow )
{
   int   iret = 1;
   if( g_BmpBgnEnd[end_row] ) {
      if( irow < g_BmpBgnEnd[bgn_row] )
         iret = 0;
      if( irow > g_BmpBgnEnd[end_row] )
         iret = 0;
   }
   return iret;
}
int   col_in_range( int icol )
{
   int   iret = 1;
   if( g_BmpBgnEnd[end_col] ) {
      if( icol < g_BmpBgnEnd[bgn_col] )
         iret = 0;
      if( icol > g_BmpBgnEnd[end_col] )
         iret = 0;
   }
   return iret;
}

int   rc_in_range( int irow, int icol )
{
   int   iret = 1;
   if( !row_in_range( irow ) )
      iret = 0;
   if( !col_in_range( icol ) )
      iret = 0;
   return iret;
}


void  FormRowData( int c, LPTSTR lpd, DWORD iCol, DWORD dwBytes,
                  LPDWORD pui, LPDWORD piRow, LPDWORD piC,
                  LPDWORD pia, int irow, int icol )
{
   DWORD ui   = *pui;
   DWORD iRow = *piRow;
   DWORD iC   = *piC;
//   DWORD dwuk   = *puk;
   DWORD ia   = *pia;
   DWORD dww  = def_wrap_size;     // set to WRAP at this SIZE (if not other preferred)
   PBITMAPINFOHEADER pbi = (PBITMAPINFOHEADER)&g_bih;
   if( pbi->biBitCount == 24 ) { // Put them in THREE
      dww = def_wrap_size24;   // wrap at a convenient value
   }
//IMAGE DATA - Count of BYTES = 24 x 21 =  (504 Bytes) 
//Row  21: 07 07 07 00 00 07 07 00 00 00 00 00 00 00 00 00 
//         00 00 00 07 07 
//		if( ui == 0 )
//		{
//         _s_inewrow = giBmpVerb;
//         // start with ROW header ...
//			sprintf(lpd,
//				"Row %4d: ",
//				iRow );
//			iC = iCol;	// *RESTART* valid BYTE counter (from ValidPerRow(lpbih);)
//		}

		if(iC)
		{
         gdwColCount[(c & 0xff)]++; // NOT true for 24BBP bitmaps
         // valid byte
         iC--; // reduce VALID counter

        if( pbi->biBitCount == 24 )  // Put them in THREE
         {
  			   sprintf( EndBuf(lpd),
   			   "%02X",
	   		   ( c & 0xff ) );
            set_RGB_color( logcolm, c );
            if( iC % 3 )
            {
               // nothing
            }
            else
            {
               if( ui )
                  strcat(lpd," ");  // add a SPACE
            }
         }
         else
         {
             if (print_decimal) {
                 if (ui % 3) {
                     if ((ui+1) % 3) {
        			    sprintf( EndBuf(lpd),
	        			   "%03u,",
		        		   ( c & 0xff ) );
                     } else {
        			    sprintf( EndBuf(lpd),
	        			   "%03u) ",
		        		   ( c & 0xff ) );
                     }
                 } else {
    			    sprintf( EndBuf(lpd),
	    			   "(%03u,",
		    		   ( c & 0xff ) );
                 }

             } else {
			    sprintf( EndBuf(lpd),
				   "%02X ",
				   ( c & 0xff ) );
             }
         }
		}
		else
		{
         // ignored in BITMAP reading
			strcat(lpd, "? " );  // just PAD data - can be anything
		}

		ui++;		// shown a byte
		ia++;

		if( ui >= dwBytes )
		{
         // add terminator
         // END OF ROW OF Coloured BYTES
         if( add_row_col ) {
            sprintf(EndBuf(lpd), " row=%d col=%d", irow, icol );
            if( row_in_range( irow ) )
               strcat(lpd, " ok");
            else
               strcat(lpd, " OOR");
         }
			strcat(lpd,MEOR);
         if( g_bOneRow )
         {
            if( giBmpVerb == 1 )
            {
               // only output first and last Rows
               if( _s_inewrow )
               {
                  if( (iRow == g_dwRows) || (iRow == 1) )
                     prt(lpd);
                  _s_inewrow = 0;
               }
            }
            else
            {
               if( _s_inewrow )
               {
                  prt(lpd);
                  _s_inewrow = 0;
               }
            }
         }
         else
            prt(lpd);

			*lpd = 0;
			ui = 0;
			ia = 0;

			if( iRow )
				iRow--;
			else
				chkit();
		}

//		if( ia >= 16 )
		if( ia >= dww )   // if we have reached just a data WRAP point, then
		{
         // done this line - out it
         // =======================
			strcat(lpd,MEOR);
         if( g_bOneRow )
         {
            if( giBmpVerb == 1 )
            {
               // only output first and last Rows
               if( _s_inewrow )
               {
                  if( (iRow == g_dwRows) || (iRow == 1) )
                     prt(lpd);
                  _s_inewrow--;
               }
            }
            else
            {
               if( _s_inewrow )
               {
                  prt(lpd);
                  _s_inewrow--;
               }
            }
         }
         else
   			prt(lpd);

			//           "Row 1234: "
			strcpy(lpd, "    \"     " );
			ia = 0;
		}

   *pui   = ui;
   *piRow = iRow;
   *piC   = iC;
   *pia   = ia;   // return COUNT of char in OUT buffer

}
// NOTE: In windows, DWORD, COLORREF and RGBQUAD are interchangeable
// All can be split into red,green,blue BYTE values
VOID  ShowColourCnt( BITMAPINFOHEADER * lpbih, LPTSTR lpd )
{
   DWORD ui, ia, dwc;
   DWORD dwmx = 0;
   DWORD dwmxi = 0;
   int      ip;
   double   percent;
   RGBQUAD   cr;
   BYTE     r,g,b;

   ia = 0;
   for( ui = 0; ui < 256; ui++ )
   {
      dwc = gdwColCount[ui];
//      cr  = gsBmpColour[ui];
      if( dwc == 0 )
      {
         ia++;
      }
      else if( dwc > dwmx )
      {
         dwmx = dwc;
         dwmxi = ui; // keep the INDEX
      }
   }

   cr  = gsBmpColour[dwmxi];
   r   = cr.rgbRed;
   g   = cr.rgbGreen;
   b   = cr.rgbBlue;

   percent = ((double)dwmx * 100.0) / (double)(lpbih->biWidth * lpbih->biHeight);
   ip = (int)(percent * 10.0);
   percent = (double)ip / 10.0;

   sprintf(lpd,
      "Of 256 colour indexes, %d not used,"MEOR
      "and max freq = %d. (%0.1f%%) of colour %02X(%d,%d,%d)."MEOR,
      ia,
      dwmx,
      percent,
      dwmxi,
      (r & 0xff),
      (g & 0xff),
      (b & 0xff) );

   prt(lpd);

}

//BITMAPFILEHEADER g_bmfh;
//BITMAPINFOHEADER g_bih;
//RGBQUAD          g_ct[256];

//         Bump_Column( g_bih.biBitCount, logcolm, &icol );
void Bump_Column( int BitCount, int logcolm, int * picol )
{
   int icol = *picol;
   //switch (g_bih.biBitCount)
   switch (BitCount)
   {
   case 1:
      //wi= (wi+31) >> 3;
      // each BYTE is 8 columns
      icol += 8;
      break;
   case 4:
      //wi= (wi+7)  >> 1;
      // each BYTE is 2 columns
      icol += 2;
      break;
	case 8:
      //wi=  wi+3;
      // each BYTE is a COLUMN
      icol++;
      break;
   case 16:
      //wi= (wi*2)+3;
      // each BYTE is half a column
      if( logcolm && ((logcolm % 2) == 0) )
         icol++;
      break;
   case 24:
      //wi= (wi*3)+3;
      // each 3 BYTES is a column
      if( logcolm && ((logcolm % 3) == 0) )
         icol++;
      break;
   case 32:
      //return wi*4;
      // each 4 BYTES is a column
      if( logcolm && ((logcolm % 4) == 0) )
         icol++;
      break;
   }
   *picol = icol;
}

LE sLineList = { &sLineList, &sLineList };
typedef struct tagLINELIST {
   LE link;
   CHAR line[1];
}LINELIST, * PLINELIST;

PLE GetLineList(void)
{
   return &sLineList;
}

void Add2LinesList( PSTR ps )
{
   PLE ph = GetLineList();
   PLE pn;
   size_t len = strlen(ps);
   if(len) {
      pn = dMALLOC(sizeof(LINELIST) + len);
      if(pn) {
         PLINELIST pll = (PLINELIST)pn;
         strcpy(pll->line,ps);
         InsertTailList(ph,pn);
      }
   }
}

void KillLineList( void )
{
   PLE ph = GetLineList();
   KillLList(ph);
}

void ShowLineList( void )
{
   PLE ph = GetLineList();
   PLE pn;
   Traverse_List(ph, pn)
   {
      PLINELIST pll = (PLINELIST)pn;
      prt( pll->line );
   }
}

void prts( PSTR ps )
{
   size_t len = strlen(ps);
   if(len) {
      prt( ps );
      Add2LinesList( ps );
   }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ProcessBMP
// Return type: void 
// Arguments  : HFILE hf
//            : LPSTR fn
//            : LPSTR lpb
//            : DWORD len
//            : DWORD fsiz
// Description: Check and process a BITMAP file
// Return value, if successful             
///////////////////////////////////////////////////////////////////////////////
int	ProcessBMP( HFILE hf, LPSTR fn, LPSTR lpb, DWORD len,
				  DWORD fsiz )
{
	char		c;
	UINT		ui, cc1, cc2;
	DWORD		rd = len;
//	LPSTR		lph;
//	LPSTR		lptmp;
	DWORD		foff, ir;
	DWORD		iRow, ia, iCol, iC;  // prefer UNSIGNED 32 bits for counters, etc
	BITMAPFILEHEADER *	lpbmfh;
	BITMAPINFOHEADER *	lpbih;
	LPSTR				lps;
	LPSTR				lpd = &gszDiag[0];
	RGBQUAD *	lpq;
   BYTE     r,g,b;
   DWORD    imgsiz, imgrow, cfsize, infhdr;
   UINT     ucnt = 0;
   int   icol, irow;
   PBITMAPINFOHEADER pbi = (PBITMAPINFOHEADER)&g_bih;   // after the COPY 
//	lpb = &gcFilBuf[0];
//	foff = 0;
//	lph = &gszHex[0];
//	*lph = 0;
//	fix = 0;
//	ia = 0;
   KillLineList();

   // set some headers
	lpbmfh = (BITMAPFILEHEADER *)lpb;
	lps    = (lpb + sizeof(BITMAPFILEHEADER));
	lpbih  = (BITMAPINFOHEADER *)lps;
   // 40	BITMAPINFOHEADER	Windows NT, 3.1x or later[2]
   // 124	BITMAPV5HEADER	Windows NT 5.0, 98 or later
    infhdr = lpbih->biSize;  
    //lpq    = (LPRGBQUAD) ( lpbih + 1 ); // color table, if ANY, AFTER INFO HEADER
    lpq    = (LPRGBQUAD) (lps + infhdr ); // color table, if ANY, AFTER INFO HEADER

    cc1 = lpbih->biClrUsed;  // get number of colours used
    if(!cc1)  // if NONE specified in header
       cc1 = GetColorCount(lpbih->biBitCount); // based on BITS-PER_PIXEL, 8-bits=256 colours
    cc2 = cc1;
    if(!cc1) {
      if( lpbih->biCompression == BI_BITFIELDS )
         cc1 = 3;
   }
   // 
   imgsiz = lpbih->biSizeImage;  // extract IMAGE SIZE
   imgrow = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT

   cfsize = ( sizeof(BITMAPFILEHEADER) +
      lpbih->biSize +   // plus SIZE OF BITMAPINFOHEADER|BITMAPV5HEADER
      (cc2 * sizeof(RGBQUAD)) +   // plus COLOR BLOCK, IF ANY
      imgsiz );   // plus the IMAGE SIZE

//	sprintf( lpd,
//		"File is %s"MEOR,
//		fn );
//	prt(lpd);
	sprintf(lpd,
		"Length  =  %d Bytes (File Size)"MEOR,
		fsiz ) ;
	prts(lpd);

    // get to the BITS
	lps = (lpb +
		sizeof(BITMAPFILEHEADER) +
		lpbih->biSize +            // sizeof(BITMAPINFOHEADER) OR OTHER SIZES
		( cc2 * sizeof(RGBQUAD) ) );

	if(( lpbmfh->bfType == BMPSIG                       ) &&
		( (lpbih->biSize  == sizeof(BITMAPINFOHEADER)) || (lpbih->biSize  == sizeof(BITMAPV5HEADER)) ) &&
		( ValidBitCount(lpbih->biBitCount)               ) &&
		( lpbmfh->bfOffBits < fsiz                       ) &&
		(( lpbmfh->bfOffBits == ((DWORD)lps - (DWORD)lpb) )||( lpbih->biCompression == BI_BITFIELDS )) &&
        ( fsiz >= cfsize                                 ) )
	{
		// appears ok
	}
	else
	{
		prt( "ERROR: Not a known BITMAP file."MEOR );
      if( lpbmfh->bfType != BMPSIG ) {
   		prt( " BMP ERROR: No 'BM' signature!"MEOR );
      } else if( !( ( lpbih->biSize == sizeof(BITMAPINFOHEADER) ) ||
                    (lpbih->biSize  == sizeof(BITMAPV5HEADER) ) ) ) {
          sprintf(lpd," BMP ERROR: Information header size incorrect! Got %u, expected %u or %u"MEOR,
              (unsigned int)lpbih->biSize, (unsigned int)sizeof(BITMAPINFOHEADER), (unsigned int)sizeof(BITMAPV5HEADER));
   		prt(lpd);
      } else if( !ValidBitCount(lpbih->biBitCount) ) {
   		prt( " BMP ERROR: Bit count NOT valid!"MEOR );
      } else if( lpbmfh->bfOffBits >= fsiz ) {
   		prt( " BMP ERROR: Offset to bits too large!"MEOR );
      } else if ( lpbmfh->bfOffBits != ((DWORD)lps - (DWORD)lpb) ) {
         sprintf(lpd, " BMP ERROR: Offset to bits %d (%#X), not as computed %d (%#X)!"MEOR,
            lpbmfh->bfOffBits, lpbmfh->bfOffBits,
            ((DWORD)lps - (DWORD)lpb), ((DWORD)lps - (DWORD)lpb));
         prt(lpd);
      } else if( fsiz < cfsize ) {
   		prt( " BMP ERROR: File size LESS than calculated!"MEOR );
      	sprintf(lpd,
		      " Calculated Length = %d Bytes. MISSING %d bytes!"MEOR,
            cfsize, (cfsize - fsiz) ) ;
         prt(lpd);
      }
      KillLineList();
		return 0;
	}

   memcpy(&g_bmfh, lpb,   sizeof(BITMAPFILEHEADER) ); // get COPY of this header for later refer
   memcpy(&g_bih, lpbih, infhdr); // and INFORMATION header - TWO TYPES BITMAPINFOHEADER *AND* 
   // BITMAPV5HEADER - First set are the SAME
   // ***************************************
   if(cc2) {
      // if we have a COLOR TABLE - getitnow
      memcpy(&g_ct[0], lpq, ( cc2 * sizeof(DWORD) ) );
   }

	sprintf(lpd,
		"BITMAPFILEHEADER structure -  %d Bytes (%#x)"MEOR,
		sizeof(BITMAPFILEHEADER),
		sizeof(BITMAPFILEHEADER) );
	prts(lpd);

	sprintf(lpd,
		" Type   = BM (4D42)"MEOR );
	prts(lpd);

	sprintf(lpd,
		" Size   = %d Bytes (%#x)"MEOR,
		lpbmfh->bfSize,
		lpbmfh->bfSize );
	prts(lpd);

	sprintf(lpd,
		" Offset = %d (%#x) to bits"MEOR,
		lpbmfh->bfOffBits,
      lpbmfh->bfOffBits );
   if( lpbih->biCompression == BI_BITFIELDS ) {
      sprintf(EndBuf(lpd), " (However BI_BITFIELDS has 3 DWORD masks = Offset %d (%#x))"MEOR,
         lpbmfh->bfOffBits + (sizeof(DWORD) * 3),
         lpbmfh->bfOffBits + (sizeof(DWORD) * 3) );
   }
	prts(lpd);

   // Done FILE header, now
   // do BITMAPINFOHEADER
   imgrow = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT
   imgsiz = imgrow *  // bytes per row, rounded be ALLIGNMENT
            lpbih->biHeight;     // HEIGHT = rows in the image
   if (lpbih->biSize == sizeof(BITMAPINFOHEADER)) {
        sprintf(lpd,
		    "BITMAPINFOHEADER - dwSize =  %d Bytes  (%#x)"MEOR,
		    lpbih->biSize,	// sizeof(BITMAPINFOHEADER) );
            lpbih->biSize );
   } else {
        sprintf(lpd,
		    "BITMAPV5HEADER - dwSize =  %d Bytes  (%#x)"MEOR,
		    lpbih->biSize,	// sizeof(BITMAPINFOHEADER) );
            lpbih->biSize );
   }
	prts(lpd);

	sprintf(lpd,
		" Width    = %d Pixels"MEOR,
		lpbih->biWidth );
	prts(lpd);

	sprintf(lpd,
		" Height   = %d Pixels"MEOR,
		lpbih->biHeight );
	prts(lpd);

	sprintf(lpd,
		" Planes   = %d (Should be 1!)"MEOR,
		lpbih->biPlanes );
	prts(lpd);

	sprintf(lpd,
		" BitCount = %d",
		lpbih->biBitCount );

   // depends on the TYPE of bitmap
	if( lps = GetColorStg(lpbih->biBitCount) )
		strcat(lpd,lps);
   // end of this line
	strcat(lpd,MEOR);
   // and OUT IT
	prts(lpd);

	sprintf(lpd,
		" BitCompr = %d ",
		lpbih->biCompression );
   // various types 
	if( lps = GetCompStg( lpbih->biCompression ) )
		strcat(lpd,lps);
	strcat(lpd,MEOR);
	prts(lpd);

	//uk   = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT
	//iRow = lpbih->biHeight;    // HEIGHT = rows in the image
   if(lpbih->biSizeImage == imgsiz) // (uk * iRow) )
   {
      // this is as it should be
      sprintf(lpd,
		   " Img.Size = %d. ok",
		   lpbih->biSizeImage );
   }
   else if(lpbih->biSizeImage == 0)
   {
      sprintf(lpd,
		   " Img.Size = %d. Seems zero is ok, Should be %d.",
		   lpbih->biSizeImage,
         imgsiz  );
   }
   else
   {
      sprintf(lpd,
		   " Img.Size = %d. (at variance to %d?)",
		   lpbih->biSizeImage,
         imgsiz ); // ( uk * iRow ) );
   }
   sprintf(EndBuf(lpd), " (%dx%d=%d)",
      imgrow, // bytes per row, rounded be ALLIGNMENT
      lpbih->biHeight, // HEIGHT = rows in the image
      imgsiz );
   strcat(lpd,MEOR);
	prts(lpd);

	sprintf(lpd,
		" X.PPMet. = %d"MEOR,
		lpbih->biXPelsPerMeter );
	prts(lpd);

	sprintf(lpd,
		" Y.PPMet. = %d"MEOR,
		lpbih->biYPelsPerMeter );
	prts(lpd);
	
	sprintf(lpd,
		" ClrUsed  = %d",
		lpbih->biClrUsed );
   if(lpbih->biClrUsed) {
      sprintf(EndBuf(lpd)," (cc2 = %d)",  cc2);
   } else {
      sprintf(EndBuf(lpd), " (cc2 = %d)", cc2);
   }
   strcat(lpd,MEOR);
	prts(lpd);

	sprintf(lpd,
		" ClrImp.  = %d"MEOR,
		lpbih->biClrImportant );
	prts(lpd);

	if( cc2 ) {
      UINT uk;
//typedef struct tagRGBQUAD { // rgbq 
//    BYTE    rgbBlue; 
//    BYTE    rgbGreen; 
//    BYTE    rgbRed; 
//    BYTE    rgbReserved; 
//} RGBQUAD; 
//		RGBQUAD *	lpq;
//		UINT		uk;

		lps = (lpb + sizeof(BITMAPFILEHEADER) + lpbih->biSize); //sizeof(BITMAPINFOHEADER)
		sprintf(lpd,
			"COLOUR %s - Count of RGBQUADs = %d (%d Bytes)"MEOR,
         ( (lpbih->biCompression == BI_BITFIELDS) ? "MASK" : "TABLE" ),
			cc2,
			( cc2 * sizeof(RGBQUAD) ) );
		prts(lpd);
		prt( "Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
		lpq = (RGBQUAD *)lps;

		*lpd = 0;
		uk = 0;
		for( ui = 0; ui < cc2; ui++ )
		{
//  0(  0,  0,  0)   1(  0,  0,191)   2(  0,191,  0)   3(  0,191,191) 
         r = lpq->rgbRed;
         g = lpq->rgbGreen;
         b = lpq->rgbBlue;
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
            gsBmpColour[ui] = *lpq; // keep COPY of colour from FILE TABLE
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
			lpq++;
		}

		if( uk )
		{
			strcat(lpd,MEOR);
			prt(lpd);
			*lpd = 0;
			uk = 0;
		}

      uk = SortRGBQ( (LPTSTR)&gsBmpColour[0], FALSE, cc2 );
		sprintf(lpd,
			"COLOUR TABLE - Count of Colours  = %d."MEOR,
			uk );
		prt(lpd);

	}  // done colour map - ie cc contains colour count

	foff = lpbmfh->bfOffBits; // can be WRONG for lpbih->biCompression == BI_BITFIELDS
   // but this SHOULD BE IT, *** IN ALL CASES ***
   foff = sizeof(BITMAPFILEHEADER) +
		lpbih->biSize +   // = sizeof(BITMAPINFOHEADER) or OTHER SIZES
		( cc2 * sizeof(RGBQUAD) );
	lps = (lpb + foff);	// = lpbmfh->bfOffBits);
	// *** OR ***
	//lps = (lpb +
	//	sizeof(BITMAPFILEHEADER) +
	//	lpbih->biSize +   // = sizeof(BITMAPINFOHEADER) or OTHER SIZES
	//	( cc * sizeof(RGBQUAD) ) );

	//uk   = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT
	iRow = lpbih->biHeight;
	iC   = iCol = ValidPerRow(lpbih);
   if( iC == imgrow )
   {
	   sprintf(lpd,
		   "IMAGE (%dx%d) - Count of BYTES = %d x %d = %d Bytes"MEOR,
		   lpbih->biWidth,
		   lpbih->biHeight,
		   imgrow,
		   lpbih->biHeight,
		   ( lpbih->biHeight * imgrow ) );
   }
   else
   {
	   sprintf(lpd,
		   "IMAGE (%dx%d) - Count of BYTES = %d(v=%d) x %d = %d Bytes"MEOR,
		   lpbih->biWidth,
		   lpbih->biHeight,
		   imgrow, iC,
		   lpbih->biHeight,
		   ( lpbih->biHeight * imgrow ) );
   }
	prts(lpd);
		 
	ui = 0;  // output counter - the row of ascii being shown
	ia = 0;
//	for( ir = foff; ir < rd; ir++ )
   // get to DATA - remember there may be some EXTRA bytes on each ROW
   // to ensure ROW allignment 
	ir = foff;
//#ifdef   USEMAPPING

   g_dwRows = iRow;
   g_dwCols = iCol;
   logcolm = 0;
   irow = lpbih->biHeight;   // upside down bitmaps
   icol = 0;
   if( giBmpVerb == 1 )
   {
      // just a QUICK partial show of the ROWS
      for( ; ir < rd; ir++ )
	   {
		   c = lpb[ir];
		   foff++;
   		if( ui == 0 )
	   	{
            irow--;  // reduce logical ROW
            _s_inewrow = (giBmpVerb == 1) ? 1 : 0;
            // start with ROW header ...
			   iC = iCol;	// *RESTART* valid BYTE counter (from ValidPerRow(lpbih);)
	         sprintf(lpd,
		         "Row %4d: ",
		         iRow );
            logcolm = 0;
            icol = 0;
		   }

         FormRowData( c, lpd, iCol, imgrow,   // total number of BYTES in this ROW incl allignment
            &ui, &iRow, &iC, &ia, irow, icol );

         Bump_Column( pbi->biBitCount, logcolm, &icol );
         logcolm++;
         if( ui == 0 ) {
            break;   // done ONE ROW of data
         }
      }
      // we have done the last row, now cycle forward to the FIRST row
      for( ; ir < rd; ir++ )
	   {
		   c = lpb[ir];
		   foff++;
         if( ui == 0 ) {
            irow--;
            iC = iCol;  // start of NEW row
            logcolm = 0;
            icol = 0;
         }
         ui++;
         if(iC)
         {  // valid BYTE - index into 256 DWORD array - ie bump the COUNT of this index
            gdwColCount[c]++;
            iC--;
         }
         Bump_Column( pbi->biBitCount, logcolm, &icol );
         logcolm++;
         if( ui >= imgrow ) // wrap at the row end
         {
            ui = 0;     // restart counter
            if(iRow == 1)
               break;
         }
      }
      ia = 0;
      for( ; ir < rd; ir++ )
	   {
		   c = lpb[ir];
		   foff++;
   		if( ui == 0 )
	   	{
            irow--;
            _s_inewrow = (giBmpVerb == 1) ? 1 : 0;
            // start with ROW header ...
			   iC = iCol;	// *RESTART* valid BYTE counter (from ValidPerRow(lpbih);)
      	   sprintf(lpd,
		         "Row %4d: ",
		         iRow );
            logcolm = 0;
            icol = 0;
		   }

         FormRowData( c, lpd, iCol, imgrow,   // total number of BYTES in this ROW incl allignment
            &ui, &iRow, &iC, &ia, irow, icol );

         Bump_Column( pbi->biBitCount, logcolm, &icol );
         logcolm++;
//         if( ui == 0 )
//            break;   // done ONE ROW of data
      }

      if( cc2 == 256 )
      {
         ShowColourCnt(lpbih,lpd);
      }

   	ui = 0;  // output counter - the row of ascii being shown
	   ia = 0;  // any remainder, still in the pipe, sort of ...

   }
   else  // NOT -bmp1
   {
      ucnt = 0;
      logcolm = 0;
      if(( giBmpVerb == 2 ) &&
         ( pbi->biBitCount == 24 )) { // Put them in THREE
         rgb24cnt = 0;
         rgb24max = 256;
         prgb24 = dMALLOC( (sizeof(RGB24) * rgb24max) );
         CHKMEM(prgb24);
      }

      // go through the bits data, byte by byte
      // but KEEP track of irow and icol during the byte processing
      // ==========================================================
      for( ; ir < rd; ir++ )
	   {
		   c = lpb[ir];
		   foff++;
   		if( ui == 0 )
	   	{
            irow--;
            icol = 0;
            _s_inewrow = (giBmpVerb == 1) ? 1 : 0;
            // start with ROW header ...
			   sprintf(lpd,
				   "Row %4d: ",
				   iRow );
			   iC = iCol;	// *RESTART* valid BYTE counter (from ValidPerRow(lpbih);)
            //if(ucnt) {
            //   sprtf( "WARNING: Have a count (%d)???"MEOR, ucnt );
            //   wait_keyin();
            //}
            ucnt = 0;
            logcolm = 0;
		   }

//void  FormRowData( LPTSTR lpd, LPTSTR lpb, DWORD ir, DWORD iCol,
//                  LPDWORD pfoff, LPDWORD pui, LPDWORD piRow, LPDWORD piC, LPDWORD puk,
//                  LPDWORD pia )
         FormRowData( c, lpd, iCol, imgrow,   // total number of BYTES in this ROW incl allignment
            &ui, &iRow, &iC, &ia, irow, icol );
         ucnt++;
         if( giBmpVerb == 2 ) {
            // *** BUT PRESENTLY ONLY FOR 24-bpp BITMAPS ***
            // =============================================
            if( pbi->biBitCount == 24 ) { // Put them in THREE
               if(ucnt == 3) {
                  add_COLOR_rbg24();
                  ucnt = 0;
               }
            }
         }
         Bump_Column( pbi->biBitCount, logcolm, &icol );
         logcolm++;
      }
   }
// #endif   // #ifdef   USEMAPPING

   if( ia ) { // if any remainder
	   strcat(lpd,MEOR);
	   prt(lpd);
      prt("WARNING: Appears did NOT finish this ROW!"MEOR );
   }
   if(( giBmpVerb == 2 ) &&
      ( pbi->biBitCount == 24 ) &&
      ( prgb24 ) ) { // Put them in THREE
      // *** BUT PRESENTLY ONLY FOR 24-bpp BITMAPS ***
      // =============================================
      Show_RGB_Colors();
      if( do_vertical_strip ) {
         int   i = 0;
         PRGB24 tmp = prgb24; // keep this color set safe
         UINT  tmpcnt = rgb24cnt;   // and its count
         rgb24cnt = 0;  // restart color COUNTER
         prgb24 = dMALLOC( (sizeof(RGB24) * rgb24max) );
         CHKMEM(prgb24);
      	foff = lpbmfh->bfOffBits;
	      lps = (lpb + foff);	// = lpbmfh->bfOffBits);
         for( icol = 0; icol < lpbih->biWidth; icol++ ) {  // proceed in COLUMNS
            for( irow = lpbih->biHeight - 1; irow >= 0; irow-- ) { // process HEIGHT
               PRGB24 p24 = (PRGB24) &lps[ (irow * imgrow) + (icol * sizeof(RGB24)) ];
               r = p24->rgbRed;
               g = p24->rgbGreen;
               b = p24->rgbBlue;
               i = rc_in_range( irow, icol );
               if((( irow < 4 )||(irow > 30)) ||
                  (( icol < 4 )||(icol > 30)))
                  i = 0;
               else
                  i = 1;

               if( rc_in_range( irow, icol ) ) {
                  sprtf( "In range %d,%d rbg(%3d,%3d,%3d) "MEOR, irow, icol, r, g, b );
                  add_RGB_color( r, g, b );
               } else {
                  sprtf( "NOT IN RANGE %d,%d rbg(%3d,%3d,%3d) "MEOR, irow, icol, r, g, b );
               }
            }
         }
         //// Compare_RGB_Colors( tmp, tmpcnt, prgb24, rgb24cnt );
         Show_RGB_Colors2();
         dMFREE(tmp);
      }
   }

   if( prgb24 )
      dMFREE(prgb24);
   prgb24 = NULL;

   ShowLineList();
   KillLineList();

   return 1;   // successfully shown bitmap
}

// ===============================================================
// PPM = Portable Pixmap (.ppm) P6 format.
DWORD GetPPMLine( LPTSTR lpb, PBYTE pb, DWORD dwmax, PDWORD pdwi )
{
   DWORD    dwk = 0;
   DWORD    dwi = *pdwi;
   DWORD    b;
   for(; dwi < dwmax; dwi++)
   {
      if( pb[dwi] > ' ' )
         break;
   }
   for(; dwi < dwmax; dwi++)
   {
      b = pb[dwi];
      if( b < 0x20 )
         break;
      lpb[dwk++] = (TCHAR)b;
   }
   lpb[dwk] = 0;     // zero terminate
   while(dwk)
   {
      dwk--;
      if(lpb[dwk] > ' ' )
      {
         dwk++;
         break;
      }
      lpb[dwk] = 0;
   }
   *pdwi    = dwi;   // update offset
   return dwk;
}

#define  dwHeight lpdf->df_dwHeight
#define  dwWidth  lpdf->df_dwWidth
#define  dwdfMax    lpdf->df_dwMax

BOOL  IsPPMFile( LPDFSTR lpdf, LPTSTR lpf, PBYTE pb, DWORD dwmax )
{
   BOOL  bRet = FALSE;
   LPTSTR   lpb = &gcOutBuf[0];  // [16K]
   DWORD    dwi, dwk;
   //LPTSTR   lpb2 = &gcOutBuf2[0];  // [16K]
   LPTSTR   lpb2, p;

   dwi = 0;
   dwHeight = dwWidth = dwdfMax = 0;
   dwk = GetPPMLine( lpb, pb, dwmax, &dwi );
   if( !dwk )
      return FALSE;
   if( STRCMPI( lpb, "P6" ) )
      return FALSE;
Nxt_Line:
   dwk = GetPPMLine( lpb, pb, dwmax, &dwi );
   if( !dwk )
      return FALSE;
   if( *lpb == '#' )
      goto Nxt_Line;
   // expect TWO numbers, height and width
   // from FGFS is usually 800 x 600
   if( !ISNUMERIC(*lpb) )
      return FALSE;
   lpb2 = lpb;
   p = lpb2;
   while( ISNUMERIC(*p) )
      p++;
   *p = 0;
   dwWidth = atoi(lpb2);
   p++;
   lpb2 = p;
   if( !ISNUMERIC(*p) )
      return FALSE;
   while( ISNUMERIC(*p) )
      p++;
   *p = 0;
   dwHeight = atoi(lpb2); 
   if( !dwHeight || !dwWidth )
      return FALSE;

   dwk = GetPPMLine( lpb, pb, dwmax, &dwi );
   dwdfMax = atoi(lpb);
   if( dwdfMax != 255 )
      return FALSE;

   if( pb[dwi] != '\n' )
      return FALSE;

   dwi++;
   lpdf->df_dwOff = dwi;         // offset to BEGIN of DATA
   dwk = 3 * dwWidth * dwHeight;
   if( (dwi + dwk) <= dwmax )
      bRet = TRUE;

   return bRet;
}

// ok, if FEELS like a PPM file
// I have height and width, and sufficient data
//BITMAPFILEHEADER
//The BITMAPFILEHEADER structure contains information about the type, size, and 
//layout of a file that contains a DIB. 
//typedef struct tagBITMAPFILEHEADER { 
//  WORD    bfType; 
//  DWORD   bfSize; 
//  WORD    bfReserved1; 
//  WORD    bfReserved2; 
//  DWORD   bfOffBits; 
//} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 
//Members
//bfType 
//Specifies the file type, must be BM. 
//bfSize 
//Specifies the size, in bytes, of the bitmap file. 
//bfReserved1 
//Reserved; must be zero. 
//bfReserved2 
//Reserved; must be zero. 
//bfOffBits 
//Specifies the offset, in bytes, from the BITMAPFILEHEADER structure to the 
//bitmap bits. 
//Remarks
//A BITMAPINFO or BITMAPCOREINFO structure immediately follows the 
//BITMAPFILEHEADER structure in the DIB file. 
//typedef struct tagBITMAPINFOHEADER{
//  DWORD  biSize; 
//  LONG   biWidth; 
//  LONG   biHeight; 
//  WORD   biPlanes; 
//  WORD   biBitCount; 
//  DWORD  biCompression; 
//  DWORD  biSizeImage; 
//  LONG   biXPelsPerMeter; 
//  LONG   biYPelsPerMeter; 
//  DWORD  biClrUsed; 
//  DWORD  biClrImportant; 
//} BITMAPINFOHEADER, *PBITMAPINFOHEADER;  

BOOL  WrtFil( HANDLE * ph, PVOID pv, DWORD len )
{
   BOOL  bRet = FALSE;
   HANDLE   h = *ph;
   if( VFH(h) )
   {
      DWORD    dww = 0;
#ifdef WIN32
      if( ( WriteFile( h, pv, len, &dww, NULL ) ) &&
          ( dww == len ) )
      {
         bRet = TRUE;
      }
      else
      {
         CloseHandle(h);
         h = INVALID_HANDLE_VALUE;
         *ph = h;
      }
#else
      dww = fwrite(pv, 1, len, h);
      if (dww == len)
      {
          bRet = TRUE;
      }
      else
      {
          fclose(h);
          h = INVALID_HANDLE_VALUE;
          *ph = h;
      }
#endif
   }
   return bRet;
}

BOOL  DumpPPM( LPDFSTR lpdf )
{
   BOOL     bRet = FALSE;
   DWORD    dwi, dww, dwh, dwwp, dwk;
   DWORD    w, h, i;
   DWORD    b;
   PBYTE    pb = lpdf->lpb;
   PBYTE    ptmp, pln;
   LPTSTR   lpb2 = &gcOutBuf2[0];  // [16K]
	BITMAPFILEHEADER *	lpbmfh;
	BITMAPINFOHEADER *	lpbih;
   HANDLE   fh;

   pln = 0;
   fh  = 0;

   dww = lpdf->df_dwWidth;
   dwh = lpdf->df_dwHeight;
   dwi = lpdf->df_dwOff;
   if( ( !dww ) ||
       ( !dwh ) ||
       ( ( dwi + (3 * dww * dwh) ) > lpdf->dwmax ) )
       return FALSE;

   dwwp = WIDTHBYTES( (dww * 3 * 8) );
   pln = (PBYTE)dMALLOC( (dwwp + 16) );
   if( !pln )
      goto Got_Err;
#ifdef WIN32
   fh = CreateFile( &g_szBmpNm[0],   // file name
      GENERIC_READ | GENERIC_WRITE,                      // access mode
      0,                          // share mode
      0, // SD
      CREATE_ALWAYS,                // how to create
      FILE_ATTRIBUTE_NORMAL,                 // file attributes
      0 );        // handle to template file
#else
    fh = (HANDLE)fopen(&g_szBmpNm[0],"wb");
#endif      
   if( !VFH(fh) )
      goto Got_Err;

   lpbmfh = (BITMAPFILEHEADER *)lpb2;
   lpbih  = (BITMAPINFOHEADER *) ((BITMAPFILEHEADER *)lpbmfh + 1);

   lpbmfh->bfType = BMPSIG;
   lpbmfh->bfReserved1 = 0;
   lpbmfh->bfReserved2 = 0;
   lpbmfh->bfOffBits   = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); 
   lpbmfh->bfSize      = lpbmfh->bfOffBits + (dwwp * dwh);
   
   if( !WrtFil( &fh, lpbmfh, sizeof(BITMAPFILEHEADER) ) )
      goto Got_Err;

   lpbih->biSize          = sizeof(BITMAPINFOHEADER);
   lpbih->biWidth         = dww;
   lpbih->biHeight        = dwh;
   lpbih->biPlanes        = 1;
   lpbih->biBitCount      = 24;
   lpbih->biCompression   = BI_RGB;
   lpbih->biSizeImage     = 0;
   lpbih->biXPelsPerMeter = 0;
   lpbih->biYPelsPerMeter = 0;
   lpbih->biClrUsed       = 0;
   lpbih->biClrImportant  = 0;

   if( !WrtFil( &fh, lpbih, sizeof(BITMAPINFOHEADER) ) )
      goto Got_Err;

   ptmp = &pb[dwi];  // get pointer to data start
   dwk  = 0;
   // No, this is NEARLY ok, EXCEPT we MUST invert the ROWS
   // for a Window BITMAP file
   for( h = 0; h < dwh; h++ )
   {
      ptmp = &pb[ dwi + ((dwh - 1 - h) * (dww * 3)) ];   // get bottom first
      for( w = 0; w < dww; w++ )
      {
         for( i = 0; i < 3; i++ )
         {
            b = *ptmp;              // extract color
            pln[dwk++] = (BYTE)b;   // insert in line buffer
            ptmp++;                 // bump to NEXT
         }
      }

      // write a ROW
      if( !WrtFil( &fh, pln, dwwp ) )
         goto Got_Err;
      dwk = 0;       // restart line buffer
   }
#ifdef WIN32
   CloseHandle(fh);
#else
    fclose((FILE *)fh);
#endif
   dMFREE(pln);


   if( VERB )
   {
      sprintf( lpb2, "BMP file [%s] written."MEOR, &g_szBmpNm[0] );
      prt(lpb2);
   }
   return TRUE;

Got_Err:
#ifdef WIN32
   dww = GetLastError();
#endif
   if(pln)
      dMFREE(pln);
#ifdef WIN32
   if(VFH(fh))
      CloseHandle(fh);
#else 
   if (VFH(fh))
       fclose(fh);
#endif
   if( VERB )
   {
      sprintf( lpb2, "FAILED to write file [%s]."MEOR, &g_szBmpNm[0] );
      prt(lpb2);
   }
   return FALSE;
}

BOOL  ProcessPPM( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   if( IsPPMFile( lpdf, lpdf->fn, lpdf->lpb, (INT)lpdf->dwmax ) )
   {
      bRet = DumpPPM( lpdf );
   }

   return bRet;
}

// END PPM = Portable Pixmap (.ppm) P6 format.
// ===============================================================

#ifndef  PRGBQUAD
#define  PRGBQUAD LPRGBQUAD
#endif   // #ifndef  PRGBQUAD
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : SortRGBQ
// Return type: DWORD 
// Argument   : LPTSTR lps - ptr to array dwords 256
//            : BOOL upd   - if 'sorted' array to be returned
// Description: Passed a pointer to a 256 RGBQUAD array,
//              extract each DIFFERENT colour, and
// return actual COLOUR COUNT
///////////////////////////////////////////////////////////////////////////////
DWORD  SortRGBQ( LPTSTR lps, BOOL upd, DWORD count )
{
	RGBQUAD * lpq = (RGBQUAD *)lps;
   DWORD       ui, uk, uj;
   PRGBQUAD    pq;
   PRGBQUAD    pq2;
   BYTE        r,g,b;
   BYTE        r2,g2,b2;

   ui = uk = 0;
   pq = (PRGBQUAD)malloc((sizeof(RGBQUAD) * 256));
   if(pq)
   {
      ZeroMemory(pq, (sizeof(RGBQUAD) * 256));
      while((ui < 256)&&(ui < count))
      {
         r = lpq->rgbRed;
	   	g = lpq->rgbGreen;
		   b = lpq->rgbBlue;
         for(uj = 0; uj < uk; uj++)
         {
            pq2 = &pq[uj];
            r2 = pq2->rgbRed;
	      	g2 = pq2->rgbGreen;
		      b2 = pq2->rgbBlue;
            if(RGB(r,g,b) == RGB(r2,g2,b2))
               break;
         }
         if( uj == uk )
         {
            pq2 = &pq[uk];
            pq2->rgbRed   = r;
	      	pq2->rgbGreen = g;
		      pq2->rgbBlue  = b;
            uk++;
         }

         lpq++;
         ui++;
      }

      if(upd)
      {
         // *** COPY SORT OF SORTED ARRAY BACK ***
         memcpy(lps,pq, (sizeof(RGBQUAD) * 256) );
         // **************************************
      }

      free(pq);

   }
   return uk;
}

//---------------------------------------------------------------------
//
// Function:   InitBitmapInfoHeader
//
// Purpose:    Does a "standard" initialization of a BITMAPINFOHEADER,
//             given the Width, Height, and Bits per Pixel for the
//             DIB.
//
//             By standard, I mean that all the relevant fields are set
//             to the specified values.  biSizeImage is computed, the
//             biCompression field is set to "no compression," and all
//             other fields are 0.
//
//             Note that DIBs only allow BitsPixel values of 1, 4, 8, or
//             24.  This routine makes sure that one of these values is
//             used (whichever is most appropriate for the specified
//             nBPP).
//
// Parms:      lpBmInfoHdr == Pointer to a BITMAPINFOHEADER structure
//                            to be filled in.
//             dwWidth     == Width of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             dwHeight    == Height of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             nBPP        == Bits per Pixel for the DIB.
//
// History:   Date      Reason
//            11/07/91  Created
//             
//---------------------------------------------------------------------


void InitBitmapInfoHeader( LPBITMAPINFOHEADER pih, DWORD dwWid, DWORD dwHt, int nBPP )
{
	int		iBPP;
	//dv_fmemset( lpBmInfoHdr, 0, sizeof (BITMAPINFOHEADER) );
	ZeroMemory( pih, sizeof (BITMAPINFOHEADER) );

	pih->biSize      = sizeof(BITMAPINFOHEADER);
	pih->biWidth     = dwWid;
	pih->biHeight    = dwHt;
	pih->biPlanes    = 1;

	// Fix Bits per Pixel
	if( nBPP <= 1 )
		iBPP = 1;
	else if( nBPP <= 4 )
		iBPP = 4;
	else if( nBPP <= 8 )
		iBPP = 8;
	else
		iBPP = 24;

	pih->biBitCount  = iBPP;
	pih->biSizeImage = (WIDTHBYTES(dwWid * iBPP) * dwHt);
   pih->biCompression = BI_RGB;  // assume RGB output
}


// eof = DumpBmp.c
