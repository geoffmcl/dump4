

// DumpCis.c

#include	"Dump4.h"
#include	"CIS.H" // fix CaSe for unix

#ifdef   ADDCISD4    /* add CIS Compuserve files support */

#define		DIAGFILE
#define		MINCOLS			65
#define		MAXCOLS			78

char	szCisKHdr[] = KEYHDR;	//	"CIM KEYFILE"
char	szVisHdr[]  = VISHDR;	// 	"VIS000"

#ifdef	DIAGFILE
// External Items
extern	void	OpenDiagFile( LPTSTR lpf, HANDLE * lpHF, BOOL bApd );
extern	void	CloseDiagFile( HANDLE * lpHF );
extern   void	WriteDiagFile( LPTSTR lpd );

#define		wrtit( a )	WriteDiagFile( a )
#define		closeit		CloseDiagFile( &hDbgFile )
#else	// !DIAGFILE

#define		wrtit( a )
#define		closeit

#endif	// DIAGFILE y/n

//typedef struct  tagCISKHDR {     /* ch */
//		BYTE	ch_Name[LCISKHDR];
//		BYTE	ch_Stop;
//		WORD    ch_Res1;
//		WORD    ch_Count;               // Count of NAME + DIRECTORY
//		DWORD   ch_FFFFFFFF;    // ????
//}CISKHDR;

//typedef CISKHDR * PCISKHDR;
//typedef	struct	tagCISDATE {
//	BYTE	cDay;
//	BYTE	cMth;
//	BYTE	cYr;
//}CISDATE;
//typedef CISDATE * PCISDATE;
//typedef	struct	tagCISTD	{
//	BYTE	tSec;
//	BYTE	tMin;
//	BYTE	tHr;
//	CISDATE	tDate;
//}CISTD;
//typedef struct {
//	BYTE    cf_Vis[LCISHDR];      // = "VIS000"
//	BYTE    cf_V1a;         // = 0x1a
//	BYTE    cf_Type;        // = "K"=ART "F"=MSG "J"=PLX
//	BYTE    cf_Unk1[2];     // Unknown
//	//BYTE	cf_sec;
//	//BYTE	cf_Min;
//	//BYTE	cf_Hr;
//	//BYTE	cf_Day;
//	//BYTE	cf_Mth;
//	//BYTE	cf_Yr;
//	CISTD	cf_TD1;
//	//BYTE	cf_Sec2;
//	//BYTE	cf_Min2;
//	//BYTE	cf_Hr2;
//	//BYTE	cf_Day2;
//	//BYTE	cf_Mth2;
//	//BYTE	cf_Yr2;
//	CISTD	cf_TD2;
//	BYTE    cf_Unk2[2];     // Unknown
//	WORD    cf_Unk3;        // = 00 00
//	//BYTE    cf_Len;         // Length
//} CISF1;
//typedef CISF1 FAR * PCISF1;

//typedef	struct {
//	BYTE	v2_Unk1;
//	BYTE	v2_Unk2;
//	CISTD	v2_TD1;
//	CISTD	v2_TD2;
//}VISHDR2;
//typedef VISHDR2 * PVISHDR2;

//typedef struct {
//	BYTE	vb_AutoFile;	// 01=Yes 00=No
//	BYTE	vb_Unk5;	// 00
//	BYTE	vb_Unk6;	// 00
//	BYTE	vb_Unk7;	// 00
//	BYTE	vb_Unk8;	// 29
//	BYTE	vb_Unk9;	// 00
//	BYTE	vb_Unk10;	// 02
//	BYTE	vb_Unk11;	// 00
//	BYTE	vb_Unk12;	// 00
//	BYTE	vb_Unk13;	// 00
//	BYTE	vb_Importance;	// 01=Normal 00=Low 02=High
//	BYTE	vb_Sensitive;	// 00=Normal 01=Personal 02=Private
//03=Confidential
//	BYTE	vb_Receipt;	// 00= No 01=Yes
//	BYTE	vb_Unk17;	// 00
//	BYTE	vb_Payment;	// 08  0x40=Shared 0x80=Receiver 0x01=Rel.Date
//				//	0x02=Exp.Date
//	BYTE	vb_Unk19;	//
//	BYTE	vb_Unk20;
//	BYTE	vb_Unk21;
//	//BYTE	vb_RelDay;	// 1C 06 1B =
//	//BYTE	vb_RelMth;	// 28/06/97
//	//BYTE	vb_RelYr;	// NOTE: 0x01 in vb_Payment for Valid Date
//	CISDATE	vb_RelDate;
//	BYTE	vb_Unk25;
//	BYTE	vb_Unk26;
//	BYTE	vb_Unk27;
//	//BYTE	vb_ExpDay;	// 1C 06 1B =
//	//BYTE	vb_ExpMth;	// 28/06/97
//	//BYTE	vb_ExpYr;	// NOTE: 0x02 in vb_Payment for Valid Date
//	CISDATE	vb_ExpDate;
//	BYTE	vb_Unk31;	// 00
//	BYTE	vb_Attach;	// 01=None 02=Attached
//	BYTE	vb_Unk33;	// 01
//	WORD	vb_TxtLen;	// Length of TEXT content (incl @b, etc)
//	BYTE	vb_Unk36;
//	BYTE	vb_Unk37;
//	BYTE	vb_Unk38;
//	BYTE	vb_Unk39;
//	BYTE	vb_Unk40;	// 05
//	BYTE	vb_Unk41;
//	BYTE	vb_Unk42;
//	BYTE	vb_Unk43;
//	BYTE	vb_Unk44;
//	BYTE	vb_Unk45;
//	BYTE	vb_Attach2;	// 02
//	WORD	vb_AttLen;	// 00 1C for example
//	BYTE	vb_Unk49;
//	BYTE	vb_Unk50;
//	// Text starts - If NO attachment
//	// If attachement then
//	// 00 or Len,"Description"
//	// followed by Len,"FileName"
//	// Then 03 00 00 00 00
//	// Where 0x03=Binary 0x04=Text 0x02=GIF 0x06=JPEG
//	// Len(1C),"Path Name"
//	// Plus 00 00 00 00 00
//	// THEN TEXT
//}VISBLK;
////typedef VISBLK FAR * PVISBLK;

