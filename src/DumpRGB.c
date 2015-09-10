
// DumpRGB.c
#include  "Dump4.h"

//#ifdef   ADD_SGI_RGB_READ
//	VER_BUILD29	// fixes to readrgb - FIX20030724
//#define  ft_RGB      8  // RGB (SGI texture files)
// readrgb.c
//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h> 
//#include <string.h>
//#include "readrgb.hxx"

// WIDTHBYTES takes # of bits in a scan line and rounds up to nearest
//  dword (32-bits). The # of bits in a scan line would typically be
//  the pixel width (bmWidth) times the BPP (bits-per-pixel = bmBitsPixel)
//#define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)
//--------------  DIB header Marker Define -------------------------
//#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')	/* Simple "BM" ... */


typedef struct tagRGBHDR {
   unsigned short rgb_imagic;
   unsigned short rgb_type;
   unsigned short rgb_dim;
   unsigned short rgb_xsize, rgb_ysize, rgb_zsize;
}RGBHDR, * PRGBHDR;

typedef struct _ImageRec {
   unsigned short imagic;
   unsigned short type;
   unsigned short dim;
   unsigned short xsize, ysize, zsize;
   unsigned int min, max;
   unsigned int wasteBytes;
   char name[80];
   unsigned long colorMap;
// *not used here*   FILE *file;
   LPDFSTR ir_pdf;
   unsigned char *tmp, *tmpR, *tmpG, *tmpB;
   unsigned long rleEnd;
   unsigned int *rowStart;
   int *rowSize;
} ImageRec;


RGBHDR   sRGBHdr;
//#ifndef  dv_fmemset
//#define  dv_fmemset(a,b,c) ZeroMemory(a,c)
//#endif   // #ifndef  dv_fmemset

//extern void InitBitmapInfoHeader( LPBITMAPINFOHEADER lpBmInfoHdr,
//						  DWORD dwWidth,
//						  DWORD dwHeight,
//						  int nBPP );

void bwtorgba(unsigned char *b,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = *b;
      l[1] = *b;
      l[2] = *b;
      l[3] = 0xff;
      l += 4; b++;
   }
}

void latorgba(unsigned char *b, unsigned char *a,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = *b;
      l[1] = *b;
      l[2] = *b;
      l[3] = *a;
      l += 4; b++; a++;
   }
}

void rgbtorgba(unsigned char *r,unsigned char *g,
   unsigned char *b,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = r[0];
      l[1] = g[0];
      l[2] = b[0];
      l[3] = 0xff;
      l += 4; r++; g++; b++;
   }
}

void rgbatorgba(unsigned char *r,unsigned char *g,
   unsigned char *b,unsigned char *a,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = r[0];
      l[1] = g[0];
      l[2] = b[0];
      l[3] = a[0];
      l += 4; r++; g++; b++; a++;
   }
}

static void ConvertShort(unsigned short *array, long length) 
{
   unsigned b1, b2;
   unsigned char *ptr;

   ptr = (unsigned char *)array;
   while (length--) {
      b1 = *ptr++;
      b2 = *ptr++;
      *array++ = (b1 << 8) | (b2);
   }
}

static void ConvertLong(unsigned *array, long length) 
{
   unsigned b1, b2, b3, b4;
   unsigned char *ptr;

   ptr = (unsigned char *)array;
   while (length--) {
      b1 = *ptr++;
      b2 = *ptr++;
      b3 = *ptr++;
      b4 = *ptr++;
      *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
   }
}

static ImageRec _s_sImageRec;
//   if( fread(image, 1, 12, image) != 12 )
size_t m_fread( void *buffer, size_t size, size_t count, LPDFSTR lpdf )
{
   PBYTE pb = (PBYTE) lpdf->df_pVoid;
   DWORD cnt = size * count;
   if(pb) {
      if( (lpdf->df_dwOff + cnt) > lpdf->dwmax )
         return 0;

      pb += lpdf->df_dwOff;

      memcpy(buffer,pb,cnt);

      lpdf->df_dwOff += cnt;

      return cnt;
   }

   return 0;

}

//int fseek( FILE *stream, long offset, int origin );
int m_fseek( LPDFSTR lpdf, long offset, int origin )
{
   if((lpdf->df_pVoid) &&
      (origin == SEEK_SET) )
   {
      if( (DWORD)offset <= lpdf->dwmax ) {
         // ok, move to this location
         lpdf->df_dwOff = offset;

         return 0;

      }
   }

   return -1;
}



