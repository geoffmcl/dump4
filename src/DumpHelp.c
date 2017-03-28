
// DumpHelp.c
#include	"Dump4.h"

typedef	struct	{
	int	iCnt;
	char * pPtrs[MXPTRS];
}CMDHDR;
typedef CMDHDR * PCMDHDR;

// forward references
DWORD	GetNumber( char * cp );

// date and time of last compile of this module
//#if (defined(BLDDATE) && defined(BLDTIME))
#if (defined(DUMP4_DATE) && defined(DUMP4_VERSION))
TCHAR g_szCDate[] = DUMP4_DATE;
TCHAR g_szCVers[] = DUMP4_VERSION;
TCHAR g_szCTime[] = "00:00:00";
#else
TCHAR g_szCDate[] = __DATE__;
TCHAR g_szCVers[] = "0.9.9";
TCHAR g_szCTime[] = __TIME__;
#endif
// application header

#if (defined(IS_64BIT_BUILD) || defined(_WIN64))    // 64-bit build
TCHAR	g_szDesc[] = "Dump4 64-Bits";
#else
TCHAR	g_szDesc[] = "Dump4 32-Bits";
#endif
TCHAR	g_szName[] = "HEX DUMP UTILITY";
//TCHAR	g_szDate[] = BLDDATE;

TCHAR	g_szHelp[] =
 	"Usage   : Dump4 [@]InputFile[s] [Switches]"MEOR
	"Switches: Each preceded by - or / space separated."MEOR
	"  -? or H This brief help. (?? or HH for more)"MEOR
    "  -a[a][n] Dump ASCII only. 2nd 'a' for alphanumeric; n is min length. (Def=4)."MEOR
	"  -bmp[n] Dump as a BITMAP file. n=1 for one line; 2 for add colors (24BBP)."MEOR
    "  -bmp[:br=nn:bc=nn:er=nn:ec=nn] to set begin and end row and column of the BITMAP."MEOR
#ifdef ADD_BLOCK_CMD
    "  -block:nn[Xn][:nn[Xn]] - Dump hex in BLOCKS."MEOR
#endif // ADD_BLOCK_CMD
	"  -cab    Dump as a MS Cabinet file."MEOR
	"  -cis    Dump as a Compuserve SUPPORT file. ADDRBOOK.DAT type"MEOR
	"  -Bnnn   Begin at offset nnn into file."MEOR
	"  -Ennn   End at offset nnn into file."MEOR
#ifdef   ADDOBJSW
#ifdef USE_PEDUMP_CODE
    "  -exe    See -obj[:options] below."MEOR
#else // #ifdef USE_PEDUMP_CODE
    "  -exe    Dump as a COFF/EXE/DLL Object file."MEOR
#endif // #ifdef USE_PEDUMP_CODE y/n
#endif   // ADDOBJSW
    "  -dd[o]  Dump as fixed VOS table file. o=only"MEOR
    "  -dfs    Dump as private DFS dataset."MEOR
#ifdef   ADDOBJSW
#ifdef USE_PEDUMP_CODE
    "  -dll    See -obj[:options] below."MEOR
#else // #ifdef USE_PEDUMP_CODE
    "  -dll    Dump as a COFF/EXE/DLL Object file."MEOR
#endif // #ifdef USE_PEDUMP_CODE y/n
#endif   // ADDOBJSW
    "  -dw[s]  Dump as DWORD entries, default as bytes. s=swap bigendians."MEOR
//  "  -g      Dump as geoff's telephone file."MEOR
//   "  -g[x[0|1|+|-]][o\"Name\"]   Dump as geoff's tele file. o=outfile name"MEOR
    "  -gif[n][t][:|+[outname.bmp]] GIF file. n=1 red.out t=xlat :|+=write bmp"MEOR
	"  -i[@]Nm Input Name (alternative method)."MEOR
#ifdef  ADDLNK
    "  -lnk    Dump as a Microsoft SHORTCUT (LNK) file."MEOR
#endif  // ADDLNK
#ifdef ADDARCH2
#ifdef USE_PEDUMP_CODE
    "  -lib    See -obj[:options] below."MEOR
#else // #ifdef USE_PEDUMP_CODE
    "  -lib    Dump as a Microsoft LIBRARY (.LIB archive) file."MEOR
#endif // #ifdef USE_PEDUMP_CODE y/n
#endif // #ifdef ADDARCH2
#ifdef   ADDMK4
    "  -mk4    Dump as MK4 file."MEOR
#endif   // #ifdef   ADDMK4
    "  -O[+|-]OutFile - Output to OutFile (+=append, -=new[default]"MEOR
    "  -o:[h|n] - Output file OFFSET as H=Hex(default), or N=Number."MEOR
#ifdef   ADDOBJSW
#ifdef USE_PEDUMP_CODE
    "  -obj[:options] - Dump as a COFF/EXE/DLL/LIB Object file. (:? to see options)"MEOR
#else // !#ifdef USE_PEDUMP_CODE
    "  -obj    Dump as a COFF/EXE/DLL Object file."MEOR
#endif   // #ifdef USE_PEDUMP_CODE y/n
#endif   // ADDOBJSW
//	"  -p      Remove parity bit before display."MEOR
	"  -p[pm]  Remove parity, [or convert ppm to bmp]."MEOR
#ifdef   DUMP_RGB2
    "  -rgb    Dump as an SGI RGB graphic file."MEOR
#endif   // #ifdef   DUMP_RGB2
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
    "  -shp    Dump as SHAPEFILE ..."MEOR
#endif // #ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
#ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file
    "  -sonic  Dump as Sonic project file."MEOR
#endif // #ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file
#ifdef ADD_TAR_FILE  // FIX20080908 - Add -tar - dump TAR format files
    "  -tar    Dump as TAR file ..."MEOR
