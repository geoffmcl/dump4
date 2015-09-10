
// DumpTarX.c
#pragma warning(disable:4146) // C4146: unary minus operator applied to unsigned type, result still unsigned
#include "dump4.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "DumpTar2.h"
#include <math.h>

extern void * xmalloc (size_t n);
extern char * xstrdup (char const *string);
extern void assign_string (char **string, const char *value);
typedef __int64 inttype;
extern char * umaxtostr (inttype i, char *buf);
extern int getpid(void);
extern union block * find_next_block( void );
extern void set_next_block_after (union block *block);
extern void xheader_destroy (struct xheader *xhdr);
extern void xheader_init (struct xheader *xhdr);
extern void xheader_finish (struct xheader *xhdr);
extern void * xcalloc (size_t n, size_t s);
extern union block * start_private_header (const char *name, size_t size);

/* List of keyword patterns set by delete= option */
static struct keyword_list *keyword_pattern_list;

/* List of keyword/value pairs set by `keyword=value' option */
static struct keyword_list *keyword_global_override_list;

/* List of keyword/value pairs set by `keyword:=value' option */
static struct keyword_list *keyword_override_list;

/* List of keyword/value pairs decoded from the last 'g' type header */
static struct keyword_list *global_header_override_list;

/* Template for the name field of an 'x' type header */
static char *exthdr_name;

/* Template for the name field of a 'g' type header */
static char *globexthdr_name;
//#define __attribute__(a)

int absolute_names_option = 0;

static bool xheader_protected_pattern_p (char const *pattern);
static bool xheader_protected_keyword_p (char const *keyword);
//static void xheader_set_single_keyword (char *) __attribute__ ((noreturn));

/* Used by xheader_finish() */
static void code_string (char const *string, char const *keyword,
			 struct xheader *xhdr);

/* Number of global headers written so far. */
static size_t global_header_count;
/* FIXME: Possibly it should be reset after changing the volume.
   POSIX %n specification says that it is expanded to the sequence
   number of current global header in *the* archive. However, for
   multi-volume archives this will yield duplicate header names
   in different volumes, which I'd like to avoid. The best way
   to solve this would be to use per-archive header count as required
   by POSIX *and* set globexthdr.name to, say,
   $TMPDIR/GlobalHead.%p.$NUMVOLUME.%n.

   However it should wait until buffer.c is finally rewritten */


/* Interface functions to obstacks */

static void
x_obstack_grow (struct xheader *xhdr, const char *ptr, size_t length)
{
  obstack_grow (xhdr->stk, ptr, length);
  xhdr->size += length;
}

static void
x_obstack_1grow (struct xheader *xhdr, char c)
{
  obstack_1grow (xhdr->stk, c);
  xhdr->size++;
}

static void
x_obstack_blank (struct xheader *xhdr, size_t length)
{
  obstack_blank (xhdr->stk, length);
  xhdr->size += length;
}


/* Keyword options */
#define fnmatch(a,b,c) strcmp(a,b)

bool
xheader_keyword_deleted_p (const char *kw)
{
  struct keyword_list *kp;

  for (kp = keyword_pattern_list; kp; kp = kp->next)
    if (fnmatch (kp->pattern, kw, 0) == 0)
      return true;
  return false;
}

static bool
xheader_keyword_override_p (const char *keyword)
{
  struct keyword_list *kp;

  for (kp = keyword_override_list; kp; kp = kp->next)
    if (strcmp (kp->pattern, keyword) == 0)
      return true;
  return false;
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
xheader_set_single_keyword (char *kw)
{
  //USAGE_ERROR ((0, 0, _("Keyword %s is unknown or not yet implemented"), kw));
   sprtf( "ERROR: Keyword %s is unknown or not yet implemented", kw );
   pgm_exit(2);
}

static void
xheader_set_keyword_equal (char *kw, char *eq)
{
  bool global = true;
  char *p = eq;

  if (eq[-1] == ':')
    {
      p--;
      global = false;
    }

  while (p > kw && isspace (*p))
    p--;

  *p = 0;

  for (p = eq + 1; *p && isspace (*p); p++)
    ;

  if (strcmp (kw, "delete") == 0)
    {
      if (xheader_protected_pattern_p (p))
      {
      	//USAGE_ERROR ((0, 0, _("Pattern %s cannot be used"), quote (p)));
         sprtf( "ERROR: Pattern %s cannot be used.\n", p );
         pgm_exit(2);
      }
      xheader_list_append (&keyword_pattern_list, p, NULL);
    }
  else if (strcmp (kw, "exthdr.name") == 0)
    assign_string (&exthdr_name, p);
  else if (strcmp (kw, "globexthdr.name") == 0)
    assign_string (&globexthdr_name, p);
  else
    {
      if (xheader_protected_keyword_p (kw))
      {
         //USAGE_ERROR ((0, 0, _("Keyword %s cannot be overridden"), kw));
         sprtf( "ERROR: Keyword %s cannot be overridden", kw );
         pgm_exit(2);
      }
      if (global)
	xheader_list_append (&keyword_global_override_list, kw, p);
      else
	xheader_list_append (&keyword_override_list, kw, p);
    }
}

void
xheader_set_option (char *string)
{
  char *token;
  for (token = strtok (string, ","); token; token = strtok (NULL, ","))
    {
      char *p = strchr (token, '=');
      if (!p)
	xheader_set_single_keyword (token);
      else
	xheader_set_keyword_equal (token, p);
    }
}

char *
dir_name (char const *file)
{
   char * dir = xmalloc( strlen(file) + 1 );
   return dir;
}

char *
safer_name_suffix (char const *file_name, bool link_target,
		   bool absolute_names)
{
  char *p = (char *)file_name;
  return p;
}

#   define _IS_DRIVE_LETTER(c) (((unsigned int) (c) | ('a' - 'A')) - 'a' \
				<= 'z' - 'a')
#   define FILE_SYSTEM_PREFIX_LEN(Filename) \
	   (_IS_DRIVE_LETTER ((Filename)[0]) && (Filename)[1] == ':' ? 2 : 0)

//# define ISSLASH(C) ((C) == '/' || (C) == '\\')

