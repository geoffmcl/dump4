// DumpM2TS.c

#include "Dump4.h"
#include <assert.h>

/* ---------------------------------------

from : http://en.wikipedia.org/wiki/MPEG-2_transport_stream

Desc                            bits Info
sync byte                       8    0x47 
Transport Error Indicator (TEI) 1    Set by demodulator if can't correct errors in the stream[2] 
Payload Unit Start Indicator    1    1 means start of PES data or PSI otherwise zero only. 
Transport Priority              1    One means higher priority than other packets with the same PID. 
PID                             13   Packet ID 
Scrambling control              2    '00' = Not scrambled. 
    The following per DVB spec:[3]   '01' = Reserved for future use,
                                     '10' = Scrambled with even key,
                                     '11' = Scrambled with odd key 
Adaptation field exist          1    1 means presence of the adaptation field 
Payload data exist              1    1 means presence of data 
Continuity counter              4  
  Note: the total number of bits above is 32 and is called the 
  transport stream 4-byte prefix. 
  ==============================================

Adaptation Field Format 
Name                    bits Description 
Adaptation Field Length 8    Number of bytes in the adaptation field immediately following this byte 
Discontinuity indicator 1    Set to 1 if a discontinuity occurred in the continuity counter of the TS packet 
Random Access indicator 1    Set to 1 if the PES packet in this TS packet 
                             starts a video/audio sequence 
Elementary stream
 priority indicator     1    1 = higher priority 
PCR flag                1    1 means adaptation field does contain a PCR field 
OPCR flag               1  
Splicing point flag     1    1 means presence of splice countdown field in adaptation field 
Transport private 
       data flag        1    1 means presence of private data bytes in adaptation field 
Adaptation field 
         extension flag 1    1 means presence of adaptation field extension 
 Above is 8 bits, in the 2nd BYTE of the adaptation field

Below fields are optional variable Depends on flags 
PCR                     33+9 Program clock reference 
OPCR                    33+9 Original Program clock reference. Helps when one TS is copied into another 
Splice countdown        8    Indicates how many TS packets from this one a splicing point occurs (may be negative) 
stuffing bytes          variable 

   --------------------------------------- */

#pragma pack(1) // force byte alignment
typedef struct tagM2TS_PKT {
    BYTE sync;  // MPEG2T_SYNC_BYTE = 0x47
    BYTE tprpsei;   // this NOT correct - these are BITS!!!
    BYTE PID;
    BYTE scrafcc;
    BYTE pkt[184];
}M2TS_PKT, * PM2TS_PKT;

typedef struct tagM2TS_TP {
    DWORD header;
    M2TS_PKT pkt;
}M2TS_TP, * PM2TS_TP;

#pragma pack()

static int adaption_control;
static M2TS_TP  _s_block;
static char _s_offset[32];

#define MPEG2T_SYNC_BYTE   0x47

static DWORD aligned_units = 0;

#define CHECK_MP2T_HEADER assert(*pHdr == MPEG2T_SYNC_BYTE)

// from mpeg2_transport.c
// ======================
// Transport Error Indicator (TEI) 1    
// Set by demodulator if can't correct errors in the stream[2] 
int mpeg2t_transport_error_inidicator( BYTE * pHdr )
{
    CHECK_MP2T_HEADER;
    return ((pHdr[1] >> 7) & 0x1);
}

// Payload Unit Start Indicator    1    
// 1 means start of PES data or PSI otherwise zero only. 
int mpeg2t_payload_unit_start_indicator( BYTE * pHdr )
{
    CHECK_MP2T_HEADER;
    return ((pHdr[1] >> 6) & 0x1);
}


int mpeg2t_pid( BYTE * pHdr )
{
    int pid;
    CHECK_MP2T_HEADER;
    pid = (pHdr[1] & 0x1f) << 8;
    pid |= pHdr[2];
    return pid;
}

