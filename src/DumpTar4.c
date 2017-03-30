
// DumpTar4.c

// ANOTHER ATTEMPT AT dumping a TAR file

#include	"Dump4.h"   // 20170330 - Fix cse for unix
#include <sys/types.h>
#include <sys/stat.h>

#include "DumpTar2.h"

#define  true  TRUE

extern union block *current_block;
extern union block *record_end;
extern enum dump_status sparse_skip_file (struct tar_stat_info *st);

extern void xheader_read (struct xheader *xhdr, union block *p, size_t size);

extern BOOL bExtraDebug;
extern size_t iMaxNameLen, iMaxFileSize, iMinFileSize, iFileCount, iDirCount;
extern struct tar_stat_info current_stat_info;
extern enum read_header rdhdr;
extern char const * get_format_stg(enum archive_format current);
extern enum archive_format current_format;
extern void tar_stat_destroy (struct tar_stat_info *st);
extern union block *current_header;
extern enum read_header tar_checksum (union block *header, bool silent);
extern enum archive_format form;
extern void decode_header (union block *header, struct tar_stat_info *stat_info,
	       enum archive_format *format_pointer, int do_user_group);
extern char * get_header_stg( enum read_header hdr );
extern char * get_type_string( char type );
extern char const * tartime (struct timespec t, bool full_time);
extern int REGTYPE_count, LNKTYPE_count, SYMTYPE_count;  //, "reserved" },
extern int CHRTYPE_count, BLKTYPE_count, DIRTYPE_count;
extern int FIFOTYPE_count, CONTTYPE_count, XHDTYPE_count, XGLTYPE_count;
extern int default_count;
extern int collect_octal_size( char * lpb, struct posix_header * pph, size_t * pfsz, int warn );
extern size_t collect_prefix_name( char * lpb, struct posix_header * pph );
extern int warn_count, max_warn_count;
extern size_t available_space_after (union block *pointer);
extern off_t current_block_ordinal (void);
extern void set_next_block_after (union block *block);
extern enum read_header read_header_primitive (bool raw_extended_headers, struct tar_stat_info *info);
extern void assign_string (char **string, const char *value);
extern bool tar_sparse_init (struct tar_sparse_file *file);
extern union block * find_next_block( void );
extern void print_header (struct tar_stat_info *st, off_t block_ordinal);
extern union block *record_start;

int verbose_option = 6;
size64_t  save_totsize = 0;
size64_t  save_sizeleft = 0;
int ignore_zeros_option = 0;
int skipping_option = 0;

enum read_header read_header (BOOL raw_extended_headers)
{
  return read_header_primitive (raw_extended_headers, &current_stat_info);
}

void
mv_begin (struct tar_stat_info *st)
{
  //if (multi_volume_option)
  //  {
  //    assign_string (&save_name,  st->orig_file_name);
  //    save_totsize = save_sizeleft = st->stat.st_size;
  //  }
}

void
mv_end ()
{
  //if (multi_volume_option)
  //  assign_string (&save_name, 0);
}

void
mv_total_size (off64_t size)
{
  save_totsize = size;
}

void
mv_size_left (off64_t size)
{
  save_sizeleft = size;
}

static bool
tar_sparse_decode_header (struct tar_sparse_file *file)
{
  if (file->optab->decode_header)
    return file->optab->decode_header (file);
  return true;
}

/* Skip over SIZE bytes of data in blocks in the archive.  */
void
skip_file (off64_t size)
{
  union block *x;

  /* FIXME: Make sure mv_begin is always called before it */
#if 0
  if (seekable_archive)
    {
      off64_t nblk = seek_archive (size);
      if (nblk >= 0)
	size -= nblk * BLOCKSIZE;
      else
	seekable_archive = false;
    }
#endif // 0

  mv_size_left (size);

  while (size > 0)
    {
      x = find_next_block ();
      if (! x)
      {
	      //FATAL_ERROR ((0, 0, _("Unexpected EOF in archive")));
         sprtf( "Unexpected EOF in archive! Aborting ...\n" );
         pgm_exit(2);
      }

      set_next_block_after (x);
      size -= BLOCKSIZE;
      mv_size_left (size);
    }
}

static bool
tar_sparse_done (struct tar_sparse_file *file)
{
  if (file->optab->done)
    return file->optab->done (file);
  return true;
}

#if 0 // 0 *****************************
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

#endif // 0 ****************************


/* Skip the current member in the archive.
   NOTE: Current header must be decoded before calling this function. */
