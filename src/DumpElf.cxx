/*
 * Copyright (c) Christos Zoulas 2003.
 * All Rights Reserved.
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
    References found in the beye source
    http://www.caldera.com/developers/gabi/ = 404 NOT FOUND
    http://segfault.net/~scut/cpu/generic/  = 404 NOT FOUND
    http://www.linuxassembly.org            = FOUND NOTHING in a brief search
    ELFkickers This distribution is a collection of programs that are generally
	       unrelated, except in that they all deal with the ELF file format.
    teensy     tools that are provided as exhibitions of just how compressed a
	       Linux ELF executable can be and still function.
    These can be found at:
    http://www.muppetlabs.com/~breadbox/software/ = These programs are on GITHUB
    https://github.com/BR903/ELFkickers - elfls lloks promising
     elfls is a utility that displays an ELF file's program and/or
     section header tables, which serve as a kind of global roadmap to
     the file's contents.
     DOWNLOAD: https://github.com/BR903/ELFkickers.git - updelfkickers.bat C:\FG\18

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
//#include "file.h"
#include <sstream>
#include <string>
#include <iomanip>
//#include <cmath>
#include "Dump4.h"

#ifdef WIN32
/////////////////////////////////////////////////////////////////////


#ifdef _MSC_VER
/////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4018 ) // signed/unsigned mismatch
#include <io.h>
#ifndef HAVE_STRTOUL
#define HAVE_STRTOUL 1
#endif

#ifndef HAVE_STRERROR
#define HAVE_STRERROR 1
#endif

#ifndef ELFCORE
#define ELFCORE 1
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else // !_HAVE_STDINT_H
typedef __int32    int32_t;
typedef __int16    int16_t;
typedef __int64    int64_t;
typedef __int8     int8_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int64    uint64_t;
typedef unsigned __int8     uint8_t;
#endif // HAVE_STDINT_H

#define SIZEOF_UINT64_T 8 //sizeof(__int64)

#define ssize_t   size_t
#define inline __inline

/////////////////////////////////////////////////////////////////////
#endif // _MSC_VER

#include "elffile.h"
#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef BUILTIN_ELF
//#include <string.h>
//#include <ctype.h>
//#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

//#include "readelf.h"
#include "DumpElf.h"

#ifndef lint
FILE_RCSID("@(#)$File: readelf.c,v 1.63 2007/01/16 14:56:45 ljt Exp $")
#endif

#ifdef _MSC_VER
/////////////////////////////////////////////////////////////////////
// TODO: need to do something with these STUBS
protected void file_badread(struct magic_set *ms) { }
protected void file_badseek(struct magic_set *ms) { }
protected int file_pipe2file(struct magic_set *ms, int i, const void *v, size_t s) { return -1; }
protected int file_printf(struct magic_set *ms, const char * cp, ...) { return 0; }
protected void file_error(struct magic_set *ms, int error, const char *f, ...) { }
/////////////////////////////////////////////////////////////////////
#endif // _MSC_VER


#ifdef  ELFCORE
private int dophn_core(struct magic_set *ms, int iclass, int swap, int fd,
    off_t off, int num, size_t size, off_t fsize, int *flags)
        /*@modifies ms, *flags @*/;
#endif

private int dophn_exec(struct magic_set *ms, int iclass, int swap, int fd,
    off_t off, int num, size_t size, off_t fsize, int *flags, int sh_num)
        /*@modifies ms, *flags @*/;
private int doshn(struct magic_set *ms, int iclass, int swap, int fd, off_t off,
    int num, size_t size, int *flags)
        /*@modifies ms, *flags @*/;
private size_t donote(struct magic_set *ms, unsigned char *nbuf, size_t offset,
    size_t size, int iclass, int swap, size_t align, int *flags)
        /*@modifies ms, *flags @*/;

#define ELF_ALIGN(a)    ((((a) + align - 1) / align) * align)

#define isquote(c) (strchr("'\"`", (c)) != NULL)

private uint16_t getu16(int, uint16_t)
        /*@*/;
private uint32_t getu32(int, uint32_t)
        /*@*/;
private uint64_t getu64(int, uint64_t)
        /*@*/;

private uint16_t
getu16(int swap, uint16_t value)
{
        union {
                uint16_t ui;
                char c[2];
        } retval, tmpval;

        if (swap) {
                tmpval.ui = value;

                retval.c[0] = tmpval.c[1];
                retval.c[1] = tmpval.c[0];
                
                return retval.ui;
        } else
                return value;
}

private uint32_t
getu32(int swap, uint32_t value)
{
        union {
                uint32_t ui;
                char c[4];
        } retval, tmpval;

        if (swap) {
                tmpval.ui = value;

                retval.c[0] = tmpval.c[3];
                retval.c[1] = tmpval.c[2];
                retval.c[2] = tmpval.c[1];
                retval.c[3] = tmpval.c[0];
                
                return retval.ui;
        } else
                return value;
}

private uint64_t
getu64(int swap, uint64_t value)
{
        union {
                uint64_t ui;
                char c[8];
        } retval;
        union {
                uint64_t ui;
                char c[8];
        } tmpval;

        if (swap) {
                tmpval.ui = value;

                retval.c[0] = tmpval.c[7];
                retval.c[1] = tmpval.c[6];
                retval.c[2] = tmpval.c[5];
                retval.c[3] = tmpval.c[4];
                retval.c[4] = tmpval.c[3];
                retval.c[5] = tmpval.c[2];
                retval.c[6] = tmpval.c[1];
                retval.c[7] = tmpval.c[0];
                
                return retval.ui;
        } else
                return value;
}

#ifdef USE_ARRAY_FOR_64BIT_TYPES
# define elf_getu64(swap, array) \
        ((swap ? ((uint64_t)getu32(swap, array[0])) << 32 : getu32(swap, array[0])) + \
         (swap ? getu32(swap, array[1]) : ((uint64_t)getu32(swap, array[1]) << 32)))
#else
# define elf_getu64(swap, value) getu64(swap, value)
#endif

#define xsh_addr        (iclass == ELFCLASS32            \
                         ? (void *) &sh32               \
                         : (void *) &sh64)
#define xsh_sizeof      (iclass == ELFCLASS32            \
                         ? sizeof sh32                  \
                         : sizeof sh64)
#define xsh_size        (iclass == ELFCLASS32            \
                         ? getu32(swap, sh32.sh_size)   \
                         : getu64(swap, sh64.sh_size))
#define xsh_offset      (iclass == ELFCLASS32            \
                         ? getu32(swap, sh32.sh_offset) \
                         : getu64(swap, sh64.sh_offset))
#define xsh_type        (iclass == ELFCLASS32            \
                         ? getu32(swap, sh32.sh_type)   \
                         : getu32(swap, sh64.sh_type))
#define xph_addr        (iclass == ELFCLASS32            \
                         ? (void *) &ph32               \
                         : (void *) &ph64)
#define xph_sizeof      (iclass == ELFCLASS32            \
                         ? sizeof ph32                  \
                         : sizeof ph64)
#define xph_type        (iclass == ELFCLASS32            \
                         ? getu32(swap, ph32.p_type)    \
                         : getu32(swap, ph64.p_type))
#define xph_offset      (off_t)(iclass == ELFCLASS32     \
                         ? getu32(swap, ph32.p_offset)  \
                         : getu64(swap, ph64.p_offset))
#define xph_align       (size_t)((iclass == ELFCLASS32   \
                         ? (off_t) (ph32.p_align ?      \
                            getu32(swap, ph32.p_align) : 4) \
                         : (off_t) (ph64.p_align ?      \
                            getu64(swap, ph64.p_align) : 4)))
#define xph_filesz      (size_t)((iclass == ELFCLASS32   \
                         ? getu32(swap, ph32.p_filesz)  \
                         : getu64(swap, ph64.p_filesz)))
#define xnh_addr        (iclass == ELFCLASS32            \
                         ? (void *) &nh32               \
                         : (void *) &nh64)
#define xph_memsz       (size_t)((iclass == ELFCLASS32   \
                         ? getu32(swap, ph32.p_memsz)   \
                         : getu64(swap, ph64.p_memsz)))
#define xnh_sizeof      (iclass == ELFCLASS32            \
                         ? sizeof nh32                  \
                         : sizeof nh64)
#define xnh_type        (iclass == ELFCLASS32            \
                         ? getu32(swap, nh32.n_type)    \
                         : getu32(swap, nh64.n_type))
