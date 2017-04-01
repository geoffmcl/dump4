// DumpTar2.c

// some services cut from tar 1.20 source

#pragma warning(disable:4146) // C4146: unary minus operator applied to unsigned type, result still unsigned
#include	"Dump4.h"   // 20170330 - Fix cse for unix
#include <sys/types.h>
#include <sys/stat.h>
#include "DumpTar2.h"
#include <stddef.h>
#include <limits.h>
#include <time.h>

extern enum archive_format archive_format;
extern void xheader_decode (struct tar_stat_info *st);
extern char * umaxtostr (inttype i, char *buf);
extern bool strip_trailing_slashes (char *file);
extern void xheader_decode_global (struct xheader *xhdr);
extern void code_ns_fraction (int ns, char *p);
extern bool sparse_member_p (struct tar_stat_info *st);
extern int verbose_option;
#ifdef DUMP4
extern void show_current_block(void);
#endif // DUMP4

#define PAXEXIT_SUCCESS 0
#define PAXEXIT_DIFFERS 1
#define PAXEXIT_FAILURE 2

#ifndef gid_t
typedef short gid_t;
#endif
#ifndef uid_t
typedef short uid_t;
#endif

int incremental_option = 0;
struct timespec start_time;
int numeric_owner_option = 0;
enum archive_format current_format;
// enum archive_format archive_format = DEFAULT_ARCHIVE_FORMAT;
union block *current_header;
union block *record_start = 0;
/* From file: [C:\Projects\include\stdint.h]
 85: [typedef unsigned long int uintmax_t;]
 90: [typedef unsigned long long int uintmax_t;]
 */
static off_t record_start_block = 0; /* block ordinal at record_start */
union block *recent_long_name = NULL;	/* recent long name header and contents */
union block *recent_long_link = NULL;	/* likewise, for long link */
size64_t recent_long_name_blocks = 0;	/* number of blocks in recent_long_name */
size64_t recent_long_link_blocks = 0;	/* likewise, for long link */


/* Make sure you link with the proper libraries if you are running the
   Yellow Peril (thanks for the good laugh, Ian J.!), or, euh... NIS.
   This code should also be modified for non-UNIX systems to do something
   reasonable.  */

char *cached_uname = 0;
char *cached_gname = 0;

uid_t cached_uid = 0;	/* valid only if cached_uname is not empty */
gid_t cached_gid = 0;	/* valid only if cached_gname is not empty */

/* These variables are valid only if nonempty.  */
static char *cached_no_such_uname = 0;
static char *cached_no_such_gname = 0;

/* These variables are valid only if nonzero.  It's not worth optimizing
   the case for weird systems where 0 is not a valid uid or gid.  */
uid_t cached_no_such_uid = 0;
gid_t cached_no_such_gid = 0;

//static void register_individual_file (char const *name);


/* The extra casts in the following macros work around compiler bugs,
   e.g., in Cray C 5.0.3.0.  */

#if 0 // **********************************************
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
#define mode_t int

#endif // 0 *******************************************


//#define  WARN(a)
//#define  TARERROR(a)

#define WARN(Args) \
  tar_error Args
#define TARERROR(Args) \
  (tar_error Args, exit_status = PAXEXIT_FAILURE)
#define _(a)   a

int exit_status = 0;

char *
quote( char * msg )
{
   static char _s_buf[264];
   char * cp = _s_buf;
   size_t len = strlen(msg);
   strcpy(cp,"[");
   if( len < 256 )
      strcat(cp,msg);
   else
   {
      strncat(cp,msg,256);
      strcat(cp,"...");
   }
   strcat(cp,"]");
   return cp;
}

static void
tar_error_tail (int status, int errnum, const char *message, va_list args)
{
   vsprtf( (char *)message, args );
   //vfprintf (stderr, message, args);
   //va_end (args);
}


void
tar_error (int status, int errnum, const char *message, ...)
{
  va_list args;
  va_start (args, message);
  tar_error_tail (status, errnum, message, args);
}

#if 0 // ***************************************

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
#endif // 0 ****************************************

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
static int done_base_init = 0;
static void
base64_init (void)
{
  int i;
  memset (base64_map, 64, sizeof base64_map);
  for (i = 0; i < 64; i++)
    base64_map[(int) base_64_digits[i]] = i;
  done_base_init = 1;
}

// **********************************************
// quoting options

#define INT_BITS (sizeof (int) * CHAR_BIT)

struct quoting_options
{
  /* Basic quoting style.  */
  int style;

  /* Additional flags.  Bitwise combination of enum quoting_flags.  */
  int flags;

  /* Quote the characters indicated by this bit vector even if the
     quoting style would not normally require them to be quoted.  */
  unsigned int quote_these_too[(UCHAR_MAX / INT_BITS) + 1];
};

/* Names of quoting styles.  */
char const *const quoting_style_args[] =
{
  "literal",
  "shell",
  "shell-always",
  "c",
  "c-maybe",
  "escape",
  "locale",
  "clocale",
  0
};

/* Correspondences to quoting style names.  */
//enum quoting_style const quoting_style_vals[] =
enum quoting_style
{
  literal_quoting_style,
  shell_quoting_style,
  shell_always_quoting_style,
  c_quoting_style,
  c_maybe_quoting_style,
  escape_quoting_style,
  locale_quoting_style,
  clocale_quoting_style
};

/* The default quoting options.  */
static struct quoting_options default_quoting_options;

static size_t
quotearg_buffer_restyled (char *buffer, size_t buffersize,
			  char const *arg, size_t argsize,
			  enum quoting_style quoting_style, int flags,
			  unsigned int const *quote_these_too)
{
   int i;
   unsigned char c;
   size_t len = 0;

#define STORE(c) \
    do \
      { \
	if (len < buffersize) \
	  buffer[len] = (c); \
	len++; \
      } \
    while (0)

   STORE('[');
   for (i = 0;  ! (argsize == SIZE_MAX ? arg[i] == '\0' : i == argsize);  i++)
   {
      c = arg[i];
      STORE(c);
      if( c == '%' )
         STORE(c);
   }
   STORE(']');

   return len;
}  

/* Place into buffer BUFFER (of size BUFFERSIZE) a quoted version of
   argument ARG (of size ARGSIZE), using O to control quoting.
   If O is null, use the default.
   Terminate the output with a null character, and return the written
   size of the output, not counting the terminating null.
   If BUFFERSIZE is too small to store the output string, return the
   value that would have been returned had BUFFERSIZE been large enough.
   If ARGSIZE is SIZE_MAX, use the string length of the argument for
   ARGSIZE.  */
