

// DumpCode.c

#include	"Dump4.h"
// #include "DumpObj.h"

//#ifndef USE_PEDUMP_CODE // FIX20080507
#if !defined(USE_PEDUMP_CODE) && defined(_WIN32) // FIX20080507

extern   BOOL     bIsMS;   // = TRUE;
extern   INT GetSymName( LPTSTR lpb, PIMAGE_SYMBOL pSym, PBYTE pStgs, BOOL bAll );


// In this subset, an instruction has the following format:

// ====================================
// Group 1 - Lock and repeat prefixes:
#define  pre_LOCK    0X0F0    // LOCK.
#define  pre_REPNZ   0X0F2    // REPNE/REPNZ (used only with string instructions).
#define  pre_REP     0X0F3    // REP (use only with string instructions).
#define  pre_REPZ    0X0F3    // REPE/REPZ (use only with string instructions).
// Group 2 - Segment override prefixes:
#define  pre_CS      0X02E //CS segment override (use with any branch instruction is reserved).
#define  pre_SS      0X036 //SS segment override prefix (use with any branch instruction is reserved).
#define  pre_DS      0X03E //DS segment override prefix (use with any branch instruction is reserved).
#define  pre_ES      0X026 //ES segment override prefix (use with any branch instruction is reserved).
#define  pre_FS      0X064 //FS segment override prefix (use with any branch instruction is reserved).
#define  pre_GS      0X065 //GS segment override prefix (use with any branch instruction is reserved).
//- Branch hints:
#define  pre_HNBR    0X02E //Branch not taken (used only with Jcc instructions).
#define  pre_HBR     0X03E //Branch taken (used only with Jcc instructions).
// Group 3
#define  pre_SIZE    0X066 // Operand-size override prefix.
// Group 4
#define  pre_ADDR    0X067 // Address-size override prefix.

//typedef struct tagINTEL1 {
//   DWORD    dwOp1;
//   DWORD    dwOp2;
//   DWORD    dwPre;
//   LPTSTR   pReg;
//   LPTSTR   pNm;
//   LPTSTR   pTail;
//   DWORD    dwRes1;
//   DWORD    dwRes2;
//   DWORD    dwRes3;
//}INTEL1, * PINTEL1;

//#define  NO_OP2   (DWORD)-1
//#define  NO_PRE   0


//NOTES:
//* The moffs8, moffs16, and moffs32 operands specify a
//simple offset relative to the segment base, where
//8, 16, and 32 refer to the size of the data. The
//address-size attribute of the instruction determines
//the size of the offset, either 16 or 32 bits.

//       |       |       |         |              |                       |
//345678901234567892123456789312345678941234567895123456789612345678971234567898123456
// 0x033, NO_OP2, NO_PRE, "123456", "12345678901", "123456789012345678901" },
// See Opcode4.doc for these tables
//Prefixes    Opcode ModR/M SIB    Displacement Immediate
//Up to four  1 or 2 1 b    1 b    Address      Immediate
//prefixes of byte   (if    (if    displacement Data of
//1-byte each opcode  req.)  req.) of 1, 2, 4   1, 2, 4
//(optional)                       bytes or 0   bytes or 0
//
//       Mod R/M
//7 - 6 5  -  3 2  -  0
// Mod    Reg./   R/M
//       Opcode
#define  GETMOD(dwo)       (( dwo & 0x0C0 ) >> 6 )
#define  GETOP(dwo)        (( dwo & 0x038 ) >> 3 )
#define  GETRM(dwo)        (  dwo & 0x007        )

//         SIB
//7  -  6 5  -  3 2  -  0
// Scale   Index   Base
#define  GETSS(dwo)       (( dwo & 0x0C0 ) >> 6 )
#define  GETIND(dwo)      (( dwo & 0x038 ) >> 3 )
#define  GETBASE(dwo)     (  dwo & 0x007        )

//
// For example the two bytes 0x8b 0xec would be mov ebp, esp, VERY COMMON IN 'C/C++'
// From the above table 8B is listed as
//  { 0x08B, NO_OP2, NO_PRE, "/r", "MOV", "r16,r/m16" }, and
//  { 0x08B, NO_OP2, NO_PRE, "/r", "MOV", "r32,r/m32" },
// The /r denotes there is a ModR/M byte following
// If in 32-bit mode then (0xEC = 11101100b)
// From the 0EC the Mod = 11, the Reg = 101 , the R/M = 100
// Thus from table 2-2
// Mod 11 is a register, and R/M show it to be ESP
// The Reg = 5 which is EBP,
// So we have the instruction mov ebp, esp, savvy!

typedef struct tagMODRM2 {
   LPTSTR   pRM[8];
}MODRM2, * PMODRM2;

MODRM2 sModRM16[] = {
   // 0 Mod
   { "[BX+SI]" , "[BX+DI]" , "[BP+SI]" , "[BP+DI]",
      "[SI]" , "[DI]" , "disp16" , "[BX]" },
   // 1
   { "[BX+SI]+disp8" , "[BX+DI]+disp8" , "[BP+SI]+disp8" , "[BP+DI]+disp8",
      "[SI]+disp8" , "[DI]+disp8" , "[BP]+disp8", "[BX]+disp8" },
   // 2
   { "[BX+SI]+disp16" , "[BX+DI]+disp16" , "[BP+SI]+disp16", "[BP+DI]+disp16",
      "[SI]+disp16" , "[DI]+disp16" , "[BP]+disp16" , "[BX]+disp16" },
   // 3
   { "EAX/AX/AL/MM0/XMM0" , "ECX/CX/CL/MM1/XMM1" , "EDX/DX/DL/MM2/XMM2", "EBX/BX/BL/MM3/XMM3",
      "ESP/SP/AH/MM4/XMM4" , "EBP/BP/CH/MM5/XMM5", "ESI/SI/DH/MM6/XMM6", "EDI/DI/BH/MM7/XMM7" },
   { 0, 0, 0, 0, 0, 0, 0 ,0 }
};

MODRM2 sModRM32[] = {
   // 0 Mod
   { "[EAX]", "[ECX]", "[EDX]", "[EBX]",
      "[.][.]", "disp32", "[ESI]", "[EDI]" },
   // 1
   { "disp8[EAX]", "disp8[ECX]", "disp8[EDX]", "disp8[EBX]",
      "disp8[.][.]", "disp8[EBP]", "disp8[ESI]", "disp8[EDI]" },
   // 2
   {  "disp32[EAX]", "disp32[ECX]", "disp32[EDX]", "disp32[EBX]",
      "disp32[.][.]", "disp32[EBP]", "disp32[ESI]", "disp32[EDI]" },
   // 3
   { "EAX/AX/AL/MM0/XMM0", "ECX/CX/CL/MM1/XMM1", "EDX/DX/DL/MM2/XMM2",
      "EBX/BX/BL/MM3/XMM3", "ESP/SP/AH/MM4/XMM4", "EBP/BP/CH/MM5/XMM5",
      "ESI/SI/DH/MM6/XMM6", "EDI/DI/BH/MM7/XMM7" },
   { 0, 0, 0, 0, 0, 0, 0, 0 }
};

//NOTES:
//1. The [.][.] nomenclature means a SIB follows the ModR/M byte.
//2. The disp32 nomenclature denotes a 32-bit displacement following
//    the SIB byte, to be added to the index.
//3. The disp8 nomenclature denotes an 8-bit displacement following
//    the SIB byte, to be sign-extended and added to the index.

MODRM2   sSIB32[] = {
   // 0 Scale Index (SS), per Index 0-7
   { "[EAX]", "[ECX]", "[EDX]", "[EBX]",
      szNul, "[EBP]", "[ESI]", "[EDI]" },
   // 1 SS
   { "[EAX*2]", "[ECX*2]", "[EDX*2]", "[EBX*2]",
      szNul, "[EBP*2]", "[ESI*2]", "[EDI*2]" },
   // 2 SS
   {  "[EAX*4]", "[ECX*4]", "[EDX*4]", "[EBX*4]",
      szNul, "[EBP*4]", "[ESI*4]", "[EDI*4]" },
   // 3
   { "[EAX*8]", "[ECX*8]", "[EDX*8]", "[EBX*8]",
      szNul, "[EBP*8]", "[ESI*8]", "[EDI*8]" },
   { 0, 0, 0, 0, 0, 0, 0, 0 }
};

typedef struct tagREGT {
   LPTSTR   pType;
   LPTSTR   pName;
}REGT, * PREGT;

typedef struct tagREGSET {
   DWORD    dwReg;
   REGT     sRegt[7];
}REGSET, * PREGSET;

typedef struct tagREGSET2 {
   LPTSTR   pName[8];
}REGSET2, * PREGSET2;

TCHAR    szr8r[]  = "r8(/r)";
TCHAR    szr16r[] = "r16(/r)";
TCHAR    szr32r[] = "r32(/r)";
TCHAR    szmmr[]  = "mm(/r)";
TCHAR    szxmmr[] = "xmm(/r)";

#define  am_r8    0
#define  am_r16   1
#define  am_r32   2
#define  am_mm    3
#define  am_xmm   4

INT      giActMode = am_r32;
INT      giAltMode = am_r16;  // the flg_SIZE would switch to WIN16

REGSET   sRegSet[] = {
   { 0, { szr8r, "AL", szr16r, "AX" , szr32r, "EAX" , szmmr, "MM0" ,
    szxmmr, "XMM0" , "/digit(/r)", "0" , "REG=(/r)", "000" } },
   { 1, { szr8r, "CL" ,  szr16r, "CX" ,  szr32r, "ECX" ,  szmmr, "MM1" ,
    szxmmr, "XMM1" ,  "/digit(/r)", "1" ,  "REG=(/r)", "001" } },
   { 2, { szr8r, "DL" ,  szr16r, "DX" ,  szr32r, "EDX" ,  szmmr, "MM2" ,
    szxmmr, "XMM2" ,  "/digit(/r)", "2" ,  "REG=(/r)", "010" } },
   { 3, { szr8r, "BL" ,  szr16r, "BX" ,  szr32r, "EBX" ,  szmmr, "MM3" ,
    szxmmr, "XMM3" ,  "/digit(/r)", "3" ,  "REG=(/r)", "011" } },
   { 4, { szr8r, "AH" , szr16r, "SP" ,  szr32r, "ESP" ,  szmmr, "MM4" ,
    szxmmr, "XMM4" ,  "/digit(/r)", "4" ,  "REG=(/r)", "100" } },
   { 5, { szr8r, "CH" ,  szr16r, "BP" ,  szr32r, "EBP" ,  szmmr, "MM5" ,
    szxmmr, "XMM5" ,  "/digit(/r)", "5" ,  "REG=(/r)", "101" } },
   { 6, { szr8r, "DH" ,  szr16r, "SI" ,  szr32r, "ESI" ,  szmmr, "MM6" ,
    szxmmr, "XMM6" ,  "/digit(/r)", "6" ,  "REG=(/r)", "110" } },
   { 7, { szr8r, "BH" ,  szr16r, "DI" ,  szr32r, "EDI" ,  szmmr, "MM7" ,
    szxmmr, "XMM7" ,  "/digit(/r)", "7" ,  "REG=(/r)", "111" } },
   { 0, { 0,         0   ,  0,         0    ,  0,         0     ,  0,         0    ,
    0,         0      ,  0,         0      ,  0,         0      } }
};