int mpeg2t_adaptation_control( BYTE * pHdr )
{
    CHECK_MP2T_HEADER;
    return ((pHdr[3] >> 4) & 0x3);
}

int mpeg2t_continuity_counter( BYTE * pHdr )
{
    CHECK_MP2T_HEADER;
    return (pHdr[3] & 0xf);
}

BYTE * mpeg2t_transport_payload_start( BYTE * pHdr, DWORD * plen )
{
#ifdef ADD_IX_OUTPUT
    int ix;
#endif // #ifdef ADD_IX_OUTPUT
    int pid;
    long long pcr;
    int ext;
    CHECK_MP2T_HEADER;
    if ( mpeg2t_transport_error_inidicator( pHdr ) != 0 ) {
        *plen = 0;
        return NULL;
    }

    adaption_control = mpeg2t_adaptation_control( pHdr );

    if ( adaption_control == 1 ) {
        *plen = 184;
        return pHdr + 4;
    }
    if ( adaption_control == 3 ) {
        if ( pHdr[4] > 183 ) {
            *plen = 0;
            return NULL;
        }
        *plen = 183 - pHdr[4];
        pid = ((pHdr[1] << 8) | pHdr[2]) & 0x1fff;
        if( (pHdr[5] & 0x10) != 0 ) {
            pcr = pHdr[6];
            pcr = (pcr << 8) | pHdr[7];
            pcr = (pcr << 8) | pHdr[8];
            pcr = (pcr << 8) | pHdr[9];
            pcr <<= 1;
            if ( (pHdr[10] & 0x80) != 0 ) {
                pcr |= 1;
            }
            ext = (pHdr[10] & 0x1) << 8;
            ext |= pHdr[11];
            //sprtf( "pid %x pcr %I64d ext %u"MEOR, pid, pcr, ext );
            sprtf( " pcr=%I64u ext=%u ", pcr, ext );
        }
#ifdef ADD_IX_OUTPUT
        for (ix = 0; ix < pHdr[4]; ix += 4) {
            sprtf( "pid %x %d - %02x %02x %02x %02x "MEOR,
                pid, ix,
                pHdr[4 + ix],
                pHdr[5 + ix],
                pHdr[6 + ix],
                pHdr[7 + ix]);
        }
#endif // #ifdef ADD_IX_OUTPUT
        return pHdr + 5 + pHdr[4];
    }
    *plen = 0;
    return NULL;
}

#define ADDFLAG(a,b) if(*a)strcat(a,"|"); strcat(a,b); flags++;

static char _s_afb_buf[32];

int adaptation_field_bits( char * buf, BYTE b )
{
    int flags = 0;
    // Discontinuity indicator 1
    //  Set to 1 if a discontinuity occurred in the continuity counter of the TS packet 
    if ( b & 0x80 )
        ADDFLAG(buf,"di");
    // Random Access indicator 1
    //    Set to 1 if the PES packet in this TS packet 
    //                         starts a video/audio sequence 
    if ( b & 0x40 )
        ADDFLAG(buf,"RA");
    // Elementary stream
    //  priority indicator     1    1 = higher priority 
    if ( b & 0x20 )
        ADDFLAG(buf,"pi");
    // PCR flag                1    
    // 1 means adaptation field does contain a PCR field 
    if ( b & 0x10 )
        ADDFLAG(buf,"PCR");
    // OPCR flag               1  
    if ( b & 0x08 )
        ADDFLAG(buf, "OPCR");
    // Splicing point flag     1    
    //1 means presence of splice countdown field in adaptation field 
    if ( b & 0x04 )
        ADDFLAG(buf,"sp");
    // Transport private 
    //   data flag        1    1 means presence of private data bytes in adaptation field 
    if ( b & 0x02 )
        ADDFLAG(buf,"priv");
    // Adaptation field 
    //     extension flag 1    1 means presence of adaptation field extension 
    if ( b & 0x01 )
        ADDFLAG(buf,"ext");

    return flags;
}

