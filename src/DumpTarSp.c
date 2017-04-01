
// DumpTarSp.c

// spare file handling ...

#pragma warning(disable:4146) // C4146: unary minus operator applied to unsigned type, result still unsigned
#include	"Dump4.h"   // 20170330 - Fix cse for unix
#include <sys/types.h>
#include <sys/stat.h>

#include "DumpTar2.h"
#include <math.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <time.h>

#ifndef _WIN32
typedef __int64 ssize_t;
#endif

#define MODE_WXUSR	(S_IWUSR | S_IXUSR)
#define MODE_R		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_RW		(S_IWUSR | S_IWGRP | S_IWOTH | MODE_R)
#define MODE_RWX	(S_IXUSR | S_IXGRP | S_IXOTH | MODE_RW)
#define MODE_ALL	(S_ISUID | S_ISGID | S_ISVTX | MODE_RWX)

extern char * umaxtostr (inttype i, char *buf);
extern enum archive_format current_format;
extern union block * find_next_block( void );
extern void mv_size_left (off64_t size);
extern void set_next_block_after (union block *block);
extern void mv_begin (struct tar_stat_info *st);
extern void mv_end (void);
extern void skip_file (off64_t size);
extern union block *current_header;
extern off_t current_block_ordinal (void);
extern bool string_ascii_p (char const *p);
extern void xheader_store (char const *keyword, struct tar_stat_info *st,
	       void const *data);
extern bool time_to_chars (time_t v, char *p, size_t s);
extern uid_t cached_no_such_uid;
extern gid_t cached_no_such_gid;
extern char * xstrdup (char const *string);
extern char *cached_uname;
extern char *cached_gname;
extern uid_t cached_uid;	/* valid only if cached_uname is not empty */
extern gid_t cached_gid;	/* valid only if cached_gname is not empty */
extern void assign_string (char **string, const char *value);
extern struct group *getgrgid (gid_t gid);
extern int verbose_option;
extern void print_header (struct tar_stat_info *st, off_t block_ordinal);
extern void xheader_finish (struct xheader *xhdr);
extern char * xheader_ghdr_name (void);
extern char * xheader_xhdr_name (struct tar_stat_info *st);
extern void xheader_write (char type, char *name, struct xheader *xhdr);
extern bool uintmax_to_chars (uintmax_t v, char *p, size_t s);
extern void * xcalloc (size_t n, size_t s);
extern size_t available_space_after (union block *pointer);
extern int incremental_option;
extern int numeric_owner_option;
extern bool xheader_keyword_deleted_p (const char *kw);
extern char * xheader_format_name (struct tar_stat_info *st, const char *fmt, size_t n);
extern void xheader_string_begin (struct xheader *xhdr);
extern void xheader_string_add (struct xheader *xhdr, char const *s);
extern bool xheader_string_end (struct xheader *xhdr, char const *keyword);

/* If true, override actual mtime (see below) */
bool set_mtime_option = 0;
struct timespec mtime_option = {0};

/* Functions for dealing with sparse files

   Copyright (C) 2003, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.

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

//#include <system.h>
//#include <inttostr.h>
//#include <quotearg.h>
//#include "common.h"

unsigned tar_sparse_major = 0;
unsigned tar_sparse_minor = 0;
/* Specified value to be put into tar file in place of stat () results, or
   just -1 if such an override should not take place.  */
uid_t owner_option = -1;
gid_t group_option = -1;

/* Initial umask, if needed for mode change string.  */
mode_t initial_umask = 0;

/* Description of a mode change.  */
struct mode_change
{
  char op;			/* One of "=+-".  */
  char flag;			/* Special operations flag.  */
  mode_t affected;		/* Set for u, g, o, or a.  */
  mode_t value;			/* Bits to add/remove.  */
  mode_t mentioned;		/* Bits explicitly mentioned.  */
};

/* Specified mode change string.  */
struct mode_change *mode_option = 0;

struct tar_sparse_file;
static bool sparse_select_optab (struct tar_sparse_file *file);

enum sparse_scan_state
  {
    scan_begin,
    scan_block,
    scan_end
  };

#if 0 // 0 ************************************************
struct tar_sparse_optab
{
  bool (*init) (struct tar_sparse_file *);
  bool (*done) (struct tar_sparse_file *);
  bool (*sparse_member_p) (struct tar_sparse_file *);
  bool (*dump_header) (struct tar_sparse_file *);
  bool (*fixup_header) (struct tar_sparse_file *);
  bool (*decode_header) (struct tar_sparse_file *);
  bool (*scan_block) (struct tar_sparse_file *, enum sparse_scan_state,
		      void *);
  bool (*dump_region) (struct tar_sparse_file *, size_t);
  bool (*extract_region) (struct tar_sparse_file *, size_t);
};

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

/* Dump zeros to file->fd until offset is reached. It is used instead of
   lseek if the output file is not seekable */
static bool
dump_zeros (struct tar_sparse_file *file, off64_t offset)
{
  static char const zero_buf[BLOCKSIZE];

  if (offset < file->offset)
    {
      errno = EINVAL;
      return false;
    }

  while (file->offset < offset)
    {
      size_t size = (size_t)(BLOCKSIZE < offset - file->offset
		     ? BLOCKSIZE
		     : offset - file->offset);
      ssize_t wrbytes;

      wrbytes = write (file->fd, zero_buf, size);
      if (wrbytes <= 0)
	{
	  if (wrbytes == 0)
	    errno = EINVAL;
	  return false;
	}
      file->offset += wrbytes;
    }

  return true;
}
#endif // 0 **********************************************

static bool
tar_sparse_member_p (struct tar_sparse_file *file)
{
  if (file->optab->sparse_member_p)
    return file->optab->sparse_member_p (file);
  return false;
}

static bool
tar_sparse_init (struct tar_sparse_file *file)
{
  memset (file, 0, sizeof *file);

  if (!sparse_select_optab (file))
    return false;

  if (file->optab->init)
    return file->optab->init (file);

  return true;
}

static bool
tar_sparse_done (struct tar_sparse_file *file)
{
  if (file->optab->done)
    return file->optab->done (file);
  return true;
}

static bool
tar_sparse_scan (struct tar_sparse_file *file, enum sparse_scan_state state,
		 void *block)
{
  if (file->optab->scan_block)
    return file->optab->scan_block (file, state, block);
  return true;
}

static bool
tar_sparse_dump_region (struct tar_sparse_file *file, size_t i)
{
  if (file->optab->dump_region)
    return file->optab->dump_region (file, i);
  return false;
}

static bool
tar_sparse_extract_region (struct tar_sparse_file *file, size_t i)
{
  if (file->optab->extract_region)
    return file->optab->extract_region (file, i);
  return false;
}

static bool
tar_sparse_dump_header (struct tar_sparse_file *file)
{
  if (file->optab->dump_header)
    return file->optab->dump_header (file);
  return false;
}

static bool
tar_sparse_decode_header (struct tar_sparse_file *file)
{
  if (file->optab->decode_header)
    return file->optab->decode_header (file);
  return true;
}

static bool
tar_sparse_fixup_header (struct tar_sparse_file *file)
{
  if (file->optab->fixup_header)
    return file->optab->fixup_header (file);
  return true;
}


static bool
lseek_or_error (struct tar_sparse_file *file, off64_t offset)
{
#ifdef _MSC_VER
   if( FDSEEK (file->fd, offset, SEEK_SET) == -1LL )
   {
      //seek_diag_details (file->stat_info->orig_file_name, offset);
      sprtf( "ERROR: Unable to seek to %I64u in %s!\n", offset, file->stat_info->orig_file_name);
      return false;
   }
#else /* !_MSC_VER */
  if (file->seekable
      ? FDSEEK (file->fd, offset, SEEK_SET) < 0
      : ! dump_zeros (file, offset))
    {
      seek_diag_details (file->stat_info->orig_file_name, offset);
      return false;
    }
#endif /* _MSC_VER y/n */
  return true;
}

