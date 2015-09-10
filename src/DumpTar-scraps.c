// DumpTar.c

#include "dump4.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>

#ifndef false
#define false FALSE
#endif
#ifndef true
#define true TRUE
#endif
#ifndef DEFAULT_BLOCKING
# define DEFAULT_BLOCKING 20
#endif

enum compress_type {
  ct_none,
  ct_compress,
  ct_gzip,
  ct_bzip2,
  ct_lzma
};

LPDFSTR active_lpdf;
size_t record_size;
int blocking_factor;
int record_index;
int records_read;
/* ========================
   Headers

Field Offset Field Size Field 
0            100        File name 
100          8          File mode 
108          8          Owner user ID 
116          8          Group user ID 
124          12         File size in bytes (octal) 
136          12         Last modification time 
148          8          Check sum for header block 
156          1          Link indicator 
157          100        Name of linked file 

The Link indicator field can have the following values:

Value Meaning 
'0' Normal file 
(ASCII NUL)[1] Normal file 
'1' Hard link 
'2' Symbolic link[2] 
'3' Character special 
'4' Block special 
'5' Directory 
'6' FIFO 
'7' Contiguous file[3] 

========= OR =========

The USTAR format allows for longer file names and stores extra information about each file.

Field Offset Field Size Field 
0            156        (as in old format) 
156          1          Type flag 
157          100        (as in old format) 
257          6          USTAR indicator "ustar" 
263          2          USTAR version "00" 
265          32         Owner user name 
297          32         Owner group name 
329          8          Device major number 
337          8          Device minor number 
345          155        Filename prefix 

   ============================================================= */

#pragma pack(1)

typedef struct tagTAR {
   // Field Offset Field Size Field 
   char file_name[100]; // 0            100        File name 
   char file_mode[8];   // 100          8          File mode 
   char Owner_ID[8];    // 108          8          Owner user ID 
   char Group_ID[8];    // 116          8          Group user ID 
   char file_size[12];  // 124          12         File size in bytes (octal) 
   char mod_time[12];   // 136          12         Last modification time 
   char chk_sum[8];     // 148          8          Check sum for header block 
   char link_ind[1];    // 156          1          Link indicator 
   char linked[100];    // 157          100        Name of linked file 
} TAR, * PTAR;

typedef struct tagUSTAR {
   // Field Offset Field Size Field 
   char file_name[156]; // 0            156        (as in old format) 
   char flag[1];        // 156          1          Type flag 
   char linked[100];    // 157          100        (as in old format) 
   char ustar[6];       // 257          6          USTAR indicator "ustar" 
   char uvers[2];       // 263          2          USTAR version "00" 
   char Owner_name[32]; // 265          32         Owner user name 
   char Group_name[32]; // 297          32         Owner group name 
   char Device_maj[8];  // 329          8          Device major number 
   char Device_min[8];  // 337          8          Device minor number 
   char fn_prefix[155]; // 345          155        Filename prefix 
} USTAR, * PUSTAR;

/* copy of tar.h */
//#ifndef off_t
//typedef size_t off_t;
//#endif
#ifndef bool
typedef char bool;
#endif
#ifndef uintmax_t
typedef unsigned int uintmax_t;
#endif
#ifndef _TIMESPEC           
#define _TIMESPEC
struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* and nanoseconds */  
};
#endif

/* GNU tar Archive Format description.

   Copyright (C) 1988, 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
   2000, 2001, 2003, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* tar Header Block, from POSIX 1003.1-1990.  */

/* POSIX header.  */

struct posix_header
{				/* byte offset */
  char name[100];		/*   0 */
  char mode[8];			/* 100 */
  char uid[8];			/* 108 */
  char gid[8];			/* 116 */
  char size[12];		/* 124 */
  char mtime[12];		/* 136 */
  char chksum[8];		/* 148 */
  char typeflag;		/* 156 */
  char linkname[100];		/* 157 */
  char magic[6];		/* 257 */
  char version[2];		/* 263 */
  char uname[32];		/* 265 */
  char gname[32];		/* 297 */
  char devmajor[8];		/* 329 */
  char devminor[8];		/* 337 */
  char prefix[155];		/* 345 */
				/* 500 */
};

#define TMAGIC   "ustar"	/* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"		/* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE	 '0'		/* regular file */
#define AREGTYPE '\0'		/* regular file */
#define LNKTYPE  '1'		/* link */
#define SYMTYPE  '2'		/* reserved */
#define CHRTYPE  '3'		/* character special */
#define BLKTYPE  '4'		/* block special */
#define DIRTYPE  '5'		/* directory */
#define FIFOTYPE '6'		/* FIFO special */
#define CONTTYPE '7'		/* reserved */

#define XHDTYPE  'x'            /* Extended header referring to the
				   next file in the archive */
#define XGLTYPE  'g'            /* Global extended header */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000		/* set UID on execution */
#define TSGID    02000		/* set GID on execution */
#define TSVTX    01000		/* reserved */
				/* file permissions */
#define TUREAD   00400		/* read by owner */
#define TUWRITE  00200		/* write by owner */
#define TUEXEC   00100		/* execute/search by owner */
#define TGREAD   00040		/* read by group */
#define TGWRITE  00020		/* write by group */
#define TGEXEC   00010		/* execute/search by group */
#define TOREAD   00004		/* read by other */
#define TOWRITE  00002		/* write by other */
#define TOEXEC   00001		/* execute/search by other */

/* tar Header Block, GNU extensions.  */

/* In GNU tar, SYMTYPE is for to symbolic links, and CONTTYPE is for
   contiguous files, so maybe disobeying the `reserved' comment in POSIX
   header description.  I suspect these were meant to be used this way, and
   should not have really been `reserved' in the published standards.  */

/* *BEWARE* *BEWARE* *BEWARE* that the following information is still
   boiling, and may change.  Even if the OLDGNU format description should be
   accurate, the so-called GNU format is not yet fully decided.  It is
   surely meant to use only extensions allowed by POSIX, but the sketch
   below repeats some ugliness from the OLDGNU format, which should rather
   go away.  Sparse files should be saved in such a way that they do *not*
   require two passes at archive creation time.  Huge files get some POSIX
   fields to overflow, alternate solutions have to be sought for this.  */

