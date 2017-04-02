// DumpDFS.c

#include	"Dump4.h"

// FIX980523 - Add ID, height(alitude), and time
typedef struct tagLLD {
	double	ll_Lat;  // objects latitude
	double	ll_Long; // longitude, and
   double   ll_Height;  // end of the 3 earth equations
}LLD, * PLLD;
typedef LLD FAR * LPLLD;

// FsDfs.c
// ==================================================
#define		FSDHDROLD		'MEFS'
#define		FSDHDR			'MYFS'

// Record types - First not really
// Also OFFSETS into some GDI Paint items
// ======================================
#define		fsd_Unk			0
#define		fsd_Air			1	// Airport
#define		fsd_Rwy			2	// Runway
#define		fsd_Vor			3	// VOR
#define		fsd_Ils			4	// ILS
#define		fsd_Ndb			5	// NDB
// Added Dec 1997
#define		fsd_Atis		6	// ATIS
// FIX980407 - Add Land-Me and Markers
#define		fsd_LMe			7	// LandMe (ie an Airport!)
#define		fsd_IMO			8	// Markers (Inner,Middle,Outer)
#define     fsd_Building   9  // building from BGL file - 20020714
#define     fsd_Poly       10 // polygon
#define     fsd_Line       11 // lines

#define		fsd_Max			32	// MAXIMUM (for now)

// And some combined definitions
// =============================
// Is a Airport/Runway TYPE
#define	IsVA( typ )		(\
	( typ == fsd_Air ) ||\
	( typ == fsd_Rwy ) ||\
	( typ == fsd_Atis) ||\
	( typ == fsd_LMe ) ||\
	( typ == fsd_IMO ) )

// Is a NavAid TYPE
#define	IsVN( typ )		(\
	( typ == fsd_Vor ) ||\
	( typ == fsd_Ils ) ||\
	( typ == fsd_Ndb ) )

// Is a Visible TYPE
#define  IsVB( typ )  (\
   ( typ == fsd_Building ) ||\
   ( typ == fsd_Poly     ) ||\
   ( typ == fsd_Line     ) )

// Is a valid TYPE
#define	IsVT( typ )		(\
	( IsVA( typ ) ) ||\
	( IsVN( typ ) ) ||\
   ( IsVB( typ ) ) )
// NOTE: If it IsVT = Is VALID TYPE, then must also
//	if(IsVT(dwc)) chkme( "MUST ADD AN ICON LOAD FOR EACH NEW ITEM!"MEOR );
// ===================================================

typedef	struct	tagFSDFSHDR {	/* fd */
	DWORD		fd_Type;	// Type = "MYFS"
	DWORD		fd_Size;	// File SIZE (EXACT)
}FSDFSHDR, * PFSDFSHDR;
typedef FSDFSHDR FAR * LPFSDFSHDR;

typedef struct tagFSDFSRECOld {	/* fr */
	DWORD		fr_Type;
	DWORD		fr_Len;
	BYTE		fr_File[MAX_PATH];
}FSDFSRECOld;
typedef FSDFSRECOld FAR * LPFSDFSRECOld;

typedef struct tagFSDFSREC {	/* fr */
	DWORD		fr_Type;    // fsd_Air, etc - type of record
	DWORD		fr_Len;		// Total LENGTH of this record (inclusive)
	DWORD		fr_Res;		// Various ADDITONAL things, like Freq.
	BYTE		fr_File[MAX_PATH];   // BGL source file path
}FSDFSREC, * PFSDFSREC;
typedef FSDFSREC FAR * LPFSDFSREC;

typedef struct tagTYP2STG {
   int   type;
   PTSTR name;
}TYP2STG, * PTYP2STG;

TYP2STG sType2Stg[] = {
   { fsd_Unk, "Unknown" },
   { fsd_Air, "Airport" },
   { fsd_Rwy, "Runway" },
   { fsd_Vor, "VOR" },
   { fsd_Ils, "ILS" },
   { fsd_Ndb, "NDB" },
   { fsd_Atis, "ATIS" },
   { fsd_LMe, "LandMe" },  // (ie an Airport!)
   { fsd_IMO, "Marker" },  // (Inner,Middle,Outer)
   { fsd_Building, "Building" }, // from BGL file - 20020714
   { fsd_Poly, "Polygon" },
   { fsd_Line, "Line" },
   { 0,        0   }
};