char * get_field_bit_stg( BYTE b )
{
    char * cp = _s_afb_buf;
    *cp = 0;
    adaptation_field_bits( cp, b );
    return cp;
}

BOOL Error_M2TS( PTSTR pmsg )
{
    prt(pmsg);
    return FALSE;
}

int is_pads_to_end( PM2TS_TP ptp, DWORD max )
{
    int blkcnt = 0;
    int i;
    BYTE * pb;
    while(max) {
        PM2TS_PKT ppkt = &ptp->pkt;
        if(max >= sizeof(M2TS_TP))
            max -= sizeof(M2TS_TP);
        else {
            return 0;
        }
        pb = (PBYTE)&ppkt->pkt[0];
        for(i = 0; i < 184; i++) {
            if( pb[i] != 0xff )
                return 0;
        }
        ptp++;
        blkcnt++;
    }
    return blkcnt;
}

typedef struct tagPIDVAL {
    int pid;
    int level;
    int cnt;
}PIDVAL, * PPIDVAL;

typedef struct tagPIDLIST {
    int count;
    int max;
    int level;
    PPIDVAL ppids;
}PIDLIST, * PPIDLIST;

PIDLIST pidlist = { 0, 0, 0, NULL };


void show_pids( void )
{
    int i;
    int lev0 = 0;
    int lev1 = 0;
    int cnt,wrap;
    PPIDLIST ppl = &pidlist;
    if ( ppl->ppids == NULL )
        return;
    for( i = 0; i < ppl->count; i++ ) {
        if ( ppl->ppids[i].level == 0 )
            lev0++;
        else
            lev1++;
    }
    wrap = 5;
    cnt  = 0;
    sprtf( "Collected %d PIDs, %d level 0, %d level 1..."MEOR,
        ppl->count, lev0, lev1 );
    if (lev0) {
        sprtf( "Level 0: " );
        for( i = 0; i < ppl->count; i++ ) {
            if ( ppl->ppids[i].level == 0 ) {
                cnt++;
                if( cnt == wrap ) {
                    cnt = 0;
                    sprtf( MEOR"         ");
                }
                sprtf( "%d(%d) ", ppl->ppids[i].pid, ppl->ppids[i].cnt );
            }
        }
        sprtf(MEOR);
    }
    if (lev1) {
        sprtf( "Level 1: " );
        cnt = 0;
        for( i = 0; i < ppl->count; i++ ) {
            if ( ppl->ppids[i].level != 0 ) {
                cnt++;
                if( cnt == wrap ) {
                    cnt = 0;
                    sprtf( MEOR"         ");
                }
                sprtf( "%d(%d) ", ppl->ppids[i].pid, ppl->ppids[i].cnt );
            }
        }
        sprtf(MEOR);
    }
}

void add_to_pid_list( int pid, int level )
{
    int i;
    PPIDLIST ppl = &pidlist;
    if ( ppl->ppids == NULL ) {
        ppl->max = 16;
        ppl->ppids = (PPIDVAL)malloc(sizeof(PIDVAL) * ppl->max);
    }
    if ( ppl->ppids == NULL )
        return;
    for ( i = 0; i < ppl->count; i++ ) {
        if (( ppl->ppids[i].pid == pid ) &&
            ( ppl->ppids[i].level == level )) {
            ppl->ppids[i].cnt++;    // bump the COUNT of this PID
            return;
        }
    }
    if (( ppl->count + 1 ) >= ppl->max ) {
        ppl->max *= 2;
        ppl->ppids = (PPIDVAL)realloc(ppl->ppids, sizeof(PIDVAL) * ppl->max);
    }
    if ( ppl->ppids == NULL )
        return;
    ppl->ppids[ppl->count].pid   = pid;
    ppl->ppids[ppl->count].level = level;
    ppl->ppids[ppl->count].cnt   = 1;
    ppl->count++;
}