#define xnh_namesz      (iclass == ELFCLASS32            \
                         ? getu32(swap, nh32.n_namesz)  \
                         : getu32(swap, nh64.n_namesz))
#define xnh_descsz      (iclass == ELFCLASS32            \
                         ? getu32(swap, nh32.n_descsz)  \
                         : getu32(swap, nh64.n_descsz))
#define prpsoffsets(i)  (iclass == ELFCLASS32            \
                         ? prpsoffsets32[i]             \
                         : prpsoffsets64[i])

#ifdef ELFCORE
/*@unchecked@*/ /*@observer@*/
size_t  prpsoffsets32[] = {
        8,              /* FreeBSD */
        28,             /* Linux 2.0.36 (short name) */
        44,             /* Linux (path name) */
        84,             /* SunOS 5.x */
};

/*@unchecked@*/ /*@observer@*/
size_t  prpsoffsets64[] = {
        16,             /* FreeBSD, 64-bit */
        40,             /* Linux (tested on core from 2.4.x, short name) */
        56,             /* Linux (path name) */
       120,             /* SunOS 5.x, 64-bit */
};

#define NOFFSETS32      (sizeof prpsoffsets32 / sizeof prpsoffsets32[0])
#define NOFFSETS64      (sizeof prpsoffsets64 / sizeof prpsoffsets64[0])

#define NOFFSETS        (iclass == ELFCLASS32 ? NOFFSETS32 : NOFFSETS64)

/*
 * Look through the program headers of an executable image, searching
 * for a PT_NOTE section of type NT_PRPSINFO, with a name "CORE" or
 * "FreeBSD"; if one is found, try looking in various places in its
 * contents for a 16-character string containing only printable
 * characters - if found, that string should be the name of the program
 * that dropped core.  Note: right after that 16-character string is,
 * at least in SunOS 5.x (and possibly other SVR4-flavored systems) and
 * Linux, a longer string (80 characters, in 5.x, probably other
 * SVR4-flavored systems, and Linux) containing the start of the
 * command line for that program.
 *
 * The signal number probably appears in a section of type NT_PRSTATUS,
 * but that's also rather OS-dependent, in ways that are harder to
 * dissect with heuristics, so I'm not bothering with the signal number.
 * (I suppose the signal number could be of interest in situations where
 * you don't have the binary of the program that dropped core; if you
 * *do* have that binary, the debugger will probably tell you what
 * signal it was.)
 */

/*@unchecked@*/ /*@observer@*/
private const char *os_style_names[] = {
        "SVR4",
        "FreeBSD",
        "NetBSD",
};

private int
dophn_core(struct magic_set *ms, int iclass, int swap, int fd, off_t off,
    int num, size_t size, off_t fsize, int *flags)
{
        Elf32_Phdr ph32;
        Elf64_Phdr ph64;
        size_t offset;
        unsigned char nbuf[BUFSIZ];
        ssize_t bufsize;
        off_t savedoffset;
        struct stat st;

        if (fstat(fd, &st) < 0) {
                file_badread(ms);
                return -1;
        }

        if (size != xph_sizeof) {
                if (file_printf(ms, ", corrupted program header size") == -1)
                        return -1;
                return 0;
        }

        /*
         * Loop through all the program headers.
         */
        for ( ; num; num--) {
                if ((savedoffset = lseek(fd, off, SEEK_SET)) == (off_t)-1) {
                        file_badseek(ms);
                        return -1;
                }
                if (read(fd, xph_addr, xph_sizeof) == -1) {
                        file_badread(ms);
                        return -1;
                }
                if (xph_offset > fsize) {
                        if (lseek(fd, savedoffset, SEEK_SET) == (off_t)-1) {
                                file_badseek(ms);
                                return -1;
                        }
                        continue;
                }

                off += size;
                if (xph_type != PT_NOTE)
                        continue;

                /*
                 * This is a PT_NOTE section; loop through all the notes
                 * in the section.
                 */
                if (lseek(fd, xph_offset, SEEK_SET) == (off_t)-1) {
                        file_badseek(ms);
                        return -1;
                }
                bufsize = read(fd, nbuf,
                    ((xph_filesz < sizeof(nbuf)) ? xph_filesz : sizeof(nbuf)));
                if (bufsize == -1) {
                        file_badread(ms);
                        return -1;
                }
                offset = 0;
                for (;;) {
                        if (offset >= (size_t)bufsize)
                                /*@innerbreak@*/ break;
                        offset = donote(ms, nbuf, offset, (size_t)bufsize,
                            iclass, swap, 4, flags);
                        if (offset == 0)
                                /*@innerbreak@*/ break;

                }
        }
        return 0;
}
#endif

#define OS_STYLE_SVR4           0
#define OS_STYLE_FREEBSD        1
#define OS_STYLE_NETBSD         2

#define FLAGS_DID_CORE          1
#define FLAGS_DID_NOTE          2