REGSET2  sModRMReg[] = {
   // am_r8
   { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" },
   // am_r16
   { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" },
   // am_r32
   { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" },
   // am_mm
   { "MM0", "MM1", "MM2", "MM3", "MM4", "MM5", "MM6", "MM7" },
   // am_xmm
   { "XMM0", "XMM1", "XMM2", "XMM3", "XMM4", "XMM5", "XMM6", "XMM7" }
};

// NOTE: NOTE: The SIB Base nomenclature of "[*]" means a disp32
// with no base if MOD (dwMod) is 00, [EBP], otherwise
LPTSTR  sSIBBase[8] = { "EAX", "ECX", "EDX", "EBX", "ESP", "[*]", "ESI", "EDI" };

#define  ie_None           0
#define  ie_OutOfData      1
#define  ie_Failed         2
#define  ie_NotYetDone     3
#define  ie_NotIntel       4
#define  ie_CheckGetPtr    5

#define  sie_OutOfData     "Out of DATA within an INSTRUCTION"
#define  sie_Failed        "FAILED in DECODE"
#define  sie_NotYetDone    "NOT yet CODED"
#define  sie_NotIntel      "NO pointer for OPCODE"
#define  sie_CheckGetPtr   "GetIntelPtr returned INVALID pointer"

static   TCHAR cToBuf[32];
static   TCHAR cFromBuf[32];
static   TCHAR cComment[32];
   
#define  it_AddB2To        1
#define  it_AddD2From      2
#define  it_AddB2From      3
#define  it_AddDM2From     4

#define  st_None        0
#define  st_Disp        1
#define  st_Disp8       2
#define  st_Disp32      3

#undef   ADDMODRM2


PINTEL1 GetTablePtr(DWORD dwOp1, DWORD dwOp2, DWORD dwn)
{
   PINTEL1  pir = 0;
   PINTEL1  pi; // = sIntel16 or 52
   LPTSTR   pr;
   DWORD    dwo1, dwo2;
   DWORD    dwrl, dwv;

   if( giActMode == am_r16 )
      pi = &sIntel16[0];
   else
      pi = &sIntel32[0];

   // NOTE: Still NOT dealing with PRE-OPCODES!!!
   if( dwOp1 == 0x0c6 )
      dwrl = 0;

   while( (pr = pi->pReg) != 0 )    // get pointer to the "register" STRING
   {
      dwrl = strlen(pr);
      dwo1 = pi->dwOp1;
      dwo2 = pi->dwOp2;
      if( dwo1 == dwOp1 )    // the opcode
      {
         if( dwo2 == NO_OP2 )
         {
            if( (dwrl > 1) && (pr[0] == '/') && ( ISNUMERIC(pr[1] ) ) )
            {
               dwv = pr[1] - '0';   // gt the value
               //dwt = (DWORD)pCode[dwi+1];
               //dwRM  = GETRM(dwt);     // get r/m operand
               //if( dwn != dwRM )
               if( dwv == GETOP(dwOp2) )     // get REGISTER operand
               {
                  pir = pi;
                  break;
               }
            }
            else
            {
               pir = pi;
               break;
            }
         }
         else
         {
            // we have an opcode 2 to contend with
            if( dwo2 == dwOp2 )
            {
               if( (dwrl > 1) && (pr[0] == '/') && ( ISNUMERIC(pr[1] ) ) )
               {
                  dwv = pr[1] - '0';   // gt the value
                  //dwt = (DWORD)pCode[dwi+1];
                  //dwRM  = GETRM(dwt);     // get r/m operand
                  //if( dwn != dwRM )
                  if( dwv == GETOP(dwn) )     // get REGISTER operand
                  {
                     pir = pi;
                     break;
                  }
               }
               else
               {
                  pir = pi;
                  break;
               }
            }
         }
      }
      else if( dwo2 == NO_OP2 )  // assumes ALL "+??", NOT "+i" have NO 2nd OPCODE!!!
      {
         if(dwrl > 1)
         {
            if( ( pr[0] == '+' ) && ( pr[1] != 'i' ) )
            {
               if( ( dwo1 <= dwOp1 ) &&
                   ( (dwo1 + 7) >= dwOp1 ) )
               {
                  pir = pi;
                  break;
               }
            }
         }
      }
      pi++;
   }

   if( !pir )
   {
      sprtf( "FAILED to get Intel pointer with %x %x %x!"MEOR,
         dwOp1, dwOp2, dwn );
   }

   return pir;
}


#define  DBGSTOP  ((dwb==0x25C)&&(dwop1==0x83)&&(dwop2==0xB8)&&(dwnxt==0x80))

// ===== NEW CODE IMPORTED FROM YAHU - 31 JULY 2001 =====
#define  MXBSZ       1024
#define   USELCPTR
//LPTSTR  sSIBBase[8] = { "EAX", "ECX", "EDX", "EBX", "ESP", "[*]", "ESI", "EDI" };

#define  ie_None           0
#define  ie_OutOfData      1
#define  ie_Failed         2
#define  ie_NotYetDone     3
#define  ie_NotIntel       4
#define  ie_CheckGetPtr    5
#define  ie_FailedCode     6

//#define  sie_DebugExit     "Debug set exit"
#define  sie_OutOfData     "Out of DATA within an INSTRUCTION"
#define  sie_Failed        "FAILED in DECODE"
#define  sie_NotYetDone    "NOT yet CODED"
#define  sie_NotIntel      "NO pointer for OPCODE"
#define  sie_CheckGetPtr   "GetIntelPtr returned INVALID pointer"
#define  sie_FailedCode    "Expected CODE failed to set a pointer"

/* ========
#define  ISUPPER(a)  ( ( a >= 'A' ) && ( a <= 'Z' ) )
#define  ISLOWER(a)  ( ( a >= 'a' ) && ( a <= 'z' ) )
#define  ISNUMERIC(a)      ( ( a >= '0' ) && ( a <= '9' ) )
#define  ISNUM(a)    ISNUMERIC(a)
   ========= */
// __LINE__
#define  NYD   { iGotErr = ie_NotYetDone; s_iLine = __LINE__; goto Chk_Err; }
#define  TBD   NYD
#define  FAILEDCODE   { iGotErr = ie_FailedCode; s_iLine = __LINE__; goto Chk_Err; }
#define  CHKPTR   { iGotErr = ie_CheckGetPtr; s_iLine = __LINE__; goto Chk_Err; }

#define  TEST1    if( (dwi + 1) >= dwLen ) { iGotErr = ie_OutOfData; s_iLine = __LINE__; goto Chk_Err; }
#define  TEST3    if( (dwi + 3) >= dwLen ) { iGotErr = ie_OutOfData; s_iLine = __LINE__; goto Chk_Err; }
#define  TEST4    if( (dwi + 4) >= dwLen ) { iGotErr = ie_OutOfData; s_iLine = __LINE__; goto Chk_Err; }

// bump to next byte
#define  BUMP1    TEST1; dwi++
#define  BUMP3    TEST3; dwi+=3

#ifdef   USELCPTR

// use LOWER case
static   TCHAR szBP[]    = "byte ptr ";
static   TCHAR szBPss[]  = "byte ptr %s%s";
static   TCHAR szBPs[]   = "byte ptr %s";
static   TCHAR szBPb[]   = "byte ptr [";
static   TCHAR szBPbs[]  = "byte ptr [%s";
static   TCHAR szBPbsb[] = "byte ptr [%s]";

static   TCHAR szDP[]    = "dword ptr ";
static   TCHAR szDPss[]  = "dword ptr %s%s";
static   TCHAR szDPs[]   = "dword ptr %s";
static   TCHAR szDPb[]   = "dword ptr [";
static   TCHAR szDPbs[]  = "dword ptr [%s";
static   TCHAR szDPbsb[] = "dword ptr [%s]";
static   TCHAR szDPbxb[] = "dword ptr [%09xh]";
static   TCHAR szDPfs[]  = "dword ptr fs:";

static   TCHAR szWP[]    = "word ptr ";
static   TCHAR szWPs[]   = "word ptr %s";
static   TCHAR szWPb[]   = "word ptr [";
static   TCHAR szWPbsb[] = "word ptr [%s]";
static   TCHAR szWPbxb[] = "word ptr [%09xh]";
static   TCHAR szWPfs[]  = "word ptr fs:";

static   TCHAR szQP[]    = "qword ptr ";
static   TCHAR szQPs[]   = "qword ptr %s";
static   TCHAR szQPbsb[] = "qword ptr [%s]";

#else    // !USELCPTR

// USE ALL UPPER CASE
static   TCHAR szBP[]    = "BYTE PTR ";
static   TCHAR szBPss[]  = "BYTE PTR %s%s";
static   TCHAR szBPs[]   = "BYTE PTR %s";
static   TCHAR szBPb[]   = "BYTE PTR [";
static   TCHAR szBPbs[]  = "BYTE PTR [%s";
static   TCHAR szBPbsb[] = "BYTE PTR [%s]";

static   TCHAR szDP[]    = "DWORD PTR ";
static   TCHAR szDPss[]  = "DWORD PTR %s%s";
static   TCHAR szDPs[]   = "DWORD PTR %s";
static   TCHAR szDPb[]   = "DWORD PTR [";
static   TCHAR szDPbs[]  = "DWORD PTR [%s";
static   TCHAR szDPbsb[] = "DWORD PTR [%s]";
static   TCHAR szDPbxb[] = "DWORD PTR [%09XH]";
static   TCHAR szDPfs[]  = "DWORD PTR FS:";

static   TCHAR szWP[]    = "WORD PTR ";
static   TCHAR szWPs[]   = "WORD PTR %s";
static   TCHAR szWPb[]   = "WORD PTR [";
static   TCHAR szWPbsb[] = "WORD PTR [%s]";
static   TCHAR szWPbxb[] = "WORD PTR [%09XH]";
static   TCHAR szWPfs[]  = "WORD PTR FS:";

static   TCHAR szQP[]    = "QWORD PTR ";
static   TCHAR szQPs[]   = "QWORD PTR %s";
static   TCHAR szQPbsb[] = "QWORD PTR [%s]";

#endif   // #ifdef   USELCPTR y/n

static   TCHAR szBSpSB[] = "[%s+%s]";
static   TCHAR szBSmSB[] = "[%s-%s]";
static   TCHAR szBsB[]   = "[%s]";

TCHAR m_cToBuf[MXBSZ];      // to buffer
TCHAR m_cFromBuf[MXBSZ];    // form buffer
TCHAR m_cComment[MXBSZ];  // comment buffer
TCHAR m_cHeader[MXBSZ];  // comment buffer

#define  rs_32    0
#define  rs_16    1
#define  rs_08    2
#define  rs_MM    3
#define  rs_XM    4
#define  rs_MX    5

typedef struct {
   LPTSTR   pnm[rs_MX];
}MTCHREG, * PMTCHREG;

// example
//  else if( ( dwMod == 3 ) &&   // like dwst == st_None
//         ( InStr( pt, "r/m8" ) ) ) {
//      // not a displacement
//      prto = sMtchReg[dwRM].pnm[rs_08]; }

MTCHREG  sMtchReg[] = {
   // R/M 000
   { "EAX", "AX", "AL", "MM0", "XMM0" },
   // R/M 001
   { "ECX", "CX", "CL", "MM",  "XMM1" },
   // R/M 010
   { "EDX", "DX", "DL", "MM2", "XMM2" },
   // R/M 011
   { "EBX", "BX", "BL", "MM3", "XMM3" },
   // R/M 100
   { "ESP", "SP", "AH", "MM4", "XMM4" },
   // R/M 101
   { "EBP", "BP", "CH", "MM5", "XMM5" },
   // R/M 110
   { "ESI", "SI", "DH", "MM6", "XMM6" },
   // R/M 111
   { "EDI", "DI", "BH", "MM7", "XMM7" }
};


//#define  DBGSTOP  (dwb==0xffffffff) // none
//#define  SETEXIT

#ifndef  NDEBUG
#define  DBGEXIT     bDoExit = TRUE;
#else // !NDEBUG
#define  DBGEXIT
#endif   // NDEBUG y/n

#define  flg_FSOVER     0x00000001
#define  flg_GSOVER     0x00000002
#define  flg_SIZE       0x00000004
#define  flg_ADDR       0x00000008

static   BOOL  bDoExit = FALSE;
static   TCHAR szDbgStop[80];
#undef  GETSLOPSYM
static   INT   s_iLine;

typedef  struct tagCDWordArray {
   DWORD    dwSize;
   DWORD    dwCount;
   PDWORD   pdwArray;
}CDWordArray, * PCDWA;

#define  ADD2ARRAY( dwa, pir ) \
   if( dwa.pdwArray == 0 ) {\
      dwa.dwSize = MXBSZ;\
      dwa.pdwArray = (PDWORD)mMALLOC( (sizeof(DWORD) * dwa.dwSize) );}\
   if( dwa.pdwArray ) {\
      if( ( dwa.dwCount + 1 ) >= dwa.dwSize ) { \
         PDWORD _pdw = (PDWORD)mMALLOC( (sizeof(DWORD) * (dwa.dwSize+MXBSZ)) );\
         if( !_pdw ) { chkme( "MEMORY FAILED"MEOR ); pgm_exit(-2); }\
         memcpy( _pdw, dwa.pdwArray, (sizeof(DWORD) * dwa.dwCount) );\
         LocalFree( dwa.pdwArray );\
         dwa.pdwArray = _pdw;\
         dwa.dwSize += MXBSZ; }\
      dwa.pdwArray[ dwa.dwCount++ ] = pir; }


VOID  Add2Array( PCDWA dwa, PIMAGE_RELOCATION pir )
{
   if( dwa->pdwArray == 0 )
   {
      dwa->dwSize = MXBSZ;
      // dwa->pdwArray = (PDWORD)LocalAlloc( LPTR, (sizeof(DWORD) * dwa->dwSize) );
      dwa->pdwArray = (PDWORD)dMALLOC( (sizeof(DWORD) * dwa->dwSize) );
   }
   if( dwa->pdwArray )
   {
      if( ( dwa->dwCount + 1 ) >= dwa->dwSize )
      {
         PDWORD _pdw = (PDWORD)dMALLOC( (sizeof(DWORD) * (dwa->dwSize+MXBSZ)) );
         if( !_pdw )
         {
            chkme( "MEMORY FAILED"MEOR );
            pgm_exit(-2);
         }
         memcpy( _pdw, dwa->pdwArray, (sizeof(DWORD) * dwa->dwCount) );
         LocalFree( dwa->pdwArray );
         dwa->pdwArray = _pdw;
         dwa->dwSize += MXBSZ;
      }

      dwa->pdwArray[ dwa->dwCount++ ] = pir;
   }
}


#define  MXREGLEN    8     // 12
#define  MXNMLEN     12
#define  MXTAILLEN   12

VOID  AddpiStg( LPTSTR lpd, PINTEL1 pi )
{
   INT   i;
// dwOp1  dwOp2,   dwPre, pReg,     pNm,            pTail,    dwRes1, dwRes2, dwRes3;
//{ 0x0A7,NO_OP2, 0x0F2,  szNul,    "REPNE",        "CMPS m16, m16",        0,0,0 },
   sprintf(lpd, " { %#04X, ", pi->dwOp1 );   // note 04 to get (hopefully) 0x0A7

   if( pi->dwOp2 == NO_OP2 )
      strcat( lpd, "NO_OP2,  ");
   else
      sprintf(EndBuf(lpd), "%#03X, ", pi->dwOp2 );
                        if( pi->dwPre == NO_PRE )
                           strcat( lpd, "NO_PRE, ");
                        else
                           sprintf(EndBuf(lpd), "%#03X, ", pi->dwPre );

                        i = strlen(lpd);
                        if( *pi->pReg == 0 )
                        {
                           strcat( lpd, "szNul,  ");
                        }
                        else
                        {
                           strcat(lpd, "\"");
                           strcat( lpd, pi->pReg );
                           strcat( lpd, "\", ");
                        }
                        while( ( strlen(lpd) - i ) < MXREGLEN )
                           strcat(lpd, " ");

                        strcat(lpd, "\"");
                        i = strlen(lpd);
                        strcat(lpd, pi->pNm);
                        strcat(lpd, "\", ");
                        while( ( strlen(lpd) - i ) < MXNMLEN )
                           strcat(lpd, " ");

                        i = strlen(lpd);
                        if( *pi->pTail == 0 )
                        {
                           strcat(lpd, "szNul, ");
                        }
                        else
                        {
                           strcat(lpd, "\"");
                           strcat( lpd, pi->pTail );
                           strcat( lpd, "\", ");
                        }
                        while( ( strlen(lpd) - i ) < MXTAILLEN )
                           strcat(lpd, " ");

                        strcat(lpd, "0,0,0 }"MEOR);

}

// GetMinHex
#ifdef   USELCPTR

VOID  GetMinHex(LPTSTR lps, DWORD dwo)
{
   LPTSTR   lpf;  // = "%08H"
   //sprintf(lps, "%08XH", dwo);
   if(dwo < 10 )
      lpf = "%d";    // just use 'little' decimal
   else if(dwo < 0x10)
      lpf = "%02xh";
   else if(dwo < 0x100)
      lpf = "%03xh";
   else if(dwo < 0x1000)
      lpf = "%04xh";
   else if(dwo < 0x10000)
      lpf = "%05xh";
   else if(dwo < 0x100000)
      lpf = "%06xh";
   else if(dwo < 0x1000000)
      lpf = "%07xh";
   else if(dwo < 0x10000000)
      lpf = "%08xh";
   else
      lpf = "%09xh";

   sprintf(lps, lpf, dwo);
}

#else // !#ifdef   USELCPTR

VOID  GetMinHex(LPTSTR lps, DWORD dwo)
{
   LPTSTR   lpf;  // = "%08H"

   //sprintf(lps, "%08XH", dwo);
   if(dwo < 10 )
      lpf = "%d";    // just use 'little' decimal
   else if(dwo < 0x10)
      lpf = "%02XH";
   else if(dwo < 0x100)
      lpf = "%03XH";
   else if(dwo < 0x1000)
      lpf = "%04XH";
   else if(dwo < 0x10000)
      lpf = "%05XH";
   else if(dwo < 0x100000)
      lpf = "%06XH";
   else if(dwo < 0x1000000)
      lpf = "%07XH";
   else if(dwo < 0x10000000)
      lpf = "%08XH";
   else
      lpf = "%09XH";

   sprintf(lps, lpf, dwo);
}
#endif   // #ifdef   USELCPTR y/n


BOOL  bHadStop = FALSE;
BOOL  bClrStop = FALSE;
VOID  dbgstop( LPTSTR lpd )
{
   INT   i;
   bHadStop = TRUE;
   if(bClrStop)
      bHadStop = FALSE;
   i = 0;
}

#define  m_cBuf   g_cBuf
#define  m_cBuf2  g_cBuf2
#define  m_cBuf3  g_cBuf3

VOID DumpCode(PMWL pmwl, PBYTE pBegin, PBYTE pStgs,
              PBYTE pCode, DWORD dwLen, LPTSTR lpfn, ProcType typ )
{
   if( typ == pt_32 ) {
      giActMode = am_r32;
      giAltMode = am_r16;  // the flg_SIZE would switch to WIN16
   } else {
      giActMode = am_r16;
      giAltMode = am_r8;  // the flg_SIZE would switch to WIN16
   }

   DumpCodeSection( pmwl, pBegin, pStgs,
              pCode, dwLen, lpfn );

}

// ========== FROM YAHU PROJECT =============
// ************************************************************************
VOID DumpCodeSection(PMWL pmwl, PBYTE pBegin, PBYTE pStgs,
                               PBYTE pCode, DWORD dwLen, LPTSTR lpfn )
{
   LPTSTR                  lpd    = &m_cBuf[0];
   LPTSTR                  lpb    = &m_cBuf2[0];
   LPTSTR                  lpn    = &m_cBuf3[0];
   LPTSTR                  lpto   = &m_cToBuf[0];
   LPTSTR                  lpfrom = &m_cFromBuf[0];
   LPTSTR                  lpcomm = &m_cComment[0];  // comment buffer
   PIMAGE_SECTION_HEADER   psh = pmwl->pSectHdr;
   PIMAGE_SYMBOL           pis = pmwl->pSym;
   DWORD                   dwi, dwo, dwrel;
   PBYTE                   pb, pRel;
   char *                  pc;
   PINT                    pint;
   PINTEL1                 pi;
   LPTSTR                  pr, pt, preg, prto, prfrom, lps;
   PREGSET                 prs;
   DWORD                   dwb, dwn, dwrl, dwtl, dwnxt;
   DWORD                   dwop1, dwo1;
   DWORD                   dwop2, dwo2;
   INT                     i, iGotErr, iPos, iPos2;
//7 - 6 5  -  3 2  -  0
   DWORD                   dwMRM, dwMod, dwOp, dwRM;
   DWORD                   dwSIB, dwst;
   PMODRM2                 pmrm = &sModRM32[0];
   PREGSET                 pregset = &sRegSet[0];
   PREGSET2                pregset2 = &sModRMReg[giActMode];   // fast pointer to mode (32)
   PDWORD                  pdw;
   DWORD                   dwType, dwPre;
   LPTSTR                  lpsibs;
   LPTSTR                  lpsbase; // if a SIB (dwRM=4 & dwMod!=3) ie [..][..]
   LPTSTR                  lpmods;
   CDWordArray             dwa;
   PIMAGE_RELOCATION       pir;
   PWORD                   pword;
   DWORD                   dwFlag;  // various flags, like FS: override
   INT                     ialen;
   BOOL                    bDnSIB;  // just while filter the SIB increase up
   short *                 pshort;
   LPTSTR                  lpform;  // sprintf formation string pointer

   if( giActMode == am_r16 )
      pmrm = &sModRM16[0];

   iGotErr = 0;
   ZeroMemory( &dwa, sizeof(CDWordArray) );
   dwa.dwCount = 0;
   dwa.pdwArray = 0;
// FIX20061216 - NO ERROR EXIT IF IN DUMP4
#ifndef  DUMP4
   if( !pis || !psh )
   {
      dwb = 0;
      dwn = (DWORD)-1;

Do_Dump:

      // process a simple DUMP of the data
      pb = &pCode[dwb];
      dwo = dwLen - dwb;
      while( dwo )
      {
         if( dwo > 16 )
            dwi = 16;
         else
            dwi = dwo;
         *lpd = 0;
         //GetHEXString( lpd, pb, dwi, pBegin, TRUE );
         GetHEXString( lpd, pb, dwi, pCode, TRUE );
         AddStringandAdjust(lpd);
         pb  += dwi;    // bump the offset
         dwo -= dwi;    // reduce remaining count

         dwn--;
         if(dwn == 0)
            break;
      }

      if( !bDoExit )
      {
         sprintf( lpd, "ERROR %d indication is %s (Line=%d)",
            iGotErr,
            ( (iGotErr == ie_OutOfData) ? sie_OutOfData :
              (iGotErr == ie_Failed   ) ? sie_Failed    :
              (iGotErr == ie_NotYetDone) ? sie_NotYetDone :
              (iGotErr == ie_NotIntel ) ? sie_NotIntel :
              (iGotErr == ie_CheckGetPtr) ? sie_CheckGetPtr :
              (iGotErr == ie_FailedCode) ? sie_FailedCode :
               "Undefined error value" ),
            s_iLine );

         AddStringandAdjust(lpd);
      }
      sprtf( "ERROR EXIT: File[%s] (-5)"MEOR, lpfn );
#ifdef   DUMP4
      if( dwa.pdwArray )
         LocalFree( dwa.pdwArray );
#else // !DUMP4
      dwa.RemoveAll();
#endif   // DUMP4 y/n
      pgm_exit(-5);

      return;
   }
#endif // #ifndef  DUMP4

#ifdef   DUMP4
   ZeroMemory( &dwa, sizeof(CDWordArray) );
#else // !DUMP4
   dwa.RemoveAll();
#endif   // DUMP4 y/n

   *lpd = 0;
   GetSymName( lpd, pis, pStgs, FALSE );
   strcpy( m_cHeader, lpd );  // keep name of SERVICE handy
   AddASMString( lpd, 0 );
   //AddStringandAdjust(lpd);
   
   iGotErr = ie_None;      // no error yet
   dwo = 0;
   dwb = 0;    // start at ZERO
   pi = 0;
   dwrel = 0;
   dwi   = 0;
   if(psh) {
      dwrel = psh->NumberOfRelocations;
      dwi   = psh->PointerToRelocations;
   }
   if( dwi && dwrel && m_pSymTbl && m_dwSymCnt )
      pRel = pBegin + dwi;
   else
      pRel = 0;

   dwFlag = 0;    // clear the flag word
   // process BYTE BY BYTE
   // ================================================
   for( dwi = 0; dwi < dwLen; dwi++ )
   {
      pb = &pCode[dwi];
      dwop1 = (DWORD)*pb;        // get BYTE
      if( dwop1 == pre_FS )
      {
         //sprintf(lpd, "WARNING: ***TBD*** pre_FS prefix instructions!");
         //dwo = strlen(lpd);
         //AddASMString( lpd, dwo );
         dwFlag |= flg_FSOVER;   // setup the OVERRIDE flag
         continue;
      }
      else if( dwop1 == pre_GS )
      {
         dwFlag |= flg_GSOVER;   // setup the OVERRIDE flag
         continue;
      }
      else if( dwop1 == pre_SIZE )
      {
         dwFlag |= flg_SIZE;
         continue;
      }
      else if( dwop1 == pre_ADDR )
      {
         dwFlag |= flg_ADDR;
         continue;
      }

      if( (dwi + 1) < dwLen )
         dwop2 = (DWORD)pb[1];
      else
         dwop2 = (DWORD)-2;
      if( (dwi + 2) < dwLen )
         dwnxt = (DWORD)pb[2];
      else
         dwnxt = (DWORD)-2;

      sprintf(szDbgStop, "#define  DBGSTOP  DBS( %#X, %#03X, %#03X, %#03X )"MEOR,
         dwb, dwop1, dwop2, dwnxt );

      //pi = &sIntel1[0];
      pi = GetTablePtr( dwop1, dwop2, dwnxt );
      prs     = 0;
      preg    = 0;       // kill pointer to REGISTER name, like "ECX"
      prto    = 0;       // kill pointer to TO register or [memory]
      prfrom  = 0;       // kill pointer to FROM register, [memory] or immediate
      dwSIB   = 0;
      dwType  = 0;
      dwst    = st_None;   // no SIB type
      iPos2   = 0;        // so no offfset to [.][.], or disp32 or disp8
      lpmods  = 0;       // no ModR/M string yet
      *lpcomm = 0;      // always start with NO "; comment ..." string
      dwPre   = 0;      // NO - PRE-OPCODE
      bDnSIB  = FALSE;  // no increment for SIB yet
      dwMod   = 0;
      dwOp    = 0;     // get REGISTER operand
      dwRM    = 0;

      // that then allows a continue if failed all test
      //while( pi->pNm != 0 )
      //while(TRUE)
      if(pi)
      {
         pr = pi->pReg;    // get pointer to the "register" STRING
         pt = pi->pTail;   // and the "tail" string
         dwrl = strlen(pr);   // get "register" length
         dwtl = strlen(pt);   // and the "tail" length
         dwo1 = pi->dwOp1;    // extract the opcode 1
         dwo2 = pi->dwOp2;    // and opcode 2 (if any)

         if( DBGSTOP )
         {
            *lpd = 0;
            AddpiStg( lpd, pi );
            dbgstop(lpd);
            iPos2 = 0;
#ifdef   SETEXIT
            bDoExit = TRUE;
#endif   // SETEXIT
            if( !bDoExit )
            {
               sprtf(lpd);
            }
         }

         if( ( pr[0] != '+' ) &&
            ( dwo1 != dwop1 ) ) // CHECK for PRE-OPCODE
         {
            if( pi->dwPre == dwop1 )
            {
               BUMP1;   // bump to next byte
               dwPre = dwop1;
               dwop1 = dwop2;
               dwop2 = dwnxt;
               if( dwo2 != NO_OP2 )
               {
                  // we have an opcode 2 to contend with
                  dwi++;   // bump to next byte
                  if( dwo2 != dwop2 )
                  {
                     CHKPTR;
                  }
               }
               if( dwrl == 0 )
               {
                  // no 'register string
                  if( ISUPPER( *pt ) )
                  {
//{0x06D, NO_OP2, 0x0F3,  szNul,    "REP",          "INS r/m32, DX",        0,0,0 },
//{ 0XAE, NO_OP2,  0XF2, szNul,  "REPNE",     "SCAS m8",  0,0,0 }
// { 0x0AE, NO_OP2, NO_PRE, szNul,    "SCAS",         "m8",                   0,0,0 },
////{0x0AF, NO_OP2, NO_PRE, szNul,    "SCAS",         "m16",                  0,0,0 },
// { 0x0AF, NO_OP2, NO_PRE, szNul,    "SCAS",         "m32",                  0,0,0 },
// { 0x0AE, NO_OP2, NO_PRE, szNul,    "SCASB",        szNul,                  0,0,0 },
////{0x0AF, NO_OP2, NO_PRE, szNul,    "SCASW",        szNul,                  0,0,0 },
// { 0x0AF, NO_OP2, NO_PRE, szNul,    "SCASD",        szNul,                  0,0,0 },

// { 0x0AA, NO_OP2, 0x0F3,  szNul,    "REP",          "STOS m8",              0,0,0 },
////{0x0AB, NO_OP2, 0x0F3,  szNul,    "REP",          "STOS m16",             0,0,0 },
// { 0x0AB, NO_OP2, 0x0F3,  szNul,    "REP",          "STOS m32",             0,0,0 },
// { 0x0A6, NO_OP2, 0x0F3,  szNul,    "REPE",         "CMPS m8,m8",           0,0,0 },
////{0x0A7, NO_OP2, 0x0F3,  szNul,    "REPE",         "CMPS m16,m16",         0,0,0 },
// { 0x0A7, NO_OP2, 0x0F3,  szNul,    "REPE",         "CMPS m32,m32",         0,0,0 },
// { 0x0AE, NO_OP2, 0x0F3,  szNul,    "REPE",         "SCAS m8",              0,0,0 },
////{0x0AF, NO_OP2, 0x0F3,  szNul,    "REPE",         "SCAS m16",             0,0,0 },
// { 0x0AF, NO_OP2, 0x0F3,  szNul,    "REPE",         "SCAS m32",             0,0,0 },
// { 0x0A6, NO_OP2, 0x0F2,  szNul,    "REPNE",        "CMPS m8,m8",           0,0,0 },
////{0x0A7, NO_OP2, 0x0F2,  szNul,    "REPNE",        "CMPS m16,m16",         0,0,0 },
// { 0x0A7, NO_OP2, 0x0F2,  szNul,    "REPNE",        "CMPS m32,m32",         0,0,0 },
// { 0x0AE, NO_OP2, 0x0F2,  szNul,    "REPNE",        "SCAS m8",              0,0,0 },
////{0x0AF, NO_OP2, 0x0F2,  szNul,    "REPNE",        "SCAS m16",             0,0,0 },
// { 0x0AF, NO_OP2, 0x0F2,  szNul,    "REPNE",        "SCAS m32",             0,0,0 },
// still problem with
// FIX20010728-10 - release\dc4w.obj - ParseArgs()
//#define  DBGSTOP  DBS( 0XD3, 0XF2, 0XAE, 0XF7 )
// { 0XAE, NO_OP2,  0XF2, szNul,  "REPNE",     "SCAS m8",  0,0,0 }
//ERROR: (r= t=SCAS m8) MRM=C2(0,0,0) m=<nul> to=(null) NOSIB
//0000:00D5 F7 D1 2B F9 8B C1 8B F7  8B FA C1 E9 02 F3 A5 8B  ..+.............
// i get
// 000000D3: F2 AE                REPNE       SCAS m8
// should be
//  00409983 F2 AE                repne scas  byte ptr [edi]
                     iPos = InStr( pt, "r/" );
                     if(iPos)
                     {
                        NYD;
                     }
                     else
                     {
                        // the forms seen are "UUU m32", "UUU m32,m32"
                        // 8-bit        forms "UUU m8",  "UUU m8,m8" 
                        // 16-bit "UUU m16", "UUU m16,m16" - not yet coded
                        iPos = InStr( pt, "m32" );
                        if(iPos)
                        {
                           for( i = 0; i < iPos; i++ )
                           {
                              lpfrom[i] = pt[i];
                              if( !ISUPPER( pt[i] ) )
                              {
                                 i++;     // include the space
                                 break;
                              }
                           }
                           lpfrom[i] = 0;
                           sprintf(EndBuf(lpfrom), szDPbsb, "EDI");
                           lps = Right( pt, (strlen(pt) - (iPos+2)) );
                           if( InStr(lps, "m32") )
                           {
                              strcat(lpfrom, ", ");
                              sprintf(EndBuf(lpfrom), szDPbsb, "ESI");
                           }
                           preg   = lpfrom;
                           goto Got_Stg;
                        }
                        iPos = InStr( pt, "m8" );
                        if(iPos)
                        {
                           for( i = 0; i < iPos; i++ )
                           {
                              lpfrom[i] = pt[i];
                              if( !ISUPPER( pt[i] ) )
                              {
                                 i++;     // include the space
                                 break;
                              }
                           }
                           lpfrom[i] = 0;
                           sprintf(EndBuf(lpfrom), szBPbsb, "EDI");
                           lps = Right( pt, (strlen(pt) - (iPos+2)) );
                           if( InStr(lps, "m8") )
                           {
                              strcat(lpfrom, ", ");
                              sprintf(EndBuf(lpfrom), szBPbsb, "ESI");
                           }
                           preg   = lpfrom;
                           goto Got_Stg;
                        }
                     }
                  }
                  else
                  {
                     NYD;
                  }
               }
               else
               {
                  NYD;
               }
            }
            else
            {
               CHKPTR;
            }
         }

         if( dwo2 != NO_OP2 )
         {
            // we have an opcode 2 to contend with
            BUMP1;   // bump to next byte
            if( dwo2 != dwop2 )
            {
               CHKPTR;
            }
         }

         // ASSUMPTION: If the tail is szNul,
         // only ADD the pnemonic, simple
         // ********************************
         if( dwtl == 0 )
         {
            // EXCEPT: As to be expected perhaps there are EXCEPTION(s)
            // 0050ECF2   faddp       st(1),st
            preg = szNul;  // set this, and
            if( strcmp( pi->pNm, "FADDP" ) == 0 )
            {
               if( dwop2 == 0xc1 )
               {
                  strcpy(lpto, "ST(1)");
                  strcpy(lpfrom, "ST" );
                  prto = lpto;
                  prfrom = lpfrom;
                  preg = 0;
               }
               else
               {
                  NYD;
               }
            }
            goto Got_Stg;  // go for it
            // Note below there is code marked // ***** TO BE DELETED *****
            // that previously handled this case
         }

         if( (dwrl > 1) && ( pr[0] == '+' ) )
         {
            if( pr[1] == 'i' )
            {
               chkme( "WARNING: The FPU set NOT YET CODED"MEOR );
            }
            else if( pr[1] == 'r' )
            {
               // opcode indicates register as an offset
               if( dwrl < 3 )
               {
                  chkme( "WHAT does this +r indicate!!!"MEOR );
               }
               else if( pr[2] == 'b' )
               {
                  // FIX20010620-09
// example
//004089DB B0 01                mov         al,1
//004089DD E9 94 00 00 00       jmp         std::basic_string<char,std::char_traits<char>,std::allocator<char> >::_Grow+136h (00408a
//#define  DBGSTOP  DBS( 0X9B, 0XB0, 0X1, 0XE9 )
//FAILED 0X9B 0XB0 0X1 0XE9!
// { 0XB0, NO_OP2,  NO_PRE, "+rb",  "MOV",       "r8,imm8",  0,0,0 }
//ERROR: (r=+rb t=r8,imm8) MRM=4D(1,1,5) m=<nul> to=(null)
//0000:009B B0 01 E9 94 00 00 00 83  7D 08 00 75 30 8B 55 0C  ........}..u0.U.
                  if( ( strbgn( pt, "r8" ) == 0 ) && ( InStr( pt, "imm8" ) ) )
                  {
                     // can NOT use
                     // prto = prs->sRegt[am_r8].pName; // get the relevant register
                     // since there is NO ModR/M byte here
                     strcpy(lpto, "AL");
                     prto = lpto;
                     // appears OK
                     //sprtf( "CHECKME-8: Added code to get 8-bit register!"MEOR );
                     BUMP1;   // bump to next byte
                     pb = &pCode[dwi];
                     dwo = (DWORD)*pb; // get next byte, an additive item
                     pc = (char *)pb;
                     i = *pc;
                     sprintf( lpfrom, "%d", i );
                     prfrom = lpfrom;
                  }
                  else
                  {
                     sprtf( "The 8-bit register set NOT YET fully CODED"MEOR
                        "\t(r=%s t=%s)"MEOR, pr, pt );
                     TBD;
                  }
               }
               else if( pr[2] == 'd' )
               {
//{0x050, NO_OP2, NO_PRE, "+rd",    "PUSH",         "r32",                  0,0,0 },
//{0x0B8, NO_OP2, NO_PRE, "+rd",    "MOV",          "r32,imm32",            0,0,0 },
            // we have value up to value plus 7
                  if( ( dwo1 <= dwop1 ) &&
                      ( (dwo1 + 7) >= dwop1 ) )
                  {
                     dwo = dwop1 - dwo1;
                     //prs = &pregset[dwo];    // =&sRegSet[0]
                     //preg = prs->sRegt[giActMode].pName; // get the relevant register
                     //preg = sModRMReg[giActMode].pName[dwo];
                     if( dwFlag & flg_SIZE )
                     {
                        preg = sModRMReg[giAltMode].pName[dwo];   // fast pointer to mode (32)
                        sprtf( "CHECKME-REG: Early selection per flg_SIZE! (r=%s)"MEOR, preg );
                     }
                     else
                     {
                        preg = sModRMReg[giActMode].pName[dwo];   // pregset2->pName[dwo];
                     }
                     // if "r32,imm32" then must collect the DATA
                     if( InStr( pt, "imm32" ) > 0 )
                     {
                        // iPos = InStr( pi->pNm, "PUSH" ); or MOV
// example
// { 0XB8, NO_OP2,  NO_PRE, "+rd",  "MOV",       "r32,imm32", 0,0,0 }
// (r=+rd t=r32,imm32) mrm=20(1,7,5) m=<nul> to=(null)
// 0000:001D E8 00 00 00 00 5F 5E 5B  83 C4 40 3B EC E8 00 00  ....._^
                        BUMP1;
                        TEST3;
                        pdw = (PDWORD) &pCode[dwi];
                        dwo = *pdw;
                        //sprintf( lpfrom, "%08XH", dwo );
                        //if( !ISNUM( *lpfrom ) )
                        //   sprintf( lpfrom, "%09XH", dwo );
                        *lpb = 0;
                        pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                        if(pir)
                        {
                           ADD2ARRAY( dwa, pir );
                           sprintf(lpfrom, szDPbsb, lpb );
                           //strcpy( lpcomm, "; Adj. by rel. item" );
                        }
                        else
                        {
                           GetMinHex( lpfrom, dwo );
                           strcpy( lpcomm, "; Adj. by value" );
                        }
                        prfrom = lpfrom;
                        dwi += 3;
                     }
                  }
                  else
                  {
                     chkme( "CAN NEVER BE HERE!!! (r=%s t=%s)"MEOR, pr, pt );
                     TBD;
                  }
               }
               else if( pr[2] == 'w' )
               {
//{0x050, NO_OP2, NO_PRE, "+rw",    "PUSH",         "r16",                  0,0,0 },
//{0x0B8, NO_OP2, NO_PRE, "+rw",    "MOV",          "r16,imm16",            0,0,0 },
            // we have value up to value plus 7
                  if( ( dwo1 <= dwop1 ) &&
                      ( (dwo1 + 7) >= dwop1 ) )
                  {
                     dwo = dwop1 - dwo1;
                     //prs = &pregset[dwo];    // =&sRegSet[0]
                     //preg = prs->sRegt[giActMode].pName; // get the relevant register
                     //preg = sModRMReg[giActMode].pName[dwo];
                     if( dwFlag & flg_SIZE )
                     {
                        preg = sModRMReg[giAltMode].pName[dwo];   // fast pointer to mode (32)
                        sprtf( "CHECKME-REG: Early selection per flg_SIZE! (r=%s)"MEOR, preg );
                     }
                     else
                     {
                        preg = sModRMReg[giActMode].pName[dwo];   // pregset2->pName[dwo];
                     }
                     // if "r16,imm16" then must collect the DATA
                     if( InStr( pt, "imm16" ) > 0 )
                     {
                        // iPos = InStr( pi->pNm, "PUSH" ); or MOV
// example
// { 0XB8, NO_OP2,  NO_PRE, "+rw",  "MOV",       "r16,imm16", 0,0,0 }
// (r=+rd t=r32,imm32) mrm=20(1,7,5) m=<nul> to=(null)
// 0000:001D E8 00 00 00 00 5F 5E 5B  83 C4 40 3B EC E8 00 00  ....._^
                        BUMP1;
                        TEST1;
                        pword = (PWORD) &pCode[dwi];
                        dwo = *pword;
                        //sprintf( lpfrom, "%08XH", dwo );
                        //if( !ISNUM( *lpfrom ) )
                        //   sprintf( lpfrom, "%09XH", dwo );
                        *lpb = 0;
                        pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                        if(pir)
                        {
                           ADD2ARRAY( dwa, pir );
                           sprintf(lpfrom, szDPbsb, lpb );
                           //strcpy( lpcomm, "; Adj. by rel. item" );
                        }
                        else
                        {
                           GetMinHex( lpfrom, dwo );
                           strcpy( lpcomm, "; Adj. by value" );
                        }
                        prfrom = lpfrom;
                        dwi += 1;
                     }
                  }
                  else
                  {
                     chkme( "WHAT does this r=%s indicate!!! (t=%s)"MEOR, pr, pt );
                     TBD;
                  }
               }
               else
               {
                  chkme( "WHAT does this r=%s indicate!!! (t=%s)"MEOR, pr, pt );
                  TBD;
               }
            }
            else
            {
               chkme( "Unknown + set NOT YET CODED (r=%s t=%s)"MEOR, pr, pt );
            }
         }
         else
         {
            if( dwo1 == dwop1 )  // they match
            {
               if( (dwrl > 1) && ( pr[0] == '/' ) && ( pr[1] == 'r' ) )
               {
                  //> /r-Indicates that the ModR/M byte of the instruction
                  // contains both a register operand and an r/m operand.
                  BUMP1;   // bump to next byte
//{0x08B, NO_OP2, NO_PRE, "/r",     "MOV",          "r32,r/m32",            0,0,0 },
// eg   00000009: 8B 45 08           mov         eax,dword ptr [ebp+8]
// BUT
//{0x089, NO_OP2, NO_PRE, "/r",     "MOV",          "r/m32,r32",            0,0,0 },
// should be
//  00000015: 89 45 F4           mov         dword ptr [ebp-0Ch],eax
// but I get
//  00004B18 89 45 F4              MOV         EAX, DWORD PTR [EBP+F4H]
// fixed at the end = (saddly) if the "tail" is "r/m32,r32" THEN switch "to" and "from"!!!
//
                  pb = &pCode[dwi];
                  dwMRM = (DWORD)*pb; // get the ModR/M byte
                  dwMod = GETMOD(dwMRM);
                  dwOp  = GETOP(dwMRM);     // get REGISTER operand
                  dwRM  = GETRM(dwMRM);
                  lpmods = pmrm[dwMod].pRM[dwRM];  // = MODRM2 sModRM32;
                  prs = &pregset[dwOp];   // = &sRegSet[0] note dwOp IS in range!
                  if( dwFlag & flg_SIZE )
                  {
                     prto = prs->sRegt[giAltMode].pName; // get the relevant register
                     // this looks to be working well, so
                     //sprtf( "CHECKME-TO: /r Early selection per flg_SIZE! (to=%s)"MEOR, prto );
                  }
                  else
                  {
                     prto = prs->sRegt[giActMode].pName; // get the relevant register
                  }

                  //if( (iPos2 = InStr(lpmods, "[.][.]")) > 0 )
                  //   dwst = st_Disp;
                  //else if( (iPos2 = InStr(lpmods, "disp32")) > 0 )
                  //   dwst = st_Disp32;
                  //else if( (iPos2 = InStr(lpmods, "disp8")) > 0 )
                  //   dwst = st_Disp8;

                  if( (iPos2 = InStr(lpmods, "[.][.]")) > 0 )
                  {
                     dwst = st_Disp;
                     if( dwRM != 4 )
                     {
                        // defensive code only
                        sprtf( "WARNING: Documentation note that [.][.] means"MEOR
                           "\ta SIB byte follows APPEARS ERRANT!"MEOR );
                        DBGEXIT;
                     }
                  }
                  else if( (iPos2 = InStr(lpmods, "disp32")) > 0 )
                  {
                     dwst = st_Disp32;
                     if( dwMod != 2 )
                     {
                        // defensive code only
                        if( !( ( dwMod == 0 ) && ( dwRM == 5 ) ) )  // the 1st group exception
                        {
                           sprtf( "WARNING: Documentation note that disp32 means"MEOR
                              "\tan Effective Address in 3rd group APPEARS ERRANT!"MEOR );
                           DBGEXIT;
                        }
                     }
                  }
                  else if( (iPos2 = InStr(lpmods, "disp8")) > 0 )
                  {
                     dwst = st_Disp8;
                     if( dwMod != 1 )
                     {
                        // defensive code only
                        sprtf( "WARNING: Documentation note that disp8 means"MEOR
                           "\tan Effective Address in 2nd group APPEARS ERRANT!"MEOR );
                        DBGEXIT;
                     }
                  }
                  else
                  {
                     if(( dwMod == 3 ) ||
                        ( ( dwMod == 0 ) && ( ( dwRM != 4 ) || ( dwRM != 5 ) ) ) )
                     {
                        // this is OK
                     }
                     else
                     {
                        // defensive code only
                        chkme( "WARNING: Table facts appear errant!"MEOR );
                        DBGEXIT;
                     }
                  }

                  if( dwst == st_None )
                  {
                     if( ( DBGSTOP  ) &&   // 0x37 or ...
                         ( !bDoExit ) )
                     {
                        sprintf( lpd, "MRM ~D (r=%s t=%s) "      // 1,2
                           "%02X(%d,%d,%d) "                // 3,4,5,6
                           "m=%s to=%s"MEOR,               // 7,8
                           pr, pt,                   // 1, 2
                           dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
                           lpmods, prto );                // 7,8
                        sprtf(lpd);
                     }
                  }
                  else  // we have a DISPLACEMENT [.][.] or disp8 or disp32
                  {
                     //possible errata: InStr(prfrom, "disp8") does NOT appear to denote SIB
                     TEST1;

                     dwSIB = (DWORD)pb[1];
                     lpsibs = sSIB32[GETSS(dwSIB)].pRM[GETIND(dwSIB)];
                     // CARE if GETIND(dwSIB) = 4, since this is "" ie NONE
                     //lpsbase = sSIB32[0].pRM[GETBASE(dwSIB)];
                     // OR
                     if( ( dwRM == 4 ) && ( dwMod != 3 ) )
                     {
                        lpsbase = sSIBBase[GETBASE(dwSIB)];
                     }
//#ifdef   ADDMODRM2
                     if( ( DBGSTOP  ) &&  // 0x37 or ...
                         ( !bDoExit ) )
                     {
//                        sprintf( lpd, "mrm(r=%s t=%s) "      // 1,2
//                           "%02X(%d,%d,%d) "                // 3,4,5,6
//                           "m=%s to=%s "                     // 7,8
//                           "SIB%02X"                       // 9
//                           "(%d,%d,%d)"                    // 10,11,12
//                           "s=%s b=%s"MEOR,                       // 13
//                           pr, pt,                   // 1, 2
//                           dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
//                           lpmods, prto,                    // 7,8
//                           dwSIB,                           // 9
//                           GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
//                           lpsibs,     // 13
//                           sSIBBase[GETBASE(dwSIB)] );   // 14 lpsbase
                        if( ( dwRM  == 4 ) &&
                            ( dwMod != 3 ) )
                        {
                           sprintf( lpd, "mrm (r=%s t=%s) "      // 1,2
                              "%02X(%d,%d,%d) "                // 3,4,5,6
                              "m=%s to=%s "MEOR                 // 7,8
                              "\tSIB[%02X]"                       // 9
                              "(%d,%d,%d)"                    // 10,11,12
                              "si=%s b=%s"MEOR,                       // 13, 14
                              pr, pt,                   // 1, 2
                              dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
                              lpmods, prto,                    // 7,8
                              dwSIB,                           // 9
                              GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
                              lpsibs,                    // 13
                              lpsbase );  // 14 = sSIBBase[GETBASE(dwSIB)]
                        }
                        else
                        {
                           sprintf( lpd, "mrm (r=%s t=%s) " // 1,2
                              "%02X(%d,%d,%d) "             // 3,4,5,6
                              "m=%s to=%s NoSIB"MEOR,       // 7,8
                              pr, pt,                       // 1, 2
                              dwMRM, dwMod, dwOp, dwRM,     // 3,4,5,6
                              lpmods, prto );               // 7,8
                              //dwSIB,                           // 9
                              //GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
                              //lpsibs,                    // 13
                              //sSIBBase[GETBASE(dwSIB)] ); // 14 lpsbase
                        }
                        sprtf(lpd);
                     }
//#endif   // #ifdef   ADDMODRM2
                  }

                  if( ( dwRM == 4 ) && ( dwMod != 3 ) )
                  {
                     // SIB byte indication
                     if( !bDnSIB )
                     {
                        //sprtf( "CHECKME-SIB: Changed to early SIB increment!"MEOR );
                        BUMP1;   // use up the SIB byte
                        bDnSIB = TRUE; // eat the SIB now being used
                     }
                  }
                  // CHANGE CODE FROM this
                  //if( ( iPos = InStr(lpmods, "/") ) > 1 )
                  //to simply
                  iPos = InStr(lpmods, "/");
                  if( dwMod == 3 )  // then in the Effective Address group 4 (with "/")
                  {
                     // this is a dwMod==3 (11b)
// error when
//ModR/M ~D (r=/r t=r32,r/m16) C0(3,0,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
// 00000065: 0F BF C0             MOVSX       EAX, EAX
// { 0X0F, 0XBF, NO_PRE, "/r",   "MOVSX",     "r32,r/m16", 0,0,0 }
//(r=/r t=r32,r/m16) mrm=20(3,0,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
//0000:0068 85 C0 0F 84 85 01 00 00  E8 00 00 00 00 89 45 F4  ......
// The FROM must be like
//  else if( ( dwMod == 3 ) && // like dwst == st_None ( InStr( pt, "r/m8" ) ) ) {
// not a displacement so prto = sMtchReg[dwRM].pnm[rs_08]; }
// to get the correct form, like MOVSX  EAX, AX

                        // FIX20010620-08-A
//#define  DBGSTOP  DBS(0x88,0x32,0xC0,0xe9)
// I get 00000088: 32 C0                XOR         EAX, EAX
// CV    004089C8  32 C0                xor         al,al
// MRM ~D (r=/r t=r8,r/m8) C0(3,0,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
                     if( strbgn( pt, "r8" ) == 0 )
                     {
                        prto = prs->sRegt[am_r8].pName; // get the relevant register
                        sprtf( "CHECKME-82: Changed code to get 8-bit register!"MEOR );
                     }

                     if( InStr( pt, "r/m16" ) )
                     {
                        prfrom = sMtchReg[dwRM].pnm[rs_16];
                     }
                     else if( InStr( pt, "r/m8" ) )
                     {
// FIX20010620-08-B
//#define  DBGSTOP  DBS(0x88,0x32,0xC0,0xe9)
// I get 00000088: 32 C0                XOR         EAX, EAX
// CV    004089C8  32 C0                xor         al,al
// MRM ~D (r=/r t=r8,r/m8) C0(3,0,0) m=EAX/AX/AL/MM0/XMM0 to=EAX

// still error on
// FIX20010728-9 - release\dc4w.obj - ParseArgs()
//#define  DBGSTOP  DBS(0xa7, 0x84, 0xc0, 0x75)
// { 0X84, NO_OP2,  NO_PRE, "/r",   "TEST",      "r/m8,r8",  0,0,0 }
//ERROR: (r=/r t=r/m8,r8) MRM=C0(3,0,0) m=EAX/AX/AL/MM0/XMM0 to=EAX NOSIB
// i get
//000000A7: 84 C0                TEST        EAX, AL
// should be
// 00409957 84 C0                test        al,al
                        // was
                        //prfrom = Left(lpmods, (iPos - 1));
                        // perhaps
                        //sprtf( "CHECKME-R8: Switching DST to %s from %s!"MEOR,
                        //   prs->sRegt[am_r8].pName,
                        //   prto );
                        prto = prs->sRegt[am_r8].pName; // get the relevant register
                        prfrom = sMtchReg[dwRM].pnm[rs_08];
                     }
                     else
                     {
// FIX20010729-08 - File[D:\GTOOLS\TOOLS\DC4W\RELEASE\DC4W.OBJ] - see INTEL-21.TXT - My_mbsrchr
// #define  DBGSTOP  DBS( 0X1A, 0X85, 0XFF, 0X74 ) // missed flg_SIZE
// want 0040D82A 66 85 FF             test        di,di
                        // and REMEMBER the to/from can be SWITCHED below
                        if( dwFlag & flg_SIZE )
                        {
                           prfrom = Mid(lpmods, (iPos+1), 2);
                           dwFlag &= ~( flg_SIZE );
                        }
                        else
                        {
                           prfrom = Left(lpmods, (iPos - 1));
                        }
                     }
//ModR/M (r=/r ib t=r32,r/m32,imm8) C9(3,1,1) m=ECX/CX/CL/MM1/XMM1 to=ECX
// 00000228: 6B C9                IMUL        ECX, ECX
// should be
// 00000228: 6B C9 0A           imul        ecx,ecx,0Ah
                     if( strcmp( pt, "r32,r/m32,imm8" ) == 0 )
                     {
                        BUMP1;   // bump to next byte
                        pb = &pCode[dwi];
                        dwo = (DWORD)*pb; // get the next byte
                        sprintf(EndBuf(prfrom), ",%02XH", dwo );
                     }
                  }
                  else if( dwst == st_Disp32 )  // InStr(lpmods, "disp32") == 1 )
                  {
                     // could simply use if( dwMod == 2 ) !!!
                     BUMP1;
                     TEST3;
                     pdw = (PDWORD) &pCode[dwi];
                     dwo = *pdw; // get the dword
                     pint = (PINT)pdw;
                     i = *pint;
                     *lpb = 0;
                     pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                     // That is the Effective Address = "disp32" only
                     //if( strlen(lpmods) == 6 )  // then there is NO REGISTER
                     if( ( dwMod == 0 ) && ( dwRM == 5 ) )
                     {
                        if( strlen(lpmods) != 6 )  // then there is NO REGISTER
                           chkme( "CHECKME-NAP: WARNING: Change in CODE not as previous!!!" );
//FAILED BD 89 15! (r=/r t=r/m32,r32) mrm=00(0,2,5) m=disp32 to=(null) SIB00(0,0,0)s=[EAX]
// This failure is due to the SWITCH below!!!
// 000000AF: 8B 15 00 00 00 00  mov         edx,dword ptr [_GetListType]
                        if( strcmp( pt, "r/m32,r32" ) == 0 )
                        {
                           // NOTE: Already know dwMod=0 and dwRM=5
// error in
// { 0X89, NO_OP2,  NO_PRE, "/r",   "MOV",       "r/m32,r32", 0,0,0 }
//ModR/M (r=/r t=r/m32,r32) 0D(0,1,5) m=disp32 to=ECX SIB04(0,0,4)s=[EAX]
// 00000023: 89 0D 04 00 00 00    MOV         [_defaultLegendFont], ECX
// should have mov         dword ptr [defaultLegendFont+4 (00a55a84)],ecx
                           if( dwFlag & flg_FSOVER )
                           {
                              if( dwFlag & flg_SIZE )
                                 strcpy( lpfrom, szWPfs );
                              else
                                 strcpy( lpfrom, szDPfs );
                              dwFlag &= ~(flg_FSOVER);
                           }
                           else
                           {
                              if( dwFlag & flg_SIZE )
                                 strcpy(lpfrom, szWP);
                              else
                                 strcpy(lpfrom, szDP);
                           }

                           if( dwFlag & flg_SIZE )
                              sprtf( "CHECKME-SZ: Should the DST register also be!"MEOR );

                           dwFlag &= ~(flg_SIZE);

                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              if(dwo)
                                 sprintf( EndBuf(lpfrom), "[%s+%d]", lpb, dwo );
                              else
                                 sprintf( EndBuf(lpfrom), szBsB, lpb ); // ="[%s]"
                              pir = 0;
                           }
                           else
                           {
                              sprintf( EndBuf(lpfrom), "[%08XH]", dwo );

                           }

                           if( i < 0 )
                              chkme( "CHECKME-RNH: WARNING: Relative NEGATIVE offset NOT handled"MEOR );

                           prfrom = lpfrom;
                           // NOTE: This will be SWITCHED below
                        }
                        else
                        {
// FIX20010728-8 - release\dc4w.obj
// 000000AE: 66 8B 0D 00 00 00 00 MOV         ECX, [??_C@_01PJCK@?4?$AA@]
//WARNING: SIZE PRE-OP FLAG NOT USED!
//#define  DBGSTOP  DBS( 0XAE, 0X8B, 0XD, 0x00 )
//FAILED 0XB4 0X8B 0XD 000! (Line=3307)
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=0D(0,1,5) m=disp32 to=ECX NOSIB
//0000:00B5 66 89 08 8B 43 2C 85 C0  74 6D 8B 15 00 00 00 00  f...C,..tm......
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              if( dwFlag & flg_SIZE ) // the 16-bit operand-size prefix (66H)
                              {
                                 // if we ALTER the SRC size, then MUST alter DST
                                 sprintf(lpfrom, szWPbsb, lpb);
                                 prto = prs->sRegt[am_r16].pName; // get the relevant register
                              }
                              else
                                 sprintf(lpfrom, szDPbsb, lpb );
                              pir = 0;
                           }
                           else
                           {
                              //pdw = (PDWORD) &pCode[dwi];
                              //dwo = *pdw; // get the dword
                              if( dwFlag & flg_SIZE ) // the 16-bit operand-size prefix (66H)
                              {
                                 // if we ALTER the SRC size, then MUST alter DST
                                 sprintf( lpfrom, szWPbxb, dwo );
                                 prto = prs->sRegt[am_r16].pName; // get the relevant register
                              }
                              else
                                 sprintf( lpfrom, szDPbxb, dwo );
                           }

                           dwFlag &= ~( flg_SIZE ); // the 16-bit operand-size prefix (66H)
                           prfrom = lpfrom;
                        }
                     }
                     else
                     {
// ModR/M (r=/r t=r8,r/m8) 81(2,0,1) m=disp32[ECX] to=EAX SIB00(0,0,0)s=[EAX]
// 00000093: 8A 81 00 00 00 00  mov         al,byte ptr _GetListType[ecx]
                        lps = Right(lpmods, 5);    // get the "[REG]" from the string
                        if( strcmp( pt, "r8,r/m8" ) == 0 )
                        {
                           prto = prs->sRegt[am_r8].pName; // get the relevant register
                           //*lpb = 0;
                           //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              //sprintf(lpfrom, "BYTE PTR %s%s", lpb, lps );
                              sprintf(lpfrom, szBPss, lpb, lps );
                              pir = 0;
                           }
                           else
                           {
                              //sprintf(lpfrom, "BYTE PTR %s", &lpmods[6] );
                              //sprintf(lpfrom, "BYTE PTR %s", lps );
                              sprintf(lpfrom, szBPs, lps );
                              if( i < 0 )
                              {
                                 dwo = ~(dwo) + 1;
                                 sprintf( &lpfrom[(strlen(lpfrom)-1)], "-0%XH]", dwo );
                              }
                              else
                              {
                                 sprintf( &lpfrom[(strlen(lpfrom)-1)], "+0%XH]", dwo );
                              }
                           }
                           prfrom = lpfrom;
                        }
                        else if( strcmp( pt, "r/m8,r8" ) == 0 )
                        {
// FIX20010727-02 - DEBUG\DC4W.obj - see INTEL-10.txt
// I get
//00000157 88 8D 7C FE FF FF    MOV         BYTE PTR [EBP], BYTE PTR [EAX-0184H]
// should be
//00412D87 88 8D 7C FE FF FF    mov         byte ptr [ebp-184h],cl
// { 0X88, NO_OP2,  NO_PRE, "/r",   "MOV",       "r/m8,r8",  0,0,0 }
//mrm (r=/r t=r/m8,r8) 8D(2,1,5) m=disp32[EBP] to=ECX NoSIB
// remember pr=/r indicates the ModR/M contains both a register and an r/m operand.
                           // just s SWITCH of direction
                           prfrom = prs->sRegt[am_r8].pName; // get the relevant register
                           //*lpb = 0;
                           //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              //sprintf(lpfrom, "BYTE PTR %s%s", lpb, &lpmods[6] );
                              sprintf(lpto, szBPss, lpb, lps );
                              pir = 0;
                           }
                           else
                           {
                              //sprintf(lpfrom, "BYTE PTR %s", &lpmods[6] );
                              sprintf(lpto, szBPs, lps );
                              if( i < 0 )
                              {
                                 dwo = ~(dwo) + 1;
                                 // this LOOKS like an ERROR
                                 //sprintf( &lpfrom[(strlen(lpfrom)-1)], "-0%XH]", dwo );
                                 sprintf( &lpto[(strlen(lpto)-1)], "-0%XH]", dwo );
                              }
                              else
                              {
                                 sprintf( &lpto[(strlen(lpto)-1)], "+%02XH]", dwo );
                              }
                           }
                           // another ERROR, since prfrom is SET above
                           //prfrom = lpfrom;
                           prto   = lpto;
                        }
                        else  // should be "r/m32,r32" OR "r32,r/m32"!!!
                        {
                           // and dwMod would be 2!
                           // NOTE: have already extracted the from "[REG]" string with
                           // lps = Right(lpmods, (strlen(lpmods) - 6) );
                           // Also note IFF if( strcmp( pt, "r/m32,r32" ) == 0 )
                           // then the exchange "to" and "from" is done below
// FAILED: Produced
// 00000020: 66 8B 80 CA 01 00 00 MOV         EAX, DWORD PTR [EAX+1CAH]
// with WARNING: SIZE PRE-OP FLAG NOT USED!
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//(r=/r t=r32,r/m32) MRM=80(2,0,0) m=disp32[EAX] to=EAX SIBCA(3,1,2)s=[ECX*8]
//0000:0027 5F 5E 5B 8B E5 5D C3                              _^[..].
//ERROR EXIT: File[D:\FG2\MSVC6\FGFS32\LibPLIBPui\DEBUG\puMenuBar.obj] (-5)
//ModR/M (r=/r t=r32,r/m32) 80(2,0,0) m=disp32[EAX] to=EAX SIBCA(3,1,2)s=[ECX*8]
// should be mov ax,word ptr [eax+1CAh]
                           //*lpb = 0;
                           //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                           if( dwFlag & flg_SIZE ) // the 16-bit operand-size prefix (66H)
                           {
                              sprtf( "WARNING: New code implemented - flg_SIZE(66H). CHECKME-S!!!"MEOR );
                              prto = prs->sRegt[am_r16].pName; // get the relevant register
                              if( !pir )
                                 strcpy(lpfrom, szWP);
                              dwFlag &= ~(flg_SIZE); // the 16-bit operand-size prefix (66H)
                           }
                           else if( !pir )
                           {
                              strcpy(lpfrom, szDP);
                           }
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              sprintf(lpfrom, "%s%s", lpb, lps );
                              pir = 0;
                           }
                           else
                           {
                              strcat(lpfrom, lps);
                              if( i < 0 )
                              {
                                 dwo = ~(dwo) + 1;
                                 sprintf( &lpfrom[(strlen(lpfrom)-1)], "-0%XH]", dwo );
                              }
                              else if( dwo )
                              {
                                 sprintf( &lpfrom[(strlen(lpfrom)-1)], "+%02XH]", dwo );
                              }
                              prfrom = lpfrom;
                           }
                        }
                     }
                     dwi += 3;   // note - for loop will do final increment
                     if(pir)
                     {
                        chkme( "Relocation POINTER not used!"MEOR );
                     }
                  }
                  else if( dwst == st_Disp8 )   // InStr(lpmods, "disp8") == 1 )
                  {
// failed on
// FIX20010727-08 - see INTEL-12.TXT - Fixed - typo of pr instead of pt
//#define  DBGSTOP  DBS(0x1d6, 0x8a, 0x4d, 0xdc)
// i get
// 000001D6: 8A 4D DC             MOV         ECX, DWORD PTR [EBP-24H]
// 000001D9: 88 4D E0             MOV         BYTE PTR [EBP-20H], CL
// should be
//  00410556 8A 4D DC             mov         cl,byte ptr [d]
//  00410559 88 4D E0             mov         byte ptr [c],cl
// { 0X8A, NO_OP2,  NO_PRE, "/r",   "MOV",       "r8,r/m8",  0,0,0 }
//ERROR: (r=/r t=r8,r/m8) MRM=4D(1,1,5) m=disp8[EBP] to=ECX
//	NoSIBDC(3,3,4)s=[EBX*8] b=ESP
//0000:01D9 88 4D E0 8B 55 F0 83 C2  01 89 55 F0 EB 09 8B 45  .M..U.....U....E

// But still a problem with
// FIX20010727-09 - see INTEL-12.TXT
//#define  DBGSTOP  DBS( 0X203, 0XF, 0XBE, 0X4D )
// { 0X0F, 0XBE, NO_PRE, "/r",   "MOVSX",     "r32,r/m8", 0,0,0 }
//ERROR: (r=/r t=r32,r/m8) MRM=4D(1,1,5) m=disp8[EBP] to=ECX
//	NoSIBE0(3,4,0)s= b=EAX
//0000:0207 83 F9 20 7D 02 EB 16 0F  BE 55 DC 83 FA 22 74 0B  .. }.....U..."t.
// i get
// 00000203: 0F BE 4D E0          MOVSX       ECX, DWORD PTR [EBP-20H]
// should be
//  00410583 0F BE 4D E0          movsx       ecx,byte ptr [c]
// similarly
// 0000020E: 0F BE 55 DC          MOVSX       EDX, DWORD PTR [EBP-24H]
// should be
//  0041058E 0F BE 55 DC          movsx       edx,byte ptr [d]
// and more instances of SAME movsx edx, dword ptr [ebp-24h] instead of byte ptr!!!
// but I get it correct in other places, like
// 00000242: 0F BE 08             MOVSX       ECX, BYTE PTR [EAX]
// like
//  004105C2 0F BE 08             movsx       ecx,byte ptr [eax]
                     BUMP1;   // bump to next byte
                     pb = &pCode[dwi];
                     dwo = (DWORD)*pb; // get next byte, an additive item
                     pc = (char *)pb;
                     i = *pc;
                     lps = Right(lpmods, (strlen(lpmods) - 5) );

                     iPos = 0;
                     if( strcmp( pt, "r/m8,r8" ) == 0 )
                        iPos = 1;
                     else if( strcmp( pt, "r8,r/m8" ) == 0 )
                        iPos = 2;
                     else if( strcmp( pt, "r32,r/m8" ) == 0 )
                        iPos = 3;
                     else if( strcmp( pt, "r/m8,r32" ) == 0 )
                        iPos = 4;

                     if(iPos)
                     {
                        strcpy(lpb, szBP);
                        if( iPos < 3 )
                           prto = prs->sRegt[am_r8].pName; // get the relevant register
                     }
                     else if( dwFlag & flg_SIZE )
                     {
                        strcpy(lpb, szWP);
                        prto = prs->sRegt[am_r16].pName; // get the relevant register
                        sprtf( "WARNING: Check new flg_SIZE change!"MEOR );
                        dwFlag &= ~( flg_SIZE );
                     }
                     else
                     {
                        strcpy(lpb, szDP);
                     }

                     if( i < 0 )
                     {
                        dwo = (0x100 - dwo);
                        sprintf( &lps[(strlen(lps)-1)], "-%02XH]", dwo );
                     }
                     else if(dwo)
                     {
                        sprintf( &lps[(strlen(lps)-1)], "+%02XH]", dwo );
                     }
                     strcat(lpb, lps);


                     if( ( iPos == 1 ) || ( iPos == 4 ) )
                     {
                        // the old switch-er-roo
                        prfrom = prto;
                        prto = lpb;
                     }
                     else
                     {
                        prfrom = lpb;
                     }

                  }
                  else if( InStr( pt, "rel32" ) ) // InStr(lpmods, "rel32") == 1 )
                  {
                     BUMP1;   // bump to next byte
                     TEST3;
                     pdw = (PDWORD) &pCode[dwi];
                     dwo = *pdw;
                     sprintf( lpto, "%08XH", dwo );
                     prto = lpto;
                     prfrom = szNul;
                     dwi += 3;   // note the next will happen as part of the for loop
                  }
                  else if( strcmp( pt, "r32,r/m8" ) == 0 )
                  {
                     if( dwst == st_Disp )
                     {
// ModR/M (r=/r t=r32,r/m8) 0C(0,1,4) m=[.][.] to=ECX SIB10(0,2,0)s=[EDX]
// FAILED on 0X0F! n=MOVSX r=/r t=r32,r/m8 m=[EDX]
//  000000B2 0F BE 0C              MOVSX       ECX, BYTE PTR [.][.]
// should be
//  000000B2: 0F BE 0C 10        movsx       ecx,byte ptr [eax+edx]
                        if( !bDnSIB )
                        {
                           BUMP1;   // use up the SIB byte
                           bDnSIB = TRUE; // eat the SIB now being used
                        }
                        strcpy(lpfrom, szBPb);
                        dwo = GETBASE(dwSIB);
                        //strcat(lpfrom, sRegSet[dwo].sRegt[giActMode].pName);
                        if( dwo == 5 )
                        {
                           // NOTE: The SIB Base nomenclature of "[*]" means a disp32
                           // with no base id MOD is 00, [EBP], otherwise
                           if( dwMod != 0 )
                           {
                              strcat(lpfrom, "EBP" );
                              strcat(lpfrom, "+");
                           }
                        }
                        else
                        {
                           strcat(lpfrom, sSIBBase[dwo] );  // lpsbase
                           strcat(lpfrom, "+");
                        }
                        // now, this
                        //strcat(lpfrom, sRegSet[GETIND(dwSIB)].sRegt[giActMode].pName);
                        // or this
                        strcat(lpfrom, &lpsibs[1]);
                        //lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;
                        //strcat(lpfrom, "]");
                        prfrom = lpfrom;
                     }
                     else
                     {
// Add
//{0x00F, 0x0BE,  NO_PRE, "/r",     "MOVSX",        "r32,r/m8",             0,0,0 },
// like 0F BE 02           movsx       eax,byte ptr [edx]

// but in this case
// FIX20010729-09 - File[D:\GTOOLS\TOOLS\DC4W\RELEASE\DC4W.OBJ] - see INTEL-21.TXT - My_mbsrchr
//WARNING: SIZE PRE-OP FLAG NOT USED!  ***CHECKME***
//#define  DBGSTOP  DBS( 0X2E, 0XF, 0XB6, 0X8 )
//FAILED 0X31 0XF 0XB6 0X8! (Line=3597)
// { 0X0F, 0XB6, NO_PRE, "/r",   "MOVZX",     "r32,r/m8", 0,0,0 }
//ERROR: (r=/r t=r32,r/m8) MRM=08(0,1,0) m=[EAX] to=CX NOSIB
// however, CODE appears correct!!!
//0040D83E 66 0F B6 08          movzx       cx,byte ptr [eax]
                        if( dwFlag & flg_SIZE )
                        {
                           // appears there can be an OVERRIDE here,
                           // but we have the correct CODE, so
                           sprtf( "CHECKME-SZ: Override flag dumped!"MEOR );
                           dwFlag &= ~(flg_SIZE);
                        }
                        sprintf( lpfrom, szBPs, lpmods );
                        prfrom = lpfrom;
                     }
                  }
                  else if( ( strcmp( pt, "r32,r/m32" ) == 0 ) ||
                           ( strcmp( pt, "r/m32,r32" ) == 0 ) )
                  {
// FAILED with (r=/r t=r32,r/m32) 11(0,2,1) m=[ECX] to=EDX
//  00000099: 8B 11              mov         edx,dword ptr [ecx]
// FAILED with (r=/r t=r/m32,r32) 08(0,1,0) m=[EAX] to=ECX

// got error with
// 0000024E: 8B 0C                MOV         ECX, DWORD PTR [.][.]
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=0C(0,1,4) m=[.][.] to=ECX
//   SIB82(2,0,2)s=[EAX*4] b=EDX
//0000:0250 82 89 4D D4 8B 55 D8 52  8B 45 DC 50 8B 4D D4 51  ..M..U.R.
                     if( ( dwRM == 4 ) && ( dwMod != 3 ) )
                     {
                        // we have a SIB byte
                        if( !bDnSIB )
                        {
                           BUMP1;   // use up the SIB byte
                           bDnSIB = TRUE;
                        }
                        if( dwMod == 0 )
                        {
// FIX20010726-02 - File: DEBUG\DC4W.OBJ
// Failed in case         // lpsbase
// 000000BF: 89 0C D5             MOV         DWORD PTR [[*]+EDX*8], ECX
//#define  DBGSTOP  DBS( 0XBF, 0X89, 0XC, 0XD5 )
// { 0X89, NO_OP2,  NO_PRE, "/r",   "MOV",       "r/m32,r32", 0,0,0 }
//ERROR: (r=/r t=r/m32,r32) MRM=0C(0,1,4) m=[.][.] to=DWORD PTR [[*]+EDX*8]
//   SIBD5(3,2,5)s=[EDX*8] b=[*]
//0000:00C2 80 02 00 00 8B 45 FC 8D  0C C5 80 02 00 00 8B 55  .....E.......
// should be
// 0040F3CF 89 0C D5 C0 B9 4E 00 mov         dword ptr [edx*8+4EB9C0h],ecx
                           // NOTE: The SIB Base nomenclature of "[*]" means a disp32
// NOTE: if( strcmp( pt, "r/m32,r32" ) == 0 ), then exchange "to" and "from" done below
                           strcpy(lpfrom, szDP);
                           if( GETBASE(dwSIB) == 5 )  // This is [*]
                           {
                              BUMP1;   // bump to next byte
                              TEST3;
                              pdw = (PDWORD) &pCode[dwi];
                              dwo = *pdw;
                              *lpb = 0;
                              pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                              strcat(lpfrom, lpsibs);
                              if(pir)
                              {
                                 ADD2ARRAY( dwa, pir );
                                 sprintf(EndBuf(lpfrom), "+%s", lpb);
                                 GetMinHex( lpb, dwo );
                                 sprintf(lpcomm, "; +[%s]", lpb );
                              }
                              else
                              {
                                 GetMinHex( lpb, dwo );
                                 lpfrom[ (strlen(lpfrom)-1) ] = 0; // kill trailing "]"
                                 sprintf(EndBuf(lpfrom), "+%s]", lpb);  // and ADD hex offset
                              }
                              prfrom = lpfrom;
                              dwi += 3;
                              sprtf( "CHECKME-4: New code WARNING: dwMod=0 dwRM=4 dwSB=5"MEOR );
                           }
                           else
                           {
                              sprintf(EndBuf(lpfrom), "[%s+", sSIBBase[GETBASE(dwSIB)] );
                              strcat(  lpfrom, &lpsibs[1] );
                              // this has been checked
                              //sprtf( "CHECKME: New code WARNING: dwMod=0 dwRM=4 dwSB!=5"MEOR );
                           }
                        }
                        else if( dwMod == 1 ) // disp8[.][.] case - 2nd Group
                        {
//example
// FIX20010701-03 = Fixed
// 0000004A: 8B 55 0C             MOV         EDX, DWORD PTR [EBP+0CH]
//FAILED 0X4F 0X8B 0X44 0X2!
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=44(1,0,4) m=disp8[.][.] to=EAX
//        SIB02(0,0,2)s=[EAX] b=EDX
//0000:004D 8B 44 02 08 23 C1 85 C0  74 1E 8B 4D F8 81 E1 FF  .D..#...t
// should get
//00417E3D 8B 44 02 08          mov         eax,dword ptr [edx+eax+8]
//00417E41 23 C1                and         eax,ecx
//00417E43 85 C0                test        eax,eax
                           BUMP1;   // bump to next byte
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get next byte, an additive item
                           pc = (char *)pb;
                           i = *pc;
                           if( GETIND(dwSIB) == 4 )
                           {
// FIX20010728-1 - see INTEL-13.TXT
//#define  DBGSTOP  DBS( 0X13, 0X8B, 0X7C, 0X24 )
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=7C(1,7,4) m=disp8[.][.] to=EDI
//	SIB24(0,4,4)s= b=ESP
//0000:0013 8B 7C 24 0C 85 FF 74 06  FF 15 00 00 00 00 A1 00  .|$...t.........
// should get
// 00408863 8B 7C 24 0C          mov         edi,dword ptr [esp+0Ch]
                              if( GETBASE(dwSIB) == 5 )
                              {
                                 // NOTE: The SIB Base nomenclature of "[*]", ie 5
                                 // means a disp32 with no base if dwMod = 0,
                                 // else [EBP] otherwise
                                 TBD;
                              }
                              else
                              {
// FIX20010729-07 - File[D:\GTOOLS\TOOLS\DC4W\RELEASE\DC4W.OBJ] - see INTEL-21.TXT - My_mbsrchr
//CHECKME-TO: /r Early selection per flg_SIZE! (to=DI)
// 00000015: 66 8B 7C 24 10       MOV         DI, dword ptr [ESP+010h]
//WARNING: SIZE PRE-OP FLAG NOT USED!  ***CHECKME***
// #define  DBGSTOP  DBS( 0X15, 0X8B, 0X7C, 0X24 )
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=7C(1,7,4) m=disp8[.][.] to=DI
//	SIB24(0,4,4)s= b=ESP
                                 if( dwFlag & flg_SIZE )
                                 {
                                    strcpy( lpfrom, szWP );
                                    dwFlag &= ~( flg_SIZE );
                                 }
                                 else
                                    strcpy( lpfrom, szDP ); // establish DWORD PTR

                                 if(dwo)  // if there is an offset
                                 {
                                    if( i < 0 )
                                    {
                                       dwo = 0x100 - dwo;
                                       lpform = szBSmSB; // "[%s-%s]";
                                    }
                                    else
                                    {
                                       lpform = szBSpSB; // "[%s+%s]";
                                    }
                                    GetMinHex( lpd, dwo );
                                    sprintf(EndBuf(lpfrom), lpform, lpsbase, lpd );
                                    prfrom = lpfrom;
                                 }
                                 else
                                 {
                                    sprintf(EndBuf(lpfrom), szBsB, lpsbase); // "[%s]"
                                 }
                              }
                           }
                           else
                           {
// FIX20010701-05 - FIXED
// 00000015: 33 C0                XOR         EAX, EAX
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
// mrm (r=/r t=r32,r/m32) 44(1,0,4) m=disp8[.][.] to=EAX
//        SIB[0A](0,1,2)si=[ECX] b=EDX
// 00000017: 66 8B 44 0A 06       MOV         EAX, DWORD PTR [EDX+ECX+6]
// WARNING: SIZE PRE-OP FLAG NOT USED!
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=44(1,0,4) m=disp8[.][.] to=EAX
//        SIB0A(0,1,2)s=[ECX] b=EDX
//0000:001C 89 45 F4 8B 4D 08 8B 51  3C 8B 45 08 8D 8C 10 F8  .E..M..Q<.
// Should be -
//00418655 33 C0                xor         eax,eax
//00418657 66 8B 44 0A 06       mov         ax,word ptr [edx+ecx+6]
//0041865C 89 45 F4             mov         dword ptr [ebp-0Ch],eax
                              if( dwFlag & flg_SIZE )
                              {
                                 strcpy( lpfrom, szWP);
                                 //sprtf( "WARNING: NEW code implemented - flg_SIZE(66H). CHECKME-S2!!!"MEOR );
                                 //prto = prs->sRegt[am_r16].pName; // get the relevant register
                                 dwFlag &= ~(flg_SIZE);
                              }
                              else
                                 strcpy( lpfrom, szDP);

                              if( GETSS(dwSIB) == 0 )
                              {
                                 sprintf(EndBuf(lpfrom),
                                    "[%s+%s+%d]",
                                    sSIBBase[GETBASE(dwSIB)], // 14 lpsbase
                                    Mid( lpsibs, 2, 3 ), // = sSIB32[GETSS(dwSIB)].pRM[GETIND(dwSIB)];
                                    dwo );
                              }
                              else // if( GETSS(dwSIB) == 2 ) or 1 or 3
                              {
// FIX20010701-04 - FIXED
// 00000044: 8B 4D 08             MOV         ECX, DWORD PTR [EBP+08H]
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=54(1,2,4) m=disp8[.][.] to=EDX
//        SIBC1(3,0,1)s=[EAX*8] b=ECX
//0000:0047 8B 54 C1 04 89 55 E8 8B  45 E8 50 8B 4D EC 51 8B  .T...U..E
//Should be
//004181F4 8B 4D 08             mov         ecx,dword ptr [ebp+8]
//004181F7 8B 54 C1 04          mov         edx,dword ptr [ecx+eax*8+4]
//004181FB 89 55 E8             mov         dword ptr [ebp-18h],edx
                                 sprintf(EndBuf(lpfrom),
                                    "[%s+%s+%d]",
                                    sSIBBase[GETBASE(dwSIB)], // 14 lpsbase
                                    Mid( lpsibs, 2, 5 ), // = sSIB32[GETSS(dwSIB)].pRM[GETIND(dwSIB)];
                                    dwo );
                              }
                           }
                        }
                        else if( dwMod == 2 )   // disp32[.][.] case
                        {
// FIX20010729-01
//#define  DBGSTOP  DBS( 0X91, 0X89, 0X84, 0X24 )
//FAILED 0X93 0X89 0X84 0X24! (Line=4041)
// { 0X89, NO_OP2,  NO_PRE, "/r",   "MOV",       "r/m32,r32", 0,0,0 }
//ERROR: (r=/r t=r/m32,r32) MRM=84(2,0,4) m=disp32[.][.] to=EAX
//	SIB24(0,4,4)s= b=ESP
//0000:0091 89 84 24 A8 00 00 00 C7  84 24 88 00 00 00 08 00  ..$......$......
//00409D61 89 84 24 A8 00 00 00 mov         dword ptr [esp+0A8h],eax
                           // NOTE: if "r/m32,r32" then to/from switch done below
                           BUMP1;   // bump to next byte
                           TEST3;
                           pdw = (PDWORD)&pCode[dwi];
                           dwo = *pdw; // get dword, an additive item
                           pint = (PINT)pdw;
                           i = *pint;  // and get SIGNED value
                           if( GETBASE(dwSIB) == 5 )
                           {
                              // since dwMod=2, then [EBP]
                              TBD;
                           }
                           else
                           {
                              if( dwFlag & flg_SIZE )
                              {
                                 strcpy( lpfrom, szWPb);
                                 //sprtf( "CHECKME4: NEW code implemented - flg_SIZE(66H)."MEOR );
                                 prto = prs->sRegt[am_r16].pName; // get the relevant register
                                 dwFlag &= ~(flg_SIZE);
                              }
                              else
                                 strcpy( lpfrom, szDPb);

                              strcat(lpfrom, lpsbase);

                              if(dwo)
                              {
                                 if( i < 0 )
                                 {
                                    dwo = ~(dwo) + 1;
                                    lpform = "-%s]";
                                 }
                                 else
                                 {
                                    lpform = "+%s]";
                                 }

                                 GetMinHex(lpd, dwo);

                                 sprintf(EndBuf(lpfrom), lpform, lpd);

                              }
                              else
                              {
                                 strcat(lpfrom, "]");
                              }

                              prfrom = lpfrom;
                           }
                           BUMP3;
                        }
                        else
                        {
                           TBD;
                        }
                     }
                     else
                     {
// FIX20010701-07 = FIXED
// 00000055: 66 8B 11             MOV         EDX, DWORD PTR [ECX]
//WARNING: SIZE PRE-OP FLAG NOT USED!
// { 0X8B, NO_OP2,  NO_PRE, "/r",   "MOV",       "r32,r/m32", 0,0,0 }
//ERROR: (r=/r t=r32,r/m32) MRM=11(0,2,1) m=[ECX] to=EDX
//0000:0058 52 8B 45 F8 83 C0 02 50  6A 00 6A 00 FF 15 00 00  R.E....
// should be
//00418A43 33 D2                xor         edx,edx
//00418A45 66 8B 11             mov         dx,word ptr [ecx]
//00418A48 52                   push        edx
                        if( dwFlag & flg_SIZE )
                        {
                           prto = prs->sRegt[am_r16].pName; // get the relevant register
                           sprintf(lpfrom, szWPs, lpmods );
                           dwFlag &= ~(flg_SIZE);
                        }
                        else
                           sprintf(lpfrom, szDPs, lpmods );
                     }
                     prfrom = lpfrom;
                     // NOTE: If "r/m32,r32" "from" and "to" will be switched
                  }
                  else if( strcmp( pt, "r8,r/m8" ) == 0 )
                  {
// FAILED with (r=/r t=r8,r/m8) 08(0,1,0) m=[EAX] to=ECX
//  00000051: 8A 08              mov         cl,byte ptr [eax]
// am_r8

// also failed on
// FIX20010728-4 - see INTEL-15.TXT
//#define  DBGSTOP  DBS( 0xd8, 0x8a, 0x04, 0x16)
// i get
//000000D8: 8A 04                MOV         AL, byte ptr [.][.]
//000000DA: 16                   PUSH        SS
//000000DB: 8D 2C 16 3C 20 0F 8E LEA         EBP, [ESI+EDX+08E0F203CH]
// should be
// 00409128 8A 04 16             mov         al,byte ptr [esi+edx]
// 0040912B 8D 2C 16             lea         ebp,[esi+edx]
// 0040912E 3C 20                cmp         al,20h
// { 0X8A, NO_OP2,  NO_PRE, "/r",   "MOV",       "r8,r/m8",  0,0,0 }
//ERROR: (r=/r t=r8,r/m8) MRM=04(0,0,4) m=[.][.] to=AL
//	SIB16(0,2,6)s=[EDX] b=ESI
                     prto = prs->sRegt[am_r8].pName; // get the relevant register
                     if( dwRM == 4 )
                     {
                        // must FIX the SRC - the SIB gives the register
                        strcpy(lpfrom, szBP);  // establish BYTE PTR
                        if( dwMod == 0 )
                        {
                           //sprtf( "CHECKME2-RM4-0: New handling of this case!"MEOR );
                           if( GETBASE(dwSIB) == 5 )
                           {
                              // then NO base since dwMod=0
                              sprintf(EndBuf(lpfrom),
                                       szBsB,   // "[%s]",
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                           }
                           else
                           {
                              // just put in BASE and SIB register
                              sprintf(EndBuf(lpfrom),
                                       szBSpSB, // = "[%s+%s]",
                                       lpsbase,    // = sSIBBase[GETBASE(dwSIB)],
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                           }
                        }
                        else if( dwMod == 1 )
                        {
                           BUMP1;   // bump to next byte
                           sprtf( "CHECKME3-RM4-1: New handling of this case!"MEOR );
                           pdw = (PDWORD) &pCode[dwi];
                           pc = (char *)pdw; // get the displacement
                           dwo = (DWORD)*pc;
                           i  = *pc;
                           if( i < 0 )
                           {
                              dwo = 0x100 - dwo;
                              lpform = szBSmSB; // = "[%s-%s]";
                           }
                           else
                           {
                              lpform = szBSpSB; // = "[%s+%s]";
                           }
                           GetMinHex(lpd, dwo);

                           if( GETBASE(dwSIB) == 5 )
                              sprintf(EndBuf(lpfrom), lpform, "EBP", lpd);
                           else
                              sprintf(EndBuf(lpfrom), lpform, lpsbase, lpd);
                        }
                        else
                        {
                           // disp32
                           TBD;
                        }
                     }
                     else
                     {
                        sprintf( lpfrom, szBPs, lpmods );
                     }
                     prfrom = lpfrom;
                  }
                  else if( strcmp( pt, "r/m8,r8" ) == 0 )
                  {
//FAILED with (r=/r t=r/m8,r8) 0A(0,1,2) m=[EDX] to=ECX
//FAILED on 88 0A! n=MOV r=/r t=r/m8,r8 m=[EDX] off=0x3a
// 00000039: 88 0A              mov         byte ptr [edx],cl
                     // note the switch (from r8,r/m8 above)
                     prfrom = prs->sRegt[am_r8].pName; // get the relevant register
                     if( dwRM == 4 )
                     {
                        // must FIX the DST - the SIB gives the register
                        strcpy(lpto, szBP);  // establish BYTE PTR
                        if( dwMod == 0 )
                        {
                           sprtf( "CHECKME3-RM4-0: New handling of this case!"MEOR );
                           if( GETBASE(dwSIB) == 5 )
                           {
                              // then NO base since dwMod=0
                              sprintf(EndBuf(lpto),
                                       szBsB,   // "[%s]",
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                           }
                           else
                           {
                              // just put in BASE and SIB register
                              sprintf(EndBuf(lpto),
                                       szBSpSB,    // = "[%s+%s]",
                                       lpsbase,    // = sSIBBase[GETBASE(dwSIB)],
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                           }
                        }
                        else if( dwMod == 1 )
                        {
                           // disp8
// FIX20010728-6 - see INTEL-15.TXT - release\dc4w.obj - Getpis()
//#define  DBGSTOP  DBS( 0X11F, 0X88, 0X4C, 0X24 )
//FAILED 0X121 0X88 0X4C 0X24! (Line=3940)
// { 0X88, NO_OP2,  NO_PRE, "/r",   "MOV",       "r/m8,r8",  0,0,0 }
//ERROR: (r=/r t=r/m8,r8) MRM=4C(1,1,4) m=disp8[.][.] to=ECX
//	SIB24(0,4,4)s= b=ESP
//0000:011F 88 4C 24 2C 8A D9 89 44  24 10 73 1B 8A 1C 06 80  .L$,...D$.s.....
//0040916F 88 4C 24 2C          mov         byte ptr [esp+2Ch],cl
//00409173 8A D9                mov         bl,cl
//00409175 89 44 24 10          mov         dword ptr [esp+10h],eax
                           BUMP1;   // bump to next byte
                           sprtf( "CHECKME3-RM4-1: New handling of this case!"MEOR );
                           pdw = (PDWORD) &pCode[dwi];
                           pc = (char *)pdw; // get the displacement
                           dwo = (DWORD)*pc;
                           i  = *pc;
                           if( i < 0 )
                           {
                              dwo = 0x100 - dwo;
                              lpform = szBSmSB; // = "[%s-%s]";
                           }
                           else
                           {
                              lpform = szBSpSB; // = "[%s+%s]";
                           }
                           GetMinHex(lpd, dwo);

                           if( GETBASE(dwSIB) == 5 )
                              sprintf(EndBuf(lpto), lpform, "EBP", lpd);
                           else
                              sprintf(EndBuf(lpto), lpform, lpsbase, lpd);
                        }
                        else if( dwMod == 2 )
                        {
                           // disp32
// FIX20010729-04 - see INTEL-19.txt - release\dc4w.obj
//#define  DBGSTOP  DBS( 0XBA, 000, 0X84, 0XC0 )
//FAILED 0XBC 000 0X84 0XC0! (Line=4342)
// { 0000, NO_OP2,  NO_PRE, "/r",   "ADD",       "r/m8,r8",  0,0,0 }
//ERROR: (r=/r t=r/m8,r8) MRM=84(2,0,4) m=disp32[.][.] to=EAX
//	SIBC0(3,0,0)s=[EAX*8] b=EAX
//0000:00BA 00 84 C0 0F 84 93 00 00  00 8B 2D 00 00 00 00 8A  ..........-.....
                           TBD;
                        }
                        else  // if( dwMod == 3 )
                        {
                           // effect.addr
                           TBD;
                        }
                     }
                     else
                     {
                        sprintf(lpto, szBPs, lpmods );
                     }
                     prto   = lpto;
                  }
                  else if( dwst == st_Disp )
                  {
                     // ie dwRM=4 & dwMod!=3
// FAILED with (r=/r t=r32,m) 4C(1,1,4) m=disp8[.][.] to=ECX SIB10(0,2,0)s=[EDX]
//   0000023E: 8D 4C 10 02        lea         ecx,[eax+edx+2]
//  { 0x08D, NO_OP2, NO_PRE, "/r",     "LEA",          "r32,m",                0,0,0 },
                     // use the SIB bytes
                     // we have a SIB byte
                     if( !bDnSIB )
                     {
                        BUMP1;   // use up the SIB byte
                        bDnSIB = TRUE;
                     }
                     strcpy(lpfrom, "[");
                     dwo = GETBASE(dwSIB);
                     //strcat(lpfrom, sRegSet[dwo].sRegt[giActMode].pName);
                     if( dwo == 5 )
                     {
                        // NOTE: The SIB Base nomenclature of "[*]" means a disp32
                        // with no base id MOD is 00, [EBP], otherwise
                        if( dwMod != 0 )
                        {
                           strcat(lpfrom, "EBP" );
                        }
                     }
                     else
                     {
                        strcat(lpfrom, sSIBBase[dwo] );
                     }

                     // now, this
                     //strcat(lpfrom, sRegSet[GETIND(dwSIB)].sRegt[giActMode].pName);
                     // or this
                     //if( lpsibs && *lpsibs )
                     if( GETIND(dwSIB) != 4 )
                     {
                        strcat(lpfrom, "+");
                        strcat(lpfrom, &lpsibs[1]);
                     }
                     // now done only if dwo>0 - lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;

                     BUMP1;   // bump to next byte
                     if( dwMod == 1 )     // base case
                     {
                        pb = &pCode[dwi];
                        dwo = (DWORD)*pb; // get next byte, an additive item
                        pc = (char *)pb;
                        i = *pc;
// FIX20010729-02 - minor - release\dc4w.obj - see INTEL-17.TXT
// 0000007D: 8D 4C 24 00          LEA         ECX, [ESP+00H]
// should have
//  00409D4D 8D 4C 24 00          lea         ecx,[esp]
                        if(dwo)
                        {
                           if( GETIND(dwSIB) != 4 )
                              lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;   // kill ']' tail

                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf(EndBuf(lpfrom), "-%02XH]", dwo );
                           }
                           else
                           {
                              sprintf(EndBuf(lpfrom), "+%02XH]", dwo );
                           }
                        }
                        else if( GETIND(dwSIB) == 4 )
                        {
                           strcat(lpfrom,"]");  // no SIB register, so CLOSE braces
                        }
                        prfrom = lpfrom;
                     }
                     else if( dwMod == 2 )   // disp32
                     {
// FIX20010701-06 = FIXED
// get
// 00000028: 8D 8C 10 F8          LEA         ECX, [EAX+EDX-08H]
// 0000002C: 00 00                ADD         BYTE PTR [EAX], AL
// 0000002E: 00 89 4D F8 83 7D    ADD         BYTE PTR [ECX+7D83F84DH], [EAX+EDX-
// 00000034: F8                   CLC
// 00000035: 00 74                ADD         BYTE PTR disp8[.][.], DH
// should be
//00418668 8D 8C 10 F8 00 00 00 lea         ecx,[eax+edx+0F8h]
//0041866F 89 4D F8             mov         dword ptr [ebp-8],ecx
//00418672 83 7D F8 00          cmp         dword ptr [ebp-8],0
// { 0X8D, NO_OP2,  NO_PRE, "/r",   "LEA",       "r32,m",    0,0,0 }
//ERROR: (r=/r t=r32,m) MRM=8C(2,1,4) m=disp32[.][.] to=ECX
//        SIB10(0,2,0)s=[EDX] b=EAX
//0000:002C 00 00 00 89 4D F8 83 7D  F8 00 74 3E C7 45 F0 00  ....M.
                        TEST3;
                        pdw = (PDWORD) &pCode[dwi];
                        dwo = *pdw;
                        *lpb = 0;
                        pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                        if(pir)
                        {
                           ADD2ARRAY( dwa, pir );

                           if( GETIND(dwSIB) != 4 )
                              lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;   // kill ']' tail

                           sprintf(EndBuf(lpfrom), "+%s]", lpb);
                        }
                        else if(dwo)
                        {
                           GetMinHex( lpb, dwo );

                           if( GETIND(dwSIB) != 4 )
                              lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;   // kill ']' tail

                           sprintf(EndBuf(lpfrom), "+%s]", lpb);
                        }
                        else if( GETIND(dwSIB) == 4 )
                        {
                           strcat(lpfrom,"]");
                        }
                        prfrom = lpfrom;
                        dwi += 3;
                     }
                     else if( dwMod == 0 )
                     {
// FIX20010701-08 (OnLoadDLL)
// 0000032B: 8B 55 84             MOV         EDX, DWORD PTR [EBP-7CH]
//#define  DBGSTOP  DBS( 0X32E, 0X8D, 0X4, 0X8A )
//FAILED 0X331 0X8D 0X4 0X8A!
// { 0X8D, NO_OP2,  NO_PRE, "/r",   "LEA",       "r32,m",    0,0,0 }
//ERROR: (r=/r t=r32,m) MRM=04(0,0,4) m=[.][.] to=EAX
//        SIB8A(2,1,2)s=[ECX*4] b=EDX
//0000:032E 8D 04 8A 89 85 68 FE FF  FF 8D 8D 7C FF FF FF 51  .....h..
                        // and
// FIX20010726-01 - File: DEBUG\DC4W.OBJ
//SECTION #38 (0x26) CODE for _InitWork (Len=462).
//#define  DBGSTOP  DBS( 0XB5, 0X8D, 0XC, 0XC5 )
//FAILED 0XB8 0X8D 0XC 0XC5!
// { 0X8D, NO_OP2,  NO_PRE, "/r",   "LEA",       "r32,m",    0,0,0 }
//ERROR: (r=/r t=r32,m) MRM=0C(0,1,4) m=[.][.] to=ECX
//   SIBC5(3,0,5)s=[EAX*8] b=[*]
//0000:00B5 8D 0C C5 80 02 00 00 8B  55 FC 89 0C D5 80 02 00  ......
// should be
// 0040F3C5 8D 0C C5 C0 B9 4E 00 lea         ecx,[eax*8+4EB9C0h]
// 0040F3CC 8B 55 FC             mov         edx,dword ptr [ebp-4]

// but NOT correct for
// FIX20010728-5 - see INTEL-15.TXT - release\dc4w.obj - Getpis()
//#define  DBGSTOP  DBS(0xDB,0x8D,0x2C,0x16)  // 3C 20 0F 8E
// i get
//000000DB: 8D 2C 16 3C 20 0F 8E LEA         EBP, [ESI+EDX+08E0F203CH]
// should be
// 0040912B 8D 2C 16             lea         ebp,[esi+edx]
// 0040912E 3C 20                cmp         al,20h
//#define  DBGSTOP  DBS( 0XDB, 0X8D, 0X2C, 0X16 )
//FAILED 0XE1 0X8D 0X2C 0X16! (Line=0)
// { 0X8D, NO_OP2,  NO_PRE, "/r",   "LEA",       "r32,m",    0,0,0 }
//ERROR: (r=/r t=r32,m) MRM=2C(0,5,4) m=[.][.] to=EBP
//	SIB16(0,2,6)s=[EDX] b=ESI I get. Try only disp32 when SIB-BASE=5
                        if( GETBASE(dwSIB) == 5 )
                        {
                           // base of [*] a disp32 with NO base
                           TEST3;
                           pdw = (PDWORD) &pCode[dwi];
                           dwo = *pdw;
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );

                              if( GETIND(dwSIB) != 4 )
                                 lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;   // kill ']' tail

                              sprintf(EndBuf(lpfrom), "+%s]", lpb);
                              if(dwo)
                              {
                                 GetMinHex( lpb, dwo );
                                 sprintf( lpcomm, "; [+%s]", lpb );
                              }
                           }
                           else if(dwo)
                           {
                              GetMinHex( lpb, dwo );

                              if( GETIND(dwSIB) != 4 )
                                 lpfrom[ (strlen(lpfrom) - 1 ) ] = 0;   // kill ']' tail

                              sprintf(EndBuf(lpfrom), "+%s]", lpb);
                           }
                           else if( GETIND(dwSIB) == 4 )
                           {
                              strcat(lpfrom,"]");  // close braces
                           }
                           dwi += 3;
                        }
                        else
                        {
                           // checks ok in Getpis(), but want other checks
                           sprtf( "CHECKME-B~5: No displacement added!"MEOR );

                           if( GETIND(dwSIB) == 4 )   // no SIB added
                              strcat(lpfrom,"]");     // so close the braces

                           dwi--;   // back up one
                        }
                        prfrom = lpfrom;
                     }
                     else
                     {
                        chkme( "WARNING: Perhaps this should work for dwMod=3!");
                        TBD;
                     }
                  }
                  else
                  {
                     // what else has to be done?????
                     NYD;
                  }

                  if( strcmp( pt, "r/m32,r32" ) == 0 )
                  {
                     // exchange "to" and "from"
                     lps = prto;
                     prto = prfrom;
                     prfrom = lps;
                  }
               }
               else if( (dwrl > 1) && (pr[0] == '/') && ( ISNUMERIC(pr[1] ) ) )
               {
                  // NEXT BYTE IS ModR/M BYTE
                  // ************************
                  // /digit-A digit between 0 and 7 indicates that the 
                  // ModR/M byte of the instruction uses only the r/m
                  // (register or memory) operand. The reg field 
                  // contains the digit that provides an extension 
                  // to the instruction's opcode.
//{0x083, NO_OP2, NO_PRE, "/5 ib",  "SUB",          "r/m32,imm8",           0,0,0 },
//{0x083, NO_OP2, NO_PRE, "/2 ib",  "ADC",          "r/m16,imm8",           0,0,0 },
                  // NOTE: The value of the REGISTER operand MUST equal the /digit!!!
                  //dwn = pr[1] - '0';   // gt the value
                  //dwt = (DWORD)pCode[dwi+1];
                  //dwRM  = GETRM(dwt);     // get r/m operand
                  //if( dwn != dwRM )
                  //dwOp  = GETOP(dwt);     // get REGISTER operand
                  //if( dwn != dwOp )
                  //   continue;   // keep this dwo op1 value and CONTINUE down table
                  BUMP1;   // bump to next byte
                  pb = &pCode[dwi];
                  dwMRM = (DWORD)*pb; // get the ModR/M byte
                  dwMod = GETMOD(dwMRM);
                  dwOp  = GETOP(dwMRM);     // get REGISTER operand
                  dwRM  = GETRM(dwMRM);
                  //prs = &pregset[dwOp];   // note dwOp IS in range!
                  prs = &pregset[dwRM];   // &sRegSet[0] note dwRM IS in range!
                  if( dwFlag & flg_SIZE )
                  {
                     prto = prs->sRegt[giAltMode].pName; // get the relevant register
                     //sprtf( "CHECKME-TO: /n Early selection per flg_SIZE! (to=%s)"MEOR, prto );
                  }
                  else
                  {
                     prto = prs->sRegt[giActMode].pName; // get the relevant register
                  }

                  lpmods = pmrm[dwMod].pRM[dwRM];  // = MODRM2 sModRM32;
                  //if( ( iPos = InStr(prto, "/") ) > 0 )
                  //   prto = Left(prto, (iPos - 1));
                  // ************************************************************
                  //if( InStr(lpmods, "[.][.]") || InStr(lpmods, "disp32") || InStr(lpmods, "disp8") )
                  if( (iPos2 = InStr(lpmods, "[.][.]")) > 0 )
                  {
                     dwst = st_Disp;
                     if( dwRM != 4 )
                     {
                        // defensive code only
                        sprtf( "WARNING: Documentation note that [.][.] means"MEOR
                           "\ta SIB byte follows APPEARS ERRANT!"MEOR );
                        DBGEXIT;
                     }
                  }
                  else if( (iPos2 = InStr(lpmods, "disp32")) > 0 )
                  {
                     dwst = st_Disp32;
                     if( dwMod != 2 )
                     {
                        // defensive code only
                        if( !( ( dwMod == 0 ) && ( dwRM == 5 ) ) )  // the 1st group exception
                        {
                           sprtf( "WARNING: Documentation note that disp32 means"MEOR
                              "\tan Effective Address in 3rd group APPEARS ERRANT!"MEOR );
                           DBGEXIT;
                        }
                     }
                  }
                  else if( (iPos2 = InStr(lpmods, "disp8")) > 0 )
                  {
                     dwst = st_Disp8;
                     if( dwMod != 1 )
                     {
                        // defensive code only
                        sprtf( "WARNING: Documentation note that disp8 means"MEOR
                           "\tan Effective Address in 2nd group APPEARS ERRANT!"MEOR );
                        DBGEXIT;
                     }
                  }
                  
                  if( dwst == st_None )
                  {
//#ifdef   ADDMODRM2
                     if( ( DBGSTOP ) &&   // 0x37 or ...
                         ( !bDoExit ) )
                     {
                        // NOT one of the special [.][.] or disp? Effective Addresses
                        sprintf( lpd, "ModR/M !D (r=%s t=%s) "      // 1,2
                           "%02X(%d,%d,%d) "                // 3,4,5,6
                           "m=%s to=%s"MEOR,               // 7,8
                           pr, pt,                   // 1, 2
                           dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
                           lpmods, prto );                // 7,8
                        sprtf(lpd);
                     }
//#endif   // #ifdef   ADDMODRM2
                  }
                  else  // if(dwst != st_None)
                  {
                     TEST1;

                     dwSIB = (DWORD)pb[1];   // Extract SIB byte
                     // get the Scaled Index per the SS and Index bits
                     lpsibs = sSIB32[GETSS(dwSIB)].pRM[GETIND(dwSIB)];
                     //lpsbase = sSIB32[0].pRM[GETBASE(dwSIB)];
                     // OR
                     if( ( dwRM == 4 ) && ( dwMod != 3 ) )
                     {
                        lpsbase = sSIBBase[GETBASE(dwSIB)];
                     }
                     // **********************************************
//#ifdef   ADDMODRM2
                     if( ( DBGSTOP  ) &&   // 0x37 or ...
                         ( !bDoExit ) )
                     {
                        if( ( dwRM  == 4 ) &&
                            ( dwMod != 3 ) )
                        {
                           sprintf( lpd, "mrm (r=%s t=%s) "      // 1,2
                              "%02X(%d,%d,%d) "                // 3,4,5,6
                              "m=%s to=%s "MEOR                 // 7,8
                              "\tSIB[%02X]"                       // 9
                              "(%d,%d,%d)"                    // 10,11,12
                              "si=%s b=%s"MEOR,                       // 13, 14
                              pr, pt,                   // 1, 2
                              dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
                              lpmods, prto,                    // 7,8
                              dwSIB,                           // 9
                              GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
                              lpsibs,                    // 13
                              lpsbase );  // 14 = sSIBBase[GETBASE(dwSIB)]
                        }
                        else
                        {
                           sprintf( lpd, "mrm (r=%s t=%s) "      // 1,2
                              "%02X(%d,%d,%d) "                // 3,4,5,6
                              "m=%s to=%s NoSIB"MEOR,            // 7,8
                              pr, pt,                   // 1, 2
                              dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
                              lpmods, prto );              // 7,8
                              //dwSIB,                           // 9
                              //GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
                              //lpsibs,                    // 13
                              //sSIBBase[GETBASE(dwSIB)] ); // 14 lpsbase
                        }
                        sprtf(lpd);
                     }
//#endif   // #ifdef   ADDMODRM2
                  }
                  // *************************************************************

                  if( ( dwRM  == 4 ) &&
                      ( dwMod != 3 ) )
                  {
                     BUMP1;   // use up the SIB byte
                     //sprtf( "CHECKME: New CODE path FIX20010619 - Done SIB bump"MEOR );
                     bDnSIB = TRUE; // already DONE SIB byte increment
                  }

                  // get AFTER the /digit
                  lps = Right(pr, (strlen(pr) - 3) );
                  if( strlen(lps) >= 2 )
                  {
                     // ok, we have some more AFTER the /digit
                     if( ( lps[0] == 'i' ) && (lps[1] == 'b') )
                     {
                        // sub-string AFTER /digit IF "ib"
                        // IMMEDIATE BYTE
                        BUMP1;   // bump to next byte
                        pb = &pCode[dwi];
                        dwo = (DWORD)*pb; // get the next byte
                        pc = (char *)pb;
                        i = *pc;          // get the signed value
//{0x083, NO_OP2, NO_PRE, "/7 ib",  "CMP",          "r/m32,imm8",           0,0,0 },
                        if( strcmp( pi->pNm, "CMP" ) == 0 )
                        {
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
// FIX20010727-03 - DEBUG\DC4W.obj - see INTEL-10.txt
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }
//mrm (r=/7 ib t=r/m8,imm8) BD(2,7,5) m=disp32[EBP] to=EBP NoSIB
// I get
//0000015D 80 BD 7C FE FF FF 25 CMP         DWORD PTR [EBP-0184H], 37 ; Chr '%' (0x25)
// Should be
//00412D8D 80 BD 7C FE FF FF 25 cmp         byte ptr [ebp-184h],25h
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }

// still error on
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }
//ModR/M !D (r=/7 ib t=r/m8,imm8) FB(3,7,3) m=EBX/BX/BL/MM3/XMM3 to=EBX
// 0000012E: 80 FB 20             CMP         EBX, 32     ; Chr <space> (0x20)
// 00000131: 7C 13                JL          [_Getpis + 0146H]
// 00000133: 80 F9 22             CMP         ECX, 34     ; Chr '"' (0x22)
//#define  DBGSTOP  DBS( 0X133, 0X80, 0XF9, 0X22 )
//FAILED 0X135 0X80 0XF9 0X22! (Line=0)
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }
//ERROR: (r=/7 ib t=r/m8,imm8) MRM=F9(3,7,1) m=ECX/CX/CL/MM1/XMM1 to=ECX NOSIB
                           if( InStr(pt, "r/m8") )
                           {
                              strcpy(lpd, szBPb );
                           }
                           else
                           {
                              strcpy(lpd, szDPb);
                           }
                           if( dwst == st_Disp32 ) // InStr(lpmods, "disp32") == 1 )
                           {
                              TEST3;
                              // ie dwMod == 2
                              // build a disp32 effective address
                              pdw = (PDWORD)pb;
                              dwo = *pdw;
                              pint = (PINT)pdw;
                              i = *pint;
                              if(pir)
                              {
                                 ADD2ARRAY( dwa, pir );
                                 //strcpy(lpd, "[");
                                 //strcat(lpd, lpb);
                                 //strcat(lpd, "]");
                                 sprintf( EndBuf(lpd), "%s]", lpb );
                                 pir = 0;
                              }
                              else
                              {
//ModR/M (r=/7 ib t=r/m32,imm8) B8(2,7,0) m=disp32[EAX] to=EAX SIB80(2,0,0)s=[EAX*4]
// 0000025C: 83 B8 80 00 00 00 00 CMP         DWORD PTR [00000080], 0
//  0000025C: 83 B8 80 00 00 00  cmp         dword ptr [eax+80h],0
                                 if( dwRM == 4 )
                                 {
                                    chkme( "CHECKME-U2: Unhandled case dwMod=2 RM=[.][.]!"MEOR );
                                    NYD;
                                 }
                                 //strcpy( lpd, "DWORD PTR ");
                                 //strcat( lpd, Right(lpmods,5) );
                                 strcat( lpd, Right(lpmods,4) );
                                 if( i < 0 )
                                 {
                                    dwo = ~(dwo) + 1;
                                    sprintf( &lpd[(strlen(lpd)-1)], "-0%XH]", dwo );
                                 }
                                 else
                                 {
                                    sprintf( &lpd[(strlen(lpd)-1)], "+0%XH]", dwo );
                                 }
                                 //sprintf( &lpb[ (strlen(lpb) - 1) ], "+0%XH]", dwo );
                              }
                              dwi += 3;
                           }
                           else if( dwst == st_Disp8 )   //InStr(lpmods, "disp8") == 1 )
                           {
                              // already loaded first byte
                              // build a disp8 + register address
                              if( dwRM == 4 )
                              {
                                 chkme( "CHECKME-U1: Unhandled case dwMod=1 RM=[.][.]!"MEOR );
                                 NYD;
                              }
                              strcat( lpd, prto );
                              if( i < 0 )
                              {
                                 dwo = (0x100 - dwo);
                                 sprintf( EndBuf(lpd), "-%02XH]", dwo );
                              }
                              else
                              {  
                                 sprintf( EndBuf(lpd), "+%02XH]", dwo );
                              }
                           }
                           else if(dwMod == 0)
                           {
                              // simply compare REGISTER with imm8
// FIX20010727-05 - see INTEL-11.TXT
// i get
// 0000011E: 83 3A 00             CMP         EDX, 0
// should be
//  0041453E 83 3A 00             cmp         dword ptr [edx],0
// { 0X83, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m32,imm8", 0,0,0 }
//ModR/M !D (r=/7 ib t=r/m32,imm8) 3A(0,7,2) m=[EDX] to=EDX
                              //strcpy(lpd, prto);
                              strcat(lpd, Right(lpmods,4));
                              // worked in several cases, so
                              //sprtf( "CHECKME2: Altered code from prto=[%s] to lpmods=[%s]!"MEOR,
                              //   prto, lpmods );
                              dwi--;   // decrement BACK to compare byte
                           }
                           else  // dwMod == 3 Effective Address
                           {
// FIX20010728-7 - INTEL-15.txt - release\dc4w.obj - Getpis()
// #define  DBGSTOP  DBS(0x12e, 0x80, 0xfb, 0x20)
// i get
// 0000012E: 80 FB 20             CMP         EBX, 32     ; Chr <space> (0x20)
// 00000131: 7C 13                JL          [_Getpis + 0146H]
// 00000133: 80 F9 22             CMP         ECX, 34     ; Chr '"' (0x22)
// 00000136: 74 05                JE          [_Getpis + 013DH]
// 00000138: 80 FB 3B             CMP         EBX, 59     ; Chr ';' (0x3b)
// should be
//  0040917E 80 FB 20             cmp         bl,20h
//  00409181 7C 13                jl          00409196
//  00409183 80 F9 22             cmp         cl,22h
//  00409186 74 05                je          0040918D
//  00409188 80 FB 3B             cmp         bl,3Bh
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }
//ModR/M !D (r=/7 ib t=r/m8,imm8) FB(3,7,3) m=EBX/BX/BL/MM3/XMM3 to=EBX
//#define  DBGSTOP  DBS( 0X133, 0X80, 0XF9, 0X22 )
// { 0X80, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m8,imm8", 0,0,0 }
//ERROR: (r=/7 ib t=r/m8,imm8) MRM=F9(3,7,1) m=ECX/CX/CL/MM1/XMM1 to=ECX NOSIB
                              //strcpy(lpd, szBPb );
                              //if( InStr(lpd,szBPb) == 1 )
                              if( InStr(pt, "r/m8") )
                              {
                                 strcpy(lpd, sMtchReg[dwRM].pnm[rs_08] );
                              }
                              else
                              {
                                 strcpy(lpd, prto);
                              }
                              dwi--;   // decrement BACK to compare byte
                              // check in several case, and appears OK, so
                              //sprtf( "CHECKME3: dwMod=3 Effective Address!"MEOR );
                              //TBD;
                           }

                           strcpy( lpto, lpd );    // move it to the "to" buffer
// Should be: 00513BE8   cmp         dword ptr [edx+ecx*4],0
// 00000158: 83 3C 8A             CMP         ESP, -118   ; 0x8a
//FAILED 0X15A 0X83 0X3C 0X8A!
// { 0X83, NO_OP2,  NO_PRE, "/7 ib", "CMP",       "r/m32,imm8", 0,0,0 }
//ERROR: (r=/7 ib t=r/m32,imm8) MRM=3C(0,7,4) m=[.][.] to=ESP
//   SIB8A(2,1,2)s=[ECX*4] b=EDX
//0000:015B 00 74 1E 8B 45 DC 8B 4D  10 8B 14 81 52 8B 45 DC  .t..E..M.
                           if( ( dwMod == 0 ) && ( dwRM == 4 ) )
                           {
                              // SIB byte give the register set
                              if( GETBASE(dwSIB) == 5 )
                              {
                                 chkme( "CHECKME-NH: This is NOT supposed to happen"MEOR
                                    "Base=5 is a NULL (ie no register)!" );
                                 // NOTE: The SIB Base nomenclature of "[*]", ie 5
                                 // means a disp32 with no base if dwMod = 0,
                                 // else [EBP] otherwise
                                 NYD;
                              }
                              sprintf( lpto, "DWORD PTR [%s+", sSIBBase[GETBASE(dwSIB)] );
                              strcat(  lpto, &lpsibs[1] );
                           }
                           if(pir)
                           {
                              chkme( "Relocation item NOT being USED!"MEOR );
                           }

                           BUMP1;   // bump to next byte
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get the next byte
                           pc = (char *)pb;
                           i = *pc;          // get the signed value
                           sprintf(lpfrom, "%d", i );
                           if( isprint(i) )
                           {
                              if( i == ' ' )
                                 sprintf( lpcomm, "; Chr <space> (%#x)", i );
                              else
                                 sprintf( lpcomm, "; Chr '%c' (%#x)", i, i );
                           }
                           else if( ( i < 0 ) || ( i > 9 ) )
                           {
                              if( i == '\r' )
                                 sprintf( lpcomm, "; <CR> (%#x)", dwo );
                              else if( i == '\n' )
                                 sprintf( lpcomm, "; <LF> (%#x)", dwo );
                              else if( i == '\t' )
                                 sprintf( lpcomm, "; <TAB> (%#x)", dwo );
                              else
                                 sprintf( lpcomm, "; %#x", dwo );
                           }

                           prto = lpto;
                           prfrom = lpfrom;

                        }  // if NOT the "CMP" case
                        else if( dwst == st_Disp8 )
                        {
                           // same as (dwMod=1 & dwRM!=4)
//ModR/M (r=/0 ib t=r/m8,imm8) 45(1,0,5) m=disp8[EBP] to=EBP SIBDC(3,3,4)s=[EBX*8]
// 00000064: C6 45 DC             MOV         BYTE PTR [EBP], DCH! Should be
// 00000064: C6 45 DC 00        mov         byte ptr [ebp-24h],0
                           sprintf(lpto, szBPbs, prto );
                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf(EndBuf(lpto), "-%02XH]", dwo );
                           }
                           else
                           {
                              sprintf(EndBuf(lpto), "+%02XH]", dwo );
                           }
                           prto = lpto;
                           BUMP1;   // move up to the immediate 8-bits
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get the next byte
                           sprintf(lpfrom, "%02XH", dwo );
                           prfrom = lpfrom;
                        }
                        else // is NOT "CMP", nor (dwMod=1 & dwRM!=4), done above
                        {
                           // and IS "/n ib", for immediate BYTE
// FIX20010727-06 - see INTEL-12.TXT
// i get
// 0000015B: C6 44 11 FF          MOV         BYTE PTR [ESP], FFH
// 0000015F: 00 E9                ADD         EBP, CL
// 00000161: 14 FF                ADC         AL, FFH
// should be
//  0041457B C6 44 11 FF 00       mov         byte ptr [ecx+edx-1],0
//  00414580 E9 14 FF FF FF       jmp         do_getdata+79h (00414499)
// { 0XC6, NO_OP2,  NO_PRE, "/0 ib", "MOV",       "r/m8,imm8", 0,0,0 }
//ERROR: (r=/0 ib t=r/m8,imm8) MRM=44(1,0,4) m=disp8[.][.] to=BYTE PTR [ESP]
//	SIB11(0,2,1)s=[EDX] b=ECX - note the dwRM = 4 case
                           //if( ( dwMod != 3 ) && ( dwRM == 4 ) && ( GETBASE(dwSIB) != 5 ) )
                           if( ( dwMod != 3 ) && ( dwRM == 4 ) )
                           {
                              // then SIB gives the register
                              // *lpb = 0;
                              //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                              if( GETBASE(dwSIB) == 5 )
                              {
                                 sprtf( "CHECKME-RM44-5: New handling of this case!"MEOR );
                                 // add the offset as well
                                 // put in BASE
                                 if( dwMod == 0 )
                                 {
                                    // no base - Add SIB register
                                    sprintf(lpto, szBPbs,
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                                 }
                                 else
                                 {
                                    sprintf(lpto, szBPbs, "EBP");
                                    // Add SIB register
                                    sprintf(EndBuf(lpto),
                                       "+%s",
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                                 }
                                 if(dwo)
                                 {
                                    // add/subtract (BYTE) offset (if any)
                                    if( i < 0 )
                                    {
                                       dwo = (0x100 -dwo);
                                       lpform = "-%s]";
                                    }
                                    else
                                    {
                                       lpform = "+%s]";
                                    }

                                    GetMinHex(lpd, dwo);
                                    sprintf(EndBuf(lpto), lpform, lpd );
                                 }
                                 else
                                 {
                                    strcat(lpto,"]");
                                 }

                                 BUMP1;   // move up to the immediate 8-bits
                                 pc = (char *)&pCode[dwi];
                                 dwo = (DWORD)*pc;
                                 i  = *pc; // get the next byte

                              }
                              else
                              {
                                 sprtf( "CHECKME-RM44~5: New handling of this case!"MEOR );
                                 // add the offset as well
                                 // put in BASE
                                 sprintf(lpto, szBPbs, lpsbase ); // = sSIBBase[GETBASE(dwSIB)]
                                 // Add SIB register
                                 sprintf(EndBuf(lpto),
                                    "+%s]",
                                    Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                              }

                              prto = lpto;   // set DST

                              sprintf(lpfrom, "%d", i );
                              prfrom = lpfrom;  // set SRC

                           }
                           else if( dwop1 == 0xc6 )
                           {
                              sprintf(lpto, szBPbsb, prto );
                              prto = lpto;
                              sprintf(lpfrom, "%02XH", dwo );  // can ONLY use 8-bit value
                              prfrom = lpfrom;
                           }
                           else if( ( dwMod == 3 ) &&   // like dwst == st_None
                              ( InStr( pt, "r/m8" ) ) )
                           {
                              // not a displacement
                              prto = sMtchReg[dwRM].pnm[rs_08];
                              sprintf(lpfrom, "%02XH", dwo );  // can ONLY use 8-bit value
                              prfrom = lpfrom;
                           }
                           else
                           {
// FIX20010727-01 - DEBUG\DC4W.obj - see INTEL-10.txt
// I get
//00000067: 83 C8 FF             OR          EAX, FFH
// should be
//2245:            return -1;
//00412C97 83 C8 FF             or          eax,0FFFFFFFFh
// { 0X83, NO_OP2,  NO_PRE, "/1 ib", "OR",        "r/m32,imm8", 0,0,0 }
//ERROR: (r=/1 ib t=r/m32,imm8) MRM=C8(3,1,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
//0000:006A E9 E9 03 00 00 8B 45 F8  50 8B 4D F0 51 E8 00 00  ......E.P.M.Q...
// this is ok, we have some more AFTER the /digit, like
// if( ( lps[0] == 'i' ) && (lps[1] == 'b') )
// so, instead of
                              sprintf(lpfrom, "%02XH", dwo );  /* let's try */
                              *lpfrom = 0;
                              GetMinHex(lpfrom,i); // passed the SIGNED value
                              prfrom = lpfrom;
                           }
                        }
                     }
                     else if( ( lps[0] == 'i' ) && (lps[1] == 'd') )
                     {
                        // sub-string AFTER /digit if "id"
// ModR/M (r=/0 id t=r/m32,imm32) 45(1,0,5) m=disp8[EBP] to=EBP SIBDC(3,3,4)s=[EAX*2]
//  00000037: C7 45 DC 00 00 00 00 mov         dword ptr [ebp-24h],offset _OutFunList
// this is NOT  00000037 C7 45 DC 00 00 00     MOV         EBP, 000000DCH
                        if( ( dwst == st_Disp8 ) && (InStr( pt, "imm32" )) )
                        {
                           // ie dwMod = 1
// FIX20010701-01
// 0000000C: 66 C7 45 F8 00 00 EB
//           0C                   MOV         DWORD PTR [EBP-08H], 216727552
//WARNING: SIZE PRE-OP FLAG NOT USED!
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
                           BUMP1;   // bump to offset
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get the next byte
                           pc = (char *)pb;
                           i = *pc;          // get the signed value
                           if( dwFlag & flg_SIZE )
                              strcpy( lpto, szWP);
                           else
                              strcpy( lpto, szDP);

                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf(EndBuf(lpto), "[%s-%02XH]", prto, dwo );
                           }
                           else
                           {  
                              sprintf(EndBuf(lpto), "[%s+%02XH]", prto, dwo );
                           }
                           BUMP1;
                           if( dwFlag & flg_SIZE )
                           {
                              TEST1;
                              pword = (PWORD) &pCode[dwi];
                              dwo = *pword;
                              dwi += 1;
                              dwFlag &= ~(flg_SIZE);
                           }
                           else
                           {
                              TEST3;
                              pdw = (PDWORD) &pCode[dwi];
                              dwo = *pdw;
                              dwi += 3;
                           }
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              if(dwo)
                              {
                                 sprintf( lpfrom, "OFFSET [%s + %d]", dwo );
                                 chkme( "WARNING: New coding adding offset!!!"MEOR );
                              }
                              else
                                 sprintf( lpfrom, "OFFSET %s", lpb );
                           }
                           else
                           {
                              //pdw = (PDWORD) &pCode[dwi];
                              //dwo = *pdw;
                              sprintf( lpfrom, "%d", dwo );
                           }
                           prfrom = lpfrom;
                           prto   = lpto;
                        }
                        else if( ( dwst == st_Disp32 ) && (InStr( pt, "imm32" )) )
                        {
                           // NOTE: dwMod = 2, like
// (r=/0 id t=r/m32,imm32) mrm=29(2,0,2) m=disp32[EDX] to=[0000014CH] SIB4C(1,1,4)
// FAILED ON
// ModR/M (r=/0 id t=r/m32,imm32) 05(0,0,5) m=disp32 to=EBP SIB00(0,0,0)s=[EAX]
// 00000016: C7 05 00 00 00 00  mov         dword ptr [_ShowFHelp],1
//           01 00 00 00

// also failed on I get
//09F: 66 C7 80 C8 01 00 00
//     00 00 8B 4D          MOV         WORD PTR [EAX+01C8H], 1300955136
//0AA: F0                   LOCK
// should be
// 0043F73F   mov         word ptr [eax+1C8h],offset puGroup::puGroup+0A6h (0043f746)
// 0043F748   mov         ecx,dword ptr [ebp-10h]
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=80(2,0,0) m=disp32[EAX] to=WORD PTR [EAX+01CH]
//   SIBC8(3,1,0)s=[ECX*8] b=EAX
                           // ModR/M NOTES 2. says
                           // This disp32 denotes a 32-bit displacement following
                           // the SIB byte, to be add to the index.
// still error on
// FIX20010725-03 - File: DEBUG\DC4W.OBJ
//00000DA: EB C7                JMP         [_InitWork + 0A3H]
//00000E6: C7 05 5C 02 00 00 58
//          02 00 00             MOV         DWORD PTR [_sFW], offset _sFW
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=05(0,0,5) m=disp32 to=DWORD PTR [_sFW]
//   NoSIB5C(1,3,4)s=[EBX*2] b=ESP
//0000:00F0 C7 05 60 02 00 00 60 02  00 00 C7 05 64 02 00 00  ..`...`.....d
// should be
//0040F3EA EB C7                jmp         InitWork+0A3h (0040f3b3)
//395:     InitLList( &gsCompList );  // list of COMPARE items
//0040F3EC C7 05 98 B9 4E 00 98 mov         dword ptr [_sFW+258h (004eb998)],offset _sFW+258h (004eb998)
//0040F3F6 C7 05 9C B9 4E 00 98 mov         dword ptr [_sFW+25Ch (004eb99c)],offset _sFW+258h (004eb998)

// STILL ERROR ON
// FIX20010727-04 - see INTEL-10.TXT
// got
//00000345 66 C7 85 CE FE FF FF 
//           00 00 8D 8D          MOV         WORD PTR [EBP-0132H], 0
// 00000350: EC                   IN          AL,DX
// should be
//00412F75 66 C7 85 CE FE FF FF mov         word ptr [ebp-132h],offset do_editfile+34Ch (00412f7c)
// but this 'looks' strange - an offset INTO a word pointer???
// 00000345: 66 C7 85 CE FE FF FF 
//           00 00 8D 8D          MOV         WORD PTR [EBP-0132H], 0
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=85(2,0,5) m=disp32[EBP] to=WORD PTR [EBP-0132H]
//	NoSIBCE(3,1,6)s=[ECX*8] b=ESI
//0000:0350 EC FE FF FF 89 8D A8 FE  FF FF C7 85 A4 FE FF FF  ................
                           BUMP1;   // to offset
                           TEST3;
                           pdw = (PDWORD) &pCode[dwi];
                           dwo = *pdw;
                           pint = (PINT)pdw;
                           i = *pint;
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                           if( dwFlag & flg_SIZE )
                           {
                              // change to NOT-default operand size
                              sprtf( "WARNING: New code implemented - flg_SIZE(66H). CHECKME-S!!!"MEOR );
                              strcpy( lpto, szWP);
                              pword = (PWORD)pdw;
                              dwo = (DWORD)*pword; // extract the value
                              dwi += 1;
                              pshort = (short *)pdw;
                              i = (int)*pshort;
                              if( i < 0 )
                                 dwo = (( ~(dwo) + 1 ) & 0xffff );
                           }
                           else
                           {
                              strcpy( lpto, szDP);
                              dwi += 3;  // get to end of displacement
                              if( i < 0 )
                                 dwo = ~(dwo) + 1;
                           }
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              // Also prior to this there was lps = Right(lpmods, 5); // get the "[REG]" from the string
                              if(( dwMod == 0 ) &&
                                 ( dwRM  == 5 ) &&
                                 ( dwo   != 0 ) )
                              {
                                 // NOTE: using lpd for value
                                 *lpd = 0;
                                 GetMinHex( lpd, dwo );
                                 sprintf(EndBuf(lpto), szBSpSB, lpb, lpd );   //"[%s+%s]"
                              }
                              else
                              {
                                 sprintf( EndBuf(lpto), szBsB, lpb );   //"[%s]"  // no displacement
                              }
                           }
                           else
                           {
                              if( dwMod == 2 )
                              {
                                 // have already eliminated dwRM == 4 (Special [.][.] case, so
                                 lps = Right(lpmods, 5); // get the "[REG]" from the string
                                 sprintf(EndBuf(lpto), "%s", lps );
                                 //pint = (PINT)pdw;
                                 //i = *pint;
                                 if( i < 0 )
                                 {
                                    //dwo = ~(dwo) + 1;
                                    sprintf( &lpto[(strlen(lpto)-1)], "-0%XH]", dwo );
                                 }
                                 else
                                 {
                                    sprintf( &lpto[(strlen(lpto)-1)], "+0%XH]", dwo );
                                 }
                              }
                              else
                              {
                                 sprintf( EndBuf(lpto), "[%08XH]", dwo );
                              }
                           }

                           prto = lpto;   // set the "to"

                           BUMP1;      // bump to imm32 data
                           TEST3;
                           pdw = (PDWORD) &pCode[dwi];
                           dwo = *pdw;
                           //if( dwFlag & flg_SIZE )
                           //{
                           //   if( dwo > 0xffff )
                           //   {
                           //      chkme( "SIZE override where imm32 greater than WORD!"MEOR );
                           //      dwo &= 0xffff;    // dwo to WORD size
                           //   }
                           //}
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              // Also prior to this there was lps = Right(lpmods, 5); // get the "[REG]" from the string
                              if(( dwMod == 0 ) &&
                                 ( dwRM  == 5 ) &&
                                 ( dwo   != 0 ) )
                              {
                                 // NOTE: using lpd for value
                                 *lpd = 0;
                                 GetMinHex( lpd, dwo );
                                 sprintf( lpfrom, szBSpSB, lpb, lpd );
                              }
                              else
                              {
                                 sprintf( lpfrom, "offset %s", lpb );
                              }
                           }
                           else
                           {
                              sprintf( lpfrom, "%d", dwo );
                           }

                           prfrom = lpfrom;  // set the from
                           dwi += 3;

                           // remove USED size orver-ride flag
                           dwFlag &= ~( flg_SIZE );

                        }
                        else  // !st_Disp8 or st_disp32 with "imm32"
                        {
                           if( ( dwRM == 4 ) && ( dwMod != 3 ) )
                           {
                              if( !bDnSIB )
                              {
                                 BUMP1;
                                 bDnSIB = TRUE;
                              }
                           }

                           BUMP1;   // bump to offset
                           TEST3;
                           pdw = (PDWORD) &pCode[dwi];
                           dwo = *pdw;
                           pint = (PINT)pdw;
                           i = *pint;
                           *lpb = 0;
                           // does not seem cosha! but if it works
                           // was simply
                           //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                           // changed to
                           //if( bDnSIB )
                           //   pir = GetRelName( lpb, pRel, dwrel, pStgs, (dwi+1), 0 );
                           //else
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 1 );
// fails on
// FIX20010727-07 - see INTEL-12.TXT
// i get
// 00000137: C7 04 90 00 00 00 00 MOV         ESP, OFFSET [??_C@_05PNMN@dummy?$AA@]
// 0000013E: 8B 4D E4             MOV         ECX, DWORD PTR [EBP-1CH]
// should be
//  004104B7 C7 04 90 00 21 4C 00 mov         dword ptr [eax+edx*4],offset string "dummy" (004c2100)
//  004104BE 8B 4D E4             mov         ecx,dword ptr [icnt]
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=04(0,0,4) m=[.][.] to=ESP
//	SIB90(2,2,0)s=[EDX*4] b=EAX