static ImageRec *ImageOpen(LPDFSTR lpdf)  // const char *fileName)
{
   union {
      int testWord;
      char testByte[4];
   } endianTest;
   ImageRec *image;
   int swapFlag;
   int x;
   PRGBHDR  phdr = &sRGBHdr;

   endianTest.testWord = 1;
   if (endianTest.testByte[0] == 1) {
      swapFlag = 1;
   } else {
      swapFlag = 0;
   }

   //image = (ImageRec *)malloc(sizeof(ImageRec));
   image = &_s_sImageRec;
   ZeroMemory(image,sizeof(ImageRec));
//   if (image == NULL) {
//      fprintf(stderr, "Out of memory!\n");
//      pgm_exit(1);
//   }
//   if ((image->file = fopen(fileName, "rb")) == NULL) {
//      perror(fileName);
//      pgm_exit(1);
//   }

//   if( fread(image, 1, 12, image->file) != 12 )
   if( m_fread(image, 1, 12, lpdf) != 12 )
   {
      sprtf("NOT an SGI RGB file!"MEOR);
      return 0;   // out of here ...
   }

   if (swapFlag) {
      memcpy( phdr, image, sizeof(RGBHDR) );
      sprtf( "PreHdr: magic %d type=%d dim=%d x=%d y=%d z=%d"MEOR,
         phdr->rgb_imagic,
         phdr->rgb_type,
         phdr->rgb_dim,
         phdr->rgb_xsize,
         phdr->rgb_ysize,
         phdr->rgb_zsize );
      ConvertShort(&image->imagic, 6);
   }

   memcpy( phdr, image, sizeof(RGBHDR) );

#ifdef   CONSOLE
   cout << "Header: magic " << phdr->rgb_imagic
      << " type " << phdr->rgb_type
      << " dim " << phdr->rgb_dim
      << " x " << phdr->rgb_xsize
      << " y " << phdr->rgb_ysize
      << " z " << phdr->rgb_zsize
      << endl;
#else // #ifdef   CONSOLE
   sprtf( "Header: magic %d type=%d dim=%d x=%d y=%d z=%d"MEOR,
      phdr->rgb_imagic,
      phdr->rgb_type,
      phdr->rgb_dim,
      phdr->rgb_xsize,
      phdr->rgb_ysize,
      phdr->rgb_zsize );
#endif      // #ifdef   CONSOLE

   image->tmp = (unsigned char *)malloc(image->xsize*256);
   image->tmpR = (unsigned char *)malloc(image->xsize*256);
   image->tmpG = (unsigned char *)malloc(image->xsize*256);
   image->tmpB = (unsigned char *)malloc(image->xsize*256);
   if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
      image->tmpB == NULL) {
      fprintf(stderr, "Out of memory!\n");
      pgm_exit(1);
   }

   if ((image->type & 0xFF00) == 0x0100) {
      x = image->ysize * image->zsize * sizeof(unsigned);
      sprtf("RGB type 1: 2 arrays of row Start and Size, each %d bytes."MEOR,
         x );
      image->rowStart = (unsigned *)malloc(x);
      image->rowSize = (int *)malloc(x);
      if (image->rowStart == NULL || image->rowSize == NULL) {
         fprintf(stderr, "Out of memory!\n");
         pgm_exit(1);
      }

      image->rleEnd = 512 + (2 * x);
      //fseek(image->file, 512, SEEK_SET);
      //fread(image->rowStart, 1, x, image->file);
      //fread(image->rowSize, 1, x, image->file);
      if( m_fseek(lpdf, 512, SEEK_SET) ) {
         // failed
         sprtf("NOT an SGI RGB file! Filed first seek 512 bytes"MEOR);
         return 0;
      }
      if( m_fread(image->rowStart, 1, x, lpdf) != (size_t)x ) {
         // failed
         sprtf("NOT an SGI RGB file! Filed row offsets read failed!!"MEOR);
         return 0;
      }
      if( m_fread(image->rowSize, 1, x, lpdf) != (size_t)x ) {
         // failed
         sprtf("NOT an SGI RGB file! Filed row sizes read failed!!"MEOR);
         return 0;
      }

      if (swapFlag) {
          ConvertLong(image->rowStart, x/(int)sizeof(unsigned));
          ConvertLong((unsigned *)image->rowSize, x/(int)sizeof(int));
      }
   } else {
      image->rowStart = NULL;
      image->rowSize = NULL;
   }
   return image;
}

static void ImageClose(ImageRec *image) 
{
//   fclose(image->file);
   if(image->tmp)
      free(image->tmp);

   if(image->tmpR)
      free(image->tmpR);

   if(image->tmpG)
      free(image->tmpG);

   if(image->tmpB)
      free(image->tmpB);

   if(image->rowSize)
      free(image->rowSize);

   if(image->rowStart)
      free(image->rowStart);

   //free(image);
   image->tmp = 0;
   image->tmpR = 0;
   image->tmpG = 0;
   image->tmpB = 0;
   image->rowSize = 0;
   image->rowStart = 0;
}

