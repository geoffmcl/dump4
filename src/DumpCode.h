
// DumpCode.h
#ifndef  _DumpCode_h
#define  _DumpCode_h

extern VOID DumpCodeSection(PMWL pmwl, PBYTE pBegin, PBYTE pStgs,
                               PBYTE pCode, DWORD dwLen, LPTSTR lpfn );

extern VOID DumpCode(PMWL pmwl, PBYTE pBegin, PBYTE pStgs,
              PBYTE pCode, DWORD dwLen, LPTSTR lpfn, ProcType typ );

#endif // _DumpCode_h
// eof - DumpCode.h