// still fails on
// FIX20010728-2 - see INTEL-14.TXT
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=44(1,0,4) m=disp8[.][.] to=ESP
//	SIB24(0,4,4)s= b=ESP
// #define  DBGSTOP  DBS( 0X11, 0XC7, 0X44, 0X24 )
// I get
// 00000011: C7 44 24 0C FD FF FF MOV         ESP, FFFFFD0CH
// should be
// 004089D1 C7 44 24 0C FD FF FF mov         dword ptr [esp+0Ch],0FFFFFFFDh

                           //if( ( dwMod == 0 ) && ( dwRM == 4 ) && ( GETBASE(dwSIB) != 5 ) )
                           if( dwRM == 4 )   // && ( GETBASE(dwSIB) != 5 ) )
                           {
                              // must FIX the DST - the SIB gives the register
                              if( dwMod == 0 )
                              {
                                 strcpy(lpto, szDP);  // establish DWORD PTR
                                 sprtf( "CHECKME-RM4-0: New handling of this case!"MEOR );
                                 if( GETBASE(dwSIB) == 5 )
                                 {
                                    // then NO base since dwMod=0
                                    sprintf(EndBuf(lpto),
                                       szBsB,   // "[%s]"
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                                 }
                                 else
                                 {
                                    // just put in BASE and SIB register
                                    sprintf(EndBuf(lpto),
                                       szBSpSB,
                                       lpsbase,    // = sSIBBase[GETBASE(dwSIB)],
                                       Mid( lpsibs, 2, (( GETSS(dwSIB) == 0 ) ? 3 : 5 )) );
                                 }
                                 prto = lpto;
                              }
                              else if( dwMod == 1 )
                              {
                                 // disp8
                                 strcpy(lpto, szDP);  // establish DWORD PTR
                                 // this appear to check-out OK, so
                                 //sprtf( "CHECKME-RM4-1: New handling of this case!"MEOR );
                                 pc = (char *)pdw; // get the displacement
                                 dwo = (DWORD)*pc;
                                 i  = *pc;
                                 if( i < 0 )
                                 {
                                    dwo = 0x100 - dwo;
                                    lpform = szBSmSB;
                                 }
                                 else
                                 {
                                    lpform = szBSpSB;
                                 }
                                 GetMinHex(lpd, dwo);

                                 if( GETBASE(dwSIB) == 5 )
                                    sprintf(EndBuf(lpto), lpform, "EBP", lpd);
                                 else
                                    sprintf(EndBuf(lpto), lpform, lpsbase, lpd);

                                 BUMP1;
                                 TEST3;
                                 pdw = (PDWORD) &pCode[dwi];
                                 dwo = *pdw;
                                 pint = (PINT)pdw;
                                 i = *pint;
                                 if( !pir )
                                 {
                                    *lpb = 0;
                                    pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                                 }
                                 prto = lpto;
                              }
                              else if( dwMod == 2 )
                              {
                                 // disp32
                                 strcpy(lpto, szDP);  // establish DWORD PTR
                                 // appears OK, so
                                 //sprtf( "CHECKME-RM4-2: New handling of this case!"MEOR );
                                 if(dwo)
                                 {
                                    if( i < 0 )
                                    {
                                       dwo = ( ~(dwo) + 1 );
                                       lpform = szBSmSB;
                                    }
                                    else
                                    {
                                       lpform = szBSpSB;
                                    }
                                    GetMinHex(lpd, dwo);
                                 }
                                 else
                                 {
                                    lpform = szBsB;   // "[%s]"
                                 }

                                 if( GETBASE(dwSIB) == 5 )
                                    sprintf(EndBuf(lpto), lpform, "EBP", lpd);
                                 else
                                    sprintf(EndBuf(lpto), lpform, lpsbase, lpd);

                                 dwi += 3;
                                 BUMP1;   // bump to offset
                                 TEST3;
                                 pdw = (PDWORD) &pCode[dwi];
                                 dwo = *pdw;
                                 pint = (PINT)pdw;
                                 i = *pint;
                                 if( !pir )
                                 {
                                    *lpb = 0;
                                    pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                                 }
                                 prto = lpto;
                              }
                              // else dwMod = 3, and nothing to do
// broke this case
// 000000B4: 81 C4 D0 00 00 00    ADD         dword ptr , 0D0H
// { 0X81, NO_OP2,  NO_PRE, "/0 id", "ADD",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=C4(3,0,4) m=ESP/SP/AH/MM4/XMM4 to=ESP  NOSIB
                           }
                           else if( dwMod == 0 )
                           {
// and yet another on
// FIX20010728-3 - see INTEL-15.TXT
//#define  DBGSTOP  DBS(0xb9, 0xc7, 0x01, 0x00)
// i get
// 000000B9: C7 01 00 00 00 00    MOV         ECX, OFFSET [??_C@_05PNMN@dummy?$AA@]
// should be
//  00409109 C7 01 30 F4 43 00    mov         dword ptr [ecx],43F430h
// { 0XC7, NO_OP2,  NO_PRE, "/0 id", "MOV",       "r/m32,imm32", 0,0,0 }
//ERROR: (r=/0 id t=r/m32,imm32) MRM=01(0,0,1) m=[ECX] to=ECX NOSIB
// The lpmods has been loaded lpmods = pmrm[dwMod].pRM[dwRM];  // = MODRM2 sModRM32;
                              if( dwRM == 5 )   // do we have the disp32
                              {
                                 // this indicates 32-bit displacement FOLLOWING the
                                 // SIB byte to be added to index???
                                 TBD;
                              }
                              else
                              {
                                 // establish DWORD PTR %s to lpmods ([EAX]) register
                                 sprintf(lpto, szDPs, lpmods );
                                 prto = lpto;
                              }
                           }
                           //else  // dwMod = 1, 2 or 3
                           //{
                           //   sprtf( "CHECKME-RM~4: Have NOT added DWORD PTR!"MEOR );
                           //}

                           if(pir)
                           {
                              ADD2ARRAY( dwa , pir );
                              //sprtf( "CHECKME-R: New code using [%s]"MEOR, lpb );
                              sprintf( lpfrom, "OFFSET [%s]", lpb );
                              if(dwo)
                              {
                                 if( i < 0 )
                                 {
                                    dwo = ~(dwo) + 1;
                                    sprintf( &lpfrom[(strlen(lpfrom)-1)], "-%02XH]", dwo );
                                 }
                                 else
                                 {
                                    sprintf( &lpfrom[(strlen(lpfrom)-1)], "+%02XH]", dwo );
                                 }
                              }
                           }
                           else
                           {
                              //sprintf( lpfrom, "%08XH", dwo );
                              GetMinHex( lpfrom, dwo );  // get minimum HEX string
                           }

                           prfrom = lpfrom;
                           dwi += 3;
                        }
                     }
                     else
                     {
                        chkme( "Unhandled item 1 r=%s t=%s!"MEOR, pr, pt );
                     }
                     // above if there is MORE after the /digit ModR/M indicator
                  }
                  else
                  {
                     // ==============================================================
                     // the "register" string just had a /digit and nothing more, like
                     // ==============================================================
                     if( dwst == st_Disp32 )
                     {
//{0x0FF, NO_OP2, NO_PRE, "/6",     "PUSH",         "r/m32",                0,0,0 },
//{0x0C6, NO_OP2, NO_PRE, "/0",     "MOV",          "r/m8,imm8",            0,0,0 },
                        if( dwMod == 0 )
                        {
                           if( dwRM == 5 )
                           {
// 0000057D: FF 15 00 00 00 00  call        dword ptr [_Out_Funcs2]
// FAILED on FF 15! n=CALL r=/2 t=r/m32 m=disp32 off=0x57e
// ModR/M (r=/2 t=r/m32) 15(0,2,5) m=disp32 to=EBP SIB00(0,0,0)s=[EAX] 

                              // FIX20010620-02
// failed on
// { 0XDD, NO_OP2,  NO_PRE, "/0",   "FLD",       "m64fp",    0,0,0 }
//mrm (r=/0 t=m64fp) 05(0,0,5) m=disp32 to=EBP SIB[38](0,7,0)si=[EDI] b=EAX
// getting
// 000000CA: 8B 4D FC             MOV         ECX, DWORD PTR [EBP-04H]
// 000000CD: DD 05 38 00 00 00  FLD         DWORD PTR [?scenery@@3UfgSCENERY@@A]
// 000000D3: DC 05 00 00 00 00  FADD   DWORD PTR [__real@8@3fffd1f947e51f948000]
// should have
//00435CFD   fld         qword ptr [scenery+38h (00a55368)]
                              BUMP1;   // move up to the immediate disp32
                              TEST3;
                              pdw = (PDWORD)&pCode[dwi];
                              dwo = *pdw; // get the next dword
                              pint = (PINT)pdw;
                              i = *pint;
                              *lpb = 0;
                              pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                              if( InStr( pt, "m64" ) )
                              {
                                 // read somewhere operand take size per instruction
                                 //sprtf( "WARNING: New QWORD coding!"MEOR );
                                 strcpy(lpto, szQP );
                              }
                              else
                              {
                                 strcpy(lpto, szDP);
                              }
                              if(pir)
                              {
                                 ADD2ARRAY( dwa, pir );
                                 if(dwo)
                                 {
                                    if( i < 0 )
                                    {
                                       dwo = ~(dwo) + 1;
                                       sprintf( EndBuf(lpb), "-0%XH", dwo );
                                    }
                                    else
                                    {
                                       sprintf( EndBuf(lpb), "+0%XH", dwo );
                                    }
                                 }
                              }
                              else
                              {
                                 // just use the offset
                                 sprintf( lpb, "%08XH", dwo );
                              }
                              sprintf(EndBuf(lpto), szBsB, lpb);  // "[%s]"
                              prto   = lpto;
                              prfrom = szNul;
                              dwi += 3;
                           }
                           else
                           {
                              chkme( "Unhandled item 4 r=%s t=%s!"MEOR, pr, pt );
                           }
                        }
                        else if( dwMod == 2 )
                        {
// example
// { 0XFF, NO_OP2,  NO_PRE, "/2",   "CALL",      "r/m32",    0,0,0 }
//(r=/2 t=r/m32) mrm=92(2,2,2) m=disp32[EDX] to=EDX SIBA4(2,4,4)s=
//0000:0032 FF 92 A4 01 00 00 83 C4  04 3B F4 E8 00 00 00 00 
// or another example
// 00000083: D9 95 AC FE FF FF    FST         DWORD PTR [EBP-0154H]
                           BUMP1;   // move up to 32-bits of displacement
                           TEST3;   // check we have 32 bits left
                           pdw = (PDWORD) &pCode[dwi];
                           *lpb = 0;
                           pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                           dwo = *pdw;
                           // have already eliminated dwRM == 4 (Special [.][.] case, so
                           lps = Right(lpmods, 5); // get the "[REG]" from the string
                           sprintf(lpto, szDPs, lps );
                           pint = (PINT)pdw;
                           i = *pint;
                           if( i < 0 )
                           {
                              dwo = ~(dwo) + 1;
                              sprintf( &lpto[(strlen(lpto)-1)], "-0%XH]", dwo );
                           }
                           else
                           {
                              sprintf( &lpto[(strlen(lpto)-1)], "+0%XH]", dwo );
                           }
                           if(pir)
                           {
                              ADD2ARRAY( dwa, pir );
                              sprintf( &lpto[(strlen(lpto)-1)], "+%s]", lpb );
                           }
                           prto   = lpto;
                           prfrom = szNul;
                           dwi += 3;   // and move to end of 32 bits

                        }
                        else
                        {
                           chkme( "Unhandled item 3 r=%s t=%s!"MEOR, pr, pt );
                        }
                     }
                     else if( strcmp( pt, "r/m32" ) == 0 )
                     {
                        // note: "reg" stg is just a /digit, like
                        // so the R/M of the ModR/M byte give the register
// FAILED 9A FF 24! (r=/4 t=r/m32) mrm=24(0,4,4) m=[.][.] to=ESP SIB85(2,0,5)s=[EAX*4]
// 00000099: FF 24 85 00 00 00 00 jmp         dword ptr [eax*4]
// FAILED ON ModR/M (r=/2 t=r/m32) 52(1,2,2) m=disp8[EDX] to=EDX SIB28(0,5,0)s=[EBP]
// Gave:  000002A3: FF 52 28 3B F4 E8 00 CALL        DWORD PTR [EBP]
// Should be 0050F173   call        dword ptr [edx+28h]
                        //if( (dwi + 3) >= dwLen ) { iGotErr = ie_OutOfData; goto Chk_Err; }
                        if( dwMod == 0 )
                        {
                           // this is too simple - must add more i think
// example
//  { 0XFF, NO_OP2,  NO_PRE, "/4",   "JMP",       "r/m32",    0,0,0 }
//ModR/M (r=/4 t=r/m32) 24(0,4,4) m=[.][.] to=ESP SIB8D(2,1,5)s=[ECX*4]
// 0000007C: FF 24 8D             JMP         DWORD PTR [ECX*4]
// 0000007F: 00 00                ADD         BYTE PTR [EAX], AL
// 00000081: 00 00                ADD         BYTE PTR [EAX], AL
// should have 0050F46C   jmp         dword ptr [ecx*4+50F8F9h]
                           BUMP1;   // move up to the displacement
                           *lpb = 0;
                           pb = &pCode[dwi];
                           prfrom = 0;
                           if( dwRM == 4 )
                           {
// FIX20010729-05 - see INTEL-20.txt - release\dc4w.obj - Do_WM_COMMAND()
//#define  DBGSTOP  DBS(0x2A,0xFF,0x24,0x85)   // 00 00 00 00
// not QUITE right on
// 0000002A: FF 24 85 00 00 00 00 JMP         dword ptr [EAX*4+0H+$L76608]
// should be
//  0040CA4A FF 24 85 C0 D0 40 00 jmp         dword ptr [eax*4+40D0C0h]
//#define  DBGSTOP  DBS( 0X2A, 0XFF, 0X24, 0X85 )
// { 0XFF, NO_OP2,  NO_PRE, "/4",   "JMP",       "r/m32",    0,0,0 }
//ERROR: (r=/4 t=r/m32) MRM=24(0,4,4) m=[.][.] to=dword ptr [EAX*4+0H+$L76608]
//	SIB85(2,0,5)s=[EAX*4] b=[*]
                              TEST3;   // check the 32 bits
                              pdw = (PDWORD)pb;
                              dwo = *pdw; // get the next dword
                              pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                              if( GETIND(dwSIB) == 4 )
                              {
                                 NYD;
                              }
                              else
                              {
                                 sprintf(lpto, szDPs, lpsibs);
                                 // or maybe
                                 //sprintf( &lpto[(strlen(lpto)-1)], "+%XH]", (dwo + dwi + 1));
                                 if(pir)
                                 {
                                    ADD2ARRAY( dwa, pir );
                                    sprintf( &lpto[(strlen(lpto)-1)], "+%s]", lpb );
                                 }
                                 else
                                 {
                                    GetMinHex(lpd, dwo);
                                    sprintf( &lpto[(strlen(lpto)-1)], "+%s]", lpd );
                                 }
                              }
                              BUMP3;   // leave final bump to LOOP
                           }
                           else
                           {
                              sprtf( "WARNING: Check this simple coding!!!"MEOR );
                              sprintf( lpto, szDPs, lpsibs );
                           }
                        }
                        else if( dwMod == 1 )   // disp8[REG]
                        {
                           // dwSIB is displacment value
                           BUMP1;   // move up to the displacement
                           *lpb = 0;
                           pb = &pCode[dwi];
                           lps = Right(lpmods, 5); // get the "[REG]" from the string
                           dwo = (DWORD)*pb; // get byte, an additive item
                           pc  = (char *)pb;
                           i = *pc;          // get sign extended
                           sprintf(lpto, szDPs, lps);
                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf( &lpto[(strlen(lpto)-1)], "-%02XH]", dwo );
                           }
                           else
                           {
                              sprintf( &lpto[(strlen(lpto)-1)], "+%02XH]", dwo );
                           }
                        }
                        else if( dwMod == 3 )
                        {
// example
// ModR/M !D (r=/3 t=r/m32) D8(3,3,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
// { 0XF7, NO_OP2,  NO_PRE, "/3",   "NEG",       "r/m32",    0,0,0 }
// 0000:009A F7 D8 50 8B 4D F8 51 8B  4D F4 E8 00 00 00 00 8B  ..P.M.Q.M

// FIX20010729-06a - see INTEL-20.txt - release\dc4w.obj - Do_WM_COMMAND()
// i get
// { 0XF7, NO_OP2,  NO_PRE, "/6",   "DIV",       "r/m32",    0,0,0 }
//ModR/M !D (r=/6 t=r/m32) F6(3,6,6) m=ESI/SI/DH/MM6/XMM6 to=ESI
// 00000250: F7 F6                DIV         ESI
// should get
//  0040CC70 F7 F6                div         eax,esi
                           if( pi->dwRes3 & 0x01 )
                           {
                              strcpy(lpto, "EAX");
                              strcpy(lpfrom, Left(lpmods, 3) );
                              prfrom = lpfrom;
                              sprtf( "WARNING: CHECKME-M3 New code path!"MEOR );
                           }
                           else
                           {
// FIX20010729-10 - File[D:\GTOOLS\TOOLS\DC4W\RELEASE\DC4W.OBJ] - see INTEL-21.TXT - My_mbsrchr
// 00000043: 66 F7 DA             NEG         EDX
// should be                      NEG         DX
//WARNING: SIZE PRE-OP FLAG NOT USED!  ***CHECKME***
//#define  DBGSTOP  DBS( 0X43, 0XF7, 0XDA, 0X1B )
// { 0XF7, NO_OP2,  NO_PRE, "/3",   "NEG",       "r/m32",    0,0,0 }
//ERROR: (r=/3 t=r/m32) MRM=DA(3,3,2) m=EDX/DX/DL/MM2/XMM2 to=EDX NOSIB
//0000:0046 1B D2 F7 D2 23 C2 5F 5E  C3 90                    ....#._^..
                              if( dwFlag & flg_SIZE )
                              {
                                 iPos = InStr( lpmods, "/" );
                                 if( iPos )
                                 {
                                    strcpy( lpto, Mid(lpmods, (iPos + 1), 2 ) );
                                    dwFlag &= ~( flg_SIZE );

                                 }
                                 else
                                 {
                                    NYD;  // actually a REAL sort of INTERNAL ERROR
                                    // SHOULD NEVER HAPPEN - he says
                                 }
                              }
                              else
                              {
                                 strcpy( lpto, Left( lpmods, 3 ) );
                              }
                           }
                           // in all example so far, looks good, so
                           //sprtf( "WARNING: CHECKME-M3 New code path!"MEOR );
                        }
                        else
                        {
                           TBD;
                           //sprtf( "WARNING: Removed TBD, but CHECKME!"MEOR );
                        }

                        prto = lpto;
                        if( prfrom == 0 )
                           prfrom = szNul;

                     }
                     else if( strcmp( pt, "r/m32,1" ) == 0 )
                     {
// this above strcmp( pt, "r/m32" ) == 0 MISSES the case
// { 0XD1, NO_OP2,  NO_PRE, "/7",   "SAR",       "r/m32,1",  0,0,0 }
//(r=/7 t=r/m32,1) mrm=F8(3,7,0) m=EAX/AX/AL/MM0/XMM0 to=EAX
//0000:0156 D1 F8 8B 4D FC 8B 91 D8  00 00 00 2B D0 8B 45 FC  ...M..
// NOTE: The ,1 on the tail means ONCE, rather than per the CL count
                        // note: "reg" stg is just a /digit, like
                        strcpy(lpfrom,"1");
                        prfrom = lpfrom;

                     }
                     else if( strcmp( pt, "m32fp" ) == 0 )
                     {
// where to put this, and the following
// 0000:0038 D9 45 14 D8 35 00 00 00  00 51 D9 1C 24 D9 45 10  .E..5....Q..$.E.
//  { 0XD9, NO_OP2,  NO_PRE, "/0",   "FLD",       "m32fp",    0,0,0 }
// 0050EC58  D9 45 14          fld   dword ptr [ebp+14h]
// /digit - A digit between 0 and 7 indicates that the ModR/M byte of the 
//instruction uses only the r/m (register or memory) operand. The reg field
//contains the digit that provides an extension to the instruction's opcode.

//  { 0x0D8, NO_OP2, NO_PRE, "/6",     "FDIV",         "m32fp",                0,0,0 },
// 0050EC5B  D8 35 00 00 00 00 fdiv  dword ptr [__real@4@40008000000000000000 (009ff100)]
// 0050EC61  51                push        ecx
                        // was 
                        //if( dwst == st_Disp8 )   // InStr(lpmods, "disp8") == 1 )
                        if( dwMod == 1 )   // InStr(lpmods, "disp8") == 1 )
                        {
                           BUMP1;   // bump to next byte
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get next byte, an additive item
                           pc = (char *)pb;
                           i = *pc;
                           lps = Right(lpmods, (strlen(lpmods) - 5) );
                           strcpy(lpb, szDP);
                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf( &lps[(strlen(lps)-1)], "-%02XH]", dwo );
                           }
                           else
                           {
                              sprintf( &lps[(strlen(lps)-1)], "+%02XH]", dwo );
                           }
                           strcat(lpb, lps);
                           preg = lpb;
                           goto Got_Stg;
                        }  // was else if( ( dwst == st_Disp ) && ( prto ) && ( *prto) )
                        else if( dwRM == 4 )
                        {
                           if( !( prto && *prto ) )
                           {
                              FAILEDCODE;
                           }
                           //FIX20010620-01
// example 0050EC9C   fstp        dword ptr [esp]
// { 0XD9, NO_OP2,  NO_PRE, "/3",   "FSTP",      "m32fp",    0,0,0 }
//ModR/M (r=/3 t=m32fp) 1C(0,3,4) m=[.][.] to=ESP SIB24(0,4,4)s=
//FAILED 43 D9 1C! (r=/3 t=m32fp) mrm=1C(0,3,4) m=[.][.] to=ESP SIB24(0,4,4)s=
//0000:0042 D9 1C 24 D9 45 10 D8 35  00 00 00 00 51 D9 1C 24  ..$.E..5....Q..$
                           // note: it appears ALL "m32fp" have /digit (ie a SIB)
                           if( !bDnSIB ) // if not already DONE SIB byte increment
                           {  // to be removed once SIB incremanet above fully tested
                              BUMP1;   // use up SIB
                              bDnSIB = TRUE; // DONE SIB byte increment
                           }
                           strcpy(lpb, szDP);
                           sprintf(EndBuf(lpb), szBsB, prto ); // "[%s]"
                           preg = lpb;
                           prto = 0;
                           goto Got_Stg;
                        }
                        else if( dwMod == 0 )
                        {
// example
// FIX20010620-04
// { 0XD9, NO_OP2,  NO_PRE, "/0",   "FLD",       "m32fp",    0,0,0 }
//ERROR: (r=/0 t=m32fp) MRM=00(0,0,0) m=[EAX] to=EAX
//0000:000F D9 00 D8 01 8B 55 08 D9  1A 8B 45 0C 8B 4D 10 D9  .....U
// should be
//0043553F   fld         dword ptr [eax]
//00435541   fadd        dword ptr [ecx]
                           if( dwRM == 5 )
                           {
                              NYD;
                           }
                           else
                           {
                              sprintf( lpto, szDPs, lpmods );
                              prto = lpto;
                              prfrom = szNul;
                              goto Got_Stg;
                           }
                        }
                        else
                        {
                           NYD;
                        }
                     }
                     else if( strcmp( pt, "m32int" ) == 0 )
                     {
                        if( ( dwMod == 1 ) && ( dwRM != 4 ) )
                        {
// example
// { 0XDB, NO_OP2,  NO_PRE, "/0",   "FILD",      "m32int",   0,0,0 }
//(r=/0 t=m32int) mrm=45(1,0,5) m=disp8[EBP] to=EBP SIB08(0,1,0)s=[ECX]
//0000:0026 DB 45 08 8B 55 FC D9 5A  0C 8B 45 08 50 68 00 00  .E..U..Z.
// ModR/M (r=/0 t=m32int) 45(1,0,5) m=disp8[EBP] to=EBP SIB[08](0,1,0)si=[ECX]
// SB: 0050E5F6   fild        dword ptr [ebp+8]
                           BUMP1;
                           pb = &pCode[dwi];
                           dwo = (DWORD)*pb; // get the next byte
                           pc = (char *)pb;
                           i = *pc;          // get the signed value
                           strcpy(lpto, szDPb);
                           if( i < 0 )
                           {
                              dwo = (0x100 - dwo);
                              sprintf(EndBuf(lpto), "%s-%02XH]", prto, dwo );
                           }
                           else
                           {  
                              sprintf(EndBuf(lpto), "%s+%02XH]", prto, dwo );
                           }
                           prto = lpto;
                           prfrom = szNul;
                           goto Got_Stg;
                        }
                        else
                        {
                           NYD;
                        }
                     }
                     else if( strcmp( pt, "m64fp" ) == 0 )
                     {
                        if( dwMod == 0 )  // Effective Address - first group
                        {
                           if( dwRM == 4 )   // special SIB case [.][.]
                           {
// example
// { 0XDD, NO_OP2,  NO_PRE, "/3",   "FSTP",      "m64fp",    0,0,0 }
//ERROR: (r=/3 t=m64fp) MRM=1C(0,3,4) m=[.][.] to=ESP
//	SIB24(0,4,4)s= b=ESP
//0000:003A DD 1C 24 68 00 00 00 00  8B 4D FC 83 C1 10 51 E8  ..$h.....M....Q.
// should produce 0043E82A   fstp        qword ptr [esp]
                              // lpsbase
                              // note: already inc'ed for SIB
                              if( GETBASE(dwSIB) == 5 )  // special [*] case
                              {
                                 NYD;
                              }
                              else
                              {
                                 sprintf( lpto, szQPbsb, sSIBBase[GETBASE(dwSIB)] );
                                 //sprtf( "CHECKME: More new code WARNING: dwMod=0 dwRM=4"MEOR );
                                 prto = lpto;
                                 prfrom = szNul;
                                 goto Got_Stg;
                              }
                           }
                           else if( dwRM != 5 ) // ie not disp32
                           {
// example
// { 0XDD, NO_OP2,  NO_PRE, "/0",   "FLD",       "m64fp",    0,0,0 }
//ERROR: (r=/0 t=m64fp) MRM=00(0,0,0) m=[EAX] to=EAX
//0000:0081 DD 00 D9 95 AC FE FF FF  51 D9 1C 24 6A 01 8D 4D  ......
// should produce fld         qword ptr [eax]
// NOTE:Already loaded lpmods = pmrm[dwMod].pRM[dwRM];  // = MODRM2 sModRM32;
// which is 0 Mod, with Effective Address set of
//   { "[EAX]", "[ECX]", "[EDX]", "[EBX]", "[.][.]", "disp32", "[ESI]", "[EDI]" },
                              sprintf( lpto, szQPs, lpmods );
                              prto = lpto;
                              prfrom = szNul;
                           }
                           else
                           {
                              TBD;
                           }
                        }
                        else if( dwMod = 1 ) // Effective Address - 2nd group
                        {
                           if(dwRM == 4)  // [.][.] case
                           {
                              TBD;
                           }
                           else
                           {
                              // FIX20010620-03
// example
//ERROR: (r=/3 t=m64fp) MRM=59(1,3,1) m=disp8[ECX] to=ECX
//   SIB48(1,1,0)s=[ECX*2] b=EAX
//0000:00D9 DC 59 48 DF E0 F6 C4 01  74 24 8D 8D 74 FF FF FF
// should be //00435D09   fcomp       qword ptr [ecx+48h]
// now got  000000D9: DC 59 48             FCOMP       QWORD PTR [ECX+48H]
                              BUMP1;   // bump to next byte
                              pb = &pCode[dwi];
                              dwo = (DWORD)*pb; // get next byte, an additive item
                              pc = (char *)pb;
                              i = *pc;
                              lps = Right(lpmods, (strlen(lpmods) - 5) );
                              sprintf(lpto, szQPs, lps);
                              if(dwo)
                              {
                                 if( i < 0 )
                                 {
                                    dwo = (0x100 - dwo);
                                    sprintf( &lpto[(strlen(lpto)-1)], "-%02XH]", dwo );
                                 }
                                 else
                                 {
                                    sprintf( &lpto[(strlen(lpto)-1)], "+%02XH]", dwo );
                                 }
                              }
                              prto = lpto;
                              prfrom = szNul;
                           }
                        }
                        else  // dwMod 2 and 3
                        {

                           TBD;
                        }
                     }
                     else if( strcmp( pt, "m64int" ) == 0 )
                     {
                        if(( dwRM == 4 ) ||
                           ( dwMod == 3 ) )
                        {
                           NYD;
                        }
                        else if( dwMod == 1 )
                        {
// FIX20010731-01 - File[D:\GTOOLS\CONAPPS\TEST\DEBUG\TEST.OBJ] - INTEL-24.txt
// Unhandled item 2 r=/5 t=m64int!  ***CHECKME*** ***WARNING***
//#define  DBGSTOP  DBS( 0X4C, 0XDF, 0X6D, 0XF8 )
// { 0XDF, NO_OP2,  NO_PRE, "/5",   "FILD",      "m64int",   0,0,0 }
//ERROR: (r=/5 t=m64int) MRM=6D(1,5,5) m=disp8[EBP] to=EBP NOSIB
//0000:06D1 DF 6D F8 DD 5D EC E8 00  00 00 00 89 45 E4 DD 45  .m..].......E..E
// should get
//  000006D1: DF 6D F8           fild        qword ptr [ebp-8]
//  000006D4: DD 5D EC           fstp        qword ptr [ebp-14h]
                              BUMP1;   // bump to next byte
                              pb = &pCode[dwi];
                              dwo = (DWORD)*pb; // get next byte, an additive item
                              pc = (char *)pb;
                              i = *pc;
                              lps = Right(lpmods, (strlen(lpmods) - 5) );
                              sprintf(lpto, szQPs, lps);
                              if(dwo)
                              {
                                 if( i < 0 )
                                 {
                                    dwo = (0x100 - dwo);
                                    sprintf( &lpto[(strlen(lpto)-1)], "-%02XH]", dwo );
                                 }
                                 else
                                 {
                                    sprintf( &lpto[(strlen(lpto)-1)], "+%02XH]", dwo );
                                 }
                              }
                              prto = lpto;
                              prfrom = szNul;
                        }
                        else
                        {
                           NYD;
                        }
                     }
                     else
                     {
// FIX20010729-06b - see INTEL-20.txt - release\dc4w.obj - Do_WM_COMMAND()
// Unhandled item 2 r=/4 t=r/m16! ***CHECKME*** ***WARNING***
//#define  DBGSTOP  DBS( 0X258, 0XF7, 0XE1, 0XC1 )
// FAILED 0X259 0XF7 0XE1 0XC1! (Line=6048)
// { 0XF7, NO_OP2,  NO_PRE, "/4",   "MUL",       "r/m16",    0,0,0 }
//ERROR: (r=/4 t=r/m16) MRM=E1(3,4,1) m=ECX/CX/CL/MM1/XMM1 to=ECX NOSIB
//0000:0258 F7 E1 C1 EA 06 52 6A 2B  E8 00 00 00 00 50 53 E8  .....Rj+.....PS.
// i get no code - should be
//  0040CC78 F7 E1                mul         eax,ecx
//  0040CC7A C1 EA 06             shr         edx,6
                        chkme( "Unhandled item 2 r=%s t=%s!"MEOR,
                           pr,
                           pt );
                        NYD;
                     }
                  }
                  // inside the if register had /digit, indicating
                  // a ModR/M where the R/M give the register.
                  // thus lpmods = pmrm[dwMod].pRM[dwRM];  // = MODRM2 sModRM32;
                  // that is the string per Table 2-2. 32-bit Addressing Forms
                  // with the ModR/M byte
                  // NEXT BYTE WAS ModR/M BYTE
                  // *************************
               }
               else if( (dwrl == 2) && ( pr[0] == 'i'     ) &&
                  ( ( pr[1] == 'b' ) || ( pr[1] == 'd' )  ) &&
                  ( ( iPos = InStr(pt,"imm") ) > 0 ) )
               {
                  // NOT /r, nor /digit, so maybe
//{0x004, NO_OP2, NO_PRE, "ib",     "ADD",          "AL,imm8",              0,0,0 },
//{0x005, NO_OP2, NO_PRE, "id",     "ADD",          "EAX,imm32",            0,0,0 },
//{0x0CD, NO_OP2, NO_PRE, "ib",     "INT",          "imm8",                 0,0,0 },
//{0x0E6, NO_OP2, NO_PRE, "ib",     "OUT",          "imm8, AL",             0,0,0 },
//{0x0E7, NO_OP2, NO_PRE, "ib",     "OUT",          "imm8, EAX",            0,0,0 },
                  if( iPos == 1 )
                  {
                     if( strlen( pt ) == 4 )
                     {
                        dwType = it_AddB2To;
                        prfrom = szNul;
                     }
                     else
                     {
                        if( InStr( pt, "EAX" ) )
                           prfrom = Right( pt, 3 );
                        else
                           prfrom = Right( pt, 2 );
                        prto = 0;
                        if( InStr( pt, "imm8" ) )
                           dwType = it_AddB2To;
                        else
                           chkme( "Thought this did NOT exist!" );
                     }
                  }
                  else
                  {
                        if( InStr( pt, "EAX" ) )
                        {
                           if( dwFlag & flg_SIZE )
                           {
// FIX20010701-02
// 00000018: 66 05 01 00 66 89    ADD         EAX, 89660001H
//WARNING: SIZE PRE-OP FLAG NOT USED!
//FAILED 0X1D 0X5 0X1 000!
// { 0X05, NO_OP2,  NO_PRE, "id",   "ADD",       "EAX,imm32", 0,0,0 }
//ERROR: (r=id t=EAX,imm32) MRM=45(1,0,5) m=<nul> to=EAX
//0000:001E 45 F8 8B 4D F8 81 E1 FF  FF 00 00 8B 55 10 81 E2  E..M....
                              //prto = prs->sRegt[am_r16].pName; // get the relevant register
                              strcpy( lpto, "AX" );
                              prto = lpto;
                           }
                           else
                              prto = Left( pt, 3 );
                        }
                        else
                        {
                           prto = Left( pt, 2 );
                        }
                        if( InStr( pt, "imm32" ) )
                           dwType = it_AddD2From;
                        else
                           dwType = it_AddB2From;
                  }
               }  // NOTE: This case is NOW handled EARLIER
               else if( (dwrl == 0) && (dwtl == 0) )  //(strlen(pi->pTail) == 0 ) )
               {
                  // ***** TO BE DELETED *****
                  // we appear to ONLY have a nmonic (name), so
                  prto = szNul;
                  prfrom = szNul;
                  chkme( "CHECKME: WARNING: This CODE is to be DELETED"MEOR
                     "So simply should NOT be HERE!" );
                  // ***** TO BE DELETED *****
               }
               else if( ( dwrl == 0 ) && ( ( iPos = InStr(pt, "moffs") ) > 0 ) )
               {
//{0x0A0, NO_OP2, NO_PRE, szNul,    "MOV",          "AL,moffs8*",           0,0,0 },
////{0x0A1, NO_OP2, NO_PRE, szNul,    "MOV",          "AX,moffs16*",          0,0,0 },
//{0x0A1, NO_OP2, NO_PRE, szNul,    "MOV",          "EAX,moffs32*",         0,0,0 },
//{0x0A2, NO_OP2, NO_PRE, szNul,    "MOV",          "moffs8*,AL",           0,0,0 },
////{0x0A3, NO_OP2, NO_PRE, szNul,    "MOV",          "moffs16*,AX",          0,0,0 },
//{0x0A3, NO_OP2, NO_PRE, szNul,    "MOV",          "moffs32*,EAX",         0,0,0 },
                  if( iPos == 1 )
                  {
                     BUMP1;   // bump to first byte of 32-bit address
                     TEST3;
                     pdw = (PDWORD)&pCode[dwi];
                     dwo = *pdw; // get the dword
                     pint = (PINT)pdw;
                     i = *pint;
                     *lpb = 0;
                     pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                     if( InStr( pt, "moffs8" ) )
                     {
                        prfrom = Right( pt, 2 );
                        //BUMP1;
                        //pb = &pCode[dwi];
                        //dwo = (DWORD)*pb; // get the byte
                        //sprintf( lpto, "[%08X]", dwo );
                        sprtf( "CHECKME-MO8: Now always using 4 byte offset!"MEOR );
                     }
                     else
                     {
                        prfrom = Right( pt, 3 );
                     }
                     //   BUMP1;   // bump to first byte of 32-bit address
                     //   TEST3;
                     //   pdw = (PDWORD)&pCode[dwi];
                     //   dwo = *pdw; // get the dword
                     //   pint = (PINT)pdw;
                     //   i = *pint;
                     //   *lpb = 0;
                     //   pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                        if(pir)
                        {
                           ADD2ARRAY( dwa, pir );
                           // was simply sprintf( lpto, szBsB, lpb );
                           // try
                           //if(dwo)
                           //   sprintf( lpto, "[%s+%d]", lpb, dwo );
                           //else
                              sprintf( lpto, szBsB, lpb);   // "[%s]"
                        }
                        else
                        {
                           sprintf( lpto, "[%08X]", dwo );
                        }

                        if( i < 0 )
                           chkme( "CHECKME-N: WARNING: NEGATIVE offset not HANDLED!"MEOR );

                     //   dwi += 3;   // note the last is per the FOR LOOP
                     //}
                     BUMP3;
                     prto = lpto;
                  }
                  else
                  {
// FIX20010729-04 - see INTEL-19.txt - release\dc4w.obj
//#define  DBGSTOP  DBS( 0XB6, 0XA0, 0X00, 0X00 )
// i get
// 000000B6: A0 00                MOV         AL, [00000000]
// 000000B8: 00 00                ADD         byte ptr [EAX], AL
// should be
//0040A736 A0 2C EE 43 00       mov         al,[0043EE2C]
//0040A73B 84 C0                test        al,al
// { 0XA0, NO_OP2,  NO_PRE, szNul,  "MOV",       "AL,moffs8*", 0,0,0 }
//ERROR: (r= t=AL,moffs8*) MRM=D8(0,0,0) m=<nul> to=AL NOSIB
//0000:00B8 00 00 00 84 C0 0F 84 93  00 00 00 8B 2D 00 00 00  ............-...
                     BUMP1;
                     TEST3;
                     pdw = (PDWORD)&pCode[dwi];
                     dwo = *pdw; // get the dword
                     pint = (PINT)pdw;
                     i = *pint;
                     *lpb = 0;
                     pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                     if( InStr( pt, "moffs8" ) )
                     {
                        prto = Left( pt, 2 );
                        //BUMP1;
                        //pb = &pCode[dwi];
                        //dwo = (DWORD)*pb; // get the byte
                        //sprintf( lpfrom, "[%08X]", dwo );
                        sprtf( "CHECKME-MO8: Now always using 4 byte offset!"MEOR );
                     }
                     else // must be moffs32*
                     {
                        prto = Left( pt, 3 );
                     }
                        //BUMP1;
                        //TEST3;
                        //*lpb = 0;
                        //pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 ); 
                     if(pir)
                     {
                        ADD2ARRAY( dwa, pir );
                        if( dwFlag & flg_FSOVER )
                        {
                           sprintf( lpfrom, "FS:[%s]", lpb );
                           dwFlag &= ~(flg_FSOVER);
                        }
                        else
                           sprintf( lpfrom, szBsB, lpb); // "[%s]"
                     }
                     else
                     {
                        //pdw = (PDWORD)&pCode[dwi];
                        //dwo = *pdw; // get the dword
                           if( dwFlag & flg_FSOVER )
                           {
                              sprintf( lpfrom, "FS:[%09X]", dwo );
                              dwFlag &= ~(flg_FSOVER);
                           }
                           else
                              sprintf( lpfrom, "[%09X]", dwo );
                     
                        //dwi += 3;   // note the last is per the FOR LOOP
                     }

                     if( i < 0 )
                        chkme( "CHECKME-N2: NEGATIVE offset detected!"MEOR );

                     BUMP3;   // note the last is per the FOR LOOP
                     prfrom = lpfrom;
                  }
               }
               else
               {
//{0x06A, NO_OP2, NO_PRE, szNul,    "PUSH",         "imm8",                 0,0,0 },
//{0x068, NO_OP2, NO_PRE, szNul,    "PUSH",         "imm32",                0,0,0 },
                  *lpb = 0;   // start with NO call "name"
                  iPos = InStr( pi->pNm, "CALL" );
                  if( iPos )
                  {
                     TEST4;
                     pir = GetRelName( lpb, pRel, dwrel, pStgs, (dwi + 1), 0 ); 
                     if(pir)
                     {
                        ADD2ARRAY( dwa, pir );
                        dwi += 4;
                        prto = lpb;
                        prfrom = szNul;
                     }
                  }
                  else //  if( iPos == 0 )
                  {
//  { 0x068, NO_OP2, NO_PRE, szNul,    "PUSH",         "imm32",                0,0,0 },
                     if( InStr( pt, "imm32" ) > 0 )
                     {
                        // iPos = InStr( pi->pNm, "PUSH" );
                        iPos = 1;
                        TEST4;
                        pir = GetRelName( lpb, pRel, dwrel, pStgs, (dwi + 1), 0 ); 
                        if(pir)
                        {
                           ADD2ARRAY( dwa, pir );
                           dwi += 4;
                           prto = lpb;
                           prfrom = szNul;
                        }
                     }
                     else if( strcmp( pt, "rel32" ) == 0 )
                     {
                        iPos = (DWORD)-1;
                     }
                     else if( strcmp( pt, "rel8" ) == 0 )
                     {
                        iPos = (DWORD)-2;
                     }
                     else if( strcmp( pt, "imm8" ) == 0 )
                     {
                        iPos = (DWORD)-3;
                     }
                     else if( strcmp( pt, "imm16" ) == 0 )
                     {
                        iPos = (DWORD)-4;
                     }
                     else if( strcmp( pt, "AX" ) == 0 )
                     {
//  { 0XDF, 0XE0, NO_PRE, szNul,  "FNSTSW",    "AX",       0,0,0 }
                        prto = pt;
                        prfrom = szNul;
                        goto Got_Stg;
                     }
                  }
                  
                  if( iPos == 1 )
                  {
                     // This is an imm32 memory offset
                     if( *lpb == 0 )
                     {
                        // CRAZY CODE WAS
                        //if( (dwi + 4) >= dwLen )
                        //{
                        //   iGotErr = ie_OutOfData;
                        //   goto Chk_Err;
                        //}
                        //dwn = 4;
                        //while(dwn--)
                        //{
                        //   dwi++;
                        //   pb = &pCode[dwi];
                        //   dwo = (DWORD)*pb; // get the byte
                        //   if(*lpb)
                        //      strcat(lpb," ");
                        //   sprintf(EndBuf(lpb), "%02X", dwo );
                        //}
                        // NEW CODE: FIX20010618
                        BUMP1;
                        TEST3;
                        pdw = (PDWORD)&pCode[dwi];
                        dwo = *pdw;
                        sprintf(lpb, "%08XH", dwo);
                        if( !ISNUM(*lpb) )
                           sprintf(lpb, "%09XH", dwo);
                        dwi += 3;
                        prto = lpb;
                        prfrom = szNul;
                     }
                  }
                  else if( iPos == (DWORD)-1 )
                  {
                     //  else if( strcmp( pt, "rel32" ) == 0 )
                     // this is a RELATIVE JUMP 32-bit OFFSET
// 00000054: 0F 84 6F 04 00 00    JZ          000004C9H
// { 0X0F, 0X84, NO_PRE, "cd",   "JZ",        "rel32",    0,0,0 }
//(r=cd t=rel32) mrm=20(2,7,1) m=<nul> to=000004C9H
//0000:005A 8B 55 FC 8B 82 7C 01 00  00 89 85 18 FF FF FF 83  .U.
                     BUMP1;   // move up to the 32 bit displacment
                     TEST3;   // check we have the data
                     pdw = (PDWORD)&pCode[dwi];
                     *lpb = 0;
                     pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
                     dwo = *pdw; // get the DWORD following
                     pint = (PINT)pdw;
                     i = *pint;
                     dwo = dwo + dwi + 4;
                     i += (dwi + 4);
                     if(pir)
                     {
                        sprtf( "WARNING: CHECK this rel32 jump instruction!"MEOR );
                        sprintf(lpto, "[%s + ", lpb);
                        GetMinHex(lpd, dwo);
                        strcat(lpto,lpd);
                        strcat(lpto,"]");
                     }
                     else
                     {
                        // FIX20010618 - Seem to have this CORRECT now
                        //sprtf( "WARNING: CHeck this rel32 jump!"MEOR );
                        //sprintf(lpto, "%08XH", (dwo + dwi + 1));
                        sprintf(lpto, "[%s + ", m_cHeader);
                        GetMinHex(lpd, dwo);
                        strcat(lpto,lpd);
                        strcat(lpto,"]");
                     }

                     if( i < 0 )
                        sprtf( "CHECKME-N32: WARNING: A negative 32bit offset NOT HANDLED!");

                     prto   = lpto;
                     prfrom = szNul;
                     dwi += 3;      // do not forget normal LOOP increment
                  }
                  else if( iPos == (DWORD)-2 )
                  {
                     // else if( strcmp( pt, "rel8" ) == 0 )
                     // this is a RELATIVE JUMP 8-bit OFFSET
                     // NOTE: This is PLUS or MINUS 127!!!
                     BUMP1;
                     pb = &pCode[dwi];
                     dwo = (DWORD)*pb; // get the byte
                     // so convert to SIGNED load
                     pc = (char *)pb;
                     i = *pc;          // get the signed value
                     // ***TBD*** This is a RELATIVE JUMP
                     // The only way to create ASM code would be to INSERT a
                     // LABEL at the jump location!!! ***TBD***
                     // ****************************************************
                     // FIX20010617 - But for NOW always use RELATIVE the header
                     //sprintf(lpto, "%08XH", (dwo + dwi + 1));
                     //if( i < 0 )
                     //   sprintf(lpto, "%08XH", ((dwo + dwi + 1) - 0x100) );
                     if( i < 0 )
                     {
                        //sprintf(lpto, "[%s - ", m_cHeader);
                        // Always a PLUS relative to the HEADER
                        dwo = ((dwo + dwi + 1) - 0x100);
                     }
                     else
                     {
                        //sprintf(lpto, "[%s + ", m_cHeader);
                        dwo = dwo + dwi + 1;
                     }
                     sprintf(lpto, "[%s + ", m_cHeader);
                     GetMinHex(lpd, dwo);
                     strcat(lpto,lpd);
                     strcat(lpto,"]");

                     prto   = lpto;
                     prfrom = szNul;
                  }
                  else if( iPos == (DWORD)-3 )
                  {
                     // else if( strcmp( pt, "imm8" ) == 0 )
                     // this is an imm8 of from memory
                     BUMP1;
                     pb = &pCode[dwi];
                     dwo = (DWORD)*pb; // get the byte
                     GetMinHex(lpto, dwo);
                     prto   = lpto;
                     prfrom = szNul;
                  }
                  else if( iPos == (DWORD)-4 )
                  {
                     // this is an imm16 of from memory
                     BUMP1;
                     pword = (PWORD) &pCode[dwi];
                     dwo = (DWORD)*pword; // extract the value
                     sprintf(lpto, "%XH", dwo);
                     prto   = lpto;
                     prfrom = szNul;
                     BUMP1;   // and do the second bump also
                  }
                  else if( strcmp( pt, "r/m8" ) == 0 )
                  {
// example
// { 0X0F, 0X95, NO_PRE, szNul,  "SETNE",     "r/m8",     0,0,0 }
//ERROR: (r= t=r/m8) MRM=7D(1,7,5) m=<nul> to=(null)
//0000:006C 0F 95 C0 48 24 F6 89 45  D8 8B 4D D8 89 4D DC 8B  ...H$..E..M..M..
                     //NYD;  // what should this be???
                     sprintf( lpto, "AL");
                     prto   = lpto;
                     prfrom = szNul;
                     strcpy( lpcomm, ";*??* CHECKME-?: r/m8 coding?" );
                  }
                  else if( strlen(pr) == 0 )
                  {
// FIX20010725-04 - File: DEBUG\DC4W.OBJ - see INTEL-09.TXT
// 00000772: 00 06                ADD         BYTE PTR [ESI], AL
//#define  DBGSTOP  DBS( 0X774, 0X6, 0X6, 0X6 )
// { 0X06, NO_OP2,  NO_PRE, szNul,  "PUSH",      "ES",       0,0,0 }
//ERROR: (r= t=ES) MRM=06(0,0,6) m=<nul> to=(null)
//0000:0774 06 06 06 06 06 06 01 06  06 06 06 06 02 03 06 06  ......
// should get -
//00411211 00 00                add         byte ptr [eax],al
//00411213 06                   push        es
//00411214 06                   push        es

// still problem with
// FIX20010728-10 - release\dc4w.obj - ParseArgs() - FIXED ABOVE
// { 0XAE, NO_OP2,  0XF2, szNul,  "REPNE",     "SCAS m8",  0,0,0 }

// next coding for here
// FIX20010729-03 - see INTEL-18.txt - release\dc4w.obj - Paint_Warning()
// 00000072: 66 A5                MOVS        m32,m32
//WARNING: SIZE PRE-OP FLAG NOT USED!  ***CHECKME***
//#define  DBGSTOP  DBS( 0X72, 0XA5, 0X55, 0XA4 )
// { 0XA5, NO_OP2,  NO_PRE, szNul,  "MOVS",      "m32,m32",  0,0,0 }
//ERROR: (r= t=m32,m32) MRM=FD(0,0,0) m=<nul> to=(null) NOSIB
                     preg = 0;
                     if( *pt )
                     {
                        if( strcmp(pt, "m32,m32") == 0 )
                        {  // this catches
// { 0x0A7, NO_OP2, NO_PRE, szNul,    "CMPS",         "m32,m32",              0,0,0 },
// { 0x0A5, NO_OP2, NO_PRE, szNul,    "MOVS",         "m32,m32",              0,0,0 },
                           if( dwFlag & flg_SIZE )
                           {
                              strcpy( lpfrom, "word ptr [EDI], word ptr [ESI]");
                              dwFlag &= ~( flg_SIZE );
                           }
                           else
                           {
                              strcpy( lpfrom, "dword ptr [EDI], dword ptr [ESI]");
                           }
                           preg = lpfrom;
                        }
                        else if( strcmp( pt, "m8,m8" ) == 0 )
                        {
// this catches
// { 0x0A6, NO_OP2, NO_PRE, szNul,    "CMPS",         "m8,m8",                0,0,0 },
// { 0x0A4, NO_OP2, NO_PRE, szNul,    "MOVS",         "m8,m8",                0,0,0 },
                           strcpy( lpfrom, "byte ptr [EDI], byte ptr [ESI]");
                           preg = lpfrom;
                        }
                        else
                        {
                           iPos = strlen(pt);
                           for( i = 0; i < iPos; i++ )
                           {
                              if( ISLOWER( pt[i] ) )
                              {
                                 NYD;
                              }
                           }
                        }
                     }
                     
                     if( !preg )
                        preg = pt;  // just use the TAIL
                  }
                  else
                  {
                     NYD;
                  }
               }
               // we have FOUND the "opcode" in the table
            }
         }
      }
      else
      {
         iGotErr = ie_NotIntel;
         s_iLine = __LINE__;
         goto Chk_Err;
      }

      switch( dwType )
      {
      case it_AddB2To:
         {
            BUMP1;
            pb = &pCode[dwi];
            dwo = (DWORD)*pb; // get the byte
            sprintf( lpto, "%02XH", dwo );
            prto = lpto;
         }
         break;
      case it_AddD2From:
         {
            BUMP1;
// FIX20010701-02
// 00000018: 66 05 01 00 66 89    ADD         EAX, 89660001H
//WARNING: SIZE PRE-OP FLAG NOT USED!
//FAILED 0X1D 0X5 0X1 000!
// { 0X05, NO_OP2,  NO_PRE, "id",   "ADD",       "EAX,imm32", 0,0,0 }
//ERROR: (r=id t=EAX,imm32) MRM=45(1,0,5) m=<nul> to=EAX
//0000:001E 45 F8 8B 4D F8 81 E1 FF  FF 00 00 8B 55 10 81 E2  E..M....
            if( dwFlag & flg_SIZE )
            {
               TEST1;
               pword = (PWORD) &pCode[dwi];
               dwo = *pword; // get the word
               dwi += 1;   // note - for loop will do final increment
               sprintf( lpfrom, "%04XH", dwo );
               dwFlag &= ~(flg_SIZE);
               sprtf( "WARNING: New code path for 0x66 SIZE override!"MEOR );
            }
            else
            {
               TEST3;
               *lpb = 0;
               pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
               if(pir)
               {
                  ADD2ARRAY( dwa, pir );
                  sprintf( lpfrom, "%s", lpb );
               }
               else
               {
                  pdw = (PDWORD) &pCode[dwi];
                  dwo = *pdw; // get the dword
                  sprintf( lpfrom, "%08XH", dwo );
               }
               dwi += 3;   // note - for loop will do final increment
            }
            prfrom = lpfrom;
         }
         break;

      case it_AddB2From:
         {
            BUMP1;
            pb = &pCode[dwi];
            dwo = (DWORD)*pb; // get the byte
            sprintf( lpfrom, "%02XH", dwo );
            prfrom = lpfrom;
         }
         break;

//      case it_AddDM2From:
//         {
//            BUMP1;
//            TEST3;
//            *lpb = 0;
//            pir = GetRelName( lpb, pRel, dwrel, pStgs, dwi, 0 );
//            if(pir)
//            {
//               ADD2ARRAY( dwa, pir );
//               sprintf( lpfrom, "[%s]", lpb );
//            }
//            else
//            {
//               pdw = (PDWORD) &pCode[dwi];
//               dwo = *pdw; // get the dword
//               sprintf( lpfrom, "DWORD PTR [%08XH]", dwo );
//            }
//            prfrom = lpfrom;
//            dwi += 3;   // note - for loop will do final increment
//         }
//         break;
      }