BYTE  szHdr[8];
UINT  uiTypCnts[fsd_Max];  //32	// MAXIMUM (for now)

void ClearCounts( void )
{
   int   i;
   for( i = 0; i < fsd_Max; i++ )
      uiTypCnts[i] = 0;
}

PTSTR Type2Stg( DWORD type )
{
   static TCHAR typbuf[128];
   PTSTR ps = typbuf;
   PTYP2STG pt2s = &sType2Stg[0];
   while( pt2s->name )
   {
      if( pt2s->type == type )
         return pt2s->name;

      pt2s++;
   }
   sprtf(ps, "Unknown(%d)", type );
   return ps;
}

typedef	struct tagB6LOC {
	WORD	b6_Loc[3];
}B6LOC, * PB6LOC;
typedef	B6LOC FAR * LPB6LOC;

#define	B6P2DWL( a )	((DWORDLONG) (\
		( ((DWORDLONG)((LPB6LOC)a)->b6_Loc[2] << 32) ) |\
		( ((DWORDLONG)((LPB6LOC)a)->b6_Loc[1] << 16) ) |\
		( ((DWORDLONG)((LPB6LOC)a)->b6_Loc[0]) ) ))

// MAIN CONVERSION
// LATITUDE: is basically expressed as a linear distance
// from the Equator, the base unit being 2 m (henceforth
// called "Nu").
// North latitude is positive and South latitude is negative.
// FS5 seems to assume that the Equator-North Pole distance 
// is 10,001,750 m, so:
//     90° of latitude = 5,000,875        Nu
//      1° of latitude =    55,565.277    Nu
//      1' of latitude =       926.08795  Nu
//      1" of latitude =        15.434799 Nu
//          0.0647886" =         1        Nu
// LONGITUDE: is basically expressed as an angle from the
// Greenwich meridian and the base unit (henceforth called "Eu")
// has been chosen in order to have 360° = 1000000h, i.e. in
// order to have a whole angle exactly fitting in 24 bits (3
// bytes), so:
//     90° of longit.  = 4,194,304        Eu
//      1° of longit.  =    46,603.377    Eu
//      1' of longit.  =       776.72296  Eu
//      1" of longit.  =        12.945382 Eu
//         0.0772476"  =         1        Eu
// Longitude increases toward East: East Longitude is below
// 7FFFFFh (and can be considered positive), West Longitude is
// above 800000h (and can also be considered negative).

double dDivsm2 = (double)5000875.0;
double dDivsm  = (double)(10001750 * 128);
double dDivx8  = (double)0x800000;
double dDivx82 = (double)0x80000000;
double dMul90  = (double)90.0;
double dMul180 = (double)180.0;

LONGLONG	llMaxNS  = ((LONGLONG)10001750 << 16);	// ie x 65536
long double	ldDivsm  = (long double)10001750.0;
long double ldMul90  = (long double)90.0;
long double ldDiv64K = (long double)65536.0;
long double ldDivx8  = (long double)0x80000000;
long double ldMul180 = (long double)180.0;


double	ddwLat2Dbl( LONGLONG l )
{
	LONGLONG	lLat;
	long double	dlat, dres;
	double	dret;

	if( l > llMaxNS )
		lLat = (0xffff000000000000 | l);
	else
		lLat = l;
	dlat = (long double)lLat;
	dres = ( dlat * ldMul90 ) / ldDivsm;
	dret = (double)( dres / ldDiv64K );
	return dret;
}

double	ddwLong2Dbl( LONGLONG l )
{
	LONGLONG	lLong;
//	long double	div, mul;
	long double	dlat, dres;
	double	dret;

	if( l & 0x800000000000 )
		lLong = (0xffff000000000000 | l);
	else
		lLong = l;
	dlat = (long double)lLong;
//	div = (long double)0x80000000;
//	mul = (long double)180;
	dres = ( dlat * ldMul180 ) / ldDivx8;
	dret = (double)( dres / ldDiv64K );
	return dret;
}