typedef	struct {
	BYTE	in_Unk1;
	BYTE	in_Unk2;
	BYTE	in_Unk3;
	BYTE	in_Unk4;
	BYTE	in_Unk5;
	BYTE	in_Unk6;
	BYTE	in_Unk7;
	BYTE	in_Unk8;
	BYTE	in_Unk9;
}VISIN1;
typedef VISIN1 * PVISIN1;
// Len, "Text"

typedef	struct {
	BYTE	i2_Unk1;
	BYTE	i2_Unk2;
	BYTE	i2_Unk3;
	BYTE	i2_Unk4;
	BYTE	i2_Unk5;
	BYTE	i2_Unk6[16];
	BYTE	i2_Unk22[16];
}VISIN2;
typedef	VISIN2 * PVISIN2;

#define		CISKFILE			1
#define		CISVFILE			2

#ifdef	DIAGFILE

void	AddHr( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], " [%02d:", c );
	strcat( lpb, &buf[0] );
}
void	AddMin( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], "%02d:", c );
	strcat( lpb, &buf[0] );
}
void	AddSec( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], "%02d] ", c );
	strcat( lpb, &buf[0] );
}

// Short versions
void	AddHr2( LPSTR lpb, BYTE c )
{
	sprintf( (lpb + strlen(lpb)),
		" %02d:",
		c );
}
void	AddMin2( LPSTR lpb, BYTE c )
{
	sprintf( (lpb + strlen( lpb )),
		"%02d",
		c );
}

void	AddDay( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], " [%02d/", c );
	strcat( lpb, &buf[0] );
}

void	AddMth( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], "%02d/", c );
	strcat( lpb, &buf[0] );
}

void	AddYr( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], "%04d] ", (c + 1970) );
	strcat( lpb, &buf[0] );
}

// Short Versions
void	AddDay2( LPSTR lpb, BYTE c )
{
	sprintf( (lpb + strlen( lpb )),
		" %02d/",
		c );
}

void	AddMth2( LPSTR lpb, BYTE c )
{
	sprintf( (lpb + strlen( lpb )),
		"%02d/",
		c );
}

void	AddYr2( LPSTR lpb, BYTE c )
{
	sprintf( (lpb + strlen( lpb )),
		"%02d",
		(c + 70) );
}

void	AddHex( LPSTR lpb, BYTE c )
{
	char	buf[10];
	sprintf( &buf[0], "%02x ", c );
	strcat( lpb, &buf[0] );
}

void	AddDT( LPSTR lpd, PCISTD pTD )
{
	AddHr( lpd, pTD->tHr );
	AddMin( lpd, pTD->tMin );
	AddSec( lpd, pTD->tSec );
	AddDay( lpd, pTD->tDate.cDay );
	AddMth( lpd, pTD->tDate.cMth );
	AddYr( lpd, pTD->tDate.cYr );
}

char	szInSub[] = "Copy of:";
char	szSubj[132];

BOOL	IsOut( void )
{
	BOOL	flg;
	int		i, j, k;
	flg = FALSE;
	if( ( i = strlen( &szSubj[0] ) ) &&
		( j = strlen( &szInSub[0] ) ) )
	{
		for( k = 0; k < j; k++ )
		{
			if( toupper(szSubj[k]) != toupper(szInSub[k]) )
				break;
		}
		if( k == j )
		{
			// Subject begins with "Copy of:"
			// FLAG IT AS OUT!!!
			flg = TRUE;
		}
	}
	return flg;
}

#endif	// DIAGFILE

UINT	IsCisFile( HFILE hf, LPSTR lpf, LPSTR lpb, DWORD len,
				  DWORD fsiz )
{
	UINT		ftype = 0;

	return ftype;
}