/* Descriptor for a single file hole.  */

struct sparse
{				/* byte offset */
  char offset[12];		/*   0 */
  char numbytes[12];		/*  12 */
				/*  24 */
};

/* Sparse files are not supported in POSIX ustar format.  For sparse files
   with a POSIX header, a GNU extra header is provided which holds overall
   sparse information and a few sparse descriptors.  When an old GNU header
   replaces both the POSIX header and the GNU extra header, it holds some
   sparse descriptors too.  Whether POSIX or not, if more sparse descriptors
   are still needed, they are put into as many successive sparse headers as
   necessary.  The following constants tell how many sparse descriptors fit
   in each kind of header able to hold them.  */

#define SPARSES_IN_EXTRA_HEADER  16
#define SPARSES_IN_OLDGNU_HEADER 4
#define SPARSES_IN_SPARSE_HEADER 21

/* Extension header for sparse files, used immediately after the GNU extra
   header, and used only if all sparse information cannot fit into that
   extra header.  There might even be many such extension headers, one after
   the other, until all sparse information has been recorded.  */

struct sparse_header
{				/* byte offset */
  struct sparse sp[SPARSES_IN_SPARSE_HEADER];
				/*   0 */
  char isextended;		/* 504 */
				/* 505 */
};

/* The old GNU format header conflicts with POSIX format in such a way that
   POSIX archives may fool old GNU tar's, and POSIX tar's might well be
   fooled by old GNU tar archives.  An old GNU format header uses the space
   used by the prefix field in a POSIX header, and cumulates information
   normally found in a GNU extra header.  With an old GNU tar header, we
   never see any POSIX header nor GNU extra header.  Supplementary sparse
   headers are allowed, however.  */

struct oldgnu_header
{				/* byte offset */
  char unused_pad1[345];	/*   0 */
  char atime[12];		/* 345 Incr. archive: atime of the file */
  char ctime[12];		/* 357 Incr. archive: ctime of the file */
  char offset[12];		/* 369 Multivolume archive: the offset of
				   the start of this volume */
  char longnames[4];		/* 381 Not used */
  char unused_pad2;		/* 385 */
  struct sparse sp[SPARSES_IN_OLDGNU_HEADER];
				/* 386 */
  char isextended;		/* 482 Sparse file: Extension sparse header
				   follows */
  char realsize[12];		/* 483 Sparse file: Real size*/
				/* 495 */
};

/* OLDGNU_MAGIC uses both magic and version fields, which are contiguous.
   Found in an archive, it indicates an old GNU header format, which will be
   hopefully become obsolescent.  With OLDGNU_MAGIC, uname and gname are
   valid, though the header is not truly POSIX conforming.  */
#define OLDGNU_MAGIC "ustar  "	/* 7 chars and a null */

/* The standards committee allows only capital A through capital Z for
   user-defined expansion.  Other letters in use include:

   'A' Solaris Access Control List
   'E' Solaris Extended Attribute File
   'I' Inode only, as in 'star'
   'N' Obsolete GNU tar, for file names that do not fit into the main header.
   'X' POSIX 1003.1-2001 eXtended (VU version)  */

/* This is a dir entry that contains the names of files that were in the
   dir at the time the dump was made.  */
#define GNUTYPE_DUMPDIR	'D'

/* Identifies the *next* file on the tape as having a long linkname.  */
#define GNUTYPE_LONGLINK 'K'

/* Identifies the *next* file on the tape as having a long name.  */
#define GNUTYPE_LONGNAME 'L'

/* This is the continuation of a file that began on another volume.  */
#define GNUTYPE_MULTIVOL 'M'

/* This is for sparse files.  */
#define GNUTYPE_SPARSE 'S'

/* This file is a tape/volume header.  Ignore it on extraction.  */
#define GNUTYPE_VOLHDR 'V'

/* Solaris extended header */
#define SOLARIS_XHDTYPE 'X'

/* J@"org Schilling star header */

struct star_header
{				/* byte offset */
  char name[100];		/*   0 */
  char mode[8];			/* 100 */
  char uid[8];			/* 108 */
  char gid[8];			/* 116 */
  char size[12];		/* 124 */
  char mtime[12];		/* 136 */
  char chksum[8];		/* 148 */
  char typeflag;		/* 156 */
  char linkname[100];		/* 157 */
  char magic[6];		/* 257 */
  char version[2];		/* 263 */
  char uname[32];		/* 265 */
  char gname[32];		/* 297 */
  char devmajor[8];		/* 329 */
  char devminor[8];		/* 337 */
  char prefix[131];		/* 345 */
  char atime[12];               /* 476 */
  char ctime[12];               /* 488 */
                                /* 500 */
};

#define SPARSES_IN_STAR_HEADER      4
#define SPARSES_IN_STAR_EXT_HEADER  21

struct star_in_header
{
  char fill[345];       /*   0  Everything that is before t_prefix */
  char prefix[1];       /* 345  t_name prefix */
  char fill2;           /* 346  */
  char fill3[8];        /* 347  */
  char isextended;      /* 355  */
  struct sparse sp[SPARSES_IN_STAR_HEADER]; /* 356  */
  char realsize[12];    /* 452  Actual size of the file */
  char offset[12];      /* 464  Offset of multivolume contents */
  char atime[12];       /* 476  */
  char ctime[12];       /* 488  */
  char mfill[8];        /* 500  */
  char xmagic[4];       /* 508  "tar" */
};

struct star_ext_header
{
  struct sparse sp[SPARSES_IN_STAR_EXT_HEADER];
  char isextended;
};

/* END */

# define PTR_INT_TYPE ptrdiff_t

struct _obstack_chunk		/* Lives at front of each chunk. */
{
  char  *limit;			/* 1 past end of this chunk */
  struct _obstack_chunk *prev;	/* address of prior chunk or NULL */
  char	contents[4];		/* objects begin here */
};

