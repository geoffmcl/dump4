

// DumpWork.h
#ifndef  _DumpWork_H
#define  _DumpWork_H

#define	MXLINE			16
#define  MXOUTBUF       (1024*16)
#define	MXDIAGBUF		1024	// For DIAG output in Dumpcis.c

#ifndef  USEMAPPING
// these are NOT needed if using MAPPING
#define  MMXFIO         4096
#define  MMXALLOC       (1024*64)

#endif   // #ifndef  USEMAPPING


#define  MXLINES     256        // 20090930 - was a mean 16
#define  MXLINEB     256
#define  MXLINEB2    (MXLINEB + 16)

// -g(eoff) tele list constants
#define  NMLEN          24
#define  NMMX2C         45
#define  MMXBU          10
#define  MXNUMSET       24

// g_BmpBgnEnd[4]
typedef enum {
   bgn_row,
   bgn_col,
   end_row,
   end_col,
   BE_MAX
} BGNEND;

typedef struct tagHEXBLOCK {
   DWORD num;
   DWORD cnt;
}HEXBLOCK, * PHEXBLOCK;

typedef struct tagWB {

   INT      w_iVerbose;          // giVerbose - verbosity level (Def = 1)
   INT      w_iInCount;
   BOOL     w_fAddOffset;        // add the file offset to the output
   BOOL	   w_fAddHex;
   BOOL     w_fUnicode;    // gfUnicode   // be UNICODE aware
   BOOL	   w_fAddAscii;
   BOOL	   w_fDoBMP;            // gfDoBMP - process as BITMAP file
   BOOL	   w_fDoWAV;            // gfDoWAV - process as WAVE file
   BOOL     w_fDoASCII;    // gfDoASCII - -a[n] find only ASCII of min length n
   BOOL     w_fDoASCII2;   // gfDoASCII2 -aa = find only alphanumeric
   INT      w_iMinASCII;   // giMinASCII - min length - default = ASCII_MIN
   INT      w_iBmpVerb;    // giBmpVerb    // verbosity of BITMAP output
   BGNEND   w_BgnEnd[BE_MAX]; // g_BmpBgnEnd
   RGBQUAD  w_sBmpColour[256];  // gsBmpColour
   DWORD    w_dwColCount[256];   // gdwColCount

   BOOL     w_fDoCISAddr;        // gfDoCISAddr - process as CIS addressbook file
   BOOL     w_fDoCABFile;        // gfDoCABFile - process as MS CAB file
   BOOL     w_fDoSHPFile;  // gfDoSHPFile - process as a SHAPEFILE ...
   BOOL     w_fDoTARFile;  // gfDoTARFile - process as a TAR file ...
   BOOL	   w_fRemPar;
   BOOL	   w_fUsePrintf;
   BOOL	   w_fDnUseP;
   BOOL     w_bDoGeoff;
   BOOL     w_fDoVosTab;
   BOOL     w_fDoVosOnly;
   BOOL  w_fDoDFS;   // gfDoDFS - Private DATASET

   BOOL     w_bDoExtra; // gbDoExtra = -g[x[0|-|+|1]]
   BOOL     w_bChkPWD;  // gbChkPWD = TRUE; // -gp[0|1|+|-]
   BOOL     w_bIgnoreMNU;  // gbIgnoreMNU  = TRUE; -gi[...] ignore 123ABC
   BOOL     w_bUseSee;     // gbUseSee     = FALSE; -gs[.] use SEE in place of ...
   BOOL     w_bDumpGif; // g_bDumpGif from -gif[n];
   BOOL     w_bDumpRGB; // g_bDumpRGB from -rgb ...

   BOOL     w_bDumpMK4;    // g_bDumpMK4
   DWORD	   w_dwBgnOff;          // gdwBgnOff - begin file processing here (def=0)
   DWORD	   w_dwEndOff;          // gdwEndOff - end file processing here

   DFSTR    w_sDoFil;               // gsDoFil
   TCHAR    w_szALine[MXALINE+8];   // gszALine
   HANDLE	w_hStdOut;
   BOOL     w_bGotWild;   // GotWild(fn);

   BOOL     w_bDumpLib;    // g_bDumpLib
   BOOL     w_bDumpLNK;    // g_bDumpLNK
   BOOL     w_bDumpObj;    // g_bDumpObj -obj switch
   BOOL     w_bFullObj;    // g_bFullObj

   LPTSTR   w_lpOut;       // glpOut = &gcOutBuf[0];
   LPTSTR   w_lpOut2;      // glpOut2 = &gcOutBuf2[0];
   DWORD    w_dwOutSz;     // gdwOutSz = MXOUTBUF;

   TCHAR    w_szTmpOut[1024];
   TCHAR    w_szNxtFile[MAX_PATH+8];
   TCHAR    w_szCurDir[MAX_PATH+8];
   TCHAR    w_szOffset[32];
   TCHAR    w_szHex[(MXLINE * 3) + 16];
   TCHAR    w_szAscii[MXLINE + 16];
   TCHAR    w_szcwd[MAX_PATH+8];	/* for _getcwd( lpb, 256 ); */
   TCHAR    w_szModule[MAX_PATH+8]; // gszModule - current runtime module

   TCHAR    w_szBmpNm[264];   // g_szBmpNm

#ifndef  USEMAPPING
   TCHAR    w_cFilBuf[MXFIO+8];  // gcFilBuf - File buffer
#endif   // #ifndef  USEMAPPING

   LPTSTR   w_pFilNames[MXFILNAMES];
   TCHAR    w_cGOutFile[264];    // gcGOutFile

   BOOL     w_bAppend;  // g_bAppend
   TCHAR    w_szOutFile[264];    // g_szOutFile
   HANDLE   w_hOutFile;          // g_hOutFile

   // CAB STUFF
   BOOL	   w_bShowInfo;      // gbShowInfo
   BOOL     w_fCopyAll;       // gfCopyAll
   BOOL     w_fCopyNone;      // gfCopyNone
   DWORD    w_dwCabSize;      // gdwCabSize
   DWORD	   w_dwTotSize;      // gdwTotSize
   DWORD	   w_dwComplete;     // gdwComplete
   DWORD	   w_dwInPrev;       // gdwInPrev
   DWORD	   w_dwInNext;       // gdwInNext
   DWORD	   w_dwInfoCnt;      // gdwInfoCnt
   BOOL     w_bHasPrev;   // gbHasPrev = fdici.hasprev; // TRUE ? "yes" : "no",
   BOOL     w_bHasNext;   // gbHasNext = fdici.hasnext; // TRUE ? "yes" : "no" );
   /* Destination directory for extracted files */
   TCHAR    w_szdest_dir[264];   // gszdest_dir
  
   TCHAR    w_cPrevCab[264];   // gcPrevCab =,pfdin->psz2);
   TCHAR    w_cPrevPath[264]; // gcPrevPath
   TCHAR    w_cPrevDisk[264];  // gcPrevDisk =,pfdin->psz3);
   TCHAR    w_cPrevFull[264]; // gcPrevFull
   TCHAR    w_cPrevCab2[264];  // gcPrevCab2 =,pfdin->psz2);
   TCHAR    w_cNextCab[264];  // gcNextCab
   TCHAR    w_cNextDisk[264]; // gcNextDisk
   TCHAR    w_cNextCab2[264]; // gcNextCab2

   // -g(eoff) tele list stuff
   BOOL     w_bGotBU1, w_bGotCU;   // (BOOL)chkbu( lpb, &dwo );
   BOOL     w_bDnCompile;     // gbDnCompile
   BOOL     w_bNotSame; // gbNotSame - I IF def CHKSAMELET =chkme( "Same letter inhibit is ON ..." );
   BOOL     w_bAddComp; // gbAddComp    = FALSE;
   INT      w_iLnCnt;   // giLnCnt -  count 1, 2 ... max 3 until code added
   TCHAR    w_cHeadOrg[ (NMMX2C + 15) ];   // gcHeadOrg - Orginal
   TCHAR    w_cHeadX1[ (NMMX2C + 15) ];    // gcHeadX1 - Last name rotation
   TCHAR    w_cHeadX2[ (NMMX2C + 15) ];    // cHeadX2 - Split of last if hythenated
   // Split of information when listed say as "Annie & Geoff", then sort
   // will catch BOTH names
   TCHAR    w_cHead1[ (NMMX2C + 15) ]; // gcHead1
   TCHAR    w_cHead2[ (NMMX2C + 15) ]; // gcHead2
   TCHAR    w_cTailSee[ ((NMMX2C*2) + 15) ]; // gcTailSee
   TCHAR    w_cNum[MXNUMSET+8];   // gcNum - could be attached to TAIL idea

   // dump of BMP
   BITMAPFILEHEADER  w_bmfh;     // g_bmfh
   //BITMAPINFOHEADER  w_bih;      // g_bih
   BITMAPV5HEADER    w_bih;      // g_bih - v5 header is 128 versus 40
   RGBQUAD           w_ct[256];  // g_ct
   TCHAR             w_szDiag[MXDIAGBUF+8];   // gszDiag

   // in dumpcis.c
   TCHAR    w_cDiagBuf[MXDIAGBUF+8];   // gcDiagBuf
   TCHAR    w_Stg[256+8];  // g_Stg
   TCHAR    w_Out[256*4];  // g_Out

   PVOID    w_pSymTbl;  // g_pSymTbl // P1 - save the SYMBOL TABLE pointer for later use
   DWORD    w_dwSymCnt; // g_swSymCnt   // P1 - save the SYMBOL TABLE count for later use
   PVOID    w_pMchRel;  // g_pMchRel   // P1 - save the TYPE pointer for later use

   // SYNEDIT file
   BOOL     w_bDoSynEdit;  // gbDoSynEdit
   BOOL     w_bDoPPM;      // g_bDoPPM

   // M2TS file
   BOOL     w_bDoM2TS;  // g_bDoM2TS
   BOOL     w_bDoAVI;   // g_bDoAVI - process as an AVI file
   BOOL w_fDoSonic; // gfDoSonic - 2010-02-16 - add -sonic DVD project file

   //DWORD * w_dwBlkSizes;   // gdwBlkSizes
   //DWORD * w_dwBlkCnts;    // gdwBlkCnts
   PHEXBLOCK w_pHexBlock;  // g_pHexBlock
   int w_iNumbOffset; // g_iNumbOffset
   int w_DoAsDWORDS; // g_DoAsDWORDS
   int w_SwapBytes;  // g_SwapBytes

#ifndef  USEMAPPING
   BYTE     w_bfilbuf[MMXFIO+8];       // g_bfilbuf
#endif   // !USEMAPPING

   INT      w_iLnBuf;      // giLnBuf
   TCHAR    w_szLnBuf[(MXLINEB2 * MXLINES)]; // gszLnBuf

   char w_dwordbuf[64]; // gdwordbuf

   BYTE     w_cBuf[1048];  // g_cBuf
   BYTE     w_cBuf2[1048];  // g_cBuf2
   BYTE     w_cBuf3[1048];  // g_cBuf3
   BYTE     w_bBuf[1048];  // m_bBuf
   BYTE     w_cTmp[1048];  // g_cTmp

   TCHAR    w_szCmd[MXCMDBUF+260];       // gw_szCmd	//lpc = malloc( (MXCMDBUF+260) );

   TCHAR    w_cOutBuf[MXOUTBUF + 16];      // gcOutBuf
   TCHAR    w_cOutBuf2[MXOUTBUF + 16];     // gcOutBuf2

}WB, * PWB;

