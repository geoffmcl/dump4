/*
 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * file.h - definitions for file(1) program
 * @(#)$File: file.h,v 1.89 2007/03/01 22:14:54 christos Exp $
 */

#ifndef __file_h__
#define __file_h__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>      /* Include that here, to make sure __P gets defined */
#include <errno.h>
#include <fcntl.h>      /* For open and flags */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
#include <sys/types.h>
/* Do this here and now, because struct stat gets re-defined on solaris */
#include <sys/stat.h>

#define ENABLE_CONDITIONALS

#ifndef MAGIC
#define MAGIC "/etc/magic"
#endif

#ifdef __EMX__
#define PATHSEP ';'
#else
#define PATHSEP ':'
#endif

#define private static
#ifndef protected
#define protected
#endif
#define public

#ifndef __GNUC_PREREQ__
#ifdef __GNUC__
#define __GNUC_PREREQ__(x, y)                                           \
        ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) ||                  \
         (__GNUC__ > (x)))
#else
#define __GNUC_PREREQ__(x, y)   0
#endif
#endif

#ifndef f__unused
#if __GNUC_PREREQ__(2, 7)
#define f__unused       __attribute__((__unused__))
#else
#define f__unused       /* delete */
#endif
#endif

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef HOWMANY
# define HOWMANY (256 * 1024)   /* how much of the file to look at */
#endif
#define MAXMAGIS 8192           /* max entries in /etc/magic */
#define MAXDESC 64              /* max leng of text description */
#define MAXstring 32            /* max leng of "string" types */

#define MAGICNO         0xF11E041C
#define VERSIONNO       4
#define FILE_MAGICSIZE  (32 * 4)

#define FILE_LOAD       0
#define FILE_CHECK      1
#define FILE_COMPILE    2

struct magic {
        /* Word 1 */
        uint16_t cont_level;    /* level of ">" */
        uint8_t nospflag;       /* supress space character */
        uint8_t flag;
#define INDIR           1       /* if '(...)' appears */
#define OFFADD          2       /* if '>&' or '>...(&' appears */
#define INDIROFFADD     4       /* if '>&(' appears */
#define UNSIGNED        8       /* comparison is unsigned */

        /* Word 2 */
        uint8_t reln;           /* relation (0=eq, '>'=gt, etc) */
        uint8_t vallen;         /* length of string value, if any */
        uint8_t type;           /* int, short, long or string. */
        uint8_t in_type;        /* type of indirrection */
#define                         FILE_INVALID    0
#define                         FILE_BYTE       1
#define                         FILE_SHORT      2
#define                         FILE_DEFAULT    3
#define                         FILE_LONG       4
#define                         FILE_STRING     5
#define                         FILE_DATE       6
#define                         FILE_BESHORT    7
#define                         FILE_BELONG     8
#define                         FILE_BEDATE     9
#define                         FILE_LESHORT    10
#define                         FILE_LELONG     11
#define                         FILE_LEDATE     12
#define                         FILE_PSTRING    13
#define                         FILE_LDATE      14
#define                         FILE_BELDATE    15
#define                         FILE_LELDATE    16
#define                         FILE_REGEX      17
#define                         FILE_BESTRING16 18
#define                         FILE_LESTRING16 19
#define                         FILE_SEARCH     20
#define                         FILE_MEDATE     21
#define                         FILE_MELDATE    22
#define                         FILE_MELONG     23
#define                         FILE_QUAD       24
#define                         FILE_LEQUAD     25
#define                         FILE_BEQUAD     26
#define                         FILE_QDATE      27
#define                         FILE_LEQDATE    28
#define                         FILE_BEQDATE    29
#define                         FILE_QLDATE     30
#define                         FILE_LEQLDATE   31
#define                         FILE_BEQLDATE   32
#define                         FILE_NAMES_SIZE 33/* size of array to contain all names */

#define IS_STRING(t) \
        ((t) == FILE_STRING || \
         (t) == FILE_PSTRING || \
         (t) == FILE_BESTRING16 || \
         (t) == FILE_LESTRING16 || \
         (t) == FILE_REGEX || \
         (t) == FILE_SEARCH || \
         (t) == FILE_DEFAULT)

#define FILE_FMT_NONE 0
#define FILE_FMT_NUM  1 /* "cduxXi" */
#define FILE_FMT_STR  2 /* "s" */
#define FILE_FMT_QUAD 3 /* "ll" */