struct obstack		/* control current object in current chunk */
{
  long	chunk_size;		/* preferred size to allocate chunks in */
  struct _obstack_chunk *chunk;	/* address of current struct obstack_chunk */
  char	*object_base;		/* address of object we are building */
  char	*next_free;		/* where to add next char to current object */
  char	*chunk_limit;		/* address of char after current chunk */
  union
  {
    PTR_INT_TYPE tempint;
    void *tempptr;
  } temp;			/* Temporary for some macros.  */
  int   alignment_mask;		/* Mask of alignment for each object. */
  /* These prototypes vary based on `use_extra_arg', and we use
     casts to the prototypeless function type in all assignments,
     but having prototypes here quiets -Wstrict-prototypes.  */
  struct _obstack_chunk *(*chunkfun) (void *, long);
  void (*freefun) (void *, struct _obstack_chunk *);
  void *extra_arg;		/* first arg for chunk alloc/dealloc funcs */
  unsigned use_extra_arg:1;	/* chunk alloc/dealloc funcs take extra arg */
  unsigned maybe_empty_object:1;/* There is a possibility that the current
				   chunk contains a zero-length object.  This
				   prevents freeing the chunk if we allocate
				   a bigger chunk to replace it. */
  unsigned alloc_failed:1;	/* No longer used, as we now call the failed
				   handler on error, but retained for binary
				   compatibility.  */
};



/* tar Header Block, overall structure.  */

/* tar files are made in basic blocks of this size.  */
#define BLOCKSIZE 512

enum archive_format
{
  DEFAULT_FORMAT,		/* format to be decided later */
  V7_FORMAT,			/* old V7 tar format */
  OLDGNU_FORMAT,		/* GNU format as per before tar 1.12 */
  USTAR_FORMAT,                 /* POSIX.1-1988 (ustar) format */
  POSIX_FORMAT,			/* POSIX.1-2001 format */
  STAR_FORMAT,                  /* Star format defined in 1994 */
  GNU_FORMAT			/* Same as OLDGNU_FORMAT with one exception:
                                   see FIXME note for to_chars() function
                                   (create.c:189) */
};

/* Information about a sparse file.  */
struct sp_array
{
  off_t offset;
  size_t numbytes;
};

struct xheader
{
  struct obstack *stk;
  size_t size;
  char *buffer;
  uintmax_t string_length;
};

struct tar_stat_info
{
  char *orig_file_name;     /* name of file read from the archive header */
  char *file_name;          /* name of file for the current archive entry
			       after being normalized.  */
  bool had_trailing_slash;  /* true if the current archive entry had a
			       trailing slash before it was normalized. */
  char *link_name;          /* name of link for the current archive entry.  */

  char          *uname;     /* user name of owner */
  char          *gname;     /* group name of owner */
  struct stat   stat;       /* regular filesystem stat */

  /* STAT doesn't always have access, data modification, and status
     change times in a convenient form, so store them separately.  */
  struct timespec atime;
  struct timespec mtime;
  struct timespec ctime;

  off_t archive_file_size;  /* Size of file as stored in the archive.
			       Equals stat.st_size for non-sparse files */

  bool   is_sparse;         /* Is the file sparse */

  /* For sparse files: */
  unsigned sparse_major;
  unsigned sparse_minor;
  size_t sparse_map_avail;  /* Index to the first unused element in
			       sparse_map array. Zero if the file is
			       not sparse */
  size_t sparse_map_size;   /* Size of the sparse map */
  struct sp_array *sparse_map;

  /* Extended headers */
  struct xheader xhdr;
  
  /* For dumpdirs */
  bool is_dumpdir;          /* Is the member a dumpdir? */
  bool skipped;             /* The member contents is already read
			       (for GNUTYPE_DUMPDIR) */
  char *dumpdir;            /* Contents of the dump directory */
};

union block
{
  char buffer[BLOCKSIZE];
  struct posix_header header;
  struct star_header star_header;
  struct oldgnu_header oldgnu_header;
  struct sparse_header sparse_header;
  struct star_in_header star_in_header;
  struct star_ext_header star_ext_header;
};