char *
last_component (char const *name)
{
  char const *base = name + FILE_SYSTEM_PREFIX_LEN (name);
  char const *p;
  bool saw_slash = false;

  while (ISSLASH (*base))
    base++;

  for (p = base; *p; p++)
    {
      if (ISSLASH (*p))
	saw_slash = true;
      else if (saw_slash)
	{
	  base = p;
	  saw_slash = false;
	}
    }

  return (char *) base;
}

char *
stpcpy (char *dest, const char *src)
{
  register char *d = dest;
  register const char *s = src;

  do
    *d++ = *s;
  while (*s++ != '\0');

  return d - 1;
}

/*
    string Includes:          Replaced By:
     %d                       The directory name of the file,
                              equivalent to the result of the
                              dirname utility on the translated
                              file name.
     %f                       The filename of the file, equivalent
                              to the result of the basename
                              utility on the translated file name.
     %p                       The process ID of the pax process.
     %n                       The value of the 3rd argument.
     %%                       A '%' character. */

char *
xheader_format_name (struct tar_stat_info *st, const char *fmt, size_t n)
{
  char *buf;
  size_t len = strlen (fmt);
  char *q;
  const char *p;
  char *dirp = NULL;
  char *dir = NULL;
  char *base = NULL;
  char pidbuf[UINTMAX_STRSIZE_BOUND];
  char const *pptr;
  char nbuf[UINTMAX_STRSIZE_BOUND];
  char const *nptr = NULL;

  for (p = fmt; *p && (p = strchr (p, '%')); )
    {
      switch (p[1])
	{
	case '%':
	  len--;
	  break;

	case 'd':
	  if (st)
	    {
	      if (!dirp)
		dirp = dir_name (st->orig_file_name);
	      dir = safer_name_suffix (dirp, false, absolute_names_option);
	      len += strlen (dir) - 2;
	    }
	  break;

	case 'f':
	  if (st)
	    {
	      base = last_component (st->orig_file_name);
	      len += strlen (base) - 2;
	    }
	  break;

	case 'p':
	  pptr = umaxtostr (getpid (), pidbuf);
	  len += pidbuf + sizeof pidbuf - 1 - pptr - 2;
	  break;

	case 'n':
	  nptr = umaxtostr (n, nbuf);
	  len += nbuf + sizeof nbuf - 1 - nptr - 2;
	  break;
	}
      p++;
    }

  buf = xmalloc (len + 1);
  for (q = buf, p = fmt; *p; )
    {
      if (*p == '%')
	{
	  switch (p[1])
	    {
	    case '%':
	      *q++ = *p++;
	      p++;
	      break;

	    case 'd':
	      if (dir)
		q = stpcpy (q, dir);
	      p += 2;
	      break;

	    case 'f':
	      if (base)
		q = stpcpy (q, base);
	      p += 2;
	      break;

	    case 'p':
	      q = stpcpy (q, pptr);
	      p += 2;
	      break;

	    case 'n':
	      if (nptr)
		{
		  q = stpcpy (q, nptr);
		  p += 2;
		  break;
		}
	      /* else fall through */

	    default:
	      *q++ = *p++;
	      if (*p)
		*q++ = *p++;
	    }
	}
      else
	*q++ = *p++;
    }

  free (dirp);

  /* Do not allow it to end in a slash */
  while (q > buf && ISSLASH (q[-1]))
    q--;
  *q = 0;
  return buf;
}

char *
xheader_xhdr_name (struct tar_stat_info *st)
{
  if (!exthdr_name)
    assign_string (&exthdr_name, "%d/PaxHeaders.%p/%f");
  return xheader_format_name (st, exthdr_name, 0);
}

#define GLOBAL_HEADER_TEMPLATE "/GlobalHead.%p.%n"

char *
xheader_ghdr_name (void)
{
  if (!globexthdr_name)
    {
      size_t len;
      const char *tmp = getenv ("TMPDIR");
      if (!tmp)
	tmp = "/tmp";
      len = strlen (tmp) + sizeof (GLOBAL_HEADER_TEMPLATE); /* Includes nul */
      globexthdr_name = xmalloc (len);
      strcpy(globexthdr_name, tmp);
      strcat(globexthdr_name, GLOBAL_HEADER_TEMPLATE);
    }

  return xheader_format_name (NULL, globexthdr_name, global_header_count + 1);
}

#if 0 // 0 **********************************
union block *
start_private_header( char * name, size_t size )
{
   union block * h = xmalloc(size);
   return h;
}
#endif // 0 ***********************************

void
xheader_write (char type, char *name, struct xheader *xhdr)
{
  union block *header;
  size_t size;
  char *p;

  size = xhdr->size;
  // ***TBD*** header = start_private_header (name, size);
  header = start_private_header (name, size);
  header->header.typeflag = type;

  // ***TBD*** simple_finish_header (header);

  p = xhdr->buffer;

  do
    {
      size_t len;

      header = find_next_block ();
      len = BLOCKSIZE;
      if (len > size)
	len = size;
      memcpy (header->buffer, p, len);
      if (len < BLOCKSIZE)
	memset (header->buffer + len, 0, BLOCKSIZE - len);
      p += len;
      size -= len;
      set_next_block_after (header);
    }
  while (size > 0);
  xheader_destroy (xhdr);

  if (type == XGLTYPE)
    global_header_count++;
}

void
xheader_write_global (struct xheader *xhdr)
{
  char *name = 0;
  struct keyword_list *kp;

  if (!keyword_global_override_list)
    return;

  xheader_init (xhdr);
  for (kp = keyword_global_override_list; kp; kp = kp->next)
    code_string (kp->value, kp->pattern, xhdr);
  xheader_finish (xhdr);
  // ***TBD*** xheader_write (XGLTYPE, name = xheader_ghdr_name (), xhdr);
  if(name)
     free (name);
}


/* General Interface */
#if 0 // ********************************************************
struct xhdr_tab
{
  char const *keyword;
  void (*coder) (struct tar_stat_info const *, char const *,
		 struct xheader *, void const *data);
  void (*decoder) (struct tar_stat_info *, char const *, char const *, size_t);
  bool protect;
};

#endif // ********************************************************

/* This declaration must be extern, because ISO C99 section 6.9.2
   prohibits a tentative definition that has both internal linkage and
   incomplete type.  If we made it static, we'd have to declare its
   size which would be a maintenance pain; if we put its initializer
   here, we'd need a boatload of forward declarations, which would be
   even more of a pain.  */
