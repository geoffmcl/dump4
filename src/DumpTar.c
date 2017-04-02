// DumpTar.c

#include	"Dump4.h"   // 20170330 - Fix cse for unix
#include <sys/types.h>
#include <sys/stat.h>

#include "DumpTar2.h"

extern union block *current_block;
extern union block *record_end;
extern BOOL  DumpTAR( LPDFSTR lpdf );
extern void xheader_destroy (struct xheader *xhdr);

int warn_count = 0;
int max_warn_count = 32;

/* tar Header Block, overall structure.  */
struct tar_stat_info current_stat_info = { 0 };
extern enum archive_format current_format;
extern void decode_header (union block *header, struct tar_stat_info *stat_info,
	       enum archive_format *format_pointer, int do_user_group);
extern enum read_header tar_checksum (union block *header, bool silent);
extern union block *current_header;
extern char const * tartime (struct timespec t, bool full_time);

// eof - tar.h ...
// forward references
void tar_stat_destroy (struct tar_stat_info *st);
enum archive_format form;
char * get_header_stg( enum read_header hdr );

// some COUNTS
int REGTYPE_count = 0;
// case AREGTYPE: counted with above
int LNKTYPE_count = 0;  // , "link" },
int SYMTYPE_count = 0;  //, "reserved" },
int CHRTYPE_count = 0;  // , "character special" },
int BLKTYPE_count = 0;  //, "block special" }, //  '4'		/* block special */
int DIRTYPE_count = 0;  // , "directory" },     //  '5'		/* directory */
int FIFOTYPE_count = 0; //, "FIFO special" }, // '6'		/* FIFO special */
int CONTTYPE_count = 0; //, "reserved" },     // '7'		/* reserved */
int XHDTYPE_count = 0;  //, "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
int XGLTYPE_count = 0;  //, "global header" }, //  'g'            /* Global extended header */
int default_count = 0;

// some of tar.c
//static
struct fmttab {
  char const *name;
  enum archive_format fmt;
};

struct fmttab ftable[] = {
//const fmttab[] = {
  { "v7",      V7_FORMAT },
  { "oldgnu",  OLDGNU_FORMAT },
  { "ustar",   USTAR_FORMAT },
  { "posix",   POSIX_FORMAT },
//#if 0 /* not fully supported yet */
  { "star",    STAR_FORMAT },
//#endif
  { "gnu",     GNU_FORMAT },
  { "pax",     POSIX_FORMAT }, /* An alias for posix */
  { NULL,      0 }
};

char const * get_format_stg(enum archive_format current)
{
   struct fmttab * pf = &ftable[0];
   while( pf->name )
   {
      if( pf->fmt == current )
         return pf->name;
      pf++;
   }
   return "unknown";
}

// eof - tar.c

typedef struct tagTYPEFORMAT {
   char type;
   char * name;
   char * full_name;
} TYPEFORMAT, * PTYPEFORMAT;