// eof - tar.h ...
// some of tar.c
static struct fmttab {
  char const *name;
  enum archive_format fmt;
} const fmttab[] = {
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

static BOOL bExtraDebug = FALSE;
static size_t iMaxNameLen;
static size_t iMaxFileSize;
static size_t iMinFileSize;
static size_t iFileCount;
static size_t iDirCount;


BOOL  DumpTAR( LPDFSTR lpdf )
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

   if( !pph )
      return bRet;

   if( _strnicmp(pph->magic, TMAGIC, 5) == 0 ) {
      // we have a USTAR format
      off = 1;
      count = 0;
      sprtf( "TAR [%s] is USTAR format ..."MEOR, lpdf->fn );
      while(off)
      {
         entry++;
         boff = count * BLOCKSIZE;
         pph = (struct posix_header *) &pv[boff];
         off = 0;
         cp = pph->size;   // char size[12];		/* 124 */
         //fsz = atoi(cp);
         for( i = 0; i < 12; i++ )
         {
            c = cp[i];
            if( c == 0 )
               break;
            if(( c == ' ' )&&(off == 0))
               continue;
            if( !ISOCTALCHR(c) )
               sprtf( "WARNING: [%c] OUT OF OCTAL RANGE!!!"MEOR, c );
            lpb[off++] = c;
         }
         lpb[off] = 0;
         i = 1;
         fsz = 0;
         if(off)
            i = sscanf_s( lpb, "%o", &fsz );
         if( i != 1 )
            sprtf( "WARNING: Bad sscanf_s return value %d!"MEOR, i );
         cp = pph->prefix;
         off = 0;
         while(*cp) {
            c = *cp;
            lpb[off++] = c;
            cp++;
         }
         if(off) {
            if( c != '/' )
               lpb[off++] = '/';
         }
         cp = pph->name;
         while(*cp) {
            lpb[off++] = *cp;
            cp++;
         }
         lpb[off] = 0;
         if( off ) {
            sprtf( "%5d %-50s %s %10d"MEOR,
               entry,
               lpb,
               get_type_string(pph->typeflag),
               fsz );
            switch(pph->typeflag)
            {
            case REGTYPE:
            case AREGTYPE:
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
            case SYMTYPE:  //, "reserved" },
            case CHRTYPE:  // , "character special" },
            case BLKTYPE:  //, "block special" }, //  '4'		/* block special */
            case DIRTYPE:  // , "directory" },     //  '5'		/* directory */
            case FIFOTYPE: //, "FIFO special" }, // '6'		/* FIFO special */
            case CONTTYPE: //, "reserved" },     // '7'		/* reserved */
            case XHDTYPE:  //, "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
            case XGLTYPE:  //, "global header" }, //  'g'            /* Global extended header */

               break;
            default:
               sprtf("WARNING: Uncased type %c!!!"MEOR, pph->typeflag);
               break;
            }
         }
         count++;
      }

      // fell out because of a BLANK block
      sprtf( "Listed %d entries, %d files, %d folders ... min %d, max %d bytes (%d)"MEOR,
         maxcnt, iFileCount, iDirCount,
         iMinFileSize, iMaxFileSize,
         count );
      count++;
      boff = count * BLOCKSIZE;
      if(VERB5) {
         remains = (lpdf->dwmax - (boff + BLOCKSIZE));
         strcpy(lpb, (remains == 0 ? "perfect" : "remainder " ));
         if(remains) sprintf(EndBuf(lpb)," %d", remains );
         if( (remains > 0) && VERB5 )
         {
            off = 0;
            boff = count * BLOCKSIZE;
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
               if( VERB5 )
               {
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
                        *lpb = 0;
                        GetHEXString( lpb, &pv[boff+culen], hlen, pv, FALSE );
                        sprtf( "%s"MEOR, lpb );
                        off++;
                     }
                     len -= hlen;
                     culen += hlen;
                  }
               }
               count++;
            }
            // done the remainder
            if( off == 0 ) {
               // NO HEX OUTPUT
               sprtf( "But it was ALL zeros ... (to %d)"MEOR, ((count+1) * BLOCKSIZE) );
            }
         }
      }

      bRet = TRUE;

   }

   return bRet;
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
   char * cp;
   char c;
   int i;

   iMaxNameLen = 0;
   iMaxFileSize = 0;
   iMinFileSize = 0x7fffffff;
   iFileCount = 0;
   iDirCount = 0;

   if(!pv || !pb)
      return bRet;

   if( _strnicmp(put->ustar, "ustar", 5) == 0  ) {
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
         cp = pph->size;   // char size[12];		/* 124 */
         //fsz = atoi(cp);
         for( i = 0; i < 12; i++ )
         {
            c = cp[i];
            if( c == 0 )
               break;
            if(( c == ' ' )&&(off == 0))
               continue;
            //if( !ISOCTALCHR(c) )
            //   sprtf( "WARNING: [%c] OUT OF OCTAL RANGE!!!"MEOR, c );
            lpb[off++] = c;
         }
         lpb[off] = 0;
         i = 1;
         fsz = 0;
         if(off) {
            i = sscanf_s( lpb, "%o", &fsz );
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
         }
         //if( i != 1 )
         //   sprtf( "WARNING: Bad sscanf_s return value %d!"MEOR, i );
         cp = pph->prefix;
         off = 0;
         while(*cp) {
            c = *cp;
            lpb[off++] = c;
            cp++;
         }
         if(off) {
            if( c != '/' )
               lpb[off++] = '/';
         }
         cp = pph->name;
         while(*cp) {
            lpb[off++] = *cp;
            cp++;
         }
         lpb[off] = 0;
         if( off ) {
            if( strlen(lpb) > iMaxNameLen )
               iMaxNameLen = strlen(lpb);

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
            case XHDTYPE:  //, "extended header" }, //  'x'   /* Extended header referring to the next file in the archive */
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
         if(remains) sprintf(EndBuf(lpb)," %d", remains );
         sprtf( "Reached offset %d of %d ... %s"MEOR,
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
   }

   return bRet;
}

static off_t record_start_block; /* block ordinal at record_start */
struct tar_stat_info current_stat_info;
union block *current_header;
union block *record_start;	/* start of record of archive */
union block *record_end;	/* last+1 block of archive record */
union block *current_block;	/* current block of archive */
enum archive_format current_format;
bool hit_eof = false;
enum archive_format archive_format;
void *record_buffer[2];	/* allocated memory */
union block *record_buffer_aligned[2];

enum read_header
{
  HEADER_STILL_UNREAD,		/* for when read_header has not been called */
  HEADER_SUCCESS,		/* header successfully read and checksummed */
  HEADER_SUCCESS_EXTENDED,	/* likewise, but we got an extended header */
  HEADER_ZERO_BLOCK,		/* zero block where header expected */
  HEADER_END_OF_FILE,		/* true end of file while header expected */
  HEADER_FAILURE		/* ill-formed header, or bad checksum */
};

void flush_read( void )
{
   // status = rmtread (archive, record_start->buffer, record_size);
   LPDFSTR lpdf = active_lpdf;
   size_t remains = lpdf->dwmax - lpdf->df_dwOff;
   size_t copy, left;
   if( remains > record_size )
      copy = record_size;
   else
      copy = remains;
   if( copy )
      memcpy( record_start->buffer, lpdf->df_pVoid, copy );

   lpdf->df_dwOff += copy;
   if (copy == record_size)
	{
      records_read++;
      return;
   }
   left = record_size - copy;
   record_end = record_start + (record_size - left) / BLOCKSIZE;
   memset( &record_start->buffer[copy], 0, left );
   records_read++;
}

#define  WARN(a)
#define  TARERROR(a)

#if STDC_HEADERS
# define IN_CTYPE_DOMAIN(c) 1
#else
# define IN_CTYPE_DOMAIN(c) ((unsigned) (c) <= 0177)
#endif

#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)
#define ISODIGIT(c) ((unsigned) (c) - '0' <= 7)
#define ISPRINT(c) (IN_CTYPE_DOMAIN (c) && isprint (c))
#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
/* Log base 2 of common values.  */
#define LG_8 3
#define LG_64 6
#define LG_256 8

/* Base 64 digits; see Internet RFC 2045 Table 1.  */
static char const base_64_digits[64] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

/* Table of base-64 digit values indexed by unsigned chars.
   The value is 64 for unsigned chars that are not base-64 digits.  */
static char base64_map[UCHAR_MAX + 1];

static void
base64_init (void)
{
  int i;
  memset (base64_map, 64, sizeof base64_map);
  for (i = 0; i < 64; i++)
    base64_map[(int) base_64_digits[i]] = i;
}

static uintmax_t
from_header (char const *where0, size_t digs, char const *type,
	     uintmax_t in_minus_minval, uintmax_t in_maxval,
	     bool octal_only, bool silent)
{
  //*** uintmax_t value;
  int value;
  char const *where = where0;
  char const *lim = where + digs;
  int negative = 0;
  int maxval = in_maxval;
  int minus_minval = in_minus_minval;

  /* Accommodate buggy tar of unknown vintage, which outputs leading
     NUL if the previous field overflows.  */
  where += !*where;

  /* Accommodate older tars, which output leading spaces.  */
  for (;;)
    {
      if (where == lim)
	{
	  if (type && !silent)
	    TARERROR ((0, 0,
		    /* TRANSLATORS: %s is type of the value (gid_t, uid_t, etc.) */
		    _("Blanks in header where numeric %s value expected"),
		    type));
	  return -1;
	}
      if (!ISSPACE ((unsigned char) *where))
	break;
      where++;
    }

  value = 0;
  if (ISODIGIT (*where))
    {
      char const *where1 = where;
      //*** uintmax_t overflow = 0;
      int overflow = 0;

      for (;;)
	{
	  value += *where++ - '0';
	  if (where == lim || ! ISODIGIT (*where))
	    break;
	  overflow |= value ^ (value << LG_8 >> LG_8);
	  value <<= LG_8;
	}

      /* Parse the output of older, unportable tars, which generate
         negative values in two's complement octal.  If the leading
         nonzero digit is 1, we can't recover the original value
         reliably; so do this only if the digit is 2 or more.  This
         catches the common case of 32-bit negative time stamps.  */
      if ((overflow || maxval < value) && '2' <= *where1 && type)
	{
	  /* Compute the negative of the input value, assuming two's
	     complement.  */
	  int digit = (*where1 - '0') | 4;
	  overflow = 0;
	  value = 0;
	  where = where1;
	  for (;;)
	    {
	      value += 7 - digit;
	      where++;
	      if (where == lim || ! ISODIGIT (*where))
		break;
	      digit = *where - '0';
	      overflow |= value ^ (value << LG_8 >> LG_8);
	      value <<= LG_8;
	    }
	  value++;
	  overflow |= !value;

	  if (!overflow && value <= minus_minval)
	    {
	      if (!silent)
		WARN ((0, 0,
		       /* TRANSLATORS: Second %s is a type name (gid_t,uid_t,etc.) */
		       _("Archive octal value %.*s is out of %s range; assuming two's complement"),
		       (int) (where - where1), where1, type));
	      negative = 1;
	    }
	}

      if (overflow)
	{
	  if (type && !silent)
	    TARERROR ((0, 0,
		    /* TRANSLATORS: Second %s is a type name (gid_t,uid_t,etc.) */
		    _("Archive octal value %.*s is out of %s range"),
		    (int) (where - where1), where1, type));
	  return -1;
	}
    }
  else if (octal_only)
    {
      /* Suppress the following extensions.  */
    }
  else if (*where == '-' || *where == '+')
    {
      /* Parse base-64 output produced only by tar test versions
	 1.13.6 (1999-08-11) through 1.13.11 (1999-08-23).
	 Support for this will be withdrawn in future releases.  */
      int dig;
      if (!silent)
	{
	  static bool warned_once;
	  if (! warned_once)
	    {
	      warned_once = true;
	      WARN ((0, 0, _("Archive contains obsolescent base-64 headers")));
	    }
	}
      negative = *where++ == '-';
      while (where != lim
	     && (dig = base64_map[(unsigned char) *where]) < 64)
	{
	  if (value << LG_64 >> LG_64 != value)
	    {
	      //char *string = alloca (digs + 1);
	      char *string = malloc (digs + 1);
	      memcpy (string, where0, digs);
	      string[digs] = '\0';
	      if (type && !silent)
		TARERROR ((0, 0,
			_("Archive signed base-64 string %s is out of %s range"),
			quote (string), type));
	      return -1;
	    }
	  value = (value << LG_64) | dig;
	  where++;
	}
    }
  else if (*where == '\200' /* positive base-256 */
	   || *where == '\377' /* negative base-256 */)
    {
      /* Parse base-256 output.  A nonnegative number N is
	 represented as (256**DIGS)/2 + N; a negative number -N is
	 represented as (256**DIGS) - N, i.e. as two's complement.
	 The representation guarantees that the leading bit is
	 always on, so that we don't confuse this format with the
	 others (assuming ASCII bytes of 8 bits or more).  */
      int signbit = *where & (1 << (LG_256 - 2));
      uintmax_t topbits = (((uintmax_t) - signbit)
			   << (CHAR_BIT * sizeof (uintmax_t)
			       - LG_256 - (LG_256 - 2)));
      value = (*where++ & ((1 << (LG_256 - 2)) - 1)) - signbit;
      for (;;)
	{
	  value = (value << LG_256) + (unsigned char) *where++;
	  if (where == lim)
	    break;
	  if (((value << LG_256 >> LG_256) | topbits) != value)
	    {
	      if (type && !silent)
		TARERROR ((0, 0,
			_("Archive base-256 value is out of %s range"),
			type));
	      return -1;
	    }
	}
      negative = signbit;
      if (negative)
	value = -value;
    }

  if (where != lim && *where && !ISSPACE ((unsigned char) *where))
    {
      if (type)
	{
	  static char buf[1000]; /* Big enough to represent any header.  */
	  static struct quoting_options *o;

	  //***if (!o)
	  //*** {
	  //***   o = clone_quoting_options (0);
	  //***   set_quoting_style (o, locale_quoting_style);
	  //*** }

	  while (where0 != lim && ! lim[-1])
	    lim--;
	  //*** quotearg_buffer (buf, sizeof buf, where0, lim - where, o);
	  if (!silent)
	    TARERROR ((0, 0,
		    /* TRANSLATORS: Second %s is a type name (gid_t,uid_t,etc.) */
		    _("Archive contains %.*s where numeric %s value expected"),
		    (int) sizeof buf, buf, type));
	}

      return -1;
    }

  if (value <= (negative ? minus_minval : maxval))
    return negative ? -value : value;

#if   0
  if (type && !silent)
    {
      char minval_buf[UINTMAX_STRSIZE_BOUND + 1];
      char maxval_buf[UINTMAX_STRSIZE_BOUND];
      char value_buf[UINTMAX_STRSIZE_BOUND + 1];
      char *minval_string = STRINGIFY_BIGINT (minus_minval, minval_buf + 1);
      char *value_string = STRINGIFY_BIGINT (value, value_buf + 1);
      if (negative)
	*--value_string = '-';
      if (minus_minval)
	*--minval_string = '-';
      /* TRANSLATORS: Second %s is type name (gid_t,uid_t,etc.) */
      TARERROR ((0, 0, _("Archive value %s is out of %s range %s..%s"),
	      value_string, type,
	      minval_string, STRINGIFY_BIGINT (maxval, maxval_buf)));
    }
#endif

  return -1;
}

/* True if the arithmetic type T is an integer type.  bool counts as
   an integer.  */
#define TYPE_IS_INTEGER(t) ((t) 1.5 == 1)

/* True if negative values of the signed integer type T use two's
   complement, ones' complement, or signed magnitude representation,
   respectively.  Much GNU code assumes two's complement, but some
   people like to be portable to all possible C hosts.  */
#define TYPE_TWOS_COMPLEMENT(t) ((t) ~ (t) 0 == (t) -1)
#define TYPE_ONES_COMPLEMENT(t) ((t) ~ (t) 0 == 0)
#define TYPE_SIGNED_MAGNITUDE(t) ((t) ~ (t) 0 < (t) -1)

/* True if the arithmetic type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host.  */
#define TYPE_MINIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) 0 \
	: TYPE_SIGNED_MAGNITUDE (t) \
	? ~ (t) 0 \
	: ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1)))