extern   PWB   pWB;
#define  W  (*pWB)

#define  giVerbose   W.w_iVerbose   // verbosity level (def = 1)
#define  giInCount   W.w_iInCount   // input file count
#define  gfAddOffset W.w_fAddOffset // add the file offset to the output
#define  gfAddHex    W.w_fAddHex    // add the HEX
#define  gfUnicode   W.w_fUnicode   // be UNICODE aware
#define  gfAddAscii  W.w_fAddAscii  // add the ASCII
#define  gfDoBMP     W.w_fDoBMP     // process as BITMAP file
#define  giBmpVerb   W.w_iBmpVerb   // verbosity of BITMAP output
#define  gfDoWAV     W.w_fDoWAV     // process as WAVE file

#define  gfDoCISAddr W.w_fDoCISAddr // process as CIS addressbook file
#define  gfDoCABFile W.w_fDoCABFile // process as MS CAB file

#define  gfRemPar    W.w_fRemPar

#define  gw_fUsePrintf W.w_fUsePrintf
#define  gw_fDnUseP    W.w_fDnUseP
#define  gw_hStdOut    W.w_hStdOut

#define  gbDoGeoff   W.w_bDoGeoff
#define  gfDoVosTab  W.w_fDoVosTab
#define  gfDoVosOnly W.w_fDoVosOnly