extern struct xhdr_tab const xhdr_tab[];

static struct xhdr_tab const *
locate_handler (char const *keyword)
{
  struct xhdr_tab const *p;

  for (p = xhdr_tab; p->keyword; p++)
    if (strcmp (p->keyword, keyword) == 0)
      return p;
  return NULL;
}

static bool
xheader_protected_pattern_p (const char *pattern)
{
  struct xhdr_tab const *p;

  for (p = xhdr_tab; p->keyword; p++)
    if (p->protect && fnmatch (pattern, p->keyword, 0) == 0)
      return true;
  return false;
}

static bool
xheader_protected_keyword_p (const char *keyword)
{
  struct xhdr_tab const *p;

  for (p = xhdr_tab; p->keyword; p++)
    if (p->protect && strcmp (p->keyword, keyword) == 0)
      return true;
  return false;
}

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
      if (*p) //ERROR ((0, 0, _("Malformed extended header: missing length")));
         sprtf( "ERROR: Malformed extended header: missing length\n");
      return false;
    }

  errno = 0;
  len = u = strtoumax (p, &len_lim, 10);
  if (len != u || errno == ERANGE)
    {
      //ERROR ((0, 0, _("Extended header length is out of allowed range")));
       sprtf( "ERROR: Extended header length is out of allowed range!\n" );
      return false;
    }

  if (len_max < len)
    {
      int len_len = len_lim - p;
      //ERROR ((0, 0, _("Extended header length %*s is out of range"),
	   //   len_len, p));
      sprtf( "ERROR: Extended header length %*s is out of range!\n" ,
	      len_len, p );
      return false;
    }

  nextp = start + len;

  for (p = len_lim; *p == ' ' || *p == '\t'; p++)
    continue;
  if (p == len_lim)
    {
      //ERROR ((0, 0,
	   //   _("Malformed extended header: missing blank after length")));
       sprtf( "ERROR: Malformed extended header: missing blank after length!\n");
      return false;
    }

  keyword = p;
  p = strchr (p, '=');
  if (! (p && p < nextp))
    {
      //ERROR ((0, 0, _("Malformed extended header: missing equal sign")));
       sprtf( "ERROR: Malformed extended header: missing equal sign!\n" );
      return false;
    }

  if (nextp[-1] != '\n')
    {
      //ERROR ((0, 0, _("Malformed extended header: missing newline")));
       sprtf( "ERROR: Malformed extended header: missing newline!\n" );
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
run_override_list (struct keyword_list *kp, struct tar_stat_info *st)
{
  for (; kp; kp = kp->next)
    {
      struct xhdr_tab const *t = locate_handler (kp->pattern);
      if (t)
	t->decoder (st, t->keyword, kp->value, strlen (kp->value));
    }
}

static void
decx (void *data, char const *keyword, char const *value, size_t size)
{
  struct xhdr_tab const *t;
  struct tar_stat_info *st = data;

  if (xheader_keyword_deleted_p (keyword)
      || xheader_keyword_override_p (keyword))
    return;

  t = locate_handler (keyword);
  if (t)
    t->decoder (st, keyword, value, size);
  else
  {
    //WARN((0, 0, _("Ignoring unknown extended header keyword `%s'"),
	 //keyword));
     sprtf("WARNING: Ignoring unknown extended header keyword '%s'\n",
        keyword);
  }
}

void
xheader_decode (struct tar_stat_info *st)
{
  run_override_list (keyword_global_override_list, st);
  run_override_list (global_header_override_list, st);

  if (st->xhdr.size)
    {
      char *p = st->xhdr.buffer + BLOCKSIZE;
      while (decode_record (&st->xhdr, &p, decx, st))
	continue;
    }
  run_override_list (keyword_override_list, st);
}

static void
decg (void *data, char const *keyword, char const *value,
      size_t size __attribute__((unused)))
{
  struct keyword_list **kwl = data;
  xheader_list_append (kwl, keyword, value);
}

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

static void
xheader_print_n (struct xheader *xhdr, char const *keyword,
		 char const *value, size_t vsize)
{
  size_t len = strlen (keyword) + vsize + 3; /* ' ' + '=' + '\n' */
  size_t p;
  size_t n = 0;
  char nbuf[UINTMAX_STRSIZE_BOUND];
  char const *np;

  do
    {
      p = n;
      np = umaxtostr (len + p, nbuf);
      n = nbuf + sizeof nbuf - 1 - np;
    }
  while (n != p);

  x_obstack_grow (xhdr, np, n);
  x_obstack_1grow (xhdr, ' ');
  x_obstack_grow (xhdr, keyword, strlen (keyword));
  x_obstack_1grow (xhdr, '=');
  x_obstack_grow (xhdr, value, vsize);
  x_obstack_1grow (xhdr, '\n');
}

static void
xheader_print (struct xheader *xhdr, char const *keyword, char const *value)
{
  xheader_print_n (xhdr, keyword, value, strlen (value));
}

void
xheader_finish (struct xheader *xhdr)
{
  struct keyword_list *kp;

  for (kp = keyword_override_list; kp; kp = kp->next)
    code_string (kp->value, kp->pattern, xhdr);

  xhdr->buffer = obstack_finish (xhdr->stk);
}

void
xheader_destroy (struct xheader *xhdr)
{
  if (xhdr->stk)
    {
      obstack_free (xhdr->stk, NULL);
      free (xhdr->stk);
      xhdr->stk = NULL;
    }
  else
    free (xhdr->buffer);
  xhdr->buffer = 0;
  xhdr->size = 0;
}


/* Buildable strings */

void
xheader_string_begin (struct xheader *xhdr)
{
  xhdr->string_length = 0;
}

void
xheader_string_add (struct xheader *xhdr, char const *s)
{
  if (xhdr->buffer)
    return;
  xheader_init (xhdr);
  xhdr->string_length += strlen (s);
  x_obstack_grow (xhdr, s, strlen (s));
}

bool
xheader_string_end (struct xheader *xhdr, char const *keyword)
{
  uintmax_t len;
  uintmax_t p;
  uintmax_t n = 0;
  size_t size;
  char nbuf[UINTMAX_STRSIZE_BOUND];
  char const *np;
  char *cp;

  if (xhdr->buffer)
    return false;
  xheader_init (xhdr);

  len = strlen (keyword) + xhdr->string_length + 3; /* ' ' + '=' + '\n' */

  do
    {
      p = n;
      np = umaxtostr (len + p, nbuf);
      n = nbuf + sizeof nbuf - 1 - np;
    }
  while (n != p);

  p = strlen (keyword) + n + 2;
  size = (size_t)p;
  if (size != p)
    {
      //ERROR ((0, 0,
      //  _("Generated keyword/value pair is too long (keyword=%s, length=%s)"),
	   //   keyword, nbuf));
       sprtf( "ERROR: Generated keyword/value pair is too long (keyword=%s, length=%s)",
          keyword, nbuf);
      obstack_free (xhdr->stk, obstack_finish (xhdr->stk));
      return false;
    }
  x_obstack_blank (xhdr, (size_t)p);
  x_obstack_1grow (xhdr, '\n');
  cp = obstack_next_free (xhdr->stk) - xhdr->string_length - p - 1;
  memmove (cp + p, cp, (size_t)xhdr->string_length);
  cp = stpcpy (cp, np);
  *cp++ = ' ';
  cp = stpcpy (cp, keyword);
  *cp++ = '=';
  return true;
}


/* Implementations */

static void
out_of_range_header (char const *keyword, char const *value,
		     uintmax_t minus_minval, uintmax_t maxval)
{
  char minval_buf[UINTMAX_STRSIZE_BOUND + 1];
  char maxval_buf[UINTMAX_STRSIZE_BOUND];
  char *minval_string = umaxtostr (minus_minval, minval_buf + 1);
  char *maxval_string = umaxtostr (maxval, maxval_buf);
  if (minus_minval)
    *--minval_string = '-';

  /* TRANSLATORS: The first %s is the pax extended header keyword
     (atime, gid, etc.).  */
  //ERROR ((0, 0, _("Extended header %s=%s is out of range %s..%s"),
//	  keyword, value, minval_string, maxval_string));
  sprtf( "ERROR: Extended header %s=%s is out of range %s..%s" ,
     keyword, value, minval_string, maxval_string );
}

// *****************************************************
// iconv stuff
/* Conversion descriptor type */
#define iconv_t   int

#ifdef HAVE_ICONV_H
# include <iconv.h>
#endif

#ifndef ICONV_CONST
# define ICONV_CONST
#endif

#ifndef HAVE_ICONV

# undef iconv_open
# define iconv_open(tocode, fromcode) ((iconv_t) -1)

# undef iconv
# define iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft) ((size_t) 0)

# undef iconv_close
# define iconv_close(cd) 0

#endif




static iconv_t conv_desc[2] = { (iconv_t) -1, (iconv_t) -1 };

static iconv_t
utf8_init (bool to_utf)
{
  if (conv_desc[(int) to_utf] == (iconv_t) -1)
    {
      if (to_utf)
	conv_desc[(int) to_utf] = iconv_open ("UTF-8", locale_charset ());
      else
	conv_desc[(int) to_utf] = iconv_open (locale_charset (), "UTF-8");
    }
  return conv_desc[(int) to_utf];
}

bool
utf8_convert (bool to_utf, char const *input, char **output)
{
  char ICONV_CONST *ib;
  char *ob;
  size_t inlen;
  size_t outlen;
  size_t rc;
  iconv_t cd = utf8_init (to_utf);

  if (cd == 0)
    {
      *output = xstrdup (input);
      return true;
    }
  else if (cd == (iconv_t)-1)
    return false;

  inlen = strlen (input) + 1;
  outlen = inlen * MB_LEN_MAX + 1;
  ob = *output = xmalloc (outlen);
  ib = (char ICONV_CONST *) input;
  rc = iconv (cd, &ib, &inlen, &ob, &outlen);
  *ob = 0;
  return rc != -1;
}


bool
string_ascii_p (char const *p)
{
  for (; *p; p++)
    if (*p & ~0x7f)
      return false;
  return true;
}


// *****************************************************

static void
code_string (char const *string, char const *keyword, struct xheader *xhdr)
{
  char *outstr;
  if (!utf8_convert (true, string, &outstr))
    {
      /* FIXME: report error */
      outstr = xstrdup (string);
    }
  xheader_print (xhdr, keyword, outstr);
  free (outstr);
}

static void
decode_string (char **string, char const *arg)
{
  if (*string)
    {
      free (*string);
      *string = NULL;
    }
  if (!utf8_convert (false, arg, string))
    {
      /* FIXME: report error and act accordingly to --pax invalid=UTF-8 */
      assign_string (string, arg);
    }
}

enum { BILLION = 1000000000, LOG10_BILLION = 9 };
enum { TIMESPEC_STRSIZE_BOUND =
         UINTMAX_STRSIZE_BOUND + LOG10_BILLION + sizeof "-." - 1 };

/* Handling numbers.  */

/* Output fraction and trailing digits appropriate for a nanoseconds
   count equal to NS, but don't output unnecessary '.' or trailing
   zeros.  */

void
code_ns_fraction (int ns, char *p)
{
  if (ns == 0)
    *p = '\0';
  else
    {
      int i = 9;
      *p++ = '.';

      while (ns % 10 == 0)
	{
	  ns /= 10;
	  i--;
	}

      p[i] = '\0';

      for (;;)
	{
	  p[--i] = '0' + ns % 10;
	  if (i == 0)
	    break;
	  ns /= 10;
	}
    }
}

char const *
code_timespec (struct timespec t, char sbuf[TIMESPEC_STRSIZE_BOUND])
{
  time_t s = t.tv_sec;
  int ns = t.tv_nsec;
  char *np;
  bool negative = s < 0;

  if (negative && ns != 0)
    {
      s++;
      ns = BILLION - ns;
    }

  np = umaxtostr (negative ? - (uintmax_t) s : (uintmax_t) s, sbuf + 1);
  if (negative)
    *--np = '-';
  code_ns_fraction (ns, sbuf + UINTMAX_STRSIZE_BOUND);
  return np;
}

static void
code_time (struct timespec t, char const *keyword, struct xheader *xhdr)
{
  char buf[TIMESPEC_STRSIZE_BOUND];
  xheader_print (xhdr, keyword, code_timespec (t, buf));
}

enum decode_time_status
  {
    decode_time_success,
    decode_time_range,
    decode_time_bad_header
  };

static enum decode_time_status
_decode_time (struct timespec *ts, char const *arg, char const *keyword)
{
  time_t s;
  unsigned long int ns = 0;
  char *p;
  char *arg_lim;
  bool negative = *arg == '-';

  errno = 0;

  if (ISDIGIT (arg[negative]))
    {
      if (negative)
	{
	  intmax_t i = strtoimax (arg, &arg_lim, 10);
	  if (TYPE_SIGNED (time_t) ? i < TYPE_MINIMUM (time_t) : i < 0)
	    return decode_time_range;
	  s = i;
	}
      else
	{
	  uintmax_t i = strtoumax (arg, &arg_lim, 10);
	  if (TYPE_MAXIMUM (time_t) < i)
	    return decode_time_range;
	  s = i;
	}

      p = arg_lim;

      if (errno == ERANGE)
	return decode_time_range;

      if (*p == '.')
	{
	  int digits = 0;
	  bool trailing_nonzero = false;

	  while (ISDIGIT (*++p))
	    if (digits < LOG10_BILLION)
	      {
		ns = 10 * ns + (*p - '0');
		digits++;
	      }
	    else
	      trailing_nonzero |= *p != '0';

	  while (digits++ < LOG10_BILLION)
	    ns *= 10;

	  if (negative)
	    {
	      /* Convert "-1.10000000000001" to s == -2, ns == 89999999.
		 I.e., truncate time stamps towards minus infinity while
		 converting them to internal form.  */
	      ns += trailing_nonzero;
	      if (ns != 0)
		{
		  if (s == TYPE_MINIMUM (time_t))
		    return decode_time_range;
		  s--;
		  ns = BILLION - ns;
		}
	    }
	}

      if (! *p)
	{
	  ts->tv_sec = s;
	  ts->tv_nsec = ns;
	  return decode_time_success;
	}
    }

  return decode_time_bad_header;
}

static bool
decode_time (struct timespec *ts, char const *arg, char const *keyword)
{
  switch (_decode_time (ts, arg, keyword))
    {
    case decode_time_success:
      return true;
    case decode_time_bad_header:
      //ERROR ((0, 0, _("Malformed extended header: invalid %s=%s"),
	   //   keyword, arg));
       sprtf( "ERROR: Malformed extended header: invalid %s=%s" ,
          keyword, arg );
      return false;
    case decode_time_range:
      out_of_range_header (keyword, arg, - (uintmax_t) TYPE_MINIMUM (time_t),
			   TYPE_MAXIMUM (time_t));
      return false;
    }
  return true;
}



static void
code_num (uintmax_t value, char const *keyword, struct xheader *xhdr)
{
  char sbuf[UINTMAX_STRSIZE_BOUND];
  xheader_print (xhdr, keyword, umaxtostr (value, sbuf));
}

static bool
decode_num (uintmax_t *num, char const *arg, uintmax_t maxval,
	    char const *keyword)
{
  uintmax_t u;
  char *arg_lim;

  if (! (ISDIGIT (*arg)
	 && (errno = 0, u = strtoumax (arg, &arg_lim, 10), !*arg_lim)))
    {
      //ERROR ((0, 0, _("Malformed extended header: invalid %s=%s"),
	   //   keyword, arg));
       sprtf( "ERROR: Malformed extended header: invalid %s=%s" ,
          keyword, arg);
      return false;
    }

  if (! (u <= maxval && errno != ERANGE))
    {
      out_of_range_header (keyword, arg, 0, maxval);
      return false;
    }

  *num = u;
  return true;
}

static void
dummy_coder (struct tar_stat_info const *st __attribute__ ((unused)),
	     char const *keyword __attribute__ ((unused)),
	     struct xheader *xhdr __attribute__ ((unused)),
	     void const *data __attribute__ ((unused)))
{
}

static void
dummy_decoder (struct tar_stat_info *st __attribute__ ((unused)),
	       char const *keyword __attribute__ ((unused)),
	       char const *arg __attribute__ ((unused)),
	       size_t size __attribute__((unused)))
{
}

static void
atime_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_time (st->atime, keyword, xhdr);
}