LPDFSTR g_actpdfptr = 0;

static int ImageGetRow(ImageRec *image, 
   unsigned char *buf, int y, int z) 
{
   unsigned char *iPtr, *oPtr, pixel;
   unsigned int count;
   int offset;
   unsigned long size, total;

   total = 0;
   if ((image->type & 0xFF00) == 0x0100)
   {
      offset = image->rowStart[y+z*image->ysize];
      size   = image->rowSize[y+z*image->ysize];

      //fseek(image->file, (long)image->rowStart[y+z*image->ysize], SEEK_SET);
      //fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],image->file);
      //if( fseek(image->file, offset, SEEK_SET) != offset )
      //   return 0;
      //fseek(image->file, offset, SEEK_SET);
      if( m_fseek(g_actpdfptr, offset, SEEK_SET) ) {
         sprtf("Warning: seek to offset %d FAILED!"MEOR, offset);
         return 0;
      }

      //if( fread(image->tmp, 1, size, image->file) != size )
      if( m_fread(image->tmp, 1, size, g_actpdfptr) != size ) {
         sprtf("Warning: read of %d bytes FAILED!"MEOR, size);
         return 0;
      }

      iPtr = image->tmp;
      oPtr = buf;
      for (;;) // forever
      {
          pixel = *iPtr++; // get next byte
          count = (int)(pixel & 0x7F); // and count
          if (!count)
            return total;  // all done on this row

          total += count;
          if (pixel & 0x80)
          {
            while (count--)
            {
                *oPtr++ = *iPtr++;
            }
          }
          else
          {
            pixel = *iPtr++;
            while (count--)
            {
                *oPtr++ = pixel;
            }
         }
      }
   }
   else
   {
      offset = 512+(y*image->xsize)+(z*image->xsize*image->ysize);
      size   = image->xsize;

      //fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),SEEK_SET);
      //fread(buf, 1, image->xsize, image->file);
      //if( fseek(image->file, offset, SEEK_SET) != offset )
      m_fseek(g_actpdfptr, offset, SEEK_SET);

      if( m_fread(buf, 1, size, g_actpdfptr) != size )
         return 0;
   }

   return size;

}

//unsigned *read_texture(char *name, 
unsigned * read_rgb2bmp24(LPDFSTR lpdf,   // char * name,
   int *width, int *height, int *components )
{
   unsigned *base, *lptr;
   unsigned char *rbuf, *gbuf, *bbuf, *abuf;
   ImageRec *image;
   int y;

   //image = ImageOpen(name);
   image = ImageOpen(lpdf);
    
   if(!image) {
      // failed
      ImageClose(&_s_sImageRec);
      return NULL;
   }

   (*width)=image->xsize;
   (*height)=image->ysize;
   (*components)=image->zsize;
   base = (unsigned *)malloc(image->xsize*image->ysize*sizeof(unsigned));
   rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   if(!base || !rbuf || !gbuf || !bbuf) {
      // no memory
      ImageClose(&_s_sImageRec);
      return NULL;
   }

   sprtf("Processing RGBa as %d colours for %d rows..."MEOR,
      image->zsize,
      image->ysize );
   lptr = base;
//   for (y=0; y<image->ysize; y++)
//   {
      if (image->zsize>=4)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,gbuf,y,1) &&
            ImageGetRow(image,bbuf,y,2) &&
            ImageGetRow(image,abuf,y,3) )
         {
            rgbatorgba(rbuf,gbuf,bbuf,abuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else if(image->zsize==3)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,gbuf,y,1) &&
            ImageGetRow(image,bbuf,y,2) )
         {
            rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else if(image->zsize==2)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,abuf,y,1) )
         {
            latorgba(rbuf,abuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0))
         {
            bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
//   }

   sprtf("Returning RGBA 4-colours base array, %d rows, %d cols, %d bytes..."MEOR,
      image->ysize,
      image->xsize,
      (image->xsize * image->ysize * sizeof(unsigned)) );

Exit_Read:

   ImageClose(image);
   free(rbuf);
   free(gbuf);
   free(bbuf);
   free(abuf);

   return (unsigned *) base;
}

