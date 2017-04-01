
// DumpSonic.c
#include	"Dump4.h"   // 20170330 - Fix cse for unix
#include "DumpSonic.h"

/* -----------------------
File [HOMMAGE-02.sonic], 968 bytes.
0000:0000 00 BB 00 BB 01 00 00 00  34 5C AC 0B 45 DF 0F 4C ........4\..E..L
0000:0010 8D 64 8E 92 DC CF 00 7D  DB B6 6D DB 01 00 00 00 .d.....}..m.....
0000:0020 1B 00 00 00
0000:0020             12 00 00 00  43 3A 5C 48 4F 4D 45 50 ........C:\HOMEP
0000:0030 41 47 45 5C 61 6C 79 6E  63 61 08 00 00 00 5C 61 AGE\alynca....\a
0000:0040 6C 79 6E 63 61 5C 11 00  00 00 43 3A 5C 48 4F 4D lynca\....C:\HOM
0000:0050 45 50 41 47 45 5C 41 74  6C 61 73 07 00 00 00 5C EPAGE\Atlas....\
0000:0060 41 74 6C 61 73 5C 11 00  00 00 43 3A 5C 48 4F 4D Atlas\....C:\HOM
0000:0070 45 50 41 47 45 5C 65 6D  61 69 6C 07 00 00 00 5C EPAGE\email....\
...
0000:0380 08 00 00 00 5C 73 69 6D  70 6C 65 5C 10 00 00 00 ....\simple\....
0000:0390 43 3A 5C 48 4F 4D 45 50  41 47 45 5C 74 65 73 74 C:\HOMEPAGE\test
0000:03a0 06 00 00 00 5C 74 65 73  74 5C 10 00 00 00 43 3A ....\test\....C:
0000:03b0 5C 48 4F 4D 45 50 41 47  45 5C 79 6F 67 61 06 00 \HOMEPAGE\yoga..
0000:03c8 00 00 5C 79 6F 67 61 5C                          ..\yoga\
   ----------------------- */

static DWORD _s_max_len1, _s_max_len2;

typedef struct tagFILE_HEADER {
    DWORD   sig;
    DWORD   cnt1;
    BYTE    unknown[20];
    DWORD   cnt2;
    DWORD   entries;
}SFILE_HEADER, * PSFILE_HEADER;

typedef struct tagSFILE_ITEM {
    DWORD   len;
    // char for this length
}SFILE_ITEM, * PSFILE_ITEM;

VOID show_sonic_info(PTSTR pinfo1, PTSTR pinfo2)
{
#ifdef WIN32
    WIN32_FIND_DATA	fd;
    HANDLE	hFind;

    hFind = FindFirstFile(pinfo1, &fd);
#else
    struct stat buf;
    int res = stat(pinfo1, &buf);
#endif

    while (strlen(pinfo1) < _s_max_len1)
        strcat(pinfo1, " ");
    if (*pinfo2) {
        while (strlen(pinfo2) < _s_max_len2)
            strcat(pinfo2, " ");
    }

#ifdef WIN32
    if (VFH(hFind)) {
        FindClose(hFind);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            strcat(pinfo2, " [DIR]                ");
        }
        else {
            ULARGE_INTEGER ul;
            PTSTR ptmp = GetNxtBuf();

            ul.LowPart = fd.nFileSizeLow;
            ul.HighPart = fd.nFileSizeHigh;
            sprintf(ptmp, "%I64d", ul);
            sprintf(EndBuf(pinfo2), " [FIL] % 15s", My_NiceNumber(ptmp));
        }
        // fd.ftCreationTime, fd.ftLastAccessTime, fd.ftLastWriteTime
        strcat(pinfo2, " ");
        Get_FD_File_Time_Stg(pinfo2, &fd, TRUE);

    }
    else {
        strcat(pinfo2, " NOT FOUND!");
    }
#else
    if (res)
    {
        strcat(pinfo2, " NOT FOUND!");
    }
    else
    {
        if ((buf.st_mode & S_IFMT) == S_IFDIR)
        {
            strcat(pinfo2, " [DIR]                ");
        }
        else
        {
            PTSTR ptmp = GetNxtBuf();
            sprintf(ptmp, "%ll", buf.st_size);
            sprintf(EndBuf(pinfo2), " [FIL] % 15s", My_NiceNumber(ptmp));
        }
        strcat(pinfo2, " TODO: Add time");
    }