        /* Word 3 */
        uint8_t in_op;          /* operator for indirection */
        uint8_t mask_op;        /* operator for mask */
#ifdef ENABLE_CONDITIONALS
        uint8_t cond;           /* conditional type */
        uint8_t dummy1; 
#else
        uint8_t dummy1; 
        uint8_t dummy2; 
#endif

#define                         FILE_OPS        "&|^+-*/%"
#define                         FILE_OPAND      0
#define                         FILE_OPOR       1
#define                         FILE_OPXOR      2
#define                         FILE_OPADD      3
#define                         FILE_OPMINUS    4
#define                         FILE_OPMULTIPLY 5
#define                         FILE_OPDIVIDE   6
#define                         FILE_OPMODULO   7
#define                         FILE_OPS_MASK   0x07 /* mask for above ops */
#define                         FILE_UNUSED_1   0x08
#define                         FILE_UNUSED_2   0x10
#define                         FILE_UNUSED_3   0x20
#define                         FILE_OPINVERSE  0x40
#define                         FILE_OPINDIRECT 0x80

#ifdef ENABLE_CONDITIONALS
#define                         COND_NONE       0
#define                         COND_IF         1
#define                         COND_ELIF       2
#define                         COND_ELSE       3
#endif /* ENABLE_CONDITIONALS */

        /* Word 4 */
        uint32_t offset;        /* offset to magic number */
        /* Word 5 */
        int32_t in_offset;      /* offset from indirection */
        /* Word 6 */
        uint32_t lineno;        /* line number in magic file */
        /* Word 7,8 */
        union {
                uint64_t _mask; /* for use with numeric and date types */
                struct {
                        uint32_t _count;        /* repeat/line count */
                        uint32_t _flags;        /* modifier flags */
                } _s;           /* for use with string types */
        } _u;
#define num_mask _u._mask
#define str_count _u._s._count
#define str_flags _u._s._flags

        /* Words 9-16 */
        union VALUETYPE {
                uint8_t b;
                uint16_t h;
                uint32_t l;
                uint64_t q;
                uint8_t hs[2];  /* 2 bytes of a fixed-endian "short" */
                uint8_t hl[4];  /* 4 bytes of a fixed-endian "long" */
                uint8_t hq[8];  /* 8 bytes of a fixed-endian "quad" */
                char s[MAXstring];      /* the search string or regex pattern */
        } value;                /* either number or string */
        /* Words 17..31 */
        char desc[MAXDESC];     /* description */
};

#define BIT(A)   (1 << (A))
#define STRING_COMPACT_BLANK            BIT(0)
#define STRING_COMPACT_OPTIONAL_BLANK   BIT(1)
#define STRING_IGNORE_LOWERCASE         BIT(2)
#define STRING_IGNORE_UPPERCASE         BIT(3)
#define REGEX_OFFSET_START              BIT(4)
#define CHAR_COMPACT_BLANK              'B'
#define CHAR_COMPACT_OPTIONAL_BLANK     'b'
#define CHAR_IGNORE_LOWERCASE           'c'
#define CHAR_IGNORE_UPPERCASE           'C'
#define CHAR_REGEX_OFFSET_START         's'
#define STRING_IGNORE_CASE              (STRING_IGNORE_LOWERCASE|STRING_IGNORE_UPPERCASE)


/* list of magic entries */
struct mlist {
        struct magic *magic;            /* array of magic entries */
        uint32_t nmagic;                        /* number of entries in array */
        int mapped;  /* allocation type: 0 => apprentice_file
                      *                  1 => apprentice_map + malloc
                      *                  2 => apprentice_map + mmap */
        struct mlist *next, *prev;
};

struct magic_set {
/*@null@*/
    struct mlist *mlist;
    struct cont {
        size_t len;
                struct level_info {
                        int32_t off;
                        int got_match;
#ifdef ENABLE_CONDITIONALS
                        int last_match;
                        int last_cond;  /* used for error checking by parse() */
#endif
                } *li;
    } c;
    struct out {
        /* Accumulation buffer */
        char *buf;
        char *ptr;
                size_t left;
        size_t size;
        /* Printable buffer */
        char *pbuf;
        size_t psize;
    } o;
        uint32_t offset;
    int error;
    int flags;
    int haderr;
    const char *file;
        size_t line;                    /* current magic line number */

        /* data for searches */
        struct {
                const char *s;          /* start of search in original source */
                size_t s_len;           /* length of search region */
                size_t offset;          /* starting offset in source: XXX - should this be off_t? */
                size_t rm_len;          /* match length */
        } search;

       // union VALUETYPE ms_value;       /* either number or string */
};

struct stat;
/*@observer@*/
protected const char *file_fmttime(uint32_t, int)
        /*@*/;