/* Takes a blockful of data and basically cruises through it to see if
   it's made *entirely* of zeros, returning a 0 the instant it finds
   something that is a nonzero, i.e., useful data.  */
static bool
zero_block_p (char const *buffer, size_t size)
{
  while (size--)
    if (*buffer++)
      return false;
  return true;
}

static void
sparse_add_map (struct tar_stat_info *st, struct sp_array const *sp)
{
  struct sp_array *sparse_map = st->sparse_map;
  size64_t avail = st->sparse_map_avail;
  if (avail == st->sparse_map_size)
  {
     sprtf( "ERROR: NOT IMPLEMENTED! Aborting!!\n" );
    //st->sparse_map = sparse_map =
    //  x2nrealloc (sparse_map, &st->sparse_map_size, sizeof *sparse_map);
     pgm_exit(2);
  }
  sparse_map[avail] = *sp;
  st->sparse_map_avail = avail + 1;
}

#define SAFE_READ_ERROR -1

size_t
safe_read (int fd, void const *buf, size_t count)
{
   size_t rd = read(fd, (void *)buf, count);
   if( rd != count )
      return SAFE_READ_ERROR;
   return rd;
}

/* Scan the sparse file and create its map */
static bool
sparse_scan_file (struct tar_sparse_file *file)
{
  struct tar_stat_info *st = file->stat_info;
  int fd = file->fd;
  char buffer[BLOCKSIZE];
  size_t count;
  off64_t offset = 0; /* this can be a BIG file ... ensure 64-bits */
  struct sp_array sp = {0, 0};

  if (!lseek_or_error (file, 0))
    return false;

  st->archive_file_size = 0;
  
  if (!tar_sparse_scan (file, scan_begin, NULL))
    return false;

  while ((count = safe_read (fd, buffer, sizeof buffer)) != 0
	 && count != SAFE_READ_ERROR)
    {
      /* Analyze the block.  */
      if (zero_block_p (buffer, count))
	{
	  if (sp.numbytes)
	    {
	      sparse_add_map (st, &sp);
	      sp.numbytes = 0;
	      if (!tar_sparse_scan (file, scan_block, NULL))
		return false;
	    }
	}
      else
	{
	  if (sp.numbytes == 0)
	    sp.offset = offset;
	  sp.numbytes += count;
	  st->archive_file_size += count;
	  if (!tar_sparse_scan (file, scan_block, buffer))
	    return false;
	}

      offset += count;
    }

  if (sp.numbytes == 0)
    sp.offset = offset;

  sparse_add_map (st, &sp);
  st->archive_file_size += count;
  return tar_sparse_scan (file, scan_end, NULL);
}

static struct tar_sparse_optab const oldgnu_optab;
static struct tar_sparse_optab const star_optab;
static struct tar_sparse_optab const pax_optab;

#ifndef DEFAULT_ARCHIVE_FORMAT
# define DEFAULT_ARCHIVE_FORMAT GNU_FORMAT
#endif
enum archive_format archive_format = DEFAULT_ARCHIVE_FORMAT;

static bool
sparse_select_optab (struct tar_sparse_file *file)
{
  switch (current_format == DEFAULT_FORMAT ? archive_format : current_format)
    {
    case V7_FORMAT:
    case USTAR_FORMAT:
      return false;

    case OLDGNU_FORMAT:
    case GNU_FORMAT: /*FIXME: This one should disappear? */
      file->optab = &oldgnu_optab;
      break;

    case POSIX_FORMAT:
      file->optab = &pax_optab;
      break;

    case STAR_FORMAT:
      file->optab = &star_optab;
      break;

    default:
      return false;
    }
  return true;
}

static bool
sparse_dump_region (struct tar_sparse_file *file, size_t i)
{
  union block *blk;
  off64_t bytes_left = file->stat_info->sparse_map[i].numbytes;

  if (!lseek_or_error (file, file->stat_info->sparse_map[i].offset))
    return false;

  while (bytes_left > 0)
    {
      size_t bufsize = (size_t)((bytes_left > BLOCKSIZE) ? BLOCKSIZE : bytes_left);
      size_t bytes_read;

      blk = find_next_block ();
      bytes_read = safe_read (file->fd, blk->buffer, bufsize);
      if (bytes_read == SAFE_READ_ERROR)
	{
       //   read_diag_details (file->stat_info->orig_file_name,
	    //                 (file->stat_info->sparse_map[i].offset
		 //	      + file->stat_info->sparse_map[i].numbytes
		 //	      - bytes_left),
		 //	     bufsize);
      sprtf( "ERROR: Read FAILED on %u bytes ...\n", bufsize );
	  return false;
	}

      memset (blk->buffer + bytes_read, 0, BLOCKSIZE - bytes_read);
      bytes_left -= bytes_read;
      file->dumped_size += bytes_read;
      mv_size_left (file->stat_info->archive_file_size - file->dumped_size);
      set_next_block_after (blk);
    }

  return true;
}

size_t full_write( int fd, void * buf, size_t size )
{
   size_t cnt = write( fd, buf, size );
   return cnt;
}

static bool
sparse_extract_region (struct tar_sparse_file *file, size_t i)
{
  size64_t write_size;

  if (!lseek_or_error (file, file->stat_info->sparse_map[i].offset))
    return false;

  write_size = file->stat_info->sparse_map[i].numbytes;

  if (write_size == 0)
    {
      /* Last block of the file is a hole */
      // **TBD*** if (file->seekable && sys_truncate (file->fd))
      // **TBD***	truncate_warn (file->stat_info->orig_file_name);
    }
  else while (write_size > 0)
    {
      size_t count;
      size_t wrbytes = (size_t)((write_size > BLOCKSIZE) ? BLOCKSIZE : write_size);
      union block *blk = find_next_block ();
      if (!blk)
	{
	  //ERROR ((0, 0, _("Unexpected EOF in archive")));
      sprtf( "ERROR: Unexpected EOF in archive!\n");
	  return false;
	}
      set_next_block_after (blk);
      count = full_write (file->fd, blk->buffer, wrbytes);
      write_size -= count;
      file->dumped_size += count;
      mv_size_left (file->stat_info->archive_file_size - file->dumped_size);
      file->offset += count;
      if (count != wrbytes)
	{
      //write_error_details (file->stat_info->orig_file_name,
		//	       count, wrbytes);
      sprtf( "ERROR: Failed to write %d bytes ...\n", wrbytes );
	  return false;
	}
    }
  return true;
}



void
pad_archive (off64_t size_left)
{
  union block *blk;
  while (size_left > 0)
    {
      mv_size_left (size_left);
      blk = find_next_block ();
      memset (blk->buffer, 0, BLOCKSIZE);
      set_next_block_after (blk);
      size_left -= BLOCKSIZE;
    }
}

/* Interface functions */
enum dump_status
sparse_dump_file (int fd, struct tar_stat_info *st)
{
  bool rc;
  struct tar_sparse_file file;

  if (!tar_sparse_init (&file))
    return dump_status_not_implemented;

  file.stat_info = st;
  file.fd = fd;
  file.seekable = true; /* File *must* be seekable for dump to work */

  rc = sparse_scan_file (&file);
  if (rc && file.optab->dump_region)
    {
      tar_sparse_dump_header (&file);

      if (fd >= 0)
	{
	  size_t i;

	  mv_begin (file.stat_info);
	  for (i = 0; rc && i < file.stat_info->sparse_map_avail; i++)
	    rc = tar_sparse_dump_region (&file, i);
	  mv_end ();
	}
    }

  pad_archive (file.stat_info->archive_file_size - file.dumped_size);
  return (tar_sparse_done (&file) && rc) ? dump_status_ok : dump_status_short;
}

bool
sparse_member_p (struct tar_stat_info *st)
{
  struct tar_sparse_file file;

  if (!tar_sparse_init (&file))
    return false;
  file.stat_info = st;
  return tar_sparse_member_p (&file);
}