double	dwLat2Dbl( long lLat )
{
//	double	div, mul;
	double	dlat, dres;

//	div = (double)(10001750 * 128);
//	mul = (double)90;
	dlat = (double)lLat;
//	dres = ( dlat * mul ) / div;
	dres = ( dlat * dMul90 ) / dDivsm;
	return dres;
}

double	dwLong2Dbl( long lLong )
{
//	double	div, mul;
	double	dlat, dres;

	dlat = (double)lLong;
//	div = (double)0x80000000;
//	mul = (double)180;
	//dres = ( dlat * mul ) / div;
	dres = ( dlat * dMul180 ) / dDivx82;
	return dres;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : lLat2Dbl
// Return type: double 
// Argument   : long l
// Description: Given a MSFS BGL latitude value,
//              convert to range South, -90.0 to North up to 90.0
///////////////////////////////////////////////////////////////////////////////
double	lLat2Dbl( long l )
{
	long	lLat;
	double	dlat, dres;

	if( l > 5000875 ) // if LATITUDE is GT the value for 90 degrees
		lLat = (0xff000000 | l);   // yes, establish the NEGATIVE
	else
		lLat = l;
	dlat = (double)lLat;
	dres = ( dlat * dMul90 ) / dDivsm2;
	return dres;
}

double	lLong2Dbl( long l )
{
	long	lLong;
//	double	div, mul;
	double	dlat, dres;

	if( l & 0x800000 )
		lLong = (0xff000000 | l);
	else
		lLong = l;
	dlat = (double)lLong;
	dres = ( dlat * dMul180 ) / dDivx8;
	return dres;
}


typedef	struct	tagFSRUNWAY	{	/* rw */
	WORD	rw_Type;
	B6LOC	rw_Lat;		//2     6   Latitude in Mu
	B6LOC	rw_Long;	//8     6   Longitude in Mu
	B6LOC	rw_Alt;		//14    6   Altitude in Mu
	WORD	rw_Hdg;		//20    2   Heading in Deg16
	WORD	rw_Len;		//22    2   Length in feet
	WORD	rw_Wid;		//24    2   Width in feet
	BYTE	rw_Mkr;		//26    1   Markers: bits  0 ??
//                         1 Threshold marker
//                         2 Touchdown zone marker
//                         3 Fixed distance marker
//                         4 Center line and rwy number
//                         5 ??
//                         6 ??
//                         7 ??
	BYTE	rw_ID;	//27    1   Rwy ID; bit 6 on = L, bit 7 on = R, both on = C
	WORD	rw_EL;	//28    2   Rwy edge lighting: 0 = off, 1 = on
	WORD	rw_Surf;//30    2   Rwy surface: this field references the RUNWAY??.R8
//textures:
//          Num. 0: RUNWAY00.R8 (mud)
//               1: RUNWAY01.R8 (concrete)
//               2: RUNWAY02.R8 (asphalt)
//               3: RUNWAY03.R8 (grass)
	WORD	rw_DTh1;	//32    2   displaced threshold in feet (not supported?)
	WORD	rw_Unk1;	//34    2   ???
	WORD	rw_DTh2;	//36    2   displaced threshold in feet (not supported?)
	WORD	rw_Unk2;	//38    2   ???
//Landing end info:
	BYTE	rw_ThL;	//40    1   Threshold lights: 0 = off, 1 = on
	BYTE	rw_ApL;	//41    1   approach lighting: 2 = MALSR, 6 = ALSF-1, 7 = ALSF-2
	BYTE	rw_SrL;	//42    1   number of strobes
	BYTE	rw_Vasi;//43    1   # VASI bars: 0=none, 1=two, 2=three bars
	WORD	rw_VaA;	//44    2   VASI angle in Deg16
	WORD	rw_VaD;	//46    2   VASI displacement from center line in feet
	WORD	rw_VaM;	//48    2   VASI displacement from midpoint of runway in feet
	WORD	rw_VaB;	//50    2   distance between each bar of VASI in feet
//Takeoff end info:
	BYTE	rw_ThL2;//52    1   threshold lights: 0 = off, 1 = on
	BYTE	rw_ApL2;//53    1   approach lighting: 2 = MALSR, 6 = ALSF-1, 7 = ALSF-2
	BYTE	rw_SrL2;//54    1   number of strobes
	BYTE	rw_Vas2;//55    1   # VASI bars: 0=none, 1=two, 2=three bars
	WORD	rw_VaA2;//56    2   VASI angle in Deg16
	WORD	rw_VaD2;//58    2   VASI displacement from center line in feet
	WORD	rw_VaM2;//60    2   VASI displacement from midpoint of runway in feet
	WORD	rw_VaB2;//62    2   distance between each bar of VASI in feet
}FSRUNWAY;
typedef FSRUNWAY FAR * LPFSRUNWAY;

typedef struct FSAP {	/* ap */
	BYTE	ap_Type;	// 03h Airport (27 bytes + airport name)
	WORD	ap_Len;		// 1  2  record length
	DWORD	ap_Lat;		// 3  4  Latitude  (in Du)
	DWORD	ap_Long;	// 7  4  Longitude (in Du)
	DWORD	ap_Alt;		//11  4  Altitude in 1/256 meters
	WORD	ap_Hdg;		//15  2  Runway heading in Deg16
	WORD	ap_Com1;	//17  2  COM1 frequency (BCD coded: 30h 21h = 121.30)
	WORD	ap_Nav1;	//19  2  NAV1 frequency (as above)
	WORD	ap_Obi1;	//21  2  OBI1 course
	WORD	ap_Nav2;	//23  2  NAV2 frequency (as above)
	WORD	ap_Obi2;	//25  2  OBI2 course
	//BYTE	ap_Name;	//27  -  Variable length 0-terminated ASCII
	// string with the airport and runway name
}FSAP;
typedef FSAP FAR * LPFSAP;

typedef struct tagB3LOC {
	BYTE	b3_Loc[3];
}B3LOC;
typedef B3LOC FAR * LPB3LOC;

typedef struct tagFSVOR	{	/* vo */
	BYTE	vo_Type;	// 04h
	BYTE	vo_Range;	//1     1   Range in 2048 m units
	WORD	vo_Mag;		//2     2   Magnetic dev. (in Deg16)
	BYTE	vo_Unk1;	//4     1   ??
	B3LOC	vo_Lat;		//5     3   Latitude  in INu
	B3LOC	vo_Long;	//8     3   Longitude in IEu
	WORD	vo_Alt;		//11    2   Altitude  in m
	WORD	vo_Appr;	//13    2   always 0000 (approach course???; see rec. 05h below)
	BYTE	vo_ID[5];	//15    5   VOR ID; string padded with spaces
	BYTE	vo_Name[24];//20    24  VOR Name; string padded with spaces
}FSVOR;
typedef FSVOR FAR * LPFSVOR;

typedef struct tagFSILS {	/* il */
	BYTE	il_Type;	//	05h!
	BYTE	il_Range;	//1     1   Range in 2048 m units
	WORD	il_Dev;		//2     2   Magnetic dev. (in Deg16)
	BYTE	il_Unk1;	//4     1   ??
	B3LOC	il_Lat;		//5     3   Latitude  in INu
	B3LOC	il_Long;	//8     3   Longitude in IEu
	WORD	il_Alt;		//11    2   Altitude  in m
	WORD	il_Appr;	//13    2   Approach course (in Deg16)
	BYTE	il_ID[5];	//15    5   VOR ID; string padded with spaces
	BYTE	il_Name[24];//20    24  VOR Name; string padded with spaces
	B3LOC	il_gsLat;	//44    3   Glideslope latitude  in INu?
	B3LOC	il_gsLong;	//47    3   Glideslope longitude in IEu?
	WORD	il_gsAlt;	//50    2   Glideslope altitude  in m?
	WORD	il_gsAng;	//52    2   Glideslope angle?
}FSILS;
typedef FSILS FAR * LPFSILS;

typedef struct FSNDB {	/* nd */
	BYTE	nd_Type;	// 0  1  Constant = 04
	WORD	nd_FrBCD;	// 1  2  Freq. integral part (BCD: 59h 03h = 359 kHz)
	BYTE	nd_FrDec;	// 3  1  Freq. Decimal (00h = .00 kHz; 05h = .50 kHz)
	B3LOC	nd_Lat;		// 4  3  Latitude  in Iu
	B3LOC	nd_Long;	// 7  3  Longitude in Iu
	WORD	nd_Alt;		// 10 2  Altitude in m
	BYTE	nd_Rng;		// 12 1  Range in 2048m units
	BYTE	nd_Id[5];	// 13 5  NDB ID; string padded with spaces
	BYTE	nd_Nm[24];	// 18 24 NDB Name; string padded with spaces
}FSNDB;
typedef FSNDB FAR * LPFSNDB;

typedef struct tagFSATIS	{	/* at */
	BYTE	at_Type;	// 04h ATIS (16 bytes + ATIS text)
	WORD	at_Freq;	// 1  2 frequency (BCD coded: 65h 28h = 128.65)
	B3LOC	at_Lat;		// 3  3 Latitude  in Iu
	B3LOC	at_Long;	// 6  3 Longitude in Iu
	BYTE	at_Rng;		// 9  1 Range in 2048m units
	WORD	at_Len;		//10  2 record length
	BYTE	at_PR1;		//12  1 preferred runway 1 (bit 6 and 7 may be set: ??)
	BYTE	at_PR2;		//13  1 preferred runway 2 (bit 6 and 7 may be set: ??)
	BYTE	at_PR3;		//14  1 preferred runway 3 (bit 6 and 7 may be set: ??)
	BYTE	at_PR4;		//15  1 preferred runway 4 (bit 6 and 7 may be set: ??)
	//BYTE	at_Name;	//16  x Var. len 0-term'd string
}FSATIS;
typedef FSATIS FAR * LPFSATIS;

typedef struct tagFSLANDME {	/* lm */
	BYTE	lm_Type;	// 07h - 24 bytes + airport name
	BYTE	lm_Len;		// 1  1 record length
	B3LOC	lm_LatTD;	// 2  3 Touchdown point latitude  in Iu
	B3LOC	lm_LongTD;	// 5  3 Touchdown point longitude in Iu
	B3LOC	lm_AltTD;	// 8  3 Touchdown altitude in m
	B3LOC	lm_LatA;	// 11 3 Another latitude  in Iu (presumably at the other end of the rwy)
	B3LOC	lm_LongA;	// 15 3 Another longitude in Iu
	B3LOC	lm_AltA;	// 18 3 Another altitude in m
	// BYTE	lm_Nm[1];	// 22 x Variable length 0-terminated ASCII 
	// string with the airport name
	// 22+x  1   runway 1 ID (bit 6 and 7 may be set: ??)
	// 23+x  1   runway 2 ID (bit 6 and 7 may be set: ??)
}FSLANDME;
typedef FSLANDME FAR * LPFSLANDME;

// Airport Markers
// ===============
typedef struct tagFSIMO {	/* im */
	BYTE	im_Type;	// 04h-Inner 05h-Middle 06-Outer
	BYTE	im_Len;		// 1  1 record length
	B3LOC	im_Lat;		// 2  3 Latitude  in Iu
	B3LOC	im_Long;	// 5  3 Longitude in Iu
	WORD	im_Alt;		// 8  2 Altitude in m ??
	BYTE	im_Unk1;	// 10 1 ???
}FSIMO;
typedef FSIMO FAR * LPFSIMO;

//BOOL	GetLLDRwy( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDRwy( PVOID pmv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
//	LPSTR	lpout;

	if( ( lprec ) &&
		( lprec->fr_Type == fsd_Rwy ) )
	{
		LPFSRUNWAY	lpfsrwy;
		LONGLONG	llLat, llLong;

		lpfsrwy = (LPFSRUNWAY)((LPFSDFSREC)lprec + 1);

		llLat  = B6P2DWL( &lpfsrwy->rw_Lat );
		llLong = B6P2DWL( &lpfsrwy->rw_Long );
		if( lplld )
		{
			lplld->ll_Lat  = ddwLat2Dbl( llLat );
			lplld->ll_Long = ddwLong2Dbl( llLong );
		}
		flg = TRUE;
	}
	return flg;
}