#endif // #ifdef ADD_TAR_FILE
    "  -m2ts   Dump as M2TS HD Video file ..."MEOR
#ifdef ADD_AVI_FILE
    "  -avi    Dump as AVI (Audio Video Interleave) file ..."MEOR
#endif
	"  -v[n]   Verbosity. -v=-v1 -v0=Silent -vn=Level up to 9."MEOR
	"  -wav    Dump as WAVE file."MEOR
	"  -X[oha[u]] Exclude [Offset|Hex|Ascii] display. (-xohu=Unicode)"MEOR;

TCHAR g_szNotes[] =
	"Notes  1: Up to 200 Input files can be given."MEOR
	"       2: Input File Names can contains wildcards."MEOR
	"       3: Preceded with @ means further commands in file, line separated."MEOR
	"                                                  Happy Dumping!";

TCHAR	g_szHelpg[] =
	"Switch -g[xopis]: Dump as geoff's telephone file."MEOR
   "  -g[o[\"]Name[\"]] Output file name (gcGOutFile)."MEOR
   "  -gx[1|0]    Set/Reset Do Extra. (def. gbDoExtra=F)."MEOR
   "  -gp[1|0]    Set/Reset Show password(s) (def. gbChkPWD=T)."MEOR
   "  -gi[1|0]    Set/Reset Ignore 123ABC (def. gbIgnoreMNU=T)."MEOR
   "  -gs[1|0]    Set/Reset primary entry. (def. gbUseSee=F)."MEOR
#ifdef   ADDSYNPROF
   "Switch -prof: Dump as SYNEDIT profile type file."MEOR
#endif   // #ifdef   ADDSYNPROF
   "                                                  Happy Dumping!";

#ifdef   DUMP_RGB2
//         case 'R':
//				if( ( toupper(cp[1]) == 'G' ) &&
//					 ( toupper(cp[2]) == 'B' ) ) {
// g_bDumpRGB = TRUE;
#endif   // #ifdef   DUMP_RGB2

void	ShwTitle( void )
{
	char	buf[256];
	sprintf( &buf[0],
		"%s - %s - circa %s, version %s" MEOR,
		&g_szDesc[0],
		&g_szName[0],
		&g_szCDate[0],
        &g_szCVers[0] );
	prt( buf );
}

void showVersion()
{
    ShwTitle();
    MyExit0();
}

void	Usage( void )
{
	ShwTitle();
	prt( g_szHelp  );
	prt( g_szNotes );
}

void	UsageX( void )
{
	Usage();
	MyExit1();
}

void	Usage0( void )
{
	Usage();
	MyExit0();
}

void	UsageG( void )
{
	ShwTitle();
	prt( g_szHelpg );
	MyExit0();
}

#ifdef USE_PEDUMP_CODE  // FIX20080507 - switch to PEDUMP code

TCHAR g_szHelpPE[] =
"PEDUMP options - Switch -obj[:options[+|-] - '+' On (def), '-' Off"MEOR
" A - include ALL, well BHILPRS, in dump."MEOR
" B - show Base relocations.(def=Off)"MEOR
" C = dump seCtions: def = ON."MEOR
" D = dump DOS, or other File Header: def = ON."MEOR
#ifdef ADD_EXE_FOLLOW
" F = follow import trail, using PATH to find, and dump imported DLLs: def = OFF."MEOR
#endif
" G = dump debuG description: def = ON."MEOR
" H - include Hex dump of sections.(def=Off)"MEOR
" I - include Import Address Table thunk addresses.(def=Off)"MEOR
" L - include Line number information.(def=Off)"MEOR
" M = dump iMport Names: def = ON."MEOR
" O = dump Optional Header: def = ON."MEOR
" P - include PDATA (runtime functions).(def=Off)"MEOR
" R - include detailed Resources (stringtables and dialogs).(def=Off)"MEOR
" S - show Symbol table.(def=Off)"MEOR
" T = dump daTa directory: def = ON."MEOR
" X = dumps only first linker, skipping the rest. (def = OFF)."MEOR
" special ':imports' - set all above off, except Import Names."MEOR
" and ':exports' set all off, except First Linker Member: see X above."MEOR
"This applies to -dll, -exe, -lib with the same options, ABCDHILMOPRST.";

TCHAR g_HelpHD[] =
"                                                        Happy Dumping!";

void ShowCurrPEOpts( void )
{
    if ( VERB )
        sprtf( "Current DumpPE Options: %s"MEOR, Get_Current_Opts() );
}

void	UsagePE( void )
{
	ShwTitle();
	sprtf( "%s"MEOR, g_szHelpPE );
   ShowCurrPEOpts();
   prt( g_HelpHD );
	MyExit0();
}