bool
sparse_fixup_header (struct tar_stat_info *st)
{
  struct tar_sparse_file file;

  if (!tar_sparse_init (&file))
    return false;
  file.stat_info = st;
  return tar_sparse_fixup_header (&file);
}

enum dump_status
sparse_extract_file (int fd, struct tar_stat_info *st, off64_t *size)
{
  bool rc = true;
  struct tar_sparse_file file;
  size_t i;

  if (!tar_sparse_init (&file))
    return dump_status_not_implemented;

  file.stat_info = st;
  file.fd = fd;
  file.seekable = lseek (fd, 0, SEEK_SET) == 0;
  file.offset = 0;

  rc = tar_sparse_decode_header (&file);
  for (i = 0; rc && i < file.stat_info->sparse_map_avail; i++)
    rc = tar_sparse_extract_region (&file, i);
  *size = file.stat_info->archive_file_size - file.dumped_size;
  return (tar_sparse_done (&file) && rc) ? dump_status_ok : dump_status_short;
}

enum dump_status
sparse_skip_file (struct tar_stat_info *st)
{
  bool rc = true;
  struct tar_sparse_file file;

  if (!tar_sparse_init (&file))
    return dump_status_not_implemented;

  file.stat_info = st;
  file.fd = -1;

  rc = tar_sparse_decode_header (&file);
  skip_file (file.stat_info->archive_file_size - file.dumped_size);
  return (tar_sparse_done (&file) && rc) ? dump_status_ok : dump_status_short;
}


static bool
check_sparse_region (struct tar_sparse_file *file, off64_t beg, off64_t end)
{
  if (!lseek_or_error (file, beg))
    return false;

  while (beg < end)
    {
      size_t bytes_read;
      size_t rdsize = (size_t)(BLOCKSIZE < end - beg ? BLOCKSIZE : end - beg);
      char diff_buffer[BLOCKSIZE];

      bytes_read = safe_read (file->fd, diff_buffer, rdsize);
      if (bytes_read == SAFE_READ_ERROR)
	{
      sprtf( "ERROR: Failed in read of %d bytes ...\n", rdsize );
      //    read_diag_details (file->stat_info->orig_file_name,
	   //                  beg,
		//	     rdsize);
	  return false;
	}
      if (!zero_block_p (diff_buffer, bytes_read))
	{
	  //char begbuf[INT_BUFSIZE_BOUND (off_t)];
 	  //report_difference (file->stat_info,
	//		     _("File fragment at %s is not a hole"),
	//		     offtostr (beg, begbuf));
      sprtf( "WARNING: File fragment at is not a hole!\n" );
	  return false;
	}

      beg += bytes_read;
    }
  return true;
}

static bool
check_data_region (struct tar_sparse_file *file, size_t i)
{
  size64_t size_left;

  if (!lseek_or_error (file, file->stat_info->sparse_map[i].offset))
    return false;
  size_left = file->stat_info->sparse_map[i].numbytes;
  mv_size_left (file->stat_info->archive_file_size - file->dumped_size);
      
  while (size_left > 0)
    {
      size_t bytes_read;
      size_t rdsize = (size_t)((size_left > BLOCKSIZE) ? BLOCKSIZE : size_left);
      char diff_buffer[BLOCKSIZE];

      union block *blk = find_next_block ();
      if (!blk)
	{
	  //ERROR ((0, 0, _("Unexpected EOF in archive")));
      sprtf("ERROR: Unexpected EOF in archive!\n");
	  return false;
	}
      set_next_block_after (blk);
      bytes_read = safe_read (file->fd, diff_buffer, rdsize);
      if (bytes_read == SAFE_READ_ERROR)
	{
          //read_diag_details (file->stat_info->orig_file_name,
			 //    (file->stat_info->sparse_map[i].offset
			 //     + file->stat_info->sparse_map[i].numbytes
			 //     - size_left),
			 //    rdsize);
      sprtf( "WARNING: Failed on READ of %d bytes!\n", rdsize );
	  return false;
	}
      file->dumped_size += bytes_read;
      size_left -= bytes_read;
      mv_size_left (file->stat_info->archive_file_size - file->dumped_size);
      if (memcmp (blk->buffer, diff_buffer, rdsize))
	{
	  //report_difference (file->stat_info, _("Contents differ"));
      sprtf( "WARNING: Contents differ!\n");
	  return false;
	}
    }
  return true;
}

bool
sparse_diff_file (int fd, struct tar_stat_info *st)
{
  bool rc = true;
  struct tar_sparse_file file;
  size_t i;
  off64_t offset = 0;

  if (!tar_sparse_init (&file))
    return dump_status_not_implemented;

  file.stat_info = st;
  file.fd = fd;
  file.seekable = true; /* File *must* be seekable for compare to work */
  
  rc = tar_sparse_decode_header (&file);
  mv_begin (st);
  for (i = 0; rc && i < file.stat_info->sparse_map_avail; i++)
    {
      rc = check_sparse_region (&file,
				offset, file.stat_info->sparse_map[i].offset)
	    && check_data_region (&file, i);
      offset = file.stat_info->sparse_map[i].offset
	        + file.stat_info->sparse_map[i].numbytes;
    }

  if (!rc)
    skip_file (file.stat_info->archive_file_size - file.dumped_size);
  mv_end ();
  
  tar_sparse_done (&file);
  return rc;
}


/* Old GNU Format. The sparse file information is stored in the
   oldgnu_header in the following manner:

   The header is marked with type 'S'. Its `size' field contains
   the cumulative size of all non-empty blocks of the file. The
   actual file size is stored in `realsize' member of oldgnu_header.

   The map of the file is stored in a list of `struct sparse'.
   Each struct contains offset to the block of data and its
   size (both as octal numbers). The first file header contains
   at most 4 such structs (SPARSES_IN_OLDGNU_HEADER). If the map
   contains more structs, then the field `isextended' of the main
   header is set to 1 (binary) and the `struct sparse_header'
   header follows, containing at most 21 following structs
   (SPARSES_IN_SPARSE_HEADER). If more structs follow, `isextended'
   field of the extended header is set and next  next extension header
   follows, etc... */

enum oldgnu_add_status
  {
    add_ok,
    add_finish,
    add_fail
  };

static bool
oldgnu_sparse_member_p (struct tar_sparse_file *file __attribute__ ((unused)))
{
  return current_header->header.typeflag == GNUTYPE_SPARSE;
}

/* Add a sparse item to the sparse file and its obstack */
static enum oldgnu_add_status
oldgnu_add_sparse (struct tar_sparse_file *file, struct sparse *s)
{
  struct sp_array sp;

  if (s->numbytes[0] == '\0')
    return add_finish;
  sp.offset = OFF_FROM_HEADER (s->offset);
  sp.numbytes = SIZE_FROM_HEADER (s->numbytes);
  if (sp.offset < 0
      || file->stat_info->stat.st_size < sp.offset + sp.numbytes
      || file->stat_info->archive_file_size < 0)
    return add_fail;

  sparse_add_map (file->stat_info, &sp);
  return add_ok;
}

static bool
oldgnu_fixup_header (struct tar_sparse_file *file)
{
  /* NOTE! st_size was initialized from the header
     which actually contains archived size. The following fixes it */
  file->stat_info->archive_file_size = file->stat_info->stat.st_size;
  file->stat_info->stat.st_size =
    OFF_FROM_HEADER (current_header->oldgnu_header.realsize);
  return true;
}