#endif
    sprtf("%s %s" MEOR, pinfo1, pinfo2 );
}

BOOL  DumpSONIC( LPDFSTR lpdf )
{
    PTSTR pfn = lpdf->fn;
    PBYTE pb = (PBYTE)lpdf->lpb;
    DWORD dwmax = lpdf->dwmax;
    PSFILE_HEADER ph = (PSFILE_HEADER)pb;
    PSFILE_ITEM pitm = (PSFILE_ITEM) (pb + sizeof(SFILE_HEADER));
    DWORD   sig, entries, dwi, dwj, len, off;
    PBYTE   ptmp, ptmp2, pend;
    PTSTR   pinfo1 = GetNxtBuf();
    PTSTR   pinfo2 = GetNxtBuf();
    DWORD   off1, off2;

    off = 0;
    pend = (pb + dwmax);
    ptmp2 = (PBYTE) (ph + 1);   // check after HEADER
    if (ptmp2 > pend) {
        sprtf("ERROR: At beginning, and insufficient size %u for even HEADER block (%u)!"MEOR,
            dwmax, sizeof(SFILE_HEADER) );
        return FALSE;
    }
    ptmp2 = (PBYTE) (pitm + 1); // check first FILE block
    off = ptmp2 - pb;
    if (ptmp2 > pend) {
        sprtf("ERROR: At offset %u with insufficient size %u for first block!"MEOR, off, dwmax );
        return FALSE;
    }
    
    sig = ph->sig;
    entries = ph->entries;
    sprtf("File has %u entries..."MEOR, entries );
    strcpy(pinfo1,"SOURCE");
    while (strlen(pinfo1) < _s_max_len1) strcat(pinfo1," ");
    strcat(pinfo1,"  TYPE  SIZE (if file)   DATE     TIME");
    sprtf("%s"MEOR, pinfo1);
    for (dwi = 0; dwi < entries; dwi++) {
        // should be two for each - source and destination
        off = (PBYTE)pitm - pb;
        len = pitm->len;
        if (len == 0) {
            sprtf("ERROR: Length at offset %u in file is ZERO!"MEOR, off );
            return FALSE;
        }
        ptmp = (PBYTE) (pitm + 1);
        off = ptmp - pb;
        ptmp2 = ptmp + len;
        if (ptmp2 > pend) {
            sprtf("ERROR: At offset %u gives length %u, beyond end of file!"MEOR, off, len );
            return FALSE;
        }
        off1 = 0;
        for (dwj = 0; dwj < len; dwj++) {
            if ( !ISSIGCHAR(ptmp[dwj]) ) {
                sprtf("ERROR: At offset %u gives non significant char %02x!"MEOR, off, ptmp[dwj] );
                return FALSE;
            }
            pinfo1[off1++] = ptmp[dwj];
        }
        ptmp += len;
        pitm = (PSFILE_ITEM)ptmp;
        off = (PBYTE)pitm - pb;
        len = pitm->len;
        if (len == 0) {
            sprtf("ERROR: Length at offset %u in file is ZERO!"MEOR, off );
            return FALSE;
        }
        ptmp = (PBYTE) (pitm + 1);
        off = ptmp - pb;
        ptmp2 = ptmp + len;
        if (ptmp2 > pend) {
            sprtf("ERROR: At offset %u gives length %u, beyond end of file!"MEOR, off, len );
            return FALSE;
        }
        off2 = 0;
        for (dwj = 0; dwj < len; dwj++) {
            if ( !ISSIGCHAR(ptmp[dwj]) ) {
                sprtf("ERROR: At offset %u gives non significant char %02x!"MEOR, off, ptmp[dwj] );
                return FALSE;
            }
            pinfo2[off2++] = ptmp[dwj];
        }
        pinfo1[off1] = 0;
        pinfo2[off2] = 0;
        if ( !VERB9 )
            *pinfo2 = 0;    // kill the DESTINATION
        show_sonic_info(pinfo1,pinfo2);
        pinfo1 = GetNxtBuf();
        pinfo2 = GetNxtBuf();

        ptmp += len;
        pitm = (PSFILE_ITEM)ptmp;
        off = ptmp - pb;
        ptmp2 = (PBYTE) (pitm + 1);
        if (((dwi+1) < entries) && (ptmp2 > pend)) {
            sprtf("ERROR: At offset %u is beyond end of file!"MEOR, off );
            return FALSE;
        }
    }
    return TRUE;
}