BOOL ProcessM2TS( LPDFSTR lpdf )
{
    PBYTE pb = lpdf->lpb;
    PM2TS_TP ptp = (PM2TS_TP)pb;
    DWORD max = lpdf->dwmax;
    DWORD cnt = 0;
    DWORD block = 0;
    PTSTR ptmp = g_cTmp;
    DWORD   i, len, skipped, off, i2;
    DWORD   pads = 0;
    BYTE * ppb;
    char * poffset = _s_offset;
    int pid, cc, usi;
    BYTE * psblk = (BYTE *)&_s_block;
    char * pafbstg;
    BYTE * tmp;
    int TEI, PUSI, TP, SC, AFE, PLDE, CC;
    while(max) {
        PM2TS_PKT ppkt = &ptp->pkt;
        ppb = (BYTE *)ppkt;

        if( ppkt->sync != MPEG2T_SYNC_BYTE ) // = 0x47
            return Error_M2TS("ERROR: Missing SYNC byte (0x47)!"MEOR );

        if(max >= sizeof(M2TS_TP))
            max -= sizeof(M2TS_TP);
        else {
            return Error_M2TS("ERROR: NOT correct source packet sizing (192)!"MEOR );
        }


        pb = (PBYTE)ptp;
        // if given the BEGIN address, show a relative offset
        off = (DWORD)(pb - lpdf->lpb);
        sprintf( poffset, "%4.4X:%4.4X", ((off & 0xffff0000) >> 16), (off & 0x0000ffff) );

        if( cnt == 0 ) {
            block++;
            sprtf("Block %d:"MEOR, block );
        }

        memcpy( psblk, ptp, sizeof(M2TS_TP) );  // copy the BLOCK
        cnt++;
        sprtf( "%s: %4d:%2d: %02X %02X %02X %02X ", poffset, block, cnt,
            pb[0] & 0xFF, pb[1] &0xFF, pb[2] & 0xFF, pb[3] & 0xFF );

        for( i = 0; i < 4; i++ )
            psblk[i] = 0xff;    // fill in DONE
        off = 4;    // done first 4 bytes

        len = 0;
        cc = mpeg2t_continuity_counter(ppb);
        pid = mpeg2t_pid(ppb);
        usi = mpeg2t_payload_unit_start_indicator(ppb);
        sprtf("P%d cc=%d", pid, cc);
        add_to_pid_list( pid, 0 );
        pb = mpeg2t_transport_payload_start( ppb, &len );
        if( pb ) {
            pafbstg = get_field_bit_stg( ppb[0] );
            skipped = (pb - ppb);
            if( skipped ) {
                sprtf( "= " );
                for( i = 0; i < skipped; i++ ) {
                    sprtf( "%02x ", ppb[i] );
                    psblk[off + i] = 0xff;  // done this byte
                    if(( ppb[i] == 0xFF )&&( ( i + 1 ) < skipped )) {
                        for( i2 = i + 1; i2 < skipped; i2++ ) {
                            if( ppb[i2] != 0xFF ) {
                                break;
                            }
                        }
                        if( i2 == skipped ) {
                            sprtf( "+ %d more...", skipped - (i + 1));
                            break;
                        }
                    }
                }
            }
            off += skipped; // increase DONE
            sprtf("(%d) ", skipped);
            // check BACKWARD for 0xff indicators
            i = len;
            skipped = 0;
            while(i--) {
                if( pb[i] != 0xFF ) {
                    i++;
                    break;
                }
                skipped++;
            }
            if(skipped)
                skipped--;
            sprtf( " hex %d %s"MEOR, (len - skipped),
                (usi ? "(USI)" : "") );
            if ( VERB2 ) {
                *ptmp = 0;
                GetHEXString( ptmp, pb, len - skipped, lpdf->lpb, TRUE );
                sprtf("%s"MEOR, ptmp);
            }
            for( i = 0; i < len; i++ )
                psblk[off + i] = 0xff;  // done this byte
            off += len;
        } else {
/* -----------------
from : http://en.wikipedia.org/wiki/MPEG-2_transport_stream

Desc                            bits Info
sync byte                       8    0x47   ; Byte 1
Transport Error Indicator (TEI) 1    Set by demodulator if can't correct errors in the stream[2] 
Payload Unit Start Indicator    1    1 means start of PES data or PSI otherwise zero only. 
Transport Priority              1    One means higher priority than other packets with the same PID. 
PID                             13   Packet ID ; part byte 2 | byte 3
Scrambling control              2    '00' = Not scrambled. ; byte 4 - upper nibble
    The following per DVB spec:[3]   '01' = Reserved for future use,
                                     '10' = Scrambled with even key,
                                     '11' = Scrambled with odd key 
Adaptation field exist          1    1 means presence of the adaptation field 
Payload data exist              1    1 means presence of data 
Continuity counter              4       ; byte 4 lower nibble
  Note: the total number of bits above is 32 and is called the 
  transport stream 4-byte prefix. 
   ----------------- */
            // from Handbrake
            tmp = (BYTE *)ppkt;
            TEI  = (tmp[1] & 0x80) ? 1 : 0;     // 1 bit
            PUSI = (tmp[1] & 0x40) ? 1 : 0;     // 1 bit
            TP   = (tmp[1] & 0x20) ? 1 : 0;     // 1 bit
            pid  = (((tmp[1] & 0x1f) << 8) | tmp[2]) & 0x1fff;
            SC   = (tmp[3] & (0x80 | 0x40)) >> 6;    // 2-bits
            AFE  = (tmp[3] & 0x20) ? 1 : 0;     // 1 bit
            PLDE = (tmp[3] & 0x10) ? 1 : 0;     // 1 bit
            CC   = (tmp[3] & 0x0F);     // 4 bits
            sprtf( " S47 %02X %02X %02X"MEOR, ppkt->tprpsei & 0xff, ppkt->PID & 0xff, ppkt->scrafcc & 0xff );
            sprtf( "Bits fields: TEI=%d, PUSI=%d, TP=%d PID=%d, SC=%d, PLDE=%d, CC=%d",
                TEI, PUSI, TP, pid, SC, AFE, PLDE, CC );
            if (pid == 0) {
                sprtf( " Note: ZERO PID found!" );
            }
            sprtf(MEOR);
            add_to_pid_list( pid, 1 );
            for( i = 0; i < 4; i++ )
                psblk[off + i] = 0xff;  // done this byte
            off += 4;
        }

        *ptmp = 0;
        pb = (PBYTE)&ppkt->pkt[0];
        for(i = 0; i < 184; i++) {
            if( pb[i] != 0xff )
                break;
        }
        if( i == 184 ) {
            i = is_pads_to_end( ptp, max );
            if(i) {
                sprtf( "Padding only to END - all 0xFF for %d bytes."MEOR,
                    ( i * sizeof(M2TS_TP) ) );
                pads += i;
                break;
            } else {
                if ( VERB9 )
                    sprtf( "padding only - all 0xFF for 184 bytes."MEOR );
                pads++;
            }
        } else {
            // found some NOT 0xFF...
            if(( len < 184 ) && ( off < sizeof(M2TS_TP) )) {
                skipped = 0;
                i = 184;
                skipped = 0;
                while(i--) {
                    if( pb[i] != 0xFF ) {
                        i++;
                        break;
                    }
                    skipped++;
                }
                if(skipped)
                    skipped--;
                // check if any output needed
                if(skipped) {
                    sprtf( "%s: %4d:%2d: Hex of block %d (184-%d of 0xff)"MEOR,
                        poffset, block, cnt, 184 - skipped, skipped);
                } else {
                    sprtf( "%s: %4d:%2d: Hex of block %d"MEOR,
                        poffset, block, cnt, 184);
                }
                if ( VERB2 ) {
                    *ptmp = 0;
                    GetHEXString( ptmp, pb, 184 - skipped, lpdf->lpb, TRUE );
                    sprtf("%s"MEOR, ptmp);
                }
            }
        }


        ptp++;
        if(cnt == 32) {
            cnt = 0;
        }
    }
    if(pads) {
        max = (pads * sizeof(M2TS_TP));
        sprtf( "Total %d pad blocks (%d bytes on %d - %f%%) in file."MEOR,
            pads,
            max,
            lpdf->dwmax,
            get_percent2(lpdf->dwmax, max) );
    }
    sprtf("Processed a total of %d 'aligned units' (6144 bytes each = %d)"MEOR,
        aligned_units,
        (aligned_units * 6144) );
    show_pids();
    return TRUE;
}

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