/* Convert old GNU format sparse data to internal representation */
static bool
oldgnu_get_sparse_info (struct tar_sparse_file *file)
{
  size_t i;
  union block *h = current_header;
  int ext_p;
  enum oldgnu_add_status rc;

  file->stat_info->sparse_map_avail = 0;
  for (i = 0; i < SPARSES_IN_OLDGNU_HEADER; i++)
    {
      rc = oldgnu_add_sparse (file, &h->oldgnu_header.sp[i]);
      if (rc != add_ok)
	break;
    }

  for (ext_p = h->oldgnu_header.isextended;
       rc == add_ok && ext_p; ext_p = h->sparse_header.isextended)
    {
      h = find_next_block ();
      if (!h)
	{
	  //ERROR ((0, 0, _("Unexpected EOF in archive")));
      sprtf( "ERROR: Unexpected EOF in archive!\n");
	  return false;
	}
      set_next_block_after (h);
      for (i = 0; i < SPARSES_IN_SPARSE_HEADER && rc == add_ok; i++)
	rc = oldgnu_add_sparse (file, &h->sparse_header.sp[i]);
    }

  if (rc == add_fail)
    {
      //ERROR ((0, 0, _("%s: invalid sparse archive member"),
	   //   file->stat_info->orig_file_name));
       sprtf( "ERROR: %s: invalid sparse archive member!\n",
          file->stat_info->orig_file_name);
      return false;
    }
  return true;
}

static void
oldgnu_store_sparse_info (struct tar_sparse_file *file, size_t *pindex,
			  struct sparse *sp, size_t sparse_size)
{
  for (; *pindex < file->stat_info->sparse_map_avail
	 && sparse_size > 0; sparse_size--, sp++, ++*pindex)
    {
      OFF_TO_CHARS (file->stat_info->sparse_map[*pindex].offset,
		    sp->offset);
      SIZE_TO_CHARS (file->stat_info->sparse_map[*pindex].numbytes,
		     sp->numbytes);
    }
}

/* Copy at most LEN bytes from the string SRC to DST.  Terminate with
   NUL unless SRC is LEN or more bytes long.  */

static void
tar_copy_str (char *dst, const char *src, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    if (! (dst[i] = src[i]))
      break;
}

/* Same as tar_copy_str, but always terminate with NUL if using
   is OLDGNU format */

static void
tar_name_copy_str (char *dst, const char *src, size_t len)
{
  tar_copy_str (dst, src, len);
  if (archive_format == OLDGNU_FORMAT)
    dst[len-1] = 0;
}

/* Create a new header and store there at most NAME_FIELD_SIZE bytes of
   the file name */

static union block *
write_short_name (struct tar_stat_info *st)
{
  union block *header = find_next_block ();
  memset (header->buffer, 0, sizeof (union block));
  tar_name_copy_str (header->header.name, st->file_name, NAME_FIELD_SIZE);
  return header;
}

static size_t
split_long_name (const char *name, size_t length)
{
  size_t i;

  if (length > PREFIX_FIELD_SIZE)
    length = PREFIX_FIELD_SIZE + 1;
  for (i = length - 1; i > 0; i--)
    if (ISSLASH (name[i]))
      break;
  return i;
}

static union block *
write_ustar_long_name (const char *name)
{
  size_t length = strlen (name);
  size_t i;
  union block *header;

  if (length > PREFIX_FIELD_SIZE + NAME_FIELD_SIZE + 1)
    {
      //ERROR ((0, 0, _("%s: file name is too long (max %d); not dumped"),
	   //   quotearg_colon (name),
	   //   PREFIX_FIELD_SIZE + NAME_FIELD_SIZE + 1));
       sprtf( "ERROR: %s: file name is too long (max %d); not dumped!\n",
          name,
          PREFIX_FIELD_SIZE + NAME_FIELD_SIZE + 1);
      return NULL;
    }

  i = split_long_name (name, length);
  if (i == 0 || length - i - 1 > NAME_FIELD_SIZE)
    {
      //ERROR ((0, 0,
	   //   _("%s: file name is too long (cannot be split); not dumped"),
	   //   quotearg_colon (name)));
       sprtf( "ERROR: [%s](%d): file name is too long (cannot be split); not dumped!\n",
          name, strlen(name));
      return NULL;
    }

  header = find_next_block ();
  memset (header->buffer, 0, sizeof (header->buffer));
  memcpy (header->header.prefix, name, i);
  memcpy (header->header.name, name + i + 1, length - i - 1);

  return header;
}

#define TIME_TO_CHARS(val, where) time_to_chars (val, where, sizeof (where))
#define UID_TO_CHARS(val, where) uid_to_chars (val, where, sizeof (where))
#define UINTMAX_TO_CHARS(val, where) uintmax_to_chars (val, where, sizeof (where))
#define UNAME_TO_CHARS(name,buf) string_to_chars (name, buf, sizeof(buf))
#define GNAME_TO_CHARS(name,buf) string_to_chars (name, buf, sizeof(buf))
#define GID_TO_CHARS(val, where) gid_to_chars (val, where, sizeof (where))
extern bool gid_to_chars (gid_t v, char *p, size_t s);
#define MAJOR_TO_CHARS(val, where) major_to_chars (val, where, sizeof (where))
extern bool major_to_chars (major_t v, char *p, size_t s);
#define MINOR_TO_CHARS(val, where) minor_to_chars (val, where, sizeof (where))
extern bool minor_to_chars (minor_t v, char *p, size_t s);

#define  getgid   GetCurrentProcessId
#define  getuid   GetCurrentProcessId

/* Write a "private" header */
union block *
start_private_header (const char *name, size_t size)
{
  time_t t;
  union block *header = find_next_block ();

  memset (header->buffer, 0, sizeof (union block));

  tar_name_copy_str (header->header.name, name, NAME_FIELD_SIZE);
  OFF_TO_CHARS (size, header->header.size);

  time (&t);
  TIME_TO_CHARS (t, header->header.mtime);
  MODE_TO_CHARS (S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, header->header.mode);
  UID_TO_CHARS ((short)getuid (), header->header.uid);
  GID_TO_CHARS ((short)getgid (), header->header.gid);
  MAJOR_TO_CHARS (0, header->header.devmajor);
  MINOR_TO_CHARS (0, header->header.devminor);
  strncpy (header->header.magic, TMAGIC, TMAGLEN);
  strncpy (header->header.version, TVERSION, TVERSLEN);
  return header;
}

/* Given UID, find the corresponding UNAME.  */
void
uid_to_uname (uid_t uid, char **uname)
{
  struct passwd *passwd;

  if (uid != 0 && uid == cached_no_such_uid)
    {
      *uname = xstrdup ("");
      return;
    }

  if (!cached_uname || uid != cached_uid)
    {
      passwd = getpwuid (uid);
      if (passwd)
	{
	  cached_uid = uid;
	  assign_string (&cached_uname, passwd->pw_name);
	}
      else
	{
	  cached_no_such_uid = uid;
	  *uname = xstrdup ("");
	  return;
	}
    }
  *uname = xstrdup (cached_uname);
}

void
string_to_chars (char const *str, char *p, size_t s)
{
  tar_copy_str (p, str, s);
  p[s - 1] = '\0';
}

/* Given GID, find the corresponding GNAME.  */
void
gid_to_gname (gid_t gid, char **gname)
{
  struct group *group;

  if (gid != 0 && gid == cached_no_such_gid)
    {
      *gname = xstrdup ("");
      return;
    }

  if (!cached_gname || gid != cached_gid)
    {
      group = getgrgid (gid);
      if (group)
	{
	  cached_gid = gid;
	  assign_string (&cached_gname, group->gr_name);
	}
      else
	{
	  cached_no_such_gid = gid;
	  *gname = xstrdup ("");
	  return;
	}
    }
  *gname = xstrdup (cached_gname);
}

union block *
write_extended (bool global, struct tar_stat_info *st, union block *old_header)
{
  union block *header, hp;
  char *p;
  int type;

  if (st->xhdr.buffer || st->xhdr.stk == NULL)
    return old_header;

  xheader_finish (&st->xhdr);
  memcpy (hp.buffer, old_header, sizeof (hp));
  if (global)
    {
      type = XGLTYPE;
      p = xheader_ghdr_name ();
    }
  else
    {
      type = XHDTYPE;
      p = xheader_xhdr_name (st);
    }
  xheader_write (type, p, &st->xhdr);
  free (p);
  header = find_next_block ();
  memcpy (header, &hp.buffer, sizeof (hp.buffer));
  return header;
}