#define  gbDoExtra   W.w_bDoExtra   // = TRUE  -g[x[0|-|+|1]]
#define  gbChkPWD    W.w_bChkPWD    // = TRUE  -gp[0|1|+|-]
#define  gbIgnoreMNU W.w_bIgnoreMNU // = TRUE  -gi[...] ignore 123ABC
#define  gbUseSee    W.w_bUseSee    // = FALSE -gs[.] use SEE in place of ...

#define  gdwBgnOff   W.w_dwBgnOff   // begin file processing here (def=0)
#define  gdwEndOff   W.w_dwEndOff   // end file processing here

#define  gsDoFil     W.w_sDoFil
#define  gszALine    W.w_szALine    // [MXALINE+8]

#define  gbGotWild   W.w_bGotWild   // GotWild(fn);

#define  gszTmpOut   W.w_szTmpOut   // [1024]
#define  gszNxtFile  W.w_szNxtFile  // [MAX_PATH+8]
#define  gszCurDir   W.w_szCurDir   // [MAX_PATH+8]
#define  gszOffset   W.w_szOffset   // [32];
#define  gszHex      W.w_szHex      // [(MXLINE * 3) + 16];
#define  gszAscii    W.w_szAscii    // [MXLINE + 16];
#define  gw_szcwd      W.w_szcwd      // [MAX_PATH+8];	/* for _getcwd( lpb, 256 ); */
#define  gszModule   W.w_szModule   // current runtime module