/* Values used in typeflag field.  */
TYPEFORMAT typeform[] = {
   { REGTYPE,  "file ", "regular file" },
   { AREGTYPE, "file ", "regular file" },
   { LNKTYPE,  "link ", "link" },
   { SYMTYPE,  "resed", "reserved" },
   { CHRTYPE,  "chspl", "character special" },
   { BLKTYPE,  "block", "block special" }, //  '4'		/* block special */
   { DIRTYPE,  "<DIR>", "directory" },     //  '5'		/* directory */
   { FIFOTYPE, "FIFO ", "FIFO special" }, // '6'		/* FIFO special */
   { CONTTYPE, "resed", "reserved" },     // '7'		/* reserved */
   { XHDTYPE,  "EXTHD", "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
   { XGLTYPE,  "GLOBH", "global header" }, //  'g'            /* Global extended header */
   { 0, 0, 0 }
};

char * get_type_string( char type )
{
   PTYPEFORMAT ptf = &typeform[0];
   while( ptf->name )
   {
      if( ptf->type == type )
         return ptf->name;

      ptf++;
   }
   return "undefined";
}

#define ISOCTALCHR(a)   (( a >= '0' )&&( a <= '7' ))

BOOL bExtraDebug = FALSE;
size_t iMaxNameLen;
size_t iMaxFileSize;
size_t iMinFileSize;
size_t iFileCount;
size_t iDirCount;

enum read_header rdhdr;
struct posix_header * pheader = NULL;
struct star_header * pstar_header = NULL;
struct oldgnu_header * poldgnu_header = NULL;
struct sparse_header * psparse_header = NULL;
struct star_in_header * pstar_in_header = NULL;
struct star_ext_header * pstar_ext_header = NULL;


size_t collect_prefix_name( char * lpb, struct posix_header * pph )
{
   size_t off;
   size_t max;
   char * cp;
   char c;
   // collect any prefix first
   cp = pph->prefix;
   max = sizeof(pph->prefix); // = 155 (0x9b)
   off = 0;
   while(*cp && max) {
      c = *cp;
      lpb[off++] = c;
      cp++;
      max--;
   }
   // add directory separator if not last char
   if(off) {
      if( c != '/' )
         lpb[off++] = '/';
   }
   // now collect name
   cp = pph->name;
   max = sizeof(pph->name);   // = 100 (0x64)
   while(*cp && max) {
      lpb[off++] = *cp;
      cp++;
      max--;
   }
   lpb[off] = 0;  // zero terminate string
   return off;    // return length
}

int collect_octal_size( char * lpb, struct posix_header * pph, size_t * pfsz, int warn )
{
   size_t off = 0;
   char * cp = pph->size;   // char size[12];		/* 124 */
   size_t max = sizeof( pph->size );
   size_t i;
   char c;

   // fsz = atoi(cp); = NO, this is in OCTAL ASCII
   for( i = 0; i < max; i++ )
   {
      c = cp[i];
      if( c == 0 )
         break;
      if(( c == ' ' )&&(off == 0))
         continue;
      if( !ISOCTALCHR(c) ) {
         if(warn)
         {
            if( warn_count < max_warn_count ) {
               sprtf( "WARNING: [%c] OUT OF OCTAL RANGE!!!"MEOR, c );
            }
            warn_count++;
         }
      }
      lpb[off++] = c;
   }
   lpb[off] = 0;
   i = 1;
   *pfsz = 0;
   if (off)
   {
#ifdef WIN32
       i = sscanf_s(lpb, "%o", (unsigned int *)pfsz);
#else
       i = sscanf(lpb, "%o", (unsigned int *)pfsz);
#endif
   }
   if( i != 1 )
   {
      if( warn )
         sprtf( "WARNING: Bad sscanf_s return value %d!"MEOR, i );
   }
   return i;
}

extern void xheader_read (struct xheader *xhdr, union block *p, size_t size);

BOOL  DumpTAR_FAILED( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   LPTSTR   lpb = &gcOutBuf[0];  // [16K]
   LPTSTR   lpb2 = &gcOutBuf2[0];  // [16K]
   char * pv = (char *)lpdf->df_pVoid;
   struct posix_header * pph = (struct posix_header *)lpdf->df_pVoid;
   char * cp;
   size_t   off, boff, count, fsz, remains, len, hlen, culen;
   int   i;
   char c;
   size_t maxcnt = iFileCount + iDirCount;
   size_t entry = 0;
   enum read_header h;
   struct tar_stat_info * info = &current_stat_info;
   DWORD dwm = lpdf->dwmax;
   union block * p;
   if( !pph )
      return bRet;

   iMaxFileSize = 0;
   iMinFileSize = 0x7fffffff;
   current_block = (union block *)pv;
   dwm = lpdf->dwmax;
   p = current_block;
   while(dwm)
   {
      record_end = p;
      p++;
      if(dwm >= BLOCKSIZE)
      {
         dwm -= BLOCKSIZE;
         if(dwm == 0)
            record_end = p;
      }
      else
         dwm = 0;
   }

   if(( STRNICMP(pph->magic, TMAGIC, 5) == 0 )||
      ( rdhdr == HEADER_SUCCESS ) ){
      // we have a USTAR format
      off = 1;
      count = 0;
      sprtf( "TAR [%s] is [%s] format ..."MEOR,
         lpdf->fn,
         get_format_stg(current_format) );
      while(off)
      {
         entry++;
         boff = count * BLOCKSIZE;
         pph = (struct posix_header *) &pv[boff];
         if( (boff + BLOCKSIZE) > lpdf->dwmax )
         {
            sprtf( "WARNING: DATA EXPIRED BEFORE ZERO BLOCK!"MEOR );
            break;
         }
         tar_stat_destroy(info);
         current_header = (union block *)pph;
         h = tar_checksum ((union block *)pph, TRUE);
         form = -1;
         decode_header ((union block *)pph, info, &form, 0);
         //      if (header->header.typeflag == LNKTYPE)
         if( pph->typeflag == LNKTYPE )
            info->stat.st_size = 0;	/* links 0 size on tape */
         else
            info->stat.st_size = OFF_FROM_HEADER (pph->size);
         fsz = (size_t)info->stat.st_size;
         if( VERB5 )
         {
            cp = (char *)pph;
            sprtf( "%s: form [%s](%d): type [%s]: size %9d; offset %#08X; time %s"MEOR,
               get_header_stg( h ),
               get_format_stg(form),
               form,
               get_type_string( pph->typeflag),
               info->stat.st_size,
               (cp - pv),
               tartime( info->mtime, 0 ));
         }
         // else if (header->header.typeflag == XHDTYPE
		   // || header->header.typeflag == SOLARIS_XHDTYPE)
         if(( h == HEADER_SUCCESS )&&
            (( pph->typeflag == XHDTYPE )||
             ( pph->typeflag == SOLARIS_XHDTYPE )))
         {
            // we need to READ the xheader ...
            struct posix_header * pph_save = pph;
            fsz = (size_t)info->stat.st_size;
            len = 1;
            xheader_read (&info->xhdr, (union block *)pph, (size_t)fsz);
            if( VERB9 )
               sprtf( "Skipping %d extended header blocks ...\n", len );
            boff = ((count + len) * BLOCKSIZE);
            pph = (struct posix_header *)&pv[boff];
            if( (boff + BLOCKSIZE) > lpdf->dwmax )
            {
               sprtf( "WARNING: DATA EXPIRED BEFORE ZERO BLOCK!"MEOR );
               pph = pph_save;
               break;
            }
            tar_stat_destroy(info);
            pph = (struct posix_header *)current_block; // move up to this block
            current_header = (union block *)pph;
            h = tar_checksum ((union block *)pph, TRUE);
            form = -1;
            decode_header ((union block *)pph, info, &form, 0);
            if( pph->typeflag == LNKTYPE )
               info->stat.st_size = 0;	/* links 0 size on tape */
            else
               info->stat.st_size = OFF_FROM_HEADER (pph->size);
            fsz = (size_t)info->stat.st_size;
            if( h == HEADER_SUCCESS ) {
               XHDTYPE_count++;
               if( VERB5 )
               {
                  cp = (char *)pph;
                  sprtf( "%s: form [%s](%d): type [%s]: size %9d; offset %#08X; time %s"MEOR,
                     get_header_stg( h ),
                     get_format_stg(form),
                     form,
                     get_type_string( pph->typeflag),
                     info->stat.st_size,
                     (cp - pv),
                     tartime( info->mtime, 0 ));
               }
               count += len;
            } else {
               pph = pph_save;
               tar_stat_destroy(info);
               current_header = (union block *)pph;
               h = tar_checksum ((union block *)pph, TRUE);
               form = -1;
               decode_header ((union block *)pph, info, &form, 0);
            }
         }

         fsz = 0;
         i = collect_octal_size( lpb, pph, &fsz, 1 );
         off = collect_prefix_name( lpb, pph );
         if( off ) {
            // we GOT a NAME
            sprtf( "%5d %-50s %s %10d %s"MEOR,
               entry,
               lpb,
               get_type_string(pph->typeflag),
               fsz,
               tartime( info->mtime, 0 ));
            // gather counts
            switch(pph->typeflag)
            {
            case REGTYPE:
            case AREGTYPE:
               REGTYPE_count++;
               if( fsz > iMaxFileSize )
                  iMaxFileSize = fsz;
               if( fsz < iMinFileSize )
                  iMinFileSize = fsz;
               remains = fsz;
               while(remains)
               {
                  count++;
                  boff = count * BLOCKSIZE;
                  if(remains > BLOCKSIZE)
                     len = BLOCKSIZE;
                  else
                     len = remains;
                  culen = 0;
                  if( VERB9 && bExtraDebug )
                  {
                     while( len )
                     {
                        if(len > 16)
                           hlen = 16;
                        else
                           hlen = len;
                        *lpb = 0;
                        GetHEXString( lpb, &pv[boff+culen], hlen, pv, FALSE );
                        sprtf( "%s"MEOR, lpb );
                        len -= hlen;
                        culen += hlen;
                     }
                  }
                  pph = (struct posix_header *) &pv[boff];
                  if(remains > BLOCKSIZE)
                     remains -= BLOCKSIZE;
                  else
                     remains = 0;
               }
               break;
            case LNKTYPE:  // , "link" },
               LNKTYPE_count++;
               break;
            case SYMTYPE:  //, "reserved" },
               SYMTYPE_count++;
               break;
            case CHRTYPE:  // , "character special" },
               CHRTYPE_count++;
               break;
            case BLKTYPE:  //, "block special" }, //  '4'		/* block special */
               BLKTYPE_count++;
               break;
            case DIRTYPE:  // , "directory" },     //  '5'		/* directory */
               DIRTYPE_count++;
               break;
            case FIFOTYPE: //, "FIFO special" }, // '6'		/* FIFO special */
               FIFOTYPE_count++;
               break;
            case CONTTYPE: //, "reserved" },     // '7'		/* reserved */
               CONTTYPE_count++;
               break;
            case XHDTYPE:  //, "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
               XHDTYPE_count++;
               break;
            case XGLTYPE:  //, "global header" }, //  'g'            /* Global extended header */
               XGLTYPE_count++;
               break;
            default:
               default_count++;
               sprtf("WARNING: Uncased type %c!!!"MEOR, pph->typeflag);
               break;
            }
         }
         count++;
      }  // while ...

      if( max_warn_count < warn_count )
      {
         sprtf( "WARNING: Plus another [%d] OUT OF OCTAL RANGE!!!"MEOR,
            warn_count - max_warn_count );
      }

      // fell out because of a BLANK block
      sprtf( "Processed %d entries", maxcnt );
      if( REGTYPE_count ) {
         // case AREGTYPE: counted with above
         sprtf( ", %d file%s (min %d, max %d)",
            REGTYPE_count,
            ((REGTYPE_count == 1) ? "" : "s"),
            iMinFileSize,
            iMaxFileSize );
      }
      if( LNKTYPE_count )
         sprtf( ", %d link%s", LNKTYPE_count, ((LNKTYPE_count == 1) ? "" : "s") );
      if( SYMTYPE_count )
         sprtf( ", %d sym%s", SYMTYPE_count, ((SYMTYPE_count == 1) ? "" : "s") );
      if( CHRTYPE_count )
         sprtf( ", %d char%s", CHRTYPE_count, ((CHRTYPE_count == 1) ? "" : "s") );
      if( BLKTYPE_count )
         sprtf( ", %d block%s", BLKTYPE_count, ((BLKTYPE_count == 1) ? "" : "s") );
      if( DIRTYPE_count )
         sprtf( ", %d dir%s", DIRTYPE_count, ((DIRTYPE_count == 1) ? "" : "s") );
      if( FIFOTYPE_count )
         sprtf( ", %d fifo%s", FIFOTYPE_count, ((FIFOTYPE_count == 1) ? "" : "s") );
      if( CONTTYPE_count )
         sprtf( ", %d cont%s", CONTTYPE_count, ((CONTTYPE_count == 1) ? "" : "s") );
      if( XHDTYPE_count )
         sprtf( ", %d xhdr%s", XHDTYPE_count, ((XHDTYPE_count == 1) ? "" : "s") );
      if( XGLTYPE_count )
         sprtf( ", %d xgl%s", XGLTYPE_count, ((XGLTYPE_count == 1) ? "" : "s") );
      if( default_count )
         sprtf( ", %d default%s", default_count, ((default_count == 1) ? "" : "s") );
      sprtf( " (blocks %d)"MEOR,
         count );

      count++;
      boff = count * BLOCKSIZE;
      remains = (lpdf->dwmax - (boff + BLOCKSIZE));
      strcpy(lpb, (remains == 0 ? "perfect" : "remainder " ));
      if(remains) 
        sprintf(EndBuf(lpb)," %d", (int)remains );
      if(remains > 0)
      {
         off = 0;
         boff = count * BLOCKSIZE;
         if( VERB )
            sprtf( "Reached offset %d of %d ... %s"MEOR,
            (boff + BLOCKSIZE), lpdf->dwmax, lpb );
         while(remains > 0)
         {
            if(remains > BLOCKSIZE)
               len = BLOCKSIZE;
            else
               len = remains;
            culen = 0;
            remains -= len;
            while( len )
            {
               if(len > 16)
                  hlen = 16;
               else
                  hlen = len;
               for( i = 0; i < (int)hlen; i++ )
               {
                  c = pv[boff+culen+i];
                  if(c)
                     break;
               }
               if(c) {
                  if(VERB5)
                  {
                     *lpb = 0;
                     GetHEXString( lpb, &pv[boff+culen], hlen, pv, FALSE );
                     sprtf( "%s"MEOR, lpb );
                  }
                  off++;
               }
               len -= hlen;
               culen += hlen;
            }
            count++;
         }
         // done the remainder
         if( off == 0 ) {
            // NO HEX OUTPUT
            sprtf( "But it was ALL zeros ... (to %d)"MEOR, ((count+1) * BLOCKSIZE) );
         }
      }
      bRet = TRUE;
   }

   return bRet;
}

#if 0 // ************************************************
void
xheader_destroy (struct xheader *xhdr)
{
   if (xhdr->stk)
      free (xhdr->stk);
   obstack_free (xhdr->stk, NULL);
   xhdr->stk = NULL;
   if(xhdr->buffer)
      free (xhdr->buffer);
   xhdr->buffer = 0;
   xhdr->size = 0;
}
#endif // 0 **********************************************

void
tar_stat_destroy (struct tar_stat_info *st)
{
   if(st->orig_file_name)
      free (st->orig_file_name);
   if(st->file_name)
      free (st->file_name);
   if(st->link_name)
      free (st->link_name);
   if(st->uname)
      free (st->uname);
   if(st->gname)
      free (st->gname);
   if(st->sparse_map)
      free (st->sparse_map);
   if(st->dumpdir)
      free (st->dumpdir);
  xheader_destroy (&st->xhdr);
  memset (st, 0, sizeof (*st));
}


typedef struct tagHDR2STG
{
   enum read_header rhdr;
   char * name;
}HDR2STG, * PHDR2STG;

HDR2STG hdr2stg[] = {
   { HEADER_STILL_UNREAD,	"HEADER_STILL_UNREAD" },	/* for when read_header has not been called */
   { HEADER_SUCCESS,	"HEADER_SUCCESS" },	/* header successfully read and checksummed */
   { HEADER_SUCCESS_EXTENDED,	"HEADER_SUCCESS_EXTENDED" }, /* likewise, but we got an extended header */
   { HEADER_ZERO_BLOCK,	"HEADER_ZERO_BLOCK" },	/* zero block where header expected */
   { HEADER_END_OF_FILE, "HEADER_END_OF_FILE" },		/* true end of file while header expected */
   { HEADER_FAILURE, "HEADER_FAILURE" },		/* ill-formed header, or bad checksum */
   { 0, 0 }
};

char * get_header_stg( enum read_header hdr )
{
   PHDR2STG ph = &hdr2stg[0];
   while( ph->name )
   {
      if( ph->rhdr == hdr )
         return ph->name;
      ph++;
   }
   return "HEADER_UNCASED";
}

void
decode_header_format (union block *header, struct tar_stat_info *stat_info,
	       enum archive_format *format_pointer, int do_user_group)
{
  enum archive_format format;

  if (strcmp (header->header.magic, TMAGIC) == 0)
    {
      if (header->star_header.prefix[130] == 0
	  && ISOCTAL (header->star_header.atime[0])
	  && header->star_header.atime[11] == ' '
	  && ISOCTAL (header->star_header.ctime[0])
	  && header->star_header.ctime[11] == ' ')
	format = STAR_FORMAT;
      else if (stat_info->xhdr.size)
	format = POSIX_FORMAT;
      else
	format = USTAR_FORMAT;
    }
  else if (strcmp (header->header.magic, OLDGNU_MAGIC) == 0)
    format = OLDGNU_FORMAT;
  else
    format = V7_FORMAT;
  *format_pointer = format;

#if 0 // 0 *************************************************
  stat_info->stat.st_mode = MODE_FROM_HEADER (header->header.mode);
  stat_info->mtime.tv_sec = TIME_FROM_HEADER (header->header.mtime);
  stat_info->mtime.tv_nsec = 0;
  assign_string (&stat_info->uname,
		 header->header.uname[0] ? header->header.uname : NULL);
  assign_string (&stat_info->gname,
		 header->header.gname[0] ? header->header.gname : NULL);

  if (format == OLDGNU_FORMAT && incremental_option)
    {
      stat_info->atime.tv_sec = TIME_FROM_HEADER (header->oldgnu_header.atime);
      stat_info->ctime.tv_sec = TIME_FROM_HEADER (header->oldgnu_header.ctime);
      stat_info->atime.tv_nsec = stat_info->ctime.tv_nsec = 0;
    }
  else if (format == STAR_FORMAT)
    {
      stat_info->atime.tv_sec = TIME_FROM_HEADER (header->star_header.atime);
      stat_info->ctime.tv_sec = TIME_FROM_HEADER (header->star_header.ctime);
      stat_info->atime.tv_nsec = stat_info->ctime.tv_nsec = 0;
    }
  else
    stat_info->atime = stat_info->ctime = start_time;

  if (format == V7_FORMAT)
    {
      stat_info->stat.st_uid = UID_FROM_HEADER (header->header.uid);
      stat_info->stat.st_gid = GID_FROM_HEADER (header->header.gid);
      stat_info->stat.st_rdev = 0;
    }
  else
    {
      if (do_user_group)
	{
	  /* FIXME: Decide if this should somewhat depend on -p.  */

	  //**if (numeric_owner_option
	  //**    || !*header->header.uname
	  //**    || !uname_to_uid (header->header.uname, &stat_info->stat.st_uid))
	    stat_info->stat.st_uid = UID_FROM_HEADER (header->header.uid);

	  //**if (numeric_owner_option
	  //**    || !*header->header.gname
	  //**    || !gname_to_gid (header->header.gname, &stat_info->stat.st_gid))
	    stat_info->stat.st_gid = GID_FROM_HEADER (header->header.gid);
	}

      switch (header->header.typeflag)
	{
	case BLKTYPE:
	case CHRTYPE:
	  stat_info->stat.st_rdev = 0;
	  //**  makedev (MAJOR_FROM_HEADER (header->header.devmajor),
		//**     MINOR_FROM_HEADER (header->header.devminor));
	  break;

	default:
	  stat_info->stat.st_rdev = 0;
	}
    }

  stat_info->archive_file_size = stat_info->stat.st_size;
#endif // 0 **************************************************
  
}

int  IsTARFile( LPDFSTR lpdf, LPTSTR lpf, PBYTE pb, DWORD dwmax )
{
   int  bRet = 0;
   LPTSTR   lpb = &gcOutBuf[0];  // [16K]
   PTAR  pt = (PTAR)pb;
   PUSTAR put = (PUSTAR)pb;
   char * pv = (char *)lpdf->df_pVoid;
   size_t off, count, boff, fsz, remains, len, culen;
   struct posix_header * pph = (struct posix_header *)lpdf->df_pVoid;
   int i;

   iMaxNameLen = 0;
   iMaxFileSize = 0;
   iMinFileSize = 0x7fffffff;
   iFileCount = 0;
   iDirCount = 0;
   if(!pv || !pb)
      return bRet;

   tar_stat_destroy(&current_stat_info);
   current_header = (union block *)pph;
   rdhdr = tar_checksum ((union block *)pph, TRUE);
   current_format = -1;
   decode_header_format ((union block *)pph, &current_stat_info, &current_format, 0);
   if( VERB5 )
   {
      sprtf( "%s: Format appears to be [%s] (%d)..."MEOR,
         get_header_stg( rdhdr ),
         get_format_stg(current_format),
         current_format );
   }

   switch(current_format)
   {
   case V7_FORMAT:
      break;
   case OLDGNU_FORMAT:
      poldgnu_header = (struct oldgnu_header *)pph;
      break;
   case USTAR_FORMAT:
      break;
   case POSIX_FORMAT:
      pheader = (struct posix_header * )pph;
      break;
   case STAR_FORMAT:
      pstar_header = (struct star_header *)pph; 
      break;
   case GNU_FORMAT:
      break;
   default:
      break;
   }

   if(( STRNICMP(put->ustar, "ustar", 5) == 0  )||
      ( rdhdr == HEADER_SUCCESS ) ) {
      off = 1;
      count = 0;
      while(off)
      {
         boff = count * BLOCKSIZE;
         pph = (struct posix_header *) &pv[boff];
         if( (boff + BLOCKSIZE) > lpdf->dwmax )
         {
            sprtf( "ERROR: Offset %d GREATER THAN max %d ..."MEOR,
               (boff + BLOCKSIZE), lpdf->dwmax );
            return 0;
         }
         off = 0;
         fsz = 0;
         i = collect_octal_size( lpb, pph, &fsz, 0 );
         if( i == 1 )
         {
            if(( pph->typeflag == REGTYPE ) || ( pph->typeflag == AREGTYPE ))
            {
               if(fsz > iMaxFileSize)
                  iMaxFileSize = fsz;
               if(fsz < iMinFileSize)
                  iMinFileSize = fsz;
            }
         }
         off = collect_prefix_name( lpb, pph );
         if( off ) {
            if( off > iMaxNameLen )
               iMaxNameLen = off;
            //sprtf( "Entry [%s] is %s ... size %d"MEOR, lpb,
            //   get_type_string(pph->typeflag),
            //   fsz );
            switch(pph->typeflag)
            {
            case REGTYPE:
            case AREGTYPE:
               iFileCount++;
               remains = fsz;
               while(remains)
               {
                  count++;
                  boff = count * BLOCKSIZE;
                  if( (boff + BLOCKSIZE) > lpdf->dwmax )
                  {
                     sprtf( "ERROR: Offset %d GREATER THAN max %d ..."MEOR,
                        (boff + BLOCKSIZE), lpdf->dwmax );
                     return 0;
                  }
                  if(remains > BLOCKSIZE)
                     len = BLOCKSIZE;
                  else
                     len = remains;
                  culen = 0;
                  pph = (struct posix_header *) &pv[boff];
                  if(remains > BLOCKSIZE)
                     remains -= BLOCKSIZE;
                  else
                     remains = 0;
               }
               break;
            case DIRTYPE:  // , "directory" },     //  '5'		/* directory */
               iDirCount++;            
               break;
            case LNKTYPE:  // , "link" },
            case SYMTYPE:  //, "reserved" },
            case CHRTYPE:  // , "character special" },
            case BLKTYPE:  //, "block special" }, //  '4'		/* block special */
            case FIFOTYPE: //, "FIFO special" }, // '6'		/* FIFO special */
            case CONTTYPE: //, "reserved" },     // '7'		/* reserved */
               break;
            case XHDTYPE:  //, "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
               // should bump to NEXT, but this is only a CHECK of TAR, not decode yet

               break;
            case XGLTYPE:  //, "global header" }, //  'g'            /* Global extended header */
               break;
            default:
               //sprtf("WARNING: Uncased type %c!!!"MEOR, pph->typeflag);
               break;
            }
         } else {
            break;
         }
         count++;
      }
      count++;
      boff = count * BLOCKSIZE;
      if( (boff + BLOCKSIZE) > lpdf->dwmax )
      {
         sprtf( "ERROR: Offset %d GREATER THAN max %d ..."MEOR,
            (boff + BLOCKSIZE), lpdf->dwmax );
         return 0;
      }
      if(VERB5) {
         remains = (lpdf->dwmax - (boff + BLOCKSIZE));
         strcpy(lpb, (remains == 0 ? "perfect" : "remainder " ));
         if(remains)
            sprintf(EndBuf(lpb)," %d", (int)remains );
         sprtf( "Tested to offset %d of %d ... %s"MEOR,
            (boff + BLOCKSIZE), lpdf->dwmax, lpb );
         if( remains && VERB9 )
         {
            while(remains)
            {
               boff = count * BLOCKSIZE;
               if(remains > BLOCKSIZE)
                  len = BLOCKSIZE;
               else
                  len = remains;
               culen = 0;
               pph = (struct posix_header *) &pv[boff];
               if(remains > BLOCKSIZE)
                  remains -= BLOCKSIZE;
               else
                  remains = 0;
               count++;
            }
         }
      }
      return 2;
   } else {
      // NOT ustar 
      // what to do ...
   }

   return bRet;
}


BOOL  ProcessTAR( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   if( IsTARFile( lpdf, lpdf->fn, lpdf->lpb, lpdf->dwmax ) )
   {
      bRet = DumpTAR( lpdf );
   }
   return bRet;
}


// eof - DumpTar.c