void Get_PEDUMP_Sw( PTSTR ptmp, PTSTR pbgn )
{
   INT      c1, c2;
   SETFUNC spe = NULL;
   BOOL     act;
   if(*ptmp) {
      if( *ptmp == ':' ) {
         ptmp++;
         while(*ptmp)
         {
            c1 = toupper(ptmp[0]);
            c2 = toupper(ptmp[1]);
            act = TRUE;
            spe = NULL;
            if( _strnicmp(ptmp, "imports", 7) == 0 ) {
               Set_Imports_Only();
               ptmp = &ptmp[7];
            } else if( _strnicmp(ptmp, "exports", 7) == 0 ) {
               Set_Exports_Only();
               ptmp = &ptmp[7];
            } else {
               switch(c1)
               {
               case '?':
                  UsagePE();
                  break;
               case 'A':
                  spe = Set_PEDUMP_A;
                  break;
               case 'B':   // - show Base relocations
                  spe = Set_PEDUMP_B;
                  break;
               case 'C':
                  spe = Set_PEDUMP_C;  // fDumpSectionTable
                  break;
               case 'D':
                  spe = Set_PEDUMP_D;
                  break;

               case 'F':
                  spe = Set_PEDUMP_F;
                  break;
               case 'G':
                  spe = Set_PEDUMP_G;
                  break;
               case 'H':   // include Hex dump of sections
                  spe = Set_PEDUMP_H;
                  break;
               case 'I':   // include Import Address Table thunk addresses
                  spe = Set_PEDUMP_I;
                  break;
               case 'L':   // include Line number information
                  spe = Set_PEDUMP_L;
                  break;
               case 'M':
                  spe = Set_PEDUMP_M;
                  break;
               case 'O':
                  spe = Set_PEDUMP_O; // fDumpOptionalHeader
                  break;
               case 'P':   // include PDATA (runtime functions)
                  spe = Set_PEDUMP_P;
                  break;
               case 'R':   // include detailed Resources (stringtables and dialogs)
                  spe = Set_PEDUMP_R;
                  break;
               case 'S':   // show Symbol table
                  spe = Set_PEDUMP_S;
                  break;
               case 'T':   // Dump Data Directory
                  spe = Set_PEDUMP_T;
                  break;
               case 'X':   // Dump First linker table only - fDumpExportsOnly
                  Set_Exports_Only();
                  break;
               default:
                  sprtf( "ERROR: Unknown switch [%s] ... aborting!"MEOR, pbgn );
                  UsagePE();
                  break;
               }

               if((c2 == '+')||(c2 == '-'))
               {
                  ptmp++;
                  if(c2 == '-')
                     act = FALSE;
               }
               if(spe)
                  spe(act);
               ptmp++;
            }
         }
      } else {
         sprtf( "ERROR: [%s] NOT VALID! Character ':' must precede options!"MEOR,
               pbgn );
        	MyExit1();
      }
   }
}
#endif // #ifdef USE_PEDUMP_CODE


int	LastLen( LPSTR lps )
{
	int		i, j, k;
	char	c;
	k = 0;
	if( lps && (i = lstrlen( lps )) )
	{
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c < ' ' )
			{
				if( c == '\n' )
					k = 0;
				else if( c == '\t' )
					k += 6;
			}
			else
			{
				k++;
			}
		}
	}
	return k;
}

BOOL	IsNumber( LPSTR lps )
{
	BOOL	flg = FALSE;
	int		i, j;
	char	c;
	if( lps &&
		( ( i = lstrlen( lps ) ) != 0 ) )
	{
		for( j = 0; j < i; j++ )
		{
			if( !( ( (c = lps[j]) == ' ' ) ||
				( ( c >= '0' ) && ( c <= '9' ) ) ) )
			{
				break;
			}
		}
		if( j == i )
			flg = TRUE;
	}
	return flg;
}

void Get_L_Sw( PTSTR cp )
{
   PTSTR ptmp = g_cTmp;
   PTSTR pbgn = &cp[-1];
   size_t len;
   strcpy(ptmp, cp);
   len = strlen(ptmp);
   if( _strnicmp(ptmp, "lib", 3) == 0 ) {
      g_bDumpLib = TRUE;
#ifdef USE_PEDUMP_CODE
      ptmp = &ptmp[3];
      Get_PEDUMP_Sw( ptmp, pbgn );
#endif // #ifdef USE_PEDUMP_CODE
#ifdef  ADDLNK
   } else if( ( toupper(cp[1]) == 'N' ) &&
              ( toupper(cp[2]) == 'K' ) )
   {
      g_bDumpLNK = TRUE;
#endif  // ADDLNK
   } else {
#ifdef  ADDLNK
      prt( "ERROR: NOT -LIB or -LNK switch!"MEOR );
#else // !#ifdef ADDLNK
		prt( "ERROR: NOT -LIB switch!"MEOR );
#endif  // #ifdef ADDLNK y/n
  		UsageX();
   }
}


extern   BOOL  OpenOut( LPTSTR lpf, HANDLE * pHand, BOOL bAppend );

INT  Get_O_Sw( PTSTR cp )
{
   INT      iRet = 0;   // no error yet
   LPTSTR   lpb = &gszDiag[0];
   INT      c;
   size_t   len;
   PTSTR pbgn = cp - 2;
   PTSTR ptmp = g_cTmp;

   strcpy(ptmp, &cp[-1]);
   len = strlen(ptmp);
#ifdef   ADDOBJSW
   if( _strnicmp(ptmp, "obj", 3) == 0 ) {
      g_bDumpObj = TRUE;
#ifdef USE_PEDUMP_CODE
      ptmp = &ptmp[3];
      Get_PEDUMP_Sw( ptmp, pbgn );
#endif // #ifdef USE_PEDUMP_CODE
      return 0;
   }
#endif   // ADDOBJSW
   if( *cp )
   {
      if(*cp == ':') {
         cp++;
         c = toupper(*cp);
         if( c == 'H' )
            g_iNumbOffset = 0;
         else if( c == 'N' )
            g_iNumbOffset = 1;
         else {
            sprintf( lpb, "ERROR: [%s] NOT VALID! Only -o:h(hex-default), or -o:n(number) allowed!"MEOR,
               pbgn );
            prt(lpb);
         	MyExit1();
         }
      } else if(*cp == '+') {
         g_bAppend = TRUE;
         cp++;
      } else if( *cp == '-' ) {
         g_bAppend = FALSE;
         cp++;
      }
      strcpy( g_szOutFile, cp );
      if( !OpenOut( g_szOutFile, &g_hOutFile, g_bAppend ) )
      {
         sprintf( lpb, "ERROR: Could not %s output file [%s]!"MEOR,
            ( g_bAppend ? "Open" : "Create" ),
            g_szOutFile );
         prt(lpb);
      	MyExit1();
      }
   }
   else
   {
#ifdef   ADDOBJSW
	   prt( "ERROR: -O[+|-]OutFile or -obj switch error!"MEOR );
#else // !#ifdef   ADDOBJSW
	   prt( "ERROR: -O[+|-]OutFile switch error!"MEOR );
#endif   // #ifdef   ADDOBJSW y/n
  	   UsageX();
   }
   return iRet;
}

