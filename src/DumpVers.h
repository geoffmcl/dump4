// DumpVers.h
#ifndef  _DumpVers_H
#define  _DumpVers_H

// HISTORY upwards:
// 20170328 - From version 5.0.0 of 2017.03.28 the DUMP4_VERSION and DUMP4_DATE come from 'version.txt', through CMake parsing
// BLDDATE	"23 Mar, 2017"	// FIX20170323 - Use msvc140, and try to fix -exe
// BLDDATE	"10 Sep, 2015"	// FIX20150910 - Put in github repo, bgn unix port

// BLDDATE	"21 April, 2015"	// FIX20150421 - Fix for BITMAPV5HEADER
// BLDDATE	"21 August, 2014"	// FIX20140821 - Tidy up the source ready to push to a repo

// #define	BLDDATE	"14 June, 2014"	// FIX20140614 - Try to tidy up the .lib display
// as development of x64 apps and libraries increases, is important to know the machine type 386 vs x64, etc
// BLDTIME "18:13:11"

// BLDDATE	"17 August, 2013"	// FIX20130817 - More work on ELF, but still not complete - borrowed from beye project
// BLDTIME "17:01:59"
// BLDDATE		"25 February, 2013"	// FIX20130225 - Output a final import DLL list
//	BLDDATE		"13 April, 2012"	// FIX20120413 - commence mapping for LARGE files
// 20120416: put WARNING - sprtf( "WARNING: Aborting on fully BLANK section #%d!\n", i ); only if VERB9
#undef USE_64BIT_SIZE      // UGH - this looks like a MASSIVE change

// BLDDATE		"8 Febuary, 2012"	// FIX20120208 - Skip if count 0xFFFF in object
//	BLDDATE		"1 October, 2011"	// FIX20111001 - Seek .drectve, and SHOW like 
//    Linker Directives like /DEFAULTLIB:"LIBCMT" to determine RUNTIME type
// Section Hex Dumps Sections - count = 125
// section 01 (.drectve)  size: 000000C2  file offs: 0000139C


// BLDDATE		"8 September, 2011"	// FIX20110908 - Add ELF decode, BUT NOT COMPLETED
#define BUILTIN_ELF
#define ADD_ELF_SUPPORT

//	BLDDATE		"30 March, 2011"	// 19.zip - Some tweeks to sprtf to handle very
// LONG format,args by allocating memory, using int len = _vscprintf( ps, arglist )
// AND if -v2 and up, then using the DbgHlp.[h|lib] UnDecorateSymbolName() to
// show the un-mangled name of '?functions' after the lib name, in [ ]...
// Modified DumpFile.c (sprtf()), and DumpPE.cxx (DumpFirstLinkerMember())
// BLDDATE		"28 March, 2011"	// 19.zip Added -exe:X, to just show first table

// 20100527 - add -exe:F (fDumpFollowImports)
// #ifdef ADD_EXE_FOLLOW
// " F = follow import trail, using PATH to find, and dump imported DLLs: def = OFF."MEOR
// BLDDATE		"27 April, 2010"	// Added -exe:F, to follow DLL include trail
#define ADD_EXE_FOLLOW

// 2010-02-16 - add -sonic to DUMP a sonic DVD creator file
// BLDDATE		"16 February, 2010"	// Added -sonic to process Sonic DVD project file
#define ADD_SONIC_PROJECT

    // 20091112 - add -aa to restrict to alphanumeric output
//	BLDDATE		"12 November, 2009"	// Added -aa to process only alphanumeric

// BLDDATE		"28 September, 2009"	// Added -avi to process AVI files
#define ADD_AVI_FILE

// BLDDATE		"13 August, 2009"	// Added -m2ts to process HD Video file
// BLDDATE		"04 Dec, 2008"	// FIX20081204 - Added -wav to process WAVE file
// BLDDATE		"12 Sep, 2008"	// FIX20080912 - More work on -tar 
// lots of code from tar 1.20 source, which helped in understanding the TAR formats
// most of it is useless for DUMP4, but has been left in this source

// BLDDATE		"08 Aug, 2008"	// FIX20080908 - Add -tar - dump TAR format files
#define  ADD_TAR_FILE

// BLDDATE		"07 May, 2008"	// FIX20080507 - Add DumpPE (PEDUMP) for exe,lib.dll,obj
#ifdef _WIN32
#define  USE_PEDUMP_CODE
#else   // _WIN32
// PE code if not ported
// #undef USE_PEDUMP_CODE
#endif

// BLDDATE		"25 March, 2008"	// Add -shp, for shapefile dump
//#define  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
#undef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib - 2011-09-08 OFF this for now

// BLDDATE		"02 February, 2008"	// Add -blocks:nn[xn]:nn[xn]...
// and -o:n, output offset as number, not hex
#define  ADD_BLOCK_CMD
// BLDDATE		"30 December, 2007"	// fix for -bmp when file size too small
// now aborts show of bitmap, and reverts to standard HEX dump ...
// Also fix for BI_BITFIELDS with 3 DWORD colour MASK, lpbmfh->bfOffBits seemingly
// NOT exactly correct when lpbih->biCompression == BI_BITFIELDS!!!