#define TYPE_MAXIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) -1 \
	: ~ (~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))))


enum read_header
tar_checksum (union block *header, bool silent)
{
  size_t i;
  int unsigned_sum = 0;		/* the POSIX one :-) */
  int signed_sum = 0;		/* the Sun one :-( */
  int recorded_sum;
  uintmax_t parsed_sum;
  char *p;

  p = header->buffer;
  for (i = sizeof *header; i-- != 0;)
    {
      unsigned_sum += (unsigned char) *p;
      signed_sum += (signed char) (*p++);
    }

  if (unsigned_sum == 0)
    return HEADER_ZERO_BLOCK;

  /* Adjust checksum to count the "chksum" field as blanks.  */

  for (i = sizeof header->header.chksum; i-- != 0;)
    {
      unsigned_sum -= (unsigned char) header->header.chksum[i];
      signed_sum -= (signed char) (header->header.chksum[i]);
    }
  unsigned_sum += ' ' * sizeof header->header.chksum;
  signed_sum += ' ' * sizeof header->header.chksum;

  parsed_sum = from_header (header->header.chksum,
			    sizeof header->header.chksum, 0,
			    (uintmax_t) 0,
			    (uintmax_t) TYPE_MAXIMUM (int), true, silent);
  if (parsed_sum == (uintmax_t) -1)
    return HEADER_FAILURE;

  recorded_sum = parsed_sum;

  if (unsigned_sum != recorded_sum && signed_sum != recorded_sum)
    return HEADER_FAILURE;

  return HEADER_SUCCESS;
}