static void
atime_decoder (struct tar_stat_info *st,
	       char const *keyword,
	       char const *arg,
	       size_t size __attribute__((unused)))
{
  struct timespec ts;
  if (decode_time (&ts, arg, keyword))
    st->atime = ts;
}

static void
gid_coder (struct tar_stat_info const *st, char const *keyword,
	   struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_num (st->stat.st_gid, keyword, xhdr);
}

static void
gid_decoder (struct tar_stat_info *st,
	     char const *keyword,
	     char const *arg,
	     size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (gid_t), keyword))
    st->stat.st_gid = (short)u;
}

static void
gname_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_string (st->gname, keyword, xhdr);
}

static void
gname_decoder (struct tar_stat_info *st,
	       char const *keyword __attribute__((unused)),
	       char const *arg,
	       size_t size __attribute__((unused)))
{
  decode_string (&st->gname, arg);
}

static void
linkpath_coder (struct tar_stat_info const *st, char const *keyword,
		struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_string (st->link_name, keyword, xhdr);
}

static void
linkpath_decoder (struct tar_stat_info *st,
		  char const *keyword __attribute__((unused)),
		  char const *arg,
		  size_t size __attribute__((unused)))
{
  decode_string (&st->link_name, arg);
}

static void
ctime_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_time (st->ctime, keyword, xhdr);
}