// from handbrake, stream.c
static int check_ts_sync(const uint8_t *buf)
{
    // must have initial sync byte, no scrambling & a legal adaptation ctrl
    return (buf[0] == 0x47) && ((buf[3] >> 6) == 0) && ((buf[3] >> 4) > 0);
}

static int have_ts_sync(const uint8_t *buf, int psize)
{
    return check_ts_sync(&buf[0*psize]) && check_ts_sync(&buf[1*psize]) &&
           check_ts_sync(&buf[2*psize]) && check_ts_sync(&buf[3*psize]) &&
           check_ts_sync(&buf[4*psize]) && check_ts_sync(&buf[5*psize]) &&
           check_ts_sync(&buf[6*psize]) && check_ts_sync(&buf[7*psize]);
}

static int hb_stream_check_for_ts(const uint8_t *buf)
{
    // transport streams should have a sync byte every 188 bytes.
    // search the first 8KB of buf looking for at least 8 consecutive
    // correctly located sync patterns.
    int offset = 0;

    for ( offset = 0; offset < 8*1024-8*188; ++offset )
    {
        if ( have_ts_sync( &buf[offset], 188) )
            return 188 | (offset << 8);
        if ( have_ts_sync( &buf[offset], 192) )
            return 192 | (offset << 8);
        if ( have_ts_sync( &buf[offset], 204) )
            return 204 | (offset << 8);
        if ( have_ts_sync( &buf[offset], 208) )
            return 208 | (offset << 8);
    }
    return 0;
}


