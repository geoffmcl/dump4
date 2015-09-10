// DumpArch.h

#ifndef _DUMPARCH_H_
#define _DUMPARCH_H_
/* =====================================================================================
    20140614:
    from : http://msdn.microsoft.com/library/windows/hardware/gg463119.aspx
    DWN: 14/06/2014  17:43 217,765 pecoff_v83.docx
    14 Archive (Library) File Format
    The first 8 bytes of an archive consist of the file signature.
    The rest of the archive consists of a series of archive members, as follows:
    The first and second members are “linker members.” Each of these members has its own format as described in section 8.3, “Import Name Type.”
    the general structure of an archive.
    * Signature :”!<arch>\n”
    * Header
    * 1st Linker Member
    * Header
    * 2nd Linker Member
    * Header
    * Longnames Member
    * Header
    * Contents of OBJ File 1 (COFF format)
    * Header
    * Contents of OBJ File 2 (COFF format)
    * Header
    * Contents of OBJ File N (COFF format)
    The Header
    Off Len Field Desc
    0   16  Name  The name of the archive member, with a slash (/) appended to terminate the name. 
                    If the first character is a slash, the name has a special interpretation, as described 
                    in the following table.
    16  12  Date  The date and time that the archive member was created: This is the ASCII decimal 
                    representation of the number of seconds since 1/1/1970 UCT.
    28  6   User ID An ASCII decimal representation of the user ID. This field does not contain a meaningful 
                    value on Windows platforms because Microsoft tools emit all blanks.
    34  6   Group ID An ASCII decimal representation of the group ID. This field does not contain a meaningful 
                    value on Windows platforms because Microsoft tools emit all blanks.
    40  8   Mode  An ASCII octal representation of the member’s file mode. This is the ST_MODE value from the 
                    C run-time function _wstat.
    48  10  Size  An ASCII decimal representation of the total size of the archive member, not including the 
                    size of the header.
    58  2   End of Header The two bytes in the C string “‘\n” (0x60 0x0A).

   ===================================================================================== */

// from : http://en.wikipedia.org/wiki/Ar_(Unix)
/* -------------------------
Field Offset from Field Offset to Field Name Field Format 
0 15 File name ASCII 
16 27 File modification timestamp Decimal 
28 33 Owner ID Decimal 
34 39 Group ID Decimal 
40 47 File mode Octal 
48 57 File size in bytes Decimal 
58 59 File magic 0x60 0x0A 
   ------------------------- */
#pragma pack(push,1)    // align to a byte

typedef struct tagARCH {
    char name[16];       // 0  15
    char timestamp[12];  // 16 - 27
    char ownerID[6];     // 28 - 33
    char groupID[6];     // 34 - 39
    char mode[8];        // 40 - 47
    char size[10];       // 48 - 57
    char magic[2];       // 58 - 59 = 0x60 0x0a
} ARCH, *PARCH;

#pragma pack(pop)
#ifdef __cplusplus
extern "C" {
#endif

extern int is_archive_format(unsigned char *base, int size, char * name);

#ifdef __cplusplus
}
#endif


#endif // #ifndef _DUMPARCH_H_
// eof - DumpArch.h