// Airport - Integer 4 byte
//BOOL	GetLLDAir( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDAir( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_Air ) )
	{
		LPFSAP	lpfsap;
		long	lLat, lLong;
		lpfsap = (LPFSAP)((LPFSDFSREC)lprec + 1);

		lLat  = (long)( *(LPDWORD)&lpfsap->ap_Lat  );
		lLong = (long)( *(LPDWORD)&lpfsap->ap_Long );

		if( lplld )
		{
			lplld->ll_Lat  = dwLat2Dbl( lLat );
			lplld->ll_Long = dwLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if( ( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_Air];
			lpfsp->fsp_Type = fsd_Air;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = dwLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = dwLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)Meters2Feet( (DWORD)(lpfsap->ap_Alt & 0xffff) );
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT

		flg = TRUE;
	}
	return flg;
}

//BOOL	GetLLDVor( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDVor( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if(( lprec                     ) &&
		( lprec->fr_Type == fsd_Vor ) )
	{
		LPFSVOR	lpfsvor;
		long	lLat, lLong;
		lpfsvor = (LPFSVOR)((LPFSDFSREC)lprec + 1);
		lLat  = (long)( *(PDWORD)&lpfsvor->vo_Lat  & 0x00ffffff );
		lLong = (long)( *(PDWORD)&lpfsvor->vo_Long & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat    = lLat2Dbl( lLat );
			lplld->ll_Long   = lLong2Dbl( lLong );
         lplld->ll_Height = (double)lpfsvor->vo_Alt;  // altitude in m.
		}
#ifdef   ADD_PMV_STRUCT
		if(( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_Vor];
			lpfsp->fsp_Type = fsd_Vor;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)Meters2Feet( (DWORD)(lpfsvor->vo_Alt & 0xffff) );
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT
		flg = TRUE;
	}
	return flg;
}