UINT	IsCisFile2( HFILE hf, LPSTR lpf, LPSTR lpb, DWORD len,
				  DWORD fsiz )
{
	UINT		ftype;
	int			i, j;
	PCISKHDR	pCisKey;
	PCISF1		pCisVis;
	char		c;
	LPSTR		lpd, lps;
	BYTE		bi, bl, rc, ri, bc;
	PVISHDR2	pV2;
	PVISBLK		pVis;
	BOOL		bDir;
	PVISIN1		pI1;
	PVISIN2		pI2;
	DWORD		max;

	ftype = 0;
	lpd = &gcDiagBuf[0];
	bi = bl = rc = ri = bc = 0;
	pVis = 0;
	pV2 = 0;
	bDir = FALSE;
	pI1 = 0;
	pI2 = 0;
	if( ( max = len ) &&
		( pCisKey = (PCISKHDR)lpb ) )
	{
		if( len > sizeof( CISKHDR ) )
		{
			for( i = 0; i < LCISKHDR; i++ )
			{
				if( lpb[i] != szCisKHdr[i] )
					break;
			}
			if( i == LCISKHDR )
			{
				if( ( pCisKey->ch_Stop == 0x1a ) &&
					( j = pCisKey->ch_Count ) )
				{
					ftype = CISKFILE;
				}
			}
		}
		if( ( ftype == 0 ) &&
			( len > sizeof( CISF1 ) ) )
		{
			pCisVis = (PCISF1)lpb;
			for( i = 0; i < LCISHDR; i++ )
			{
				if( lpb[i] != szVisHdr[i] )
					break;
			}
			if( i == LCISHDR )
			{
//					( ( c == 'K' ) || ( c == 'F' ) || ( c == 'J' ) ) )
				c = pCisVis->cf_Type;
				if( ( pCisVis->cf_V1a == 0x1a ) &&  // = 0x1a
					c )
				{
					// = ";"=PLX "K"=ART "F"=MSG "J"=PLX
					ftype = CISVFILE;
					lps = (LPSTR)((PCISF1)pCisVis + 1);
#ifdef	DIAGFILE
					sprintf( lpd, "Processing file [%s].", lpf );
					wrtit( lpd );
					strcpy( lpd, "Unknown 1: " );
					AddHex( lpd, pCisVis->cf_Type );  // = "K"=ART "F"=MSG "J"=PLX
					AddHex( lpd, pCisVis->cf_Unk1[0] );     // Unknown
					AddHex( lpd, pCisVis->cf_Unk1[1] );     // Unknown
					wrtit( lpd );
					strcpy( lpd, "HDT: 1 " );
					//AddHr( lpd, pCisVis->cf_TD1.tHr );
					//AddMin( lpd, pCisVis->cf_TD1.tMin );
					//AddSec( lpd, pCisVis->cf_TD1.tSec );
					//AddDay( lpd, pCisVis->cf_TD1.tDate.cDay );
					//AddMth( lpd, pCisVis->cf_TD1.tDate.cMth );
					//AddYr( lpd, pCisVis->cf_TD1.tDate.cYr );
					AddDT( lpd, &pCisVis->cf_TD1 );
					//wrtit( lpd );
					//strcpy( lpd, "Header DT: 2 = " );
					//AddHr( lpd, pCisVis->cf_TD2.tHr );
					//AddMin( lpd, pCisVis->cf_TD2.tMin );
					//AddSec( lpd, pCisVis->cf_TD2.tSec );
					//AddDay( lpd, pCisVis->cf_TD2.tDate.cDay );
					//AddMth( lpd, pCisVis->cf_TD2.tDate.cMth );
					//AddYr( lpd, pCisVis->cf_TD2.tDate.cYr );
					strcat( lpb, " 2 " );
					AddDT( lpd, &pCisVis->cf_TD2 );
					wrtit( lpd );
					strcpy( lpd, "Unknown 2: " );
					AddHex( lpd, pCisVis->cf_Unk2[0] );     // Unknown
					AddHex( lpd, pCisVis->cf_Unk2[1] );     // Unknown
					AddHex( lpd,
						(BYTE)(((pCisVis->cf_Unk3 & 0xff00) >> 8) & 0xff) );
					AddHex( lpd,
						(BYTE)((pCisVis->cf_Unk3 & 0x00ff) & 0xff) );
					wrtit( lpd );
					strcpy( lpd, "Subject:[" );
					lps = (LPSTR)((PCISF1)pCisVis + 1);
					max -= sizeof( CISF1 );	// Reduce by HEADER
					bl = *lps++;	// Get SUBJECT Length
					max--;
					if( max == 0 )
						goto Dn_VBlock;
					szSubj[0] = 0;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					if( bl )
					{
						for( bi = 0; bi < bl; bi++ )
						{
							szSubj[bi] = lps[bi];
						}
						szSubj[bi] = 0;
						sprintf( (lpd + strlen( lpd )),
							"%s",
							&szSubj[0] );
						bDir = IsOut();
					}
					strcat( lpd, "]" );
					//wrtit( lpd );
					lps = lps + bl;
					// REDUCE SIZE
					max -= bl;
					if( max == 0 )
						goto Dn_VBlock;
					if( bl )
					{
						//strcpy( lpd, "Author:[" );
						strcat( lpd, " Author:[" );
						bl = *lps++;
						max--;	// Reduce max
						if( max == 0 )
							goto Dn_VBlock;
						if( (DWORD)bl > max )
							bl = (BYTE)max;
						if( bl )
						{
							for( bi = 0; bi < bl; bi++ )
							{
								sprintf( (lpd + strlen( lpd )),
									"%c",
									(lps[bi] & 0xff) );
							}
						}
						strcat( lpd, "]" );
						//wrtit( lpd );
						lps = lps + bl;
						// REDUCE SIZE
						max -= bl;
						if( max < 2 )
							goto Dn_VBlock;
						bl = *lps++;
						max--;
						if( (DWORD)bl > max )
							bl = (BYTE)max;
						if( bl )
						{
							//strcpy( lpd, "Address:[" );
							strcat( lpd, " Address:[" );
							for( bi = 0; bi < bl; bi++ )
							{
								sprintf( (lpd + strlen( lpd )),
									"%c",
									(lps[bi] & 0xff) );
							}
							strcat( lpd, "]" );
							wrtit( lpd );
							lps = lps + bl;
							max -= bl;
						}
					}
					if( max < 2 )
						goto Dn_VBlock;
					bl = *lps++;	// This SHOULD be ZERO!!!
					max--;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					*lpd = 0;
					while( bl )	// BUT if we do have something
					{
						strcpy( lpd, "Unknown String: [" );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "]" );
						wrtit( lpd );
						lps = lps + bl;
						max -= bl;
						if( max < 2 )
							goto Dn_VBlock;
						bl = *lps++;
						max--;
						if( (DWORD)bl > max )
							bl = (BYTE)max;
					}
					// We are now at Header 2
					pV2 = (PVISHDR2)lps;
					if( max < sizeof( VISHDR2 ) )
						goto Dn_VBlock;
					strcpy( lpd, "Unknown 3: " );
					AddHex( lpd, pV2->v2_Unk1 );
					AddHex( lpd, pV2->v2_Unk2 );
					strcat( lpd, "TD1 = " );
					AddDT( lpd, &pV2->v2_TD1 );
					strcat( lpd, "TD2 = " );
					AddDT( lpd, &pV2->v2_TD2 );
					wrtit( lpd );
					// Now Subject From Address
					lps = (LPSTR)((PVISHDR2)pV2 + 1);
					max -= sizeof( VISHDR2 );
					if( max < 2 )
						goto Dn_VBlock;
					bl = *lps++;
					max--;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					*lpd = 0;
					if( bl )
					{
						strcat( lpd, "Subj: [" );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "] " );
						lps += bl;
						max -= bl;
					}
					if( max < 2 )
						goto Dn_VBlock;
					bl = *lps++;
					max--;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					if( bl )
					{
						strcat( lpd, "From: [" );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "] " );
						lps += bl;
						max -= bl;
					}
					if( max < 2 )
						goto Dn_VBlock;
					bl = *lps++;
					max--;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					if( bl )
					{
						strcat( lpd, "Addr: [" );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "] " );
						lps += bl;
						max -= bl;
					}
					wrtit( lpd );
					if( max < 2 )
						goto Dn_VBlock;
					rc = *lps++;
					max--;
					sprintf( lpd, "Recipient(s) = %u",
						(rc & 0xff) );
					wrtit( lpd );
					ri = rc;
					if( max < 2 )
						goto Dn_VBlock;
					bl = *lps++;
					max--;
					if( (DWORD)bl > max )
						bl = (BYTE)max;
					while( ri-- )
					{
						if( (ri+1) == rc )
							strcpy( lpd, "To: [" );
						else
							strcpy( lpd, "Cc: [" );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "] " );
						lps += bl;
						max -= bl;
						if( max < 2 )
							goto Dn_VBlock;
						strcat( lpd, "] Addr: [" );
						bl = *lps++;
						max--;
						if( (DWORD)bl > max )
							bl = (BYTE)max;
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								(lps[bi] & 0xff) );
						}
						strcat( lpd, "]" );
						lps += bl;
						max -= bl;
						strcat( lpd, " +4 Unks " );
						if( max < 4 )
							goto Dn_VBlock;
						bi = 4;
						while( bi-- )
							AddHex( lpd, *lps++ );
						max -= 4;
						if( ri )	// If NOT last
						{
							if( max < 2 )
								goto Dn_VBlock;
							bl = *lps++;
							max--;
							if( (DWORD)bl > max )
								bl = (BYTE)max;
						}
						wrtit( lpd );
					}	// while recipients

					// We are now at HEADER 3
					if( bDir )	// If OUT
					{
						if( max < sizeof( VISBLK ) )
							goto Dn_VBlock;
						pVis = (PVISBLK)lps;
						//strcpy( lpd, "Unknown 4: " );
						//AddHex( lpd, pVis->vb_Unk1 );	// 00
						//AddHex( lpd, pVis->vb_Unk2 );	// 00
						//AddHex( lpd, pVis->vb_Unk3 );	// 00
						//wrtit( lpd );
						sprintf( lpd, "OUT: AutoFile = %s",
							(LPSTR)(pVis->vb_AutoFile ? "Yes " : "No ") );
						//wrtit( lpd );
						//strcpy( lpd, "Unknown 5: " );
						strcat( lpd, "Unknown 4: " );
						AddHex( lpd, pVis->vb_Unk5 );	// 00
						AddHex( lpd, pVis->vb_Unk6 );	// 00
						AddHex( lpd, pVis->vb_Unk7 );	// 00
						AddHex( lpd, pVis->vb_Unk8 );	// 29
						AddHex( lpd, pVis->vb_Unk9 );	// 00
						AddHex( lpd, pVis->vb_Unk10 );	// 02
						AddHex( lpd, pVis->vb_Unk11 );	// 00
						AddHex( lpd, pVis->vb_Unk12 );	// 00
						AddHex( lpd, pVis->vb_Unk13 );	// 00
						wrtit( lpd );
						strcpy( lpd, "Importance: " );
						switch( pVis->vb_Importance )	// 01=Normal 00=Low 02=High
						{
						case 0:
							strcat( lpd, "Low(0) " );
							break;
						case 1:
							strcat( lpd, "Normal(1) " );
							break;
						case 2:
							strcat( lpd, "Hight(2) " );
							break;
						default:
							strcat( lpd, "Unknown " );
							AddHex( lpd, pVis->vb_Importance );
							break;
						}
						strcat( lpd, "Sensitivity: " );
						switch( pVis->vb_Sensitive )	// 00=Normal 01=Personal 02=Private
//03=Confidential
						{
						case 0:
							strcat( lpd, "Normal(0) " );
							break;
						case 1:
							strcat( lpd, "Personal(1) " );
							break;
						case 2:
							strcat( lpd, "Private(2) " );
							break;
						case 3:
							strcat( lpd, "Confidential(3) " );
							break;
						default:
							strcat( lpd, "Unknown " );
							AddHex( lpd, pVis->vb_Sensitive );
							break;
						}
						sprintf( (lpd + strlen( lpd )),
							"Receipt=%s",
							(LPSTR)(pVis->vb_Receipt ? "Yes" : "No" ) );
						wrtit( lpd );
//	BYTE	vb_Unk17;	// 00
//	BYTE	vb_Payment;	// 08  0x40=Shared 0x80=Receiver 0x01=Rel.Date
//				//	0x02=Exp.Date
//	BYTE	vb_Unk19;	//
//	BYTE	vb_Unk20;
//	BYTE	vb_Unk21;
//	//BYTE	vb_RelDay;	// 1C 06 1B =
//	//BYTE	vb_RelMth;	// 28/06/97
//	//BYTE	vb_RelYr;	// NOTE: 0x01 in vb_Payment for Valid Date
//	CISDATE	vb_RelDate;
//	BYTE	vb_Unk25;
//	BYTE	vb_Unk26;
//	BYTE	vb_Unk27;
//	//BYTE	vb_ExpDay;	// 1C 06 1B =
//	//BYTE	vb_ExpMth;	// 28/06/97
//	//BYTE	vb_ExpYr;	// NOTE: 0x02 in vb_Payment for Valid Date
//	CISDATE	vb_ExpDate;
//	BYTE	vb_Unk31;	// 00
//	BYTE	vb_Attach;	// 01=None 02=Attached
//	BYTE	vb_Unk33;	// 01
//	WORD	vb_TxtLen;	// Length of TEXT content (incl @b, etc)
//	BYTE	vb_Unk36;
//	BYTE	vb_Unk37;
//	BYTE	vb_Unk38;
//	BYTE	vb_Unk39;
//	BYTE	vb_Unk40;	// 05
//	BYTE	vb_Unk41;
//	BYTE	vb_Unk42;
//	BYTE	vb_Unk43;
//	BYTE	vb_Unk44;
//	BYTE	vb_Unk45;
//	BYTE	vb_Attach2;	// 02
//	WORD	vb_AttLen;	// 00 1C for example
//	BYTE	vb_Unk49;
//	BYTE	vb_Unk50;
//	// Text starts - If NO attachment
//	// If attachement then
//	// 00 or Len,"Description"
//	// followed by Len,"FileName"
//	// Then 03 00 00 00 00
//	// Where 0x03=Binary 0x04=Text 0x02=GIF 0x06=JPEG
//	// Len(1C),"Path Name"
//	// Plus 00 00 00 00 00
//	// THEN TEXT
//}VISBLK;
////typedef VISBLK FAR * PVISBLK;
						lps = (LPSTR)((PVISBLK)pVis + 1);
						max -= sizeof( VISBLK );
					}
					else	// !bDir = IN
					{
						if( max < sizeof( VISIN1 ) )
							goto Dn_VBlock;
						pI1 = (PVISIN1)lps;
						strcpy( lpd, "IN: Unk: " );
						bl = sizeof( VISIN1 );
						for( bi = 0; bi < bl; bi++ )
						{
							AddHex( lpd, lps[bi] );
						}
						wrtit( lpd );
//typedef	struct {
//	BYTE	in_Unk1;
//	BYTE	in_Unk2;
//	BYTE	in_Unk3;
//	BYTE	in_Unk4;
//	BYTE	in_Unk5;
//	BYTE	in_Unk6;
//	BYTE	in_Unk7;
//	BYTE	in_Unk8;
//	BYTE	in_Unk9;
//}VISIN1;
//typedef VISIN1 * PVISIN1;
						lps += bl;
						max -= sizeof( VISIN1 );
						if( max < 2 )
							goto Dn_VBlock;
						bl = *lps++;
						max--;
						if( (DWORD)bl > max )
							bl = (BYTE)max;
						strcpy( lpd, "String [" );
						for( bi = 0; bi < bl; bi++ )
						{
// Len, "Text"
							sprintf( (lpd + strlen( lpd )),
								"%c",
								lps[bi] );
						}
						lps += bl;
						max -= bl;
						if( max < sizeof( VISIN2 ) )
							goto Dn_VBlock;
						pI2 = (PVISIN2)lps;
						bl = 5;
						strcpy( lpd, "Unknown: " );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								lps[bi] );
						}
						lps += bl;
