

// Cis.h

#ifndef	_CIS_H
#define	_CIS_H

#define	KEYHDR			"CIM KEYFILE"
#define	VISHDR			"VIS000"

#define	LCISKHDR		11
#define	LCISHDR			6
#define MXCSUB			11
#define MXCNAM			19

typedef struct {
        char    cSubDir[MXCSUB];
        char    cSubName[MXCNAM];
}CIMENT;
typedef CIMENT * PCIMENT;

typedef	struct	tagCISDATE {
	BYTE	cDay;
	BYTE	cMth;
	BYTE	cYr;
}CISDATE;
typedef CISDATE * PCISDATE;

typedef	struct	tagCISTD	{
	BYTE	tSec;
	BYTE	tMin;
	BYTE	tHr;
	CISDATE	tDate;
}CISTD;

typedef CISTD * PCISTD;

typedef struct  tagCISKHDR {     /* ch */
        BYTE	ch_Name[LCISKHDR];
		BYTE	ch_Stop;
		WORD    ch_Res1;
		WORD    ch_Count;               // Count of NAME + DIRECTORY
		DWORD   ch_FFFFFFFF;    // ????
}CISKHDR;
typedef CISKHDR * PCISKHDR;

typedef struct {
	BYTE    cf_Vis[LCISHDR];      // = "VIS000"
	BYTE    cf_V1a;         // = 0x1a
	BYTE    cf_Type;        // = "K"=ART "F"=MSG "J"=PLX
	BYTE    cf_Unk1[2];     // Unknown
	//BYTE	cf_sec;
	//BYTE	cf_Min;
	//BYTE	cf_Hr;
	//BYTE	cf_Day;
	//BYTE	cf_Mth;
	//BYTE	cf_Yr;
	CISTD	cf_TD1;
	//BYTE	cf_Sec2;
	//BYTE	cf_Min2;
	//BYTE	cf_Hr2;
	//BYTE	cf_Day2;
	//BYTE	cf_Mth2;
	//BYTE	cf_Yr2;
	CISTD	cf_TD2;
	BYTE    cf_Unk2[2];     // Unknown
	WORD    cf_Unk3;        // = 00 00
	//BYTE    cf_Len;         // Length
} CISF1;

typedef CISF1 FAR * PCISF1;

typedef	struct {
	BYTE	v2_Unk1;
	BYTE	v2_Unk2;
	CISTD	v2_TD1;
	CISTD	v2_TD2;
}VISHDR2;
typedef VISHDR2 * PVISHDR2;

// OUT STRUCTURE
typedef struct {
	//BYTE	vb_Unk1;	// 00
	//BYTE	vb_Unk2;	// 00
	//BYTE	vb_Unk3;	// 00
	BYTE	vb_AutoFile;	// 01=Yes 00=No
	BYTE	vb_Unk5;	// 00
	BYTE	vb_Unk6;	// 00
	BYTE	vb_Unk7;	// 00
	BYTE	vb_Unk8;	// 29
	BYTE	vb_Unk9;	// 00
	BYTE	vb_Unk10;	// 02
	BYTE	vb_Unk11;	// 00
	BYTE	vb_Unk12;	// 00
	BYTE	vb_Unk13;	// 00
	BYTE	vb_Importance;	// 01=Normal 00=Low 02=High
	BYTE	vb_Sensitive;	// 00=Normal 01=Personal 02=Private 03=Confidential
	BYTE	vb_Receipt;	// 00= No 01=Yes
	BYTE	vb_Unk17;	// 00
	BYTE	vb_Payment;	// 08  0x40=Shared 0x80=Receiver 0x01=Rel.Date
				//	0x02=Exp.Date
	BYTE	vb_Unk19;	//
	BYTE	vb_Unk20;
	BYTE	vb_Unk21;
	//BYTE	vb_RelDay;	// 1C 06 1B =
	//BYTE	vb_RelMth;	// 28/06/97
	//BYTE	vb_RelYr;	// NOTE: 0x01 in vb_Payment for Valid Date
	CISDATE	vb_RelDate;
	BYTE	vb_Unk25;
	BYTE	vb_Unk26;
	BYTE	vb_Unk27;
	//BYTE	vb_ExpDay;	// 1C 06 1B =
	//BYTE	vb_ExpMth;	// 28/06/97
	//BYTE	vb_ExpYr;	// NOTE: 0x02 in vb_Payment for Valid Date
	CISDATE	vb_ExpDate;
	BYTE	vb_Unk31;	// 00
	BYTE	vb_Attach;	// 01=None 02=Attached
	BYTE	vb_Unk33;	// 01
	WORD	vb_TxtLen;	// Length of TEXT content (incl @b, etc)
	BYTE	vb_Unk36;
	BYTE	vb_Unk37;
	BYTE	vb_Unk38;
	BYTE	vb_Unk39;
	BYTE	vb_Unk40;	// 05
	BYTE	vb_Unk41;
	BYTE	vb_Unk42;
	BYTE	vb_Unk43;
	BYTE	vb_Unk44;
	BYTE	vb_Unk45;
	BYTE	vb_Attach2;	// 02
	WORD	vb_AttLen;	// 00 1C for example
	BYTE	vb_Unk49;
	BYTE	vb_Unk50;
	// Text starts - If NO attachment
	// If attachement then
	// 00 or Len,"Description"
	// followed by Len,"FileName"
	// Then 03 00 00 00 00
	// Where 0x03=Binary 0x04=Text 0x02=GIF 0x06=JPEG
	// Len(1C),"Path Name"
	// Plus 00 00 00 00 00
	// THEN TEXT
}VISBLK;
typedef VISBLK FAR * PVISBLK;

#endif	// _CIS_H
// eof - Cis.h