//BOOL	GetLLDIls( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDIls( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_Ils ) )
	{
		LPFSILS	lpfsils;
		long	lLat, lLong;
		lpfsils = (LPFSILS)((LPFSDFSREC)lprec + 1);
		lLat  = (long)( *(LPDWORD)&lpfsils->il_Lat  & 0x00ffffff );
		lLong = (long)( *(LPDWORD)&lpfsils->il_Long & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat  = lLat2Dbl( lLat );
			lplld->ll_Long = lLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if(( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_Ils];
			lpfsp->fsp_Type = fsd_Ils;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)Meters2Feet( (DWORD)(lpfsils->il_Alt & 0xffff) );
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT
		flg = TRUE;
	}
	return flg;
}

//BOOL	GetLLDNdb( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDNdb( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_Ndb ) )
	{
		LPFSNDB	lpfsndb;
		long	lLat, lLong;
		lpfsndb = (LPFSNDB)((LPFSDFSREC)lprec + 1);
		lLat  = (long)( *(LPDWORD)&lpfsndb->nd_Lat  & 0x00ffffff );
		lLong = (long)( *(LPDWORD)&lpfsndb->nd_Long & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat  = lLat2Dbl( lLat );
			lplld->ll_Long = lLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if( ( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_Ndb];
			lpfsp->fsp_Type = fsd_Ndb;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)Meters2Feet( (DWORD)(lpfsndb->nd_Alt & 0xffff) );
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT
		flg = TRUE;
	}
	return flg;
}

