// DumpCOFF.cxx

//==================================
// PEDUMP - Matt Pietrek 1997
// FILE: coffsymboltable.cpp
//==================================
#ifdef DUMP4
#include "Dump4.h"
#else // !DUMP4
#include "pedump.h"
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#endif // DUMP4 y/n
#ifdef DUMP4
#include "DumpCOFF.h"
#else
#include "COFFSymbolTable.h"
#endif

COFFSymbol::COFFSymbol(
	PIMAGE_SYMBOL pSymbolData,
	PSTR pStringTable,
	DWORD index )
{
	m_pSymbolData = pSymbolData;
	m_pStringTable = pStringTable;
	m_index = index;
	m_pszShortString = 0;
}

COFFSymbol::~COFFSymbol( )
{
	CleanUp();
}

void
COFFSymbol::CleanUp( void )
{
	if ( m_pszShortString )
		free( m_pszShortString );

	m_pszShortString = 0;
}

PSTR pBadName = "?UNK?";

PSTR
COFFSymbol::GetName()
{
	if ( !m_pSymbolData )
		return pBadName;
    if (out_of_top_range((unsigned char *)m_pSymbolData))
        return pBadName;

	if ( 0 != m_pSymbolData->N.Name.Short )
	{
		m_pszShortString = (PSTR)malloc( 9 );
		memcpy( m_pszShortString, m_pSymbolData->N.ShortName, 8 );
		m_pszShortString[8] = 0;

		return m_pszShortString;
	}
	
	return m_pStringTable + m_pSymbolData->N.Name.Long;
}

PSTR
COFFSymbol::GetTypeName()
{
	if ( m_pSymbolData ) 
    	sprintf( m_szTypeName, "%04X", m_pSymbolData->Type );
    else
    	sprintf( m_szTypeName, "%04X", 0xffff );

	return m_szTypeName;
}

SHORT
COFFSymbol::GetSectionNumber(void)
{
#ifdef WIN32
   SHORT num = 0;
   if (m_pSymbolData) {

      __try
      {
         num = m_pSymbolData->SectionNumber;
      }
      __except(TRUE)
      {
         m_pSymbolData = NULL;
      }
   }

   return num;
#else
    return m_pSymbolData ? m_pSymbolData->SectionNumber : 0;
#endif
}


//
// Dumps the auxillary symbol for a regular symbol.  It takes a pointer
// to the regular symbol, since the the information in that record defines
// how the "auxillary" record that follows it is interpreted
//
BOOL
COFFSymbol::GetAuxSymbolAsString( PSTR pszBuffer, unsigned cbBuffer )
{
	if ( !m_pSymbolData || (0==m_pSymbolData->NumberOfAuxSymbols) )
		return FALSE;

	//
	// Auxillary symbol immediately follows the main symbol.  Note the pointer
	// arithmetic going on here.	
	//
    PIMAGE_AUX_SYMBOL auxSym = (PIMAGE_AUX_SYMBOL)(m_pSymbolData+1);
    
    if ( m_pSymbolData->StorageClass == IMAGE_SYM_CLASS_FILE )
	{
        lstrcpyn( pszBuffer, (char *)auxSym, cbBuffer );
	}
    else if ( (m_pSymbolData->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) )
    {
        if ( (m_pSymbolData->Type & 0xF0) == (IMAGE_SYM_DTYPE_FUNCTION << 4))
        {   
        	wsprintf( pszBuffer,
				"tag: %04X  size: %04X  Line #'s: %08X  next fn: %04X",
            	auxSym->Sym.TagIndex, auxSym->Sym.Misc.TotalSize,
	            auxSym->Sym.FcnAry.Function.PointerToLinenumber,
    	        auxSym->Sym.FcnAry.Function.PointerToNextFunction);
        }
    }
    else if ( (m_pSymbolData->StorageClass == IMAGE_SYM_CLASS_STATIC) )
    {
        wsprintf( pszBuffer,
            "Section: %04X  Len: %05X  Relocs: %04X  LineNums: %04X",
            auxSym->Section.Number, auxSym->Section.Length,
            auxSym->Section.NumberOfRelocations,
            auxSym->Section.NumberOfLinenumbers );
    }
	else
	{
		lstrcpyn( pszBuffer, "<unhandled aux symbol>", cbBuffer );
	}
	
	return TRUE;
}


//
// MERGE THESE ARRAYS INTO THE COFFSymbol class!!!
//