size_t
quotearg_buffer (char *buffer, size_t buffersize,
		 char const *arg, size_t argsize,
		 struct quoting_options const *o)
{
  struct quoting_options const *p = o ? o : &default_quoting_options;
  int e = errno;
  size_t r = quotearg_buffer_restyled (buffer, buffersize, arg, argsize,
				       p->style, p->flags, p->quote_these_too);
  errno = e;
  return r;
}


static uintmax_t
from_header (char const *where0, size_t digs, char const *type,
	     uintmax_t in_minus_minval, uintmax_t in_maxval,
	     bool octal_only, bool silent)
{
   uintmax_t value;
   //int value;
   char const *where = where0;
   char const *lim = where + digs;
   int negative = 0;
   //int maxval = in_maxval;
   //int minus_minval = in_minus_minval;
   uintmax_t maxval = in_maxval;
   uintmax_t minus_minval = in_minus_minval;

   if( !done_base_init )
      base64_init();

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
         static bool warned_once = 0;
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
         static struct quoting_options *o = NULL;
         //***if (!o)
         //*** {
         //***   o = clone_quoting_options (0);
         //***   set_quoting_style (o, locale_quoting_style);
         //*** }

         while (where0 != lim && ! lim[-1])
            lim--;
         quotearg_buffer (buf, sizeof buf, where0, lim - where, o);
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

uintmax_t
uintmax_from_header (const char *p, size_t s)
{
  return from_header (p, s, "uintmax_t", (uintmax_t) 0,
		      TYPE_MAXIMUM (uintmax_t), false, false);
}


void xalloc_die (void)
{
   sprtf( "ERROR: MEMORY FAILED! Aborting!!\n" );
   exit(-1);
}

#define HAVE_GNU_CALLOC 0
# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))

void *
xcalloc (size_t n, size_t s)
{
  void *p;
  /* Test for overflow, since some calloc implementations don't have
     proper overflow checks.  But omit overflow and size-zero tests if
     HAVE_GNU_CALLOC, since GNU calloc catches overflow and never
     returns NULL if successful.  */
  if ((! HAVE_GNU_CALLOC && xalloc_oversized (n, s))
      || (! (p = calloc (n, s)) && (HAVE_GNU_CALLOC || n != 0)))
    xalloc_die ();
  return p;
}
/* Allocate N bytes of memory dynamically, with error checking.  */

void *
xmalloc (size_t n)
{
  void *p = malloc (n);
  if (!p && n != 0)
    xalloc_die ();
  return p;
}

void *
xrealloc (void *p, size_t n)
{
  p = realloc (p, n);
  if (!p && n != 0)
    xalloc_die ();
  return p;
}


/* Clone an object P of size S, with error checking.  There's no need
   for xnmemdup (P, N, S), since xmemdup (P, N * S) works without any
   need for an arithmetic overflow check.  */

void *
xmemdup (void const *p, size_t s)
{
  return memcpy (xmalloc (s), p, s);
}

/* Clone STRING.  */

char *
xstrdup (char const *string)
{
  return xmemdup (string, strlen (string) + 1);
}


void
assign_string (char **string, const char *value)
{
  if (*string)
    free (*string);
  *string = value ? xstrdup (value) : 0;
}

mode_t
mode_from_header (const char *p, size_t s)
{
  /* Do not complain about unrecognized mode bits.  */
  mode_t u = (mode_t)from_header (p, s, "mode_t",
			    - (uintmax_t) TYPE_MINIMUM (mode_t),
			    TYPE_MAXIMUM (uintmax_t), false, false);
  return ((u & TSUID ? S_ISUID : 0)
	  | (u & TSGID ? S_ISGID : 0)
	  | (u & TSVTX ? S_ISVTX : 0)
	  | (u & TUREAD ? S_IRUSR : 0)
	  | (u & TUWRITE ? S_IWUSR : 0)
	  | (u & TUEXEC ? S_IXUSR : 0)
	  | (u & TGREAD ? S_IRGRP : 0)
	  | (u & TGWRITE ? S_IWGRP : 0)
	  | (u & TGEXEC ? S_IXGRP : 0)
	  | (u & TOREAD ? S_IROTH : 0)
	  | (u & TOWRITE ? S_IWOTH : 0)
	  | (u & TOEXEC ? S_IXOTH : 0));
}

time_t
time_from_header (const char *p, size_t s)
{
  return from_header (p, s, "time_t",
		      - (uintmax_t) TYPE_MINIMUM (time_t),
		      (uintmax_t) TYPE_MAXIMUM (time_t), false, false);
}

gid_t
gid_from_header (const char *p, size_t s)
{
  return (gid_t)from_header (p, s, "gid_t",
		      - (uintmax_t) TYPE_MINIMUM (gid_t),
		      (uintmax_t) TYPE_MAXIMUM (gid_t),
		      false, false);
}

uid_t
uid_from_header (const char *p, size_t s)
{
  return (uid_t)from_header (p, s, "uid_t",
		      - (uintmax_t) TYPE_MINIMUM (uid_t),
		      (uintmax_t) TYPE_MAXIMUM (uid_t), false, false);
}


static bool
tar_sparse_member_p (struct tar_sparse_file *file)
{
  if (file->optab->sparse_member_p)
    return file->optab->sparse_member_p (file);
  return false;
}

//static struct tar_sparse_optab const oldgnu_optab;
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
//#define __attribute__(a)
//#define OFF_FROM_HEADER(where) off_from_header (where, sizeof (where))
//#define SIZE_FROM_HEADER(where) size_from_header (where, sizeof (where))
//#define OFF_TO_CHARS(val, where) off_to_chars (val, where, sizeof (where))
//#define SIZE_TO_CHARS(val, where) size_to_chars (val, where, sizeof (where))

off64_t
off_from_header (const char *p, size_t s)
{
  /* Negative offsets are not allowed in tar files, so invoke
     from_header with minimum value 0, not TYPE_MINIMUM (off_t).  */
  return from_header (p, s, "off64_t", (uintmax_t) 0,
		      (uintmax_t) TYPE_MAXIMUM (off64_t), false, false);
}

size64_t
size_from_header (const char *p, size_t s)
{
  return from_header (p, s, "size64_t", (uintmax_t) 0,
		      (uintmax_t) TYPE_MAXIMUM (size64_t), false, false);
}

/* Convert NEGATIVE VALUE (which was originally of size VALSIZE) to
   external form, using SUBSTITUTE (...) if VALUE won't fit.  Output
   to buffer WHERE with size SIZE.  NEGATIVE is 1 iff VALUE was
   negative before being cast to uintmax_t; its original bitpattern
   can be deduced from VALSIZE, its original size before casting.
   TYPE is the kind of value being output (useful for diagnostics).
   Prefer the POSIX format of SIZE - 1 octal digits (with leading zero
   digits), followed by '\0'.  If this won't work, and if GNU or
   OLDGNU format is allowed, use '\200' followed by base-256, or (if
   NEGATIVE is nonzero) '\377' followed by two's complement base-256.
   If neither format works, use SUBSTITUTE (...)  instead.  Pass to
   SUBSTITUTE the address of an 0-or-1 flag recording whether the
   substitute value is negative.  */

/* The maximum uintmax_t value that can be represented with DIGITS digits,
   assuming that each digit is BITS_PER_DIGIT wide.  */
#define MAX_VAL_WITH_DIGITS(digits, bits_per_digit) \
   ((digits) * (bits_per_digit) < sizeof (uintmax_t) * CHAR_BIT \
    ? ((uintmax_t) 1 << ((digits) * (bits_per_digit))) - 1 \
    : (uintmax_t) -1)

static void
to_octal (uintmax_t value, char *where, size64_t size)
{
  uintmax_t v = value;
  size64_t i = size;

  do
    {
      where[--i] = (char)('0' + (v & ((1 << LG_8) - 1)));
      v >>= LG_8;
    }
  while (i);
}

static void
to_base256 (int negative, uintmax_t value, char *where, size64_t size)
{
  uintmax_t v = value;
  uintmax_t propagated_sign_bits =
    ((uintmax_t) - negative << (CHAR_BIT * sizeof v - LG_256));
  size64_t i = size;

  do
    {
      where[--i] = (char)(v & ((1 << LG_256) - 1));
      v = propagated_sign_bits | (v >> LG_256);
    }
  while (i);
}


#define STRINGIFY_BIGINT(i, b) umaxtostr (i, b)

#if 0 // ******************************************

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

#endif // 0 *********************************

//typedef int inttype;
#ifdef _WIN32
typedef __int64 inttype;
#else
typedef size_t inttype;
#endif

char *
umaxtostr (inttype i, char *buf)
{
  char *p = buf + INT_STRLEN_BOUND (inttype);
  *p = 0;

  if (i < 0)
    {
      do
	*--p = (char)('0' - i % 10);
      while ((i /= 10) != 0);

      *--p = '-';
    }
  else
    {
      do
	*--p = (char)('0' + i % 10);
      while ((i /= 10) != 0);
    }

  return p;
}

bool
to_chars (int negative, uintmax_t value, size64_t valsize,
	  uintmax_t (*substitute) (int *),
	  char *where, size64_t size, const char *type);

static bool
to_chars_subst (int negative, int gnu_format, uintmax_t value, size64_t valsize,
		uintmax_t (*substitute) (int *),
		char *where, size64_t size, const char *type)
{
  uintmax_t maxval = (gnu_format
		      ? MAX_VAL_WITH_DIGITS (size - 1, LG_256)
		      : MAX_VAL_WITH_DIGITS (size - 1, LG_8));
  char valbuf[UINTMAX_STRSIZE_BOUND + 1];
  char maxbuf[UINTMAX_STRSIZE_BOUND];
  char minbuf[UINTMAX_STRSIZE_BOUND + 1];
  char const *minval_string;
  char const *maxval_string = STRINGIFY_BIGINT (maxval, maxbuf);
  char const *value_string;

  if (gnu_format)
    {
      uintmax_t m = maxval + 1 ? maxval + 1 : maxval / 2 + 1;
      char *p = STRINGIFY_BIGINT (m, minbuf + 1);
      *--p = '-';
      minval_string = p;
    }
  else
    minval_string = "0";

  if (negative)
    {
      char *p = STRINGIFY_BIGINT (- value, valbuf + 1);
      *--p = '-';
      value_string = p;
    }
  else
    value_string = STRINGIFY_BIGINT (value, valbuf);

  if (substitute)
    {
      int negsub;
      uintmax_t sub = substitute (&negsub) & maxval;
      /* NOTE: This is one of the few places where GNU_FORMAT differs from
	 OLDGNU_FORMAT.  The actual differences are:

	 1. In OLDGNU_FORMAT all strings in a tar header end in \0
	 2. Incremental archives use oldgnu_header.
	 
	 Apart from this they are completely identical. */
      uintmax_t s = (negsub &= archive_format == GNU_FORMAT) ? - sub : sub;
      char subbuf[UINTMAX_STRSIZE_BOUND + 1];
      char *sub_string = STRINGIFY_BIGINT (s, subbuf + 1);
      if (negsub)
	*--sub_string = '-';
      WARN ((0, 0, _("value %s out of %s range %s..%s; substituting %s"),
	     value_string, type, minval_string, maxval_string,
	     sub_string));
      return to_chars (negsub, s, valsize, 0, where, size, type);
    }
  else
    TARERROR ((0, 0, _("value %s out of %s range %s..%s"),
	    value_string, type, minval_string, maxval_string));
  return false;
}

bool
to_chars (int negative, uintmax_t value, size64_t valsize,
	  uintmax_t (*substitute) (int *),
	  char *where, size64_t size, const char *type)
{
  int gnu_format = (archive_format == GNU_FORMAT
		    || archive_format == OLDGNU_FORMAT);

  /* Generate the POSIX octal representation if the number fits.  */
  if (! negative && value <= MAX_VAL_WITH_DIGITS (size - 1, LG_8))
    {
      where[size - 1] = '\0';
      to_octal (value, where, size - 1);
      return true;
    }
  else if (gnu_format)
    {
      /* Try to cope with the number by using traditional GNU format
	 methods */

      /* Generate the base-256 representation if the number fits.  */
      if (((negative ? -1 - value : value)
	   <= MAX_VAL_WITH_DIGITS (size - 1, LG_256)))
	{
	  where[0] = negative ? -1 : 1 << (LG_256 - 1);
	  to_base256 (negative, value, where + 1, size - 1);
	  return true;
	}

      /* Otherwise, if the number is negative, and if it would not cause
	 ambiguity on this host by confusing positive with negative
	 values, then generate the POSIX octal representation of the value
	 modulo 2**(field bits).  The resulting tar file is
	 machine-dependent, since it depends on the host word size.  Yuck!
	 But this is the traditional behavior.  */
      else if (negative && valsize * CHAR_BIT <= (size - 1) * LG_8)
	{
	  static int warned_once;
	  if (! warned_once)
	    {
	      warned_once = 1;
	      WARN ((0, 0, _("Generating negative octal headers")));
	    }
	  where[size - 1] = '\0';
	  to_octal (value & MAX_VAL_WITH_DIGITS (valsize * CHAR_BIT, 1),
		    where, size - 1);
	  return true;
	}
      /* Otherwise fall back to substitution, if possible: */
    }
  else
    substitute = NULL; /* No substitution for formats, other than GNU */

  return to_chars_subst (negative, gnu_format, value, valsize, substitute,
			 where, size, type);
}

bool
off_to_chars (off64_t v, char *p, size64_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, 0, p, s, "off64_t");
}

