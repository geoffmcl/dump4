
// DumpUtil.h
#ifndef	_DumpUtil_H
#define	_DumpUtil_H

extern int wait_keyin( void ); // get key getkey wait key waitkey
extern LPTSTR   GetNxtBuf( void );
extern BOOL IsStaticBuffer(LPTSTR buf);

extern	LPTSTR	GetShortName( LPTSTR lps, DWORD dwmx );
extern   INT      GetHEXString( LPTSTR lpd, PBYTE pb, INT iLen, PBYTE pBgn, BOOL fill );
extern LPTSTR GetHEXOffset( DWORD dwo );    // convert offset to ????:???? (in hex)

extern   void     AddStringandAdjust( LPTSTR lpd );
extern   INT      AppendASCII( LPTSTR lpd, PBYTE pb, INT iLen );

extern   void     InitASMFile( void );
extern   void     CloseASMFile(void);
extern   void     CreateASMFile( void );
extern   void     WriteASMFile( LPTSTR lps );
extern   INT      strbgn( LPTSTR lpb, LPTSTR lps );
extern   INT      strbgni( LPTSTR lpb, LPTSTR lps );
extern   void     AddASMString( LPTSTR lps, DWORD dwo );
extern   void     Write2ASMFile(LPTSTR lps, DWORD dwo);

extern   INT      InStr( LPTSTR lpb, LPTSTR lps );
extern   LPTSTR   Left( LPTSTR lpl, DWORD dwi );
extern   LPTSTR   Right( LPTSTR lpl, DWORD dwl );

#define  ISNUMERIC(a)      ( ( a >= '0' ) && ( a <= '9' ) )

#define  SetMinLen(a,b) while( strlen(a) < b ) strcat(a," ")
#define  MMXNAME     32
#define  MINHDSZ        24
//#define  MINHDSZ        20
#define  MINSHDSZ       12

#define  MINOFF2        32
#define  MINOPR         12
#define  MINOF2C        (MINOPR + 12)

extern   LPTSTR   Mid( LPTSTR lpl, DWORD dwb, DWORD dwl );

extern double get_percent2( DWORD max, DWORD amt );
#ifdef WIN32
extern BOOL Get_FD_File_Time_Stg( PTSTR pDest, WIN32_FIND_DATA * pfd, BOOL bLocal );
#endif
extern PTSTR My_NiceNumberStg( DWORD num );
extern PTSTR My_NiceNumber( PTSTR lpn );

#endif	// #ifndef	_DumpUtil_H
// eof - DumpUtil.h