private size_t
donote(struct magic_set *ms, unsigned char *nbuf, size_t offset, size_t size,
    int iclass, int swap, size_t align, int *flags)
{
        Elf32_Nhdr nh32;
        Elf64_Nhdr nh64;
        size_t noff, doff;
//#ifdef ELFCORE
        int os_style = -1;
//#endif
        uint32_t namesz, descsz;

        (void)memcpy(xnh_addr, &nbuf[offset], xnh_sizeof);
        offset += xnh_sizeof;

        namesz = xnh_namesz;
        descsz = xnh_descsz;
        if ((namesz == 0) && (descsz == 0)) {
                /*
                 * We're out of note headers.
                 */
                return (offset >= size) ? offset : size;
        }

        if (namesz & 0x80000000) {
            (void)file_printf(ms, ", bad note name size 0x%lx",
                (unsigned long)namesz);
            return offset;
        }

        if (descsz & 0x80000000) {
            (void)file_printf(ms, ", bad note description size 0x%lx",
                (unsigned long)descsz);
            return offset;
        }


        noff = offset;
        doff = ELF_ALIGN(offset + namesz);

        if (offset + namesz > size) {
                /*
                 * We're past the end of the buffer.
                 */
                return doff;
        }

        offset = ELF_ALIGN(doff + descsz);
        if (doff + descsz > size) {
                /*
                 * We're past the end of the buffer.
                 */
                return (offset >= size) ? offset : size;
        }

        if (*flags & FLAGS_DID_NOTE)
                goto core;

        if (namesz == 4 && strcmp((char *)&nbuf[noff], "GNU") == 0 &&
            xnh_type == NT_GNU_VERSION && descsz == 16) {
                uint32_t desc[4];
                (void)memcpy(desc, &nbuf[doff], sizeof(desc));

                if (file_printf(ms, ", for GNU/") == -1)
                        return size;
                switch (getu32(swap, desc[0])) {
                case GNU_OS_LINUX:
                        if (file_printf(ms, "Linux") == -1)
                                return size;
                        break;
                case GNU_OS_HURD:
                        if (file_printf(ms, "Hurd") == -1)
                                return size;
                        break;
                case GNU_OS_SOLARIS:
                        if (file_printf(ms, "Solaris") == -1)
                                return size;
                        break;
                default:
                        if (file_printf(ms, "<unknown>") == -1)
                                return size; 
                }
                if (file_printf(ms, " %d.%d.%d", getu32(swap, desc[1]),
                    getu32(swap, desc[2]), getu32(swap, desc[3])) == -1)
                        return size;
                *flags |= FLAGS_DID_NOTE;
                return size;
        }

        if (namesz == 7 && strcmp((char *)&nbuf[noff], "NetBSD") == 0 &&
            xnh_type == NT_NETBSD_VERSION && descsz == 4) {
                uint32_t desc;
                (void)memcpy(&desc, &nbuf[doff], sizeof(desc));
                desc = getu32(swap, desc);

                if (file_printf(ms, ", for NetBSD") == -1)
                        return size;
                /*
                 * The version number used to be stuck as 199905, and was thus
                 * basically content-free.  Newer versions of NetBSD have fixed
                 * this and now use the encoding of __NetBSD_Version__:
                 *
                 *      MMmmrrpp00
                 *
                 * M = major version
                 * m = minor version
                 * r = release ["",A-Z,Z[A-Z] but numeric]
                 * p = patchlevel
                 */
                if (desc > 100000000U) {
                        uint32_t ver_patch = (desc / 100) % 100;
                        uint32_t ver_rel = (desc / 10000) % 100;
                        uint32_t ver_min = (desc / 1000000) % 100;
                        uint32_t ver_maj = desc / 100000000;

                        if (file_printf(ms, " %u.%u", ver_maj, ver_min) == -1)
                                return size;
                        if (ver_rel == 0 && ver_patch != 0) {
                                if (file_printf(ms, ".%u", ver_patch) == -1)
                                        return size;
                        } else if (ver_rel != 0) {
                                while (ver_rel > 26) {
                                        if (file_printf(ms, "Z") == -1)
                                                return size;
                                        ver_rel -= 26;
                                }
                                if (file_printf(ms, "%c", 'A' + ver_rel - 1)
                                    == -1)
                                        return size;
                        }
                }
                *flags |= FLAGS_DID_NOTE;
                return size;
        }

        if (namesz == 8 && strcmp((char *)&nbuf[noff], "FreeBSD") == 0 &&
            xnh_type == NT_FREEBSD_VERSION && descsz == 4) {
                uint32_t desc;
                (void)memcpy(&desc, &nbuf[doff], sizeof(desc));
                desc = getu32(swap, desc);
                if (file_printf(ms, ", for FreeBSD") == -1)
                        return size;

                /*
                 * Contents is __FreeBSD_version, whose relation to OS
                 * versions is defined by a huge table in the Porter's
                 * Handbook.  This is the general scheme:
                 * 
                 * Releases:
                 *      Mmp000 (before 4.10)
                 *      Mmi0p0 (before 5.0)
                 *      Mmm0p0
                 * 
                 * Development branches:
                 *      Mmpxxx (before 4.6)
                 *      Mmp1xx (before 4.10)
                 *      Mmi1xx (before 5.0)
                 *      M000xx (pre-M.0)
                 *      Mmm1xx
                 * 
                 * M = major version
                 * m = minor version
                 * i = minor version increment (491000 -> 4.10)
                 * p = patchlevel
                 * x = revision
                 * 
                 * The first release of FreeBSD to use ELF by default
                 * was version 3.0.
                 */
                if (desc == 460002) {
                        if (file_printf(ms, " 4.6.2") == -1)
                                return size;
                } else if (desc < 460100) {
                        if (file_printf(ms, " %d.%d", desc / 100000,
                            desc / 10000 % 10) == -1)
                                return size;
                        if (desc / 1000 % 10 > 0)
                                if (file_printf(ms, ".%d", desc / 1000 % 10)
                                    == -1)
                                        return size;
                        if ((desc % 1000 > 0) || (desc % 100000 == 0))
                                if (file_printf(ms, " (%d)", desc) == -1)
                                        return size;
                } else if (desc < 500000) {
                        if (file_printf(ms, " %d.%d", desc / 100000,
                            desc / 10000 % 10 + desc / 1000 % 10) == -1)
                                return size;
                        if (desc / 100 % 10 > 0) {
                                if (file_printf(ms, " (%d)", desc) == -1)
                                        return size;
                        } else if (desc / 10 % 10 > 0) {
                                if (file_printf(ms, ".%d", desc / 10 % 10)
                                    == -1)
                                        return size;
                        }
                } else {
                        if (file_printf(ms, " %d.%d", desc / 100000,
                            desc / 1000 % 100) == -1)
                                return size;
                        if ((desc / 100 % 10 > 0) ||
                            (desc % 100000 / 100 == 0)) {
                                if (file_printf(ms, " (%d)", desc) == -1)
                                        return size;
                        } else if (desc / 10 % 10 > 0) {
                                if (file_printf(ms, ".%d", desc / 10 % 10)
                                    == -1)
                                        return size;
                        }
                }
                *flags |= FLAGS_DID_NOTE;
                return size;
        }

        if (namesz == 8 && strcmp((char *)&nbuf[noff], "OpenBSD") == 0 &&
            xnh_type == NT_OPENBSD_VERSION && descsz == 4) {
                if (file_printf(ms, ", for OpenBSD") == -1)
                        return size;
                /* Content of note is always 0 */
                *flags |= FLAGS_DID_NOTE;
                return size;
        }

        if (namesz == 10 && strcmp((char *)&nbuf[noff], "DragonFly") == 0 &&
            xnh_type == NT_DRAGONFLY_VERSION && descsz == 4) {
                uint32_t desc;
                if (file_printf(ms, ", for DragonFly") == -1)
                        return size;
                (void)memcpy(&desc, &nbuf[doff], sizeof(desc));
                desc = getu32(swap, desc);
                if (file_printf(ms, " %d.%d.%d", desc / 100000,
                    desc / 10000 % 10, desc % 10000) == -1)
                        return size;
                *flags |= FLAGS_DID_NOTE;
                return size;
        }

core:
        /*
         * Sigh.  The 2.0.36 kernel in Debian 2.1, at
         * least, doesn't correctly implement name
         * sections, in core dumps, as specified by
         * the "Program Linking" section of "UNIX(R) System
         * V Release 4 Programmer's Guide: ANSI C and
         * Programming Support Tools", because my copy
         * clearly says "The first 'namesz' bytes in 'name'
         * contain a *null-terminated* [emphasis mine]
         * character representation of the entry's owner
         * or originator", but the 2.0.36 kernel code
         * doesn't include the terminating null in the
         * name....
         */
        if ((namesz == 4 && strncmp((char *)&nbuf[noff], "CORE", 4) == 0) ||
            (namesz == 5 && strcmp((char *)&nbuf[noff], "CORE") == 0)) {
                os_style = OS_STYLE_SVR4;
        } 

        if ((namesz == 8 && strcmp((char *)&nbuf[noff], "FreeBSD") == 0)) {
                os_style = OS_STYLE_FREEBSD;
        }

        if ((namesz >= 11 && strncmp((char *)&nbuf[noff], "NetBSD-CORE", 11)
            == 0)) {
                os_style = OS_STYLE_NETBSD;
        }

#ifdef ELFCORE
        if ((*flags & FLAGS_DID_CORE) != 0)
                return size;

        if (os_style != -1) {
                if (file_printf(ms, ", %s-style", os_style_names[os_style])
                    == -1)
                                return size;
        }

        switch (os_style) {
        case OS_STYLE_NETBSD:
                if (xnh_type == NT_NETBSD_CORE_PROCINFO) {
                        uint32_t signo;
                        /*
                         * Extract the program name.  It is at
                         * offset 0x7c, and is up to 32-bytes,
                         * including the terminating NUL.
                         */
                        if (file_printf(ms, ", from '%.31s'",
                            &nbuf[doff + 0x7c]) == -1)
                                return size;
                        
                        /*
                         * Extract the signal number.  It is at
                         * offset 0x08.
                         */
                        (void)memcpy(&signo, &nbuf[doff + 0x08],
                            sizeof(signo));
                        if (file_printf(ms, " (signal %u)",
                            getu32(swap, signo)) == -1)
                                return size;
                        return size;
                }
                break;

        default:
                if (xnh_type == NT_PRPSINFO) {
                        size_t i, j;
                        unsigned char c;
                        /*
                         * Extract the program name.  We assume
                         * it to be 16 characters (that's what it
                         * is in SunOS 5.x and Linux).
                         *
                         * Unfortunately, it's at a different offset
                         * in varous OSes, so try multiple offsets.
                         * If the characters aren't all printable,
                         * reject it.
                         */
                        for (i = 0; i < NOFFSETS; i++) {
                                size_t reloffset = prpsoffsets(i);
                                size_t noffset = doff + reloffset;
                                for (j = 0; j < 16; j++, noffset++,
                                    reloffset++) {
                                        /*
                                         * Make sure we're not past
                                         * the end of the buffer; if
                                         * we are, just give up.
                                         */
                                        if (noffset >= size)
                                                goto tryanother;

                                        /*
                                         * Make sure we're not past
                                         * the end of the contents;
                                         * if we are, this obviously
                                         * isn't the right offset.
                                         */
                                        if (reloffset >= descsz)
                                                goto tryanother;

                                        c = nbuf[noffset];
                                        if (c == '\0') {
                                                /*
                                                 * A '\0' at the
                                                 * beginning is
                                                 * obviously wrong.
                                                 * Any other '\0'
                                                 * means we're done.
                                                 */
                                                if (j == 0)
                                                        goto tryanother;
                                                else
                                                        /*@innerbreak@*/ break;
                                        } else {
                                                /*
                                                 * A nonprintable
                                                 * character is also
                                                 * wrong.
                                                 */
                                                if (!isprint(c) || isquote(c))
                                                        goto tryanother;
                                        }
                                }
                                /*
                                 * Well, that worked.
                                 */
                                if (file_printf(ms, ", from '%.16s'",
                                    &nbuf[doff + prpsoffsets(i)]) == -1)
                                        return size;
                                return size;

                        tryanother:
                                ;
                        }
                }
                break;
        }
#endif
        *flags |= FLAGS_DID_CORE;
        return offset;
}