int Get_M_Sw( PTSTR cp )
{
   PTSTR ptmp = g_cTmp;
#ifdef   ADDMK4
    if( ( toupper(cp[1]) == 'K' ) &&
        ( toupper(cp[2]) == '4' ) )
    {
       g_bDumpMK4 = TRUE;
       return 0;
    }
#endif   //#ifdef   ADDMK4
    if( _strnicmp(cp, "m2ts", 4) == 0  ) {
        g_bDoM2TS = TRUE;
        return 0;
    }

    sprintf(ptmp, "ERROR: -%s switch error!"MEOR, cp );
    prt(ptmp);
    UsageX();
    return 1;
}


void Get_S_Sw( PTSTR cp )
{
   PTSTR pbgn = cp - 1;
   PTSTR ptmp = &gszDiag[0];
   strcpy(ptmp,cp);
   if( _strnicmp(ptmp, "shp", 3) == 0  ) {
      gfDoSHPFile = 1;
#ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file
   } else if (_strnicmp(ptmp, "sonic", 5) == 0 ) {
      gfDoSonic = 1;
#endif // #ifdef ADD_SONIC_PROJECT    // 2010-02-16 - add -sonic DVD project file
   } else {
      sprintf(ptmp, "ERROR: Only -S?? switches are -shp or -sonic! Got %s!!"MEOR, pbgn );
      prt(ptmp);
  	  UsageX();
   }
}

// FIX20080908 - Add -tar - dump TAR format files
void Get_T_Sw( PTSTR cp )
{
   PTSTR pbgn = cp - 1;
   PTSTR ptmp = &gszDiag[0];
   strcpy(ptmp,cp);
   if( _strnicmp(ptmp, "tar", 3) == 0  ) {
      gfDoTARFile = 1;
   } else {
      sprintf(ptmp, "ERROR: Only -T switch is -tar! Got %s!!"MEOR, pbgn );
      prt(ptmp);
  	   UsageX();
   }
}

// -a[a][n] or -avi
void Do_A_Opt( PTSTR cp )
{
   PTSTR pbgn = cp - 2;
#ifdef ADD_AVI_FILE
    // BOOL w_bDoAVI; // g_bDoAVI - process as an AVI file
    if( stricmp(cp, "vi") == 0  ) {
        g_bDoAVI = TRUE;
        return;
    }
#endif // #ifdef ADD_AVI_FILE
    // 20091112 - add -aa to restrict to alphanumeric output
    if ( *cp && (toupper(*cp) == 'A')) {
        gfDoASCII2 = TRUE;
        cp++;
    }

   if( *cp ) {
      if( IsNumber( cp ) ) {
         giMinASCII = GetNumber( cp );
         if( giMinASCII == 0 )
            goto Bad_A_Opt;

      } else {
Bad_A_Opt:
         sprtf( "ERRANT COMMAND: [%s]!"MEOR, pbgn );
	   	prt( "ERROR: Only decimal number greater than ZERO can follow -A!"MEOR );
   		UsageX();
      }
   }
   gfDoASCII = TRUE;
}

// FIX20080201 - add -blocks:nn[Xn][:nn[Xn]]...
int Do_BLOCK_CMD( PTSTR pbgn, PTSTR pcmd )
{
   int iret = 0;
   DWORD dwCnt = 0;
   PTSTR ptmp1 = g_cBuf2;
   PTSTR ptmp2 = g_cBuf3;
   size_t len = strlen(pcmd);
   size_t i, j, k;
   DWORD num, cnt;
   int c;
   for( i = 0; i < len; i++ )
   {
      if( pcmd[i] == ':' )
         dwCnt++;
   }
   // got COUNT of BLOCKS
   dwCnt++;
   g_pHexBlock = dMALLOC( sizeof(HEXBLOCK) * dwCnt );
   CHKMEM(g_pHexBlock);
   dwCnt = 0;
   j = 0;
   k = 0;
   for( i = 0; i < len; i++ )
   {
      c = pcmd[i];
      if( c == ':' ) {
         i++;
         if( pcmd[i] ) {
            j = 0;
            k = 0;
            for( ; i < len; i++ )
            {
               c = toupper(pcmd[i]);
               if(( c == ':' )||( c == 0 )) {
                  break;
               } else if( ISNUM(c) ) {
                  ptmp1[j++] = (TCHAR)c;
               } else if( c == 'X' ) {
                  break;
               } else {
                  goto Bad_BLOCK;
               }
            }
            if( c == 'X' ) {
               i++;
               for( ; i < len; i++ )
               {
                  c = pcmd[i];
                  if(( c == ':' )||( c == 0 )) {
                     break;
                  } else if( ISNUM(c) ) {
                     ptmp2[k++] = (TCHAR)c;
                  } else {
                     goto Bad_BLOCK;
                  }
               }
            }
            if(j) {
               ptmp1[j] = 0;
               num = atoi(ptmp1);
            } else {
               num = 1;
            }
            if(k) {
               ptmp2[k] = 0;
               cnt = atoi(ptmp2);
            } else {
               cnt = 1;
            }
            i--;
         } else {
            num = 1;
            cnt = 1;
         }
         g_pHexBlock[dwCnt].num = num;
         g_pHexBlock[dwCnt].cnt = cnt;
         dwCnt++;
      } // after a colon ':' ...
   }
   if(dwCnt) {
      for(num = 0; num < dwCnt; num++) {
         if(g_pHexBlock[num].num == 0)
            g_pHexBlock[num].num = 1;
         if(g_pHexBlock[num].cnt == 0)
            g_pHexBlock[num].cnt = 1;
      }
      g_pHexBlock[dwCnt].num = 0;
      g_pHexBlock[dwCnt].cnt = 0;
   } else {
      g_pHexBlock[dwCnt].num = 1;
      g_pHexBlock[dwCnt].cnt = 1;
      dwCnt++;
      g_pHexBlock[dwCnt].num = 0;
      g_pHexBlock[dwCnt].cnt  = 0;
   }
   goto Done_BLOCK;
Bad_BLOCK:
   sprtf( "ERRANT COMMAND: [%s]!"MEOR, pbgn );
   prt( "ERROR: Only decimal number can follow -block:!"MEOR );
   prt( "\tOr -BLOCK:nn[Xn]:nn[Xn]...!"MEOR );
	UsageX();
Done_BLOCK:
   return iret;


}