bool
size_to_chars (size64_t v, char *p, size64_t s)
{
  return to_chars (0, (uintmax_t) v, sizeof v, 0, p, s, "size64_t");
}

bool
time_to_chars (time_t v, char *p, size_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, 0, p, s, "time_t");
}
bool
mode_to_chars (mode_t v, char *p, size_t s)
{
  /* In the common case where the internal and external mode bits are the same,
     and we are not using POSIX or GNU format,
     propagate all unknown bits to the external mode.
     This matches historical practice.
     Otherwise, just copy the bits we know about.  */
  int negative;
  uintmax_t u;
  if (S_ISUID == TSUID && S_ISGID == TSGID && S_ISVTX == TSVTX
      && S_IRUSR == TUREAD && S_IWUSR == TUWRITE && S_IXUSR == TUEXEC
      && S_IRGRP == TGREAD && S_IWGRP == TGWRITE && S_IXGRP == TGEXEC
      && S_IROTH == TOREAD && S_IWOTH == TOWRITE && S_IXOTH == TOEXEC
      && archive_format != POSIX_FORMAT
      && archive_format != USTAR_FORMAT
      && archive_format != GNU_FORMAT
      && archive_format != OLDGNU_FORMAT)
    {
      negative = v < 0;
      u = v;
    }
  else
    {
      negative = 0;
      u = ((v & S_ISUID ? TSUID : 0)
	   | (v & S_ISGID ? TSGID : 0)
	   | (v & S_ISVTX ? TSVTX : 0)
	   | (v & S_IRUSR ? TUREAD : 0)
	   | (v & S_IWUSR ? TUWRITE : 0)
	   | (v & S_IXUSR ? TUEXEC : 0)
	   | (v & S_IRGRP ? TGREAD : 0)
	   | (v & S_IWGRP ? TGWRITE : 0)
	   | (v & S_IXGRP ? TGEXEC : 0)
	   | (v & S_IROTH ? TOREAD : 0)
	   | (v & S_IWOTH ? TOWRITE : 0)
	   | (v & S_IXOTH ? TOEXEC : 0));
    }
  return to_chars (negative, u, sizeof v, 0, p, s, "mode_t");
}

