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
 * @(#)Id: readelf.h,v 1.9 2002/05/16 18:45:56 christos Exp
 *
 * Provide elf data structures for non-elf machines, allowing file
 * non-elf hosts to determine if an elf binary is stripped.
 * Note: cobbled from the linux header file, with modifications
 */
#ifndef __fake_elf_h__
#define __fake_elf_h__

#if HAVE_STDINT_H
#include <stdint.h>
#endif

typedef uint32_t        Elf32_Addr;
typedef uint32_t        Elf32_Off;
typedef uint16_t        Elf32_Half;
typedef uint32_t        Elf32_Word;
typedef uint8_t         Elf32_Char;

#if SIZEOF_UINT64_T != 8
#define USE_ARRAY_FOR_64BIT_TYPES
typedef uint32_t        Elf64_Addr[2];
typedef uint32_t        Elf64_Off[2];
typedef uint32_t        Elf64_Xword[2];
#else
#undef USE_ARRAY_FOR_64BIT_TYPES
typedef uint64_t        Elf64_Addr;
typedef uint64_t        Elf64_Off;
typedef uint64_t        Elf64_Xword;
#endif
typedef uint16_t        Elf64_Half;
typedef uint32_t        Elf64_Word;
typedef uint8_t         Elf64_Char;

//#define EI_NIDENT       16
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


//#pragma pack(show)
#pragma pack(1) // establish byte allignment
//#pragma pack(show)

// compare with beye code
struct Elf_Ehdr {
uint8_t		e_ident[16];	/**< ELF "magic number" */
uint16_t	e_type;		/**< Identifies object file type */
uint16_t	e_machine;	/**< Specifies required architecture */
uint32_t	e_version;	/**< Identifies object file version */
uint64_t	e_entry;	/**< Entry point virtual address */
uint64_t	e_phoff;	/**< Program header table file offset */
uint64_t	e_shoff;	/**< Section header table file offset */
uint32_t	e_flags;	/**< Processor-specific flags */
uint16_t	e_ehsize;	/**< ELF header size in bytes */
uint16_t	e_phentsize;	/**< Program header table entry size */
uint16_t	e_phnum;	/**< Program header table entry count */
uint16_t	e_shentsize;	/**< Section header table entry size */
uint16_t	e_shnum;	/**< Section header table entry count */
uint16_t	e_shstrndx;	/**< Section header string table index */
};

typedef struct {
    Elf32_Char  e_ident[EI_NIDENT];
    Elf32_Half  e_type;
    Elf32_Half  e_machine;
    Elf32_Word  e_version;
    Elf32_Addr  e_entry;  /* Entry point */
    Elf32_Off   e_phoff;
    Elf32_Off   e_shoff;
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize;
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    Elf32_Half  e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf64_Char  e_ident[EI_NIDENT];
    Elf64_Half  e_type;
    Elf64_Half  e_machine;
    Elf64_Word  e_version;
    Elf64_Addr  e_entry;  /* Entry point */
    Elf64_Off   e_phoff;
    Elf64_Off   e_shoff;
    Elf64_Word  e_flags;
    Elf64_Half  e_ehsize;
    Elf64_Half  e_phentsize;
    Elf64_Half  e_phnum;
    Elf64_Half  e_shentsize;
    Elf64_Half  e_shnum;
    Elf64_Half  e_shstrndx;
} Elf64_Ehdr;

/* e_type */
//#define ET_EXEC         2
//#define ET_CORE         4
/** Values for e_type, which identifies the object file type */
enum {
    ET_NONE	=0,		/**< No file type */
    ET_REL	=1,		/**< Relocatable file */
    ET_EXEC	=2,		/**< Executable file */
    ET_DYN	=3,		/**< Shared object file */
    ET_CORE	=4,		/**< Core file */
    ET_LOOS	=0xFE00U,		/**< OS-specific */
    ET_HIOS	=0xFEFFU,		/**< OS-specific */
    ET_LOPROC	=0xFF00U,		/**< Processor-specific */
    ET_HIPROC	=0xFFFFU		/**< Processor-specific */
};

/** Values for e_machine, which identifies the architecture */
enum {
    EM_NONE		=0,	/**< No machine */
    EM_M32		=1,	/**< AT&T WE 32100 */
    EM_SPARC		=2,	/**< Sun SPARC */
    EM_386		=3,	/**< Intel 80386 */
    EM_68K		=4,	/**< Motorola m68k */
    EM_88K		=5,	/**< Motorola m88k */
				/* 6 reserved, was EM_486 */
    EM_860		=7,	/**< Intel 80860 */
    EM_MIPS		=8,	/**< MIPS I */
    EM_S370		=9,	/**< IBM System/370 */
    EM_MIPS_RS3_LE	=10,	/**< MIPS R3000 little-endian */
				/**< 11-14 reserved */
    EM_PARISC		=15,	/**< HP PA-RISC */
				/**< 16 reserved */
    EM_VPP500		=17,	/**< Fujitsu VPP500 */
    EM_SPARC32PLUS	=18,	/**< Sun SPARC v8plus */
    EM_960		=19,	/**< Intel 80960 */
    EM_PPC		=20,	/**< PowerPC */
    EM_PPC64		=21,	/**< PowerPC 64-bit */
    EM_S390		=22,	/**< IBM System/390 */
				/**< 23-35 reserved */
    EM_ADSP		=29,	/**< Atmel ADSP */
    EM_V800		=36,	/**< NEC V800 */
    EM_FR20		=37,	/**< Fujitsu FR20 */
    EM_RH32		=38,	/**< TRW RH-32 */
    EM_RCE		=39,	/**< Motorola RCE */
    EM_ARM		=40,	/**< Advanced RISC Machines ARM */
    EM_ALPHA		=41,	/**< Digital Alpha */
    EM_SH		=42,	/**< Hitachi SH */
    EM_SPARCV9		=43,	/**< SPARC Version 9 64-bit */
    EM_TRICORE		=44,	/**< Siemens TriCore embedded processor */
    EM_ARC		=45,	/**< Argonaut RISC Core, Argonaut Technologies Inc. */
    EM_H8_300		=46,	/**< Hitachi H8/300 */
    EM_H8_300H		=47,	/**< Hitachi H8/300H */
    EM_H8S		=48,	/**< Hitachi H8S */
    EM_H8_500		=49,	/**< Hitachi H8/500 */
    EM_IA_64		=50,	/**< Intel IA-64 processor architecture */
    EM_MIPS_X		=51,	/**< Stanford MIPS-X */
    EM_COLDFIRE		=52,	/**< Motorola ColdFire */
    EM_68HC12		=53,	/**< Motorola M68HC12 */
    EM_MMA		=54,	/**< Fujitsu MMA Multimedia Accelerator */
    EM_PCP		=55,	/**< Siemens PCP */
    EM_NCPU		=56,	/**< Sony nCPU embedded RISC processor */
    EM_NDR1		=57,	/**< Denso NDR1 microprocessor */
    EM_STARCORE		=58,	/**< Motorola Star*Core processor */
    EM_ME16		=59,	/**< Toyota ME16 processor */
    EM_ST100		=60,	/**< STMicroelectronics ST100 processor */
    EM_TINYJ		=61,	/**< Advanced Logic Corp. TinyJ embedded processor family */
    EM_X86_64		=62,	/**< AMD x86-64 architecture */
    EM_PDSP		=63,	/**< Sony DSP Processor */
    EM_PDP10		=64,	/**< Digital Equipment Corp. PDP-10 */
    EM_PDP11		=65,	/**< Digital Equipment Corp. PDP-11 */
    EM_FX66		=66,	/**< Siemens FX66 microcontroller */
    EM_ST9PLUS		=67,	/**< STMicroelectronics ST9+ 8/16 bit microcontroller */
    EM_ST7		=68,	/**< STMicroelectronics ST7 8-bit microcontroller */
    EM_68HC16		=69,	/**< Motorola MC68HC16 Microcontroller */
    EM_68HC11		=70,	/**< Motorola MC68HC11 Microcontroller */
    EM_68HC08		=71,	/**< Motorola MC68HC08 Microcontroller */
    EM_68HC05		=72,	/**< Motorola MC68HC05 Microcontroller */
    EM_SVX		=73,	/**< Silicon Graphics SVx */
    EM_ST19		=74,	/**< STMicroelectronics ST19 8-bit microcontroller */
    EM_VAX		=75,	/**< Digital VAX */
    EM_CRIS		=76,	/**< Axis Communications 32-bit embedded processor */
    EM_JAVELIN		=77,	/**< Infineon Technologies 32-bit embedded processor */
    EM_FIREPATH		=78,	/**< Element 14 64-bit DSP Processor */
    EM_ZSP		=79,	/**< LSI Logic 16-bit DSP Processor */
    EM_MMIX		=80,	/**< Donald Knuth's educational 64-bit processor */
    EM_HUANY		=81,	/**< Harvard University machine-independent object files */
    EM_PRISM		=82,	/**< SiTera Prism */
    EM_AVR		=83,	/**< Atmel AVR 8-bit microcontroller */
    EM_FR30		=84,	/**< Fujitsu FR30 */
    EM_D10V		=85,	/**< Mitsubishi D10V */
    EM_D30V		=86,	/**< Mitsubishi D30V */
    EM_V850		=87,	/**< NEC v850 */
    EM_M32R		=88,	/**< Mitsubishi M32R */
    EM_MN10300		=89,	/**< Matsushita MN10300 */
    EM_MN10200		=90,	/**< Matsushita MN10200 */
    EM_PJ		=91,	/**< picoJava */
    EM_OPENRISC		=92,	/**< OpenRISC 32-bit embedded processor */
    EM_ARC_A5		=93,	/**< ARC Cores Tangent-A5 */
    EM_XTENSA		=94,	/**< Tensilica Xtensa Architecture */
    EM_VIDEOCORE	=95,	/**< Alphamosaic VideoCore processor */
    EM_TMM_GPP		=96,	/**< Thompson Multimedia General Purpose Processor */
    EM_NS32K		=97,	/**< National Semiconductor 32000 series */
    EM_TPC		=98,	/**< Tenor Network TPC processor */
    EM_SNP1K		=99,	/**< Trebia SNP 1000 processor */
    EM_IP2K		=101,	/**< Ubicom IP2022 micro controller */
    EM_CR		=103,	/**< National Semiconductor CompactRISC */
    EM_MSP430		=105,	/**< TI msp430 micro controller */
    EM_BLACKFIN		=106,	/**< ADI Blackfin */
    EM_ALTERA_NIOS2	=113,	/**< Altera Nios II soft-core processor */
    EM_CRX		=114,	/**< National Semiconductor CRX */

    EM_XGATE		=115,	/**< Motorola XGATE embedded processor */
    EM_C166		=116,	/**< Infineon C16x/XC16x processor */
    EM_M16C		=117,	/**< Renesas M16C series microprocessors */
    EM_DSPIC30F		=118,	/**< Microchip Technology dsPIC30F Digital Signal Controller */
    EM_CE		=119,	/**< Freescale Communication Engine RISC core */
    EM_M32C		=120,	/**< Renesas M32C series microprocessors */

    EM_TSK3000		=131,	/**< Altium TSK3000 core */
    EM_RS08		=132,	/**< Freescale RS08 embedded processor */

    EM_ECOG2		=134,	/**< Cyan Technology eCOG2 microprocessor */
    EM_SCORE		=135,	/**< Sunplus Score */
    EM_DSP24		=136,	/**< New Japan Radio (NJR) 24-bit DSP Processor */
    EM_VIDEOCORE3	=137,	/**< Broadcom VideoCore III processor */
    EM_LATTICEMICO32	=138,	/**< RISC processor for Lattice FPGA architecture */
    EM_SE_C17		=139,	/**< Seiko Epson C17 family */

    EM_MMDSP_PLUS	=160,	/**< STMicroelectronics 64bit VLIW Data Signal Processor */
    EM_CYPRESS_M8C	=161,	/**< Cypress M8C microprocessor */
    EM_R32C		=162,	/**< Renesas R32C series microprocessors */
    EM_TRIMEDIA		=163,	/**< NXP Semiconductors TriMedia architecture family */
    EM_QDSP6		=164,	/**< QUALCOMM DSP6 Processor */
    EM_8051		=165,	/**< Intel 8051 and variants */
    EM_STXP7X		=166,	/**< STMicroelectronics STxP7x family */
    EM_NDS32		=167,	/**< Andes Technology compact code size embedded RISC processor family */
    EM_ECOG1X		=168,	/**< Cyan Technology eCOG1X family */
    EM_MAXQ30		=169,	/**< Dallas Semiconductor MAXQ30 Core Micro-controllers */
    EM_XIMO16		=170,	/**< New Japan Radio (NJR) 16-bit DSP Processor */
    EM_MANIK		=171,	/**< M2000 Reconfigurable RISC Microprocessor */
    EM_CRAYNV2		=172,	/**< Cray Inc. NV2 vector architecture */
    EM_RX		=173,	/**< Renesas RX family */
    EM_METAG		=174,	/**< Imagination Technologies META processor architecture */
    EM_MCST_ELBRUS	=175,	/**< MCST Elbrus general purpose hardware architecture */
    EM_ECOG16		=176,	/**< Cyan Technology eCOG16 family */
    EM_CR16		=177,	/**< National Semiconductor CompactRISC 16-bit processor */

/** If it is necessary to assign new unofficial EM_* values, please pick large
   random numbers (0x8523, 0xa7f2, etc.) to minimize the chances of collision
   with official or non-GNU unofficial values.

   NOTE: Do not just increment the most recent number by one.
   Somebody else somewhere will do exactly the same thing, and you
   will have a collision.  Instead, pick a random number.  */

/** Cygnus PowerPC ELF backend.  Written in the absence of an ABI.  */
    EM_CYGNUS_POWERPC	=0x9025U,

/** Cygnus M32R ELF backend.  Written in the absence of an ABI.  */
    EM_CYGNUS_M32R	=0x9041U,

/** D10V backend magic number.  Written in the absence of an ABI.  */
    EM_CYGNUS_D10V	=0x7650U,

/** mn10200 and mn10300 backend magic numbers.
   Written in the absense of an ABI.  */
    EM_CYGNUS_MN10200	=0xdeadL,
    EM_CYGNUS_MN10300	=0xbeefL
};
/** See the above comment before you add a new EM_* value here.  */

/** List of CPU platform. */
    enum {
	DISASM_DATA	=0,  /**< indicates data disassembler */
	DISASM_CPU_IX86	=1,  /**< indicates Intel-x86 disassembler */
	DISASM_CPU_AVR	=2,  /**< indicates Atmel-AVR disassembler */
	DISASM_JAVA	=3,  /**< indicates Java disassembler */
	DISASM_CPU_ARM	=4,  /**< indicates ARM disassembler */
	DISASM_CPU_PPC	=5,  /**< indicates PowerPC disassembler */
			    /* ... here may placed other constants!!! ... */
	DISASM_CPU_IA64	=6,  /**< indicates Itanium disassembler */
	DISASM_CPU_ALPHA=7,  /**< indicates DEC Alpha disassembler */
	DISASM_CPU_MIPS	=8,  /**< indicates MIPS disassembler */
	DISASM_CPU_SPARC=9,  /**< indicates SUN Sparc disassembler */
	DISASM_CPU_SH	=10, /**< indicates Hitachi SH disassembler */
	DISASM_CPU_CRAY	=11, /**< indicates Cray disassembler */
			    /* ... here may placed other constants!!! ... */
	DISASM_DEFAULT	=0  /**< indicates unspecified disassembler: format default */
    };



/* sh_type */
#define SHT_SYMTAB      2
#define SHT_NOTE        7
#define SHT_DYNSYM      11

/* elf type */
#define ELFDATANONE     0               /* e_ident[EI_DATA] */
#define ELFDATA2LSB     1
#define ELFDATA2MSB     2

/* elf class */
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

#if 0
/* magic number */
#define EI_MAG0         0               /* e_ident[] indexes */
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_PAD          7
#define ELFMAG0         0x7f            /* EI_MAG */
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define ELFMAG          "\177ELF"
#endif // 0

#define OLFMAG1         'O'
#define OLFMAG          "\177OLF"

typedef struct {
    Elf32_Word  p_type;
    Elf32_Off   p_offset;
    Elf32_Addr  p_vaddr;
    Elf32_Addr  p_paddr;
    Elf32_Word  p_filesz;
    Elf32_Word  p_memsz;
    Elf32_Word  p_flags;
    Elf32_Word  p_align;
} Elf32_Phdr;

typedef struct {
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define PT_NULL         0               /* p_type */
#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_SHLIB        5
#define PT_PHDR         6
#define PT_NUM          7

typedef struct {
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
    Elf32_Word  sh_flags;
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;
    Elf32_Word  sh_entsize;
} Elf32_Shdr;

typedef struct {
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Off   sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Off   sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Off   sh_addralign;
    Elf64_Off   sh_entsize;
} Elf64_Shdr;

/* Notes used in ET_CORE */
#define NT_PRSTATUS     1
#define NT_PRFPREG      2
#define NT_PRPSINFO     3
#define NT_TASKSTRUCT   4

#define NT_NETBSD_CORE_PROCINFO         1

/* Note header in a PT_NOTE section */
typedef struct elf_note {
    Elf32_Word  n_namesz;       /* Name size */
    Elf32_Word  n_descsz;       /* Content size */
    Elf32_Word  n_type;         /* Content type */
} Elf32_Nhdr;

typedef struct {
    Elf64_Word  n_namesz;
    Elf64_Word  n_descsz;
    Elf64_Word  n_type;
} Elf64_Nhdr;

#define NT_PRSTATUS     1
#define NT_PRFPREG      2
#define NT_PRPSINFO     3
#define NT_PRXREG       4
#define NT_PLATFORM     5
#define NT_AUXV         6

/* Note types used in executables */
/* NetBSD executables (name = "NetBSD") */
#define NT_NETBSD_VERSION       1
#define NT_NETBSD_EMULATION     2
#define NT_FREEBSD_VERSION      1
#define NT_OPENBSD_VERSION      1
#define NT_DRAGONFLY_VERSION    1
/* GNU executables (name = "GNU") */
#define NT_GNU_VERSION          1

/* GNU OS tags */
#define GNU_OS_LINUX    0
#define GNU_OS_HURD     1
#define GNU_OS_SOLARIS  2


//#pragma pack(show)
#pragma pack() // establish defalt allignment = 8
//#pragma pack(show)


#endif

// eof - DumpElf.h