static void
ctime_decoder (struct tar_stat_info *st,
	       char const *keyword,
	       char const *arg,
	       size_t size __attribute__((unused)))
{
  struct timespec ts;
  if (decode_time (&ts, arg, keyword))
    st->ctime = ts;
}

static void
mtime_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data)
{
  struct timespec const *mtime = data;
  code_time (mtime ? *mtime : st->mtime, keyword, xhdr);
}

static void
mtime_decoder (struct tar_stat_info *st,
	       char const *keyword,
	       char const *arg,
	       size_t size __attribute__((unused)))
{
  struct timespec ts;
  if (decode_time (&ts, arg, keyword))
    st->mtime = ts;
}

static void
path_coder (struct tar_stat_info const *st, char const *keyword,
	    struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_string (st->file_name, keyword, xhdr);
}

#define DOUBLE_SLASH_IS_DISTINCT_ROOT 0
/* Define if a drive letter prefix denotes a relative path if it is not
   followed by a file name component separator. */
#define FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE   1

size_t
base_len (char const *name)
{
  size_t len;
  size_t prefix_len = FILE_SYSTEM_PREFIX_LEN (name);

  for (len = strlen (name);  1 < len && ISSLASH (name[len - 1]);  len--)
    continue;

  if (DOUBLE_SLASH_IS_DISTINCT_ROOT && len == 1
      && ISSLASH (name[0]) && ISSLASH (name[1]) && ! name[2])
    return 2;

  if (FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE && prefix_len
      && len == prefix_len && ISSLASH (name[prefix_len]))
    return prefix_len + 1;

  return len;
}