void Do_B_Opt( PTSTR cp )
{
   PTSTR pbgn = cp - 2;
   if( IsNumber( cp ) ) {
		gdwBgnOff = GetNumber( cp );
	}
#ifdef	ADDBMPSW
	else if( ( toupper(cp[0]) == 'M' ) &&
			   ( toupper(cp[1]) == 'P' ) )
	{
      PTSTR pb, pb2;
      int   cnt;
		gfDoBMP = TRUE;
      cp++;
      cp++;
      while(*cp) {
         if( ISNUM(*cp) ) {
            pb = GetNxtBuf();
            cnt = 0;
            while( ISNUM(*cp) ) {
               pb[cnt] = *cp;
               cnt++;
               cp++;
            }
            pb[cnt] = 0;
            //giBmpVerb = *cp - '0';
            giBmpVerb = atoi(pb);
         } else if( *cp == ':' ) {
            cp++;
            if( ISALPHA( *cp ) ) {
               pb = GetNxtBuf();
               cnt = 0;
               while( ISALPHA(*cp) ) {
                  pb[cnt] = toupper(*cp);
                  cnt++;
                  cp++;
               }
               pb[cnt] = 0;
               if( cnt == 2 ) {
                  // get
                  if( *cp != '=' )
                     goto Not_BE;
                  cp++;
                  if( !ISNUM(*cp) )
                     goto Not_BE;
                  pb2 = GetNxtBuf();
                  cnt = 0;
                  while( ISNUM(*cp) ) {
                     pb2[cnt] = *cp;
                     cnt++;
                     cp++;
                  }
                  pb2[cnt] = 0;
                  // BR = begin row
                  // BC = begin column
                  // ER = end row
                  // EC = end column
                  if( pb[0] == 'B' ) {
                     if( pb[1] == 'R' ) {
                        g_BmpBgnEnd[bgn_row] = atoi(pb2);
                     } else if( pb[1] == 'C' ) {
                        g_BmpBgnEnd[bgn_col] = atoi(pb2);
                     } else {
                        goto Not_BE;
                     }
                  } else if( pb[0] == 'E' ) {
                     if( pb[1] == 'R' ) {
                        g_BmpBgnEnd[end_row] = atoi(pb2);
                     } else if( pb[1] == 'C' ) {
                        g_BmpBgnEnd[end_col] = atoi(pb2);
                     } else {
                        goto Not_BE;
                     }
                  } else {
                     goto Not_BE;
                  }
               } else {
Not_BE:
                  sprtf("ERROR: NOT BR=begin row, BC=begin column ER=end row or EC=end column"MEOR );
                  goto Bad_B_Opt;
               }
            } else {
               goto Bad_B_Opt;
            }
         }
      }
	}
#endif	/* ADDBMPSW */
#ifdef	ADD_BLOCK_CMD
	else if( ( toupper(cp[0]) == 'L' ) &&
			   ( toupper(cp[1]) == 'O' ) &&
			   ( toupper(cp[2]) == 'C' ) &&
			   ( toupper(cp[3]) == 'K' ) &&
			   ( toupper(cp[4]) == 'S' ) &&
            ( toupper(cp[5]) == ':' ) )
	{
      // we have a BLOCKS:nn
      Do_BLOCK_CMD( pbgn, &cp[5] );
   }
#endif // ADD_BLOCK_CMD
	else
	{
Bad_B_Opt:
      sprtf( "ERRANT COMMAND: [%s]!"MEOR, pbgn );
		prt( "ERROR: Only decimal number can follow -B!"MEOR );
      prt( "\tOr -BMP[n][:br=nn:bc=nn:er=nn:ec:nn] for a BITMAP file!"MEOR );
		UsageX();
	}
}

void Do_D_Opt( PTSTR cp )
{
   PTSTR pbgn = cp - 2;
   PTSTR ptmp = &gszDiag[0];
   INT c = toupper(*cp);
   strcpy(ptmp, &cp[-1]);
#ifdef   ADDOBJSW
   if( _strnicmp(ptmp, "dll", 3) == 0  ) {
      g_bDumpObj = TRUE;
#ifdef USE_PEDUMP_CODE
      ptmp = &ptmp[3];
      Get_PEDUMP_Sw( ptmp, pbgn );
#endif // #ifdef USE_PEDUMP_CODE
      return;
   }
#endif   // ADDOBJSW
   if( c == 'D' ) {
      gfDoVosTab = TRUE;
      cp++;
		c = toupper(*cp);
		if( c ) {
         if( c == 'O' )
            gfDoVosOnly = TRUE;
         else
            goto Bad_Command;
      }
   } else if( c == 'W' ) {
      g_DoAsDWORDS = 1;
      cp++;
      c = toupper(*cp);
      if(c) {
         if( c == 'S' )
            g_SwapBytes = 1;
         else
            goto Bad_Command;
      }
   } else if( c == 'F' ) {
      cp++;
      c = toupper(*cp);
      if( c == 'S' )
         gfDoDFS = TRUE;
      else
         goto Bad_Command;
   } else {
Bad_Command:
      sprtf( "ERRANT COMMAND: [%s]!"MEOR, pbgn );
		prt( "ERROR: Only letters D[O], W[S] or FS, can follow -D! ie -DD[O] Vos TAB, -DW[S] DWORD ouput, -DFS - Private dataset."MEOR );
  		UsageX();
   }
}