private int
doshn(struct magic_set *ms, int iclass, int swap, int fd, off_t off, int num,
    size_t size, int *flags)
{
        Elf32_Shdr sh32;
        Elf64_Shdr sh64;
        int stripped = 1;
        void *nbuf;
        off_t noff;

        if (size != xsh_sizeof) {
                if (file_printf(ms, ", corrupted section header size") == -1)
                        return -1;
                return 0;
        }

        if (lseek(fd, off, SEEK_SET) == (off_t)-1) {
                file_badseek(ms);
                return -1;
        }

        for ( ; num; num--) {
                if (read(fd, xsh_addr, xsh_sizeof) == -1) {
                        file_badread(ms);
                        return -1;
                }
                switch (xsh_type) {
                case SHT_SYMTAB:
#if 0
                case SHT_DYNSYM:
#endif
                        stripped = 0;
                        /*@switchbreak@*/ break;
                case SHT_NOTE:
                        if ((off = lseek(fd, (off_t)0, SEEK_CUR)) ==
                            (off_t)-1) {
                                file_badread(ms);
                                return -1;
                        }
                        if ((nbuf = malloc((size_t)xsh_size)) == NULL) {
                                file_error(ms, errno, "Cannot allocate memory"
                                    " for note");
                                return -1;
                        }
                        if ((noff = lseek(fd, (off_t)xsh_offset, SEEK_SET)) ==
                            (off_t)-1) {
                                file_badread(ms);
                                free(nbuf);
                                return -1;
                        }
                        if (read(fd, nbuf, (size_t)xsh_size) !=
                            (ssize_t)xsh_size) {
                                free(nbuf);
                                file_badread(ms);
                                return -1;
                        }

                        noff = 0;
                        for (;;) {
                                if (noff >= (size_t)xsh_size)
                                        /*@innerbreak@*/ break;
                                noff = donote(ms, (unsigned char *)nbuf, (size_t)noff,
                                    (size_t)xsh_size, iclass, swap, 4,
                                    flags);
                                if (noff == 0)
                                        /*@innerbreak@*/ break;
                        }
                        if ((lseek(fd, off, SEEK_SET)) == (off_t)-1) {
                                free(nbuf);
                                file_badread(ms);
                                return -1;
                        }
                        free(nbuf);
                        /*@switchbreak@*/ break;
                }
        }
        if (file_printf(ms, ", %sstripped", stripped ? "" : "not ") == -1)
                return -1;
        return 0;
}

/*
 * Look through the program headers of an executable image, searching
 * for a PT_INTERP section; if one is found, it's dynamically linked,
 * otherwise it's statically linked.
 */
private int
dophn_exec(struct magic_set *ms, int iclass, int swap, int fd, off_t off,
    int num, size_t size, off_t fsize, int *flags, int sh_num)
{
        Elf32_Phdr ph32;
        Elf64_Phdr ph64;
        const char *linking_style = "statically";
        const char *shared_libraries = "";
        unsigned char nbuf[BUFSIZ];
        int bufsize;
        size_t offset, align;
        off_t savedoffset = (off_t)-1;
        struct stat st;

        if (fstat(fd, &st) < 0) {
                file_badread(ms);
                return -1;
        }

        if (size != xph_sizeof) {
                if (file_printf(ms, ", corrupted program header size") == -1)
                    return -1;
                return 0;
        }
        if (lseek(fd, off, SEEK_SET) == (off_t)-1) {
                file_badseek(ms);
                return -1;
        }

        for ( ; num; num--) {
                if (read(fd, xph_addr, xph_sizeof) == -1) {
                        file_badread(ms);
                        return -1;
                }
                if (xph_offset > st.st_size && savedoffset != (off_t)-1) {
                        if (lseek(fd, savedoffset, SEEK_SET) == (off_t)-1) {
                                file_badseek(ms);
                                return -1;
                        }
                        continue;
                }

                if ((savedoffset = lseek(fd, (off_t)0, SEEK_CUR)) == (off_t)-1) {
                        file_badseek(ms);
                        return -1;
                }

                if (xph_offset > fsize) {
                        if (lseek(fd, savedoffset, SEEK_SET) == (off_t)-1) {
                                file_badseek(ms);
                                return -1;
                        }
                        continue;
                }

                switch (xph_type) {
                case PT_DYNAMIC:
                        linking_style = "dynamically";
                        /*@switchbreak@*/ break;
                case PT_INTERP:
                        shared_libraries = " (uses shared libs)";
                        /*@switchbreak@*/ break;
                case PT_NOTE:
                        if ((align = xph_align) & 0x80000000) {
                                if (file_printf(ms, 
                                    ", invalid note alignment 0x%lx",
                                    (unsigned long)align) == -1)
                                        return -1;
                                align = 4;
                        }
                        /* If we have a section header table, handle note
                           sections just in doshn.  Handling them also here
                           means that for executables we print the note content
                           twice and, more importantly, don't handle
                           strip -o created debuginfo files correctly.
                           They have PT_NOTE header, but the actual note
                           content is not present in the debuginfo file,
                           only in the original stripped executable or library.
                           The corresponding .note.* section is SHT_NOBITS
                           rather than SHT_NOTE, so doshn will not look
                           at it.  */
                        if (sh_num)
                                /*@switchbreak@*/ break;
                        /*
                         * This is a PT_NOTE section; loop through all the notes
                         * in the section.
                         */
                        if (lseek(fd, xph_offset, SEEK_SET)
                            == (off_t)-1) {
                                file_badseek(ms);
                                return -1;
                        }
                        bufsize = read(fd, nbuf, ((xph_filesz < sizeof(nbuf)) ?
                            xph_filesz : sizeof(nbuf)));
                        if (bufsize == -1) {
                                file_badread(ms);
                                return -1;
                        }
                        offset = 0;
                        for (;;) {
                                if (offset >= (size_t)bufsize)
                                        /*@innerbreak@*/ break;
                                offset = donote(ms, nbuf, offset,
                                    (size_t)bufsize, iclass, swap, align,
                                    flags);
                                if (offset == 0)
                                        /*@innerbreak@*/ break;
                        }
                        if (lseek(fd, savedoffset, SEEK_SET) == (off_t)-1) {
                                file_badseek(ms);
                                return -1;
                        }
                        /*@switchbreak@*/ break;
                }
        }
        if (file_printf(ms, ", %s linked%s", linking_style, shared_libraries)
            == -1)
            return -1;
        return 0;
}


