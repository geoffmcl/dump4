// beye_defs.h
#ifndef _BEYE_DEFS_H_
#define _BEYE_DEFS_H_

#include <string>

namespace	usr {

typedef int64_t __fileoff_t;
typedef uint64_t __filesize_t;
typedef void any_t;

#define __PURE_FUNC__
#define __CONST_FUNC__
#define UNUSED(a) a
enum beye_aka_binary_eye_project_data_type_qualifier__byte_t{ type_byte=0 };
enum beye_aka_binary_eye_project_data_type_qualifier__word_t{ type_word=0 };
enum beye_aka_binary_eye_project_data_type_qualifier_dword_t{ type_dword=0 };
enum beye_aka_binary_eye_project_data_type_qualifier_qword_t{ type_qword=0 };

    struct Symbol_Info {
	    Symbol_Info();
	    /** Public symbols classes */
	    enum symbol_class {
	        Local=0, /**< means: present as entry but not exported */
	        Global=1  /**< means: exported entry point */
	    };
	    std::string	name;	// name of public symbol
	    symbol_class	_class;	// class of symbol
	    __filesize_t	pa;	// physical address of public symbol (Bad_Address if no symbol)
    };

    struct Object_Info {
	    /** object classes */
	    enum obj_class {
	        Code=0, /**< for code objects */
	        Data=1, /**< for any data objects */
	        NoObject=-1 /**< for non objects (means: relocs, resources, tables ...) */
	    };
	    unsigned	number;	// logical number of object (0 if it's no object).
	    std::string	name;	// object name
	    __filesize_t	start;	// file offset of object's start
	    __filesize_t	end;	// file offset of object's end
	    obj_class	_class;	// _class of object
	    //Bin_Format::bitness	bitness;// bitness of object.
    };

    struct symbolic_information {
	__filesize_t pa;
	__filesize_t nameoff;
	__filesize_t addinfo;
	__filesize_t attr;

	//bool operator<(const symbolic_information& rhs) const { return pa < rhs.pa; }
    };

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



} // namespace	usr {

#endif // _BEYE_DEFS_H_
// eof