void
simple_finish_header (union block *header)
{
  size_t i;
  int sum;
  char *p;

  memcpy (header->header.chksum, CHKBLANKS, sizeof header->header.chksum);

  sum = 0;
  p = header->buffer;
  for (i = sizeof *header; i-- != 0; )
    /* We can't use unsigned char here because of old compilers, e.g. V7.  */
    sum += 0xFF & *p++;

  /* Fill in the checksum field.  It's formatted differently from the
     other fields: it has [6] digits, a null, then a space -- rather than
     digits, then a null.  We use to_chars.
     The final space is already there, from
     checksumming, and to_chars doesn't modify it.

     This is a fast way to do:

     sprintf(header->header.chksum, "%6o", sum);  */

  uintmax_to_chars ((uintmax_t) sum, header->header.chksum, 7);

  set_next_block_after (header);
}

/* Finish off a filled-in header block and write it out.  We also
   print the file name and/or full info if verbose is on.  If BLOCK_ORDINAL
   is not negative, is the block ordinal of the first record for this
   file, which may be a preceding long name or long link record.  */
void
finish_header (struct tar_stat_info *st,
	       union block *header, off_t block_ordinal)
{
  /* Note: It is important to do this before the call to write_extended(),
     so that the actual ustar header is printed */
  if (verbose_option
      && header->header.typeflag != GNUTYPE_LONGLINK
      && header->header.typeflag != GNUTYPE_LONGNAME
      && header->header.typeflag != XHDTYPE
      && header->header.typeflag != XGLTYPE)
    {
      /* These globals are parameters to print_header, sigh.  */

      current_header = header;
      current_format = archive_format;
      print_header (st, block_ordinal);
    }

  header = write_extended (false, st, header);
  simple_finish_header (header);
}


#define FILL(field,byte) do {            \
  memset(field, byte, sizeof(field)-1);  \
  (field)[sizeof(field)-1] = 0;          \
} while (0)

/* Write a GNUTYPE_LONGLINK or GNUTYPE_LONGNAME block.  */
static void
write_gnu_long_link (struct tar_stat_info *st, const char *p, char type)
{
  size_t size = strlen (p) + 1;
  size_t bufsize;
  union block *header;
  char *tmpname;

  header = start_private_header ("././@LongLink", size);
  FILL(header->header.mtime, '0');
  FILL(header->header.mode, '0');
  FILL(header->header.uid, '0');
  FILL(header->header.gid, '0');
  FILL(header->header.devmajor, 0);
  FILL(header->header.devminor, 0);
  uid_to_uname (0, &tmpname);
  UNAME_TO_CHARS (tmpname, header->header.uname);
  free (tmpname);
  gid_to_gname (0, &tmpname);
  GNAME_TO_CHARS (tmpname, header->header.gname);
  free (tmpname);

  strcpy (header->header.magic, OLDGNU_MAGIC);
  header->header.typeflag = type;
  finish_header (st, header, -1);

  header = find_next_block ();

  bufsize = available_space_after (header);

  while (bufsize < size)
    {
      memcpy (header->buffer, p, bufsize);
      p += bufsize;
      size -= bufsize;
      set_next_block_after (header + (bufsize - 1) / BLOCKSIZE);
      header = find_next_block ();
      bufsize = available_space_after (header);
    }
  memcpy (header->buffer, p, size);
  memset (header->buffer + size, 0, bufsize - size);
  set_next_block_after (header + (size - 1) / BLOCKSIZE);
}


static union block *
write_long_name (struct tar_stat_info *st)
{
  switch (archive_format)
    {
    case POSIX_FORMAT:
      xheader_store ("path", st, NULL);
      break;

    case V7_FORMAT:
      if (strlen (st->file_name) > NAME_FIELD_SIZE-1)
	{
	  //ERROR ((0, 0, _("%s: file name is too long (max %d); not dumped"),
	//	  quotearg_colon (st->file_name),
	//	  NAME_FIELD_SIZE - 1));
      sprtf("ERROR: %s: file name is too long (max %d); not dumped!\n",
         st->file_name,
         NAME_FIELD_SIZE - 1);
	  return NULL;
	}
      break;

    case USTAR_FORMAT:
    case STAR_FORMAT:
      return write_ustar_long_name (st->file_name);

    case OLDGNU_FORMAT:
    case GNU_FORMAT:
      write_gnu_long_link (st, st->file_name, GNUTYPE_LONGNAME);
      break;

    default:
      abort(); /*FIXME*/
    }
  return write_short_name (st);
}

static union block *
write_header_name (struct tar_stat_info *st)
{
  if (archive_format == POSIX_FORMAT && !string_ascii_p (st->file_name))
    {
      xheader_store ("path", st, NULL);
      return write_short_name (st);
    }
  else if ((size_t)(NAME_FIELD_SIZE - (archive_format == OLDGNU_FORMAT))
	   < strlen (st->file_name))
    return write_long_name (st);
  else
    return write_short_name (st);
}

#define CHMOD_MODE_BITS \
  (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)

/* Special operations flags.  */
enum
  {
    /* For the sentinel at the end of the mode changes array.  */
    MODE_DONE,

    /* The typical case.  */
    MODE_ORDINARY_CHANGE,

    /* In addition to the typical case, affect the execute bits if at
       least one execute bit is set already, or if the file is a
       directory.  */
    MODE_X_IF_ANY_X,

    /* Instead of the typical case, copy some existing permissions for
       u, g, or o onto the other two.  Which of u, g, or o is copied
       is determined by which bits are set in the `value' field.  */
    MODE_COPY_EXISTING
  };


mode_t
mode_adjust (mode_t oldmode, bool dir, mode_t umask_value,
	     struct mode_change const *changes, mode_t *pmode_bits)
{
  /* The adjusted mode.  */
  mode_t newmode = oldmode & CHMOD_MODE_BITS;

  /* File mode bits that CHANGES cares about.  */
  mode_t mode_bits = 0;

  for (; changes->flag != MODE_DONE; changes++)
    {
      mode_t affected = changes->affected;
      mode_t omit_change =
	(dir ? S_ISUID | S_ISGID : 0) & ~ changes->mentioned;
      mode_t value = changes->value;

      switch (changes->flag)
	{
	case MODE_ORDINARY_CHANGE:
	  break;

	case MODE_COPY_EXISTING:
	  /* Isolate in `value' the bits in `newmode' to copy.  */
	  value &= newmode;

	  /* Copy the isolated bits to the other two parts.  */
	  value |= ((value & (S_IRUSR | S_IRGRP | S_IROTH)
		     ? S_IRUSR | S_IRGRP | S_IROTH : 0)
		    | (value & (S_IWUSR | S_IWGRP | S_IWOTH)
		       ? S_IWUSR | S_IWGRP | S_IWOTH : 0)
		    | (value & (S_IXUSR | S_IXGRP | S_IXOTH)
		       ? S_IXUSR | S_IXGRP | S_IXOTH : 0));
	  break;

	case MODE_X_IF_ANY_X:
	  /* Affect the execute bits if execute bits are already set
	     or if the file is a directory.  */
	  if ((newmode & (S_IXUSR | S_IXGRP | S_IXOTH)) | dir)
	    value |= S_IXUSR | S_IXGRP | S_IXOTH;
	  break;
	}

      /* If WHO was specified, limit the change to the affected bits.
	 Otherwise, apply the umask.  Either way, omit changes as
	 requested.  */
      value &= (affected ? affected : ~umask_value) & ~ omit_change;

      switch (changes->op)
	{
	case '=':
	  /* If WHO was specified, preserve the previous values of
	     bits that are not affected by this change operation.
	     Otherwise, clear all the bits.  */
	  {
	    mode_t preserved = (affected ? ~affected : 0) | omit_change;
	    mode_bits |= CHMOD_MODE_BITS & ~preserved;
	    newmode = (newmode & preserved) | value;
	    break;
	  }

	case '+':
	  mode_bits |= value;
	  newmode |= value;
	  break;

	case '-':
	  mode_bits |= value;
	  newmode &= ~value;
	  break;
	}
    }