protected int
file_tryelf(struct magic_set *ms, int fd, const unsigned char *buf,
    size_t nbytes)
{
        union {
                int32_t l;
                char c[sizeof (int32_t)];
        } u;
        int iclass;
        int swap;
        struct stat st;
        off_t fsize;
        int flags = 0;

        /*
         * If we cannot seek, it must be a pipe, socket or fifo.
         */
        if((lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1) && (errno == ESPIPE))
                fd = file_pipe2file(ms, fd, buf, nbytes);

        if (fstat(fd, &st) == -1) {
                file_badread(ms);
                return -1;
        }
        fsize = st.st_size;

        /*
         * ELF executables have multiple section headers in arbitrary
         * file locations and thus file(1) cannot determine it from easily.
         * Instead we traverse thru all section headers until a symbol table
         * one is found or else the binary is stripped.
         */
        if (buf[EI_MAG0] != ELFMAG0
            || (buf[EI_MAG1] != ELFMAG1 && buf[EI_MAG1] != OLFMAG1)
            || buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3)
            return 0;


        iclass = buf[EI_CLASS];

        if (iclass == ELFCLASS32) {
                Elf32_Ehdr elfhdr;
                if (nbytes <= sizeof (Elf32_Ehdr))
                        return 0;


                u.l = 1;
                (void) memcpy(&elfhdr, buf, sizeof elfhdr);
                swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

                if (getu16(swap, elfhdr.e_type) == ET_CORE) {
#ifdef ELFCORE
                        if (dophn_core(ms, iclass, swap, fd,
                            (off_t)getu32(swap, elfhdr.e_phoff),
                            getu16(swap, elfhdr.e_phnum), 
                            (size_t)getu16(swap, elfhdr.e_phentsize),
                            fsize, &flags) == -1)
                                return -1;
#else
                        ;
#endif
                } else {
                        if (getu16(swap, elfhdr.e_type) == ET_EXEC) {
                                if (dophn_exec(ms, iclass, swap,
                                    fd, (off_t)getu32(swap, elfhdr.e_phoff),
                                    getu16(swap, elfhdr.e_phnum), 
                                    (size_t)getu16(swap, elfhdr.e_phentsize),
                                    fsize, &flags,
                    getu16(swap, elfhdr.e_shnum))
                                    == -1)
                                        return -1;
                        }
                        if (doshn(ms, iclass, swap, fd,
                            (off_t)getu32(swap, elfhdr.e_shoff),
                            getu16(swap, elfhdr.e_shnum),
                            (size_t)getu16(swap, elfhdr.e_shentsize),
                            &flags) == -1)
                                return -1;
                }
                return 1;
        }

        if (iclass == ELFCLASS64) {
                Elf64_Ehdr elfhdr;
                if (nbytes <= sizeof (Elf64_Ehdr))
                        return 0;


                u.l = 1;
                (void) memcpy(&elfhdr, buf, sizeof elfhdr);
                swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

                if (getu16(swap, elfhdr.e_type) == ET_CORE) {
#ifdef ELFCORE
                        if (dophn_core(ms, iclass, swap, fd,
                            (off_t)elf_getu64(swap, elfhdr.e_phoff),
                            getu16(swap, elfhdr.e_phnum), 
                            (size_t)getu16(swap, elfhdr.e_phentsize),
                            fsize, &flags) == -1)
                                return -1;
#else
                        ;
#endif
                } else {
                        if (getu16(swap, elfhdr.e_type) == ET_EXEC) {
                                if (dophn_exec(ms, iclass, swap, fd,
                                    (off_t)elf_getu64(swap, elfhdr.e_phoff),
                                    getu16(swap, elfhdr.e_phnum), 
                                    (size_t)getu16(swap, elfhdr.e_phentsize),
                                    fsize, &flags,
                    getu16(swap, elfhdr.e_shnum)) == -1)
                                        return -1;
                        }
                        if (doshn(ms, iclass, swap, fd,
                            (off_t)elf_getu64(swap, elfhdr.e_shoff),
                            getu16(swap, elfhdr.e_shnum),
                            (size_t)getu16(swap, elfhdr.e_shentsize), &flags)
                            == -1)
                                return -1;
                }
                return 1;
        }
        return 0;
}
#endif

struct magic_set _s_ms;
//#define DSPRTF sprtf
#define DSPRTF

int doshn_mm(struct magic_set *ms, int iclass, int swap, int fd, off_t off, int num,
    size_t size, int *flags, char * buf, size_t nbytes)
{
        Elf32_Shdr sh32;
        Elf64_Shdr sh64;
        int stripped = 1;
        void *nbuf;
        off_t noff;
        off_t seek_off;
        if (size != xsh_sizeof) {
                if (file_printf(ms, ", corrupted section header size") == -1)
                        return -1;
                return 0;
        }

        // if (lseek(fd, off, SEEK_SET) == (off_t)-1) {
        seek_off = off;
        DSPRTF("SEEK_SET:1: Offset %d\n", seek_off);
        if (seek_off >= nbytes) {
                file_badseek(ms);
                return -1;
        }

        for ( ; num; num--) {
                //if (read(fd, xsh_addr, xsh_sizeof) == -1) {
                if ((seek_off + xsh_sizeof) >= nbytes) {
                        file_badread(ms);
                        return -1;
                }
                memcpy(xsh_addr, &buf[seek_off], xsh_sizeof );
                seek_off += xsh_sizeof;
                DSPRTF("After read:1: offset %d\n",seek_off);
                switch (xsh_type) {
                case SHT_SYMTAB:
#if 0
                case SHT_DYNSYM:
#endif
                        stripped = 0;
                        /*@switchbreak@*/ break;
                case SHT_NOTE:
                        //if ((off = lseek(fd, (off_t)0, SEEK_CUR)) == (off_t)-1) {
                        //        file_badread(ms);
                        //        return -1;
                        //}
                        off = seek_off;
                        if ((nbuf = malloc((size_t)xsh_size)) == NULL) {
                                file_error(ms, errno, "Cannot allocate memory"
                                    " for note");
                                return -1;
                        }
                        //if ((noff = lseek(fd, (off_t)xsh_offset, SEEK_SET)) == (off_t)-1) {
                        noff = (off_t)xsh_offset;
                        seek_off = noff;
                        DSPRTF("SEEK_SET:2: Offset %d\n", seek_off);
                        if ((noff + xsh_size) >= nbytes) {
                                file_badread(ms);
                                free(nbuf);
                                return -1;
                        }
                        //if (read(fd, nbuf, (size_t)xsh_size) != (ssize_t)xsh_size) {
                        //        free(nbuf);
                        //        file_badread(ms);
                        //        return -1;
                        //}
                        memcpy(nbuf, &buf[noff], (int)xsh_size);
                        seek_off += (off_t)xsh_size;
                        DSPRTF("After read:2: offset %d\n",seek_off);
                        noff = 0;
                        for (;;) {
                                if (noff >= (size_t)xsh_size)
                                        /*@innerbreak@*/ break;
                                noff = donote(ms, (unsigned char *)nbuf, (size_t)noff,
                                    (size_t)xsh_size, iclass, swap, 4,
                                    flags);
                                if (noff == 0)
                                        /*@innerbreak@*/ break;
                        }
                        //if ((lseek(fd, off, SEEK_SET)) == (off_t)-1) {
                        seek_off = off;
                        DSPRTF("SEEK_SET:3: Offset %d\n", seek_off);
                        if (seek_off >= nbytes) {
                                free(nbuf);
                                file_badread(ms);
                                return -1;
                        }
                        free(nbuf);
                        /*@switchbreak@*/ break;
                }
        }
        if (file_printf(ms, ", %sstripped", stripped ? "" : "not ") == -1)
                return -1;
        return 0;
}