struct zip_magic
{
  enum compress_type type;
  size_t length;
  char *magic;
  char *program;
  char *option;
};

static struct zip_magic const magic[] = {
  { ct_none, },
  { ct_compress, 2, "\037\235", "compress", "-Z" },
  { ct_gzip,     2, "\037\213", "gzip", "-z"  },
  { ct_bzip2,    3, "BZh",      "bzip2", "-j" },
  { ct_lzma,     6, "\xFFLZMA", "lzma", "--lzma" }, /* FIXME: ???? */
};

#define NMAGIC (sizeof(magic)/sizeof(magic[0]))

enum compress_type flush_archive (void)
{
   struct zip_magic const *p;

  size_t buffer_level = current_block->buffer - record_start->buffer;
  record_start_block += record_end - record_start;
  current_block = record_start;
  record_end = record_start + blocking_factor;

  flush_read ();

  if (tar_checksum (record_start, true) == HEADER_SUCCESS)
    /* Probably a valid header */
    return ct_none;

  for (p = magic + 1; p < magic + NMAGIC; p++)
    if (memcmp (record_start->buffer, p->magic, p->length) == 0)
      return p->type;

  return ct_none;

}


union block *
find_next_block (void)
{
  if (current_block == record_end)
    {
      if (hit_eof)
	return 0;
      flush_archive ();
      if (current_block == record_end)
	{
	  hit_eof = true;
	  return 0;
	}
    }
  return current_block;
}

#define OFF_FROM_HEADER(where) off_from_header (where, sizeof (where))

off_t
off_from_header (const char *p, size_t s)
{
  /* Negative offsets are not allowed in tar files, so invoke
     from_header with minimum value 0, not TYPE_MINIMUM (off_t).  */
  return from_header (p, s, "off_t", (uintmax_t) 0,
		      (uintmax_t) TYPE_MAXIMUM (off_t), false, false);
}

void xalloc_die( void )
{
   sprtf( "CRITICAL ERROR: MEMORY FAILED!! Aborting ...\n" );
   exit(-1);
}

void * xmalloc (size_t size)
{
   void * vp = malloc(size);
   if(!vp)
      xalloc_die();
   return vp;
}