Got_Stg:

      if( !iGotErr &&
         ( preg || (prto && prfrom)) )
      {
         strcpy(lpn, pi->pNm);   // get nmonic name
         strcat(lpn, " ");       // must add ONE space
         SetMinLen(lpn,MINOPR);  // then fill out
         if( preg )
         {
            strcat(lpn, preg);   // like MOV  ECX, 00000010H
            if( prfrom && *prfrom ) // do we have a PROM and data in it
            {
                  strcat(lpn, ", ");
                  strcat(lpn, prfrom);
            }
         }
         else  // if( prto && prfrom )
         {
            // this is a cmp, mov, or ...
            if( *prto )
            {
               strcat(lpn, prto);
               if( *prfrom )
               {
                  strcat(lpn, ", ");
                  strcat(lpn, prfrom);
               }
            }
         }

         if( *lpcomm )
         {
            strcat(lpn, " ");    // ensure at least one space
            SetMinLen(lpn, MINOF2C);
            strcat(lpn, lpcomm);
         }

         sprintf(lpd, " %08X: ", dwb);
         pb = &pCode[dwb];
         // wrap to next line if too many bytes
         // lps = lpd;
         i = 0;
         for( dwn = dwb; dwn <= dwi; dwn++ )
         {
            if( i == 7 )   // time for wrap
            {
               i = 0;
               AddASMString( lpd, (strlen(lpd)+1) );  // no ouput to ASM file
               strcpy(lpd, "           ");
            }
            sprintf(EndBuf(lpd), "%02X ", *pb );
            pb++;
            i++;
         }

         SetMinLen(lpd,MINOFF2);
         dwo = strlen(lpd);
         strcat(lpd, lpn);
         //AddStringandAdjust(lpd);
         AddASMString( lpd, dwo );
         dwb = dwi + 1; // set next begin
         if(bHadStop)
            dbgstop(lpd);

      }
      else
      {
         // was NOT an opcode with register offset
         if( iGotErr == ie_None )
            iGotErr = ie_Failed;
      }