void
skip_member (void)
{
  if (!current_stat_info.skipped)
    {
      char save_typeflag = current_header->header.typeflag;
      set_next_block_after (current_header);

      mv_begin (&current_stat_info);

      if (current_stat_info.is_sparse)
	sparse_skip_file (&current_stat_info);
      else if (save_typeflag != DIRTYPE)
	skip_file (current_stat_info.stat.st_size);

      mv_end ();
    }
}

void
list_archive (void)
{
  off_t block_ordinal = current_block_ordinal ();
  /* Print the header block.  */

  decode_header (current_header, &current_stat_info, &current_format, 0);
  if (verbose_option)
    print_header (&current_stat_info, block_ordinal);
  //if (incremental_option)
  //  {
  //    if (verbose_option > 2)
  //	{
  //	  if (is_dumpdir (&current_stat_info))
  //	    list_dumpdir (current_stat_info.dumpdir,
  //			  dumpdir_size (current_stat_info.dumpdir));
  //	}
  //    }
  skip_member ();
}

typedef struct tagBLKINF {
   int done;
}BLKINF, * PBLKINF;

size_t block_count_size = 0;
size_t block_current = 0;
PBLKINF block_info_ptr = NULL;

size_t Set_Start_and_End( union block * p, off64_t dwm )
{
   size_t cnt = 0;
   record_start = p;
   current_block = p;
   while(dwm)
   {
      cnt++;
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
   if( block_info_ptr )
      free( block_info_ptr );

   block_info_ptr = (PBLKINF) malloc( cnt * sizeof(BLKINF) );
   CHKMEM(block_info_ptr);
   block_count_size = cnt;
   block_current = 0;
   ZeroMemory( block_info_ptr, ( cnt * sizeof(BLKINF) ));
   return cnt;
}

void show_current_block(void)
{
   if( !block_info_ptr[block_current].done )
   {
      block_info_ptr[block_current].done = 1;
      sprtf( "Done block %d ...\n", (block_current+1) );
   }
   block_current++;
}


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


BOOL  DumpTAR( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   union block * p = (union block *)lpdf->df_pVoid;
   DWORD dwm = lpdf->dwmax;
   enum read_header status = HEADER_STILL_UNREAD;
   enum read_header prev_status;
   BOOL cont = TRUE;
   size_t blocks;
   off_t block_ordinal;

   if( !p )
      return bRet;

   if( !dwm )
      return bRet;

   base64_init ();
   // ***TBD*** name_gather ();

   blocks = Set_Start_and_End( p, dwm );
   sprtf( "Processing %I64u TAR blocks ...\n", blocks );

   if( !blocks )
      return bRet;

   do
   {
      blocks--;
      prev_status = status;
      tar_stat_destroy (&current_stat_info);
      status = read_header (FALSE);
      block_ordinal = current_block_ordinal ();
      switch (status)
      {
      case HEADER_STILL_UNREAD:
      case HEADER_SUCCESS_EXTENDED:
         sprtf( "ERROR: Block %d FAILED! Aborting!\n", current_block_ordinal ());
         abort ();

      case HEADER_SUCCESS:
         bRet = TRUE;   // we have one VALID blcok at least
         /* Valid header.  We should decode next field (mode) first.
            Ensure incoming names are null terminated.  */

         //if (! name_match (current_stat_info.file_name)
	      //   || (NEWER_OPTION_INITIALIZED (newer_mtime_option)
		   //   /* FIXME: We get mtime now, and again later; this causes
		   //      duplicate diagnostics if header.mtime is bogus.  */
		   //   && ((mtime.tv_sec
		   //       = TIME_FROM_HEADER (current_header->header.mtime)),
		   //      /* FIXME: Grab fractional time stamps from
			//      extended header.  */
		   //      mtime.tv_nsec = 0,
		   //      current_stat_info.mtime = mtime,
		   //      OLDER_TAR_STAT_TIME (current_stat_info, m)))
	      //      || excluded_name (current_stat_info.file_name))
         if( skipping_option )
         {
            //if( verbose_option >= 5 )
            if( VERB5 )
            {
               decode_header (current_header,
				       &current_stat_info, &current_format, 0);
#ifdef DUMP4
               sprtf("v5: ");  /* start output with 'v5: ' ... */
#else // !DUMP4
               fprintf (stderr, "v5: ");  /* start output with 'v5: ' ... */
#endif /* DUMP4 y/n */
               print_header (&current_stat_info, block_ordinal);
            }
            switch (current_header->header.typeflag)
            {
            case GNUTYPE_VOLHDR:
            case GNUTYPE_MULTIVOL:
               sprtf( "WARNING: Got %s ...\n",
                  (( current_header->header.typeflag == GNUTYPE_VOLHDR ) ?
                  "GNUTYPE_VOLHDR" : "GNUTYPE_MULTIVOL" ));
               break;

            case DIRTYPE:
               //if (show_omitted_dirs_option)
               //   WARN ((0, 0, _("%s: Omitting"),
               //   quotearg_colon (current_stat_info.file_name)));
               sprtf( "WARNING: Skipping %s as DIRECTORY type...\n",
                  current_stat_info.file_name );
               // /* Fall through.  */
            default:
               decode_header (current_header, &current_stat_info, &current_format, 0);
               print_header (&current_stat_info, block_ordinal);
               skip_member ();
               continue;
            }
         }

         //(*do_something) ();
         list_archive();
         continue;

      case HEADER_ZERO_BLOCK:
         //if (block_number_option)
	      //{
	      //   char buf[UINTMAX_STRSIZE_BOUND];
	      //   fprintf (stdlis, _("block %s: ** Block of NULs **\n"),
		   //      STRINGIFY_BIGINT (current_block_ordinal (), buf));
         //}
         sprtf( "Block %d is a BLOCK OF NULs!\n",
            current_block_ordinal () + 1);
         set_next_block_after (current_header);
         if (!ignore_zeros_option)
	      {
	      //   char buf[UINTMAX_STRSIZE_BOUND];
            status = read_header (FALSE);
            if (status == HEADER_ZERO_BLOCK)
            {
               sprtf( "Block %d is a BLOCK OF NULs! END OF TAR!!\n",
                  current_block_ordinal () + 1);
		         break;
            }
   	   //   WARN ((0, 0, _("A lone zero block at %s"),
	   	//     STRINGIFY_BIGINT (current_block_ordinal (), buf)));
            sprtf( "WARNING: A lone block of zeros!\n" );
	         break;
	      }
	      status = prev_status;
	      continue;

      case HEADER_END_OF_FILE:
         //if (block_number_option)
	      //{
	      //   char buf[UINTMAX_STRSIZE_BOUND];
	      //   fprintf (stdlis, _("block %s: ** End of File **\n"),
		   //       STRINGIFY_BIGINT (current_block_ordinal (), buf));
	      //}
	      break;

      case HEADER_FAILURE:
         /* If the previous header was good, tell them that we are
            skipping bad ones.  */
         set_next_block_after (current_header);
         switch (prev_status)
         {
         case HEADER_STILL_UNREAD:
            //ERROR ((0, 0, _("This does not look like a tar archive")));
            sprtf( "This does not look like a tar archive!\n" );
	         /* Fall through.  */
         case HEADER_ZERO_BLOCK:
         case HEADER_SUCCESS:
            //if (block_number_option)
		      //{
		      //   char buf[UINTMAX_STRSIZE_BOUND];
		      //   off_t block_ordinal = current_block_ordinal ();
		      //   block_ordinal -= recent_long_name_blocks;
		      //   block_ordinal -= recent_long_link_blocks;
		      //   fprintf (stdlis, _("block %s: "),
			   //      STRINGIFY_BIGINT (block_ordinal, buf));
		      //}
	         //ERROR ((0, 0, _("Skipping to next header")));
            sprtf( "ERROR: Skipping to next header\n" );
	         break;

         case HEADER_END_OF_FILE:
         case HEADER_FAILURE:
            /* We are in the middle of a cascade of errors.  */
            break;

         case HEADER_SUCCESS_EXTENDED:
            abort ();
         }
         continue;

      default:
         sprtf( "WARNING: read_header() returned INVALID enum type!\n" );
         break;
      }

      break;
   } while( blocks );

   set_next_block_after (current_header);
   p = current_header = find_next_block ();
   block_ordinal = current_block_ordinal ();
   while( p < record_end )
   {
      int sz = sizeof(*p);
      char * pc = (char *)p;
      while(sz)
      {
         if(*pc)
            break;
         sz--;
         pc++;
      }
      if(sz)
      {
         sprtf( "Block %d is NOT all zeros ...\n", (block_ordinal+1) );
      }
      else
      {
         sprtf( "Block %d is ALL zeros ...\n", (block_ordinal+1) );
      }
      p++;
      if( p > record_end )
         break;
      set_next_block_after (current_header);
      p = current_header = find_next_block ();
      block_ordinal = current_block_ordinal ();
   }
   sprtf( "End processing ...\n" );


   return bRet;
}