// Airport - Integer 4 byte
//BOOL	GetLLDAtis( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDAtis( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_Atis ) )
	{
		LPFSATIS	lpfsatis;
		long	lLat, lLong;
		lpfsatis = (LPFSATIS)((LPFSDFSREC)lprec + 1);

		lLat  = (long)( *(LPDWORD)&lpfsatis->at_Lat  & 0x00ffffff );
		lLong = (long)( *(LPDWORD)&lpfsatis->at_Long & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat  = lLat2Dbl( lLat );
			lplld->ll_Long = lLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if(( pmv           ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_Atis];
			lpfsp->fsp_Type = fsd_Atis;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)0;
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
		flg = TRUE;
#endif // #ifdef   ADD_PMV_STRUCT
	}
	return flg;
}

//BOOL	GetLLDLMe( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDLMe( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_LMe ) )
	{
		LPFSLANDME	lpfslme;
		long	lLat, lLong;
		lpfslme = (LPFSLANDME)((LPFSDFSREC)lprec + 1);

		lLat  = (long)( *(LPDWORD)&lpfslme->lm_LatTD  & 0x00ffffff );
		lLong = (long)( *(LPDWORD)&lpfslme->lm_LongTD & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat  = lLat2Dbl( lLat );
			lplld->ll_Long = lLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if( ( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_LMe];
			lpfsp->fsp_Type = fsd_LMe;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)0;
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT
		flg = TRUE;
	}
	return flg;
}

