

// Dump4Cab.h
#ifndef  _Dump4Cab_H
#define  _Dump4Cab_H

#ifdef WIN32
extern   BOOL  LoadCabLib( void );
extern   BOOL  FreeCabLib( void );
extern   BOOL  ProcessCAB( char * lpcab );
#endif


#endif   // _Dump4Cab_H
// eof - Dump4Cab.h