protected int file_buffer(struct magic_set *ms, int fd, /*@null@*/ const char *inname, const void *buf,
    size_t nb)
        /*@globals fileSystem, internalState @*/
        /*@modifies ms, fileSystem, internalState @*/;
protected int file_fsmagic(struct magic_set *ms, const char *, struct stat *sb)
        /*@modifies ms, sb @*/;
protected int file_pipe2file(struct magic_set *ms, int, const void *, size_t)
        /*@globals errno, fileSystem, internalState @*/
        /*@modifies ms, errno, fileSystem, internalState @*/;
protected int file_printf(struct magic_set *ms, const char *, ...)
        /*@modifies ms @*/;
protected int file_reset(struct magic_set *ms)
        /*@modifies ms @*/;
protected int file_tryelf(struct magic_set *ms, int, const unsigned char *,
    size_t)
        /*@globals fileSystem, internalState @*/
        /*@modifies ms, fileSystem, internalState @*/;
protected int file_zmagic(struct magic_set *ms, int fd, /*@null@*/ const char *name,
    const unsigned char *buf, size_t nbytes)
        /*@globals fileSystem, internalState @*/
        /*@modifies ms, fileSystem, internalState @*/;
protected int file_ascmagic(struct magic_set *ms, const unsigned char *, size_t)
        /*@modifies ms @*/;
protected int file_is_tar(struct magic_set *ms, const unsigned char *, size_t)
        /*@modifies ms @*/;
protected int file_softmagic(struct magic_set *ms, const unsigned char *, size_t)
        /*@globals fileSystem @*/
        /*@modifies ms, fileSystem @*/;
/*@null@*/
protected struct mlist *file_apprentice(struct magic_set *ms, const char *, int)
        /*@globals fileSystem, internalState @*/
        /*@modifies ms, fileSystem, internalState @*/;
protected uint64_t file_signextend(struct magic_set *ms, struct magic *,
    uint64_t)
        /*@globals fileSystem @*/
        /*@modifies ms, fileSystem @*/;
protected void file_delmagic(/*@only@*/ struct magic *p, int type, size_t entries)
        /*@globals fileSystem @*/
        /*@modifies p, fileSystem @*/;
protected void file_badread(struct magic_set *ms)
        /*@modifies ms @*/;
protected void file_badseek(struct magic_set *ms)
        /*@modifies ms @*/;
protected void file_oomem(struct magic_set *ms, size_t)
        /*@modifies ms @*/;
protected void file_error(struct magic_set *ms, int error, const char *f, ...)
        /*@modifies ms @*/;
protected void file_magerror(struct magic_set *ms, const char *, ...)
        /*@modifies ms @*/;
protected void file_magwarn(struct magic_set *ms, const char *, ...)
        /*@globals stdout, stderr, fileSystem @*/
        /*@modifies ms, stdout, stderr, fileSystem @*/;
protected void file_mdump(struct magic *)
        /*@globals fileSystem @*/
        /*@modifies fileSystem @*/;
protected void file_showstr(FILE *fp, const char *, size_t)
        /*@globals fileSystem @*/
        /*@modifies fp, fileSystem @*/;
protected size_t file_mbswidth(const char *)
        /*@*/;
/*@observer@*/
protected const char *file_getbuffer(struct magic_set *ms)
        /*@modifies ms @*/;
protected ssize_t sread(int fd, void *buf, size_t n, int canbepipe)
        /*@modifies buf @*/;
#ifdef ENABLE_CONDITIONALS
protected int file_check_mem(struct magic_set *ms, unsigned int)
        /*@modifies ms @*/;
#endif

#ifndef COMPILE_ONLY
extern const char *file_names[];
extern const size_t file_nnames;
#endif

#ifdef WIN32
#if !defined(HAVE_STRERROR)
extern int sys_nerr;
extern char *sys_errlist[];
#define strerror(e) \
        (((e) >= 0 && (e) < sys_nerr) ? sys_errlist[(e)] : "Unknown error")
#endif
#endif

#ifndef HAVE_STRTOUL
#define strtoul(a, b, c)        strtol(a, b, c)
#endif

#ifndef HAVE_SNPRINTF
int snprintf(char *, size_t, const char *, ...)
        /*@*/;
#endif

#if defined(HAVE_MMAP) && defined(HAVE_SYS_MMAN_H) && !defined(QUICK)
#define QUICK
#endif

#ifndef O_BINARY
#define O_BINARY        0
#endif

#define FILE_RCSID(id) \
static const char *rcsid(const char *p) /*@*/ { \
        return rcsid(p = id); \
}

#endif /* __file_h__ */