  if (pmode_bits)
    *pmode_bits = mode_bits;
  return newmode;
}

        /* The maximum uintmax_t value that can be represented with DIGITS digits,
   assuming that each digit is BITS_PER_DIGIT wide.  */
#define MAX_VAL_WITH_DIGITS(digits, bits_per_digit) \
   ((digits) * (bits_per_digit) < sizeof (uintmax_t) * CHAR_BIT \
    ? ((uintmax_t) 1 << ((digits) * (bits_per_digit))) - 1 \
    : (uintmax_t) -1)

/* The maximum uintmax_t value that can be represented with octal
   digits and a trailing NUL in BUFFER.  */
#define MAX_OCTAL_VAL(buffer) MAX_VAL_WITH_DIGITS (sizeof (buffer) - 1, LG_8)



union block *
start_header (struct tar_stat_info *st)
{
  union block *header;

  header = write_header_name (st);
  if (!header)
    return NULL;

  /* Override some stat fields, if requested to do so.  */

  if (owner_option != (uid_t) -1)
    st->stat.st_uid = owner_option;
  if (group_option != (gid_t) -1)
    st->stat.st_gid = group_option;
  if (mode_option)
    st->stat.st_mode =
      ((st->stat.st_mode & ~MODE_ALL)
       | mode_adjust (st->stat.st_mode, S_ISDIR (st->stat.st_mode) != 0,
		      initial_umask, mode_option, NULL));

  /* Paul Eggert tried the trivial test ($WRITER cf a b; $READER tvf a)
     for a few tars and came up with the following interoperability
     matrix:

	      WRITER
	1 2 3 4 5 6 7 8 9   READER
	. . . . . . . . .   1 = SunOS 4.2 tar
	# . . # # . . # #   2 = NEC SVR4.0.2 tar
	. . . # # . . # .   3 = Solaris 2.1 tar
	. . . . . . . . .   4 = GNU tar 1.11.1
	. . . . . . . . .   5 = HP-UX 8.07 tar
	. . . . . . . . .   6 = Ultrix 4.1
	. . . . . . . . .   7 = AIX 3.2
	. . . . . . . . .   8 = Hitachi HI-UX 1.03
	. . . . . . . . .   9 = Omron UNIOS-B 4.3BSD 1.60Beta

	     . = works
	     # = ``impossible file type''

     The following mask for old archive removes the `#'s in column 4
     above, thus making GNU tar both a universal donor and a universal
     acceptor for Paul's test.  */

  if (archive_format == V7_FORMAT || archive_format == USTAR_FORMAT)
    MODE_TO_CHARS (st->stat.st_mode & MODE_ALL, header->header.mode);
  else
    MODE_TO_CHARS (st->stat.st_mode, header->header.mode);

  {
    uid_t uid = st->stat.st_uid;
    if (archive_format == POSIX_FORMAT
	&& MAX_OCTAL_VAL (header->header.uid) < uid)
      {
	xheader_store ("uid", st, NULL);
	uid = 0;
      }
    if (!UID_TO_CHARS (uid, header->header.uid))
      return NULL;
  }

  {
    gid_t gid = st->stat.st_gid;
    if (archive_format == POSIX_FORMAT
	&& MAX_OCTAL_VAL (header->header.gid) < gid)
      {
	xheader_store ("gid", st, NULL);
	gid = 0;
      }
    if (!GID_TO_CHARS (gid, header->header.gid))
      return NULL;
  }

  {
    off64_t size = st->stat.st_size;
    if (archive_format == POSIX_FORMAT
	&& MAX_OCTAL_VAL (header->header.size) < size)
      {
	xheader_store ("size", st, NULL);
	size = 0;
      }
    if (!OFF_TO_CHARS ((off_t)size, header->header.size))
      return NULL;
  }

  {
    struct timespec mtime = set_mtime_option ? mtime_option : st->mtime;
    if (archive_format == POSIX_FORMAT)
      {
	if (MAX_OCTAL_VAL (header->header.mtime) < mtime.tv_sec
	    || mtime.tv_nsec != 0)
	  xheader_store ("mtime", st, &mtime);
	if (MAX_OCTAL_VAL (header->header.mtime) < mtime.tv_sec)
	  mtime.tv_sec = 0;
      }
    if (!TIME_TO_CHARS (mtime.tv_sec, header->header.mtime))
      return NULL;
  }

  /* FIXME */
  if (S_ISCHR (st->stat.st_mode)
      || S_ISBLK (st->stat.st_mode))
    {
      major_t devmajor = major (st->stat.st_rdev);
      minor_t devminor = minor (st->stat.st_rdev);

      if (archive_format == POSIX_FORMAT
	  && MAX_OCTAL_VAL (header->header.devmajor) < devmajor)
	{
	  xheader_store ("devmajor", st, NULL);
	  devmajor = 0;
	}
      if (!MAJOR_TO_CHARS (devmajor, header->header.devmajor))
	return NULL;

      if (archive_format == POSIX_FORMAT
	  && MAX_OCTAL_VAL (header->header.devminor) < devminor)
	{
	  xheader_store ("devminor", st, NULL);
	  devminor = 0;
	}
      if (!MINOR_TO_CHARS (devminor, header->header.devminor))
	return NULL;
    }
  else if (archive_format != GNU_FORMAT && archive_format != OLDGNU_FORMAT)
    {
      if (!(MAJOR_TO_CHARS (0, header->header.devmajor)
	    && MINOR_TO_CHARS (0, header->header.devminor)))
	return NULL;
    }

  if (archive_format == POSIX_FORMAT)
    {
      xheader_store ("atime", st, NULL);
      xheader_store ("ctime", st, NULL);
    }
  else if (incremental_option)
    if (archive_format == OLDGNU_FORMAT || archive_format == GNU_FORMAT)
      {
	TIME_TO_CHARS (st->atime.tv_sec, header->oldgnu_header.atime);
	TIME_TO_CHARS (st->ctime.tv_sec, header->oldgnu_header.ctime);
      }

  header->header.typeflag = archive_format == V7_FORMAT ? AREGTYPE : REGTYPE;

  switch (archive_format)
    {
    case V7_FORMAT:
      break;

    case OLDGNU_FORMAT:
    case GNU_FORMAT:   /*FIXME?*/
      /* Overwrite header->header.magic and header.version in one blow.  */
      strcpy (header->header.magic, OLDGNU_MAGIC);
      break;

    case POSIX_FORMAT:
    case USTAR_FORMAT:
      strncpy (header->header.magic, TMAGIC, TMAGLEN);
      strncpy (header->header.version, TVERSION, TVERSLEN);
      break;

    default:
      abort ();
    }

  if (archive_format == V7_FORMAT || numeric_owner_option)
    {
      /* header->header.[ug]name are left as the empty string.  */
    }
  else
    {
      uid_to_uname (st->stat.st_uid, &st->uname);
      gid_to_gname (st->stat.st_gid, &st->gname);

      if (archive_format == POSIX_FORMAT
	  && (strlen (st->uname) > UNAME_FIELD_SIZE
	      || !string_ascii_p (st->uname)))
	xheader_store ("uname", st, NULL);
      UNAME_TO_CHARS (st->uname, header->header.uname);

      if (archive_format == POSIX_FORMAT
	  && (strlen (st->gname) > GNAME_FIELD_SIZE
	      || !string_ascii_p (st->gname)))
	xheader_store ("gname", st, NULL);
      GNAME_TO_CHARS (st->gname, header->header.gname);
    }

  return header;
}