//typedef	struct {
//	BYTE	i2_Unk1;
//	BYTE	i2_Unk2;
//	BYTE	i2_Unk3;
//	BYTE	i2_Unk4;
//	BYTE	i2_Unk5;
//	BYTE	i2_Unk6[16];
//	BYTE	i2_Unk22[16];
//}VISIN2;
//typedef	VISIN2 * PVISIN2;
						bl = 16;
						strcpy( lpd, "Unknown: " );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								lps[bi] );
						}
						lps += bl;
						bl = 16;
						strcpy( lpd, "Unknown: " );
						for( bi = 0; bi < bl; bi++ )
						{
							sprintf( (lpd + strlen( lpd )),
								"%c",
								lps[bi] );
						}
						lps += bl;
						lps = (LPSTR)((PVISIN2)pI2 + 1);
						max -= sizeof( VISIN2 );
					}
					// We have reached the TEXT
					strcpy( lpd, "Text:" );
					wrtit( lpd );
					bc = 0;		// Zero column
					*lpd = 0;
					while( max )
					{
						if( max > 16 )
							bl = 16;
						else
							bl = (BYTE)max;
						for( bi = 0; bi < bl; bi++ )
						{
							c = lps[bi];
							if( ( c == 0x0a ) || ( c == 0x09 ) )
							{
								// nothing to do here
							}
							else if( c < ' ' )
							{
								c = '*';	// switch to AST
							}
							if( c == 0x0a )
							{
								//sprintf( (lpd + strlen( lpd )),
								//	"%c",
								//	0x0d );
								//sprintf( (lpd + strlen( lpd )),
								//	"%c",
								//	c );
								bc = 0;	// Zero the column.
								wrtit( lpd );
								*lpd = 0;
							}
							else if( c == '@' )
							{
								if( lps[bi+1] == '@' )
								{
									sprintf( (lpd + strlen( lpd )),
										"%c",
										c );
									bc++;
									bi++;
								}
								else if( lps[bi+1] == 'b' )
								{
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0d );
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0a );
									bi++;
									bc = 0;	// Zero the column
									wrtit( lpd );
									*lpd = 0;
								}
								else
								{
									sprintf( (lpd + strlen( lpd )),
										"%c",
										c );
									bc++;	// Another column
									if( c == 0x09 )
									{
										bc += 6;
									}
								}
							}
							else
							{
								sprintf( (lpd + strlen( lpd )),
									"%c",
									c );
								bc++;	// Another column
								if( c == 0x09 )
								{
									bc += 6;
								}
							}
							if( bc > MINCOLS )
							{
								if( c == ' ' )
								{
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0d );
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0a );
									bc = 0;	// Zero the column
									wrtit( lpd );
									*lpd = 0;
								}
								else if( bc > MAXCOLS )
								{
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0d );
									//sprintf( (lpd + strlen( lpd )),
									//	"%c",
									//	0x0a );
									bc = 0;	// Zero the column
									wrtit( lpd );
									*lpd = 0;
								}
							}
						}	// for bl
						lps += bl;	// Bump the pointer
						max -= bl;	// and redcue the count
					}
					if( bc )
					{
						wrtit( lpd );
						*lpd = 0;
					}