// see Get_O_Sw also, for -obj
void Do_E_Opt(PTSTR cp)
{
   PTSTR pbgn = cp - 1;
   PTSTR ptmp = &gszDiag[0];
   strcpy(ptmp,cp);
#ifdef   ADDOBJSW
   if( _strnicmp(ptmp, "exe", 3) == 0  ) {
      g_bDumpObj = TRUE;
#ifdef USE_PEDUMP_CODE
      ptmp = &ptmp[3];
      Get_PEDUMP_Sw( ptmp, pbgn );
#endif // #ifdef USE_PEDUMP_CODE
      return;
   }
#endif   // ADDOBJSW
   cp++;
   if( IsNumber( cp ) ) {
	   gdwEndOff = GetNumber( cp );
   } else {
	   prt( "ERROR: Only decimal numbers can follow -E! %s"MEOR );
	   UsageX();
   }
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ProcessCommand
// Return type: int 
// Arguments  : int argc
//            : char *argv[]
//            : LPSTR lpc
// Description: Process the command line
//              
///////////////////////////////////////////////////////////////////////////////
int ProcessCommand( int argc, char *argv[], LPSTR lpc )
{
	int      iret, i, j, k;
	char *	cp;
    char * bcp;
	char *   *fn;
	char	   c, d;
	int		len;
	char *	buf;
	LPTSTR	lpb, lpb2;
   PBOOL    pb;

	iret = 0;
	if( argc < 2 )
		Usage0();

	//lpb = &gcFilBuf[0];
	lpb = &gszDiag[0];   // diagnostic information buffer
	for( i = 1; i < argc; i++ )
	{
		bcp = argv[i];
        cp = bcp;
		len = strlen( cp ); 
        if (strcmp(cp, "--version") == 0)
            showVersion();

   	if( ( lpc                           ) &&
	   	( (j = lstrlen(lpc)) < MXCMDBUF ) )
	   {
			if( (LastLen( lpc ) + lstrlen( cp )) >= MXONELN )
			{
				strcat( lpc, "\n\t" );
			}
			sprintf( (lpc + lstrlen( lpc )),
				"%s ", cp );
	   }
		c = *cp;
		if( ( c == '-' ) || ( c == '/' ) )
		{
			cp++;	// Bump to NEXT
            if (( c == '-' ) && ( *cp == '-' ))
                cp++;   // bump past the second
			c = toupper(*cp);
			switch( c )
			{
         case 'A':
            cp++;
            Do_A_Opt(cp);
            break;
			case 'B':
				cp++;
            Do_B_Opt(cp);
				break;

			case 'C':
				cp++;
				if( ( toupper(cp[0]) == 'I' ) &&
					 ( toupper(cp[1]) == 'S' ) )
				{
					gfDoCISAddr = TRUE;
				}
            else if( ( toupper(cp[0]) == 'A' ) &&
						 ( toupper(cp[1]) == 'B' ) )
            {
               if( LoadCabLib() ) {
                  gfDoCABFile = TRUE;
               } else {
                  prt( "ERROR: Unable to load CABINET.DLL!"MEOR );
   					UsageX();
               }
            }
				else
				{
					prt( "ERROR: Only the letters IS or AB can follow -C!"MEOR );
					prt( "\tie -CIS for Compuserve or -CAB for MS Cabinet file!"MEOR );
					UsageX();
				}
				break;

         case 'D':
            cp++;
            Do_D_Opt(cp);
            break;

			case 'E':
            Do_E_Opt(cp);
				break;

         case 'G':
            cp++;
            if(len >= 4)   // -gif[n] option
            {
               if(( toupper(cp[0]) == 'I' ) &&
                  ( toupper(cp[1]) == 'F' ) )
               {
                  g_bDumpGif = 1;
                  if(len > 4)
                  {
                     if( toupper(cp[2]) == 'T' )
                     {  // like -gift1:logo5.bmp
                        g_bTransTran = TRUE; // translate the TRANSPARENT colour to 'something' command '-gifT[...]
                        cp++;
                     }

                     if(ISNUMERIC(cp[2]))
                     {
                        g_bDumpGif += atoi(&cp[2]);
                        while( ISNUMERIC(cp[2]) )
                           cp++; // bump over the number, if any
                     }

                     if(( cp[2] == ':' ) || (cp[2] == '+'))
                     {
                        g_bWriteBmp = TRUE;

                        if(cp[2] == '+')
                           g_bOutLess = FALSE;  // force use of 256 RGBQUAD palette

                        if(cp[3])
                        {
                           strcpy(g_szbmpout, &cp[3] );  // get user's desired name
                        }
                     }
                  }
                  break;   // done GIF option
               }
            }
            gbDoGeoff = TRUE;
            while( ( c = toupper(*cp) ) != 0 )
				{
               switch(c)
               {
               case 'O':
                  {
                     // output filename
                     LPTSTR   lpf = &gcGOutFile[0];
                     cp++;
                     c = *cp;
                     d = 0;
                     if( c == '"' )
                     {
                        d = c;
                        cp++;
                        c = *cp;
                     }
                     while( c = *cp )
                     {
                        if( ( d ) &&
                           ( c == d ) )
                        {
                           cp++;
                           break;
                        }
                        *lpf++ = c;
                        cp++;
                     }
                     *lpf = 0;
                  }
                  break;
//  gbDoExtra;  // x
//  gbChkPWD;   // = TRUE; p[0|1|+|-]
//  gbIgnoreMNU;// = TRUE; i // ignore 123ABC
//  gbUseSee;  // = FALSE; s // add see (primary entry)
               case 'X':
               case 'P':
               case 'I':
               case 'S':
                  {
//                     LPBOOL pb;
                     if( c == 'X' )
                        pb = &gbDoExtra;
                     else if( c == 'P' )
                        pb = &gbChkPWD;
                     else if( c == 'I' )
                        pb = &gbIgnoreMNU;
                     else if( c == 'S' )
                        pb = &gbUseSee;

                     //gbDoExtra = TRUE;
                     *pb = TRUE;
                     cp++;
                     c = *cp;
                     if( ( c == '0' ) ||
                        ( c == '-' ) )
                     {
                        //gbDoExtra = FALSE;
                        *pb = FALSE;
                        cp++;
                        c = *cp;
                     }
                     else if( ( c == '1' ) ||
                        ( c == '+' ) )
                     {
                        //gbDoExtra = TRUE;
                        *pb = TRUE;
                        cp++;
                        c = *cp;
                     }
                  }
                  break;

               default:
                  {
   					   prt( "ERROR: -G switch error!"MEOR );
	   				   UsageX();
                  }
                  break;
               }
            }
            break;

			case '?':
			case 'H':
            cp++;
            c = toupper(*cp);
            if( ( c == '?' ) || ( c == 'H' ) )
               UsageG();
            else
               Usage0();
				break;

			case 'I':
				fn = &gpFilNames[giInCount];
				len = strlen( cp ); 
				if(len)
				{
					buf = malloc( (len + 1) ); 
					if(buf)
					{
						strcpy( buf, cp );
						*fn = buf;
						giInCount++;
					}
					else
					{
						prt( "ERROR: Memory allocation FAILED!"MEOR );
						UsageX();
					}
				}
				if( giInCount >= MXFILNAMES )
				{
					prt( "ERROR: Too many INPUT file names!"MEOR );
					UsageX();
				}
				break;

#ifdef   ADDARCH2
         case 'L':
            Get_L_Sw(cp);
            break;
#else    // !ADDARCH2
#ifdef  ADDLNK
         case 'L':
             if( ( toupper(cp[1]) == 'N' ) &&
				 ( toupper(cp[2]) == 'K' ) )
             {
                g_bDumpLNK = TRUE;
             } else {
    		    prt( "ERROR: NOT -LNK switch!"MEOR );
             }
             break;
#endif  // ADDLNK
#endif   //  ADDARCH2 y/n

         case 'M':
             Get_M_Sw(cp);
            break;

         case 'O':
            cp++;
            Get_O_Sw(cp);
            break;

			case 'P':
				cp++;
            j = strlen(cp);
				if( j )
				{
               for( k = 0; k < j; k++ )
               {
                  //lpb[k] = (char)toupper( cp[k] );
                  lpb[k] = cp[k];
               }
               lpb[k] = 0;
               // do we have a SYNEDIT -prof "profile" file
#ifdef   ADDSYNPROF  // #define  ADDSYNPROF
               if( STRCMPI(lpb, "ROF") == 0 )
               {
                  gbDoSynEdit = TRUE;
                  break;
               }
#endif   // #ifdef   ADDSYNPROF
               if( strbgni(lpb, "PM") == 0 )
               {
                  g_bDoPPM = TRUE;
                  lpb2 = &lpb[2];
                  if( *lpb2 )
                  {
                     if( *lpb2 != ':' )
                        goto Err_P;
                     lpb2++;
                     strcat(g_szBmpNm, lpb2);
                  }
                  if( g_szBmpNm[0] == 0 )
                  {
                     strcpy( g_szBmpNm, "TEMPPPM.BMP" );
                  }
                  break;
               }
Err_P:
               cp -= 2;
               sprintf(lpb, "ERROR: Switch [%s] is NOT -p, ppm[:nm] or -prof switch!"MEOR,
                  cp );
               prt(lpb);
               UsageX();
				}
            else
            {
               gfRemPar = TRUE;
            }
				break;
#ifdef   DUMP_RGB2
         case 'R':
            if(( len >= 4              ) &&  // -rgb option
				   ( toupper(cp[1]) == 'G' ) &&
					( toupper(cp[2]) == 'B' ) )
            {
               if(( len >= 6 ) &&
                  ( cp[3] == ':' ) )   // -rgb:outbm2.bmp
               {
                  extern char  g_szRgbBmp[]; // [264] = { "temprgb6.bmp" };
                  if(cp[4] == '0')
                     g_szRgbBmp[0] = 0;
                  else
                     strcpy(g_szRgbBmp, &cp[4]);
               }
               g_bDumpRGB = TRUE;
            }
            else
            {
				   prt( "ERROR: -LIB switch error!"MEOR );
  				   UsageX();
            }
            break;
#endif   // #ifdef   DUMP_RGB2
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
         //   "  -shp    Dump as SHAPEFILE ..."MEOR
         case 'S':
            Get_S_Sw( cp );
            break;
#endif // #ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
#ifdef ADD_TAR_FILE  // FIX20080908 - Add -tar - dump TAR format files
         //   "  -tar    Dump as TAR file ..."MEOR
         case 'T':
            Get_T_Sw( cp );
            break;
#endif // #ifdef ADD_TAR_FILE

			case 'V':   // VERBOSITY
				cp++;
				c = *cp;
				if( c == 0 )
					giVerbose = 1;
				else if( ( c >= '0' ) && ( c <= '9' ) )
					giVerbose = (int) (c - '0');
				else
				{
					prt( "ERROR: Errant -v switch!"MEOR );
					UsageX();
				}
				break;

         // FIX20081204 - Added -wav to process WAVE file
			case 'W':   // -wav - WAVE file
				cp++;
				if(( toupper(cp[0]) == 'A' )&&
               ( toupper(cp[1]) == 'V' ))
            {
               cp++;
               cp++;
               gfDoWAV = 1;
            }
            if(*cp)
            {
					prt( "ERROR: Errant -wav switch!"MEOR );
					UsageX();
            }
            break;

			case 'X':   // -X[OHA] switch
   				cp++; // bump to NEXT
				j = strlen( cp ); 
				if(j)
				{
					for( k = 0; k < j; k++ )
					{
						c = toupper( cp[k] );
						switch( c )
						{
						case 'O':
							gfAddOffset = FALSE;
							break;
						case 'H':
							gfAddHex = FALSE;
							break;
						case 'A':
							gfAddAscii = FALSE;
							break;
                  case 'U':
                     gfUnicode = TRUE; // be UNICODE aware (and improve output display)
							//gfAddOffset = FALSE;
							//gfAddHex = FALSE;
                     break;
						default:
							{
								sprintf(lpb, "ERROR: Unknown switch following -X! (%s?)"MEOR,
                           		argv[i] );
                        prt(lpb);
								UsageX();
							}
							break;
						}
					}
				}
				else
				{
					prt( "ERROR: Nothing following -X! switch. Need O, H, or A."MEOR );
					UsageX();
				}
				break;
            case '@':
                goto Do_Input_File;
                break;

			default:
				cp--;
				sprintf( lpb, "ERROR: [%s] Unknown switch!"MEOR, bcp );
				prt( lpb );
				UsageX();
				break;
			}
		}
		else if( c == '@' )
		{
			FILE	*	hf;
			DWORD		fs, fr, fii, fib, fibu;
			char	*	fcmd;
			char	*	fb;
			PCMDHDR	pch;
			BOOL		flg;

 Do_Input_File:
 			cp++;
            hf = fopen(cp,"r");
         if( hf )
			{
			    
				//fs = GetFileSize( (HANDLE)hf, NULL );
				fseek( hf, 0, SEEK_END );
				fs = ftell( hf );
				fseek( hf, 0, SEEK_SET );
				if( ( fs              ) &&
					 ( fs != (DWORD)-1 ) )
				{
					// Use allocated memory
               fii = (fs+15+sizeof(CMDHDR));
					fcmd = malloc( fii ); 
					if(fcmd)
					{
                  ZeroMemory(fcmd, fii); 
						fb = fcmd + sizeof(CMDHDR);
						pch = (PCMDHDR)fcmd;
//						if( ( fr = _lread( hf, fb, fs ) ) &&
                        fr = fread(fb,1,fs,hf);
						if( ( fr ) &&
							( fr == fs ) )
						{
							flg = FALSE;
							pch->iCnt = 1;	// A nothing
							pch->pPtrs[0] = 0;	// an NO pointer
							for( fii = 0; fii < fs; fii++ )
							{
								c = fb[fii];
								if( c == ';' )
								{
									// Throw the comment line
									fii++;
									for( ; fii < fs; fii++ )
									{
										//if( fb[fii] < ' ' )
                              c = fb[fii];
										if( ( c < ' ' ) && ( c != 0x09 ) )
                                 break;
									}
								}
								else if( c > ' ' )
								{
                           // we have a value ABOVE a space
                           flg = FALSE;
									if( c == '"' )
									{
										flg = TRUE;
										fii++;
									}
                           fib = fii;  // keep the beginning
									pch->pPtrs[pch->iCnt] = &fb[fii];
									pch->iCnt++;	// Count a POINTER
									if( pch->iCnt >= MXPTRS )
									{
										sprintf( lpb, "ERROR: WOW! Too many arguments in file [%s]!\nSplit into multiple files!"MEOR, cp );
										prt( lpb );
										pgm_exit( 1 );
									}
									fii++;
									for( ; fii < fs; fii++ )
									{
										c = fb[fii];
										if( flg )
										{
											if( c == '"' )
											{
												fii++;   // bump past tailing '"' char
												fb[fii] = 0;   // set zero terminator
												break;
											}
										}
										else
										{
											//if( c <= ' ' )
											if( ( c < ' ' ) || ( c == ';' ) )
											{
												fb[fii] = 0;
                                    // kill any trailing space
                                    for( fibu = fii; fibu >= fib; fibu-- )
                                    {
                                       if( fb[fibu] > ' ' )
                                          break;
                                       fb[fibu] = 0;
                                    }
												break;
											}
										}
									}
								}
							}
							if( pch->iCnt > 1 )	// GOT something?
							{
								ProcessCommand( pch->iCnt,
									&pch->pPtrs[0],
									lpc );
							}
						}
						else
						{
							free( fcmd );
							prt( "ERROR: Read File FAILED!"MEOR );
							fclose( hf );
							UsageX();
						}
						free( fcmd );
					}
					else
					{
						prt( "ERROR: Memory allocation FAILED!"MEOR );
						fclose( hf );
						UsageX();
					}
				}
				else
				{
					prt( "ERROR: Unable to get file size!"MEOR );
					fclose( hf );
					UsageX();
				}
				fclose( hf );
			}
			else
			{
				sprintf( lpb,
					"ERROR: Unable to open INPUT file [%s]!"MEOR,
					cp );
				prt( lpb );
				MyExit1();
			}
		}
		else
		{
			fn = &gpFilNames[giInCount];
			if( len = lstrlen( cp ) )
			{
				buf = malloc( (len + 1) ); 
				if(buf)
				{
					strcpy( buf, cp );
					*fn = buf;
					giInCount++;
				}
				else
				{
					prt( "ERROR: Memory allocation FAILED!"MEOR );
					UsageX();
				}
			}
			if( giInCount >= MXFILNAMES )
			{
				prt( "ERROR: Too many INPUT file names!"MEOR );
				UsageX();
			}
		}
	}
	return iret;
}

//#include	<gwscanf.h>
DWORD	GetNumber( char * cp )
{
	DWORD	dw;
	dw = 0;
   sscanf( cp, "%lu", &dw );
	return dw;
}

// eof - DumpHelp.c
