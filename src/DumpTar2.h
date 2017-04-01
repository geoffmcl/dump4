
// DumpTar2.h
#ifndef _DumpTar2_h_
#define _DumpTar2_h_

#define false FALSE
#define true TRUE

#define mode_t int
#define gid_t short
#define uid_t short
/* Type of major device numbers. */
#define major_t   int
/* Type of minor device numbers. */
#define minor_t   int

/* tar files are made in basic blocks of this size.  */
#define BLOCKSIZE 512
/* Some constants from POSIX are given names.  */
#define NAME_FIELD_SIZE   100
#define PREFIX_FIELD_SIZE 155
#define UNAME_FIELD_SIZE   32
#define GNAME_FIELD_SIZE   32

/* ********************************************
   use 64-bit seek and stat
   ******************************************** */
#ifdef _WIN32
#define struct_stat struct __stat64
#else
#define struct_stat struct stat
#endif

#define  FDSEEK   _lseeki64
#define  FNSTAT   _stat64
#define  FDSTAT   _fstat64
#define  lstat    FNSTAT

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


// some items culled from tar 1.20 source
/* ========================
   Headers

   ALSO PUT LOTS OF SCRAPS FROM TAR 1.20 INTO DumpTar-scraps.c - 2008-0824
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
#ifdef _WIN32
#ifndef uintmax_t
typedef unsigned __int64 uintmax_t;
#endif
#ifndef intmax_t
typedef __int64 intmax_t;
#endif
#ifdef USE_UNSIGNED_64BIT
typedef uintmax_t off64_t;
typedef uintmax_t size64_t;
#else /* !USE_UNSIGNED_64BIT */
typedef __int64 off64_t;
typedef __int64 size64_t;
#endif /* USE_UNSIGNED_64BIT y/n */
typedef __int64 inttype;
#else // !_WIN32
#include <inttypes.h>
typedef size_t off64_t;
typedef size_t size64_t;
typedef size_t inttype;
#endif // _WIN32 y/n

#if (defined(_MSC_VER) && (_MSC_VER < 1900))
#ifndef _TIMESPEC           
#define _TIMESPEC
struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* and nanoseconds */  
};
#endif
#endif // for OLDER MSVC

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

///////////////////////////////////////////////////////////////////
#ifdef _WIN32

/* from linux stat.h
#if defined(__KERNEL__) || !defined(__GLIBC__) || (__GLIBC__ < 2)
#define S_IFMT  00170000

#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001
#endif
 *************************** */
// from sys/stat.h
//#define _S_IFMT         0xF000          /* file type mask */
//#define _S_IFDIR        0x4000          /* directory */
//#define _S_IFCHR        0x2000          /* character special */
//#define _S_IFIFO        0x1000          /* pipe */
//#define _S_IFREG        0x8000          /* regular */
//#define _S_IREAD        0x0100          /* read permission, owner */
//#define _S_IWRITE       0x0080          /* write permission, owner */
//#define _S_IEXEC        0x0040          /* execute/search permission, owner */

#define  S_ISLNK(m)	0
#define  S_ISREG(m)	(((m) & _S_IFMT) == _S_IFREG)
#define  S_ISDIR(m)  (((m) & _S_IFMT) == _S_IFDIR)
#define  S_ISCHR(m)	(((m) & _S_IFMT) == _S_IFCHR)
#define  S_ISBLK(m)	0
#define  S_ISFIFO(m)	(((m) & _S_IFMT) == _S_IFIFO)
#define  S_ISSOCK(m)	0

#define S_IFSOCK 0
#define S_IFLNK  0
//#define S_IFREG  0100000
#define S_IFBLK  0
//#define S_IFDIR  0040000
//#define S_IFCHR  0020000
//#define S_IFIFO  0010000
#define S_ISUID  0
#define S_ISGID  0
#define S_ISVTX  0

#define S_IRUSR _S_IREAD
#define S_IRGRP S_IRUSR
#define S_IROTH S_IRUSR

#define S_IWUSR _S_IWRITE
#define S_IWGRP S_IWUSR
#define S_IWOTH S_IWUSR

#define S_IXUSR _S_IEXEC
#define S_IXGRP S_IXUSR
#define S_IXOTH S_IXUSR
#define S_IXUGO S_IXUSR

#define S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define S_IRWXG S_IRWXU
#define S_IRWXO S_IRWXU

#define  S_IRWXUGO   S_IRWXU
///////////////////////////////////////////////////////////////////
#endif // #ifdef _WIN32

/* Information about a sparse file.  */
struct sp_array
{
  off64_t offset;
  size64_t numbytes;
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
  struct_stat   stat;       /* regular filesystem stat */

  /* STAT doesn't always have access, data modification, and status
     change times in a convenient form, so store them separately.  */
  struct timespec atime;
  struct timespec mtime;
  struct timespec ctime;

  off64_t archive_file_size;  /* Size of file as stored in the archive.
			       Equals stat.st_size for non-sparse files */

  bool   is_sparse;         /* Is the file sparse */