int IsELFFile(char *buf, size_t nbytes)
{
    union {
        int32_t l;
        char c[sizeof (int32_t)];
    } u;
    struct magic_set *ms = &_s_ms;
    int iclass;
    int swap;
    //struct stat st;
    off_t fsize;
    int flags = 0;
    int fd = -1;
    int e_type;
    long long e_shoff;
    long long e_phoff;
    int e_shnum;
    int e_phnum;
    int e_phentsize;
    int e_shentsize;

    e_shoff = sizeof(struct Elf_Ehdr);
    e_phoff = sizeof(Elf64_Ehdr);
    if (e_shoff != e_phoff) {
        sprtf("Check the structure sizes beye = %I64u, dump4 = %I64u!\n", e_shoff, e_phoff);
    }
    fsize = nbytes;
        /*
         * ELF executables have multiple section headers in arbitrary
         * file locations and thus file(1) cannot determine it from easily.
         * Instead we traverse thru all section headers until a symbol table
         * one is found or else the binary is stripped.
         */

        // check for 0x7f,"ELF" header
        if (buf[EI_MAG0] != ELFMAG0
            || (buf[EI_MAG1] != ELFMAG1 && buf[EI_MAG1] != OLFMAG1)
            || buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3)
            return 0;


        iclass = buf[EI_CLASS];

        if (iclass == ELFCLASS32) {
                Elf32_Ehdr elfhdr;
                if (nbytes <= sizeof (Elf32_Ehdr))
                        return 0;


                u.l = 1;
                (void) memcpy(&elfhdr, buf, sizeof elfhdr);
                swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

                //if (getu16(swap, elfhdr.e_type) == ET_CORE) {
                e_type = getu16(swap, elfhdr.e_type);
                e_phoff = getu32(swap, elfhdr.e_phoff);
                e_phnum = getu16(swap, elfhdr.e_phnum);
                e_phentsize = getu16(swap, elfhdr.e_phentsize);
                e_shoff = getu32(swap, elfhdr.e_shoff);
                e_shnum = getu16(swap, elfhdr.e_shnum);
                e_shentsize = getu16(swap, elfhdr.e_shentsize);
                if (e_type == ET_CORE) {
#ifdef ELFCORE
                        if (dophn_core(ms, iclass, swap, fd,
                            (off_t)e_phoff,         // getu32(swap, elfhdr.e_phoff),
                            e_phnum,                // getu16(swap, elfhdr.e_phnum), 
                            (size_t)e_phentsize,    // getu16(swap, elfhdr.e_phentsize),
                            fsize, &flags) == -1)
                                return -1;
#else
                        ;
#endif
                } else {
                        //if (getu16(swap, elfhdr.e_type) == ET_EXEC) {
                        if (e_type == ET_EXEC) {
                                if (dophn_exec(ms, iclass, swap, fd,
                                    (off_t)e_phoff,         // getu32(swap, elfhdr.e_phoff),
                                    e_phnum,                // getu16(swap, elfhdr.e_phnum), 
                                    (size_t)e_phentsize,    // getu16(swap, elfhdr.e_phentsize),
                                    fsize, &flags,
                                    e_shnum)                // getu16(swap, elfhdr.e_shnum))
                                    == -1)
                                        return -1;
                        }
                        if (doshn_mm(ms, iclass, swap, fd,
                            (off_t)e_shoff,                 // getu32(swap, elfhdr.e_shoff),
                            e_shnum,                        // getu16(swap, elfhdr.e_shnum),
                            (size_t)e_shentsize,            // getu16(swap, elfhdr.e_shentsize),
                            &flags, buf, nbytes) == -1)
                                return -1;
                }
                return 1;
        }

        if (iclass == ELFCLASS64) {
                Elf64_Ehdr elfhdr;
                if (nbytes <= sizeof (Elf64_Ehdr))
                        return 0;


                u.l = 1;
                (void) memcpy(&elfhdr, buf, sizeof elfhdr);
                swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

                e_type = getu16(swap, elfhdr.e_type);
                // if (getu16(swap, elfhdr.e_type) == ET_CORE) {
                e_phoff = elf_getu64(swap, elfhdr.e_phoff);
                e_phnum = getu16(swap, elfhdr.e_phnum);
                e_phentsize = getu16(swap, elfhdr.e_phentsize);
                e_shentsize = getu16(swap, elfhdr.e_shentsize);
                e_shoff = elf_getu64(swap, elfhdr.e_shoff);
                e_shnum = getu16(swap, elfhdr.e_shnum);
                if (e_type == ET_CORE) {
#ifdef ELFCORE
                        if (dophn_core(ms, iclass, swap, fd,
                            (off_t)e_phoff,             // elf_getu64(swap, elfhdr.e_phoff),
                            e_phnum,                    // getu16(swap, elfhdr.e_phnum), 
                            (size_t)e_phentsize,        // getu16(swap, elfhdr.e_phentsize),
                            fsize, &flags) == -1)
                                return -1;
#else
                        ;
#endif
                } else {
                        //if (getu16(swap, elfhdr.e_type) == ET_EXEC) {
                        if (e_type == ET_EXEC) {
                                if (dophn_exec(ms, iclass, swap, fd,
                                    (off_t)e_phoff,         // elf_getu64(swap, elfhdr.e_phoff),
                                    e_phnum,                // getu16(swap, elfhdr.e_phnum), 
                                    (size_t)e_phentsize,    // getu16(swap, elfhdr.e_phentsize),
                                    fsize, &flags,
                                    e_shnum )               // getu16(swap, elfhdr.e_shnum))
                                    == -1)
                                        return -1;
                        }
                        if (doshn_mm(ms, iclass, swap, fd,
                            (off_t)e_shoff,                 // elf_getu64(swap, elfhdr.e_shoff),
                            e_shnum,                        // getu16(swap, elfhdr.e_shnum),
                            (size_t)e_shentsize,            // getu16(swap, elfhdr.e_shentsize),
                            &flags, buf, nbytes)
                            == -1)
                                return -1;
                }
                return 1;
        }
        return 0;
}

// 20130819 - Another go at dumping an ELF
// hopefully using the beye code as a guide, and the URLS given
inline uint16_t bswap_16(uint16_t x) { return ((x)&0x00ff)<<8|((x)&0xff00)>>8; }
// code from bits/byteswap.h (C) 1997, 1998 Free Software Foundation, Inc.
inline uint32_t bswap_32(uint32_t x) {
	return (((x)&0xff000000)>>24)|(((x)&0x00ff0000)>>8)|
		(((x)&0x0000ff00)<<8)|(((x)&0x000000ff)<<24);
    }
inline uint64_t bswap_64(uint64_t x) {
	return (((x) &0xff00000000000000ull)>>56)|
		(((x)&0x00ff000000000000ull)>>40)|
		(((x)&0x0000ff0000000000ull)>>24)|
		(((x)&0x000000ff00000000ull)>>8)|
		(((x)&0x00000000ff000000ull)<<8)|
		(((x)&0x0000000000ff0000ull)<<24)|
		(((x)&0x000000000000ff00ull)<<40)|
		(((x)&0x00000000000000ffull)<<56);
    }

std::string elf_class(unsigned char id)
{
    switch(id) {
	case ELFCLASSNONE:	return "Invalid";
	case ELFCLASS32:	return "32-bit";
	case ELFCLASS64:	return "64-bit";
	default:		break;
    }
    std::ostringstream oss;
    oss << "." << std::hex << std::setfill('0') << std::setw(2) << unsigned(id);
    return oss.str();
}

std::string elf_data(unsigned char id)
{
    switch(id) {
	case ELFDATANONE:	return "Invalid";
	case ELFDATA2LSB:	return "LSB - little endian";
	case ELFDATA2MSB:	return "MSB - big endian";
	default:		break;
    }
    std::ostringstream oss;
    oss<<"."<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(id);
    return oss.str();
}

/** Values for e_version */
enum {
    EV_NONE	=0,		/**< Invalid ELF version */
    EV_CURRENT	=1		/**< Current version */
};

#if 0
enum {
    EI_NIDENT	=16,		/**< Size of e_ident[] */
/** Fields in e_ident[] */

    EI_MAG0	=0,		/**< File identification byte 0 index */
    ELFMAG0	=0x7F,		/**< Magic number byte 0 */

    EI_MAG1	=1,		/**< File identification byte 1 index */
    ELFMAG1	='E',		/**< Magic number byte 1 */

    EI_MAG2	=2,		/**< File identification byte 2 index */
    ELFMAG2	='L',		/**< Magic number byte 2 */

    EI_MAG3	=3,		/**< File identification byte 3 index */
    ELFMAG3	='F',		/**< Magic number byte 3 */

    EI_CLASS	=4,		/**< File class */
    ELFCLASSNONE=0,		/**< Invalid class */
    ELFCLASS32	=1,		/**< 32-bit objects */
    ELFCLASS64	=2,		/**< 64-bit objects */

    EI_DATA	=5,		/**< Data encoding */
    ELFDATANONE	=0,		/**< Invalid data encoding */
    ELFDATA2LSB	=1,		/**< 2's complement, little endian */
    ELFDATA2MSB	=2,		/**< 2's complement, big endian */

    EI_VERSION	=6,		/**< File version */
    EI_OSABI	=7		/**< Operating system / ABI identification */
};

#endif


std::string elf_version(unsigned long id)
{
    switch(id) {
	case EV_NONE:    return "Invalid";
	case EV_CURRENT: return "Current";
	default:         break;
    }
    std::ostringstream oss;
    oss<<"."<<std::hex<<std::setfill('0')<<std::setw(8)<<id;
    return oss.str();
}

enum {
    ELFOSABI_SYSV	=0,	/**< UNIX System V ABI */
    ELFOSABI_NONE	=ELFOSABI_SYSV,	/**< symbol used in old spec */
    ELFOSABI_HPUX	=1,	/**< HP-UX operating system */
    ELFOSABI_NETBSD	=2,	/**< NetBSD */
    ELFOSABI_LINUX	=3,	/**< GNU/Linux */
    ELFOSABI_HURD	=4,	/**< GNU/Hurd */
    ELFOSABI_86OPEN	=5,	/**< 86Open common IA32 ABI */
    ELFOSABI_SOLARIS	=6,	/**< Solaris */
    ELFOSABI_MONTEREY	=7,	/**< Monterey */
    ELFOSABI_IRIX	=8,	/**< IRIX */
    ELFOSABI_FREEBSD	=9,	/**< FreeBSD */
    ELFOSABI_TRU64	=10,	/**< TRU64 UNIX */
    ELFOSABI_MODESTO	=11,	/**< Novell Modesto */
    ELFOSABI_OPENBSD	=12,	/**< OpenBSD */
    ELFOSABI_ARM	=97,	/**< ARM */
    ELFOSABI_STANDALONE	=255,	/**< Standalone (embedded) application */