#ifndef  USEMAPPING
#define  gcFilBuf    W.w_cFilBuf    // [MXFIO+8] - File Buffer
#endif   // #ifndef  USEMAPPING

#define  gpFilNames  W.w_pFilNames  // [MXFILNAMES]
#define  gcGOutFile  W.w_cGOutFile

// CAB STUFF
#define  gbShowInfo     W.w_bShowInfo
#define  gfCopyAll      W.w_fCopyAll
#define  gfCopyNone     W.w_fCopyNone
#define  gdwCabSize     W.w_dwCabSize
#define  gdwTotSize     W.w_dwTotSize
#define  gdwComplete    W.w_dwComplete
#define  gdwInPrev      W.w_dwInPrev
#define  gdwInNext      W.w_dwInNext
#define  gdwInfoCnt     W.w_dwInfoCnt
#define  gbHasPrev      W.w_bHasPrev   // = fdici.hasprev; // TRUE ? "yes" : "no"
#define  gbHasNext      W.w_bHasNext    // = fdici.hasnext; // TRUE ? "yes" : "no"
   /* Destination directory for extracted files */
#define  gszdest_dir    W.w_szdest_dir
   
#define  gcPrevCab      W.w_cPrevCab   // =,pfdin->psz2);
#define  gcPrevPath     W.w_cPrevPath
#define  gcPrevDisk     W.w_cPrevDisk  // =,pfdin->psz3);
#define  gcPrevFull     W.w_cPrevFull
#define  gcPrevCab2     W.w_cPrevCab2  // =,pfdin->psz2);
#define  gcNextCab      W.w_cNextCab
#define  gcNextDisk     W.w_cNextDisk
#define  gcNextCab2     W.w_cNextCab2
// =========

#define  gcOutBuf       W.w_cOutBuf    // [16K]
#define  gcOutBuf2      W.w_cOutBuf2   // [16K]

#define  glpOut         W.w_lpOut      // = &gcOutBuf[0];
#define  glpOut2        W.w_lpOut2     // = &gcOutBuf2[0];
#define  gdwOutSz       W.w_dwOutSz    // = MXOUTBUF;

   // -g(eoff) tele list stuff
#define  gbGotBU1       W.w_bGotBU1
#define  gbGotCU        W.w_bGotCU     // (BOOL)chkbu( lpb, &dwo );
#define  gbDnCompile    W.w_bDnCompile
#define  gbNotSame      W.w_bNotSame   //IF def CHKSAMELET =chkme( "Same letter inhibit is ON ..." );
#define  gbAddComp      W.w_bAddComp
#define  giLnCnt        W.w_iLnCnt  // count 1, 2 ... max 3 until code added
#define  gcHeadOrg      W.w_cHeadOrg   // Orginal
#define  gcHeadX1       W.w_cHeadX1    // Last name rotation
#define  gcHeadX2       W.w_cHeadX2    // Split of last if hythenated
   // Split of information when listed say as "Annie & Geoff", then sort
   // will catch BOTH names
