// DumpPE.h
#ifndef _DumpPE_H_
#define _DumpPE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*SETFUNC)(BOOL);

extern BOOL Set_PEDUMP_A( BOOL flag ); // set ALL ON/OFF
extern BOOL Set_PEDUMP_B( BOOL flag ); // fShowRelocations
extern BOOL Set_PEDUMP_C( BOOL flag ); // fDumpSectionTable
extern BOOL Set_PEDUMP_D( BOOL flag ); // fDumpFileHeader
#ifdef ADD_EXE_FOLLOW
extern BOOL Set_PEDUMP_F( BOOL flag ); // 20100527 - add -exe:F (fDumpFollowImports)
#endif
extern BOOL Set_PEDUMP_G( BOOL flag ); // fDumpDebugDirectory
extern BOOL Set_PEDUMP_H( BOOL flag ); // fShowRawSectionData
extern BOOL Set_PEDUMP_I( BOOL flag ); // fShowIATentries
extern BOOL Set_PEDUMP_L( BOOL flag ); // fShowLineNumbers
extern BOOL Set_PEDUMP_M( BOOL flag ); // fDumpImportNames
extern BOOL Set_PEDUMP_O( BOOL flag ); // fDumpOptionalHeader
extern BOOL Set_PEDUMP_P( BOOL flag ); // fShowPDATA
extern BOOL Set_PEDUMP_R( BOOL flag ); // fShowResources
extern BOOL Set_PEDUMP_S( BOOL flag ); // fShowSymbolTable
extern BOOL Set_PEDUMP_T( BOOL flag ); // fDumpDataDirectory
extern BOOL Set_PEDUMP_X( BOOL flag ); // fDumpExportsOnly
extern void Set_Imports_Only( void ); // all OFF except 'M'
extern void Set_Exports_Only( void ); // all OFF except 'X'
extern char * Get_Current_Opts( void );

extern int DumpMemoryMap( LPVOID lpFileBase, PSTR filename, size_t nbytes );
extern unsigned long dwFileSizeLow;
extern unsigned long dwFileSizeHigh;
extern unsigned char * pBaseLoad;
extern unsigned char * pBaseTop;

extern void kill_ptrlist();
extern int out_of_top_range( unsigned char * ptr );

typedef struct tagDWORD_FLAG_DESCRIPTIONS {
    DWORD   flag;
    PSTR    name;
} DWORD_FLAG_DESCRIPTIONS, *PDWORD_FLAG_DESCRIPTIONS;

typedef struct tagWORD_FLAG_DESCRIPTIONS {
    WORD    flag;
    PSTR    name;
} WORD_FLAG_DESCRIPTIONS, *PWORD_FLAG_DESCRIPTIONS;

extern char * pedump_ctime( time_t * ptm );
extern DWORD ConvertBigEndian(DWORD bigEndian);
extern char * get_my_big_buf(void);
#define MX_BIG_BUF 16384
extern PSTR GetMachineTypeName( WORD wMachineType );
extern UINT getImageFlagCount();
extern PWORD_FLAG_DESCRIPTIONS getImageFlagStruct();

#ifdef __cplusplus
}
#endif

#endif // #ifndef _DumpPE_H_
// eof - DumpPE.h