//BOOL	GetLLDIMO( LPMEMVIEW pmv, LPFSDFSREC lprec, LPLLD lplld )
BOOL	GetLLDIMO( PVOID pv, LPFSDFSREC lprec, LPLLD lplld )
{
	BOOL	flg = FALSE;
	if( ( lprec ) &&
		( lprec->fr_Type == fsd_IMO ) )
	{
		LPFSIMO	lpfsimo;
		long	lLat, lLong;
		lpfsimo = (LPFSIMO)((LPFSDFSREC)lprec + 1);

		lLat  = (long)( *(LPDWORD)&lpfsimo->im_Lat  & 0x00ffffff );
		lLong = (long)( *(LPDWORD)&lpfsimo->im_Long & 0x00ffffff );
		if( lplld )
		{
			lplld->ll_Lat  = lLat2Dbl( lLat );
			lplld->ll_Long = lLong2Dbl( lLong );
		}
#ifdef   ADD_PMV_STRUCT
		if( ( pmv ) &&
			( pmv->mv_DoPos ) )
		{
			LPFSPOS	lpfsp;
			LPSTR	lpout, lpnm;
			lpfsp = &pmv->mv_fsp_PosList[fsd_IMO];
			lpfsp->fsp_Type = fsd_IMO;		// Object TYPE
			lpfsp->fsp_LLD.ll_Lat  = lLat2Dbl( lLat );
			lpfsp->fsp_LLD.ll_Long = lLong2Dbl( lLong );
//			lpfsp->fsp_OnBmp = FALSE;	// Is pixel (x,y) on BITMAP
			lpfsp->fsp_OnBmp &= ~(pf_OnBmp);	// Is pixel (x,y) on BITMAP
//		LLD		fsp_Alt;		// Altitude (AMSL and AGL)
//		or wsprintf( lpout, "%d", Meters2Feet( (DWORD)lpfsvor->vo_Alt ) );
			lpfsp->fsp_Alt.ll_Lat = (double)0;
			lpout = &szBldObjNm[0];
			*lpout = 0;
			GetObjNmStg( lpout, lprec );
			chkGetLLD();
			if( lpnm = GetObjNmBuf( lpout ) )
				lpfsp->fsp_lpName = lpnm;
		}
#endif // #ifdef   ADD_PMV_STRUCT
		flg = TRUE;
	}
	return flg;
}


typedef	BOOL	(*GETLLD) ( PVOID, LPFSDFSREC, LPLLD );

static LPVOID	GetLLDTable[fsd_Max];
void	InitLLDTable( void )
{
	DWORD	dwc;
	for( dwc = 0; dwc < fsd_Max; dwc++ )
		GetLLDTable[dwc] = 0;

	GetLLDTable[fsd_Air] = &GetLLDAir;
	GetLLDTable[fsd_Rwy] = &GetLLDRwy;
	GetLLDTable[fsd_Vor] = &GetLLDVor;
	GetLLDTable[fsd_Ils] = &GetLLDIls;
	GetLLDTable[fsd_Ndb] = &GetLLDNdb;
	GetLLDTable[fsd_Atis]= &GetLLDAtis;
	GetLLDTable[fsd_LMe] = &GetLLDLMe;
	GetLLDTable[fsd_IMO] = &GetLLDIMO;
}


