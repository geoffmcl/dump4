
// Intel.h

#ifndef	_Intel_HH
#define	_Intel_HH

typedef struct tagINTEL1 {
   DWORD    dwOp1;
   DWORD    dwOp2;
   DWORD    dwPre;
   LPTSTR   pReg;
   LPTSTR   pNm;
   LPTSTR   pTail;
   DWORD    dwRes1;
   DWORD    dwRes2;
   DWORD    dwRes3;
}INTEL1, * PINTEL1;

typedef  struct {
   DWORD  dwCnt;
   INTEL1 * pSet;
}INTELS, * PINTELS;

#define  NO_OP2   (DWORD)-1
#define  NO_PRE   0

extern	INTEL1	sIntel16[];	// the big table
extern	INTEL1	sIntel32[];	// the big table

extern   INTELS   spSorted[];
extern   TCHAR    szNul[];

#endif	// _Intel_HH
// eof - Intel.h