void
set_next_block_after (union block *block)
{
  while (block >= current_block)
    current_block++;

  /* Do *not* flush the archive here.  If we do, the same argument to
     set_next_block_after could mean the next block (if the input record
     is exactly one block long), which is not what is intended.  */

  if (current_block > record_end)
    abort ();
}

size_t
available_space_after (union block *pointer)
{
  return record_end->buffer - pointer->buffer;
}

#define alignof(type) offsetof (struct { char c; type x; }, x)
#define alignto(n, d) ((((n) + (d) - 1) / (d)) * (d))

/* Determine default alignment.  */
union fooround
{
  uintmax_t i;
  long double d;
  void *p;
};
struct fooalign
{
  char c;
  union fooround u;
};
/* If malloc were really smart, it would round addresses to DEFAULT_ALIGNMENT.
   But in fact it might be less smart and round addresses to as much as
   DEFAULT_ROUNDING.  So we prepare for it to do that.  */
enum
  {
    DEFAULT_ALIGNMENT = offsetof (struct fooalign, u),
    DEFAULT_ROUNDING = sizeof (union fooround)
  };

int
_obstack_begin (struct obstack *h,
		int size, int alignment,
		void *(*chunkfun) (long),
		void (*freefun) (void *))
{
  register struct _obstack_chunk *chunk; /* points to new chunk */

  if (alignment == 0)
    alignment = DEFAULT_ALIGNMENT;
  if (size == 0)
    /* Default size is what GNU malloc can fit in a 4096-byte block.  */
    {
      /* 12 is sizeof (mhead) and 4 is EXTRA from GNU malloc.
	 Use the values for range checking, because if range checking is off,
	 the extra bytes won't be missed terribly, but if range checking is on
	 and we used a larger request, a whole extra 4096 bytes would be
	 allocated.

	 These number are irrelevant to the new GNU malloc.  I suspect it is
	 less sensitive to the size of the request.  */
      int extra = ((((12 + DEFAULT_ROUNDING - 1) & ~(DEFAULT_ROUNDING - 1))
		    + 4 + DEFAULT_ROUNDING - 1)
		   & ~(DEFAULT_ROUNDING - 1));
      size = 4096 - extra;
    }

  h->chunkfun = (struct _obstack_chunk * (*)(void *, long)) chunkfun;
  h->freefun = (void (*) (void *, struct _obstack_chunk *)) freefun;
  h->chunk_size = size;
  h->alignment_mask = alignment - 1;
  h->use_extra_arg = 0;

  chunk = h->chunk = CALL_CHUNKFUN (h, h -> chunk_size);
  if (!chunk)
    (*obstack_alloc_failed_handler) ();
  h->next_free = h->object_base = __PTR_ALIGN ((char *) chunk, chunk->contents,
					       alignment - 1);
  h->chunk_limit = chunk->limit
    = (char *) chunk + h->chunk_size;
  chunk->prev = 0;
  /* The initial chunk now contains no empty object.  */
  h->maybe_empty_object = 0;
  h->alloc_failed = 0;
  return 1;
}

#define obstack_init(h)						\
  _obstack_begin ((h), 0, 0,					\
		  (void *(*) (long)) obstack_chunk_alloc,	\
		  (void (*) (void *)) obstack_chunk_free)

void
xheader_init (struct xheader *xhdr)
{
  if (!xhdr->stk)
    {
      xhdr->stk = xmalloc (sizeof *xhdr->stk);
      obstack_init (xhdr->stk);
    }
}

void
xheader_store (char const *keyword, struct tar_stat_info *st,
	       void const *data)
{
  struct xhdr_tab const *t;

  if (st->xhdr.buffer)
    return;
  t = locate_handler (keyword);
  if (!t || !t->coder)
    return;
  if (xheader_keyword_deleted_p (keyword)
      || xheader_keyword_override_p (keyword))
    return;
  xheader_init (&st->xhdr);
  t->coder (st, keyword, &st->xhdr, data);
}

void
xheader_read (struct xheader *xhdr, union block *p, size_t size)
{
  size_t j = 0;

  xheader_init (xhdr);
  size += BLOCKSIZE;
  xhdr->size = size;
  xhdr->buffer = xmalloc (size + 1);
  xhdr->buffer[size] = '\0';

  do
    {
      size_t len = size;

      if (len > BLOCKSIZE)
	len = BLOCKSIZE;

      memcpy (&xhdr->buffer[j], p->buffer, len);
      set_next_block_after (p);

      p = find_next_block ();

      j += len;
      size -= len;
    }
  while (size > 0);
}