    EI_ABIVERSION	=8,	/**< ABI version */
    OLD_EI_BRAND	=8,	/**< Start of architecture identification. */
    EI_PAD		=9	/**< Start of padding (per SVR4 ABI). */
};

std::string elf_osabi(unsigned char id)
{
    switch(id) {
	case ELFOSABI_SYSV:		return "UNIX System V";
	case ELFOSABI_HPUX:		return "HP-UX";
	case ELFOSABI_NETBSD:		return "NetBSD";
	case ELFOSABI_LINUX:		return "GNU/Linux";
	case ELFOSABI_HURD:		return "GNU/Hurd";
	case ELFOSABI_86OPEN:		return "86Open";
	case ELFOSABI_SOLARIS:		return "Solaris";
	case ELFOSABI_MONTEREY:		return "Monterey";
	case ELFOSABI_IRIX:		return "IRIX";
	case ELFOSABI_FREEBSD:		return "FreeBSD";
	case ELFOSABI_TRU64:		return "TRU64 UNIX";
	case ELFOSABI_MODESTO:		return "Novell Modesto";
	case ELFOSABI_OPENBSD:		return "OpenBSD";
	case ELFOSABI_ARM:		return "ARM";
	case ELFOSABI_STANDALONE:	return "Standalone (embedded) application";
        default:			break;
    }
    std::ostringstream oss;
    oss<<"."<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(id);
    return oss.str();
}


std::string elf_otype(unsigned id)
{
    switch(id) {
	case ET_NONE:	return "none";
	case ET_REL:	return "relocatable";
	case ET_EXEC:	return "executable";
	case ET_DYN:	return "shared object";
	case ET_CORE:	return "core";
	case ET_LOOS:	return "OS-specific low";
	case ET_HIOS:	return "OS-specific high";
	case ET_LOPROC:	return "CPU-specific low";
	case ET_HIPROC:	return "CPU-specific high";
	default:	break;
    }
    std::ostringstream oss;
    oss<<"."<<std::hex<<std::setfill('0')<<std::setw(8)<<id;
    return oss.str();
}

/*
    only common machine types are used, add remaining if needed
*/

std::string elf_machine(unsigned id,unsigned& disasm)
{
    disasm=DISASM_DATA;
    switch(id) {
	case EM_NONE:	return "None";
	case EM_M32:	return "AT&T WE32100";
	case EM_SPARC:	disasm = DISASM_CPU_SPARC; return "Sun SPARC";
	case EM_386:	disasm = DISASM_CPU_IX86; return "Intel 386";
	case EM_68K:	disasm = DISASM_CPU_PPC; return "Motorola m68k";
	case EM_88K:	disasm = DISASM_CPU_PPC; return "Motorola m88k";
	case EM_860:	return "Intel 80860";
	case EM_MIPS:	disasm = DISASM_CPU_MIPS; return "MIPS I";
	case EM_S370:	return "IBM System/370";
	case EM_MIPS_RS3_LE:disasm = DISASM_CPU_MIPS; return "MIPS R3000";
	case EM_PARISC:	return "HP PA-RISC";
	case EM_SPARC32PLUS:disasm = DISASM_CPU_SPARC; return "SPARC v8plus";
	case EM_960:	return "Intel 80960";
	case EM_PPC:	disasm = DISASM_CPU_PPC; return "Power PC 32-bit";
	case EM_PPC64:	disasm = DISASM_CPU_PPC; return "Power PC 64-bit";
	case EM_S390:	return "IBM System/390";
	case EM_ADSP:	return "Atmel ADSP";
	case EM_V800:	return "NEC V800";
	case EM_FR20:	return "Fujitsu FR20";
	case EM_RH32:	return "TRW RH-32";
	case EM_RCE:	return "Motorola RCE";
	case EM_ARM:	disasm=DISASM_CPU_ARM; return "ARM";
	case EM_ALPHA:	disasm = DISASM_CPU_ALPHA; return "DEC Alpha";
	case EM_SH:	disasm = DISASM_CPU_SH; return "Hitachi SH";
	case EM_SPARCV9:disasm = DISASM_CPU_SPARC; return "SPARC v9 64-bit";
	case EM_TRICORE:return "Siemens TriCore embedded processor";
	case EM_ARC:	return "Argonaut RISC Core";
	case EM_H8_300:	return "Hitachi H8/300";
	case EM_H8_300H:return "Hitachi H8/300H";
	case EM_H8S:	return "Hitachi H8S";
	case EM_H8_500:	return "Hitachi H8/500";
	case EM_IA_64:	disasm = DISASM_CPU_IA64; return "Intel IA-64";
	case EM_MIPS_X:	disasm = DISASM_CPU_MIPS; return "Stanford MIPS-X";
	case EM_COLDFIRE:return "Motorola ColdFire";
	case EM_68HC12:	return "Motorola M68HC12";
	case EM_MMA:	return "Fujitsu MMA Multimedia Accelerator";
	case EM_PCP:	return "Siemens PCP";
	case EM_NCPU:	return "Sony nCPU embedded RISC processor";
	case EM_NDR1:	return "Denso NDR1 microprocessor";
	case EM_STARCORE:return "Motorola StarCore processor";
	case EM_ME16:	return "Toyota ME16 processor";
	case EM_ST100:	return "STMicroelectronics ST100 processor";
	case EM_TINYJ:	return "Advanced Logic Corp. TinyJ";
	case EM_X86_64:	disasm = DISASM_CPU_IX86; return "AMD x86-64";
	case EM_PDSP:	return "Sony DSP Processor";
	case EM_PDP10:	return "DEC PDP-10";
	case EM_PDP11:	return "DEC PDP-11";
	case EM_FX66:	return "Siemens FX66 microcontroller";
	case EM_ST9PLUS:return "STMicroelectronics ST9+ 8/16 bit microcontroller";
	case EM_ST7:	return "STMicroelectronics ST7 8-bit microcontroller";
	case EM_68HC16:	return "Motorola MC68HC16 Microcontroller";
	case EM_68HC11:	return "Motorola MC68HC11 Microcontroller";
	case EM_68HC08:	return "Motorola MC68HC08 Microcontroller";
	case EM_68HC05:	return "Motorola MC68HC05 Microcontroller";
	case EM_SVX:	return "Silicon Graphics SVx";
	case EM_ST19:	return "STMicroelectronics ST19 8-bit microcontroller";
	case EM_VAX:	return "DEC VAX";
	case EM_CRIS:	return "Axis Comm. 32-bit embedded processor";
	case EM_JAVELIN:return "Infineon Tech. 32-bit embedded processor";
	case EM_FIREPATH:return "Element 14 64-bit DSP Processor";
	case EM_ZSP:	return "LSI Logic 16-bit DSP Processor";
	case EM_MMIX:	return "Donald Knuth's educational 64-bit processor";
	case EM_HUANY:	return "Harvard University machine-independent object files";
	case EM_PRISM:	return "SiTera Prism";
	case EM_AVR:	disasm=DISASM_CPU_AVR; return "Atmel AVR 8-bit";
	case EM_FR30:	return "Fujitsu FR30";
	case EM_D10V:	return "Mitsubishi D10V";
	case EM_D30V:	return "Mitsubishi D30V";
	case EM_V850:	return "NEC v850";
	case EM_M32R:	return "Mitsubishi M32R";
	case EM_MN10300:return "Matsushita MN10300";
	case EM_MN10200:return "Matsushita MN10200";
	case EM_PJ:	return "picoJava";
	case EM_OPENRISC:return "OpenRISC 32-bit embedded processor";
	case EM_ARC_A5:	return "ARC Cores Tangent-A5";
	case EM_XTENSA:	return "Tensilica Xtensa Architecture";
	case EM_VIDEOCORE:return "Alphamosaic VideoCore processor";
	case EM_TMM_GPP:return "Thompson Multimedia General Purpose Processor";
	case EM_NS32K:	return "National Semiconductor 32000 series";
	case EM_TPC:	return "Tenor Network TPC processor";
	case EM_SNP1K:	return "Trebia SNP 1000 processor";
	case EM_IP2K:	return "Ubicom IP2022 micro controller";
	case EM_CR:	return "National Semiconductor CompactRISC";
	case EM_MSP430:	return "TI msp430 micro controller";
	case EM_BLACKFIN:return "ADI Blackfin";
	case EM_ALTERA_NIOS2: return "Altera Nios II soft-core processor";
	case EM_CRX:	return "National Semiconductor CRX";
	case EM_XGATE:	return "Motorola XGATE embedded processor";
	case EM_C166:	return "Infineon C16x/XC16x processor";
	case EM_M16C:	return "Renesas M16C series microprocessors";
	case EM_DSPIC30F:return "Microchip Technology dsPIC30F Digital Signal Controller";
	case EM_CE:	return "Freescale Communication Engine RISC core";
	case EM_M32C:	return "Renesas M32C series microprocessors";
	case EM_TSK3000:return "Altium TSK3000 core";
	case EM_RS08:	return "Freescale RS08 embedded processor";
	case EM_ECOG2:	return "Cyan Technology eCOG2 microprocessor";
	case EM_SCORE:	return "Sunplus Score";
	case EM_DSP24:	return "New Japan Radio (NJR) 24-bit DSP Processor";
	case EM_VIDEOCORE3:return "Broadcom VideoCore III processor";
	case EM_LATTICEMICO32: return "RISC processor for Lattice FPGA architecture";
	case EM_SE_C17:	return "Seiko Epson C17 family";
	case EM_MMDSP_PLUS:return "STMicroelectronics 64bit VLIW Data Signal Processor";
	case EM_CYPRESS_M8C:return "Cypress M8C microprocessor";
	case EM_R32C:	return "Renesas R32C series microprocessors";
	case EM_TRIMEDIA:return "NXP Semiconductors TriMedia architecture family";
	case EM_QDSP6:	return "QUALCOMM DSP6 Processor";
	case EM_8051:	return "Intel 8051 and variants";
	case EM_STXP7X:	return "STMicroelectronics STxP7x family";
	case EM_NDS32:	return "Andes Technology compact code size embedded RISC processor family";
	case EM_ECOG1X:	return "Cyan Technology eCOG1X family";
	case EM_MAXQ30:	return "Dallas Semiconductor MAXQ30 Core Micro-controllers";
	case EM_XIMO16:	return "New Japan Radio (NJR) 16-bit DSP Processor";
	case EM_MANIK:	return "M2000 Reconfigurable RISC Microprocessor";
	case EM_CRAYNV2:disasm = DISASM_CPU_CRAY; return "Cray Inc. NV2 vector architecture";
	case EM_RX:	return "Renesas RX family";
	case EM_METAG:	return "Imagination Technologies META processor architecture";
	case EM_MCST_ELBRUS:return "MCST Elbrus general purpose hardware architecture";
	case EM_ECOG16:	return "Cyan Technology eCOG16 family";
	case EM_CR16:	return "National Semiconductor CompactRISC 16-bit processor";
	default:	break;
    }
    std::ostringstream oss;
    oss<<"."<<std::hex<<std::setfill('0')<<std::setw(8)<<id;
    return oss.str();
}