bool
uintmax_to_chars (uintmax_t v, char *p, size_t s)
{
  return to_chars (0, v, sizeof v, 0, p, s, "uintmax_t");
}

static int
dbg_chk( void )
{
   int i;
   i = 0;
   return i;
}

static int _s_done_pwd = 0;
static int _s_done_grp = 0;
static struct passwd pwd;
static struct group  grp;

struct passwd *getpwuid (uid_t uid)
{
   dbg_chk();
   if( !_s_done_pwd )
   {
      pwd.pw_name = "user";
      _s_done_pwd = 1;
   }
   return &pwd;
}
struct group *getgrgid (gid_t gid)
{ 
   dbg_chk();
   if( !_s_done_grp )
   {
      grp.gr_name = "group";
      _s_done_grp = 1;
   }
   return &grp;
}
static struct passwd *getpwnam (char const *uname)
{
   dbg_chk();
   return 0;
}
static struct group *getgrnam (char const * gname)
{ 
   dbg_chk();
   return 0;
}


/* Given UNAME, set the corresponding UID and return 1, or else, return 0.  */
int
uname_to_uid (char const *uname, uid_t *uidp)
{
  struct passwd *passwd;

  if (cached_no_such_uname
      && strcmp (uname, cached_no_such_uname) == 0)
    return 0;

  if (!cached_uname
      || uname[0] != cached_uname[0]
      || strcmp (uname, cached_uname) != 0)
    {
      passwd = getpwnam (uname);
      if (passwd)
	{
	  cached_uid = passwd->pw_uid;
	  assign_string (&cached_uname, passwd->pw_name);
	}
      else
	{
	  assign_string (&cached_no_such_uname, uname);
	  return 0;
	}
    }
  *uidp = cached_uid;
  return 1;
}

static uintmax_t
uid_substitute (int *negative)
{
  uid_t r;
#ifdef UID_NOBODY
  r = UID_NOBODY;
#else
  static uid_t uid_nobody;
  if (!uid_nobody && !uname_to_uid ("nobody", &uid_nobody))
    uid_nobody = -2;
  r = uid_nobody;
#endif
  *negative = r < 0;
  return r;
}

bool
uid_to_chars (uid_t v, char *p, size_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, uid_substitute, p, s, "uid_t");
}


static void
sparse_add_map (struct tar_stat_info *st, struct sp_array const *sp)
{
  struct sp_array *sparse_map = st->sparse_map;
  size64_t avail = st->sparse_map_avail;
  if (avail == st->sparse_map_size) {
    //??st->sparse_map = sparse_map = xrealloc( (void *)sparce_map, sizeof *sparse_map);
  //**  st->sparse_map = sparse_map = 
  //**    x2nrealloc (sparse_map, &st->sparse_map_size, sizeof *sparse_map);
  }
  sparse_map[avail] = *sp;
  st->sparse_map_avail = avail + 1;
}

/* Given GNAME, set the corresponding GID and return 1, or else, return 0.  */
int
gname_to_gid (char const *gname, gid_t *gidp)
{
  struct group *group;

  if (cached_no_such_gname
      && strcmp (gname, cached_no_such_gname) == 0)
    return 0;

  if (!cached_gname
      || gname[0] != cached_gname[0]
      || strcmp (gname, cached_gname) != 0)
    {
      group = getgrnam (gname);
      if (group)
	{
	  cached_gid = group->gr_gid;
	  assign_string (&cached_gname, gname);
	}
      else
	{
	  assign_string (&cached_no_such_gname, gname);
	  return 0;
	}
    }
  *gidp = cached_gid;
  return 1;
}