BOOL  ProcessDFS( LPDFSTR lpdf )
{
   PFSDFSHDR pdfsh = (PFSDFSHDR) lpdf->df_pVoid;
   PFSDFSREC pr;
   DWORD dwsize, type, len, res, recnum;
   PTSTR pf;
   PBYTE pb;
   int   i;
   PTSTR pts;
   LLD   lld;
   GETLLD   gd;
   PTSTR    ptmp;

   InitLLDTable();
   ClearCounts();
   if( pdfsh->fd_Type != FSDHDR )
   {
      sprtf( "File [%s] is NOT DFS ... ERRANT HEADER"MEOR, lpdf->fn );
      return FALSE;

   }
   dwsize = pdfsh->fd_Size;
   if( dwsize != lpdf->qwSize.QuadPart )
   {
      sprtf( "File [%s] is NOT DFS ... WRONG SIZE IN HEADER"MEOR, lpdf->fn );
      return FALSE;
   }
   pb = (PBYTE)pdfsh;
   for( i = 0; i < sizeof(DWORD); i++ ) {
      szHdr[i] = pb[i];
   }
   szHdr[i] = 0;

   pr = (PFSDFSREC)((PFSDFSHDR)pdfsh + 1);
   pb = (PBYTE)pr;
   dwsize -= sizeof(FSDFSHDR);
   recnum = 0;
   while( dwsize )
   {
      recnum++;
      type = pr->fr_Type;  // fsd_Air, etc - type of record
      len  = pr->fr_Len;   // Total additional LENGTH of this record (inclusive)
      res  = pr->fr_Res;   // Various ADDITONAL things, like Freq.
      pf   = &pr->fr_File[0]; // BGL source file path
      if( IsVT( type ) ) {
         uiTypCnts[type]++;
      } else {
         uiTypCnts[0]++;
      }
      len += sizeof( FSDFSREC );
      if( len && ( len <= dwsize )) {
         dwsize -= len;
         pb += len;
         pr = (PFSDFSREC)pb;
      } else {
         sprtf( "File [%s] is NOT DFS ... FAILED ON RECORD %d"MEOR, lpdf->fn,
            recnum );
         return FALSE;
      }
   }
   sprtf( "File [%s] appears to be DFS ..."MEOR
      "Successful full scan of %d records."MEOR, lpdf->fn, recnum );

   dwsize = pdfsh->fd_Size;
   pr = (PFSDFSREC)((PFSDFSHDR)pdfsh + 1);
   pb = (PBYTE)pr;
   dwsize -= sizeof(FSDFSHDR);
   recnum = 0;
   sprtf( "Header record: Marker: %s, File Size: %d."MEOR, szHdr, pdfsh->fd_Size );
   sprtf( "Record type counts:"MEOR );
   // output like
   //Record type counts:
   //Runway = 224
   //VOR = 792
   //ILS = 142
   //NDB = 1271
   //ATIS = 46
   //LandMe = 173
   //Marker = 191
   for( i = 0; i < fsd_Max; i++ )
   {
      pts = Type2Stg( i );
      if( uiTypCnts[i] ) {
         sprtf( "%s = %d (type=%d)"MEOR, pts, uiTypCnts[i], i );
      }
   }
   recnum = 0;
   while( dwsize )
   {
      recnum++;
      type = pr->fr_Type;  // fsd_Air, etc - type of record
      len  = pr->fr_Len;   // Total additional LENGTH of this record (inclusive)
      res  = pr->fr_Res;   // Various ADDITONAL things, like Freq.
      pf   = &pr->fr_File[0]; // BGL source file path
      pts = Type2Stg(type);
      if( IsVT( type ) ) {
         uiTypCnts[type]++;
         gd = GetLLDTable[type];
         if(gd) {
            gd( NULL, pr, &lld );
            ptmp = GetNxtBuf();
            // 123456789012345
            // 02838: LandMe
            sprintf(ptmp, "%5d: %s", recnum, pts );
            while(strlen(ptmp) < 16)
               strcat(ptmp," ");
            sprintf(EndBuf(ptmp), "at lat %3.7f, lon %3.7f ...", 
               lld.ll_Lat, lld.ll_Long );
            if( VERB9 ) {
                DWORD64 off = ((char *)pr - (char *)pdfsh);
               sprintf(EndBuf(ptmp), " Offset %#08X",
                  (DWORD)off );
            }
            sprtf("%s"MEOR, ptmp);
         } else {
            sprtf( "%d: %s - NO lat/lon tranlation available..."MEOR, recnum,
               pts );
         }

      } else {
         uiTypCnts[0]++;
         sprtf( "%d: %s - UNKNOWN TYPE..."MEOR, recnum,
               pts );
      }

      len += sizeof( FSDFSREC );
      if( len && ( len <= dwsize )) {
         dwsize -= len;
         pb += len;
         pr = (PFSDFSREC)pb;
      } else {
         sprtf( "File [%s] is NOT DFS ... FAILED ON RECORD %d"MEOR, lpdf->fn,
            recnum );
        return FALSE;
      }
   }
   return TRUE;
}

// eof - DumpDFS.c