// The names of the first group of possible symbol table storage classes
char * SzStorageClass1[] = {
"NULL","AUTOMATIC","EXTERNAL","STATIC","REGISTER","EXTERNAL_DEF","LABEL",
"UNDEFINED_LABEL","MEMBER_OF_STRUCT","ARGUMENT","STRUCT_TAG",
"MEMBER_OF_UNION","UNION_TAG","TYPE_DEFINITION","UNDEFINED_STATIC",
"ENUM_TAG","MEMBER_OF_ENUM","REGISTER_PARAM","BIT_FIELD"
};

// The names of the second group of possible symbol table storage classes
char * SzStorageClass2[] = {
"BLOCK","FUNCTION","END_OF_STRUCT","FILE","SECTION","WEAK_EXTERNAL"
};


PSTR
COFFSymbol::GetStorageClassName()
{
	if ( !m_pSymbolData )
        return "???";

	BYTE storageClass = m_pSymbolData->StorageClass;

    if ( storageClass <= IMAGE_SYM_CLASS_BIT_FIELD )
        return SzStorageClass1[storageClass];

    if ( (storageClass >= IMAGE_SYM_CLASS_BLOCK) &&
         (storageClass <= IMAGE_SYM_CLASS_WEAK_EXTERNAL) )
        return SzStorageClass2[storageClass-IMAGE_SYM_CLASS_BLOCK];

    return "???";
}

//============================================================================
//
// COFFSymbolTable class
//
//============================================================================


COFFSymbolTable::COFFSymbolTable( PVOID pSymbolBase, unsigned cSymbols )
{
	m_pSymbolBase = (PIMAGE_SYMBOL)pSymbolBase;
	m_cSymbols = cSymbols;

	// StringTable starts right after the array of IMAGE_SYMBOL's
	m_pStringTable = (PSTR)( m_pSymbolBase + m_cSymbols );	// PTR MATH!!!
}

COFFSymbolTable::~COFFSymbolTable( )
{

}

PCOFFSymbol
COFFSymbolTable::GetNextSymbol( PCOFFSymbol pSymbol )
{
	// 0 begins iteration
   BYTE cAuxSymbols;
    if (out_of_top_range((unsigned char *)m_pSymbolBase) ||
        out_of_top_range((unsigned char *)m_pStringTable)) {
            return 0;
    }
	if ( 0 == pSymbol )
	{
		pSymbol = new COFFSymbol(	(PIMAGE_SYMBOL)m_pSymbolBase,
									m_pStringTable,
									0 );
		return pSymbol;
	}

   if ( pSymbol->m_pSymbolData )
   {
      cAuxSymbols = pSymbol->m_pSymbolData->NumberOfAuxSymbols;
   }
   else
   {
		delete pSymbol;
		return 0;
   }

	// Are we past the end of the "array" of symbols???
	if ( (pSymbol->m_index + cAuxSymbols + 1) >= m_cSymbols )
	{
		delete pSymbol;
		return 0;
	}

	// Just tweak the values in the COFFSymbol and return it;
	pSymbol->CleanUp();
	pSymbol->m_pSymbolData += ( 1 + cAuxSymbols );
	pSymbol->m_index += (1 + cAuxSymbols );

	return pSymbol;
}

PCOFFSymbol
COFFSymbolTable::GetNearestSymbolFromRVA( DWORD rva, BOOL fExact )
{
	if ( !fExact )
		return 0;

	PCOFFSymbol pSymbol = GetNextSymbol( 0 );

	while ( pSymbol )
	{
		if ( rva == pSymbol->GetValue() )
		{
			if (pSymbol->GetStorageClass() == IMAGE_SYM_CLASS_EXTERNAL )
				return pSymbol;

			if ( pSymbol->GetStorageClass() == IMAGE_SYM_CLASS_STATIC )
			{
				if ( pSymbol->GetType() )	// Eliminate "Type: 0"
					return pSymbol;
			}
		}

		pSymbol = GetNextSymbol( pSymbol );
	}

	return 0;
}

PCOFFSymbol
COFFSymbolTable::GetSymbolFromIndex( DWORD index )
{
	PCOFFSymbol pSymbol;

	if ( index >= m_cSymbols )
		return 0;

	pSymbol = new COFFSymbol(	m_pSymbolBase + index,	// PTR MATH!!!
								m_pStringTable,
								index );

	return pSymbol;
}

// eof - coffsymboltable.cpp - DumpCOFF.cxx
