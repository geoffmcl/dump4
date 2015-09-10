
// DumpBmp.h
#ifndef  _DumpBmp_h
#define  _DumpBmp_h

#ifndef WIDTHBYTES
#define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)
#endif

//--------------  DIB header Marker Define -------------------------
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')	/* Simple "BM" ... */

extern   int	BytesPerRow( BITMAPINFOHEADER * lpbih );
extern   void InitBitmapInfoHeader( LPBITMAPINFOHEADER lpBmInfoHdr,
						  DWORD dwWidth,
						  DWORD dwHeight,
						  int nBPP );

// 20090929 - exposed for DumpAVI.c
#define		BMPSIG		'MB'

extern void KillLineList( void );
extern int	GetColorCount( int bits );
extern BOOL	ValidBitCount( int i );
extern LPSTR	GetColorStg( int bits );
extern LPSTR	GetCompStg( UINT ui );
extern DWORD  SortRGBQ( LPTSTR lps, BOOL upd, DWORD count );
extern void prts( PSTR ps );




#endif   // _DumpBmp_h
// eof - DumpBmp.h