bool
strip_trailing_slashes (char *file)
{
  char *base = last_component (file);
  char *base_lim;
  bool had_slash;

  /* last_component returns "" for file system roots, but we need to turn
     `///' into `/'.  */
  if (! *base)
    base = file;
  base_lim = base + base_len (base);
  had_slash = (*base_lim != '\0');
  *base_lim = '\0';
  return had_slash;
}

static void
path_decoder (struct tar_stat_info *st,
	      char const *keyword __attribute__((unused)),
	      char const *arg,
	      size_t size __attribute__((unused)))
{
  decode_string (&st->orig_file_name, arg);
  decode_string (&st->file_name, arg);
  st->had_trailing_slash = strip_trailing_slashes (st->file_name);
}

static void
size_coder (struct tar_stat_info const *st, char const *keyword,
	    struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_num (st->stat.st_size, keyword, xhdr);
}

static void
size_decoder (struct tar_stat_info *st,
	      char const *keyword,
	      char const *arg,
	      size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (off64_t), keyword))
    st->stat.st_size = u;
}

static void
uid_coder (struct tar_stat_info const *st, char const *keyword,
	   struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_num (st->stat.st_uid, keyword, xhdr);
}

static void
uid_decoder (struct tar_stat_info *st,
	     char const *keyword,
	     char const *arg,
	     size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (uid_t), keyword))
    st->stat.st_uid = (short)u;
}

static void
uname_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data __attribute__ ((unused)))
{
  code_string (st->uname, keyword, xhdr);
}

static void
uname_decoder (struct tar_stat_info *st,
	       char const *keyword __attribute__((unused)),
	       char const *arg,
	       size_t size __attribute__((unused)))
{
  decode_string (&st->uname, arg);
}

static void
sparse_size_coder (struct tar_stat_info const *st, char const *keyword,
	     struct xheader *xhdr, void const *data)
{
  size_coder (st, keyword, xhdr, data);
}

static void
sparse_size_decoder (struct tar_stat_info *st,
		     char const *keyword,
		     char const *arg,
		     size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (off64_t), keyword))
    st->stat.st_size = u;
}

static void
sparse_numblocks_coder (struct tar_stat_info const *st, char const *keyword,
			struct xheader *xhdr,
			void const *data __attribute__ ((unused)))
{
  code_num (st->sparse_map_avail, keyword, xhdr);
}

static void
sparse_numblocks_decoder (struct tar_stat_info *st,
			  char const *keyword,
			  char const *arg,
			  size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, SIZE_MAX, keyword))
    {
      st->sparse_map_size = u;
      st->sparse_map = xcalloc ((size_t)u, sizeof st->sparse_map[0]);
      st->sparse_map_avail = 0;
    }
}

static void
sparse_offset_coder (struct tar_stat_info const *st, char const *keyword,
		     struct xheader *xhdr, void const *data)
{
  size_t const *pi = data;
  code_num (st->sparse_map[*pi].offset, keyword, xhdr);
}

static void
sparse_offset_decoder (struct tar_stat_info *st,
		       char const *keyword,
		       char const *arg,
		       size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (off_t), keyword))
    {
      if (st->sparse_map_avail < st->sparse_map_size)
	st->sparse_map[st->sparse_map_avail].offset = u;
      else
      {
	//ERROR ((0, 0, _("Malformed extended header: excess %s=%s"),
	//	"GNU.sparse.offset", arg));
         sprtf( "ERROR: Malformed extended header: excess %s=%s\n",
            "GNU.sparse.offset", arg);
      }
    }
}

static void
sparse_numbytes_coder (struct tar_stat_info const *st, char const *keyword,
		       struct xheader *xhdr, void const *data)
{
  size_t const *pi = data;
  code_num (st->sparse_map[*pi].numbytes, keyword, xhdr);
}

static void
sparse_numbytes_decoder (struct tar_stat_info *st,
			 char const *keyword,
			 char const *arg,
			 size_t size __attribute__((unused)))
{
  uintmax_t u;
  if (decode_num (&u, arg, SIZE_MAX, keyword))
    {
      if (st->sparse_map_avail < st->sparse_map_size)
	st->sparse_map[st->sparse_map_avail++].numbytes = u;
      else
      {
	//ERROR ((0, 0, _("Malformed extended header: excess %s=%s"),
	//	keyword, arg));
         sprtf( "ERROR: Malformed extended header: excess %s=%s",
            keyword, arg);
      }
    }
}