// BLDDATE		"29 October, 2007"	// added -lib and improved -obj.
#define  ASCII_MIN   5

//	BLDDATE		"23 July, 2007"	// add -a[n] ASCII only dump. n is min string.
// ASCII_MIN   4
//	BLDDATE		"28 February, 2007"	// add -bmp2 to dump bitmap colors (MSVC8)
// *** BUT ONLY IF 24-BPP ***
// and added a rough :br=nn:bc=nn:er=nn:ec=nn for begin and end row and column
// ===========================================================================

//	BLDDATE		"13 November, 2006"	// compile in Dell01, using MSVC8
#define ADDLNK  // and add the dumping of a Microsoft LNK (shortcut) file
//	BLDDATE		"9 October, 2004"	// compile in PRO-1
//		"23 July, 2003"	// add -rgb:out.bmp to dump of an RGB file, out to bmp
//	"1 July, 2003"	// add -gif[t][n][:|+[outname.bmp]] to dump a GIF file.
//	"21 August, 2002"	// add -bmp1 to LIMIT big outputs - only 1 & 2 imp'd
//	"16 August, 2002"	// fix -B63200000 - one day must fix offset > DWORD
//	BLDDATE		"9 February, 2002"	// investigae -g... failed!
//	BLDDATE		"20 August, 2001"	// add --xohu (Unicode aware) switch
// also swing over the using file mapping, rather than reading into a buffer
//BLDDATE		"6 June, 1997"	// Commenced program
// Another try at FIXING the kernel32 access violation which
// I think happens in the use of "printf" for OUTPUT, thus
// added GetStdHandle( STD_OUTPUT_HANDLE ) and use WriteFile()
// in its place. Also increased FILE I/O to 4096
//	BLDDATE		"6 July, 1997"	// Above addition
//	BLDDATE		"5th November, 1999"	// addition of
// BLDDATE		"4th July, 2000"	// addition of
//	BLDDATE		"28th July, 2000"	// addition of -CIS switch
//	BLDDATE		"20 August, 2000"	// search for 0dh,0ah pairs
//	BLDDATE		"3 September, 2000"	// add -CAB switch for DIRECTORY of a CAB file.
//	BLDDATE		"5 September, 2000"	// some changes in the -CAB switch
//	BLDDATE		"9 September, 2000"	// add -?? shows the -g[...] switch
//	BLDDATE		"14 October, 2000"	// add current DATE to As At entry (if found)
//	BLDDATE		"13 February, 2001"	// add -prof dump of SYNEDIT profile file
//	BLDDATE		"21 April, 2001"	// add -lib dump of archive file
#define   DUMP_RGB2  // -rgb - dump as SGI rgba file

#define  USEMAPPING  // use file mapping, rather than reading in the file
#define  ADDARCH2     // -lib - dump of ARCHIVE (*.lib) file.

#define  ADDSYNPROF
#define	F20000704   /* If just ASCII output, try to break lines neatly */
#define	ADDBMPSW	/* add -BMP switch, like dump3 */
#define  F20000820   /* seek FIXED files (for fun) */
#define  ADDOBJSW    // FIX20010731 - add -obj to dump as a COFF object file

#define	MXFILNAMES		200
#define	MXPTRS			256
#define	MXCMDBUF		   2048	// Wow, this is big!!!
#define	MXONELN			68    // just wrap a command

#define	MEOR		      "\r\n"

#define	EndBuf(a)		( a + lstrlen(a) )

#define  IsNChr(a)   ( ( a >= '0' ) && ( a <= '9' ) )
#define  IsUChr(a)   ( ( a >= 'A' ) && ( a <= 'Z' ) )
#define  IsLChr(a)   ( ( a >= 'a' ) && ( a <= 'z' ) )

#define  IsPChr(a)   ( IsNChr(a) || IsUChr(a) || IsLChr(a) )

// Some CONSTANTS
#ifndef  USEMAPPING
//	MXFIO			1024	// Max. File i/o
#define	MXFIO			4096	// Max. File i/o
#endif   // #ifndef USEMAPPING

#define	DEF_VERB		1	/* default verbosity */
#define  MXALINE     76

#define  VERB  ( giVerbose )
#define  VERB1 VERB
#define  VERB2 ( giVerbose > 1 )
#define  VERB3 ( giVerbose > 2 )
#define  VERB4 ( giVerbose > 3 )
#define  VERB5 ( giVerbose > 4 )
#define  VERB6 ( giVerbose > 5 )
#define  VERB7 ( giVerbose > 6 )
#define  VERB8 ( giVerbose > 7 )
#define  VERB9 ( giVerbose > 8 )
#define  VERBMAX  VERB9

#define  VH(a)       ( a && ( a != INVALID_HANDLE_VALUE ) )
// now 'older' style of a file handle
#define  VFHO(a)      ( a && ( a != HFILE_ERROR ) )

#if  (defined(_MSC_VER) && (_MSC_VER > 1300))
#pragma warning( disable:4996 )
#endif  // MSVC8 
#define WIN32_LEAN_AND_MEAN

#endif   // _DumpVers_H
// eof - DumpVers.h