  /* For sparse files: */
  unsigned sparse_major;
  unsigned sparse_minor;
  size64_t sparse_map_avail;  /* Index to the first unused element in
			       sparse_map array. Zero if the file is
			       not sparse */
  size64_t sparse_map_size;   /* Size of the sparse map */
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

enum read_header
{
  HEADER_STILL_UNREAD,		/* for when read_header has not been called */
  HEADER_SUCCESS,		/* header successfully read and checksummed */
  HEADER_SUCCESS_EXTENDED,	/* likewise, but we got an extended header */
  HEADER_ZERO_BLOCK,		/* zero block where header expected */
  HEADER_END_OF_FILE,		/* true end of file while header expected */
  HEADER_FAILURE		/* ill-formed header, or bad checksum */
};

#define OFF_FROM_HEADER(a) off_from_header (a, sizeof (a))
#define SIZE_FROM_HEADER(wher) size_from_header (wher, sizeof (wher))
#define OFF_TO_CHARS(val, wher) off_to_chars (val, wher, sizeof (wher))
#define SIZE_TO_CHARS(val, wher) size_to_chars (val, wher, sizeof (wher))
#define MODE_TO_CHARS(val, where) mode_to_chars (val, where, sizeof (where))

extern off64_t off_from_header (const char *p, size_t s);
extern size64_t size_from_header (const char *p, size_t s);
extern bool off_to_chars (off64_t v, char *p, size64_t s);
extern bool size_to_chars (size64_t v, char *p, size64_t s);
extern bool mode_to_chars (mode_t v, char *p, size_t s);
extern bool uid_to_chars (uid_t v, char *p, size_t s);

// *****************************************************
// sparse stuff
// *****************************************************

struct tar_sparse_file
{
  int fd;                           /* File descriptor */
  bool seekable;                    /* Is fd seekable? */
  off64_t offset;                     /* Current offset in fd if seekable==false.
				       Otherwise unused */
  off64_t dumped_size;                /* Number of bytes actually written
				       to the archive */
  struct tar_stat_info *stat_info;  /* Information about the file */
  struct tar_sparse_optab const *optab; /* Operation table */
  void *closure;                    /* Any additional data optab calls might
				       require */
};

struct tar_sparse_optab
{
    bool(*init) (struct tar_sparse_file *);
    bool(*done) (struct tar_sparse_file *);
    bool(*sparse_member_p) (struct tar_sparse_file *);
    bool(*dump_header) (struct tar_sparse_file *);
    bool(*fixup_header) (struct tar_sparse_file *);
    bool(*decode_header) (struct tar_sparse_file *);
    bool(*scan_block) (struct tar_sparse_file *, enum sparse_scan_state,
        void *);
    bool(*dump_region) (struct tar_sparse_file *, size_t);
    bool(*extract_region) (struct tar_sparse_file *, size_t);
};

enum dump_status
  {
    dump_status_ok,
    dump_status_short,
    dump_status_fail,
    dump_status_not_implemented
  };

# define signed_type_or_expr__(t) 1

/* Bound on length of the string representing an integer type or expression T.
   Subtract 1 for the sign bit if T is signed; log10 (2.0) < 146/485;
   add 1 for integer division truncation; add 1 more for a minus sign
   if needed.  */
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - signed_type_or_expr__ (t)) * 146 / 485 \
   + signed_type_or_expr__ (t) + 1)

/* Bound on buffer size needed to represent an integer type or expression T,
   including the terminating null.  */
#define INT_BUFSIZE_BOUND(t) (INT_STRLEN_BOUND (t) + 1)

#define UINTMAX_STRSIZE_BOUND INT_BUFSIZE_BOUND (uintmax_t)

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

#define strtoumax _strtoui64
#define strtoimax _strtoi64
#define __attribute__(a)
# ifndef obstack_chunk_alloc
#  define obstack_chunk_alloc malloc
# endif
# ifndef obstack_chunk_free
#  define obstack_chunk_free free
# endif

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

#define ISOCTAL(c) ((c)>='0'&&(c)<='7')

# define ISSLASH(C) ((C) == '/' || (C) == '\\')

/* =================================================================
   some fill ins for windows
   ================================================================= */

#ifndef passwd
/* The passwd structure.  */
struct passwd
{
  char *pw_name;		/* Username.  */
  char *pw_passwd;		/* Password.  */
  uid_t pw_uid;		/* User ID.  */
  gid_t pw_gid;		/* Group ID.  */
  char *pw_gecos;		/* Real name.  */
  char *pw_dir;			/* Home directory.  */
  char *pw_shell;		/* Shell program.  */
};

#endif /* passwd */

#ifndef group
/* The group structure.	 */
struct group
  {
    char *gr_name;		/* Group name.	*/
    char *gr_passwd;		/* Password.	*/
    gid_t gr_gid;		/* Group ID.	*/
    char **gr_mem;		/* Member list.	*/
  };

#endif /* group */

extern struct passwd *getpwuid (uid_t uid);

/* The checksum field is filled with this while the checksum is computed.  */
#define CHKBLANKS	"        "	/* 8 blanks, no null */

//#ifndef GOT_MAJOR
# define major(device)		(((device) >> 8) & 0xff)
# define minor(device)		((device) & 0xff)
# define makedev(major, minor)	(((major) << 8) | (minor))
// #endif

#include "DumpTarX.h"
#include "DumpTarOb.h"

#endif // _DumpTar2_h_
// eof - DumpTar2.h