#endif	// DIAGFILE
Dn_VBlock:
					strcpy( lpd, "Done VIS File" );
					wrtit( lpd );
				}	// verified VIS
			}	// i == LCISHDR ie "VIS000" present
		}	// 	if( ( ftype == 0 ) && ( len > sizeof( CISF1 ) ) )
	}
	closeit;	// Close any diag. file openned
	return ftype;

}


void	SetFileType( UINT typ, LPSTR lpb )
{
	if( lpb )
	{
		switch( typ )
		{
		case CISKFILE:
			strcpy( lpb, "CIS KEY FILE:" );
			break;

		case CISVFILE:
			strcpy( lpb, "CIS VIS FILE:" );
			break;

		default:
			strcpy( lpb, "UNKNOWN TYPE:" );
			break;
		}
	}
}

// NOT USED ********
void	DoFile_OK( char * fn, HFILE hf )
{
	int			rd, ir;
	char	*	lpb;
	char		c, d;
	DWORD		foff, fsiz;
	int			ia, fix;
	char	*	lph;
#ifdef   ADDCISD4    /* add CIS compuserve file support */
	UINT		ft;
#endif   /*   ADDCISD4    == add CIS compuserve file support */
	LPSTR		lptmp;
	
	lpb = &gcFilBuf[0];
	lptmp = &gszTmpOut[0];
	if( hf &&
		(hf != HFILE_ERROR) )
	{
		fsiz = GetFileSize( (HANDLE)hf, NULL );
		if( ( giVerbose > 1 ) &&
			( fsiz != (DWORD)-1 ) )
		{
			sprintf( lptmp, "Total Size is %u bytes ..."MEOR, fsiz );
//			prt( lptmp );
		}
		foff = 0;
		lph = &gszHex[0];
		*lph = 0;
		fix = 0;
		// read a block of the file
//		if( rd = _lread( hf, lpb, MXFIO ) )
		if( rd = grmReadFile( (HANDLE)hf, lpb, MXFIO ) )
		{
#ifdef   ADDCISD4    /* add CIS compuserve file support */
			ft = IsCisFile( hf, fn, lpb, rd, fsiz );
			SetFileType( ft, lptmp );
#endif   /*   ADDCISD4    == add CIS compuserve file support */
			if( giVerbose > 1 )
			{
				if( fsiz != (DWORD)-1 )
				{
					sprintf( (lptmp + strlen( lptmp )),
						" Total Size is %u (0x%04X) bytes. ",
						fsiz, fsiz );
				}
				strcat( lptmp, ""MEOR );
				prt( lptmp );
			}
			ia = 0;
			for( ir = 0; ir < rd; ir++ )
			{
				c = lpb[ir];
				if( gfRemPar )
				{
					c = c & 0x7f;
				}
				if( c < ' ' )
					d = '.';
				else if( c >= 0x7f )
					d = '.';
				else
					d = c;

				gszAscii[ia+fix] = d;	// Fill in the ASCII
				ia++;
				if( gw_fUsePrintf &&
					( d == '%' ) )	// Care using PRINTF
				{
					gszAscii[ia+fix] = d;	// Put in 2 for PRINTF
					fix++;
				}

				sprintf( (lph + strlen( lph )),
					"%02X ",
					(c & 0xff) );

				if( ia == 8 )
					strcat( lph, " " );

				if( ia == 16 )
				{
					gszAscii[ia+fix] = 0;

					DoOutput( foff );

					foff += ia;
					ia = 0;
					fix = 0;
					*lph = 0;
				}
			}

			if( rd == MXFIO )
			{
//				while( rd = _lread( hf, lpb, MXFIO ) )
				while( rd = grmReadFile( (HANDLE)hf, lpb, MXFIO ) )
				{
					for( ir = 0; ir < rd; ir++ )
					{
						c = lpb[ir];
						if( gfRemPar )
						{
							c = c & 0x7f;
						}
						if( c < ' ' )
							d = '.';
						else if( c >= 0x7f )
							d = '.';
						else
							d = c;

						gszAscii[ia+fix] = d;	// Fill in the ASCII
						ia++;
						if( d == '%' )	// Care using PRINTF
						{
							gszAscii[ia+fix] = d;
							fix++;
						}

						sprintf( (lph + strlen( lph )),
							"%02X ",
							(c & 0xff) );

						if( ia == 8 )
							strcat( lph, " " );

						if( ia == 16 )
						{
							gszAscii[ia+fix] = 0;
							DoOutput( foff );
							foff += ia;
							ia = 0;
							fix = 0;
							*lph = 0;
						}
					}	// process the block
				}	// while read data
			}	// if MAX read
			if( ia )	// Remaining data
			{
				foff += ia;
				gszAscii[ia+fix] = 0;
				if( ia < 8 )
					strcat( lph, " " );
				while( ia < 16 )
				{
					strcat( lph, "   " );
					ia++;
				}

				DoOutput( foff );

				ia = 0;
				fix = 0;
				*lph = 0;
			}
			if( giVerbose > 1 )
			{
				sprintf( lpb, "Completed [%s] = %u Bytes."MEOR,
					fn, foff );
				prt( lpb );
			}
		}
		else
		{
			if( giVerbose )
			{
				sprintf( lpb, "WARNING: File [%s] is NULL!"MEOR, fn );
				prt( lpb );
			}
		}
	}
	else
	{
		if( giVerbose )
		{
			sprintf( lpb, "WARNING: Unable to OPEN file [%s]!"MEOR, fn );
			prt( lpb );
		}
	}
}


