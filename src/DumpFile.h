

// DumpFile.h
#ifndef	_DumpFile_H
#define	_DumpFile_H

#ifdef __cplusplus
extern "C" {
#endif

// Description: DIRECT output to the various channels
//              WITHOUT end-of-line checking.
///////////////////////////////////////////////////////////////////////////////
extern	VOID  doutput( LPTSTR lps );
extern BOOL  grmCloseFile( HANDLE * ph );
extern void prt( LPTSTR lps );  // output raw, with line ending check

#ifdef __cplusplus
}
#endif

#endif	// _DumpFile_H
// eof - DUmpFile.h