// input (and output file)
//char *  _readrgb2bmp24( char * pfile, char * pbmp )
char * _readrgb2bmp24( LPDFSTR lpdf, char * pbmp )
{
//   static char szbmpfile[264];
   int width, height, components, rows, cols;
   unsigned * base;
   int   size;
//   char * pfile = "D:/FG091/Scenery/ufo.rgb";
   int  palsz = 0;  // PaletteSize((LPSTR)lpbi);
   int  dibsz; // = sizeof(BITMAPINFOHEADER)   +
               // (sizeof(RGBQUAD) * palsz ) +
               //     bufsize;
   char * pdib;

#ifdef   CONSOLE
   cout << "Reading file [" << lpdf->fn << "]..." << endl;
#else // !#ifdef   CONSOLE
   sprtf( "Reading file [%s]..."MEOR, lpdf->fn );
#endif   // #ifdef   CONSOLE

   //image = read_texture( pfile,  // char *name, 
   base = read_rgb2bmp24( lpdf,  // pfile, // char *name
      &width, &height, &components );

   if(!base) {
      sprtf( "Read FAILED in read_rgb2bmp24() service!"MEOR );
      return 0;
   }

   // base arranged in rows y, y=0 to ysize, in array
   // l[0] = r[0]; l[1] = g[0]; l[2] = b[0]; l[3] = a[0];
   cols = WIDTHBYTES(width * 24);   // = (((bits) + 31) / 32 * 4)
   rows = height;
   dibsz = sizeof(BITMAPINFOHEADER)   +
           (sizeof(RGBQUAD) * palsz ) +
           (cols * rows);  // =  bufsize;
   size  = sizeof(BITMAPFILEHEADER)   + dibsz;

   pdib  = (char *)malloc(size);
   if(!pdib)
   {
#ifdef   CONSOLE
      cout << "ERROR: Memory FAILED!" << endl;
#endif   // #ifdef   CONSOLE
      pgm_exit(-2);
      return 0;
   }
   else
   {
      BITMAPFILEHEADER * phdr = (BITMAPFILEHEADER *)pdib;
      LPBITMAPINFOHEADER pbi  = (LPBITMAPINFOHEADER)((BITMAPFILEHEADER *) phdr + 1);
      char * pbits            = (char *)((LPBITMAPINFOHEADER) pbi + 1);
//      char * pbmp             = szbmpfile;
//      FILE * fp;
      int   y, x, row, col, alpha;
      char * pout;
      unsigned char * pin = (unsigned char *)base;

      sprtf("Converting base colour array into 24-bit bitmap..."MEOR );

      for( y = 0; y < height; y++ ) // for each row
      {
         row = y;
         pout = &pbits[(row * cols)];
         for( x = 0; x < width; x++ )
         {
            col = x * 3;
            pout[col+2] = (char) pin[0];
            pout[col+1] = (char) pin[1];
            pout[col+0] = (char) pin[2];
            alpha       =        pin[3];
            pin += 4;
         }
      }

      InitBitmapInfoHeader( pbi,    // LPBITMAPINFOHEADER lpBmInfoHdr,
                           width,
                           height,
                           24 ); // 	  int nBPP )

      phdr->bfType  = DIB_HEADER_MARKER;   // simple "BM" signature
      phdr->bfSize  = size;   // file size
      phdr->bfReserved1 = 0;
      phdr->bfReserved2 = 0;
      phdr->bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER) + pbi->biSize + palsz;

      // strcpy(pbmp,pfile);
      // strcat(pbmp,".bmp");
      if(pbmp && *pbmp) {
         static char _s_szfnbmp[264];
         PSTR  pfn = _s_szfnbmp;
         FILE * fpbm;

         _fullpath( pfn, pbmp, 256 );
         // WRITE a bitmap file
         fpbm = fopen(pfn, "wb");
         if(fpbm)
         {
            fwrite(pdib, 1, size, fpbm);
            fclose(fpbm);
            sprtf("Written [%s] file %d bytes."MEOR, pfn, size );
         }
         else
         {
   #ifdef   CONSOLE
            cout << "ERROR: Failed to create " << pfn << "!" << endl;
   #else // !#ifdef   CONSOLE
            sprtf( "ERROR: Failed to create [%s]!"MEOR, pfn );
   #endif   // #ifdef   CONSOLE
         }
      }
   }

   if(base)
      free(base);

   return pdib;

}

#define  bark  chkme("Check this ERROR!"MEOR)

char  g_szRgbBmp[264] = { "temprgb6.bmp" };

BOOL  ProcessRGB( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   char * lpf = lpdf->fn;
   //char * pdib = _readrgb2bmp24( lpf, "temprgb2.bmp" );
   char * pdib;   // = _readrgb2bmp24( lpdf, "temprgb2.bmp" );

   g_actpdfptr = lpdf;
//   pdib = _readrgb2bmp24( lpdf, "temprgb2.bmp" );
//   pdib = _readrgb2bmp24( lpdf, "temprgb3.bmp" );
//   pdib = _readrgb2bmp24( lpdf, "temprgb4.bmp" );
   pdib = _readrgb2bmp24( lpdf, g_szRgbBmp );
   if(pdib) {
      sprtf( "Sucessful decode of an SGI RGB file..."MEOR );
      free(pdib);
      bRet = TRUE;
   }


   return bRet;
}


// eof - DumpRGB.c