#define  gcHead1        W.w_cHead1
#define  gcHead2        W.w_cHead2
#define  gcTailSee      W.w_cTailSee
#define  gcNum          W.w_cNum      // could be attached to TAIL idea

   // dump of BMP
#define  g_bmfh         W.w_bmfh
#define  g_bih          W.w_bih
#define  g_ct           W.w_ct

#define  gszDiag        W.w_szDiag     // [1024]

   // in dumpcis.c
#define  gcDiagBuf      W.w_cDiagBuf   // [MXDIAGBUF+8]
#define  g_Stg          W.w_Stg        // [256+8]
#define  g_Out          W.w_Out        // [256*4]

#ifndef  USEMAPPING
#define  g_bfilbuf      W.w_bfilbuf    // [MMXFIO+8]
#endif   // #ifndef  USEMAPPING

   // SYNEDIT "profile" file
#define  gbDoSynEdit    W.w_bDoSynEdit

#define  g_bDumpLib     W.w_bDumpLib
#define  g_bDumpLNK     W.w_bDumpLNK
#define  g_bDumpObj     W.w_bDumpObj
#define  g_bFullObj     W.w_bFullObj

#define  g_bAppend      W.w_bAppend
#define  g_szOutFile    W.w_szOutFile
#define  g_hOutFile     W.w_hOutFile

#define  giLnBuf        W.w_iLnBuf
#define  gszLnBuf       W.w_szLnBuf

#define  g_cBuf         W.w_cBuf
#define  g_cBuf2        W.w_cBuf2
#define  g_cBuf3        W.w_cBuf3
#define  g_bBuf         W.w_bBuf
#define  g_cTmp         W.w_cTmp

#define  g_pSymTbl      W.w_pSymTbl // P1 - save the SYMBOL TABLE pointer for later use
#define  g_dwSymCnt     W.w_dwSymCnt   // P1 - save the SYMBOL TABLE count for later use
#define  g_pMchRel      W.w_pMchRel   // P1 - save the TYPE pointer for later use

#define  gw_szCmd       W.w_szCmd	//lpc = malloc( (MXCMDBUF+260) );

#define  g_bDumpMK4     W.w_bDumpMK4

#define  g_bDoPPM       W.w_bDoPPM

#define  g_szBmpNm      W.w_szBmpNm

#define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]
#define gdwColCount W.w_dwColCount // g DWORD [256]

#define  g_bDumpGif     W.w_bDumpGif

#define g_bDumpRGB W.w_bDumpRGB // g_from -rgb ... BOOL
#define g_BmpBgnEnd   W.w_BgnEnd 


#define  gfDoASCII   W.w_fDoASCII   //  -a[n] find only ASCII of min length n
#define  gfDoASCII2  W.w_fDoASCII2   //  -a[a] find only alphanumeric
#define  giMinASCII  W.w_iMinASCII  // min length - default = ASCII_MIN

//#define gdwBlkSizes W.w_dwBlkSizes   // gdwBlkSizes
//#define gdwBlkCnts W.w_dwBlkCnts     // gdwBlkCnts
#define g_pHexBlock W.w_pHexBlock  // PHEXBLOCK g_pHexBlock
#define g_iNumbOffset W.w_iNumbOffset
#define gdwordbuf W.w_dwordbuf
#define g_DoAsDWORDS W.w_DoAsDWORDS
#define g_SwapBytes W.w_SwapBytes

#define  gfDoDFS  W.w_fDoDFS

#define  gfDoSHPFile W.w_fDoSHPFile  // process as a SHAPEFILE ...

#define  gfDoTARFile W.w_fDoTARFile  // process as a TAR file ...

#define g_bDoM2TS W.w_bDoM2TS   // process as a M2TS file

#define g_bDoAVI W.w_bDoAVI   // process as an AVI file

#define gfDoSonic W.w_fDoSonic // 2010-02-16 - add -sonic DVD project file


#endif   // _DumpWork_H
// eof - DumpWork.h