// *****************
#endif   /* ADDCISD4 */



//0000:0000 02 38 00 01 00 0E 4A 61  6D 65 73 20 41 6E 64 65 .8....James Ande
//0000:0010 72 73 6F 6E 21 49 4E 54  45 52 4E 45 54 3A 73 76 rson!INTERNET:sv
//0000:0020 67 40 73 69 6C 69 63 6F  6E 2D 76 61 6C 6C 65 79 g@silicon-valley
//0000:0030 2E 63 6F 2E 75 6B 13 42  79 20 6C 65 74 74 65 72 .co.uk.By letter
//0000:0040 20 31 35 20 4A 61 6E 20  39 38 02 00 12 50 65 74  15 Jan 98...Pet
//0000:0050 65 72 20 42 6F 63 6B 20  70 61 72 20 41 4F 4C 18 er Bock par AOL.
//0000:0060 49 4E 54 45 52 4E 45 54  3A 70 62 6F 63 6B 31 33 INTERNET:pbock13
//0000:0070 40 41 4F 4C 2E 63 6F 6D  13 50 65 74 65 72 27 73 @AOL.com.Peter's
#define  CISECNT        3

typedef struct {
   BYTE  bV02;
   WORD  wCnt;
}CISADHDR;

typedef struct {
   BYTE  bLen;
   BYTE  bDat[1];
}CISADITEM;
typedef CISADITEM * LPCISADITEM;

typedef struct {
   WORD  wNum;
   CISADITEM   sEntry;
}CISADENTRY;
typedef  CISADENTRY * LPCISADENTRY;

typedef struct {
   CISADHDR hdr;
   CISADENTRY  ent[CISECNT];     // 3
}CISADFILE;
typedef CISADFILE * LPCISADFILE;



BOOL	chkcis2( BYTE * lpb, DWORD rd )
{
   BOOL  bFlg = FALSE;
   LPCISADFILE   lpf;
   WORD     wCnt;
   WORD     wNum;
   DWORD    dwi;

   lpf = (LPCISADFILE) lpb;
//      ( wNum == 1 ) &&
   if( ( lpf->hdr.bV02 == 2 ) &&
      ( ( wCnt = lpf->hdr.wCnt ) != 0 ) &&
      ( ( wNum = (lpf->ent[0].wNum & 0x7fff) ) == 1 ) &&
      ( ( dwi = lpf->ent[0].sEntry.bLen ) != 0 ) )
   {
//      chkme( "ok" );
//      for( dwj = 0; dwj < dwi; dwj++ )
//      {
//         b = lpb[dwj];
//      }
      bFlg = TRUE;
   }
   return bFlg;
}

