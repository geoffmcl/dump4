// DumpSHP.c
// Dump a shapefile ...

#include "dump4.h"
#ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib
#include <shapefil.h>
#ifndef NDEBUG
#pragma comment( lib, "C:\\FG\\TG\\src\\Lib\\shapelib\\Debug MT\\shapelibMT.lib" )
#else
#pragma comment( lib, "C:\\FG\\TG\\src\\Lib\\shapelib\\Release MT\\shapelibMT.lib" )
#endif

int SHP_main( int argc, char ** argv )
{
   PTSTR ptmp = &gszDiag[0];
   SHPHandle	hSHP;
   int		nShapeType, nEntities, i, iPart;
   const char 	*pszPlus;
   double 	adfMinBound[4], adfMaxBound[4];

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
   if( argc != 2 )
   {
	   sprintf(ptmp, "shpdump shp_file\n" );
      prt(ptmp);
      return 0;
   }

/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
   hSHP = SHPOpen( argv[1], "rb" );

   if( hSHP == NULL )
   {
      sprintf(ptmp, "Unable to open:%s\n", argv[1] );
      prt(ptmp);
      return 0;
   }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
   SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

   sprintf(ptmp, "Shapefile Type: %s   # of Shapes: %d\n\n",
            SHPTypeName( nShapeType ), nEntities );
   prt(ptmp);

   sprintf(ptmp, "File Bounds: (%3.5f,%3.5f,%lg,%lg)\n"
            "         to  (%3.5f,%3.5f,%lg,%lg)\n",
            adfMinBound[0], 
            adfMinBound[1], 
            adfMinBound[2], 
            adfMinBound[3], 
            adfMaxBound[0], 
            adfMaxBound[1], 
            adfMaxBound[2], 
            adfMaxBound[3] );
   prt(ptmp);

/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, printing all the vertices.	*/
/* -------------------------------------------------------------------- */
   for( i = 0; i < nEntities; i++ )
   {
	   int		j;
      SHPObject	*psShape;

      psShape = SHPReadObject( hSHP, i );

	   sprintf(ptmp, "\nShape:%d (%s)  nVertices=%d, nParts=%d\n"
                "  Bounds:(%3.5f,%3.5f, %lg, %lg)\n"
                "      to (%3.5f,%3.5f, %lg, %lg)\n",
	        i, SHPTypeName(psShape->nSHPType),
                psShape->nVertices, psShape->nParts,
                psShape->dfXMin, psShape->dfYMin,
                psShape->dfZMin, psShape->dfMMin,
                psShape->dfXMax, psShape->dfYMax,
                psShape->dfZMax, psShape->dfMMax );
      prt(ptmp);

	   for( j = 0, iPart = 1; j < psShape->nVertices; j++ )
	   {
         const char	*pszPartType = "";

         if( j == 0 && psShape->nParts > 0 )
            pszPartType = SHPPartTypeName( psShape->panPartType[0] );
            
         if( iPart < psShape->nParts
                && psShape->panPartStart[iPart] == j )
	      {
            pszPartType = SHPPartTypeName( psShape->panPartType[iPart] );
		      iPart++;
		      pszPlus = "+";
	      }
	      else
            pszPlus = " ";

         sprintf(ptmp, "   %s (%3.5f,%3.5f, %lg, %lg) %s \n",
                   pszPlus,
                   psShape->padfX[j],
                   psShape->padfY[j],
                   psShape->padfZ[j],
                   psShape->padfM[j],
                   pszPartType );
         prt(ptmp);
	   }
        
      SHPDestroyObject( psShape ); 
   }
   SHPClose( hSHP );
   return 1;
}

int ProcessSHP( PTSTR pf )
{
   int   iret = 0;
   char * argv[3];

   argv[0] = "dummy";
   argv[1] = pf;
   argv[2] = 0;
   iret = SHP_main( 2, &argv[0] );

   return iret;
}

#endif // #ifdef  ADD_SHAPE_FILE    // needs TG/lib/shapelib - shapelib.lib

// eof - DumpSHP.c