Chk_Err:

      if( dwFlag & (flg_FSOVER|flg_GSOVER|flg_SIZE|flg_ADDR) )
      {
         if( dwFlag & flg_FSOVER )
         {
            sprintf(lpd, "WARNING: FS OVERRIDE FLAG NOT USED!"MEOR );
            chkme(lpd);
            dwFlag &= ~(flg_FSOVER);
         }
         if( dwFlag & flg_GSOVER )
         {
            sprintf(lpd, "WARNING: GS OVERRIDE FLAG NOT USED!"MEOR );
            chkme(lpd);
            dwFlag &= ~(flg_GSOVER);
         }
         if( dwFlag & flg_SIZE )
         {
            sprintf(lpd, "WARNING: SIZE PRE-OP FLAG NOT USED!"MEOR );
            chkme(lpd);
            dwFlag &= ~(flg_SIZE);
         }
         if( dwFlag & flg_ADDR )
         {
            sprintf(lpd, "WARNING: ADDR PRE-OP FLAG NOT USED!"MEOR );
            chkme(lpd);
            dwFlag &= ~(flg_ADDR);
         }

#ifndef  NDEBUG
         if( iGotErr == ie_None )
            iGotErr = ie_NotYetDone;
#endif   // !NDEBUG
      }

      if( iGotErr || bDoExit )
      {
         if( iGotErr == ie_OutOfData )    // ? sie_OutOfData
         {
            // downgrade this to ONLY a WARNING, and NOT
            // really an ERROR - But still BREAK
            strcpy(lpd, "; *??* WARNING: Out of DATA within instruction!" );
            dwo = strlen(lpd);
            AddASMString( lpd, dwo );
            iGotErr = 0;   // and KILL this ERROR
#ifdef  DUMP4
            break;
#endif // #ifdef  DUMP4
         }
#ifndef  DUMP4
         break;
#endif // #ifndef  DUMP4
      }

   }  // while we have DECODE data available, and no ERROR

   if( iGotErr || bDoExit )
   {
      if(pi)
      {
         //sprintf(lpd, "FAILED on %02X %02X! n=%s r=%s t=%s m=%s off=%#x",
         //   dwop1, dwop2,
         //   pi->pNm,
         //   pr,      // get pointer to the "register" STRING
         //   pt,      // and the "tail" string
         //   ( lpmods ? lpmods : "<nul>" ),
         //   dwi );
         sprtf(szDbgStop);
         sprintf(lpd, "FAILED %#X %#03X %#03X %#03X! (Line=%d)"MEOR,
            dwi,
            dwop1,
            dwop2,
            dwnxt,
            s_iLine );
         sprtf(lpd);
         *lpd = 0;
         AddpiStg(lpd,pi);
         sprtf(lpd);
//         if(dwst == st_None)
//         {
//            sprintf(lpd, "ERROR: (r=%s t=%s) MRM="      // 1,2
//               "%02X(%d,%d,%d) "                // 3,4,5,6
//               "m=%s to=%s",               // 7,8
//               pr, pt,                   // 1, 2
//               dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
//               ( lpmods ? lpmods : "<nul>" ),
//               prto );                // 7,8
//                        sprintf( lpd, "FAILED with (r=%s t=%s) "      // 1,2
//                           "%02X(%d,%d,%d) "                // 3,4,5,6
//                           "m=%s to=%s"MEOR,               // 7,8
//                           pr, pt,                   // 1, 2
//                           dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
//                           lpmods, prto );                // 7,8
//         }
//         else
         {
            if( ( dwRM == 4 ) && ( dwMod != 3 ) )
            {
               lpform = "ERROR: (r=%s t=%s) MRM="      // 1,2
               "%02X(%d,%d,%d) "                // 3,4,5,6
               "m=%s to=%s"MEOR                // 7,8
               "\tSIB%02X"                       // 9
               "(%d,%d,%d)"                    // 10,11,12
               "s=%s b=%s";                      // 13, 14
            }
            else
            {
               lpform = "ERROR: (r=%s t=%s) MRM="      // 1,2
               "%02X(%d,%d,%d) "                // 3,4,5,6
               "m=%s to=%s NOSIB";                // 7,8
            }

            sprintf(lpd,
               lpform,
               pr, pt,                   // 1, 2
               dwMRM, dwMod, dwOp, dwRM,   // 3,4,5,6
               (lpmods ? lpmods : "<nul>"),  // 7
               prto,                         // 8
               dwSIB,                           // 9
               GETSS(dwSIB), GETIND(dwSIB), GETBASE(dwSIB), // 10,11,12
               lpsibs,                        // 13
               lpsbase );  // 14 = sSIBBase[GETBASE(dwSIB)] );
         }
      }
      else
      {
         sprintf(lpd, "FAILED to find value %#X! in table! Err=%d", dwop1, iGotErr );
      }

      AddStringandAdjust(lpd);

      dwn = 4;    // output 4 lines and abort this set

#ifndef  DUMP4
      goto Do_Dump;
#endif // #ifndef  DUMP4
   }
   else
   {
#ifdef   DUMP4
      dwn = dwa.dwCount;
      ialen = (INT)dwn;
#else // !DUMP4
      dwn = (DWORD) dwa.GetSize();
#endif   // DUMP4 y/n
      if( dwn < dwrel )
      {
         sprintf( lpd, "RELNOTE: WARNING: %d rels. only %d referenced! Missed:",
            dwrel, dwn );
         AddStringandAdjust(lpd);
         pir = (PIMAGE_RELOCATION) pRel;
         while(dwrel--)
         {
            for( i = 0; i < ialen; i++ )
            {
#ifdef   DUMP4
               if( dwa.pdwArray[i] == (DWORD)pir )
                  break;
#else    // !DUMP4
               if( dwa[i] == (DWORD)pir )
                  break;
#endif   // DUMP4 y/n
            }
            if( i == ialen )
            {
               *lpb = 0;
               dwo = pir->VirtualAddress;
               GetRelName( lpb, pRel, dwrel, pStgs, dwo, 0 );
               sprintf(lpd, "%8x %8x %s",
                  pir->VirtualAddress,
                  pir->SymbolTableIndex,
                  lpb );
               AddStringandAdjust(lpd);
            }
            pir++;   // bump to next reallocation pointer
         }
      }
      else
      {
         sprintf( lpd, "NOTE: All %d relocations referenced!", dwrel );
         AddStringandAdjust(lpd);
      }
   }
}
// ======================================================
#endif // #ifndef USE_PEDUMP_CODE // FIX20080507
// eof - DumpCode.c