LPTSTR   GetLine( void )
{
   LPTSTR   lpl = &g_Out[0];
   return lpl;
}
DWORD GetLineLen( void )
{

   LPTSTR   lpl = GetLine();
   DWORD dwi = strlen(lpl);
   return dwi;
}

void  IniLine( void )
{
   LPTSTR   lpl = &g_Out[0];
   *lpl = 0;
}
#define MCOL1SIZ           24
#define MCOL2SIZ           24


BOOL  gbByCol = TRUE;
BOOL  gbFixCIS = TRUE;

int   iCol1 = MCOL1SIZ;
int   iCol2 = MCOL1SIZ + MCOL2SIZ;
// return POSITION where the second string begin to be conatined in the first
// 0 = it is NOT
// 1 = begins at char 1
// ...
int   ginstr( LPTSTR lps, LPTSTR lpi )
{
   int   iRet = 0;
   int   i, j, k, l;
   LPTSTR   lpc;
   char     c, cb;

   if( ( lps ) &&
      ( i = strlen(lps) ) &&
      ( lpi ) &&
      ( j = strlen(lpi) ) &&
      ( cb = *lpi ) &&
      ( j <= i ) )
   {
      if( j == i )
      {
         if( lstrcmp(lps,lpi) == 0 )
            iRet++;  // it starts at POSITION 1
      }
      else
      {
         l = i - j;
         for( k = 0; k < l; k++ )
         {
            lpc = &lps[k];
//            if( *lpi == lps[k] )
            c = *lpc;
//            if( *lpi == *lpc )
            if( c == cb )
            {
               if( j == 1 )
               {
                  iRet = k + 1;
                  return iRet;
               }
               else
               {
                  int jj;
                  for( jj = 1; jj < j; jj++ )
                  {
//                     lpc = &lps[k+jj];
                     if( lpi[jj] != lps[ k + jj ] )
                        break;
                  }
                  if( jj == j )
                  {
                     iRet = k + 1;
                     return iRet;
                  }
               }
            }
         }
      }
   }
   return iRet;
}

int   ginstri( LPTSTR lps, LPTSTR lpi )
{
   int   iRet = 0;
   int   i, j, k, l;
   LPTSTR   lpc;
   char     c, cb;

   if( ( lps ) &&
      ( i = strlen(lps) ) &&
      ( lpi ) &&
      ( j = strlen(lpi) ) &&
      ( cb = toupper(*lpi) ) &&
      ( j <= i ) )
   {
      if( j == i )
      {
         if( lstrcmpi(lps,lpi) == 0 )
            iRet++;  // it starts at POSITION 1
      }
      else
      {
         l = i - j;
         for( k = 0; k < l; k++ )
         {
            lpc = &lps[k];
//            if( *lpi == lps[k] )
            c = toupper(*lpc);
//            if( *lpi == *lpc )
            if( c == cb )
            {
               if( j == 1 )
               {
                  iRet = k + 1;
                  return iRet;
               }
               else
               {
                  int jj;
                  for( jj = 1; jj < j; jj++ )
                  {
//                     lpc = &lps[k+jj];
                     if( toupper(lpi[jj]) != toupper( lps[ k + jj ] ) )
                        break;
                  }
                  if( jj == j )
                  {
                     iRet = k + 1;
                     return iRet;
                  }
               }
            }
         }
      }
   }
   return iRet;
}

int   ginstriw( LPTSTR lps, LPTSTR lpi )
{
   int   iRet = ginstri(lps,lpi);
   if( iRet )
   {
      int   i, j, l;
//      LPTSTR   lpc;
      char     c, cb;
      if( ( lps ) &&
         ( i = strlen(lps) ) &&
         ( lpi ) &&
         ( j = strlen(lpi) ) &&
         ( cb = toupper(*lpi) ) &&
         ( j <= i ) )
      {
         if( j == i )
         {
            //if( lstrcmpi(lps,lpi) == 0 )
            //   iRet++;  // it starts at POSITION 1
         }
         else
         {
            l = i - j;
            if( iRet == 1 )
            {
               // done left
            }
            else
            {
               l = iRet - 2;
               c = lps[l];
               if( (c < 0) || IsPChr(c) )
               {
                  // this is BAD
                  iRet = 0;
               }
            }
            if( iRet )
            {
               l = ( (iRet - 1) + j );
               c = lps[l];
               if( (c < 0) || IsPChr(c) )
               {
                  // this is BAD
                  iRet = 0;
               }
            }
         }  
      } 
   }
   return iRet;
}


int  CIS2Int( LPTSTR lpout )
{
   int   i, j;
   TCHAR c;

   if( ( lpout             ) &&
      ( i = strlen(lpout) ) &&
      ( i > 3              ) &&
      ( c = *lpout )       )
   {
      // 1 - seek a "[1,2]" CIS entry, and begin
      if( ( c == '['               ) &&    // a SQUARE brackes CIS entry
         ( lpout[ (i - 1) ] == ']' ) &&
         ( j = ginstr(lpout,"," )  ) &&
         ( j > 2                   ) &&
         ( i > 5                   ) && // min expected is "[1,1]" = 5 chars
         ( ( lpout[1] >= '0' ) && ( lpout[1] <= '9' ) ) )   // numeric begin
      {
         lpout[i-1] = 0;   // remove end brace
         //lpout[j-1] = '.'; // convert 
         strcpy(lpout, &lpout[1]); // remove first brace
         i = strlen(lpout);
         c = *lpout;
      }
      if( ( c >= '0' ) &&
         ( c <= '9' ) )
      {
         // is CIS
         // convert "," to ".", and append "@compuserv.ecom"
         if( ( j = ginstr( lpout, "," ) ) &&
            ( j > 1 ) )
         {
            lpout[j-1] = '.';
         }
         strcat(lpout,"@Compuserve.com");
         i = strlen(lpout);
         c = *lpout;
      }
      else
      {
         if( j = ginstr( lpout, ":" ) )
         {
            // remove the CIS addition
            strcpy(lpout, &lpout[j]);
            // =======================
            i = strlen(lpout);
            c = *lpout;
         }
      }
      // if a compuserve alias it will have no ",", no ".",
      // but more importantly, NO "@" !!!
      if( ginstr(lpout,"@") == 0 )
      {
         strcat(lpout,"@compuserve.com");
         i = strlen(lpout);
         c = *lpout;
      }
   }

   return i;

}

