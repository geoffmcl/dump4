
// DumpObj.h
#ifndef	_DumpObj_H
#define	_DumpObj_H

typedef enum tagProcType {
   pt_Bo,
   pt_16,
   pt_32
}ProcType;

#define  m_pSymTbl   g_pSymTbl
#define  m_dwSymCnt  g_dwSymCnt

#ifndef USE_PEDUMP_CODE // FIX20080507

extern	BOOL  ProcessOBJ( LPDFSTR lpdf );

#ifdef  ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

extern   PIMAGE_RELOCATION GetRelName( LPTSTR lpb, PBYTE pRel, DWORD dwLen, PBYTE pStgs,
                                       DWORD dwo, DWORD dwFlag );

#else // !#ifdef  ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file
extern   PIMAGE_RELOCATION GetRelName( LPTSTR lpb, PBYTE pRel, DWORD dwLen, PBYTE pStgs, DWORD dwo );

#endif   // #ifdef  ADDOBJSW y/n   // FIX20010731 - add -obj to dump as a COFF object file

extern VOID  Show_Target_Machine( DWORD dwo );
extern VOID Show_Number_Sections( DWORD dwo );
extern VOID Show_DateTime_Stamp( DWORD dwi );
extern VOID  Show_Symbols_Count( DWORD dwi, DWORD dwo, PBYTE pHead, PBYTE * ppSym );
extern VOID Show_OH_Size( DWORD dwo );
extern VOID Show_Characteristics( DWORD dwi );
extern VOID  AppendSectionFlag( PTSTR ptmp, DWORD dwFlag );
#endif // #ifndef USE_PEDUMP_CODE // FIX20080507

#endif	// _DumpObj_H
// eof - DumpObj.h