static bool
oldgnu_dump_header (struct tar_sparse_file *file)
{
  off_t block_ordinal = current_block_ordinal ();
  union block *blk;
  size_t i;

  blk = start_header (file->stat_info);
  blk->header.typeflag = GNUTYPE_SPARSE;
  if (file->stat_info->sparse_map_avail > SPARSES_IN_OLDGNU_HEADER)
    blk->oldgnu_header.isextended = 1;

  /* Store the real file size */
  OFF_TO_CHARS (file->stat_info->stat.st_size, blk->oldgnu_header.realsize);
  /* Store the effective (shrunken) file size */
  OFF_TO_CHARS (file->stat_info->archive_file_size, blk->header.size);

  i = 0;
  oldgnu_store_sparse_info (file, &i,
			    blk->oldgnu_header.sp,
			    SPARSES_IN_OLDGNU_HEADER);
  blk->oldgnu_header.isextended = i < file->stat_info->sparse_map_avail;
  finish_header (file->stat_info, blk, block_ordinal);

  while (i < file->stat_info->sparse_map_avail)
    {
      blk = find_next_block ();
      memset (blk->buffer, 0, BLOCKSIZE);
      oldgnu_store_sparse_info (file, &i,
				blk->sparse_header.sp,
				SPARSES_IN_SPARSE_HEADER);
      if (i < file->stat_info->sparse_map_avail)
	blk->sparse_header.isextended = 1;
      set_next_block_after (blk);
    }
  return true;
}

static struct tar_sparse_optab const oldgnu_optab = {
  NULL,  /* No init function */
  NULL,  /* No done function */
  oldgnu_sparse_member_p,
  oldgnu_dump_header,
  oldgnu_fixup_header,
  oldgnu_get_sparse_info,
  NULL,  /* No scan_block function */
  sparse_dump_region,
  sparse_extract_region,
};


/* Star */

static bool
star_sparse_member_p (struct tar_sparse_file *file __attribute__ ((unused)))
{
  return current_header->header.typeflag == GNUTYPE_SPARSE;
}

static bool
star_fixup_header (struct tar_sparse_file *file)
{
  /* NOTE! st_size was initialized from the header
     which actually contains archived size. The following fixes it */
  file->stat_info->archive_file_size = file->stat_info->stat.st_size;
  file->stat_info->stat.st_size =
            OFF_FROM_HEADER (current_header->star_in_header.realsize);
  return true;
}

/* Convert STAR format sparse data to internal representation */
static bool
star_get_sparse_info (struct tar_sparse_file *file)
{
  size_t i;
  union block *h = current_header;
  int ext_p;
  enum oldgnu_add_status rc = add_ok;

  file->stat_info->sparse_map_avail = 0;

  if (h->star_in_header.prefix[0] == '\0'
      && h->star_in_header.sp[0].offset[10] != '\0')
    {
      /* Old star format */
      for (i = 0; i < SPARSES_IN_STAR_HEADER; i++)
	{
	  rc = oldgnu_add_sparse (file, &h->star_in_header.sp[i]);
	  if (rc != add_ok)
	    break;
	}
      ext_p = h->star_in_header.isextended;
    }
  else
    ext_p = 1;

  for (; rc == add_ok && ext_p; ext_p = h->star_ext_header.isextended)
    {
      h = find_next_block ();
      if (!h)
	{
	  //ERROR ((0, 0, _("Unexpected EOF in archive")));
      sprtf( "ERROR: Unexpected EOF in archive!\n");
	  return false;
	}
      set_next_block_after (h);
      for (i = 0; i < SPARSES_IN_STAR_EXT_HEADER && rc == add_ok; i++)
	rc = oldgnu_add_sparse (file, &h->star_ext_header.sp[i]);
    }

  if (rc == add_fail)
    {
      //ERROR ((0, 0, _("%s: invalid sparse archive member"),
	   //   file->stat_info->orig_file_name));
       sprtf( "ERROR: %s: invalid sparse archive member!\n",
          file->stat_info->orig_file_name);
      return false;
    }
  return true;
}


static struct tar_sparse_optab const star_optab = {
  NULL,  /* No init function */
  NULL,  /* No done function */
  star_sparse_member_p,
  NULL,
  star_fixup_header,
  star_get_sparse_info,
  NULL,  /* No scan_block function */
  NULL, /* No dump region function */
  sparse_extract_region,
};


/* GNU PAX sparse file format. There are several versions:

   * 0.0

   The initial version of sparse format used by tar 1.14-1.15.1.
   The sparse file map is stored in x header:

   GNU.sparse.size      Real size of the stored file
   GNU.sparse.numblocks Number of blocks in the sparse map
   repeat numblocks time
     GNU.sparse.offset    Offset of the next data block
     GNU.sparse.numbytes  Size of the next data block
   end repeat

   This has been reported as conflicting with the POSIX specs. The reason is
   that offsets and sizes of non-zero data blocks were stored in multiple
   instances of GNU.sparse.offset/GNU.sparse.numbytes variables, whereas
   POSIX requires the latest occurrence of the variable to override all
   previous occurrences.
   
   To avoid this incompatibility two following versions were introduced.

   * 0.1

   Used by tar 1.15.2 -- 1.15.91 (alpha releases).
   
   The sparse file map is stored in
   x header:

   GNU.sparse.size      Real size of the stored file
   GNU.sparse.numblocks Number of blocks in the sparse map
   GNU.sparse.map       Map of non-null data chunks. A string consisting
                       of comma-separated values "offset,size[,offset,size]..."

   The resulting GNU.sparse.map string can be *very* long. While POSIX does not
   impose any limit on the length of a x header variable, this can confuse some
   tars.

   * 1.0

   Starting from this version, the exact sparse format version is specified
   explicitely in the header using the following variables:

   GNU.sparse.major     Major version 
   GNU.sparse.minor     Minor version

   X header keeps the following variables:
   
   GNU.sparse.name      Real file name of the sparse file
   GNU.sparse.realsize  Real size of the stored file (corresponds to the old
                        GNU.sparse.size variable)

   The name field of the ustar header is constructed using the pattern
   "%d/GNUSparseFile.%p/%f".
   
   The sparse map itself is stored in the file data block, preceding the actual
   file data. It consists of a series of octal numbers of arbitrary length,
   delimited by newlines. The map is padded with nulls to the nearest block
   boundary.

   The first number gives the number of entries in the map. Following are map
   entries, each one consisting of two numbers giving the offset and size of
   the data block it describes.

   The format is designed in such a way that non-posix aware tars and tars not
   supporting GNU.sparse.* keywords will extract each sparse file in its
   condensed form with the file map attached and will place it into a separate
   directory. Then, using a simple program it would be possible to expand the
   file to its original form even without GNU tar.

   Bu default, v.1.0 archives are created. To use other formats,
   --sparse-version option is provided. Additionally, v.0.0 can be obtained
   by deleting GNU.sparse.map from 0.1 format: --sparse-version 0.1
   --pax-option delete=GNU.sparse.map
*/

static bool
pax_sparse_member_p (struct tar_sparse_file *file)
{
  return file->stat_info->sparse_map_avail > 0
          || file->stat_info->sparse_major > 0;
}

static bool
pax_dump_header_0 (struct tar_sparse_file *file)
{
  off_t block_ordinal = current_block_ordinal ();
  union block *blk;
  size_t i;
  char nbuf[UINTMAX_STRSIZE_BOUND];
  struct sp_array *map = file->stat_info->sparse_map;
  char *save_file_name = NULL;
  
  /* Store the real file size */
  xheader_store ("GNU.sparse.size", file->stat_info, NULL);
  xheader_store ("GNU.sparse.numblocks", file->stat_info, NULL);
  
  if (xheader_keyword_deleted_p ("GNU.sparse.map")
      || tar_sparse_minor == 0)
    {
      for (i = 0; i < file->stat_info->sparse_map_avail; i++)
	{
	  xheader_store ("GNU.sparse.offset", file->stat_info, &i);
	  xheader_store ("GNU.sparse.numbytes", file->stat_info, &i);
	}
    }
  else
    {
      xheader_store ("GNU.sparse.name", file->stat_info, NULL);
      save_file_name = file->stat_info->file_name;
      file->stat_info->file_name = xheader_format_name (file->stat_info,
					       "%d/GNUSparseFile.%p/%f", 0);

      xheader_string_begin (&file->stat_info->xhdr);
      for (i = 0; i < file->stat_info->sparse_map_avail; i++)
	{
	  if (i)
	    xheader_string_add (&file->stat_info->xhdr, ",");
	  xheader_string_add (&file->stat_info->xhdr,
			      umaxtostr (map[i].offset, nbuf));
	  xheader_string_add (&file->stat_info->xhdr, ",");
	  xheader_string_add (&file->stat_info->xhdr,
			      umaxtostr (map[i].numbytes, nbuf));
	}
      if (!xheader_string_end (&file->stat_info->xhdr,
			       "GNU.sparse.map"))
	{
	  free (file->stat_info->file_name);
	  file->stat_info->file_name = save_file_name;
	  return false;
	}
    }
  blk = start_header (file->stat_info);
  /* Store the effective (shrunken) file size */
  OFF_TO_CHARS (file->stat_info->archive_file_size, blk->header.size);
  finish_header (file->stat_info, blk, block_ordinal);
  if (save_file_name)
    {
      free (file->stat_info->file_name);
      file->stat_info->file_name = save_file_name;
    }
  return true;
}