void  AddLine( LPTSTR lpout, DWORD dwi )
{
   LPTSTR   lpl = &g_Out[0];

   // add coma and space 1
   strcat(lpl,", ");

   if( dwi == 2 )
   {
      // second item
      if( gbByCol )
      {
         while(strlen(lpl) < iCol1 )
         {
            strcat(lpl," ");
         }
      }
   }
   else if( dwi == 3 )
   {
      if( gbByCol )
      {
         while(strlen(lpl) < iCol2 )
         {
            strcat(lpl," ");
         }
      }
   }

   if( gbFixCIS )
   {
      CIS2Int(lpout);
   }

   // now ADD this entry
   // ==================
   strcat(lpl,lpout);
   // ==================

}
void  BgnLine( LPTSTR lpout )
{
   LPTSTR   lpl = &g_Out[0];
   strcpy(lpl,lpout);
}
void  EndLine( void )
{
   LPTSTR   lpl = &g_Out[0];
   int   i;
   if( i = strlen(lpl) )
   {
      strcat(lpl,"\r\n");
      prt(lpl);
   }
   *lpl = 0;
}

typedef  struct   {
   BYTE  bCnt;
   WORD  wOff;
   BYTE  bNul;
}CISADGRP;
typedef CISADGRP * LPCISADGRP;

int	chkcis( LPDFSTR lpdf )
{
   int   iRet = 1;
   HFILE hf;
   LPTSTR fn;
   BYTE * lpb;
   DWORD rd;
   DWORD fsiz;

   LPCISADFILE   lpf;
   LPCISADENTRY   lpae;
   LPCISADITEM    lpai;
   WORD     wCnt;
   WORD     wNum, wI, wNxt;
   DWORD    dwi, dwk, dwl;
   LPTSTR    lps;
   DWORD    dwj;
   BYTE     b;
   BOOL     bGrp;
   LPTSTR   lpout = &g_Stg[0];

   if( ( hf = (HFILE)lpdf->hf ) &&
      ( fn = lpdf->fn ) &&
      ( lpb = lpdf->lpb ) &&
      ( rd = lpdf->dwrd ) &&
      ( fsiz = lpdf->qwSize.LowPart ) &&
      ( chkcis2(lpb,rd ) ) )
   {
      lpf = (LPCISADFILE) lpb;
//   if( ( lpf->hdr.bV02 == 2 ) &&
      wCnt = lpf->hdr.wCnt;
      lpae = &lpf->ent[0]; // start at first ENTRY
      bGrp = (lpf->ent[0].wNum & 0x8000);
      wNum = (lpf->ent[0].wNum & 0x7fff);
      dwi = lpf->ent[0].sEntry.bLen;
      lps = &lpf->ent[0].sEntry.bDat[0];
      dwl = sizeof(CISADHDR);
      IniLine();

      for( wI = 0; wI < wCnt; wI++ )
      {
         dwk = 0; // There are three pieces for each
         // ========================================
         // 1 - collect three parts of an entry
         while(dwi)
         {
            for( dwj = 0; dwj < dwi; dwj++ )
            {
               b = lps[dwj];
               // condition
               if( ( b < ' ' ) ||
                  ( b >= 0x7f ) )
                  b = '.';

               lpout[dwj] = b;
            }

            lpout[dwj] = 0;

            if( dwk )
            {
               AddLine( lpout, ( dwk + 1 ) );
            }
            else
            {
               BgnLine( lpout ); // assumed column 1
            }

            // done entry - move to next
            lps += dwi;
            lpai = (LPCISADITEM)lps;
            dwi = lpai->bLen;
            if( dwi == 0 )
            {
               // ==================================
//               chkme( "Break on item missing ..." );
               // ==================================
               lps = &lpai->bDat[0];
               break;
            }

            dwk++;
            if( bGrp )
            {
               LPCISADGRP lpg = (LPCISADGRP)lps;
               WORD *     lpw;
               BYTE        bc;

               bc  = lpg->bCnt;
               sprintf(lpout, "  {group:%d}", (bc & 0xff) );
               AddLine(lpout,dwk);
               lpw = &lpg->wOff;
               while(bc)
               {
                  lpw++;
                  bc--;
               }
               lps = (BYTE *)lpw;
               lps++;
               break;
            }
            else
            {
               if( dwk >= CISECNT)  // = 3
               {
                  // that ENDS this item
                  break;
               }
            }
            // stay for CISECNT (three) items
            lps = &lpai->bDat[0];
         }

         if( VERB9 )
         {
            sprintf(lpout,"  [Item %d of %d]",
               (wNum & 0xffff),
               (wCnt & 0xffff) );
            AddLine(lpout, (DWORD)-1 );
         }

         // output this line of information
         // ===============================
         EndLine();
         // ===============================

         if( wNum == wCnt )
         {
            // successful end of list ...
         }
         else
         {
            // try cycling to the next item
            lpae = (LPCISADENTRY)lps;
            wNxt = (lpae->wNum & 0x7fff);
            bGrp = (lpae->wNum & 0x8000);
            wNum++;  // up to next number
            if( wNum == wNxt )
            {
               // we are continuing ...
               dwi = lpae->sEntry.bLen;
               lps = &lpae->sEntry.bDat[0];
            }
            else  // if( wNum < wCnt )
            {

//            chkme( "The file failed to do as expected ..." );
                  chkme( "File failed to do as expected ... Cnt=%d %02x %02x %02x",
                     (wNum &0xffff),
                     ( lps[0] & 0xff ),
                     ( lps[1] & 0xff ),
                     ( lps[2] & 0xff ) );
               break;
            }

         }

      }  // we fall out when completed the count

      if( VERB1 )
      {
         if( wI == wCnt )
         {
            sprintf(lpout,
               "Successfully process %d entries in a CIS ADDRBOOK.DAT file ...", wCnt );
            BgnLine(lpout);
            EndLine();
         }
      }

      // ZERO is SUCCESS
      iRet = (wI - wCnt);

   }
   return iRet;
}


// eof - DumpCis.c