static void
sparse_map_decoder (struct tar_stat_info *st,
		    char const *keyword,
		    char const *arg,
		    size_t size __attribute__((unused)))
{
  int offset = 1;

  st->sparse_map_avail = 0;
  while (1)
    {
      uintmax_t u;
      char *delim;
      struct sp_array e;

      if (!ISDIGIT (*arg))
	{
	  //ERROR ((0, 0, _("Malformed extended header: invalid %s=%s"),
		//  keyword, arg));
      sprtf( "ERROR: Malformed extended header: invalid %s=%s" ,
         keyword, arg);
	  return;
	}

      errno = 0;
      u = strtoumax (arg, &delim, 10);
      if (offset)
	{
	  e.offset = u;
	  if (!(u == e.offset && errno != ERANGE))
	    {
	      out_of_range_header (keyword, arg, 0, TYPE_MAXIMUM (off_t));
	      return;
	    }
	}
      else
	{
	  e.numbytes = u;
	  if (!(u == e.numbytes && errno != ERANGE))
	    {
	      out_of_range_header (keyword, arg, 0, TYPE_MAXIMUM (size_t));
	      return;
	    }
	  if (st->sparse_map_avail < st->sparse_map_size)
	    st->sparse_map[st->sparse_map_avail++] = e;
	  else
	    {
	      //ERROR ((0, 0, _("Malformed extended header: excess %s=%s"),
		   //   keyword, arg));
          sprtf( "ERROR: Malformed extended header: excess %s=%s",
             keyword, arg);
	      return;
	    }
	}

      offset = !offset;

      if (*delim == 0)
	break;
      else if (*delim != ',')
	{
	  //ERROR ((0, 0,
		//  _("Malformed extended header: invalid %s: unexpected delimiter %c"),
		//  keyword, *delim));
      sprtf( "ERROR: Malformed extended header: invalid %s: unexpected delimiter %c",
         keyword, *delim);
	  return;
	}

      arg = delim + 1;
    }

  if (!offset)
  {
    //ERROR ((0, 0,
	 //   _("Malformed extended header: invalid %s: odd number of values"),
	 //   keyword));
     sprtf( "ERROR: Malformed extended header: invalid %s: odd number of values",
        keyword);
  }
}

/* Return size in bytes of the dumpdir array P */
size_t
dumpdir_size (const char *p)
{
  size_t totsize = 0;

  while (*p)
    {
      size_t size = strlen (p) + 1;
      totsize += size;
      p += size;
    }
  return totsize + 1;
}


static void
dumpdir_coder (struct tar_stat_info const *st, char const *keyword,
	       struct xheader *xhdr, void const *data)
{
  xheader_print_n (xhdr, keyword, data, dumpdir_size (data));
}

static void
dumpdir_decoder (struct tar_stat_info *st,
		 char const *keyword __attribute__((unused)),
		 char const *arg,
		 size_t size)
{
  st->dumpdir = xmalloc (size);
  memcpy (st->dumpdir, arg, size);
}

static void
volume_label_coder (struct tar_stat_info const *st, char const *keyword,
		    struct xheader *xhdr, void const *data)
{
  code_string (data, keyword, xhdr);
}

char *volume_label = 0;
char *continued_file_name = 0;
uintmax_t continued_file_size = 0;
uintmax_t continued_file_offset = 0;

static void
volume_label_decoder (struct tar_stat_info *st,
		      char const *keyword __attribute__((unused)),
		      char const *arg,
		      size_t size __attribute__((unused)))
{
  decode_string (&volume_label, arg);
}

static void
volume_size_coder (struct tar_stat_info const *st, char const *keyword,
		   struct xheader *xhdr, void const *data)
{
  off_t const *v = data;
  code_num (*v, keyword, xhdr);
}

static void
volume_size_decoder (struct tar_stat_info *st,
		     char const *keyword,
		     char const *arg, size_t size)
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (uintmax_t), keyword))
    continued_file_size = u;
}

/* FIXME: Merge with volume_size_coder */
static void
volume_offset_coder (struct tar_stat_info const *st, char const *keyword,
		     struct xheader *xhdr, void const *data)
{
  off_t const *v = data;
  code_num (*v, keyword, xhdr);
}

static void
volume_offset_decoder (struct tar_stat_info *st,
		       char const *keyword,
		       char const *arg, size_t size)
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (uintmax_t), keyword))
    continued_file_offset = u;
}

static void
volume_filename_decoder (struct tar_stat_info *st,
			 char const *keyword __attribute__((unused)),
			 char const *arg,
			 size_t size __attribute__((unused)))
{
  decode_string (&continued_file_name, arg);
}

static void
sparse_major_coder (struct tar_stat_info const *st, char const *keyword,
		    struct xheader *xhdr, void const *data)
{
  code_num (st->sparse_major, keyword, xhdr);
}

static void
sparse_major_decoder (struct tar_stat_info *st,
		      char const *keyword,
		      char const *arg,
		      size_t size)
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (unsigned), keyword))
    st->sparse_major = (size_t)u;
}

static void
sparse_minor_coder (struct tar_stat_info const *st, char const *keyword,
		      struct xheader *xhdr, void const *data)
{
  code_num (st->sparse_minor, keyword, xhdr);
}

static void
sparse_minor_decoder (struct tar_stat_info *st,
		      char const *keyword,
		      char const *arg,
		      size_t size)
{
  uintmax_t u;
  if (decode_num (&u, arg, TYPE_MAXIMUM (unsigned), keyword))
    st->sparse_minor = (size_t)u;
}

struct xhdr_tab const xhdr_tab[] = {
  { "atime",	atime_coder,	atime_decoder,	  false },
  { "comment",	dummy_coder,	dummy_decoder,	  false },
  { "charset",	dummy_coder,	dummy_decoder,	  false },
  { "ctime",	ctime_coder,	ctime_decoder,	  false },
  { "gid",	gid_coder,	gid_decoder,	  false },
  { "gname",	gname_coder,	gname_decoder,	  false },
  { "linkpath", linkpath_coder, linkpath_decoder, false },
  { "mtime",	mtime_coder,	mtime_decoder,	  false },
  { "path",	path_coder,	path_decoder,	  false },
  { "size",	size_coder,	size_decoder,	  false },
  { "uid",	uid_coder,	uid_decoder,	  false },
  { "uname",	uname_coder,	uname_decoder,	  false },

  /* Sparse file handling */
  { "GNU.sparse.name",       path_coder, path_decoder,
    true },
  { "GNU.sparse.major",      sparse_major_coder, sparse_major_decoder,
    true },
  { "GNU.sparse.minor",      sparse_minor_coder, sparse_minor_decoder,
    true },
  { "GNU.sparse.realsize",   sparse_size_coder, sparse_size_decoder,
    true },
  { "GNU.sparse.numblocks",  sparse_numblocks_coder, sparse_numblocks_decoder,
    true },