static bool
pax_dump_header_1 (struct tar_sparse_file *file)
{
  off_t block_ordinal = current_block_ordinal ();
  union block *blk;
  char *p, *q;
  size_t i;
  char nbuf[UINTMAX_STRSIZE_BOUND];
  off_t size = 0;
  struct sp_array *map = file->stat_info->sparse_map;
  char *save_file_name = file->stat_info->file_name;

#define COPY_STRING(b,dst,src) do                \
 {                                               \
   char *endp = b->buffer + BLOCKSIZE;           \
   char *srcp = src;                             \
   while (*srcp)                                 \
     {                                           \
       if (dst == endp)                          \
	 {                                       \
	   set_next_block_after (b);             \
	   b = find_next_block ();               \
           dst = b->buffer;                      \
	   endp = b->buffer + BLOCKSIZE;         \
	 }                                       \
       *dst++ = *srcp++;                         \
     }                                           \
   } while (0)                       

  /* Compute stored file size */
  p = umaxtostr (file->stat_info->sparse_map_avail, nbuf);
  size += strlen (p) + 1;
  for (i = 0; i < file->stat_info->sparse_map_avail; i++)
    {
      p = umaxtostr (map[i].offset, nbuf);
      size += strlen (p) + 1;
      p = umaxtostr (map[i].numbytes, nbuf);
      size += strlen (p) + 1;
    }
  size = (size + BLOCKSIZE - 1) / BLOCKSIZE;
  file->stat_info->archive_file_size += size * BLOCKSIZE;
  file->dumped_size += size * BLOCKSIZE;
  
  /* Store sparse file identification */
  xheader_store ("GNU.sparse.major", file->stat_info, NULL);
  xheader_store ("GNU.sparse.minor", file->stat_info, NULL);
  xheader_store ("GNU.sparse.name", file->stat_info, NULL);
  xheader_store ("GNU.sparse.realsize", file->stat_info, NULL);
  
  file->stat_info->file_name = xheader_format_name (file->stat_info,
					    "%d/GNUSparseFile.%p/%f", 0);

  blk = start_header (file->stat_info);
  /* Store the effective (shrunken) file size */
  OFF_TO_CHARS (file->stat_info->archive_file_size, blk->header.size);
  finish_header (file->stat_info, blk, block_ordinal);
  free (file->stat_info->file_name);
  file->stat_info->file_name = save_file_name;

  blk = find_next_block ();
  q = blk->buffer;
  p = umaxtostr (file->stat_info->sparse_map_avail, nbuf);
  COPY_STRING (blk, q, p);
  COPY_STRING (blk, q, "\n");
  for (i = 0; i < file->stat_info->sparse_map_avail; i++)
    {
      p = umaxtostr (map[i].offset, nbuf);
      COPY_STRING (blk, q, p);
      COPY_STRING (blk, q, "\n");
      p = umaxtostr (map[i].numbytes, nbuf);
      COPY_STRING (blk, q, p);
      COPY_STRING (blk, q, "\n");
    }
  memset (q, 0, BLOCKSIZE - (q - blk->buffer));
  set_next_block_after (blk);
  return true;
}

static bool
pax_dump_header (struct tar_sparse_file *file)
{
  file->stat_info->sparse_major = tar_sparse_major;
  file->stat_info->sparse_minor = tar_sparse_minor;

  return (file->stat_info->sparse_major == 0) ?
           pax_dump_header_0 (file) : pax_dump_header_1 (file);
}

static bool
decode_num (uintmax_t *num, char const *arg, uintmax_t maxval)
{
  uintmax_t u;
  char *arg_lim;

  if (!ISDIGIT (*arg))
    return false;
  
  u = strtoumax (arg, &arg_lim, 10);

  if (! (u <= maxval && errno != ERANGE) || *arg_lim)
    return false;
  
  *num = u;
  return true;
}

static bool
pax_decode_header (struct tar_sparse_file *file)
{
  if (file->stat_info->sparse_major > 0)
    {
      uintmax_t u;
      char nbuf[UINTMAX_STRSIZE_BOUND];
      union block *blk;
      char *p;
      size_t i;

#define COPY_BUF(b,buf,src) do                                     \
 {                                                                 \
   char *endp = b->buffer + BLOCKSIZE;                             \
   char *dst = buf;                                                \
   do                                                              \
     {                                                             \
       if (dst == buf + UINTMAX_STRSIZE_BOUND -1)                  \
         {                                                         \
         sprtf( "ERROR:%s: numeric overflow in sparse archive member!\n", \
	          file->stat_info->orig_file_name);               \
           return false;                                           \
         }                                                         \
       if (src == endp)                                            \
	 {                                                         \
	   set_next_block_after (b);                               \
           file->dumped_size += BLOCKSIZE;                         \
           b = find_next_block ();                                 \
           src = b->buffer;                                        \
	   endp = b->buffer + BLOCKSIZE;                           \
	 }                                                         \
       *dst = *src++;                                              \
     }                                                             \
   while (*dst++ != '\n');                                         \
   dst[-1] = 0;                                                    \
 } while (0)                       
      set_next_block_after (current_header);
      file->dumped_size += BLOCKSIZE;
      blk = find_next_block ();
      p = blk->buffer;
      COPY_BUF (blk,nbuf,p);
      if (!decode_num (&u, nbuf, TYPE_MAXIMUM (size64_t)))
	{
      sprtf("ERROR: %s: malformed sparse archive member\n", 
         file->stat_info->orig_file_name);
	  return false;
	}
      file->stat_info->sparse_map_size = u;
      file->stat_info->sparse_map = xcalloc ((size_t)file->stat_info->sparse_map_size,
					     sizeof (*file->stat_info->sparse_map));
      file->stat_info->sparse_map_avail = 0;
      for (i = 0; i < file->stat_info->sparse_map_size; i++)
	{
	  struct sp_array sp;
	  
	  COPY_BUF (blk,nbuf,p);
	  if (!decode_num (&u, nbuf, TYPE_MAXIMUM (off64_t)))
	    {
          sprtf("ERROR: %s: malformed sparse archive member\n", 
		      file->stat_info->orig_file_name);
	      return false;
	    }
	  sp.offset = u;
	  COPY_BUF (blk,nbuf,p);
	  if (!decode_num (&u, nbuf, TYPE_MAXIMUM (size64_t)))
	    {
          sprtf("ERROR: %s: malformed sparse archive member!\n", 
		      file->stat_info->orig_file_name);
	      return false;
	    }
	  sp.numbytes = u;
	  sparse_add_map (file->stat_info, &sp);
	}
      set_next_block_after (blk);
    }
  
  return true;
}

static struct tar_sparse_optab const pax_optab = {
  NULL,  /* No init function */
  NULL,  /* No done function */
  pax_sparse_member_p,
  pax_dump_header,
  NULL,
  pax_decode_header,  
  NULL,  /* No scan_block function */
  sparse_dump_region,
  sparse_extract_region,
};

// eof - DumpTarSp.c
