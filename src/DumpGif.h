

// DUmpGif.h
#ifndef  _DumpGif_h
#define  _DumpGif_h

#ifndef  MLPTR
#define  MLPTR *
#endif   // !MLPTR

/* Multiple GIF Image Support */
/* ========================== */

// GIF Extensions
#define GIF_AppExt              0xFF
#define GIF_CommExt             0xFE
#define GIF_CtrlExt             0xF9
#define GIF_PTxtExt             0x01

#define GIF_Trailer             0x3B
#define GIF_ImgDesc             0x2C

// NOTE: ghMaxCount member can not be larger than 0x7fff!
// That's 32,767 images - A reasonable MAXIMUM!!!
// ======================================================
typedef struct tagGIFIMG {
	WORD	giWidth;
	WORD	giHeight;
	WORD	giCMSize;
	DWORD	giBMPSize;
}GIFIMG;
typedef GIFIMG MLPTR LPGIFIMG;

typedef struct tagGIFHDR {
	WORD	ghMaxCount;
	WORD	ghImgCount;
	WORD	ghWidth;
	WORD	ghHeight;
	WORD	ghCMSize;
	DWORD	ghBMPSize;
	GIFIMG ghGifImg[1];
}GIFHDR;
typedef GIFHDR MLPTR LPGIFHDR;

// Multiple and Transparent GIF Image Support
// ==========================================
#define		gie_Flag		0x8000	// This is in the ghMaxCount
// if the application expect an EXTENDED description!

typedef	struct	tagGIFIMGEXT {
	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
// Image Descriptor - Wdith and Height in above
	WORD	giLeft;		// Left (logical) column of image
	WORD	giTop;		// Top (logical) row
	BYTE	giBits;		// See below (packed field)
// Graphic Control Extension
	BYTE	gceExt;		// Extension into - Value = 0x21
	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
	DWORD	gceSize;	// Block Size (0x04 for GCE, Big for TEXT)
	BYTE	gceBits;	// See below (packed field)
	WORD	gceDelay;	// 1/100 secs to wait
	BYTE	gceIndex;	// Transparency Index (if Bit set)
	DWORD	gceColr;	// Only valid IF bit is SET
	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
	RGBQUAD	gceBkGrnd;	// Background Colour
	// NOTE: These 6 are NOT used or altered by the Library
	// --------------------------------------------------
	HANDLE	hDIB;		// Handle to the DIB
	HPALETTE hPal;		// Handle to the bitmap's palette.
	HBITMAP	hBitmap;	// Handle to the DDB.
	HFONT	hFont;		// Handle to TEXT Font (if any)
	LPVOID	lpNext;		// Pointer to Service
	DWORD	dwgiFlag;	// Various APPLICATION defined Flags
	// --------------------------------------------------
	DWORD	gceRes1;	// Reserved
	DWORD	gceRes2;	// ditto
}GIFIMGEXT;
typedef GIFIMGEXT MLPTR LPGIFIMGEXT;

typedef struct tagGIFHDREXT {
	WORD	gheMaxCount;	// gie_Flag + MAX. Count
	WORD	gheImgCount;	// Images in GIF
	WORD	gheWidth;		// Logical Screen Width
	WORD	gheHeight;		// Logical Screen Height
	WORD	gheCMSize;		// BMP Colour map size (byte count)
	DWORD	gheBMPSize;		// Estimated final BMP size
	BYTE	gheBits;		// See below (packed field)
	BYTE	gheIndex;		// Background Colour Index
	BYTE	ghePAR;			// Pixel Aspect Ration
	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
	RGBQUAD	gheBkGrnd;		// Background Colour
	// NOTE: These 5 are NOT used or altered by the Library
	// --------------------------------------------------
	HANDLE	hDIB;			// Handle to the DIB
	HPALETTE hPal;			// Handle to the bitmap's palette.
	HBITMAP	hBitmap;		// Handle to the DDB.
	HFONT	hFont;			// Handle to TEXT Font (if any)
	DWORD	dwghFlag;		// Application FLAG
	// --------------------------------------------------
	DWORD	gheRes1;		// Reserved
	DWORD	gheRes2;		// ditto
	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
}GIFHDREXT;
typedef GIFHDREXT MLPTR LPGIFHDREXT;

// GIFHDREXT.gheBits = GIF Logical Screen Descriptor Bits
// =================
#define		ghe_ColrMap		0x80	// A Global Color Map
#define		ghe_ColrRes		0x70	// Colour Resolution
#define		ghe_Sort		0x08	// Sorted Colour Map
#define		ghe_ColrSize	0x07	// Size of Colour Table ((n+1)^2)

// GIFIMGEXT gceBits = GIF Graphic Control Extension Bits
// =================
#define		gce_Reserved	0xe0	// Reserved 3 Bits
#define		gce_Disposal	0x1c	// Disposal Method 3 Bits
#define		gce_UserInput	0x02	// User Input Flag 1 Bit
#define		gce_TransColr	0x01	// Transparent Color Flag 1 Bit

// GIFIMGEXT.giBits = GIF Graphic Image Bits
// ================
#define		gie_ColrLoc		0x80	// Local Colour Table
#define		gie_Interlace	0x40	// Interlaced Scan lines
#define		gie_SortFlag	0x20	// Sorted Color Table3
#define		gie_Reserved	0x18	// 2 reserved bits
#define		gie_ColrSize	0x07	// Colr Table Size ((n+1)^2)

// GIFHDREXT Flag
// An "ANIMATED" GIF
#define		ghf_Netscape	0x00000001	// Contains "Netscape" Extension
#define		ghf_AppExt		0x00000002	// Had App Extension
#define		ghf_CommExt		0x00000004	// Had Comment Extension
#define		ghf_CtrlExt		0x00000008	// Had Graphic Control Extension
#define		ghf_PTxtExt		0x00000010	// Had plain text extension

#define		ghf_UnknExt		0x40000000	// Had an UNKNONW extension
#define		ghf_Incomplete	0x80000000	// Incomplete GIF data

// GIFIMGEXT Flag
#define		gie_Netscape	0x00000001	// Found NETSCAPE extension
#define		gie_GCE			0x00000002	// Graphic Control Extension
#define		gie_PTE			0x00000004	// Plain Text Extentsion
#define		gie_GID			0x00000008	// Image Descriptor
#define		gie_APP			0x00000010	// Application Extension
#define		gie_COM			0x00000020	// Comment Extension
#define		gie_UNK			0x80000000	// Undefined Extension

// For Library Service WGIFNBMPX - Get GIF *AND* Plain Text
// This header comes before the Plain Text Extension header,
// and the actual "plain text" data.
typedef	struct tagPTEHDR {	/* pt */
	DWORD	pt_Total;	// Total length of Plain Text Buffer
	DWORD	pt_Missed;	// Any TEXT missed (due to buffer size)
}PTEHDR;
typedef PTEHDR MLPTR LPPTEHDR;


extern   BOOL  ProcessGIF( LPDFSTR lpdf );

extern   BOOL  g_bWriteBmp;  // command "-gif:" to write out a bmp
// of the conversion of the LZW data to 8-bit color indexes
extern   TCHAR g_szbmpout[];  // [264] = { "\0" }; command "-gif:output.bmp"
extern   BOOL  g_bOutLess; // def = TRUE; command "-gif+output256.bmp
extern   BOOL  g_bTransTran; // translate TRANSPARENT colour to 'something' command '-gifT[...]

#endif   // _DumpGif_h
// eof - DumpGif.h