  /* tar 1.14 - 1.15.90 keywords. */
  { "GNU.sparse.size",       sparse_size_coder, sparse_size_decoder, true },
  /* tar 1.14 - 1.15.1 keywords. Multiple instances of these appeared in 'x'
     headers, and each of them was meaningful. It confilcted with POSIX specs,
     which requires that "when extended header records conflict, the last one
     given in the header shall take precedence." */
  { "GNU.sparse.offset",     sparse_offset_coder, sparse_offset_decoder,
    true },
  { "GNU.sparse.numbytes",   sparse_numbytes_coder, sparse_numbytes_decoder,
    true },
  /* tar 1.15.90 keyword, introduced to remove the above-mentioned conflict. */
  { "GNU.sparse.map",        NULL /* Unused, see pax_dump_header() */,
    sparse_map_decoder, false },

  { "GNU.dumpdir",           dumpdir_coder, dumpdir_decoder,
    true },

  /* Keeps the tape/volume label. May be present only in the global headers.
     Equivalent to GNUTYPE_VOLHDR.  */
  { "GNU.volume.label", volume_label_coder, volume_label_decoder, true },

  /* These may be present in a first global header of the archive.
     They provide the same functionality as GNUTYPE_MULTIVOL header.
     The GNU.volume.size keeps the real_s_sizeleft value, which is
     otherwise kept in the size field of a multivolume header.  The
     GNU.volume.offset keeps the offset of the start of this volume,
     otherwise kept in oldgnu_header.offset.  */
  { "GNU.volume.filename", volume_label_coder, volume_filename_decoder,
    true },
  { "GNU.volume.size", volume_size_coder, volume_size_decoder, true },
  { "GNU.volume.offset", volume_offset_coder, volume_offset_decoder, true },

  { NULL, NULL, NULL, false }
};

#if 0 // **************************************************
static struct xhdr_tab const *
locate_handler (char const *keyword)
{
  struct xhdr_tab const *p;

  for (p = xhdr_tab; p->keyword; p++)
    if (strcmp (p->keyword, keyword) == 0)
      return p;
  return NULL;
}

static void
run_override_list (struct keyword_list *kp, struct tar_stat_info *st)
{
  for (; kp; kp = kp->next)
    {
      struct xhdr_tab const *t = locate_handler (kp->pattern);
      if (t)
	t->decoder (st, t->keyword, kp->value, strlen (kp->value));
    }
}

void
xheader_decode (struct tar_stat_info *st)
{
  run_override_list (keyword_global_override_list, st);
  run_override_list (global_header_override_list, st);

  if (st->xhdr.size)
    {
      char *p = st->xhdr.buffer + BLOCKSIZE;
      while (decode_record (&st->xhdr, &p, decx, st))
	continue;
    }
  run_override_list (keyword_override_list, st);
}
#endif // 0 **************************************************

void __obstack_free (struct obstack *obstack, void *block)
{
   sprtf( "Called __obstack_free with %X, and %X ...\n", obstack, block );
   // obstack_free(obstack,block);
}

/* When we copy a long block of data, this is the unit to do it with.
   On some machines, copying successive ints does not work;
   in such a case, redefine COPYING_UNIT to `long' (if that works)
   or `char' as a last resort.  */
# ifndef COPYING_UNIT
#  define COPYING_UNIT int
# endif

# define CALL_FREEFUN(h, old_chunk) \
  do { \
    if ((h) -> use_extra_arg) \
      (*(h)->freefun) ((h)->extra_arg, (old_chunk)); \
    else \
      (*(void (*) (void *)) (h)->freefun) ((old_chunk)); \
  } while (0)

/* Allocate a new current chunk for the obstack *H
   on the assumption that LENGTH bytes need to be added
   to the current object, or a new object of length LENGTH allocated.
   Copies any partial object from the end of the old chunk
   to the beginning of the new one.  */

void
_obstack_newchunk (struct obstack *h, int length)
{
  register struct _obstack_chunk *old_chunk = h->chunk;
  register struct _obstack_chunk *new_chunk;
  register long	new_size;
  register long obj_size = h->next_free - h->object_base;
  register long i;
  long already;
  char *object_base;

  /* Compute size for new chunk.  */
  new_size = (obj_size + length) + (obj_size >> 3) + h->alignment_mask + 100;
  if (new_size < h->chunk_size)
    new_size = h->chunk_size;

  /* Allocate and initialize the new chunk.  */
  new_chunk = CALL_CHUNKFUN (h, new_size);
  if (!new_chunk)
    (*obstack_alloc_failed_handler) ();
  h->chunk = new_chunk;
  new_chunk->prev = old_chunk;
  new_chunk->limit = h->chunk_limit = (char *) new_chunk + new_size;

  /* Compute an aligned object_base in the new chunk */
  object_base =
    __PTR_ALIGN ((char *) new_chunk, new_chunk->contents, h->alignment_mask);

  /* Move the existing object to the new chunk.
     Word at a time is fast and is safe if the object
     is sufficiently aligned.  */
  if (h->alignment_mask + 1 >= DEFAULT_ALIGNMENT)
    {
      for (i = obj_size / sizeof (COPYING_UNIT) - 1;
	   i >= 0; i--)
	((COPYING_UNIT *)object_base)[i]
	  = ((COPYING_UNIT *)h->object_base)[i];
      /* We used to copy the odd few remaining bytes as one extra COPYING_UNIT,
	 but that can cross a page boundary on a machine
	 which does not do strict alignment for COPYING_UNITS.  */
      already = obj_size / sizeof (COPYING_UNIT) * sizeof (COPYING_UNIT);
    }
  else
    already = 0;
  /* Copy remaining bytes one by one.  */
  for (i = already; i < obj_size; i++)
    object_base[i] = h->object_base[i];

  /* If the object just copied was the only data in OLD_CHUNK,
     free that chunk and remove it from the chain.
     But not if that chunk might contain an empty object.  */
  if (! h->maybe_empty_object
      && (h->object_base
	  == __PTR_ALIGN ((char *) old_chunk, old_chunk->contents,
			  h->alignment_mask)))
    {
      new_chunk->prev = old_chunk->prev;
      CALL_FREEFUN (h, old_chunk);
    }

  h->object_base = object_base;
  h->next_free = h->object_base + obj_size;
  /* The new chunk certainly contains no empty object yet.  */
  h->maybe_empty_object = 0;
}


// EOF - DumpTarX.c