static char _s_buf[2048];
int DumpELFFile(char *buf, size_t nbytes)
{
    char *cp = _s_buf;
    bool is_msbf;
    unsigned int dummy;
    // check for 0x7f,"ELF" header
    if (buf[EI_MAG0] != ELFMAG0
        || (buf[EI_MAG1] != ELFMAG1 && buf[EI_MAG1] != OLFMAG1)
        || buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3)
        return 1;   // failed first test
    int iclass = buf[EI_CLASS];
    if (iclass == ELFCLASS32) {
        Elf32_Ehdr _ehdr;
        if (nbytes <= sizeof (Elf32_Ehdr)) {
            return 3; // not enough size
        }
        memcpy(&_ehdr, buf, sizeof _ehdr);
        is_msbf = (_ehdr.e_ident[EI_DATA] == ELFDATA2MSB);
        sprintf(cp,
	           "Signature                         = %02X %02X %02X %02XH (%c%c%c%c)\n"
	           "File class                        = %02XH (%s)\n"
	           "Data encoding                     = %02XH (%s)\n"
	           "ELF header version                = %02XH (%s)\n"
	           "Operating system / ABI            = %02XH (%s)\n"
	           "ABI version                       = %02XH (%d)\n"
	           "Object file type                  = %04XH (%s)\n"
	           "Machine architecture              = %04XH (%s)\n"
	           "Object file version               = %08lXH (%s)\n"
	            ,_ehdr.e_ident[EI_MAG0],	_ehdr.e_ident[EI_MAG1]
	            ,_ehdr.e_ident[EI_MAG2],	_ehdr.e_ident[EI_MAG3]
	            ,_ehdr.e_ident[EI_MAG0],	_ehdr.e_ident[EI_MAG1]
	            ,_ehdr.e_ident[EI_MAG2],	_ehdr.e_ident[EI_MAG3]
	            ,_ehdr.e_ident[EI_CLASS],	elf_class(_ehdr.e_ident[EI_CLASS]).c_str()
	            ,_ehdr.e_ident[EI_DATA],	elf_data(_ehdr.e_ident[EI_DATA]).c_str()
	            ,_ehdr.e_ident[EI_VERSION],	elf_version(_ehdr.e_ident[EI_VERSION]).c_str()
	            ,_ehdr.e_ident[EI_OSABI],	elf_osabi(_ehdr.e_ident[EI_OSABI]).c_str()
	            ,_ehdr.e_ident[EI_ABIVERSION],_ehdr.e_ident[EI_ABIVERSION]
	            ,_ehdr.e_type,	elf_otype(_ehdr.e_type).c_str()
	            ,_ehdr.e_machine,	elf_machine(_ehdr.e_machine,dummy).c_str()
	            ,_ehdr.e_version,	elf_version(_ehdr.e_version).c_str()
	            );
        sprtf("%s",cp);
    } else if (iclass == ELFCLASS64) {
        Elf64_Ehdr _ehdr;
        if (nbytes <= sizeof (Elf64_Ehdr)) {
            return 3; // insufficient size to even have this header
        }
        memcpy(&_ehdr, buf, sizeof _ehdr);
        is_msbf = (_ehdr.e_ident[EI_DATA] == ELFDATA2MSB);
        sprintf(cp,
	           "Signature                         = %02X %02X %02X %02XH (%c%c%c%c)\n"
	           "File class                        = %02XH (%s)\n"
	           "Data encoding                     = %02XH (%s)\n"
	           "ELF header version                = %02XH (%s)\n"
	           "Operating system / ABI            = %02XH (%s)\n"
	           "ABI version                       = %02XH (%d)\n"
	           "Object file type                  = %04XH (%s)\n"
	           "Machine architecture              = %04XH (%s)\n"
	           "Object file version               = %08lXH (%s)\n"
	            ,_ehdr.e_ident[EI_MAG0],	_ehdr.e_ident[EI_MAG1]
	            ,_ehdr.e_ident[EI_MAG2],	_ehdr.e_ident[EI_MAG3]
	            ,_ehdr.e_ident[EI_MAG0],	_ehdr.e_ident[EI_MAG1]
	            ,_ehdr.e_ident[EI_MAG2],	_ehdr.e_ident[EI_MAG3]
	            ,_ehdr.e_ident[EI_CLASS],	elf_class(_ehdr.e_ident[EI_CLASS]).c_str()
	            ,_ehdr.e_ident[EI_DATA],	elf_data(_ehdr.e_ident[EI_DATA]).c_str()
	            ,_ehdr.e_ident[EI_VERSION],	elf_version(_ehdr.e_ident[EI_VERSION]).c_str()
	            ,_ehdr.e_ident[EI_OSABI],	elf_osabi(_ehdr.e_ident[EI_OSABI]).c_str()
	            ,_ehdr.e_ident[EI_ABIVERSION],_ehdr.e_ident[EI_ABIVERSION]
	            ,_ehdr.e_type,	elf_otype(_ehdr.e_type).c_str()
	            ,_ehdr.e_machine,	elf_machine(_ehdr.e_machine,dummy).c_str()
	            ,_ehdr.e_version,	elf_version(_ehdr.e_version).c_str()
	            );
        sprtf("%s",cp);
    } else {
        sprtf("Not class 32 or 64! Unknown class %d\n", iclass);
        return 2; // not a known class
    }

    return 0;   // successful dump done
}

/////////////////////////////////////////////////////////////////////
#endif // WIN32

// eof - DumpElf.cxx
