// DumpEXE.h
#ifndef  _DumpEXE_h_
#define  _DumpEXE_h_

extern int  IsEXEFile( LPDFSTR lpdf );
extern BOOL  DumpEXEFile( LPDFSTR lpdf );
extern VOID  Show_DOS_Header( PIMAGE_DOS_HEADER pdos, LPDFSTR lpdf );
extern PBYTE Word2Ascii( PBYTE pb );

#endif   // _DumpEXE_h_
// eof - DumpEXE.h