enum read_header
read_header_primitive (bool raw_extended_headers, struct tar_stat_info *info)
{
  union block *header;
  union block *header_copy;
  char *bp;
  union block *data_block;
  size_t size, written;
  union block *next_long_name = 0;
  union block *next_long_link = 0;
  size_t next_long_name_blocks = 0;
  size_t next_long_link_blocks = 0;

  while (1)
    {
      enum read_header status;

      header = find_next_block ();
      current_header = header;
      if (!header)
	return HEADER_END_OF_FILE;

      if ((status = tar_checksum (header, false)) != HEADER_SUCCESS)
	return status;

      /* Good block.  Decode file size and return.  */

      if (header->header.typeflag == LNKTYPE)
	info->stat.st_size = 0;	/* links 0 size on tape */
      else
	info->stat.st_size = OFF_FROM_HEADER (header->header.size);

      if (header->header.typeflag == GNUTYPE_LONGNAME
	  || header->header.typeflag == GNUTYPE_LONGLINK
	  || header->header.typeflag == XHDTYPE
	  || header->header.typeflag == XGLTYPE
	  || header->header.typeflag == SOLARIS_XHDTYPE)
	{
	  if (raw_extended_headers)
	    return HEADER_SUCCESS_EXTENDED;
	  else if (header->header.typeflag == GNUTYPE_LONGNAME
		   || header->header.typeflag == GNUTYPE_LONGLINK)
	    {
	      size_t name_size = info->stat.st_size;
	      size_t n = name_size % BLOCKSIZE;
	      size = name_size + BLOCKSIZE;
	      if (n)
		size += BLOCKSIZE - n;

	      if (name_size != info->stat.st_size || size < name_size)
		xalloc_die ();

	      header_copy = xmalloc (size + 1);

	      if (header->header.typeflag == GNUTYPE_LONGNAME)
		{
		  if (next_long_name)
		    free (next_long_name);
		  next_long_name = header_copy;
		  next_long_name_blocks = size / BLOCKSIZE;
		}
	      else
		{
		  if (next_long_link)
		    free (next_long_link);
		  next_long_link = header_copy;
		  next_long_link_blocks = size / BLOCKSIZE;
		}

	      set_next_block_after (header);
	      *header_copy = *header;
	      bp = header_copy->buffer + BLOCKSIZE;

	      for (size -= BLOCKSIZE; size > 0; size -= written)
		{
		  data_block = find_next_block ();
		  if (! data_block)
		    {
		      TARERROR ((0, 0, _("Unexpected EOF in archive")));
		      break;
		    }
		  written = available_space_after (data_block);
		  if (written > size)
		    written = size;

		  memcpy (bp, data_block->buffer, written);
		  bp += written;
		  set_next_block_after ((union block *)
					(data_block->buffer + written - 1));
		}

	      *bp = '\0';
	    }
	  else if (header->header.typeflag == XHDTYPE
		   || header->header.typeflag == SOLARIS_XHDTYPE)
	    xheader_read (&info->xhdr, header,
			  OFF_FROM_HEADER (header->header.size));
	  else if (header->header.typeflag == XGLTYPE)
	    {
	      struct xheader xhdr;
	      memset (&xhdr, 0, sizeof xhdr);
	      xheader_read (&xhdr, header,
			    OFF_FROM_HEADER (header->header.size));
	      xheader_decode_global (&xhdr);
	      xheader_destroy (&xhdr);
	    }

	  /* Loop!  */

	}
      else
	{
	  char const *name;
	  struct posix_header const *h = &current_header->header;
	  char namebuf[sizeof h->prefix + 1 + NAME_FIELD_SIZE + 1];

	  if (recent_long_name)
	    free (recent_long_name);

	  if (next_long_name)
	    {
	      name = next_long_name->buffer + BLOCKSIZE;
	      recent_long_name = next_long_name;
	      recent_long_name_blocks = next_long_name_blocks;
	    }
	  else
	    {
	      /* Accept file names as specified by POSIX.1-1996
                 section 10.1.1.  */
	      char *np = namebuf;

	      if (h->prefix[0] && strcmp (h->magic, TMAGIC) == 0)
		{
		  memcpy (np, h->prefix, sizeof h->prefix);
		  np[sizeof h->prefix] = '\0';
		  np += strlen (np);
		  *np++ = '/';
		}
	      memcpy (np, h->name, sizeof h->name);
	      np[sizeof h->name] = '\0';
	      name = namebuf;
	      recent_long_name = 0;
	      recent_long_name_blocks = 0;
	    }
	  assign_string (&info->orig_file_name, name);
	  assign_string (&info->file_name, name);
	  info->had_trailing_slash = strip_trailing_slashes (info->file_name);

	  if (recent_long_link)
	    free (recent_long_link);

	  if (next_long_link)
	    {
	      name = next_long_link->buffer + BLOCKSIZE;
	      recent_long_link = next_long_link;
	      recent_long_link_blocks = next_long_link_blocks;
	    }
	  else
	    {
	      memcpy (namebuf, h->linkname, sizeof h->linkname);
	      namebuf[sizeof h->linkname] = '\0';
	      name = namebuf;
	      recent_long_link = 0;
	      recent_long_link_blocks = 0;
	    }
	  assign_string (&info->link_name, name);

	  return HEADER_SUCCESS;
	}
    }
}

enum read_header
read_header_primitive_NO (bool raw_extended_headers, struct tar_stat_info * info)
{

   return HEADER_FAILURE;
}

enum read_header
read_header (bool raw_extended_headers)
{
  return read_header_primitive (raw_extended_headers, &current_stat_info);
}


BOOL IsTARHeader( void * ptr )
{
   BOOL bRet = FALSE;
   union block * pblk = (union block *)ptr;
   current_header = pblk;

   return bRet;
}

void
xheader_destroy (struct xheader *xhdr)
{
   if (xhdr->stk)
      free (xhdr->stk);
      //obstack_free (xhdr->stk, NULL);
   xhdr->stk = NULL;
   if(xhdr->buffer)
      free (xhdr->buffer);
   xhdr->buffer = 0;
   xhdr->size = 0;
}

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

void *
page_aligned_alloc (void **ptr, size_t size)
{
  //size_t alignment = getpagesize ();
  //size_t size1 = size + alignment;
  //if (size1 < size)
  //  xalloc_die ();
  //*ptr = xmalloc (size1);
  //return ptr_align (*ptr, alignment);
  *ptr = malloc (size);
  return (*ptr);
}

static void
init_buffer ()
{
  if (! record_buffer_aligned[record_index])
    record_buffer_aligned[record_index] =
      page_aligned_alloc (&record_buffer[record_index], record_size);

  record_start = record_buffer_aligned[record_index];
  current_block = record_start;
  record_end = record_start + blocking_factor;
}

void first_read( void )
{
   memset (&current_stat_info, 0, sizeof (current_stat_info));
   tar_stat_destroy (&current_stat_info);
   init_buffer();
   base64_init();
   record_end = current_block;   // set to start READ (actually COPY)
   find_next_block();   // this copies the FIRST block into the buffer
   // and does the check sum
}

BOOL  ProcessTAR( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;

   active_lpdf = lpdf;
   archive_format = DEFAULT_FORMAT;
   blocking_factor = DEFAULT_BLOCKING;
   record_size = DEFAULT_BLOCKING * BLOCKSIZE;
   record_index = 0;
   records_read = 0;
   lpdf->df_dwOff = 0;  /* start of buffer */
   first_read();
   if( IsTARFile( lpdf, lpdf->fn, lpdf->lpb, lpdf->dwmax ) )
   {
      bRet = DumpTAR( lpdf );
   }
   return bRet;
}


// eof - DumpTar.c