BOOL IsSONICFile( LPDFSTR lpdf, PTSTR pfn, PBYTE pb, DWORD dwmax )
{
    PSFILE_HEADER ph = (PSFILE_HEADER)pb;
    PSFILE_ITEM pitm = (PSFILE_ITEM) (pb + sizeof(SFILE_HEADER));
    DWORD   sig, entries, dwi, dwj, len, off;
    PBYTE   ptmp, ptmp2, pend;

    off = 0;
    pend = (pb + dwmax);
    ptmp2 = (PBYTE) (ph + 1);   // check after HEADER
    if (ptmp2 > pend) {
        sprtf("ERROR: At beginning, and insufficient size %u for even HEADER block (%u)!"MEOR,
            dwmax, sizeof(SFILE_HEADER) );
        return FALSE;
    }
    ptmp2 = (PBYTE) (pitm + 1); // check first FILE block
    off = ptmp2 - pb;
    if (ptmp2 > pend) {
        sprtf("ERROR: At offset %u with insufficient size %u for first block!"MEOR, off, dwmax );
        return FALSE;
    }
    
    sig = ph->sig;
    entries = ph->entries;
    if (sig == 0xbb00bb00)
        sprtf( "Signature (0xbb00bb00) looks good, with %u entries..."MEOR, entries);
    else
        sprtf( "WARNING: Signature %08x NOT 0xbb00bb00, with %u entries..."MEOR, sig, entries);

    for (dwi = 0; dwi < entries; dwi++) {
        // should be two for each - source and destination
        off = (PBYTE)pitm - pb;
        len = pitm->len;
        if (len == 0) {
            sprtf("ERROR: Length at offset %u in file is ZERO!"MEOR, off );
            return FALSE;
        }
        ptmp = (PBYTE) (pitm + 1);
        off = ptmp - pb;
        ptmp2 = ptmp + len;
        if (ptmp2 > pend) {
            sprtf("ERROR: At offset %u gives length %u, beyond end of file!"MEOR, off, len );
            return FALSE;
        }
        for (dwj = 0; dwj < len; dwj++) {
            if ( !ISSIGCHAR(ptmp[dwj]) ) {
                sprtf("ERROR: At offset %u gives non significant char %02x!"MEOR, off, ptmp[dwj] );
                return FALSE;
            }
        }
        if (len > _s_max_len1)
            _s_max_len1 = len;
        ptmp += len;
        pitm = (PSFILE_ITEM)ptmp;
        off = (PBYTE)pitm - pb;
        len = pitm->len;
        if (len == 0) {
            sprtf("ERROR: Length at offset %u in file is ZERO!"MEOR, off );
            return FALSE;
        }
        ptmp = (PBYTE) (pitm + 1);
        off = ptmp - pb;
        ptmp2 = ptmp + len;
        if (ptmp2 > pend) {
            sprtf("ERROR: At offset %u gives length %u, beyond end of file!"MEOR, off, len );
            return FALSE;
        }
        for (dwj = 0; dwj < len; dwj++) {
            if ( !ISSIGCHAR(ptmp[dwj]) ) {
                sprtf("ERROR: At offset %u gives non significant char %02x!"MEOR, off, ptmp[dwj] );
                return FALSE;
            }
        }
        if (len > _s_max_len2)
            _s_max_len2 = len;
        ptmp += len;
        pitm = (PSFILE_ITEM)ptmp;
        off = ptmp - pb;
        ptmp2 = (PBYTE) (pitm + 1);
        if (((dwi+1) < entries) && (ptmp2 > pend)) {
            sprtf("ERROR: At offset %u is beyond end of file!"MEOR, off );
            return FALSE;
        }
    }
    return TRUE;
}

BOOL  ProcessSONIC( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   if( IsSONICFile( lpdf, lpdf->fn, lpdf->lpb, lpdf->dwmax ) ) {
      bRet = DumpSONIC( lpdf );
   } else {
      sprtf("WARNING: This does NOT appear to be a SONIC DVD project file!"MEOR);
      sprtf("         Will proceed with normal HEX dump of the file..."MEOR);
   }
   return bRet;
}

// eof - DumpSonic.c