BOOL LooksLikeM2TS( LPDFSTR lpdf )
{
    PBYTE    pb = lpdf->lpb;
    PM2TS_TP ptp = (PM2TS_TP)pb;
    DWORD max = lpdf->dwmax;
    DWORD cnt = 0;
    int res, offset;
    if ( max > 8*1024 ) {
        res = hb_stream_check_for_ts((const uint8_t *)pb);
        if (res) {
            offset = (res & 0xffffff00) >> 8;
            res = res & 0xFF;
            sprtf( "Found first sync byte, 0x47, at offset %u, packet size %d\n",
                offset, res );
        }
    }
    while(max)
    {
        PM2TS_PKT ppkt = &ptp->pkt;
        if( ppkt->sync != MPEG2T_SYNC_BYTE ) // = 0x47
            return Error_M2TS("ERROR: Missing SYNC byte (0x47)!"MEOR );

        if(max >= sizeof(M2TS_TP))
            max -= sizeof(M2TS_TP);
        else {
            return Error_M2TS("ERROR: NOT correct source packet sizing (192)!"MEOR );
        }
        ptp++;
        cnt++;
        if(cnt == 32) {
            aligned_units++;
            cnt = 0;
        }
    }
    if(cnt)
        return Error_M2TS("ERROR: NOT correct aligned unit size (32)!"MEOR );


    return TRUE;    // it LOOKS like a M2TS file...
}


BOOL  DumpM2TS( LPDFSTR lpdf )
{
    BOOL bRet = FALSE;

    if( !lpdf->dwmax )
        return bRet;

    assert( sizeof(M2TS_TP) == 192 );

    if( LooksLikeM2TS( lpdf ) ) {
        bRet = ProcessM2TS(lpdf);
    }

    return bRet;
}

// eof - DumpM2TS.c
