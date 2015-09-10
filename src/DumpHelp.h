
// DumpHelp.h
#ifndef  _DumpHelp_H
#define  _DumpHelp_H

extern   void	ShwTitle( void );
extern   void	Usage( void );
extern   void	UsageX( void );
extern   void	Usage0( void );
extern   void	UsageG( void );
extern   int ProcessCommand( int argc, char *argv[], LPSTR lpc );

#ifdef USE_PEDUMP_CODE  // FIX20080507 - switch to PEDUMP code
extern void ShowCurrPEOpts( void );
#endif // #ifdef USE_PEDUMP_CODE  // FIX20080507 - switch to PEDUMP code

#endif   // _DumpHelp_H
// eof - DumpHelp.h