static uintmax_t
gid_substitute (int *negative)
{
  gid_t r;
#ifdef GID_NOBODY
  r = GID_NOBODY;
#else
  static gid_t gid_nobody;
  if (!gid_nobody && !gname_to_gid ("nobody", &gid_nobody))
    gid_nobody = -2;
  r = gid_nobody;
#endif
  *negative = r < 0;
  return r;
}

bool
gid_to_chars (gid_t v, char *p, size_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, gid_substitute, p, s, "gid_t");
}

bool
major_to_chars (major_t v, char *p, size_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, 0, p, s, "major_t");
}

bool
minor_to_chars (minor_t v, char *p, size_t s)
{
  return to_chars (v < 0, (uintmax_t) v, sizeof v, 0, p, s, "minor_t");
}



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
      || file->stat_info->stat.st_size < (off_t)(sp.offset + sp.numbytes)
      || file->stat_info->archive_file_size < (off_t)0)
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

union block *current_block = 0;
union block *record_end = 0;
union block *
find_next_block( void )
{
   //if( current_block == record_end )
   if( current_block > record_end )
   {
      // read more data
      sprtf( "ERROR: Beyond file limits ... Aborting ...\n" );
      pgm_exit(2);
   }
   return current_block;
}


void
set_next_block_after (union block *block)
{
  while (block >= current_block)
  {
#ifdef DUMP4
      show_current_block();
#endif // DUMP4
    current_block++;
  }

  /* Do *not* flush the archive here.  If we do, the same argument to
     set_next_block_after could mean the next block (if the input record
     is exactly one block long), which is not what is intended.  */

  if (current_block > record_end)
    abort ();
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
	  TARERROR ((0, 0, _("Unexpected EOF in archive")));
	  return false;
	}
      set_next_block_after (h);
      for (i = 0; i < SPARSES_IN_SPARSE_HEADER && rc == add_ok; i++)
	rc = oldgnu_add_sparse (file, &h->sparse_header.sp[i]);
    }

  if (rc == add_fail)
    {
      TARERROR ((0, 0, _("%s: invalid sparse archive member"),
	      file->stat_info->orig_file_name));
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

off_t
current_block_ordinal (void)
{
  return record_start_block + (current_block - record_start);
}



static union block _s_block;
static bool
oldgnu_dump_header (struct tar_sparse_file *file)
{
  off_t block_ordinal = current_block_ordinal ();
  union block *blk = &_s_block;
  size_t i;

  //*** blk = start_header (file->stat_info);
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
  //*** finish_header (file->stat_info, blk, block_ordinal);

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

static bool
sparse_dump_region (struct tar_sparse_file *file, size_t i)
{
   //*** *************
   return false;
}

static bool
sparse_extract_region (struct tar_sparse_file *file, size_t i)
{
   //*** ************
   return false;
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

extern bool tar_sparse_init (struct tar_sparse_file *file);

#if 0 // 0 ********************************
extern struct tar_sparse_optab const star_optab;
extern struct tar_sparse_optab const pax_optab;

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


bool
tar_sparse_init (struct tar_sparse_file *file)
{
  memset (file, 0, sizeof *file);

  if (!sparse_select_optab (file))
    return false;

  if (file->optab->init)
    return file->optab->init (file);

  return true;
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
#endif // 0 ****************************

#define MODE_FROM_HEADER(where) mode_from_header (where, sizeof (where))
#define TIME_FROM_HEADER(where) time_from_header (where, sizeof (where))
#define GID_FROM_HEADER(where) gid_from_header (where, sizeof (where))
#define UID_FROM_HEADER(where) uid_from_header (where, sizeof (where))
#define UINTMAX_FROM_HEADER(where) uintmax_from_header (where, sizeof (where))


/* Decode things from a file HEADER block into STAT_INFO, also setting
   *FORMAT_POINTER depending on the header block format.  If
   DO_USER_GROUP, decode the user/group information (this is useful
   for extraction, but waste time when merely listing).

   read_header() has already decoded the checksum and length, so we don't.

   This routine should *not* be called twice for the same block, since
   the two calls might use different DO_USER_GROUP values and thus
   might end up with different uid/gid for the two calls.  If anybody
   wants the uid/gid they should decode it first, and other callers
   should decode it without uid/gid before calling a routine,
   e.g. print_header, that assumes decoded data.  */
void
decode_header (union block *header, struct tar_stat_info *stat_info,
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
  
  xheader_decode (stat_info);

  if (sparse_member_p (stat_info))
    {
      //** sparse_fixup_header (stat_info);
      stat_info->is_sparse = true;
    }
  else
    {
      stat_info->is_sparse = false;
      if (((current_format == GNU_FORMAT
	    || current_format == OLDGNU_FORMAT)
	   && current_header->header.typeflag == GNUTYPE_DUMPDIR)
          || stat_info->dumpdir)
	stat_info->is_dumpdir = true;
    }

  //*** transform_member_name (&stat_info->file_name, xform_regfile);
}

enum read_header
tar_checksum (union block *header, bool silent)
{
  size_t i;
  int unsigned_sum = 0;		/* the POSIX one :-) */
  int signed_sum = 0;		/* the Sun one :-( */
  uintmax_t recorded_sum;
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


// eof - DumpTar2.c

int utc_option = 0;


char const *
tartime (struct timespec t, bool full_time)
{
  enum { fraclen = sizeof ".FFFFFFFFF" - 1 };
  static char buffer[max (UINTMAX_STRSIZE_BOUND + 1,
			  INT_STRLEN_BOUND (int) + 16)
		     + fraclen];
  struct tm *tm;
  time_t s = t.tv_sec;
  int ns = t.tv_nsec;
  bool negative = s < 0;
  char *p;

  if ( s >= 0xffffffff )
  {
     sprintf(buffer, "<invalid time (%#x)>", s );
     return buffer;
  }
  if (negative && ns != 0)
    {
      s++;
      ns = 1000000000 - ns;
    }

  tm = utc_option ? gmtime (&s) : localtime (&s);
  if (tm)
    {
      if (full_time)
	{
	  sprintf (buffer, "%04ld-%02d-%02d %02d:%02d:%02d",
		   tm->tm_year + 1900L, tm->tm_mon + 1, tm->tm_mday,
		   tm->tm_hour, tm->tm_min, tm->tm_sec);
	  code_ns_fraction (ns, buffer + strlen (buffer));
	}
      else
	sprintf (buffer, "%04ld-%02d-%02d %02d:%02d",
		 tm->tm_year + 1900L, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min);
      return buffer;
    }

  /* The time stamp cannot be broken down, most likely because it
     is out of range.  Convert it as an integer,
     right-adjusted in a field with the same width as the usual
     4-year ISO time format.  */
  //p = umaxtostr (negative ? - (uintmax_t) s : s,
  p = umaxtostr (negative ? - (uintmax_t) (inttype)s : (inttype)s,
		 buffer + sizeof buffer - UINTMAX_STRSIZE_BOUND - fraclen);
  if (negative)
    *--p = '-';
  while ((buffer + sizeof buffer - sizeof "YYYY-MM-DD HH:MM"
	  + (full_time ? sizeof ":SS.FFFFFFFFF" - 1 : 0))
	 < p)
    *--p = ' ';
  if (full_time)
    code_ns_fraction (ns, buffer + sizeof buffer - 1 - fraclen);
  return p;
}

#if 0
// ==========================================================

struct _obstack_chunk		/* Lives at front of each chunk. */
{
  char  *limit;			/* 1 past end of this chunk */
  struct _obstack_chunk *prev;	/* address of prior chunk or NULL */
  char	contents[4];		/* objects begin here */
};

# define PTR_INT_TYPE ptrdiff_t

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

#endif   // 0

// ******************************************************************
// other stuff extracted

// xheader stuff 

#if 0 // *******************************************************
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

#define __BPTR_ALIGN(B, P, A) ((B) + (((P) - (B) + (A)) & ~(A)))

#define __PTR_ALIGN(B, P, A)						    \
  __BPTR_ALIGN (sizeof (PTR_INT_TYPE) < sizeof (void *) ? (B) : (char *) 0, \
		P, A)


# define CALL_CHUNKFUN(h, size) \
  (((h) -> use_extra_arg) \
   ? (*(h)->chunkfun) ((h)->extra_arg, (size)) \
   : (*(struct _obstack_chunk *(*) (long)) (h)->chunkfun) ((size)))

#endif // 0 *********************************************************

static void print_and_abort (void)
{
   printf( "FAILED ... Aborting ...\n" );
   exit(2);
}

void (*obstack_alloc_failed_handler) (void) = print_and_abort;

#if 0 // **************************
# ifndef obstack_chunk_alloc
#  define obstack_chunk_alloc malloc
# endif
# ifndef obstack_chunk_free
#  define obstack_chunk_free free
# endif
#endif // 0 ************************

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

/* To prevent prototype warnings provide complete argument list.  */
#define obstack_init(h)						\
  _obstack_begin ((h), 0, 0,					\
		  (void *(*) (long)) obstack_chunk_alloc,	\
		  (void (*) (void *)) obstack_chunk_free)

/* Keyword options */

/* List of keyword/value pairs decoded from the last 'g' type header */
static struct keyword_list *global_header_override_list;

//#define strtoumax _strtoui64

/* Decode a single extended header record, advancing *PTR to the next record.
   Return true on success, false otherwise.  */
static bool
decode_record (struct xheader *xhdr,
	       char **ptr,
	       void (*handler) (void *, char const *, char const *, size_t),
	       void *data)
{
  char *start = *ptr;
  char *p = start;
  uintmax_t u;
  size64_t len;
  char *len_lim;
  char const *keyword;
  char *nextp;
  size_t len_max = xhdr->buffer + xhdr->size - start;

  while (*p == ' ' || *p == '\t')
    p++;

  if (! ISDIGIT (*p))
    {
      if (*p)
	TARERROR ((0, 0, _("Malformed extended header: missing length")));
      return false;
    }

  errno = 0;
  len = u = strtoumax (p, &len_lim, 10);
  //if (len != u || errno == ERANGE)
  //  {
  //    TARERROR ((0, 0, _("Extended header length is out of allowed range")));
  //    return false;
  //  }

  if (len_max < len)
    {
      int len_len = len_lim - p;
      TARERROR ((0, 0, _("Extended header length %*s is out of range"),
	      len_len, p));
      return false;
    }

  nextp = start + len;

  for (p = len_lim; *p == ' ' || *p == '\t'; p++)
    continue;
  if (p == len_lim)
    {
      TARERROR ((0, 0,
	      _("Malformed extended header: missing blank after length")));
      return false;
    }

  keyword = p;
  p = strchr (p, '=');
  if (! (p && p < nextp))
    {
      TARERROR ((0, 0, _("Malformed extended header: missing equal sign")));
      return false;
    }

  if (nextp[-1] != '\n')
    {
      TARERROR ((0, 0, _("Malformed extended header: missing newline")));
      return false;
    }

  *p = nextp[-1] = '\0';
  handler (data, keyword, p + 1, nextp - p - 2); /* '=' + trailing '\n' */
  *p = '=';
  nextp[-1] = '\n';
  *ptr = nextp;
  return true;
}


static void
xheader_list_destroy (struct keyword_list **root)
{
  if (root)
    {
      struct keyword_list *kw = *root;
      while (kw)
	{
	  struct keyword_list *next = kw->next;
	  free (kw->pattern);
	  free (kw->value);
	  free (kw);
	  kw = next;
	}
      *root = NULL;
    }
}

static void
xheader_list_append (struct keyword_list **root, char const *kw,
		     char const *value)
{
  struct keyword_list *kp = xmalloc (sizeof *kp);
  kp->pattern = xstrdup (kw);
  kp->value = value ? xstrdup (value) : NULL;
  kp->next = *root;
  *root = kp;
}

static void
decg (void *data, char const *keyword, char const *value,
      size_t size __attribute__((unused)))
{
  struct keyword_list **kwl = data;
  xheader_list_append (kwl, keyword, value);
}

#if 0 // **************************************************
void
xheader_decode_global (struct xheader *xhdr)
{
  if (xhdr->size)
    {
      char *p = xhdr->buffer + BLOCKSIZE;

      xheader_list_destroy (&global_header_override_list);
      while (decode_record (xhdr, &p, decg, &global_header_override_list))
	continue;
    }
}
#endif // 0 ***********************************************

size_t
available_space_after (union block *pointer)
{
  return record_end->buffer - pointer->buffer;
}

void xheader_read (struct xheader *xhdr, union block *p, size_t size);
void xheader_destroy (struct xheader *xhdr);


enum read_header
read_header_primitive (bool raw_extended_headers, struct tar_stat_info *info)
{
  union block *header;
  union block *header_copy;
  char *bp;
  union block *data_block;
  size64_t size, written;
  union block *next_long_name = 0;
  union block *next_long_link = 0;
  size64_t next_long_name_blocks = 0;
  size64_t next_long_link_blocks = 0;

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
	      size64_t name_size = info->stat.st_size;
	      size64_t n = name_size % BLOCKSIZE;
	      size = name_size + BLOCKSIZE;
	      if (n)
		size += BLOCKSIZE - n;

	      if (name_size != info->stat.st_size || size < name_size)
		xalloc_die ();

	      header_copy = xmalloc ((size_t)(size + 1));

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

		  memcpy (bp, data_block->buffer, (size_t)written);
		  bp += written;
		  set_next_block_after ((union block *)
					(data_block->buffer + written - 1));
		}

	      *bp = '\0';
	    }
	  else if (header->header.typeflag == XHDTYPE
		   || header->header.typeflag == SOLARIS_XHDTYPE)
	    xheader_read (&info->xhdr, header, (size_t)
			  OFF_FROM_HEADER (header->header.size));
	  else if (header->header.typeflag == XGLTYPE)
	    {
	      struct xheader xhdr;
	      memset (&xhdr, 0, sizeof xhdr);
	      xheader_read (&xhdr, header, (size_t)
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

#if 0 // *******************************************************
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

#endif // 0 ******************************************************

/* Actually print it.

   Plain and fancy file header block logging.  Non-verbose just prints
   the name, e.g. for "tar t" or "tar x".  This should just contain
   file names, so it can be fed back into tar with xargs or the "-T"
   option.  The verbose option can give a bunch of info, one line per
   file.  I doubt anybody tries to parse its format, or if they do,
   they shouldn't.  Unix tar is pretty random here anyway.  */


/* FIXME: Note that print_header uses the globals HEAD, HSTAT, and
   HEAD_STANDARD, which must be set up in advance.  Not very clean..  */

/* Width of "user/group size", with initial value chosen
   heuristically.  This grows as needed, though this may cause some
   stairstepping in the output.  Make it too small and the output will
   almost always look ragged.  Make it too large and the output will
   be spaced out too far.  */
static int ugswidth = 19;

/* Width of printed time stamps.  It grows if longer time stamps are
   found (typically, those with nanosecond resolution).  Like
   USGWIDTH, some stairstepping may occur.  */
static int datewidth = sizeof "YYYY-MM-DD HH:MM" - 1;

int test_label_option = 0;
int show_transformed_names_option = 0;
int block_number_option = 1;
#define  stdlis stderr

char * quotearg (char * temp_name)
{
   return temp_name;
}

void
pax_decode_mode (mode_t mode, char *string)
{
  *string++ = mode & S_IRUSR ? 'r' : '-';
  *string++ = mode & S_IWUSR ? 'w' : '-';
  *string++ = (mode & S_ISUID
	       ? (mode & S_IXUSR ? 's' : 'S')
	       : (mode & S_IXUSR ? 'x' : '-'));
  *string++ = mode & S_IRGRP ? 'r' : '-';
  *string++ = mode & S_IWGRP ? 'w' : '-';
  *string++ = (mode & S_ISGID
	       ? (mode & S_IXGRP ? 's' : 'S')
	       : (mode & S_IXGRP ? 'x' : '-'));
  *string++ = mode & S_IROTH ? 'r' : '-';
  *string++ = mode & S_IWOTH ? 'w' : '-';
  *string++ = (mode & S_ISVTX
	       ? (mode & S_IXOTH ? 't' : 'T')
	       : (mode & S_IXOTH ? 'x' : '-'));
  *string = '\0';
}

#if 0 // 0 **********************************
#ifndef GOT_MAJOR
# define major(device)		(((device) >> 8) & 0xff)
# define minor(device)		((device) & 0xff)
# define makedev(major, minor)	(((major) << 8) | (minor))
#endif
#endif // 0 *************************

void
print_header (struct tar_stat_info *st, off_t block_ordinal)
{
  char modes[11];
  char const *time_stamp;
  int time_stamp_len;
  char *temp_name;

  /* These hold formatted ints.  */
  char uform[UINTMAX_STRSIZE_BOUND], gform[UINTMAX_STRSIZE_BOUND];
  char *user, *group;
  char size[2 * UINTMAX_STRSIZE_BOUND];
  				/* holds formatted size or major,minor */
  char uintbuf[UINTMAX_STRSIZE_BOUND];
  char buf[UINTMAX_STRSIZE_BOUND];
  int pad;
  int sizelen;

  buf[0] = 0;
  if (test_label_option && current_header->header.typeflag != GNUTYPE_VOLHDR)
    return;

  if (show_transformed_names_option)
    temp_name = st->file_name ? st->file_name : st->orig_file_name;
  else
    temp_name = st->orig_file_name ? st->orig_file_name : st->file_name;

  if (block_number_option)
    {
      if (block_ordinal < 0)
	block_ordinal = current_block_ordinal ();
      block_ordinal -= (size_t)recent_long_name_blocks;
      block_ordinal -= (size_t)recent_long_link_blocks;
#ifdef DUMP4
      sprtf ( "Block %d: ", (block_ordinal+1) );
#else /* !DUMP4 */
      fprintf (stdlis, _("block %s: "), STRINGIFY_BIGINT (block_ordinal, buf));
#endif /* DUMP4 y/n */
    }

  if (verbose_option <= 1)
    {
      /* Just the fax, mam.  */
#ifdef DUMP4
      sprtf ( "%s\n", temp_name);
#else /* !DUMP4 */
      fprintf (stdlis, "%s\n", quotearg (temp_name));
#endif /* DUMP4 y/n */
    }
  else
    {
      /* File type and modes.  */

      modes[0] = '?';
      switch (current_header->header.typeflag)
	{
	case GNUTYPE_VOLHDR:
	  modes[0] = 'V';
	  break;

	case GNUTYPE_MULTIVOL:
	  modes[0] = 'M';
	  break;

	case GNUTYPE_LONGNAME:
	case GNUTYPE_LONGLINK:
	  modes[0] = 'L';
	  //ERROR ((0, 0, _("Unexpected long name header")));
     sprtf( "ERROR: Unexpected long name header!\n" );
	  break;

	case GNUTYPE_SPARSE:
	case REGTYPE:
	case AREGTYPE:
	  modes[0] = '-';
	  if (temp_name[strlen (temp_name) - 1] == '/')
	    modes[0] = 'd';
	  break;
	case LNKTYPE:
	  modes[0] = 'h';
	  break;
	case GNUTYPE_DUMPDIR:
	  modes[0] = 'd';
	  break;
	case DIRTYPE:
	  modes[0] = 'd';
	  break;
	case SYMTYPE:
	  modes[0] = 'l';
	  break;
	case BLKTYPE:
	  modes[0] = 'b';
	  break;
	case CHRTYPE:
	  modes[0] = 'c';
	  break;
	case FIFOTYPE:
	  modes[0] = 'p';
	  break;
	case CONTTYPE:
	  modes[0] = 'C';
	  break;
	}

      pax_decode_mode (st->stat.st_mode, modes + 1);

      /* Time stamp.  */

      time_stamp = tartime (st->mtime, false);
      time_stamp_len = strlen (time_stamp);
      if (datewidth < time_stamp_len)
	datewidth = time_stamp_len;

      /* User and group names.  */

      if (st->uname
	  && st->uname[0]
	  && current_format != V7_FORMAT
	  && !numeric_owner_option)
	user = st->uname;
      else
	{
	  /* Try parsing it as an unsigned integer first, and as a
	     uid_t if that fails.  This method can list positive user
	     ids that are too large to fit in a uid_t.  */
	  uintmax_t u = from_header (current_header->header.uid,
				     sizeof current_header->header.uid, 0,
				     (uintmax_t) 0,
				     (uintmax_t) TYPE_MAXIMUM (uintmax_t),
				     false, false);
	  if (u != -1)
	    user = STRINGIFY_BIGINT (u, uform);
	  else
	    {
	      sprintf (uform, "%ld",
		       (long) UID_FROM_HEADER (current_header->header.uid));
	      user = uform;
	    }
	}

      if (st->gname
	  && st->gname[0]
	  && current_format != V7_FORMAT
	  && !numeric_owner_option)
	group = st->gname;
      else
	{
	  /* Try parsing it as an unsigned integer first, and as a
	     gid_t if that fails.  This method can list positive group
	     ids that are too large to fit in a gid_t.  */
	  uintmax_t g = from_header (current_header->header.gid,
				     sizeof current_header->header.gid, 0,
				     (uintmax_t) 0,
				     (uintmax_t) TYPE_MAXIMUM (uintmax_t),
				     false, false);
	  if (g != -1)
	    group = STRINGIFY_BIGINT (g, gform);
	  else
	    {
	      sprintf (gform, "%ld",
		       (long) GID_FROM_HEADER (current_header->header.gid));
	      group = gform;
	    }
	}

      /* Format the file size or major/minor device numbers.  */

      switch (current_header->header.typeflag)
	{
	case CHRTYPE:
	case BLKTYPE:
	  strcpy (size,
		  STRINGIFY_BIGINT (major (st->stat.st_rdev), uintbuf));
	  strcat (size, ",");
	  strcat (size,
		  STRINGIFY_BIGINT (minor (st->stat.st_rdev), uintbuf));
	  break;

	default:
	  /* st->stat.st_size keeps stored file size */
	  strcpy (size, STRINGIFY_BIGINT (st->stat.st_size, uintbuf));
	  break;
	}

      /* Figure out padding and print the whole line.  */

      sizelen = strlen (size);
      pad = strlen (user) + 1 + strlen (group) + 1 + sizelen;
      if (pad > ugswidth)
	ugswidth = pad;

#ifdef DUMP4
      sprtf ( "%s %s/%s %*s %-*s",
	       modes, user, group, ugswidth - pad + sizelen, size,
	       datewidth, time_stamp);
      sprtf( " %s", temp_name);
#else /* !DUMP4 */
      fprintf (stdlis, "%s %s/%s %*s %-*s",
	       modes, user, group, ugswidth - pad + sizelen, size,
	       datewidth, time_stamp);

      fprintf (stdlis, " %s", quotearg (temp_name));
#endif /* DUMP4 y/n */

      switch (current_header->header.typeflag)
	{
	case SYMTYPE:
#ifdef DUMP4
	  sprtf( " -> %s\n", st->link_name );
#else /* !DUMP4 */
	  fprintf (stdlis, " -> %s\n", quotearg (st->link_name));
#endif /* DUMP4 y/n */
	  break;

	case LNKTYPE:
#ifdef DUMP4
	  sprtf (" link to %s\n", st->link_name );
#else /* !DUMP4 */
	  fprintf (stdlis, _(" link to %s\n"), quotearg (st->link_name));
#endif /* DUMP4 y/n */
	  break;

	default:
	  {
	    char type_string[2];
	    type_string[0] = current_header->header.typeflag;
	    type_string[1] = '\0';
#ifdef DUMP4
	    sprtf(" unknown file type %s\n", type_string);
#else /* !DUMP4 */
	    fprintf (stdlis, _(" unknown file type %s\n"), quote (type_string));
#endif /* DUMP4 y/n */
	  }
	  break;

	case AREGTYPE:
	case REGTYPE:
	case GNUTYPE_SPARSE:
	case CHRTYPE:
	case BLKTYPE:
	case DIRTYPE:
	case FIFOTYPE:
	case CONTTYPE:
	case GNUTYPE_DUMPDIR:
#ifdef DUMP4
	  sprtf( "\n" );
#else /* !DUMP4 */
	  putc ('\n', stdlis);
#endif /* DUMP4 y/n */
	  break;

	case GNUTYPE_LONGLINK:
#ifdef DUMP4
	  sprtf("--Long Link--\n");
#else /* !DUMP4 */
	  fprintf (stdlis, _("--Long Link--\n"));
#endif /* DUMP4 y/n */
	  break;

	case GNUTYPE_LONGNAME:
#ifdef DUMP4
	  sprtf( "--Long Name--\n" );
#else /* !DUMP4 */
	  fprintf (stdlis, _("--Long Name--\n"));
#endif /* DUMP4 y/n */
	  break;

	case GNUTYPE_VOLHDR:
#ifdef DUMP4
	  sprtf( "--Volume Header--\n" );
#else /* !DUMP4 */
	  fprintf (stdlis, _("--Volume Header--\n"));
#endif /* DUMP4 y/n */
	  break;

	case GNUTYPE_MULTIVOL:
	  strcpy (size,
		  STRINGIFY_BIGINT
		  (UINTMAX_FROM_HEADER (current_header->oldgnu_header.offset),
		   uintbuf));
#ifdef DUMP4
	  sprtf( "--Continued at byte %s--\n", size );
#else /* !DUMP4 */
	  fprintf (stdlis, _("--Continued at byte %s--\n"), size);
#endif /* DUMP4 y/n */
	  break;
	}
    }
#ifdef DUMP4
#else /* !DUMP4 */
  fflush (stdlis);
#endif /* DUMP4 y/n */
}

// EOF

