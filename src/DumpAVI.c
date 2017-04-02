// DumpAVI.c

#ifdef WIN32
//////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4995) // 'strcat': name was marked as #pragma deprecated
#include <Winsock2.h>
#include <Dshow.h>
#ifndef FOURCC
typedef DWORD FOURCC;
#endif
#include <Aviriff.h>
#include <Mmreg.h>
//#include <KsMedia.h>
#include "Dump4.h"

#ifdef ADD_AVI_FILE
#ifndef uint
typedef unsigned int uint;
#endif

#define MY_MIN_FOURCC   3
#define DEBUG_LOOKS_LIKE    0   // Extra DEBUG in LooksLike function

PBYTE   g_pend = NULL;

// forward reference
int is_known_fourcc(char * tmp);
int is_valid_chunk( PBYTE pb, PBYTE pend );



/* from : aviriff.h (MS file)
 * heres the general layout of an AVI riff file (new format)
 * RIFF (3F??????) AVI       <- not more than 1 GB in size
 *     LIST (size) hdrl
 *         avih (0038)
 *         LIST (size) strl
 *             strh (0038)
 *             strf (????)
 *             indx (3ff8)   <- size may vary, should be sector sized
 *         LIST (size) strl
 *             strh (0038)
 *             strf (????)
 *             indx (3ff8)   <- size may vary, should be sector sized
 *         LIST (size) odml
 *             dmlh (????)
 *         JUNK (size)       <- fill to align to sector - 12
 *     LIST (7f??????) movi  <- aligned on sector - 12
 *         00dc (size)       <- sector aligned
 *         01wb (size)       <- sector aligned
 *         ix00 (size)       <- sector aligned
 *     idx1 (00??????)       <- sector aligned
 * RIFF (7F??????) AVIX
 *     JUNK (size)           <- fill to align to sector -12
 *     LIST (size) movi
 *         00dc (size)       <- sector aligned
 * RIFF (7F??????) AVIX      <- not more than 2GB in size
 *     JUNK (size)           <- fill to align to sector - 12
 *     LIST (size) movi
 *         00dc (size)       <- sector aligned
 *
 *-===================================================================*/

/* =====================================================
   Audio Video Interleave, known by its acronym AVI, is a multimedia
   container format introduced by Microsoft in November 1992 as 
   part of its Video for Windows technology. 
   AVI files can contain both audio and video data in a file 
   container that allows synchronous audio-with-video playback.

   from : http://msdn.microsoft.com/en-us/library/ms779636(VS.85).aspx
   and : http://msdn.microsoft.com/en-us/library/dd318189(VS.85).aspx

   AVI is a derivative of the Resource Interchange File Format 
   (RIFF), which divides a file's data into blocks, or "chunks." 
   Each "chunk" is identified by a FourCC tag. An AVI file takes 
   the form of a single chunk in a RIFF formatted file, which is 
   then subdivided into two mandatory "chunks" and one optional "chunk".

   The first sub-chunk is identified by the "hdrl" tag. This sub-chunk 
   is the file header and contains metadata about the video, such as 
   its width, height and frame rate. 
   The second sub-chunk is identified by the "movi" tag. This chunk 
   contains the actual audio/visual data that make up the AVI movie. 
   The third optional sub-chunk is identified by the "idx1" tag which 
   indexes the offsets of the data chunks within the file.

   The RIFF header has the following form: 
   'RIFF' fileSize fileType (data)
File [sample.avi], 82944 bytes.
0000:0000 52 49 46 46 6A 42 01 00  41 56 49 20 4C 49 53 54 RIFFjB..AVI LIST
0000:0010 8C 05 00 00 68 64 72 6C  61 76 69 68 38 00 00 00 ....hdrlavih8...
0000:0020 40 42 0F 00 BC 1B 00 00  00 00 00 00 10 08 00 00 @B..............
0000:0030 0C 00 00 00 00 00 00 00  02 00 00 00 00 32 00 00 .............2..
0000:0040 41 01 00 00 41 01 00 00  00 00 00 00 00 00 00 00 A...A...........
0000:0050 00 00 00 00 00 00 00 00  4C 49 53 54 90 04 00 00 ........LIST....
0000:0060 73 74 72 6C 73 74 72 68  38 00 00 00 76 69 64 73 strlstrh8...vids
0000:0070 52 4C 45 20 00 00 00 00  00 00 00 00 00 00 00 00 RLE ............
0000:0080 01 00 00 00 01 00 00 00  00 00 00 00 0C 00 00 00 ................
0000:0090 7C 17 00 00 00 00 00 00  00 00 00 00 00 00 00 00 |...............
0000:00a0 41 01 41 01 73 74 72 66  28 04 00 00 28 00 00 00 A.A.strf(...(...
0000:00b0 41 01 00 00 41 01 00 00  01 00 08 00 01 00 00 00 A...A...........
0000:00c0 02 25 03 00 00 00 00 00  00 00 00 00 00 01 00 00 .%..............
    A chunk has the following form: 
    ckID ckSize ckData

    'LIST'
    A list has the following form: 
    'LIST' listSize listType listData
    The value of listSize includes the size of listType plus the size of listData; 
    it does not include the 'LIST' FOURCC or the size of listSize.

    AVI files are identified by the FOURCC 'AVI ' in the RIFF header. 
    All AVI files include two mandatory LIST chunks, which define the 
    format of the streams and the stream data, respectively. 

    RIFF ('AVI '
      LIST ('hdrl' ... )
      LIST ('movi' ... )
      ['idx1' (<AVI Index>) ]
     )
     AVI files must keep these three components in the proper sequence. 

     The 'hdrl' and 'movi' lists use subchunks for their data. 
     The following example shows the AVI RIFF form expanded with the chunks 
     needed to complete these lists: 

      RIFF ('AVI '
      LIST ('hdrl'
            'avih'(<Main AVI Header>)
            LIST ('strl'
                  'strh'(<Stream header>)
                  'strf'(<Stream format>)
                  [ 'strd'(<Additional header data>) ]
                  [ 'strn'(<Stream name>) ]
                  ...
                 )
             ...
           )
      LIST ('movi'
            {SubChunk | LIST ('rec '
                              SubChunk1
                              SubChunk2
                              ...
                             )
               ...
            }
            ...
           )
      ['idx1' (<AVI Index>) ]
     )

     MAIN HEADER from : http://msdn.microsoft.com/en-us/library/dd318180(VS.85).aspx
typedef struct _avimainheader {
    FOURCC fcc;                 // Specifies a FOURCC code. The value must be 'avih'.
    DWORD  cb;                  // Specifies the size of the structure, not including the initial 8 bytes.
    DWORD  dwMicroSecPerFrame;  // Specifies the number of microseconds between frames. This value indicates the overall timing for the file.
    DWORD  dwMaxBytesPerSec;    // Specifies the approximate maximum data rate of the file.
    DWORD  dwPaddingGranularity;// Specifies the alignment for data, in bytes. Pad the data to multiples of this value.
    DWORD  dwFlags;             // Contains a bitwise combination of zero or more of the following flags:
    DWORD  dwTotalFrames;       // Specifies the total number of frames of data in the file.
    DWORD  dwInitialFrames;     // Specifies the initial frame for interleaved files. Noninterleaved files should specify zero.
    DWORD  dwStreams;           // Specifies the number of streams in the file. For example, a file with audio and video has two streams.
    DWORD  dwSuggestedBufferSize;// Specifies the suggested buffer size for reading the file.
    DWORD  dwWidth;             // Specifies the width of the AVI file in pixels.
    DWORD  dwHeight;            // Specifies the height of the AVI file in pixels.
    DWORD  dwReserved[4];       // Reserved. Set this array to zero.
} AVIMAINHEADER;

AVI Stream Headers
One or more 'strl' lists follow the main header. A 'strl' list is required for 
each data stream. Each 'strl' list contains information about one stream 
in the file, and must contain a stream header chunk ('strh') and a 
stream format chunk ('strf').
In addition, a 'strl' list might contain a stream-header data 
chunk ('strd') and a stream name chunk ('strn').

The stream header chunk ('strh') consists of an AVISTREAMHEADER structure.
from : http://msdn.microsoft.com/en-us/library/dd318183(VS.85).aspx
typedef struct _avistreamheader {
  FOURCC fcc;                   // Specifies a FOURCC code. The value must be 'strh'
  DWORD  cb;                    // Specifies the size of the structure, not including the initial 8 bytes.
  FOURCC fccType;               // Contains a FOURCC that specifies the type of the data contained in the stream
  FOURCC fccHandler;            // Optionally, contains a FOURCC that identifies a specific data handler.
  DWORD  dwFlags;               // Contains any flags for the data stream. The bits in the high-order word of these flags are specific to the type of data 
  WORD   wPriority;             // Specifies priority of a stream type. For example, in a file with multiple audio streams, the one with the highest priority might be the default stream.
  WORD   wLanguage;             // Language tag.
  DWORD  dwInitialFrames;       // Specifies how far audio data is skewed ahead of the video frames in interleaved files. Typically, this is about 0.75 seconds.
  DWORD  dwScale;               // Used with dwRate to specify the time scale that this stream will use. Dividing dwRate by dwScale gives the number of samples per second. For video streams, this is the frame rate.
  DWORD  dwRate;                // For audio streams, this rate corresponds to the time needed to play nBlockAlign bytes of audio, which for PCM audio is the just the sample rate.
  DWORD  dwStart;               // Specifies the starting time for this stream. The units are defined by the dwRate and dwScale members 
  DWORD  dwLength;              // Specifies the length of this stream. The units are defined by the dwRate and dwScale members of the stream's header.
  DWORD  dwSuggestedBufferSize; // Specifies how large a buffer should be used to read this stream.
  DWORD  dwQuality;             // Specifies an indicator of the quality of the data in the stream. Quality is represented as a number between 0 and 10,000. -1 for default
  DWORD  dwSampleSize;          // Specifies the size of a single sample of data. This is set to zero if the samples can vary in size
  struct {
    short int left;
    short int top;
    short int right;
    short int bottom;
  } rcFrame;
}AVISTREAMHEADER;

A stream format chunk ('strf') must follow the stream header chunk. 
The stream format chunk describes the format of the data in the stream. 
The data contained in this chunk depends on the stream type. 
For video streams, the information is a BITMAPINFO structure, 
including palette information if appropriate.

from : http://msdn.microsoft.com/en-us/library/dd318229(VS.85).aspx
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;         // Specifies the number of bytes required by the structure
  LONG  biWidth;        // Specifies the width of the bitmap, in pixels.
  LONG  biHeight;       // Specifies the height of the bitmap, in pixels
  WORD  biPlanes;       // Specifies the number of planes for the target device. This value must be set to 1.
  WORD  biBitCount;     // Specifies the number of bits per pixel (bpp).
  DWORD biCompression;  // For compressed video and YUV formats, this member is a FOURCC code
  DWORD biSizeImage;    // Specifies the size, in bytes, of the image. This can be set to 0
  LONG  biXPelsPerMeter;// Specifies the horizontal resolution, in pixels per meter
  LONG  biYPelsPerMeter;// Specifies the vertical resolution, in pixels per meter
  DWORD biClrUsed;      // Specifies the number of color indices in the color table that are actually used by the bitmap
  DWORD biClrImportant; // Specifies the number of color indices that are considered important for displaying the bitmap
}BITMAPINFOHEADER;

For audio streams, the information is a WAVEFORMATEX structure.
from : http://msdn.microsoft.com/en-us/library/dd390970(VS.85).aspx
typedef struct {
  WORD  wFormatTag;     // Waveform-audio format type.
  // Format tags are registered with Microsoft Corporation for many compression 
  // algorithms. A complete list of format tags can be found in the Mmreg.h 
  // header file. For one- or two-channel Pulse Code Modulation (PCM) data, 
  // this value should be WAVE_FORMAT_PCM.
  WORD  nChannels;      // Number of channels in the waveform-audio data
  DWORD nSamplesPerSec; // Sample rate, in samples per second (hertz). 
  // If wFormatTag is WAVE_FORMAT_PCM, then common values for nSamplesPerSec 
  // are 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz. 
  // For non-PCM formats, this member must be computed according to the 
  // manufacturer's specification of the format tag. 
  DWORD nAvgBytesPerSec; // Required average data-transfer rate, in bytes per second, for the format tag.
  WORD  nBlockAlign;    // Block alignment, in bytes. 
  // The block alignment is the minimum atomic unit of data for the 
  // wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM or 
  // WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of 
  // nChannels and wBitsPerSample divided by 8 (bits per byte). 
  // For non-PCM formats, blah, blah, blah
  WORD  wBitsPerSample; // Bits per sample for the wFormatTag format type.
  // If wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be 
  // equal to 8 or 16. For non-PCM formats, blah blah blah.
  // If wFormatTag is WAVE_FORMAT_EXTENSIBLE, this value can be any 
  // integer multiple of 8. Some compression schemes cannot define a 
  // value for wBitsPerSample, so this member can be zero. 
  WORD  cbSize;     // Size, in bytes, of extra format information 
  // appended to the end of the WAVEFORMATEX structure. This information 
  // can be used by non-PCM formats to store extra attributes for the wFormatTag
}WAVEFORMATEX;

typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /=* bits of precision  *=/
        WORD wSamplesPerBlock;          /=* valid if wBitsPerSample==0 *=/
        WORD wReserved;                 /=* If neither applies, set to zero. *=/
    } Samples;
    DWORD           dwChannelMask;      /=* which channels are *=/
                                        /=* present in stream  *=/
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
http://msdn.microsoft.com/en-us/library/dd390701(VS.85).aspx
typedef struct mpeg1waveformat_tag {
  WAVEFORMATEX wfx;
  WORD         fwHeadLayer;
  DWORD        dwHeadBitrate;
  WORD         fwHeadMode;
  WORD         fwHeadModeExt;
  WORD         wHeadEmphasis;
  WORD         fwHeadFlags;
  DWORD        dwPTSLow;
  DWORD        dwPTSHigh;
}MPEG1WAVEFORMAT;
http://msdn.microsoft.com/en-us/library/dd390710(VS.85).aspx
typedef struct mpeglayer3waveformat_tag {
  WAVEFORMATEX wfx;
  WORD         wID;
  DWORD        fdwFlags;
  WORD         nBlockSize;
  WORD         nFramesPerBlock;
  WORD         nCodecDelay;
}MPEGLAYER3WAVEFORMAT;

'idx1'
The FOURCC that identifies each data chunk consists of a two-digit 
stream number followed by a two-character code that defines the 
type of information in the chunk.
Two-character code  Description  
db Uncompressed video frame 
dc Compressed video frame 
pc Palette change 
wb Audio data 

typedef struct _avioldindex {
  FOURCC                    fcc;
  DWORD                     cb;
  struct _avioldindex_entry {
    DWORD dwChunkId;    // Specifies a FOURCC that identifies a stream in the AVI file.
    DWORD dwFlags;      // Specifies a bitwise combination of zero or more of the following flags:
    DWORD dwOffset;     // Specifies the location of the data chunk in the file.
    // maybe from 'movi' LIST entry, or beginning of file
    DWORD dwSize;       // Specifies the size of the data chunk, in bytes
  } aIndex[];
}AVIOLDINDEX;
Flags:
AVIIF_KEYFRAME The data chunk is a key frame.
AVIIF_LIST The data chunk is a 'rec ' list.
AVIIF_NO_TIME The data chunk does not affect the timing of the stream. For example, this flag should be set for palette changes.

OpenDML extension
Another new feature was the inclusion of an "index" that allowed 
data stored in the file to be easily obtained. this meant that it 
was more efficient and provided smother playback of AVI files.

Base Index Form indx
Thus the actual implementation is based on a base index form indx:
struct _aviindex_chunk {
    FOURCC fcc;
    DWORD cb;
    WORD wLongsPerEntry; // size of each entry in aIndex array
    BYTE bIndexSubType; // future use. must be 0
    BYTE bIndexType; // one of AVI_INDEX_* codes
    DWORD nEntriesInUse; // index of first unused member in aIndex array
    DWORD dwChunkId; // fcc of what is indexed
    DWORD dwReserved[3]; // meaning differs for each index
                        // type/subtype. 0 if unused
    struct _aviindex_entry {
        DWORD adw[wLongsPerEntry];
    } aIndex[ ];
};

AVI Standard Index Chunk
The AVI Standard Index chunk contains information that indexes AVI frames.
  ========================== */
#ifdef AVIRIFF_H
// already defined
#else
#ifndef QUADWORD
struct quadword { long w[4]; };
#define QUADWORD struct quadword
#endif
typedef struct _avistdindex_chunk {
    FOURCC fcc; // ix##
    DWORD cb;
    WORD wLongsPerEntry; // must be sizeof(aIndex[0])/sizeof(DWORD)
    BYTE bIndexSubType; // must be 0
    BYTE bIndexType; // must be AVI_INDEX_OF_CHUNKS
    DWORD nEntriesInUse; //
    DWORD dwChunkId; // ##dc or ##db or ##wb etc..
    QUADWORD qwBaseOffset; // all dwOffsets in aIndex array are
                            // relative to this
    DWORD dwReserved3; // must be 0
    struct _avistdindex_entry {
        DWORD dwOffset; // qwBaseOffset + this is absolute file offset
        DWORD dwSize; // bit 31 is set if this is NOT a keyframe
    } aIndex[ ];
} AVISTDINDEX, * PAVISTDINDEX;
#endif
/* =================================================
AVI Field Index Chunk
The AVI Field Index Chunk is the same as the Standard Index Chunk except that it contains
the locations of each field in the frame.
   ================================================= */
#ifdef AVIRIFF_H
// already defined
#else
typedef struct _avifieldindex_chunk {
    FOURCC fcc; // ix##
    DWORD cb;
    WORD wLongsPerEntry; // must be 3 (size of each entry in
                         // aIndex array)
    BYTE bIndexSubType; // AVI_INDEX_2FIELD
    BYTE bIndexType; // AVI_INDEX_OF_CHUNKS
    DWORD nEntriesInUse; //
    DWORD dwChunkId; // ##dc or ##db
    QUADWORD qwBaseOffset; // offsets in aIndex array are relative to this
    DWORD dwReserved3; // must be 0
    struct _avifieldindex_entry {
        DWORD dwOffset;
        DWORD dwSize; // size of all fields
                        // (bit 31 set for NON-keyframes)
        DWORD dwOffsetField2; // offset to second field
    } aIndex[ ];
} AVIFIELDINDEX, * PAVIFIELDINDEX;
#endif 
/* ======================================================
AVI Super Index Chunk
The Super Index Chunk is an index of indexes and is always found 
in the indx chunk of an AVI file. It is defined as follows:
   ====================================================== */
#ifdef AVIRIFF_H
// already defined
#else
typedef struct _avisuperindex_chunk {
    FOURCC fcc; // ix##
    DWORD cb; // size of this structure
    WORD wLongsPerEntry; // must be 4 (size of each entry in aIndex array)
    BYTE bIndexSubType; // must be 0 or AVI_INDEX_2FIELD
    BYTE bIndexType; // must be AVI_INDEX_OF_INDEXES
    DWORD nEntriesInUse; // number of entries in aIndex array that
                            // are used
    DWORD dwChunkId; // ##dc or ##db or ##wb, etc
    DWORD dwReserved[3]; // must be 0
    struct _avisuperindex_entry {
        QUADWORD qwOffset; // absolute file offset, offset 0 is
                            // unused entry??
        DWORD dwSize; // size of index chunk at this offset
        DWORD dwDuration; // time span in stream ticks
    } aIndex[ ];
} AVISUPERINDEX, * PAVISUPERINDEX;
#endif
/* ==================================================
Index Locations in RIFF File
Unlike the idx1 chunk, a single index is stored per 
stream in the AVI file. An indx chunk 
follows the strf chunk in the LIST strl chunk of 
an AVI header. This indx chunk may
either be an index of indexes (super index), or 
may be an index to the chunks directly. In the
case of video, this means that the chunk is 
either a AVISUPERINDEX or an AVIFIELDINDEX/AVISTDINDEX.
If the indx chunk is a standard or field index 
chunk (i.e., not an index of indexes) then the
stream has only one index chunk and there is none in 
the movi data.

AVIPALCHANGE Structure

'xxpc'
The AVIPALCHANGE structure defines a palette change in an AVI file.
typedef struct {
  BYTE         bFirstEntry;
  BYTE         bNumEntries;
  WORD         wFlags;
  PALETTEENTRY peNew[];
}AVIPALCHANGE;

   ===================================================== */


typedef struct tagFLAG2STG {
    DWORD flag;
    CHAR * desc;
    CHAR * sdesc;
}FLAG2STG, * PFLAG2STG;

FLAG2STG hdr_flag[] = {
    { AVIF_COPYRIGHTED, "Indicates the AVI file contains copyrighted data and software. When this flag is used, software should not permit the data to be duplicated.", "COPYRIGHTED" },
    { AVIF_HASINDEX,    "Indicates the AVI file has an index.", "HASINDEX" },
    { AVIF_ISINTERLEAVED, "Indicates the AVI file is interleaved.", "ISINTERLEAVED" },
    { AVIF_MUSTUSEINDEX, "Indicates that application should use the index, rather than the physical ordering of the chunks in the file, to determine the order of presentation of the data. For example, this flag could be used to create a list of frames for editing.", "MUSTUSEINDEX" },
    { AVIF_WASCAPTUREFILE, "Indicates the AVI file is a specially allocated file used for capturing real-time video. Applications should warn the user before writing over a file with this flag set because the user probably defragmented this file.", "WASCAPTUREFILE" },
    { 0, 0, 0 }
};

FLAG2STG strm_flag[] = {
    { AVISF_DISABLED, "Indicates this stream should not be enabled by default.", "AVISF_DISABLED" },
    { AVISF_VIDEO_PALCHANGES, "Indicates this video stream contains palette changes. This flag warns the playback software that it will need to animate the palette.", "AVISF_VIDEO_PALCHANGES"},
    { 0, 0, 0 }
};

FLAG2STG index_flags[] = {
    { AVIIF_KEYFRAME, "The data chunk is a key frame.", "KEYFRAME" },
    { AVIIF_LIST, "The data chunk is a 'rec ' list.", "LIST" },
    { AVIIF_NO_TIME, "The data chunk does not affect the timing of the stream. For example, this flag should be set for palette changes.", "NOTIME" },
    { 0, 0, 0 }
};


/*  MPEG-1 audio wave format (audio layer only).   (0x0050)   */
FLAG2STG wav_format[] = {
   { WAVE_FORMAT_OLIADPCM, "Ing C. Olivetti & C., S.p.A. WAVE_FORMAT_OLIADPCM (0x1001)", "Ing C. Olivetti & C., S.p.A." },
   { WAVE_FORMAT_DSPGROUP_TRUESPEECH, "DSP Group, Inc WAVE_FORMAT_DSPGROUP_TRUESPEECH (0x0022)", "DSP Group, Inc" },
   { WAVE_FORMAT_DVM, "FAST Multimedia AG WAVE_FORMAT_DVM (0x2000)", "FAST Multimedia AG" },
   { WAVE_FORMAT_TPC, "AT&T Labs, Inc. WAVE_FORMAT_TPC (0x0681)", "AT&T Labs, Inc." },
   { WAVE_FORMAT_SOFTSOUND, "Softsound, Ltd. WAVE_FORMAT_SOFTSOUND (0x0080)", "Softsound, Ltd." },
   { WAVE_FORMAT_VOXWARE_AC10, "Voxware Inc WAVE_FORMAT_VOXWARE_AC10 (0x0071)", "Voxware Inc" },
   { WAVE_FORMAT_DIGIADPCM, "DSP Solutions, Inc. WAVE_FORMAT_DIGIADPCM (0x0036)", "DSP Solutions, Inc." },
   { WAVE_FORMAT_VOXWARE_AC16, "Voxware Inc WAVE_FORMAT_VOXWARE_AC16 (0x0072)", "Voxware Inc" },
   { WAVE_FORMAT_UNISYS_NAP_16K, "Unisys Corp. WAVE_FORMAT_UNISYS_NAP_16K (0x0173)", "Unisys Corp." },
   { WAVE_FORMAT_ECHOSC1, "Echo Speech Corporation WAVE_FORMAT_ECHOSC1 (0x0023)", "Echo Speech Corporation" },
   { WAVE_FORMAT_PROSODY_8KBPS, "Aculab plc WAVE_FORMAT_PROSODY_8KBPS (0x0094)", "Aculab plc" },
   { WAVE_FORMAT_ECHOSC3, "Echo Speech Corporation WAVE_FORMAT_ECHOSC3 (0x003A)", "Echo Speech Corporation" },
   { WAVE_FORMAT_OLISBC, "Ing C. Olivetti & C., S.p.A. WAVE_FORMAT_OLISBC (0x1003)", "Ing C. Olivetti & C., S.p.A." },
   { WAVE_FORMAT_G726ADPCM, "Dictaphone Corporation WAVE_FORMAT_G726ADPCM (0x0140)", "Dictaphone Corporation" },
   { WAVE_FORMAT_AUDIOFILE_AF10, "Virtual Music, Inc. WAVE_FORMAT_AUDIOFILE_AF10 (0x0026)", "Virtual Music, Inc." },
   { WAVE_FORMAT_NMS_VBXADPCM, "Natural MicroSystems WAVE_FORMAT_NMS_VBXADPCM (0x0038)", "Natural MicroSystems" },
   { WAVE_FORMAT_ALAW, "Microsoft Corporation WAVE_FORMAT_ALAW (0x0006)", "Microsoft Corporation" },
   { WAVE_FORMAT_GSM610, "Microsoft Corporation WAVE_FORMAT_GSM610 (0x0031)", "Microsoft Corporation" },
   { WAVE_FORMAT_SIPROLAB_ACELP4800, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_ACELP4800 (0x0131)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_RHETOREX_ADPCM, "Rhetorex Inc. WAVE_FORMAT_RHETOREX_ADPCM (0x0100)", "Rhetorex Inc." },
   { WAVE_FORMAT_VOXWARE_TQ40, "Voxware Inc WAVE_FORMAT_VOXWARE_TQ40 (0x0079)", "Voxware Inc" },
   { WAVE_FORMAT_SONY_SCX, "Sony Corp. WAVE_FORMAT_SONY_SCX (0x0270)", "Sony Corp." },
   { WAVE_FORMAT_NORRIS, "Norris Communications, Inc. WAVE_FORMAT_NORRIS (0x1400)", "Norris Communications, Inc." },
   { WAVE_FORMAT_SANYO_LD_ADPCM, "Sanyo Electric Co., Ltd. WAVE_FORMAT_SANYO_LD_ADPCM (0x0125)", "Sanyo Electric Co., Ltd." },
   { WAVE_FORMAT_DIGIREAL, "DSP Solutions, Inc. WAVE_FORMAT_DIGIREAL (0x0035)", "DSP Solutions, Inc." },
   { WAVE_FORMAT_VME_VMPCM, "AT&T Labs, Inc. WAVE_FORMAT_VME_VMPCM (0x0680)", "AT&T Labs, Inc." },
   { WAVE_FORMAT_VOXWARE_BYTE_ALIGNED, "Voxware Inc WAVE_FORMAT_VOXWARE_BYTE_ALIGNED (0x0069)", "Voxware Inc" },
   { WAVE_FORMAT_IPI_RPELP, "Interactive Products, Inc. WAVE_FORMAT_IPI_RPELP (0x0251)", "Interactive Products, Inc." },
   { WAVE_FORMAT_CS_IMAADPCM, "Crystal Semiconductor IMA ADPCM WAVE_FORMAT_CS_IMAADPCM (0x0039)", "Crystal Semiconductor IMA ADPCM" },
   { WAVE_FORMAT_CREATIVE_ADPCM, "Creative Labs, Inc WAVE_FORMAT_CREATIVE_ADPCM (0x0200)", "Creative Labs, Inc" },
   { WAVE_FORMAT_MEDIASONIC_G723, "MediaSonic WAVE_FORMAT_MEDIASONIC_G723 (0x0093)", "MediaSonic" },
   { WAVE_FORMAT_VOXWARE_RT29HW, "Voxware Inc WAVE_FORMAT_VOXWARE_RT29HW (0x0076)", "Voxware Inc" },
   { WAVE_FORMAT_CIRRUS, "Cirrus Logic WAVE_FORMAT_CIRRUS (0x0060)", "Cirrus Logic" },
   { WAVE_FORMAT_MSNAUDIO, "Microsoft Corporation WAVE_FORMAT_MSNAUDIO (0x0032)", "Microsoft Corporation" },
   { WAVE_FORMAT_OKI_ADPCM, "OKI WAVE_FORMAT_OKI_ADPCM (0x0010)", "OKI" },
   { WAVE_FORMAT_DIGITAL_G723, "Digital Equipment Corporation WAVE_FORMAT_DIGITAL_G723 (0x0123)", "Digital Equipment Corporation" },
   { WAVE_FORMAT_SONARC, "Speech Compression WAVE_FORMAT_SONARC (0x0021)", "Speech Compression" },
   { WAVE_FORMAT_SIPROLAB_G729, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_G729 (0x0133)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_YAMAHA_ADPCM, "Yamaha Corporation of America WAVE_FORMAT_YAMAHA_ADPCM (0x0020)", "Yamaha Corporation of America" },
   { WAVE_FORMAT_VOXWARE_VR18, "Voxware Inc WAVE_FORMAT_VOXWARE_VR18 (0x0078)", "Voxware Inc" },
   { WAVE_FORMAT_MULAW, "Microsoft Corporation WAVE_FORMAT_MULAW (0x0007)", "Microsoft Corporation" },
   { WAVE_FORMAT_SIPROLAB_ACELP8V3, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_ACELP8V3 (0x0132)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_DRM, "Microsoft Corporation WAVE_FORMAT_DRM (0x0009)", "Microsoft Corporation" },
   { WAVE_FORMAT_CS2, "Consistent Software WAVE_FORMAT_CS2 (0x0260)", "Consistent Software" },
   { WAVE_FORMAT_UNKNOWN, "Microsoft Corporation WAVE_FORMAT_UNKNOWN (0x0000)", "Microsoft Corporation" },
   { WAVE_FORMAT_SIPROLAB_KELVIN, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_KELVIN (0x0135)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_VIVO_G723, "Vivo Software WAVE_FORMAT_VIVO_G723 (0x0111)", "Vivo Software" },
   { WAVE_FORMAT_ROCKWELL_DIGITALK, "Rockwell International WAVE_FORMAT_ROCKWELL_DIGITALK (0x003C)", "Rockwell International" },
   { WAVE_FORMAT_MSG723, "Microsoft Corporation WAVE_FORMAT_MSG723 (0x0042)", "Microsoft Corporation" },
   { WAVE_FORMAT_OLICELP, "Ing C. Olivetti & C., S.p.A. WAVE_FORMAT_OLICELP (0x1002)", "Ing C. Olivetti & C., S.p.A." },
   { WAVE_FORMAT_MVI_MVI2, "Motion Pixels WAVE_FORMAT_MVI_MVI2 (0x0084)", "Motion Pixels" },
   { WAVE_FORMAT_IRAT, "BeCubed Software Inc. WAVE_FORMAT_IRAT (0x0101)", "BeCubed Software Inc." },
   { WAVE_FORMAT_VSELP, "Compaq Computer Corp. WAVE_FORMAT_VSELP (0x0004)", "Compaq Computer Corp." },
   { WAVE_FORMAT_PHILIPS_LPCBB, "Philips Speech Processing WAVE_FORMAT_PHILIPS_LPCBB (0x0098)", "Philips Speech Processing" },
   { WAVE_FORMAT_DSAT_DISPLAY, "Microsoft Corporation WAVE_FORMAT_DSAT_DISPLAY (0x0067)", "Microsoft Corporation" },
   { WAVE_FORMAT_CONTROL_RES_CR10, "Control Resources Limited WAVE_FORMAT_CONTROL_RES_CR10 (0x0037)", "Control Resources Limited" },
   { WAVE_FORMAT_UNISYS_NAP_ADPCM, "Unisys Corp. WAVE_FORMAT_UNISYS_NAP_ADPCM (0x0170)", "Unisys Corp." },
   { WAVE_FORMAT_VOXWARE_RT24, "Voxware Inc WAVE_FORMAT_VOXWARE_RT24 (0x0074)", "Voxware Inc" },
   { WAVE_FORMAT_AUDIOFILE_AF36, "Virtual Music, Inc. WAVE_FORMAT_AUDIOFILE_AF36 (0x0024)", "Virtual Music, Inc." },
   { WAVE_FORMAT_VOXWARE_VR12, "Voxware Inc WAVE_FORMAT_VOXWARE_VR12 (0x0077)", "Voxware Inc" },
   { WAVE_FORMAT_CREATIVE_FASTSPEECH10, "Creative Labs, Inc WAVE_FORMAT_CREATIVE_FASTSPEECH10 (0x0203)", "Creative Labs, Inc" },
   { WAVE_FORMAT_G726_ADPCM, "APICOM WAVE_FORMAT_G726_ADPCM (0x0064)", "APICOM" },
   { WAVE_FORMAT_VOXWARE_AC20, "Voxware Inc WAVE_FORMAT_VOXWARE_AC20 (0x0073)", "Voxware Inc" },
   { WAVE_FORMAT_QUALCOMM_HALFRATE, "Qualcomm, Inc. WAVE_FORMAT_QUALCOMM_HALFRATE (0x0151)", "Qualcomm, Inc." },
   { WAVE_FORMAT_G729A, "AT&T Labs, Inc. WAVE_FORMAT_G729A (0x0083)", "AT&T Labs, Inc." },
   { WAVE_FORMAT_DF_G726, "DataFusion Systems (Pty) (Ltd) WAVE_FORMAT_DF_G726 (0x0085)", "DataFusion Systems (Pty) (Ltd)" },
   { WAVE_FORMAT_DOLBY_AC3_SPDIF, "Sonic Foundry WAVE_FORMAT_DOLBY_AC3_SPDIF (0x0092)", "Sonic Foundry" },
   { WAVE_FORMAT_VOXWARE_AC8, "Voxware Inc WAVE_FORMAT_VOXWARE_AC8 (0x0070)", "Voxware Inc" },
   { WAVE_FORMAT_VOXWARE_TQ60, "Voxware Inc WAVE_FORMAT_VOXWARE_TQ60 (0x0081)", "Voxware Inc" },
   { WAVE_FORMAT_UHER_ADPCM, "UHER informatic GmbH WAVE_FORMAT_UHER_ADPCM (0x0210)", "UHER informatic GmbH" },
   { WAVE_FORMAT_MEDIASPACE_ADPCM, "Videologic WAVE_FORMAT_MEDIASPACE_ADPCM (0x0012)", "Videologic" },
   { WAVE_FORMAT_ISIAUDIO, "Iterated Systems, Inc. WAVE_FORMAT_ISIAUDIO (0x0088)", "Iterated Systems, Inc." },
   { WAVE_FORMAT_ZYXEL_ADPCM, "ZyXEL Communications, Inc. WAVE_FORMAT_ZYXEL_ADPCM (0x0097)", "ZyXEL Communications, Inc." },
   { WAVE_FORMAT_MALDEN_PHONYTALK, "Malden Electronics Ltd. WAVE_FORMAT_MALDEN_PHONYTALK (0x00A0)", "Malden Electronics Ltd." },
   { WAVE_FORMAT_EXTENSIBLE, "Microsoft WAVE_FORMAT_EXTENSIBLE (0xFFFE)", "Microsoft" },
   { WAVE_FORMAT_G722_ADPCM, "APICOM WAVE_FORMAT_G722_ADPCM (0x0065)", "APICOM" },
   { WAVE_FORMAT_SIPROLAB_G729A, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_G729A (0x0134)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_G721_ADPCM, "Antex Electronics Corporation WAVE_FORMAT_G721_ADPCM (0x0040)", "Antex Electronics Corporation" },
   { WAVE_FORMAT_G723_ADPCM, "Antex Electronics Corporation WAVE_FORMAT_G723_ADPCM (0x0014)", "Antex Electronics Corporation" },
   { WAVE_FORMAT_RT24, "InSoft, Inc. WAVE_FORMAT_RT24 (0x0052)", "InSoft, Inc." },
   { WAVE_FORMAT_OLIGSM, "Ing C. Olivetti & C., S.p.A. WAVE_FORMAT_OLIGSM (0x1000)", "Ing C. Olivetti & C., S.p.A." },
   { WAVE_FORMAT_ESPCM, "ESS Technology WAVE_FORMAT_ESPCM (0x0061)", "ESS Technology" },
   { WAVE_FORMAT_SIERRA_ADPCM, "Sierra Semiconductor Corp WAVE_FORMAT_SIERRA_ADPCM (0x0013)", "Sierra Semiconductor Corp" },
   { WAVE_FORMAT_ANTEX_ADPCME, "Antex Electronics Corporation WAVE_FORMAT_ANTEX_ADPCME (0x0033)", "Antex Electronics Corporation" },
   { WAVE_FORMAT_TUBGSM, "Ring Zero Systems, Inc. WAVE_FORMAT_TUBGSM (0x0155)", "Ring Zero Systems, Inc." },
   { WAVE_FORMAT_CREATIVE_FASTSPEECH8, "Creative Labs, Inc WAVE_FORMAT_CREATIVE_FASTSPEECH8 (0x0202)", "Creative Labs, Inc" },
   { WAVE_FORMAT_FM_TOWNS_SND, "Fujitsu Corp. WAVE_FORMAT_FM_TOWNS_SND (0x0300)", "Fujitsu Corp." },
   { WAVE_FORMAT_VOXWARE_RT29, "Voxware Inc WAVE_FORMAT_VOXWARE_RT29 (0x0075)", "Voxware Inc" },
   { WAVE_FORMAT_PACKED, "Studer Professional Audio AG WAVE_FORMAT_PACKED (0x0099)", "Studer Professional Audio AG" },
   { WAVE_FORMAT_UNISYS_NAP_ALAW, "Unisys Corp. WAVE_FORMAT_UNISYS_NAP_ALAW (0x0172)", "Unisys Corp." },
   { WAVE_FORMAT_VIVO_SIREN, "Vivo Software WAVE_FORMAT_VIVO_SIREN (0x0112)", "Vivo Software" },
   { WAVE_FORMAT_VOXWARE, "Voxware Inc WAVE_FORMAT_VOXWARE (0x0062)", "Voxware Inc" },
   { WAVE_FORMAT_MEDIAVISION_ADPCM, "Media Vision, Inc. WAVE_FORMAT_MEDIAVISION_ADPCM (0x0018)", "Media Vision, Inc." },
   { WAVE_FORMAT_DIGIFIX, "DSP Solutions, Inc. WAVE_FORMAT_DIGIFIX (0x0016)", "DSP Solutions, Inc." },
   { WAVE_FORMAT_MPEG, "Microsoft Corporation WAVE_FORMAT_MPEG (0x0050)", "Microsoft Corporation" },
   { WAVE_FORMAT_PROSODY_1612, "Aculab plc WAVE_FORMAT_PROSODY_1612 (0x0027)", "Aculab plc" },
   { WAVE_FORMAT_DF_GSM610, "DataFusion Systems (Pty) (Ltd) WAVE_FORMAT_DF_GSM610 (0x0086)", "DataFusion Systems (Pty) (Ltd)" },
   { WAVE_FORMAT_PAC, "InSoft, Inc. WAVE_FORMAT_PAC (0x0053)", "InSoft, Inc." },
   { WAVE_FORMAT_LH_CODEC, "Lernout & Hauspie WAVE_FORMAT_LH_CODEC (0x1100)", "Lernout & Hauspie" },
   { WAVE_FORMAT_MSRT24, "Microsoft Corporation WAVE_FORMAT_MSRT24 (0x0082)", "Microsoft Corporation" },
   { WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS, "AT&T Labs, Inc. WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS (0x1500)", "AT&T Labs, Inc." },
   { WAVE_FORMAT_CANOPUS_ATRAC, "Canopus, co., Ltd. WAVE_FORMAT_CANOPUS_ATRAC (0x0063)", "Canopus, co., Ltd." },
   { WAVE_FORMAT_ONLIVE, "OnLive! Technologies, Inc. WAVE_FORMAT_ONLIVE (0x0089)", "OnLive! Technologies, Inc." },
   { WAVE_FORMAT_DIGISTD, "DSP Solutions, Inc. WAVE_FORMAT_DIGISTD (0x0015)", "DSP Solutions, Inc." },
   { WAVE_FORMAT_OLIOPR, "Ing C. Olivetti & C., S.p.A. WAVE_FORMAT_OLIOPR (0x1004)", "Ing C. Olivetti & C., S.p.A." },
   { WAVE_FORMAT_DOLBY_AC2, "Dolby Laboratories WAVE_FORMAT_DOLBY_AC2 (0x0030)", "Dolby Laboratories" },
   { WAVE_FORMAT_LRC, "Merging Technologies S.A. WAVE_FORMAT_LRC (0x0028)", "Merging Technologies S.A." },
   { WAVE_FORMAT_IPI_HSX, "Interactive Products, Inc. WAVE_FORMAT_IPI_HSX (0x0250)", "Interactive Products, Inc." },
   { WAVE_FORMAT_QUALCOMM_PUREVOICE, "Qualcomm, Inc. WAVE_FORMAT_QUALCOMM_PUREVOICE (0x0150)", "Qualcomm, Inc." },
   { WAVE_FORMAT_DIALOGIC_OKI_ADPCM, "Dialogic Corporation WAVE_FORMAT_DIALOGIC_OKI_ADPCM (0x0017)", "Dialogic Corporation" },
   { WAVE_FORMAT_IEEE_FLOAT, "Microsoft Corporation WAVE_FORMAT_IEEE_FLOAT (0x0003)", "Microsoft Corporation" },
   { WAVE_FORMAT_IBM_CVSD, "IBM Corporation WAVE_FORMAT_IBM_CVSD (0x0005)", "IBM Corporation" },
   { WAVE_FORMAT_CU_CODEC, "Hewlett-Packard Company WAVE_FORMAT_CU_CODEC (0x0019)", "Hewlett-Packard Company" },
   { WAVE_FORMAT_DTS, "Microsoft Corporation WAVE_FORMAT_DTS (0x0008)", "Microsoft Corporation" },
   { WAVE_FORMAT_LUCENT_G723, "Lucent Technologies WAVE_FORMAT_LUCENT_G723 (0x0059)", "Lucent Technologies" },
   { WAVE_FORMAT_QDESIGN_MUSIC, "QDesign Corporation WAVE_FORMAT_QDESIGN_MUSIC (0x0450)", "QDesign Corporation" },
   { WAVE_FORMAT_BTV_DIGITAL, "Brooktree Corporation WAVE_FORMAT_BTV_DIGITAL (0x0400)", "Brooktree Corporation" },
   { WAVE_FORMAT_CONTROL_RES_VQLPC, "Control Resources Limited WAVE_FORMAT_CONTROL_RES_VQLPC (0x0034)", "Control Resources Limited" },
   { WAVE_FORMAT_ADPCM, "Microsoft Corporation WAVE_FORMAT_ADPCM (0x0002)", "Microsoft Corporation" },
   { WAVE_FORMAT_ROCKWELL_ADPCM, "Rockwell International WAVE_FORMAT_ROCKWELL_ADPCM (0x003B)", "Rockwell International" },
   { WAVE_FORMAT_RAW_SPORT, "Aureal Semiconductor WAVE_FORMAT_RAW_SPORT (0x0240)", "Aureal Semiconductor" },
   { WAVE_FORMAT_G728_CELP, "Antex Electronics Corporation WAVE_FORMAT_G728_CELP (0x0041)", "Antex Electronics Corporation" },
   { WAVE_FORMAT_SIPROLAB_ACEPLNET, "Sipro Lab Telecom Inc. WAVE_FORMAT_SIPROLAB_ACEPLNET (0x0130)", "Sipro Lab Telecom Inc." },
   { WAVE_FORMAT_DVI_ADPCM, "Intel Corporation WAVE_FORMAT_DVI_ADPCM (0x0011)", "Intel Corporation" },
   { WAVE_FORMAT_SBC24, "Siemens Business Communications Sys WAVE_FORMAT_SBC24 (0x0091)", "Siemens Business Communications Sys" },
   { WAVE_FORMAT_MSAUDIO1, "Microsoft Corporation WAVE_FORMAT_MSAUDIO1 (0x0160)", "Microsoft Corporation" },
   { WAVE_FORMAT_ILINK_VC, "I-link Worldwide WAVE_FORMAT_ILINK_VC (0x0230)", "I-link Worldwide" },
   { WAVE_FORMAT_MPEGLAYER3, "ISO/MPEG Layer3 Format Tag WAVE_FORMAT_MPEGLAYER3 (0x0055)", "ISO/MPEG Layer3 Format Tag" },
   { WAVE_FORMAT_ESST_AC3, "ESS Technology, Inc. WAVE_FORMAT_ESST_AC3 (0x0241)", "ESS Technology, Inc." },
   { WAVE_FORMAT_XEBEC, "Xebec Multimedia Solutions Limited WAVE_FORMAT_XEBEC (0x003D)", "Xebec Multimedia Solutions Limited" },
   { WAVE_FORMAT_UNISYS_NAP_ULAW, "Unisys Corp. WAVE_FORMAT_UNISYS_NAP_ULAW (0x0171)", "Unisys Corp." },
   { WAVE_FORMAT_QUARTERDECK, "Quarterdeck Corporation WAVE_FORMAT_QUARTERDECK (0x0220)", "Quarterdeck Corporation" },
   { WAVE_FORMAT_APTX, "Audio Processing Technology WAVE_FORMAT_APTX (0x0025)", "Audio Processing Technology" },
   // { WAVE_FORMAT_PCM, "WAVE_FORMAT_PCM", "WAVE_FORMAT_PCM" },   // = 0x01
   { WAVE_FORMAT_PCM,  "Microsoft Corporation WAVE_FORMAT_PCM (0x0001)", "Microsoft Corporation PCM" },
   { 0, 0, 0 }
};

typedef struct tagCODEC4CC {
    char * codec;
    char * src;
    char * desc1;
    char * desc2;
}CODEC4CC, * PCODEC4CC;

CODEC4CC codec_4cc[] = {
    { "3IV1", "3ivx", "3IVX", "Description, Products Using the Codec, etc." },
    { "3IV2", "3ivx", "3IVX", "MPEG4-based codec. Used by 3ivx Delta 1.0-3.5. FOURCC \"3IV0\" was also used for a while but never publicly released." },
    { "8BPS", "Planar RGB Codec", "Apple?", "MPEG4-based codec. To be used for \"3ivx Delta 4.0.\"" },
    { "AASC", "Autodesk Animator codec", "Autodesk", "RLE codec storing RGB image in planar format under Quicktime." },
    { "ABYR", "?", "Kensington?", "This codec is part of Autodesk's discontinued Animator Studio for Windows." },
    { "ADV1", "WaveCodec", "Loronix", "Apparently a low resolution, low frame rate (6fps) codec similar to AJPG which is used in implementing movie capture in some digital cameras." },
    { "ADVJ", "Avid M-JPEG", "Avid Technology", "Apparently used in various CCTV products." },
    { "AEMI", "Array VideoONE MPEG1-I Capture", "Array Microsystems", "Also known as AVRn" },
    { "AFLI", "Autodesk Animator codec", "Autodesk", "Array's codec used for I frame only MPEG1 AVI files" },
    { "AFLC", "Autodesk Animator codec", "Autodesk", "AVI equivalent of Autodesk's native FLI file format (presumably)." },
    { "AJPG", "?", "?", "AVI equivalent of the FLC native file format." },
    { "AMPG", "Array VideoONE MPEG", "Array Microsystems", "22fps JPEG-based codec used for movie capture by some digital cameras." },
    { "ANIM", "RDX", "Intel", "Codec for Array VideoONE hardware-based MPEG compression system." },
    { "AP41", "AngelPotion Definitive", "AngelPotion", "?" },
    { "ASLC", "Alparysoft Lossless Codec", "Alparysoft", "Another hacked version of Microsoft's MP43 codec. One source recommends against installing this codec \"due to its occasional tendency to modify client structures\". Apparently this means that it can destroy or otherwise mess up the HKEY_CLASSES_ROOT\avifile section of your registry leaving some tools incapable of playing video any more." },
    { "ASV1", "", "Asus", "Codec offering approximately 5x compression in mathematically lossless mode or 5-15x compression in \"visually lossless\" operation." },
    { "ASV2", "", "Asus", "Codec supplied with the Asus TNT Video Capture adapter. Supposedly a very simple, DCT-based codec. Complete technical details can be found here." },
    { "ASVX", "", "Asus", "New codec from Asus. Supposedly a very simple DCT codec. Complete technical details can be found here." },
    { "AUR2", "Aura 2 Codec - YUV 422", "Auravision", "Unusual codec which stores audio in the .avi file but puts the video in a companion .asv file. Click here for information on how to play back these files on Windows 2000 PCs." },
    { "AURA", "Aura 1 Codec - YUV 411", "Auravision", "?" },
    { "avc1", "H.264", "Apple", "?" },
    { "AVRn", "Avid M-JPEG", "Avid Technology", "Apple's version of the MPEG4 part 10/H.264 standard apparently." },
    { "BA81", "Raw 8bit RGB Bayer", "?", "Also known as ADVJ in Quicktime files." },
    { "BINK", "Bink Video", "RAD Game Tools", "This format stores raw 8bit Bayer samples. The Bayer color filter array is described here. I wonder if this is a duplicate of BYR1?" },
    { "BLZ0", "?", "Blizzard", "Pretty popular codec in Windows games. I'm not sure if this is available as a standard Windows codec but the web site makes it sound interesting enough to list here even if it does require non-standard tools to use." },
    { "BT20", "Prosumer Video", "Conexant", "MPEG-4 codec used in WarCraft 3 movies." },
    { "BTCV", "Composite Video Codec", "Conexant", "Codec optimised for realtime compression of YUV images. Download the ZIP and add VIDC.BT20=btvvc32.drv and VIDC.Y41P=btvvc32.drv to the [drivers32] section of your SYSTEM.INI to enable the codec" },
    { "BW10", "Broadway MPEG Capture/Compression", "Data Translation", "This, now obsolete, format supported a special data format used by the Brooktree Bt2115 chipset which could perform \"software encoded video output\" - a kind of software TV-out capability." },
    { "BYR1", "Raw 8bit RGB Bayer", "?", "Codec for Broadway hardware-based MPEG compression system." },
    { "BYR2", "Raw 16bit RGB Bayer", "?", "This format stores raw 8bit Bayer samples. The Bayer color filter array is described here." },
    { "CC12", "YUV12 Codec", "Intel", "This format stores packed 16bit Bayer samples with 12bit precision. The Bayer color filter array is described here." },
    { "CDVC", "Canopus DV Codec", "Canopus", "?" },
    { "CFCC", "DPS Perception", "Digital Processing Systems", "Allegedly used with digital video cameras. The Canopus download page has a free software-only version of this codec which will install on machines which are not equipped with their capture hardware. Apparently if you edit the AVI and change the FOURCC to dvsd, it will play with these codecs too." },
    { "CGDI", "Camcorder Video", "Microsoft", "Native format used when capturing AVIs using a DPS Perception adapter." },
    { "CHAM", "Caviara Champagne", "Winnov", "AVI format used by Office 97 camcorder application." },
    { "CMYK", "Uncompressed CMYK", "Colorgraph", "?" },
    { "CJPG", "WebCam JPEG", "Creative Labs", "Uncompressed 32bpp CMYK as used in printing processes." },
    { "CPLA", "YUV 4:2:0", "Weitek", "Used by Creative Video Blaster Webcam Go control. See here for info on how to download and install this codec." },
    { "CRAM", "Microsoft Video 1", "Microsoft", "This sounds like an uncompressed format to me. Anyone know?" },
    { "CSCD", "CamStudio Codec", "RenderSoft Software", "Allegedly identical to MSVC." },
    { "CTRX", "Citrix Scalable Video Codec", "Citrix?", "Open source (GPL license) codec optimised for screen capture applications. Source download is available." },
    { "CVID", "Cinepak", "Providenza &amp; Boekelheide", "?" },
    { "CWLT", "Color WLT DIB", "Microsoft", "Originally owned by Supermac then Radius, now P &amp; B. Complete technical details can be found here." },
    { "CXY1", "Conexant YUV 4:1:1", "Conexant", "Apparently WLT is \"with lookup table\". Presumably, therefore, this is similar to a standard DIB using FOURCC 0?" },
    { "CXY2", "Conexant YUV 4:2:2", "Conexant", "Uncompressed, planar 4:1:1 YUV format." },
    { "CYUV", "Creative YUV", "Creative Labs", "Uncompressed, planar 4:2:2 YUV format." },
    { "CYUY", "?", "ATI Technologies", "Proprietary YUV compression algorithm" },
    { "D261", "H.261", "DEC", "Proprietary YUV compression algorithm" },
    { "D263", "H.263", "DEC", "Presumably now owned by Intel." },
    { "davc", "MPEG-4/H.264", "Dicas", "Presumably now owned by Intel." },
    { "DCL1", "Data Connection Conferencing Codec", "Data Connection Ltd.", "H.264/MPEG-4 AVC base profile codec. Dicas tell me that this codec will be available for free download from their site some time in June 2004. I am waiting to hear the exact URL for the download." },
    { "DCL2", "Data Connection Multimedia Conferencing Codec", "Data Connection Ltd.", "Format used in Data Connection Ltd's conferencing services." },
    { "DCL3", "Data Connection Enhanced Conferencing Codec", "Data Connection Ltd.", "Format used in Data Connection Ltd's conferencing services." },
    { "DCL4", "Data Connection Extended Conferencing Codec", "Data Connection Ltd.", "Format used in Data Connection Ltd's conferencing services." },
    { "DCL5", "Data Connection Media Conferencing Codec", "Data Connection Ltd.", "Format used in Data Connection Ltd's conferencing services." },
    { "DIV3", "DivX MPEG-4", "DivX", "Format used in Data Connection Ltd's conferencing services." },
    { "DIV4", "DivX MPEG-4", "DivX", "Low motion codec (optimised for low motion source material?). Several sources tell me that this is an old and illegal codec that should not be used to encode new material." },
    { "DIV5", "?", "?", "Fast motion codec.Several sources tell me that this is an old and illegal codec that should not be used to encode new material." },
    { "DIVX", "DivX", "OpenDivX", "Apparently almost as old as DIV3 and DIV4. Changing DIV5 AVI's FOURCC to DIV3 or DIV4 seems to allow them to play just fine." },
    { "divx", "DivX", "?", "This FOURCC code is used for versions 4.0 and later of the DivX codec. DivX, \"the MP3 of video,\" is the popular and market-leading MPEG-4 video codec that is emerging as the standard for full screen, full motion, DVD-quality video over IP-based networks. Apparently version 5 also encodes using FOURCC DX50. The Microsoft codec pack available here apparently supports Divx 6. This FOURCC is supported by LEAD's MCMP Codec." },
    { "DM4V", "MPEG-4", "Dicas", "Apparently used interchangeably with \"DIVX\". This FOURCC is supported by LEAD's MCMP Codec." },
    { "dmb1", "Rainbow Runner hardware compression", "Matrox", "MPEG4 codec compatible with DivX4 and 5." },
    { "DMB2", "?", "?", "Hardware codec used by Matrox Rainbow Runner video capture product. Apparently a form of Motion JPEG). LEAD's MCMP codec supports this format." },
    { "DMK2", "?", "?", "MJPEG codec used by Paradigm." },
    { "DSVD", "DV Codec", "?", "Movies generated by a ViewSonic V36 PDA appear to be AVI files using this video codec. Oddly enough, they are named .mpg - go figure." },
    { "DUCK", "TrueMotion S", "Duck Corporation", "The DSVD codec is a VFW-based compressor that firewire- and DV-based capture cards use. It is supported by Graphedit (from divx-digest.com), Adobe Premier and StudioDV. VirtualDub doesn't support the format. I suspect that this and DVSD are one and the same - perhaps one of my informants typed the FOURCC wrongly?" },
    { "dv25", "DVCPRO", "Matrox", "Rather nice RGB codec which, strangely enough, appears to have two distinct FOURCCs." },
    { "dv50", "DVCPRO50", "Matrox", "SMPTE 314M 25Mb/s compressed video. A professional variant of DVC (dvsd). Unlike dvsd, it uses 4:1:1 sampling structure for both NTSC and PAL." },
    { "DVAN", "?", "?", "SMPTE 314M 50Mb/s compressed video. Has twice the data rate (50Mbits/sec) of dv25 and uses 4:2:2 sampling structure." },
    { "DVCS", "DV Video", "Microsoft?", "?" },
    { "DVE2", "DVE-2 Videoconferencing Codec", "InSoft", "A generic DV codec along the same lines as DVSD. Microsoft indicates here that this codec should be considered obsolete now that standard FOURCCs for the various DV flavours have been defined." },
    { "dvh1", "SMPTE 370M", "Panasonic?", "?" },
    { "dvhd", "50Mbps Consumer DV", "Microsoft?", "SMPTE 370M - data structure for DV based audio, data and compressed video at 100 Mb/s - 1080/60i, 1080/50i, and 720/60p. This is basically a high definition variant of dv25 and dv50." },
    { "dvsd", "25Mbps Consumer DV", "Pinnacle Systems?", "SD-DVCR 1125-60 or SD-DVCR 1250-50. See also dvsl and dvsd." },
    { "DVSD", "DV Video", "Microsoft?", "SD-DVCR 525-60 or SD-DVCR 625-50. See also dvsl and dvhd. Implemented in the miroVideo DV300 SW only codec which requires the presence of the 1394 (Firewire) interface card with which it shipped." },
    { "dvsl", "12.5Mbps Consumer DV", "Microsoft?", "Used by Adobe After Effects, Uleads Mediastudio 6 (and probably VideoStudio) as a generic DV FOURCC. Probably the same as \"dvsd\". Sources indicate that these FOURCCs were used interchangeably in early versions. Microsoft indicates here that this codec should be considered obsolete now that standard FOURCCs for the various DV flavours have been defined." },
    { "DVX1", "DVX1000SP Video Decoder", "Lucent", "SD-DVCR 525-60 or SD-DVCR 625-50. See also dvsd and dvhd." },
    { "DVX2", "DVX2000S Video Decoder", "Lucent", "?" },
    { "DVX3", "DVX3000S Video Decoder", "Lucent", "?" },
    { "DX50", "DivX MPEG-4 version 5", "DivX", "?" },
    { "DXGM", "?", "Electronic Arts?", "Apparently this is used interchangeably with the DIVX FOURCC when using version 5 of the codec. An alternative download site can be found here." },
    { "DXTn", "DirectX Compressed Texture", "Microsoft", "The movies in the game \"Lord of the Rings: Return of the King\" are encoded in this format. Also used in movies from \"Robin Hood\" by CinemaWare." },
    { "DXTC", "DirectX Texture Compression", "Microsoft", "5 different versions (DXT1 - DXT5) of compressed texture formats exist. Full documentation is to be found in the DirectX SDK. More info can also be found on S3's Texture Compression web site." },
    { "ELK0", "?", "Elsa", "Another of the DXTn set, I suppose." },
    { "EKQ0", "Elsa Quick Codec", "Elsa", "Codec used by some Elsa graphics cards. May be a YUV format with reduced colour resolution." },
    { "EM2V", "Etymonix MPEG-2 Video", "Etymonix", "?" },
    { "ES07", "Eyestream 7 Codec", "Eyeball Chat", "HIgh quality, MPEG-2 I picture codec with user selectable YUV 4:2:0, 4:2:2 or 4:4:4 compression. Suitable for use in video editing applications. Free trial version available." },
    { "ESCP", "Escape", "Eidos Technologies", "" },
    { "ETV1", "eTreppid Video Codec", "eTreppid Technologies", "Codec used by Eidos Technologies ESCAPE VideoStudio." },
    { "ETV2", "eTreppid Video Codec", "eTreppid Technologies", "?" },
    { "ETVC", "eTreppid Video Codec", "eTreppid Technologies", "?" },
    { "FFV1", "FFMPEG Codec", "Michael Niedermayer", "?" },
    { "FLJP", "Field Encoded Motion JPEG", "D-Vision", "A lossless video codec based on arithmetic coding developed in the open source ffmpeg project." },
    { "FMP4", "FFMpeg", "?", "Field encoded motion JPEG with LSI bitstream format. Morgan Multimedia offers a codec supporting this format." },
    { "FMVC", "FM Screen Capture Codec", "Fox Magic Software?", "The default MPEG4 format used by tool mencoder. DirectShow filters supporting the codec are available here." },
    { "FPS1", "Fraps Codec", "Fraps", "A codec intended for use in screen capture applications." },
    { "FRWA", "Forward Motion JPEG with alpha channel", "SoftLab-Nsk", "Codec used by Fraps screen video capture application." },
    { "FRWD", "Forward Motion JPEG", "SoftLab-Nsk", "A version of motion JPEG as used in the Forward project from SoftLab-Nsk. This format also includes an 8-bit alpha channel per image." },
    { "FVF1", "Fractal Video Frame", "Iterated Systems", "A version of motion JPEG as used in the Forward project from SoftLab-Nsk. Similar to FRWD but without the alpha information." },
    { "GEOX", "GEOMPEG4", "GeoVision", "" },
    { "GJPG", "GT891x Codec", "Grand Tech", "MPEG shipped as part of GeoVision's CCTV surveillance systems." },
    { "GLZW", "Motion LZW", "gabest@freemail.hu", "Shipped as part of the driver package with some dgital cameras from Fuji." },
    { "GPEG", "Motion JPEG", "gabest@freemail.hu", "GIF-like codec written by gabest@freemail.hu." },
    { "GWLT", "Greyscale WLT DIB", "Microsoft", "Motion JPEG codec written as a learning exercise by gabest@freemail.hu." },
    { "H260-9", "ITU H.26n", "Intel", "8bpp greyscale image. WLT apparently means \"with lookup table\" so it is a palettized format." },
    { "HDYC", "Raw YUV 4:2:2", "?", "Conferencing codecs H.263 format video is for POTS-based videoconferencing. Used, for example, in some Osprey products. Supposedly, the Vanguard Software H.264 codec, available in beta form here, is pretty good." },
    { "HFYU", "Huffman Lossless Codec", "?", "This is apparently identical to UYVY except that the samples use BT709 color space (as used in HD video) rather than BT470 (used for SD)." },
    { "HMCR", "Rendition Motion Compensation Format", "Rendition", "Huffman codec for YUV and RGB formats. Available in source and DLL forms. Full technical information can be found here. I am told that further information can also be found here." },
    { "HMRR", "Rendition Motion Compensation Format", "Rendition", "Proprietary motion compensation surface format used by Rendition V2x00 DirectDraw driver." },
    { "i263", "ITU H.263", "Intel", "Newer, proprietary motion compensation surface format used by Rendition drivers." },
    { "IAN", "Indeo 4 Codec", "Intel", "PictureWorks NetCard Player - another H.263 implementation from Intel. There's a FAQ on this codec here and it can also be downloaded from here." },
    { "ICLB", "CellB Videoconferencing Codec", "InSoft", "One of a collection of FOURCCs used in Indeo 4." },
    { "IGOR", "Power DVD", "?", "?" },
    { "IJPG", "Intergraph JPEG", "Intergraph", "?" },
    { "ILVC", "Layered Video", "Intel", "Intergraph's version of a JPEG codec (don't you hate it when I just state the obvious ?)" },
    { "ILVR", "ITU H.263+ Codec", "?", "?" },
    { "IPDV", "Giga AVI DV Codec", "I-O Data Device, Inc.", "?" },
    { "IR21", "Indeo 2.1", "Intel", "Codec used with I-O Data Device's IEEE1394 Digital Video Control &amp; Capture Board. This codec implements I-O DATA's original indexed AVI architecture." },
    { "IRAW", "Intel Uncompressed UYUV", "Intel", "Old Indeo codec" },
    { "ISME", "?", "?", "No indication of the pixel format - sorry." },
    { "IV30-9", "Indeo 3", "Ligos", "May be installed by Roxio DVD Creator 6?" },
    { "IV32", "Indeo 3.2", "Ligos", "The family of Indeo Video 3 codecs originally developed by Intel but now handled by Ligos." },
    { "IV40-9", "Indeo Interactive", "Ligos", "Fairly widespread Indeo 3 codec" },
    { "IV50", "Indeo Interactive", "Ligos", "Indeo 4.1 improves image quality and introduces transparency masks. Ligos also offer a download here but apparently it will cost you." },
    { "JBYR", "?", "Kensington?", "Version 5.0 of the Indeo codec series designed for internet video delivery. Ligos also offer a download here but apparently it will cost you." },
    { "jpeg", "JPEG Still Image", "Microsoft?", "?" },
    { "JPEG", "JPEG Still Image", "Microsoft", "This is presumably exactly the same as \"JPEG\". LEAD's MCMP codec supports both these formats." },
    { "JPGL", "JPEG Light?", "?", "This is presumably exactly the same as \"jpeg\". LEAD's MCMP codec supports both these formats." },
    { "KMVC", "Karl Morton's Video Codec (presumably)", "?", "Proprietary format used by many WebCams which are built around the DIVIO NW 801/802 chip such as the Logitec QuickCam Pro, VideoLogic HomeC@m and other cameras from Askey, Mustek, Microtek, and Tekom. LEAD's MCMP codec supports this format." },
    { "L261", "LEAD H.261", "LEAD Technologies", "Shipped as part of the game \"Worms\" by Team17 Software. Info is allegedly obtainable from info@beamaim.demon.co.uk." },
    { "L263", "LEAD H.263", "LEAD Technologies", "?" },
    { "LBYR", "?", "?", "LEAD tell me that \"The LEAD H.263 codec is a high quality lossy interframe DirectShow and Video for Windows (VfW) codec with a quality and compression ratio comparable to MPEG-4. It can be used for a range of applications including video-conferencing, Internet video or surveillance and monitoring. Existing video software, such as Windows&reg; Media Player&reg; and Ulead Media Studio&reg; can utilize this codec to play, create, convert and edit.\"" },
    { "LCMW", "LEAD MCMW Video Codec", "LEAD Technologies", "Based on the naming, I would hazard a guess that this describes some Bayer image format and may have been registered by LEAD Technologies. Can anyone confirm or deny?" },
    { "LCW2", "LEAD MJPEG2000", "LEAD Technologies", "This is a wavelet-based codec. LEAD tell me \"The MCMW Codec is designed to create high-quality video using low bandwidth (dial-up) delivery. This is achieved by using a state of the art intraframe wavelet based compression technique that provides superior compression performance, while still ensuring optimal quality at any compression level.\"" },
    { "LEAD", "LEAD Video Codec", "LEAD Technologies", "Codec saving standard JPEG2000 streams. LEAD has more info about it here." },
    { "LGRY", "Grayscale Image", "LEAD Technologies", "LEAD tell me that this codec is now obsolete yet it seems that their \"MCMP Codec\" supports it so I guess it's not as obsolete as all that." },
    { "LJ11", "LEAD JPEG 4:1:1", "LEAD Technologies", "Supports 12 and 16bpp grayscale images with additional low and high range information required for medical images. Format public but not finalised when this update was made." },
    { "LJ22", "LEAD JPEG 4:2:2", "LEAD Technologies", "Presumably a JPEG codec which encodes YUV in 4:1:1 format. This is supported by LEAD's MCMP Codec." },
    { "LJ2K", "LEAD MJPEG 2000", "LEAD Technologies", "Presumably a JPEG codec which encodes YUV in 4:2:2 format. This is supported by LEAD's MCMP Codec." },
    { "LJ44", "LEAD JPEG 4:4:4", "LEAD Technologies", "LEAD tell me \"the LEAD MJPEG2000 Video codec saves standard JPEG2000 streams. This codec is perfect when high compression and interoperability are necessary.\"" },
    { "Ljpg", "LEAD MJPEG Codec", "LEAD Technologies", "Presumably a JPEG codec which encodes YUV in 4:4:4 format. This is supported by LEAD's MCMP Codec." },
    { "LMP2", "LEAD MPEG-2 Video Codec", "LEAD Technologies", "Supports color JPEG 4:1:1, 4:2:2, 4:4:4, grayscale JPEG 4:0:0) FOURCCs supported: MJPG, JPEG, dmb1 Lossless JPEG support including 24-bit color and 8,12 and 16-bit grayscale. Link offers download of a time-expiring version of the codec. This format is supported by the LEAD MCMP and MJPEG2000 codecs." },
    { "LMP4", "LEAD MPEG-4 Video Codec", "LEAD Technologies", "LEAD's implementation of the MPEG-2 video compression standard. The codec linked here also supports several other standard MPEG2 FOURCCs. Supports High, Main and Simple profiles, Low, Main, High1440 and High levels." },
    { "LSVC", "Lightning Strike Video Codec", "ESPRE Solutions", "An H.264 codec suporting Main and Base Profles and levels up to 3.2. LEAD offer two versions of this codec, a normal version and a professional version which offers more options and higher compression ratios." },
    { "LSVM", "Lightning Strike Video Codec", "ESPRE Solutions", "Old version of the codec now identified as LSVX." },
    { "LSVX", "Lightning Strike Video Codec", "ESPRE Solutions", "Another old version of the codec now identified as LSVX." },
    { "LZO1", "Lempel-Ziv-Oberhumer Codec", "Markus Oberhumer", "Multi-bitrate decoder used in the eViewStreamX product." },
    { "M263", "H.263", "Microsoft", "A fast, lossless codec available in source code format." },
    { "M261", "H.261", "Microsoft", "Redmond's codec implementing the H.263 compression standard." },
    { "M4CC", "MPEG-4", "Divio", "Redmond's codec implementing the H.261 compression standard." },
    { "m4cc", "MPEG-4", "Divio", "Redmond's codec implementing the H.261 compression standard." },
    { "M4S2", "MPEG-4 (automatic WMP download)", "Microsoft", "" },
    { "MC12", "Motion Compensation Format", "ATI Technologies", "Final fully-compliant ISO MPEG4 decoder, compliant to MPEG-4 version 2 simple profile. An alternative download site can be found here." },
    { "MCAM", "Motion Compensation Format", "ATI Technologies", "Proprietary format used by ATI in MPEG decoding." },
    { "MJ2C", "Motion JPEG 2000", "Morgan Multimedia", "Proprietary format used by ATI in MPEG decoding." },
    { "mJPG", "Motion JPEG including Huffman Tables", "IBM", "Motion JPEG 2000" },
    { "MJPG", "Motion JPEG", "?", "A version of Motion JPEG which includes Huffman tables with each AVI frame. Developed by IBM before the MJPEG standard was finalised." },
    { "MMES", "MPEG-2 ES", "Matrox", "Motion JPEG video. Codecs implementing MJPEG are (or have been) available from http://www.jpg.com/imagetech_video.htm http://www.pmatrix.com Pegasus Imaging Morgan Multimedia (long trial period) The format is also used by Matrox in their Rainbow Runner add-on video capture / MPEG playback board for various of their display adapters and by Canon who's cameras generate AVIs using this codec." },
    { "MP2A", "Eval download", "Media Excel", "Matrox MPEG-2 video elementary stream.MPEG-2 main profile or 4:2:2 profile closed GOP IBP or I-frame only decoder." },
    { "MP2T", "Eval download", "Media Excel", "MPEG-2 Audio" },
    { "MP2V", "Eval download", "Media Excel", "MPEG-2 Transport Stream" },
    { "MP42", "MPEG-4 (automatic WMP download)", "Microsoft", "MPEG-2 Video" },
    { "MP43", "MPEG-4 (automatic WMP download)", "Microsoft", "Apparently one of several different and incompatible MPEG-4 codecs. Rumour has it that this codec is downloadable from the Microsoft site somewhere. This codec is distributed as part of Microsoft Windows Media Tools 4. Includes quality improvements over the earlier MPG4. Download as part of \"Windows Media Codecs 8.0 for IT Professionals.\"" },
    { "MP4A", "Eval download", "Media Excel", "Yet another MPEG-4 variation from Microsoft. This FOURCC is not, however, listed on Microsoft's codecs site. This codec is distributed as part of Microsoft Windows Media Tools 4. Includes further quality improvements over the earlier MPG4." },
    { "MP4S", "MPEG-4 (automatic WMP download)", "Microsoft ?", "MPEG-4 Audio" },
    { "MP4T", "Eval download", "Media Excel", "The first ISO standard codec for use with the Sharp digital camera implementing a restricted feature set of MPEG4." },
    { "mp4v", "MPEG-4", "?", "MPEG-4 Transport Stream" },
    { "MP4V", "Eval download", "Media Excel", "Presumably a duplicate of MP4V. LEAD's MCMP codec supports this format and you can download an evaluation version here." },
    { "MPEG", "MPEG", "?", "MPEG-4 Video. LEAD's MCMP codec also supports this format." },
    { "MPG4", "MPEG-4 (automatic WMP download)", "Microsoft", "MPEG video - presumably MPEG I ?" },
    { "MPGI", "MPEG", "Sigma Designs", "MPEG-4 Video High Speed Compressor. Downloadable here, I am told. This codec was shipped with some versions of the Microsoft Netshow encoder (probably 3.0). This codec was based on early drafts of the MPEG-4 spec." },
    { "MR16", "?", "?", "Editable MPEG codec" },
    { "MRCA", "Mrcodec", "FAST Multimedia", "?" },
    { "MRLE", "Microsoft RLE", "Microsoft", "And I thought it stood for \"Multi Role Combat Aircraft\"." },
    { "MSVC", "Microsoft Video 1", "Microsoft", "Run length encoded RGB format from Microsoft. Basically the same as the BI_RLE formats but Michael Knapp clarifies: \"MRLE is just *nearly* the same compression as the existing 4 and 8bit RLE formats but the 'copy bytes-chunk' always has an even byte-length. That means that an empty byte is added if the 'copy chunk' contains an odd number of bytes\"" },
    { "MSZH", "AVImszh", "Kenji Oshima", "Original codec shipped with Video For Windows. Deals with 8bpp and 16bpp images. Quality leaves a lot to be desired (IMHO). Full technical details are available here." },
    { "MTX1-9", "?", "Matrox", "Kenji Oshima also developed a multi-threaded M3JPEG codec based on one by Morgan Multimedia. You can download this here. Algorithm information can be found here. This is supposedly the same thing as ZLIB." },
    { "MVI1", "Motion Pixels MVI1 Codec", "?", "Apparently these are MJPG variations registered by Matrox consumer products group." },
    { "MVI2", "Motion Pixels MVI2 Codec", "?", "Part of the Motion Pixels player. Install the player and the codec will become available to other multimedia applications. Read more at this site (look for Treasure Quest)." },
    { "MWV1", "Aware Motion Wavelets", "Aware Inc.", "As for MVI1, this is part of the Motion Pixels player. Read more at this site." },
    { "nAVI", "Download here", "?", "Wavelet compression-based codec optimised for Intel MMX platforms. Allegedly downloadable from here. Codec filename is icmw_32.dll." },
    { "NDSC", "Nero Digital Cinema", "Nero Digital", "?" },
    { "ndsm", "Nero MPEG4?", "Nero Digital", "Nero registered this with me recently but provided no information other than the name - sorry." },
    { "ndsp", "Nero Digital Portable", "Nero Digital", "A contributor suggests that this codec is owned by Nero Digital and refer sto their flavour of MPEG4 but I have not been able to confirm this." },
    { "NDSP", "Nero Digital Portable", "Nero Digital", "Nero have registered NDSP so my assumption is that they get the lower case version for free." },
    { "ndss", "Nero Digital Standard", "Nero Digital", "Apparently an MPEG4 implementation of some description." },
    { "NDSS", "Nero Digital Standard", "Nero Digital", "A contributor suggests that this codec is owned by Nero Digital and refers to their flavour of MPEG4 but I have not been able to confirm this." },
    { "NDXC", "Nero Digital AVC Cinema", "Nero Digital", "Presumably the same as ndss." },
    { "NDXH", "Nero Digital AVC HDTV", "Nero Digital", "Presumably a variation of MPEG4 part 10/AVC." },
    { "NDXP", "Nero Digital AVC Portable", "Nero Digital", "Presumably a variation of MPEG4 part 10/AVC." },
    { "NDXS", "Nero Digital AVC Standard", "Nero Digital", "Presumably a variation of MPEG4 part 10/AVC." },
    { "NTN1", "Video Compression 1", "Nogatech/Zoran", "Presumably a variation of MPEG4 part 10/AVC." },
    { "NTN2", "Video Compression 2", "Nogatech/Zoran", "?" },
    { "NVDS", "NVidia Texture Format", "NVidia", "An evolution of NTN1." },
    { "NVHS", "NVidia Texture Format", "NVidia", "?" },
    { "NHVU", "NVidia Texture Format", "NVidia", "Apparently a texture format introduced for GEForce 3." },
    { "NVS0-5", "?", "NVidia?", "Apparently a texture format introduced for GEForce 3." },
    { "NVT0-5", "?", "NVidia?", "Supported by GEForce 2 GTS Pro / 64Mb DDR. Possibly a texture format." },
    { "PDVC", "DVC codec", "I-O Data Device, Inc.", "Supported by GEForce 2 GTS Pro / 64Mb DDR. Possibly a texture format." },
    { "PGVV", "Radius Video Vision", "Radius", "DV codec for I-O DATA Digital Video Capture products." },
    { "PHMO", "Photomotion", "IBM", "?" },
    { "PIM1", "Download here", "Pinnacle Systems", "?" },
    { "PIM2", "?", "Pinnacle Systems", "MPEG-1 based codec" },
    { "pimj", "Pegasus Lossless JPEG", "Pegasus Imaging", "Pinnacle DC1000 firewire video editing card supports this format." },
    { "PIXL", "Video XL", "Pinnacle Systems", "High speed compression and decompression of 24-bit RGB and 8-bit grayscale using Predictor 1 Lossless JPEG. Well suited for medical video. You may find additional information here." },
    { "PJPG", "PA MJPEG", "Pierre Albou", "This is apparently an alias for VIXL. You can find more information about it here." },
    { "PVEZ", "PowerEZ", "Horizons Technology", "Non-standard codec based on MJPEG." },
    { "PVMM", "PacketVideo Corporation MPEG-4", "PacketVideo Corporation", "TrueMotion based codec (?) It appears that Horizons Technology has now been acquired by Raytheon and is no longer in the codec business (can someone confirm this - their URL is now redirected). Allegedly, you can download this codec from support.private.com but I'm not going to add a link since this is an adult video company and I don't want to get this site associated with porn." },
    { "PVW2", "Pegasus Wavelet 2000 Compression", "Pegasus Imaging", "Software MPEG4 codec that supports multiple bitrate encoding/decoding. It is also error resilient allowing transmission over wired/wireless networks." },
    { "qpeq", "QPEG 1.1", "Q-Team", "High speed compression and decompression of 24-bit RGB and 8-bit grayscale using Pegasus' proprietary Wavelet2000 wavelet technology. Well suited for low motion, low bandwidth applications. More information is available here." },
    { "QPEG", "QPEG", "Q-Team", "?" },
    { "raw", "Raw RGB", "?", "Q-Team Dr.Knabe's 8-bit output codec with automatic palette switching for seamless edits." },
    { "RGBT", "32 bit support", "Computer Concepts", "Apparently this contains \"raw, uncompressed RGB bitmaps\"." },
    { "rle", "Apple Animation", "Apple", "That's odd. I registered RGBT at the same time as a bunch of other FOURCCs and it was granted. The format I registered is for 16- and 32-bit uncompressed RBG images with a transparency plane. Microsoft's codec site, however, lists Computer Concepts rather than Brooktree (Conexant) as the owner. Hmm..." },
    { "RLE", "Run Length Encoder", "Microsoft?", "Yes this really is lower case. This format is used to compress Quicktime files. Available in 1, 2, 4, 8, 16, 24 and 32bit flavors." },
    { "RLE4", "4bpp Run Length Encoder", "Microsoft", "I expect this is an equivalent to one of the the BI_RLEx FOURCCs (see the RGB page). The Win2K clock.avi sample uses this format. Final character in the FOURCC is a space." },
    { "RLE8", "8bpp Run Length Encoder", "Microsoft", "Equivalent to BI_RLE4. See RGB page for more details." },
    { "RMP4", "MPEG-4 AS Profile Codec", "Sigma Designs", "Equivalent to BI_RLE8. See RGB page for more details." },
    { "RPZA", "Apple Video", "Apple", "Press release here describes this codec." },
    { "RT21", "Real Time Video 2.1", "Intel", "RGB555 block-based codec used in Quicktime files." },
    { "rv20", "RealVideo G2", "Real", "What Indeo was called before the marketing guys got their hands on it. RTV or Real Time Video was the format produced by Intel's ActionMedia II adapter back in the late 80s. When the 80486 came along, this migrated to the software-decodable Indeo formats used today." },
    { "rv30", "RealVideo 8", "Real", "RealVideo G2 (6.0 and greater versions of the player and encoder)" },
    { "RV40", "RealVideo 10?", "Real", "" },
    { "RVX", "RDX", "Intel", "Apparently you need to install RealPlayer 10 to enable playback of any AVI encoded with RV40." },
    { "s422", "VideoCap C210 YUV Codec", "Tekram International", "?" },
    { "SAN3", "DivX 3", "?", "YUV422 codec shipped as part of the driver package for Tekram's C210 product." },
    { "SDCC", "Digital Camera Codec", "Sun Communications", "A direct copy of DivX 3.11a, apparently. If you use a FOURCC changer tool on these AVIs they will play with the standard DivX codecs." },
    { "SEDG", "Samsung MPEG-4", "Samsung", "?" },
    { "SFMC", "Surface Fitting Method", "CrystalNet", "MPEG-4 hardware and software codec used in Samsung digital video products." },
    { "SMC", "Apple Graphics", "Apple", "CrystalGram video email codec." },
    { "SMP4", "?", "?", "8-bit, block-based codec used in Quicktime files." },
    { "SMSC", "Proprietary codec", "Radius", "Codec used by Samsung VP-ms15 Digicam. This appears to be a DIVX clone since apparently SMP4 files play fine if you change the FOURCC to DIVX." },
    { "SMSD", "Proprietary codec", "Radius", "?" },
    { "smsv", "Wavelet Video", "WorldConnect (corporate site)", "?" },
    { "SP40", "?", "SunPlus", "Windows 95 codec installed automatically (and without warning) whenever you receive and play back a file sent from VisualMail. Very low bandwidth format." },
    { "SP44", "?", "SunPlus", "Appears to be an uncompressed YUV format of some kind but I have no information on this other than that." },
    { "SP54", "?", "SunPlus", "Presumably a precursor to SP54?" },
    { "SPIG", "Spigot", "Radius", "Apparently a form of MJPEG but with some header or other missing. Software shipped with a number of low end digital cameras and webcams such as Aiptek's Pocketcam digital still camera, Logitech's ClickSmart and Mustek's gSmart mini 2 use this format. These use SunPlus chipsets so presumably this explains the \"SP\". You can download a tool that will convert this to MJPEG here." },
    { "SQZ2", "VXTreme Video Codec V2", "Microsoft", "?" },
    { "SV10", "Video R1", "Sorenson Media", "?" },
    { "STVA", "ST CMOS Imager Data (Bayer)", "ST Microelectronics", "Allegedly popular as a Quicktime codec. Used for trailer videos on Star Wars Episode 1 and other games." },
    { "STVB", "ST CMOS Imager Data (Nudged Bayer)", "ST Microelectronics", "Data from ST CMOS Imagers that is passed to a codec external to the driver for processing to a displayable format. More info may be available at the ST Vision and Imaging Unit web site." },
    { "STVC", "ST CMOS Imager Data (Bunched)", "ST Microelectronics", "Data from ST CMOS Imagers that is passed to a codec external to the driver for processing to a displayable format. More info may be available at the ST Vision and Imaging Unit web site." },
    { "STVX", "ST CMOS Imager Data (Extended CODEC Data Format)", "ST Microelectronics", "Data from ST CMOS Imagers that is passed to a codec external to the driver for processing to a displayable format. More info may be available at the ST Vision and Imaging Unit web site." },
    { "STVY", "ST CMOS Imager Data (Extended CODEC Data Format with Correction Data)", "ST Microelectronics", "Data from ST CMOS Imagers that is passed to a codec external to the driver for processing to a displayable format. More info may be available at the ST Vision and Imaging Unit web site." },
    { "SVQ1", "Sorenson Video 1", "Sorenson Media", "Data from ST CMOS Imagers that is passed to a codec external to the driver for processing to a displayable format. More info may be available at the ST Vision and Imaging Unit web site." },
    { "SVQ3", "Sorenson Video 3", "Sorenson Media", "Hierarchicial adaptive multistage vector quantizer with mean removal and interframe motion compensation. Used in Quicktime. Complete technical details can be found here." },
    { "TLMS", "Motion Intraframe Codec", "TeraLogic", "Video codec used in Quicktime files. A variant of H.264" },
    { "TLST", "Motion Intraframe Codec", "TeraLogic", "?" },
    { "TM20", "TrueMotion 2.0", "Duck Corporation", "?" },
    { "TM2X", "TrueMotion 2X", "Duck Corporation", "Version 2.0 of Duck Corp's Truemotion codec. I'm told this codec is available as part of a pack that can be downloaded here." },
    { "TMIC", "Motion Intraframe Codec", "TeraLogic", "Duck Corp's follow-on codec after TM20." },
    { "TMOT", "TrueMotion S", "Horizons Technology", "?" },
    { "TR20", "TrueMotion RT 2.0", "Duck Corporation", "Another FOURCC for TrueMotion S. This relates to the version of the codec licensed by Horizons Technology and is not compatible with DUCK." },
    { "TSCC", "TechSmith Screen Capture Codec", "Techsmith Corp.", "Realtime version of TrueMotion." },
    { "TV10", "Tecomac Low-Bit Rate Codec", "Tecomac, Inc.", "Codec used by the Camtasia Screen \"camcorder\" application." },
    { "TVJP", "?", "Pinnacle/Truevision", "?" },
    { "TVMJ", "?", "Pinnacle/Truevision", "Used by the Targa 2000 board." },
    { "TY2C", "Trident Decompression Driver", "Trident Microsystems", "Used by the Targa 2000 board. Morgan Multimedia offers a codec supporting this format." },
    { "TY2N", "?", "Trident Microsystems", "?" },
    { "TY0N", "?", "Trident Microsystems", "?" },
    { "UCOD", "ClearVideo", "eMajix.com", "?" },
    { "ULTI", "Ultimotion", "IBM Corp.", "Fractal compression-based video codec available as a Video for Windows codec and in the ClearFusion Netscape plugin package." },
    { "v210", "10-bit 4:2:2 Component YCbCr", "AJA Video Systems", "Shipped with OS/2 but also available for Video for Windows. Link is to a very old version of the codec for Windows." },
    { "V261", "Lucent VX2000S", "Lucent", "Uncompressed format supported by AJA Video Systems Xena adapter." },
    { "V655", "YUV 4:2:2", "Vitec Multimedia", "?" },
    { "VCR1", "ATI Video Codec 1", "ATI Technologies", "Component ordering and packing unknown. Can you help?" },
    { "VCR2", "ATI Video Codec 2", "ATI Technologies", "Codec used by some ATI TV-PC products." },
    { "VCR3-9", "ATI Video Codecs", "ATI Technologies", "Codec used by some ATI TV-PC products." },
    { "VDCT", "VideoMaker Pro DIB", "Vitec Multimedia", "Registered for ATI Video Codecs version 3-9. I'm not sure these actually exist but the registrations are valid." },
    { "VDOM", "VDOWave", "VDONet", "16bpp format - no information on colour space, packing or component ordering." },
    { "VDOW", "VDOLive", "VDONet", "Another streaming video format from VDONet." },
    { "VDTZ", "VideoTizer YUV Codec", "Darim Vision Co.", "H.263 internet streaming video format. Allegedly to be used (being used ?) by Microsoft in it's NetShow offering." },
    { "VGPX", "VideoGramPix", "Alaris", "Codec used to store YUV AVIs captured with Darim Vision's VideoTizer product." },
    { "VIFP", "VFAPI Codec", "?", "Alaris VGPixel 32-bit AVI compression driver. It seems that this codec is installed along with the software for the \"Alaris Wee Cam.\" The codec doesn't appear to be available separately but, allegedly, works fine if you install the Wee Cam software even without the camera. You may also find some information at http://www.alaris.com/vg_tech/vg_tchtx.htm and http://www.videogram.com." },
    { "VIDS", "?", "Vitec Multimedia", "Take a look at http://www.doom9.org where you may find more information." },
    { "VIVO", "Vivo H.263", "Vivo Software", "YUV 4:2:2 CCIR 601 for V422 (no, I don't understand this either)" },
    { "VIXL", "Video XL", "Miro (now part of Pinnacle Systems)", "Vivo's version of the videoconferencing \"standard\" H.263 compression format (version 2.0.0)." },
    { "VLV1", "?", "VideoLogic", "Used my MiroVideo products such as the DC10, DC20, DC30, etc. A motion JPEG format that is accelerated in hardware with the Zoran chipset. You can find more information about it here." },
    { "VP30", "VP3", "On2", "Codec probably used in VideoLogic's Captivator product line" },
    { "VP31", "VP31", "On2", "On2 tell me \"On2's VP3 codec will encode video into a VP3 file in multiple bit rates (roughly 220 Kbps, 330 Kbps, and 440 Kbps) and at optimum frame rates (usually 29.97 fps). This multiple bit rate file allows the TrueCast server to scale dynamically and smoothly as bandwidth congestion increases and decreases, providing the viewer with a consistent and reliable experience, without choppiness or interruption.\"" },
    { "VP40", "VP40", "On2", "The successor to VP30. This algorithm was open sourced by On2 in 2001-2002 and is the basis for the Theora Video Codec. Technical details are available here." },
    { "VP50", "VP50", "On2", "Another in On2/Duck's line of video codecs." },
    { "VP60", "VP60", "On2", "..and another" },
    { "VP61", "VP61", "On2", "..and another" },
    { "VP62", "VP62", "On2", "..I can feel a pattern developing." },
    { "VQC1", "VideoQuest Codec 1", "ViewQuest", "..I wonder which one will be next?" },
    { "VQC2", "VideoQuest Codec 2", "ViewQuest", "Digital video camera codec. ViewQuest offer lots of driver downloads on their site so you may find this in one of those packages." },
    { "VQJC", "?", "?", "Codec apparently used in Kodak DVC325 digital camera. Check the ViewQuest site download page - you may find the codec there somewhere." },
    { "vssv", "VSS Video", "Vanguard Software Solutions", "?" },
    { "VUUU", "?", "?", "Real-time or near-real-time encoding with high compression ratios and good image quality." },
    { "VX1K", "VX1000S Video Codec", "Lucent", "?" },
    { "VX2K", "VX2000S Video Codec", "Lucent", "?" },
    { "VXSP", "VX1000SP Video Codec", "Lucent", "?" },
    { "VYU9", "ATI YUV", "ATI Technologies", "?" },
    { "VYUY", "ATI YUV", "ATI Technologies", "Planar YUV format supported by some ATI capture systems?" },
    { "WBVC", "W9960", "Winbond Electronics", "Packed YUV format supported by some ATI capture systems?" },
    { "WHAM", "Microsoft Video 1", "Microsoft", "?" },
    { "WINX", "Winnov Software Compression", "Winnov", "Yet another FOURCC describing Microsoft's MSVC/CRAM codec." },
    { "WJPG", "Winbond JPEG", "?", "Software codec used by some Winnov Videum products." },
    { "WMV1", "Windows Media Video 7", "Microsoft", "Format supported by AverMedia USB TV-tuner/capture device." },
    { "WMV2", "Windows Media Video 8", "Microsoft", "" },
    { "WMV3", "Windows Media Video 9", "Microsoft", "" },
    { "WMVA", "Windows Media Video 9 Advanced Profile", "Microsoft", "You may find other useful information here." },
    { "WNV1", "Winnov Hardware Compression", "Winnov", "The codec originally submitted for consideration as SMPTE VC1. This is not VC1 compliant and is no longer supported by Microsoft." },
    { "WVC1", "SMPTE VC1", "Microsoft", "Hardware codec used by Winnov Videum products." },
    { "x263", "Download here", "Xirlink", "Microsoft's implementation of the SMPTE VC1 codec." },
    { "X264", "H.264", "?", "Another H.263 codec. This one is apparently used by an IBM-branded webcam." },
    { "XVID", "XVID MPEG-4", "XVID", "This FOURCC was originally registered by a company called XiWave but their web presence has disappeared. I am now informed that it is used by a popular open source H.264 implementation." },
    { "XLV0", "XL Video Decoder", "NetXL Inc.", "Codec is available in source form from XVID web site. Can also be downloaded as part of the Gordian Knot Codec Pack. This FOURCC is also supported by LEAD's MCMP Codec." },
    { "XMPG", "XING MPEG", "XING Corporation", "?" },
    { "XWV0-9", "XiWave Video Codec", "XiWave", "Editable (I frame only) MPEG codec" },
    { "XXAN", "?", "Origin?", "XWV3 is currently used to describe Xi-3 Video. Others are unused." },
    { "Y16", "16bpp Grayscale Video", "Thermoteknix Systems Ltd.", "Codec useing Huffman and RLE encoding paired with basic interframing. This format is used in Wing Commander 3 and 4 movies. The codec filename is xanlib.dll and a player, xanmovie, is available on the the Kilrathi Saga and Crusader game CDs." },
    { "Y411", "YUV 4:1:1", "Microsoft", "A simple, uncompressed format for recording 16bpp grayscale images." },
    { "Y41P", "Brooktree YUV 4:1:1", "Conexant", "Supposedly 16bpp packed but 4:1:1 is usually 12bpp - odd. This is an uncompressed YUV format." },
    { "Y444", "?", "?", "This is an uncompressed YUV 411 format I registered about 7 years ago. I've stumbled on a few AVIs that use it, though, so I am listing it here. Download the ZIP and add VIDC.BT20=btvvc32.drv and VIDC.Y41P=btvvc32.drv to the [drivers32] section of your SYSTEM.INI to enable the codec" },
    { "Y8", "Grayscale video", "?", "Format provided by the Windows 2000 drivers for the iRez Stealth Fire camera. Seems to be a copy of IYU2." },
    { "YC12", "YUV 12 codec", "Intel", "Probably a duplicate of the uncompressed Y800 format. The 2 last characters are spaces. See also GREY which appears to be another duplicate" },
    { "YUV8", "Caviar YUV8", "Winnov", "?" },
    { "YUV9", "YUV9 Raw Format", "Intel", "?" },
    { "YUVP", "?", "?", "An uncompressed YUV format used by Intel Indeo video products." },
    { "YUY2", "Raw, uncompressed YUV 4:2:2", "Microsoft (probably)", "An uncompressed YCrCb 4:2:2 format using 10-bit precision components ordered Y0 U0 Y1 V0. I have no idea how the 10 bit samples are packed - sorry." },
    { "YUYV", "?", "Canopus", "Yes, I know this isn't a compressed format but I get so many questions about where to find a codec for YUY2 AVIs that it's here to allow people to find the answer easily. I'm told \"VirtualDub has been able to decode YUY2 internally since V1.3a. Newer versions of Ben Rudiak-Gould's Huffyuv codec will also convert YUY2 or UYVY data to RGB\"" },
    { "YV12", "YUV 4:2:0 Planar", "Microsoft or Apple?", "Compressed YUV format. Some of the software on the Canopus download page may include this codec (I'm guessing - please tell me if this is true)." },
    { "YV16", "YUV 4:2:2 Planar", "Elecard", "Uncompressed format commonly used in MPEG video processing. You can find more informatio on the YUV formats page." },
    { "YV92", "?", "Intel", "Uncompressed format similar to YV12 but with twice the chroma resolution." },
    { "ZLIB", "?", "?", "Codec used by Intel's Smart Video Recorder product. Apparently a compresssed YVU9 format." },
    { "ZMBV", "The DoxBox Project", "DoxBox Capture Codec", "A generic lossless codec that you can download from here. Apparently also contains the MSZH codec. Algorithm info can be found here. May be the same algorithm used in compressing PNG images." },
    { "ZPEG", "Video Zipper", "Metheus", "A codec using ZLIB compression which is used to capture screen information." },
    { "ZyGo", "ZyGoVideo", "ZyGo Digital", "?" },
    { "ZYYY", "?", "?", "Video codec usually packaged in Quicktime files. Investigations suggest that it may be a variant of H.263." },
    { 0, 0, 0, 0 }
};

int is_in_codec_4cc_range( char * tmp, char * rng )
{
    char n1[2];
    char n2[2];
    char head[4];
    int i, num1, num2;
    if ( (strlen(rng) == 6) && ISNUM(rng[3]) && (rng[4] == '-') && ISNUM(rng[5]) )
    {
        head[0] = rng[0];
        head[1] = rng[1];
        head[2] = rng[2];
        head[3] = 0;
        n1[0]   = rng[3];
        n1[1]   = 0;
        n2[0]   = rng[5];
        n2[1]   = 0;
        num1 = atoi(n1);
        num2 = atoi(n2);
        if ((num1 < num2) && (num2 > 0)) {
            char * cp = GetNxtBuf();
            for( i = num1; i <= num2; i++ ) {
                sprintf(cp, "%s%d", head, i );
                if ( strcmp(cp,tmp) == 0 )
                    return 1;
            }
        }
    }
    return 0;
}

int is_a_codec_4cc( char * tmp )
{
    PCODEC4CC pc4cc = &codec_4cc[0];
    int cnt = 0;
    size_t len;
    while( pc4cc->codec ) {
        cnt++;
        if ( strcmp(pc4cc->codec,tmp) == 0 )
            return cnt;
        len = strlen(pc4cc->codec);
        if (len > 4) {
            if (is_in_codec_4cc_range( tmp, pc4cc->codec ) )
                return cnt;
        }
        if (len && (len < 4)) {
            if (strncmp(pc4cc->codec,tmp,len) == 0)
                return cnt;
        }
        pc4cc++;
    }
    return 0;
}

typedef struct tagSTRMTYPE {
    char * fourcc;
    char * type;
    int vora;
}STRMTYPE, * PSTRMTYPE;

#define ST_NOT_DEFINED -1
#define ST_TEXT        0
#define ST_VIDEO       1
#define ST_AUDIO       2
#define ST_MIDI        3

STRMTYPE avi_strmtypes[] = {
    { "auds", "Audio stream", ST_AUDIO  },
    { "mids", "MIDI stream",  ST_MIDI   },
    { "txts", "Text stream",  ST_TEXT   },
    { "vids", "Video stream", ST_VIDEO  },
    { 0, 0 }
};

char * get_stream_data_type( char * dt, int * type )
{
    PSTRMTYPE pt = &avi_strmtypes[0];
    char * cp = GetNxtBuf();
    *cp = 0;
    while( pt->type ) {
        if ( strcmp( pt->fourcc, dt ) == 0 ) {
            sprintf(cp, "'%s' is a %s", dt, pt->type);
            if(type)
                *type = pt->vora;
            break;
        }
        pt++;
    }
    if( *cp == 0 ) {
        sprintf(cp, "'%s' is an unlisted type", dt );
        if(type)
          *type = ST_NOT_DEFINED;
    }
    return cp;
}

typedef struct tagMFOURCC {
    BYTE fourcc[4];
}MFOURCC, * PMFOURCC;

typedef struct tagMCHUNK {
    MFOURCC riff;   // = name
    DWORD size;    // 4 byte chunk size
}MCHUNK, *PMCHUNK;

typedef struct tagMAVIHDR {
    MCHUNK chk;   // = 'RIFF' and DWORD size;      // 4 byte file size
    MFOURCC type;    //
}MAVIHDR, * PMAVIHDR;

typedef struct tagMLISTCHK {
    MCHUNK chk;
    MFOURCC type;
}MLISTCHK, * PMLISTCHK;

char * get_fourcc( PMFOURCC pfcc )
{
    char * cp = GetNxtBuf();
    int i = 0;
    PBYTE pb = (PBYTE)(pfcc + 1);
    if (pb > g_pend) {
        cp = "<BEYOND EOF>";
        return cp;
    }
    for(i = 0; i < 4; i++) {
        if( ISSIGCHAR(pfcc->fourcc[i]) ) {
            cp[i] = pfcc->fourcc[i];
        } else {
            cp[i] = 0;
            break;
        }
    }
    cp[4] = 0;
    //sprtf(cp);
    return cp;
}

// 'movi'
// The FOURCC that identifies each data chunk consists of a two-digit 
// stream number followed by a two-character code that defines the 
// type of information in the chunk.
// Two-character code  Description  
typedef struct tagMOVICHUNK {
    char * cc;
    char * desc;
}MOVICHUNK, *PMOVICHUNK;

MOVICHUNK movi_chunks[] = {
    { "db", "Uncompressed video frame" },
    { "dc", "Compressed video frame" },
    { "pc", "Palette change" },
    { "wb", "Audio data" },
    { 0, 0 }
};

int is_movi_desc( char * pc )
{
    PMOVICHUNK pinx = &movi_chunks[0];
    while (pinx->desc) {
        if( strcmp(pc,pinx->cc) == 0 )
            return 1;
        pinx++;
    }
    return 0;
}


char * get_movi_desc( char * pc )
{
    PMOVICHUNK pinx = &movi_chunks[0];
    char * cp;
    while (pinx->desc) {
        if( strcmp(pc,pinx->cc) == 0 )
            return pinx->desc;
        pinx++;
    }
    cp = GetNxtBuf();
    sprintf( "<%s NOT found>", pc );
    return cp;
}

int is_movi_fourcc( char * pinx1 )
{
    size_t len = strlen(pinx1);
    if ((len == 4) &&
        (ISNUM(pinx1[0])) &&
        (ISNUM(pinx1[1])) &&
        (ISALPHA(pinx1[2])) &&
        (ISALPHA(pinx1[3])) )
    {
        char alpha[3];
        alpha[0] = pinx1[2];
        alpha[1] = pinx1[3];
        alpha[2] = 0;
        if ( is_movi_desc(alpha) )
            return 1;
    }
    return 0;
}

char * get_movi_info( char * pinx1 )
{
    size_t len = strlen(pinx1);
    char number[3];
    char type[3];
    char * cp = "<NOT 'inx1' 4 chars>";
    // if ((len == 4)&&(ISNUM(pinx1[0]))&&(ISNUM(pinx1[1]))&&(ISALPHA(pinx1[2]))&&(ISALPHA(pinx1[3])))
    if ( is_movi_fourcc(pinx1) )
    {
        number[0] = pinx1[0];
        number[1] = pinx1[1];
        number[2] = 0;
        type[0] = pinx1[2];
        type[1] = pinx1[3];
        type[2] = 0;
        cp = GetNxtBuf();
        sprintf( cp, "Number %02d: %s", atoi(&number[0]), get_movi_desc( &type[0] ) );
    }
    return cp;
}


CHAR * get_bit_stg( DWORD flag )
{
    CHAR * cp = GetNxtBuf();
    DWORD mask = 0x80000000;
    int i;
    int got_bit = 0;
    *cp = 0;
    for( i = 0; i < 32; i++ ) {
        if ( flag & mask ) {
            if ( i && (got_bit == 0) )
                strcat(cp,"0");
            got_bit++;
            strcat(cp,"1");
        } else if ( got_bit ) {
            strcat(cp,"0");
        }
        mask >>= 1;
    }
    return cp;
}

int get_bit_count( DWORD flag )
{
    DWORD mask = 0x80000000;
    int i;
    int got_bit = 0;
    for( i = 0; i < 32; i++ ) {
        if ( flag & mask ) {
            got_bit++;
        }
        mask >>= 1;
    }
    return got_bit;
}

CHAR * flag_2_short_desc( DWORD flag, PFLAG2STG pf2s, int equal )
{
    CHAR * cp = GetNxtBuf();
    *cp = 0;
    if ((equal == 0 ) && ( flag == 0 )) {
        cp = "<none>";
        return cp;
    }
    while( pf2s->desc ) {
        if ( equal ) {
            if ( flag == pf2s->flag ) {
                strcat(cp, pf2s->sdesc);
                break;
            }
        } else {
            // check BITS
            if ( flag & pf2s->flag ) {
                if ( *cp )
                    strcat(cp, " | ");
                strcat(cp, pf2s->sdesc);
                flag &= ~(pf2s->flag);  // remove the BIT
            }
        }
        pf2s++;
    }
    if ( equal ) {
        if ( *cp == 0 )
            sprintf(cp, "No desc for %d (%#x)!", flag, flag );
    } else {
        // missed bits
        if ( *cp == 0 ) {
            sprintf(cp, "Unknown flag bit%s %s [%#X]!",
                ((get_bit_count(flag) > 1) ? "s" : ""), 
                get_bit_stg(flag), flag );
        } else if (flag) {
            if (*cp) {
                sprintf(EndBuf(cp), " (+Unknown bit%s %s [%#X]!)",
                    ((get_bit_count(flag) > 1) ? "s" : ""), 
                    get_bit_stg(flag), flag );
            } else {
                sprintf(EndBuf(cp), "Unknown bit%s [%s] %#X!",
                    ((get_bit_count(flag) > 1) ? "s" : ""), 
                    get_bit_stg(flag), flag );
            }
        }
    }
    return cp;
}

CHAR * hdr_flag_2_short_desc( DWORD flag )
{
    PFLAG2STG pf2s = &hdr_flag[0];
    return flag_2_short_desc(flag, pf2s, 0);
}

CHAR * strm_flag_2_short_desc( DWORD flag )
{
    PFLAG2STG pf2s = &strm_flag[0];
    return flag_2_short_desc(flag, pf2s, 0);    // 0 = check BITS
}

CHAR * avi_index_flag_2_short_desc( DWORD flag )
{
    PFLAG2STG pf2s = &index_flags[0];
    return flag_2_short_desc(flag, pf2s, 0);    // 0 = check BITS
}

CHAR * wave_form_2_short_desc( DWORD flag )
{
    PFLAG2STG pf2s = &wav_format[0];
    return flag_2_short_desc(flag, pf2s, 1);    // use EQUAL, not BITS
}

//     MAIN HEADER from : http://msdn.microsoft.com/en-us/library/dd318180(VS.85).aspx
//typedef struct _avimainheader {
VOID show_main_header( AVIMAINHEADER * pmh )
{
    PMFOURCC pfcc = (PMFOURCC)&pmh->fcc;
    CHAR * nm = get_fourcc(pfcc);
    if ( strcmp(nm,"avih") == 0 ) {
        //FOURCC fcc;                 // Specifies a FOURCC code. The value must be 'avih'.
        // DWORD  cb;                  // Specifies the size of the structure, not including the initial 8 bytes.
        sprtf( "AVIMAINHEADER: Size of structure     = %u\n", pmh->cb );
        // DWORD  dwMicroSecPerFrame;  // Specifies the number of microseconds between frames. This value indicates the overall timing for the file.
        sprtf( "Microseconds between frames          = %u\n", pmh->dwMicroSecPerFrame );
        // DWORD  dwMaxBytesPerSec;    // Specifies the approximate maximum data rate of the file.
        sprtf( "Approximate maximum data rate        = %u\n", pmh->dwMaxBytesPerSec );
        // DWORD  dwPaddingGranularity;// Specifies the alignment for data, in bytes. Pad the data to multiples of this value.
        sprtf( "Padding alignment for data, in bytes = %u\n", pmh->dwPaddingGranularity );
        // DWORD  dwFlags;             // Contains a bitwise combination of zero or more of the following flags:
        sprtf( "Flags                                = %s (%#X)\n", hdr_flag_2_short_desc(pmh->dwFlags), pmh->dwFlags );
        // DWORD  dwTotalFrames;       // Specifies the total number of frames of data in the file.
        sprtf( "Total number of frames               = %u\n", pmh->dwTotalFrames );
        // DWORD  dwInitialFrames;     // Specifies the initial frame for interleaved files. Noninterleaved files should specify zero.
        sprtf( "Initial frame for interleaved        = %u\n", pmh->dwInitialFrames );
        // DWORD  dwStreams;           // Specifies the number of streams in the file. For example, a file with audio and video has two streams.
        sprtf( "Number of streams                    = %u\n", pmh->dwStreams );
        // DWORD  dwSuggestedBufferSize;// Specifies the suggested buffer size for reading the file.
        sprtf( "Suggested buffer size                = %u\n", pmh->dwSuggestedBufferSize );
        // DWORD  dwWidth;             // Specifies the width of the AVI file in pixels.
        sprtf( "Width of the AVI file in pixels      = %u\n", pmh->dwWidth );
        // DWORD  dwHeight;            // Specifies the height of the AVI file in pixels.
        sprtf( "Height of the AVI file in pixels     = %u\n", pmh->dwHeight );
        // DWORD  dwReserved[4];       // Reserved. Set this array to zero.
    //} AVIMAINHEADER;
    } else {
        sprtf( "Offset %p, is NOT an AVIMAINHEADER!\n", pmh );
    }
}

void show_stream_header(AVISTREAMHEADER *sh, int * st)
{
    PMFOURCC pfcc = (PMFOURCC)&sh->fcc;
    CHAR * nm = get_fourcc(pfcc);
    int stream_type = ST_NOT_DEFINED;
    CHAR * type;
    // from : http://msdn.microsoft.com/en-us/library/dd318183(VS.85).aspx
    // typedef struct _avistreamheader {
    //  FOURCC fcc;                   // Specifies a FOURCC code. The value must be 'strh'
    if ( strcmp(nm,"strh") ) {
        sprtf( "Offset %p, is NOT an AVISTREAMHEADER!\n", sh );
        if(st)
            *st = stream_type;
        return;
    }
    // DWORD  cb;                    // Specifies the size of the structure, not including the initial 8 bytes.
    sprtf( "AVISTREAMHEADER: Size of structure     = %u\n", sh->cb );
    // FOURCC fccType;               // Contains a FOURCC that specifies the type of the data contained in the stream
    pfcc = (PMFOURCC) &sh->fccType;
    nm = get_fourcc(pfcc);
    type = get_stream_data_type(nm, &stream_type);
    sprtf( "Type of the data                       = %s (%d)\n", type, stream_type );
    //  FOURCC fccHandler;            // Optionally, contains a FOURCC that identifies a specific data handler.
    pfcc = (PMFOURCC) &sh->fccHandler;
    nm = get_fourcc(pfcc);
    sprtf( "Optional, identifying data handler     = %s\n", nm );
    //  DWORD  dwFlags;               // Contains any flags for the data stream. The bits in the high-order word of these flags are specific to the type of data 
    sprtf( "Any flags for the data stream          = %s (%#X)\n", strm_flag_2_short_desc( sh->dwFlags ), sh->dwFlags );
    //  WORD   wPriority;             // Specifies priority of a stream type. For example, in a file with multiple audio streams, the one with the highest priority might be the default stream.
    sprtf( "Specifies priority of a stream type    = %d\n", sh->wPriority & 0xffff );
    //  WORD   wLanguage;             // Language tag.
    sprtf( "Language tag                           = %d\n", sh->wLanguage & 0xffff );
    //  DWORD  dwInitialFrames;       // Specifies how far audio data is skewed ahead of the video frames in interleaved files. Typically, this is about 0.75 seconds.
    sprtf( "Audio data is skew                     = %d\n", sh->dwInitialFrames );
    //  DWORD  dwScale;               // Used with dwRate to specify the time scale that this stream will use. Dividing dwRate by dwScale gives the number of samples per second. For video streams, this is the frame rate.
    //  DWORD  dwRate;                // For audio streams, this rate corresponds to the time needed to play nBlockAlign bytes of audio, which for PCM audio is the just the sample rate.
    sprtf( "Rate                                   = %f\n",
        (sh->dwScale ? (double)sh->dwRate / (double)sh->dwScale : 9999.9) );
    //  DWORD  dwStart;               // Specifies the starting time for this stream. The units are defined by the dwRate and dwScale members 
    sprtf( "Starting time for this stream          = %d\n", sh->dwStart );
    //  DWORD  dwLength;              // Specifies the length of this stream. The units are defined by the dwRate and dwScale members of the stream's header.
    sprtf( "Length of this stream                  = %d\n", sh->dwLength );
    //  DWORD  dwSuggestedBufferSize; // Specifies how large a buffer should be used to read this stream.
    sprtf( "Buffer size                            = %u\n", sh->dwSuggestedBufferSize );
    //  DWORD  dwQuality;             // Specifies an indicator of the quality of the data in the stream. Quality is represented as a number between 0 and 10,000. -1 for default
    sprtf( "Quality                                = %u\n", sh->dwQuality );
    //  DWORD  dwSampleSize;          // Specifies the size of a single sample of data. This is set to zero if the samples can vary in size
    sprtf( "Single sample size                     = %u\n", sh->dwSampleSize );
    //  struct { short int left; short int top; short int right; short int bottom; } rcFrame;
    sprtf( "rcFrame (left:top:right:bottom)        = %u:%u:%u:%u\n",
        sh->rcFrame.left & 0xffff,
        sh->rcFrame.top & 0xffff,
        sh->rcFrame.right & 0xffff,
        sh->rcFrame.bottom & 0xffff );
    // }AVISTREAMHEADER;
    if(st)
        *st = stream_type;
}

BOOL  LooksLikeAVI( LPDFSTR lpdf )
{
    BOOL bRet = FALSE;
    BYTE * pbegin = (PBYTE)lpdf->lpb;
    PMAVIHDR pavih = (PMAVIHDR)lpdf->lpb;
    DWORD max = lpdf->dwmax;
    PMFOURCC pfcc = &pavih->chk.riff;
    PMCHUNK pchk, pchk2;
    PMLISTCHK plchk;
    DWORD   flen, clen, len2, len3;
    int len;
    char * cp = get_fourcc(pfcc);
    char * tmp, *tmp2, *tmp3;
    BYTE * pb, * pb2, * pb3, *pneed;
    AVIMAINHEADER * pmh = NULL;
    int got_movi = 0;
    int got_riff = 0;
    int had_list = 0;
    int got_strl = 0;
    int got_junk = 0;
    PMFOURCC pfcc2;
    AVISTREAMHEADER * stmhdr = NULL;

    if( strcmp(cp,"RIFF") == 0 ) {
        got_riff = 1;
        flen = pavih->chk.size;
        pfcc = &pavih->type;
        tmp = get_fourcc(pfcc);
        pb3 = (PBYTE)pavih;
        if (VERB9 || DEBUG_LOOKS_LIKE) {
            sprtf( "%s: %s Size = %u, Type = %s (%s)\n",
                GetHEXOffset(pb3 - pbegin),
                cp, flen, tmp,
                (flen + sizeof(MAVIHDR) <= max) ? "ok" : "Size error" );
        }
        pchk = (PMCHUNK) ((PMAVIHDR)pavih + 1);
        len = (int)flen;
        pb2 = (PBYTE)(pchk + 1);
        while( ( len >= sizeof(MCHUNK) ) && (pb2 < g_pend) ) {
            pfcc = &pchk->riff;
            tmp = get_fourcc(pfcc);
            clen = pchk->size;
            pb3 = (PBYTE)pchk;
            if (strcmp(tmp,"LIST") == 0) {
                // is a LIST type
                had_list++;
                plchk = (PMLISTCHK)pchk;
                pfcc = &plchk->type;
                tmp2 = get_fourcc(pfcc);
                len2 = plchk->chk.size;
                if (VERB9 || DEBUG_LOOKS_LIKE) {
                    sprtf( "%s: Chunk [%s], size %u bytes, type %s (%d)\n",
                        GetHEXOffset(pb3 - pbegin),
                        tmp, clen, tmp2, len2 );
                }
                if (strcmp(tmp2,"hdrl") == 0) {
                    if (pmh) {
                        sprtf("BAD AVI RIFF! Already had 'hdrl'\n");
                        return FALSE;
                    } else {
                        pmh = (AVIMAINHEADER *)pchk;
                    }
                    pb3 = (PBYTE)(plchk + 1);
                    pb3 += sizeof(AVIMAINHEADER);
                    pneed = (pb3+sizeof(MCHUNK));
                    if ( pneed < g_pend) {
                        pchk2 = (PMCHUNK)pb3;
                        pfcc2 = &pchk2->riff;
                        tmp3 = get_fourcc(pfcc2);
                        len3 = pchk2->size;
                        if (strcmp(tmp3,"LIST") == 0) {
                            // adjust clen down to this (if required)
                            pb = (PBYTE)((PMCHUNK)pchk + 1); // pointer to DATA, beginning with 'type; FOURCCC
                            if( clen > (DWORD)(pb3 - pb) ) {
                                if (VERB9 || DEBUG_LOOKS_LIKE) {
                                    sprtf("Ajusting clen from %d (%#X), to %d (%#X) - (%d)\n",
                                        clen, clen, (pb3 - pb), (pb3 - pb), len3 );
                                }
                                clen = (pb3 - pb);
                            }
                        }
                    }
                } else if (strcmp(tmp2,"movi") == 0) {
                    got_movi = 1;
                } else if (strcmp(tmp2,"strl") == 0) {
                    got_strl++;
                    pb3 = (PBYTE)(plchk + 1);
                    pneed = (pb3+sizeof(MCHUNK));
                    /* ---------------------
                     *         LIST (size) strl
                     *             strh (0038)
                     *             strf (????)
                     *             indx (3ff8)   <- size may vary, should be sector sized
                     *         LIST (size) strl
                     * --------------------- */
                    if ( pneed < g_pend ) {
                        pchk2 = (PMCHUNK)pb3;
                        pfcc2 = &pchk2->riff;
                        tmp3 = get_fourcc(pfcc2);
                        len3 = pchk2->size;
                        if (VERB9 || DEBUG_LOOKS_LIKE) {
                            sprtf( "%s: Chunk [%s], size %u (%#X) bytes\n",
                                GetHEXOffset(pb3 - pbegin),
                                tmp3, len3, len3 );
                        }
                        if (strcmp(tmp3,"strh") == 0) {
                            stmhdr = (AVISTREAMHEADER *)pb3;
                            pb3 = (PBYTE)(stmhdr + 1);
                            pchk2 = (PMCHUNK)pb3;
                            pfcc2 = &pchk2->riff;
                            tmp3 = get_fourcc(pfcc2);
                            len3 = pchk2->size;
                            if (VERB9 || DEBUG_LOOKS_LIKE) {
                                sprtf( "%s: Chunk [%s], size %u (%#X) bytes\n",
                                    GetHEXOffset(pb3 - pbegin),
                                    tmp3, len3, len3 );
                            }
                            if (strcmp(tmp3,"strf") == 0 ) {
                                pb3 = (PBYTE)(pchk2 + 1);
                                pb3 += len3;
                                if ( !is_valid_chunk( pb3, g_pend ) ) {
                                    pb3 = (PBYTE)pchk2;
                                    pb3 += len3;
                                    if ( !is_valid_chunk( pb3, g_pend ) ) {
                                        pb3 += 2; // NO NOT WHY!!
                                    }
                                }
                                pchk2 = (PMCHUNK)pb3;
                                pfcc2 = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc2);
                                len3 = pchk2->size;
                                if (VERB9 || DEBUG_LOOKS_LIKE) {
                                    sprtf( "%s: Chunk [%s], size %u (%#X) bytes\n",
                                        GetHEXOffset(pb3 - pbegin),
                                        tmp3, len3, len3 );
                                }
                                if (strcmp(tmp3,"LIST") == 0) {
                                    // adjust clen up or down, to this (if required)
                                    pb = (PBYTE)((PMCHUNK)pchk + 1); // pointer to DATA, beginning with 'type; FOURCCC
                                    if( clen != (DWORD)(pb3 - pb) ) {
                                        if (VERB9 || DEBUG_LOOKS_LIKE) {
                                            sprtf("Ajusting clen from %d (%#X), to %d (%#X) - (%d)\n",
                                                clen, clen, (pb3 - pb), (pb3 - pb), len3 );
                                        }
                                        clen = (pb3 - pb);
                                    }
                                } else if (strcmp(tmp3,"JUNK") == 0) {
                                    // adjust clen up or down, to this (if required)
                                    pb = (PBYTE)((PMCHUNK)pchk + 1); // pointer to DATA, beginning with 'type; FOURCCC
                                    if( clen != (DWORD)(pb3 - pb) ) {
                                        if (VERB9 || DEBUG_LOOKS_LIKE) {
                                            sprtf("Ajusting clen from %d (%#X), to %d (%#X) - (%d)\n",
                                                clen, clen, (pb3 - pb), (pb3 - pb), len3 );
                                        }
                                        clen = (pb3 - pb);
                                    }
                                }
                            }
                        }
                    }
                } else if (strcmp(tmp2,"JUNK") == 0) {
                    got_junk++;
                }
                // NO, len -= sizeof(MLISTCHK);
                // pb = (PBYTE)((PMLISTCHK)plchk + 1); // pointer to DATA
                pb = (PBYTE)((PMCHUNK)pchk + 1); // pointer to DATA, beginning with 'type; FOURCCC
                len -= sizeof(MCHUNK);
                pb3 = pb;
                pb3 += clen;
                if ( !is_valid_chunk( pb3, g_pend ) ) {
                    pb3 = (PBYTE)(plchk + 1);
                    if ( !is_valid_chunk( pb3, g_pend ) ) {
                        pb3 = (PBYTE)plchk;
                        pb3 += clen;
                        if ( !is_valid_chunk( pb3, g_pend ) ) {
                            pb3 += 4;
                            if ( is_valid_chunk( pb3, g_pend ) ) {
                                if( clen != (DWORD)(pb3 - pb) ) {
                                    if (VERB9 || DEBUG_LOOKS_LIKE) {
                                        sprtf("Ajusting clen from %d (%#X), to %d (%#X) - (%d)\n",
                                            clen, clen, (pb3 - pb), (pb3 - pb), len3 );
                                    }
                                    clen = (pb3 - pb);
                                }
                            }
                        }
                    }
                }

            } else {
                // NOT a LIST
                if (VERB9 || DEBUG_LOOKS_LIKE) {
                    sprtf( "%s: Chunk [%s], size %u bytes\n",
                        GetHEXOffset(pb3 - pbegin),
                        tmp, clen );
                }
                pb = (PBYTE)((PMCHUNK)pchk + 1);
                len -= sizeof(MCHUNK);
            }
            pb += clen;
            len -= clen;
            pchk = (PMCHUNK)pb;
            pb2 = (PBYTE)(pchk + 1);
        }
    }
    // decide it is or not
    if (got_riff && pmh && got_movi)
        bRet = TRUE;
    else {
        sprtf( "NOT AVI: " );
        if (got_riff) 
            sprtf("got RIFF ");
        else
            sprtf("NO RIFF chunk! ");
        if (pmh)
            sprtf( "got hdr1 (AVIMAINHEADER) " );
        else
            sprtf("NO hdr1 (AVIMAINHEADER) chunk! " );
        if (got_movi)
            sprtf("got movi");
        else
            sprtf("NO 'movi' chunk! ");
        sprtf("\n");
    }

    return bRet;
}

// void show_BMP_info( HANDLE hf, LPSTR fn, LPSTR lpb, DWORD len,
//				  DWORD fsiz )
void show_BMP_info( BITMAPINFOHEADER * lpbih, UINT * puint )
{
    UINT        ui, cc;
	LPSTR       lps;
	LPSTR       lpd = &gszDiag[0];
	RGBQUAD *   lpq;
    BYTE        r,g,b;
    DWORD       imgsiz, imgrow;
    UINT        ucnt = sizeof(BITMAPINFOHEADER);

    KillLineList();
    lpq    = (LPRGBQUAD) ( lpbih + 1 ); // color table, if ANY, AFTER INFO HEADER
    cc = lpbih->biClrUsed;  // get number of colours used
    if(!cc)  // if NONE specified in header
        cc = GetColorCount(lpbih->biBitCount); // based on BITS-PER_PIXEL, 8-bits=256 colours
    if(!cc) {
        if( lpbih->biCompression == BI_BITFIELDS )
            cc = 3;
    }

    imgsiz = lpbih->biSizeImage;  // extract IMAGE SIZE
    imgrow = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT

    if( lpbih->biSize  != sizeof(BITMAPINFOHEADER) ) {
        sprtf("WARNING: lpbih->biSize = %d (%#X), should be %d (%#X)\n",
            lpbih->biSize, lpbih->biSize, 
            sizeof(BITMAPINFOHEADER), sizeof(BITMAPINFOHEADER) );
    }

    if( lpbih->biCompression == BI_BITFIELDS ) {
        //sprintf(EndBuf(lpd), " (However BI_BITFIELDS has 3 DWORD masks = Offset %d (%#x))"MEOR,
        //   lpbmfh->bfOffBits + (sizeof(DWORD) * 3),
        //   lpbmfh->bfOffBits + (sizeof(DWORD) * 3) );
    }
	//prts(lpd);

    // do BITMAPINFOHEADER
    imgrow = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT
    imgsiz = imgrow *  // bytes per row, rounded be ALLIGNMENT
            lpbih->biHeight;     // HEIGHT = rows in the image
    sprintf(lpd,
        "BITMAPINFOHEADER - dwSize =  %d Bytes  (%#x)"MEOR,
        lpbih->biSize,	// sizeof(BITMAPINFOHEADER) );
        lpbih->biSize );
    prts(lpd);

	sprintf(lpd,
		" Width    = %d Pixels"MEOR,
		lpbih->biWidth );
	prts(lpd);

	sprintf(lpd,
		" Height   = %d Pixels"MEOR,
		lpbih->biHeight );
	prts(lpd);

	sprintf(lpd,
		" Planes   = %d (Should be 1!)"MEOR,
		lpbih->biPlanes );
	prts(lpd);

	sprintf(lpd,
		" BitCount = %d",
		lpbih->biBitCount );

   // depends on the TYPE of bitmap
	if( lps = GetColorStg(lpbih->biBitCount) )
		strcat(lpd,lps);
   // end of this line
	strcat(lpd,MEOR);
   // and OUT IT
	prts(lpd);

	sprintf(lpd,
		" BitCompr = %d ",
		lpbih->biCompression );
    // various types 
	if( lps = GetCompStg( lpbih->biCompression ) )
		strcat(lpd,lps);
	strcat(lpd,MEOR);
	prts(lpd);

	//uk   = BytesPerRow(lpbih); // bytes per row, rounded be ALLIGNMENT
	//iRow = lpbih->biHeight;    // HEIGHT = rows in the image
    if(lpbih->biSizeImage == imgsiz) { // (uk * iRow) )
        // this is as it should be
        sprintf(lpd,
            " Img.Size = %d. ok",
            lpbih->biSizeImage );
    } else if(lpbih->biSizeImage == 0) {
        sprintf(lpd,
            " Img.Size = %d. Seems zero is ok, Should be %d.",
            lpbih->biSizeImage,
            imgsiz  );
    } else {
        sprintf(lpd,
		    " Img.Size = %d. (at variance to %d?)",
            lpbih->biSizeImage,
            imgsiz ); // ( uk * iRow ) );
    }
    sprintf(EndBuf(lpd), " (%dx%d=%d)",
        imgrow, // bytes per row, rounded be ALLIGNMENT
        lpbih->biHeight, // HEIGHT = rows in the image
        imgsiz );
    strcat(lpd,MEOR);
    prts(lpd);

	sprintf(lpd,
		" X.PPMet. = %d"MEOR,
		lpbih->biXPelsPerMeter );
	prts(lpd);

	sprintf(lpd,
		" Y.PPMet. = %d"MEOR,
		lpbih->biYPelsPerMeter );
	prts(lpd);
	
	sprintf(lpd,
		" ClrUsed  = %d",
		lpbih->biClrUsed );
    if(lpbih->biClrUsed) {
        sprintf(EndBuf(lpd)," (cc = %d)",  cc);
    } else {
        sprintf(EndBuf(lpd), " (cc = %d)", cc);
    }
    strcat(lpd,MEOR);
	prts(lpd);

	sprintf(lpd,
		" ClrImp.  = %d"MEOR,
		lpbih->biClrImportant );
	prts(lpd);

    if( cc ) {
        UINT uk;
        //typedef struct tagRGBQUAD { // rgbq 
        //    BYTE    rgbBlue; 
        //    BYTE    rgbGreen; 
        //    BYTE    rgbRed; 
        //    BYTE    rgbReserved; 
        //} RGBQUAD; 
        //		RGBQUAD *	lpq;
        //		UINT		uk;

        ucnt += cc * sizeof(RGBQUAD);   // bump the SIZE
		lps = (LPSTR)(lpbih + 1); //sizeof(BITMAPINFOHEADER)
		sprintf(lpd,
			"COLOUR %s - Count of RGBQUADs = %d (%d Bytes)"MEOR,
         ( (lpbih->biCompression == BI_BITFIELDS) ? "MASK" : "TABLE" ),
			cc, ( cc * sizeof(RGBQUAD) ) );
		prts(lpd);
        if (VERB3)
            prt( "Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
		lpq = (RGBQUAD *)lps;
		*lpd = 0;
		uk = 0;
		for( ui = 0; ui < cc; ui++ )
		{
            //  0(  0,  0,  0)   1(  0,  0,191)   2(  0,191,  0)   3(  0,191,191) 
            r = lpq->rgbRed;
            g = lpq->rgbGreen;
            b = lpq->rgbBlue;
			sprintf( EndBuf(lpd),
				"%3X(%3d,%3d,%3d) ",
				ui,
                (r & 0xff), // 	(lpq->rgbRed & 0xff),
                (g & 0xff), // 	(lpq->rgbGreen & 0xff),
				(b & 0xff) );  // (lpq->rgbBlue & 0xff) );
            //    BYTE    rgbBlue; 
            //    BYTE    rgbGreen; 
            //    BYTE    rgbRed;
            if( ui < 256 )
            {
                //#define gsBmpColour W.w_sBmpColour // g RGBQUAD [256]
                //#define gdwColCount W.w_dwColCount // g DWORD [256]
                gsBmpColour[ui] = *lpq; // keep COPY of colour from FILE TABLE
            }
			uk++;
			if( uk == 4 ) {
				strcat(lpd,MEOR);
                if(VERB3)
                    prt(lpd);
				*lpd = 0;
				uk = 0;
			}
            //  4(191,  0,  0)   5(191,  0,191)   6(191,191,  0)   7(192,192,192) 
            //248(128,128,128) 249(  0,  0,255) 250(  0,255,  0) 251(  0,255,255) 
            //252(255,  0,  0) 253(255,  0,255) 254(255,255,  0) 255(255,255,255) 
			lpq++;
		}

		if( uk ) {
			strcat(lpd,MEOR);
            if (VERB3)
                prt(lpd);
			*lpd = 0;
			uk = 0;
		}

        uk = SortRGBQ( (LPTSTR)&gsBmpColour[0], FALSE, cc );
		sprintf(lpd,
			"COLOUR TABLE - Count of Colours  = %d."MEOR,
			uk );
		prt(lpd);

	}  // done colour map - ie cc contains colour count

    KillLineList();
    if( puint )
        *puint = ucnt;

}

void show_PCM_special( WAVEFORMATEX * pwf, PBYTE pb_in, DWORD max, uint * puoff, PBYTE pbegin )
{
    PBYTE pb      = pb_in - sizeof(WORD); // they DO say IGNORE last member
    PMCHUNK  pchk = (PMCHUNK)pb;
    PMFOURCC pfcc = &pchk->riff;
    DWORD    len2 = pchk->size;
    char *   tmp2 = get_fourcc(pfcc);
    uint total = 0;
    uint uoff = 0;
    char * stg, *p;
    DWORD    len3 = len2;
    if (strcmp(tmp2,"ISFT") == 0) {
        stg = (char *)(pchk + 1);
        pb = (PBYTE)stg;
        p = stg;
        total += sizeof(MCHUNK);
        while( ISSIGCHAR(*p) && len3 ) {
            total++;
            p++;
            len3--;
        }
        if ((p > stg) && (*p == 0) && len3) {
            sprtf( "ISFT: %s\n", stg );
            p++;
            total++;
            len3--;
            pb = (PBYTE)(pchk + 1);
            if (len2 & 1)
                len2++;
            pb += len2;
            pchk = (PMCHUNK)pb;
            pfcc = &pchk->riff;
            tmp2 = get_fourcc(pfcc);
            if( strcmp(tmp2,"IDIT") == 0 ) {
                len2 = pchk->size;
                stg = (char *)(pchk + 1);
                pb = (PBYTE)stg;
                p = stg;
                len3 = len2;
                total += sizeof(MCHUNK);
                while( ISSIGCHAR(*p) && len3 ) {
                    p++;
                    total++;
                    len3--;
                }
                if ((p > stg) && (*p == 0) && len3) {
                    sprtf( "IDIT: %s\n", stg );
                    p++;
                    total++;
                    len3--;
                    pb = (PBYTE)(pchk + 1);
                    if (len2 & 1)
                        len2++;
                    pb += len2;
                    //pchk = (PMCHUNK)p;
                    pchk = (PMCHUNK)pb;
                    pfcc = &pchk->riff;
                    tmp2 = get_fourcc(pfcc);
                    if ( is_known_fourcc(tmp2) ) {
                        uoff = (pb - (PBYTE)pwf);
                        *puoff = uoff;
                    }
                }
            }
        }
    }
}

// if (stream_type == ST_AUDIO)
// WAVEFORMATEX * pwf = (WAVEFORMATEX *)(pfcc + 1);
// see MMReg.h, Ksproxy.h
void show_wave_form( WAVEFORMATEX * pwf, uint * puint, PBYTE pbegin )
{
    uint uoff = 0;
    DWORD    wFTag = (pwf->wFormatTag & 0xffff);
    char * desc = wave_form_2_short_desc( wFTag );
    DWORD   dws = pwf->cbSize;
    WAVEFORMATEXTENSIBLE * pwfe = NULL;
    MPEG1WAVEFORMAT  * pmpeg = NULL;
    MPEGLAYER3WAVEFORMAT * pmpeg3 = NULL;
    char * str_type = "WAVEFORMATEX";
    PBYTE pb = (PBYTE)(pwf + 1);

    // For audio streams, the information is a WAVEFORMATEX structure.
    // from : http://msdn.microsoft.com/en-us/library/dd390970(VS.85).aspx
    // typedef struct {
    // WORD  wFormatTag;     // Waveform-audio format type.
    sprtf( "WAVE FORMAT: %s (%#x)\n", desc, wFTag );
    // { WAVE_FORMAT_EXTENSIBLE, "WAVEFORMATEXTENSIBLE", "WAVE_FORMAT_EXTENSIBLE" },
    // { WAVE_FORMAT_MPEG, "MPEGLAYER3WAVEFORMAT", "WAVE_FORMAT_MPEG" },
    // WAVE_FORMAT_MPEGLAYER3
    if ( wFTag == WAVE_FORMAT_EXTENSIBLE ) {
        pwfe = (WAVEFORMATEXTENSIBLE *)pwf;
        uoff = sizeof(WAVEFORMATEXTENSIBLE);
        str_type = "WAVEFORMATEXTENSIBLE";
    } else if ( wFTag == WAVE_FORMAT_MPEG ) {
        pmpeg = ( MPEG1WAVEFORMAT *)pwf;
        uoff = sizeof( MPEG1WAVEFORMAT);
        str_type = "MPEG1WAVEFORMAT";
    } else if ( wFTag ==  WAVE_FORMAT_MPEGLAYER3 ) {
        pmpeg3 = (MPEGLAYER3WAVEFORMAT *)pwf;
        uoff = sizeof(MPEGLAYER3WAVEFORMAT);
        str_type = "MPEGLAYER3WAVEFORMAT";
    } else {
        uoff = sizeof(WAVEFORMATEX);
        uoff += (pwf->cbSize & 0xffff);
        if ( wFTag == WAVE_FORMAT_PCM ) {
            str_type = "WAVE_FORMAT_PCM";
        }
    }

    {
        // Format tags are registered with Microsoft Corporation for many compression 
        // algorithms. A complete list of format tags can be found in the Mmreg.h 
        // header file. For one- or two-channel Pulse Code Modulation (PCM) data, 
        // this value should be WAVE_FORMAT_PCM.
        sprtf( "WAVE FORMAT STR: structure is %s, size %u (%u)\n", str_type, dws, uoff );
        //  WORD  nChannels;      // Number of channels in the waveform-audio data
        sprtf( "Number of channels                           = %u\n", pwf->nChannels & 0xffff);
        // DWORD nSamplesPerSec; // Sample rate, in samples per second (hertz). 
        sprtf( "Sample rate (hertz)                          = %u\n", pwf->nSamplesPerSec );
        // If wFormatTag is WAVE_FORMAT_PCM, then common values for nSamplesPerSec 
        // are 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz. 
        // For non-PCM formats, this member must be computed according to the 
        // manufacturer's specification of the format tag. 
        // DWORD nAvgBytesPerSec; // Required average data-transfer rate, in bytes per second, for the format tag.
        sprtf( "Average data-transfer rate, in bytes per sec = %u\n", pwf->nAvgBytesPerSec );
        // WORD  nBlockAlign;    // Block alignment, in bytes.
        sprtf( "Block alignment, in bytes                    = %u\n", pwf->nBlockAlign & 0xffff);
        // The block alignment is the minimum atomic unit of data for the 
        // wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM or 
        // WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of 
        // nChannels and wBitsPerSample divided by 8 (bits per byte). 
        // For non-PCM formats, blah, blah, blah
        // WORD  wBitsPerSample; // Bits per sample for the wFormatTag format type.
        sprtf( "Bits per sample                              = %u\n", pwf->wBitsPerSample & 0xffff );
        // If wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be 
        // equal to 8 or 16. For non-PCM formats, blah blah blah.
        // If wFormatTag is WAVE_FORMAT_EXTENSIBLE, this value can be any 
        // integer multiple of 8. Some compression schemes cannot define a 
        // value for wBitsPerSample, so this member can be zero. 
        // WORD  cbSize;     // Size, in bytes, of extra format information 
        sprtf( "Extra format information                     = %u\n", pwf->cbSize );
        // appended to the end of the WAVEFORMATEX structure. This information 
        // can be used by non-PCM formats to store extra attributes for the wFormatTag
        // }WAVEFORMATEX;
    }
    if ( wFTag == WAVE_FORMAT_PCM ) {
        show_PCM_special( pwf, pb, dws, &uoff, pbegin );
    }

    if(puint)
        *puint = uoff;
}

char * get_name_item( char * name, uint len )
{
    char * cp = GetNxtBuf();
    uint i;
    for(i = 0; i < len; i++) {
        if ( ISSIGCHAR(name[i]) )
            cp[i] = name[i];
        else
            break;
    }
    cp[i] = 0;
    return cp;
}

/* =========================================
typedef struct _avioldindex {
  FOURCC                    fcc;
  DWORD                     cb;
  struct _avioldindex_entry {
    DWORD dwChunkId;    // Specifies a FOURCC that identifies a stream in the AVI file.
    DWORD dwFlags;      // Specifies a bitwise combination of zero or more of the following flags:
    DWORD dwOffset;     // Specifies the location of the data chunk in the file.
    // maybe from 'movi' LIST entry, or beginning of file
    DWORD dwSize;       // Specifies the size of the data chunk, in bytes
  } aIndex[];
}AVIOLDINDEX;
Flags:
AVIIF_KEYFRAME The data chunk is a key frame.
AVIIF_LIST The data chunk is a 'rec ' list.
AVIIF_NO_TIME The data chunk does not affect the timing of the stream.
 For example, this flag should be set for palette changes.
   ============================================== */
int show_indx1(PBYTE pb, PBYTE pbegin, PBYTE pend, PBYTE pmovi)
{
    AVIOLDINDEX * pindex = (AVIOLDINDEX *)pb;
    struct _avioldindex_entry * pentry = &pindex->aIndex[0];
    PMFOURCC pfcc = (PMFOURCC)&pindex->fcc;
    char * tmp; // = get_fourcc(pfcc);
    DWORD  len;
    DWORD   sz = sizeof(struct _avioldindex_entry);
    DWORD  cnt;
    DWORD  dwi, dwoff, dwsz;
    char * cp;
    PBYTE   ptmp, pneed;
    PMCHUNK pchk;
    PMFOURCC pfcc2;
    char * tmp2, *tmp3;
    int found = 0;
    ptmp = pb + sizeof(AVIOLDINDEX);
    if (ptmp >= pend) {
        sprtf( "%s:INDEX: points beyond end of file!\n" );
        return 1;
    }
    // ok, process the INDEX

    tmp = get_fourcc(pfcc);
    len = pindex->cb;
    cnt = len / sz;

    sprtf( "%s:INDEX: '%s', len %u, count %u\n", GetHEXOffset(pb - pbegin),
        tmp, len, cnt );
    if( (cnt * sz) != len ) {
        sprtf("warning: Size did not integer divide!\n");
    }
    for( dwi = 0; dwi < cnt; dwi++ )
    {
        ptmp = (PBYTE)pentry + sz;  // room for one more index
        if ( ptmp > pend ) {
            sprtf( "Index reaches BEYOND end of file! remaining %d\n",
                cnt - dwi);
            break;
        }
        cp = avi_index_flag_2_short_desc( pentry->dwFlags );
        pfcc = (PMFOURCC)&pentry->dwChunkId;
        tmp2 = get_fourcc(pfcc);
        // OFFSET - read in documentation this can be
        // (a) offset from 'movi', or
        // (b) offset from beginning of file
        // =================================
        dwoff = pentry->dwOffset;
        dwsz  = pentry->dwSize;
        // try from beginning of file
        ptmp = pbegin + dwoff;
        pneed = (ptmp + sizeof(MCHUNK));
        found = 0;  // not found
        if (pneed < pend) {
            pchk = (PMCHUNK)ptmp;
            pfcc2 = &pchk->riff;
            tmp3 = get_fourcc(pfcc2);
            if (strcmp(tmp2,tmp3) == 0) {
                found = 1;
                sprtf( " %s: off %u, sz %u, flag %s (%s)\n",
                    get_movi_info(tmp2),
                    dwoff, dwsz, cp, "Ok1" );
            }
        }
        if ( !found ) {
            // try from 'pmovi', if given
            if ( pmovi ) {
                ptmp = pmovi + dwoff;
                if ( (ptmp + sizeof(MCHUNK)) > pend ) {
                    tmp3 = "Note offset beyond EOF";
                    sprtf( " %s: off %u, sz %u, flag %s (%s)\n",
                        get_movi_info(tmp2),
                        dwoff, dwsz, cp, tmp3 );
                } else {
                    pchk = (PMCHUNK)ptmp;
                    pfcc2 = &pchk->riff;
                    tmp3 = get_fourcc(pfcc2);
                    sprtf( " %s: off %u, sz %u, flag %s (%s)\n",
                        get_movi_info(tmp2),
                        dwoff, dwsz, cp,
                        (strcmp(tmp2,tmp3) ? "??" : "Ok") );
                }
            } else {
                // do NOT have pmovi yet
                tmp3 = "<pmovi NOT set>";
                if( dwi == 0 ) {
                    sprtf( " %s: off %u, sz %u, flag %s (%s)\n",
                        get_movi_info(tmp2),
                        dwoff, dwsz, cp, tmp3 );
                } else {
                    sprtf( " %s: off %u, sz %u, flag %s\n",
                        get_movi_info(tmp2),
                        dwoff, dwsz, cp );
                }
            }
        }
        // sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
        //if ( is_movi_fourcc(tmp3)  ) {
        //  sprtf( " movi: %s\n", get_movi_info(tmp3) ); }
        pentry++;
    }
    ptmp = (PBYTE)pentry;
    sprtf( "%s:INDEX: Done len %u, count %u of %u\n", GetHEXOffset(ptmp - pbegin),
        len, dwi, cnt );
    ptmp = 0;
    return 0;
}

// Standard Index Chunk
// LIST size { movi
//              ix01 size 
//              offset size = -> data1
//              offset size = -> data2
//              ...
//              01dc size -> data1
//              01dc size -> data2
//            }
/* ---------------
    FOURCC fcc; // ix##
    DWORD cb;
    WORD wLongsPerEntry; // must be sizeof(aIndex[0])/sizeof(DWORD)
    BYTE bIndexSubType; // must be 0
    BYTE bIndexType; // must be AVI_INDEX_OF_CHUNKS
    DWORD nEntriesInUse; //
    DWORD dwChunkId; // ##dc or ##db or ##wb etc..
    QUADWORD qwBaseOffset; // all dwOffsets in aIndex array are
                            // relative to this
    DWORD dwReserved3; // must be 0
    struct _avistdindex_entry {
        DWORD dwOffset; // qwBaseOffset + this is absolute file offset
        DWORD dwSize; // bit 31 is set if this is NOT a keyframe
    } aIndex[ ];
} AVISTDINDEX, * PAVISTDINDEX;
// struct of a standard index (AVI_INDEX_OF_CHUNKS)
//
typedef struct _avistdindex_entry {
   DWORD dwOffset;       // 32 bit offset to data (points to data, not riff header)
   DWORD dwSize;         // 31 bit size of data (does not include size of riff header), bit 31 is deltaframe bit
   } AVISTDINDEX_ENTRY;
#define AVISTDINDEX_DELTAFRAME ( 0x80000000) // Delta frames have the high bit set
#define AVISTDINDEX_SIZEMASK   (~0x80000000)

typedef struct _avistdindex {
   FOURCC   fcc;               // 'indx' or '##ix'
   UINT     cb;                // size of this structure
   WORD     wLongsPerEntry;    // ==2
   BYTE     bIndexSubType;     // ==0
   BYTE     bIndexType;        // ==AVI_INDEX_OF_CHUNKS
   DWORD    nEntriesInUse;     // offset of next unused entry in aIndex
   DWORD    dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
   DWORDLONG qwBaseOffset;     // base offset that all index intries are relative to
   DWORD    dwReserved_3;      // must be 0
   AVISTDINDEX_ENTRY aIndex[NUMINDEX(2)];
   } AVISTDINDEX;

  ------------------ */

void show_index(PBYTE pb, PBYTE pbegin, PBYTE pend)
{
    AVISTDINDEX *psi = (AVISTDINDEX *)pb;
    AVISTDINDEX_ENTRY * pind = &psi->aIndex[0];
    PMFOURCC pfcc = (PMFOURCC)&psi->fcc;
    char * tmp = get_fourcc(pfcc);
    DWORD  len = psi->cb;
    PMFOURCC pfcc2 = (PMFOURCC)&psi->dwChunkId;
    char * tmp2 = get_fourcc(pfcc2);
    char * tmp3 = "<Not ##ID>";
    DWORD ents = psi->nEntriesInUse;
    DWORD   dwi, off, siz, len2;
    PBYTE   poff;
    PMFOURCC pfcc3;
    char * tmp4;
    PMCHUNK pchk;

    if ( is_movi_fourcc(tmp2)  )
        tmp3 = get_movi_info(tmp2);
    sprtf( "INDEX: %s, len %u, id %s - %s\n", tmp, len, tmp2, tmp3 );
    sprtf( "wLongsPerEntry (==2)          : %u\n", psi->wLongsPerEntry & 0xffff );
    sprtf( "bIndexSubType  (==0)          : %u\n", psi->bIndexSubType & 0xff );
    sprtf( "bIndexType                    : %s (%u) %#x\n",
        ((psi->bIndexType == AVI_INDEX_OF_CHUNKS) ? "INDEX_OF_CHUNKS" :
        (psi->bIndexType == AVI_INDEX_OF_INDEXES) ? "INDEX_OF_INDEXES" :
        "UNKNOWN" ),
        psi->bIndexType, psi->bIndexType );
    sprtf( "nEntriesInUse                 : %u\n", psi->nEntriesInUse );
    sprtf( "qwBaseOffset                  : %I64u\n", psi->qwBaseOffset );
    for (dwi = 0; dwi < ents; dwi++) {
        off = pind->dwOffset;
        siz = pind->dwSize;
        poff = pbegin + off;
        pchk = (PMCHUNK)poff;
        pfcc3 = &pchk->riff;
        tmp4 = get_fourcc(pfcc3);
        len2 = pchk->size;
        sprtf( " %3d: off %u, siz %u, abs %s '%s' sz %u\n", (dwi + 1), off, siz,
            GetHEXOffset(off),
            tmp4, len2);

        pind++;
    }
    if (psi->bIndexType == AVI_INDEX_OF_CHUNKS) {
        if (psi->bIndexSubType == AVI_INDEX_OF_SUB_2FIELD ) {
            // is FIELD index
            AVIFIELDINDEX * pfi = (AVIFIELDINDEX *)pb;

        } else {
            // is STANDARD index
        }
    } else if ( psi->bIndexType == AVI_INDEX_OF_INDEXES ) {
        // SUPER IDNEX

    } else {
        sprtf( "ERROR: %#X (%u) NOT YET SUPPORTED!\n",
            psi->bIndexType,
            psi->bIndexType );
    }

}

char * fourcc_list[] = {
    "RIFF", "LIST", "hdrl", "movi", "avih", "strl", "strh",
    "strf", "strn", "strd", "indx", "idx1", "odml", "JUNK",
    "dmlh",
    0
};

int is_known_fourcc(char * tmp)
{
    char * *list = &fourcc_list[0];
    size_t len = strlen(tmp);
    if (len < MY_MIN_FOURCC)
        return 0;
    while(*list) {
        if ( strcmp(*list,tmp) == 0 )
            return 1;
        list++;
    }
    return 0;
}


int is_valid_chunk( PBYTE pb, PBYTE pend )
{
    PBYTE ptmp = pb + sizeof(MCHUNK);
    PMCHUNK pchk = (PMCHUNK)pb;
    PMFOURCC pfcc = &pchk->riff;
    char * tmp;
    size_t len;
    if ( ptmp > pend )
        return 0;
    tmp = get_fourcc(pfcc);
    len = strlen(tmp);
    if (len < MY_MIN_FOURCC)
        return 0;
    if ( is_known_fourcc(tmp) )
        return 1;
    if ( is_movi_fourcc(tmp) )
        return 1;
    if ( is_a_codec_4cc(tmp) )
        return 1;
    return 0;
}


BOOL  ProcessAVI( LPDFSTR lpdf )
{
    BOOL bRet = TRUE;
    PMAVIHDR pavih = (PMAVIHDR)lpdf->lpb;
    DWORD max = lpdf->dwmax;
    PMFOURCC pfcc = &pavih->chk.riff;
    PMCHUNK pchk, pchk2;
    PMLISTCHK plchk, plchk2;
    DWORD   flen, clen, len2;
    int len;
    char * cp = get_fourcc(pfcc);
    char * tmp, *tmp2, *tmp3;
    BYTE * pb, *pb2, *pb3, *pb4;
    AVIMAINHEADER * pmh = NULL;
    int got_movi = 0;
    AVISTREAMHEADER * stmhdr = NULL;
    int stream_type = ST_NOT_DEFINED;
    UINT    offset = 0;
    PBYTE   pbegin = (PBYTE)lpdf->lpb;
    PBYTE   pend   = pbegin + max;
    PBYTE   pmovi;
    PBYTE   pneed;

    if( strcmp(cp,"RIFF") == 0 ) {
        plchk2 = (PMLISTCHK)pavih;
        flen = pavih->chk.size;
        pfcc = &pavih->type;
        tmp = get_fourcc(pfcc);
        sprtf( "%s Size = %u, Type = %s (%s)\n", cp, flen, tmp,
            (flen + sizeof(MAVIHDR) <= max) ? "ok" : "Size error" );
        pchk = (PMCHUNK) ((PMAVIHDR)pavih + 1);
        plchk2 = (PMLISTCHK)pchk;
        pb = (PBYTE)plchk2;
        len = (int)flen;
        pneed = (pb + sizeof(MCHUNK));
        while(( len > 0 ) && (pneed < pend)) {
            pb = (PBYTE)plchk2;
            pfcc = &plchk2->chk.riff;
            tmp = get_fourcc(pfcc);
            clen = plchk2->chk.size;
            if (strcmp(tmp,"LIST") == 0) {
                // is a LIST type
                plchk = (PMLISTCHK)pchk;
                pfcc = &plchk->type;
                tmp2 = get_fourcc(pfcc);
                sprtf( "%s: %s %u %s\n", GetHEXOffset(pb - pbegin), tmp, clen, tmp2);
                tmp3 = "";
                if (strcmp(tmp2,"hdrl") == 0) {
                    tmp3 = "AVIMAINHEADER";
                } else if (strcmp(tmp2,"movi") == 0) {
                    got_movi = 1;
                }
                if (strcmp(tmp2,"hdrl") == 0) {
                    pb  = (PBYTE)(pfcc + 1);
                    pneed = (pb + sizeof(MCHUNK));
                    plchk2 = (PMLISTCHK)pb;
                    pfcc = &plchk2->chk.riff;
                    tmp3 = get_fourcc(pfcc);
                    len2 = plchk2->chk.size;
                    if (strcmp(tmp3,"avih") == 0) {
                        sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                        pmh = (AVIMAINHEADER *)pb;
                        show_main_header(pmh);
                        //             LIST ('strl'
                        //                   'strh'(<Stream header>)
                        //                   'strf'(<Stream format>)
                        //                   [ 'strd'(<Additional header data>) ]
                        //                   [ 'strn'(<Stream name>) ]
                        //                   ...
                        //                  )
                        //              ...
                        pb  = (PBYTE)(pmh + 1);
                        pneed = (pb + sizeof(MCHUNK));
                        pchk2 = (PMCHUNK)pb;
                        plchk2 = (PMLISTCHK)pb;
                        pfcc = &pchk2->riff;
                        tmp3 = get_fourcc(pfcc);
                        len2 = plchk2->chk.size;
                        sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                        while (strcmp(tmp3,"LIST") == 0) {
                            pfcc = &plchk2->type;
                            tmp2 = get_fourcc(pfcc);
                            sprtf( "%s: %s %u %s\n", GetHEXOffset(pb - pbegin), tmp3, len2, tmp2);
                            if (strcmp(tmp2,"strl") == 0) {
                                pb  = (PBYTE)(pfcc + 1);
                                pneed = (pb + sizeof(MCHUNK));
                                pchk2 = (PMCHUNK)pb;
                                plchk2 = (PMLISTCHK)pb;
                                pfcc = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc);
                                len2 = pchk2->size;
                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                if (strcmp(tmp3,"strh") == 0) {
                                    stmhdr = (AVISTREAMHEADER *)pb;
                                    show_stream_header(stmhdr, &stream_type);
                                    pb2 = (PBYTE)(stmhdr + 1);
                                    pb3 = pb + len2 + sizeof(MCHUNK);
                                    if ( is_valid_chunk(pb2, pend) ) 
                                        pb = pb2;
                                    else if ( is_valid_chunk(pb3, pend) )
                                        pb = pb3;
                                    else {
                                        pb  = (PBYTE)(stmhdr + 1);
                                        sprtf("Warning: Have NOT found NEXT CHUNK!\n");
                                    }
                                    pneed = (pb + sizeof(MCHUNK));
                                    pchk2 = (PMCHUNK)pb;
                                    plchk2 = (PMLISTCHK)pb;
                                    pfcc = &pchk2->riff;
                                    tmp3 = get_fourcc(pfcc);
                                    len2 = pchk2->size;
                                    sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                    if (strcmp(tmp3,"strf") == 0) {
                                        // BITMAPINFOHEADER or WAVEFORMATEX
                                        if (stream_type == ST_VIDEO) {
                                            BITMAPINFOHEADER * pbmi = (BITMAPINFOHEADER *)(pchk2 + 1);
                                            offset = 0;
                                            show_BMP_info(pbmi, &offset);
                                            pb = (PBYTE)pbmi;
                                            pb += offset;
                                            pneed = (pb + sizeof(MCHUNK));
                                            pchk2 = (PMCHUNK)pb;
                                            plchk2 = (PMLISTCHK)pb;
                                            pfcc = &pchk2->riff;
                                            tmp3 = get_fourcc(pfcc);
                                            len2 = pchk2->size;
                                            sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            if (strcmp(tmp3,"strn") == 0) {
                                                pb = (PBYTE)(pchk2 + 1);
                                                sprtf( "Got NAME %s: len %d\n",
                                                    get_name_item( (char *)pb, len2 ), len2);
                                                // The data is always padded to nearest WORD boundary
                                                if (len2 & 1)
                                                    len2++;
                                                pb += len2;
                                                pneed = (pb + sizeof(MCHUNK));
                                                pchk2 = (PMCHUNK)pb;
                                                plchk2 = (PMLISTCHK)pb;
                                                pfcc = &pchk2->riff;
                                                tmp3 = get_fourcc(pfcc);
                                                len2 = pchk2->size;
                                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            }
                                            if (strcmp(tmp3,"strd") == 0) {
                                                pb = (PBYTE)(pchk2 + 1);
                                                if (len2 & 1)
                                                    len2++;
                                                pb += len2;
                                                pneed = (pb + sizeof(MCHUNK));
                                                pchk2 = (PMCHUNK)pb;
                                                plchk2 = (PMLISTCHK)pb;
                                                pfcc = &pchk2->riff;
                                                tmp3 = get_fourcc(pfcc);
                                                len2 = pchk2->size;
                                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            }
                                            if (strcmp(tmp3,"indx") == 0) {
                                                show_index(pb, pbegin, pend);
                                                pb = (PBYTE)(pchk2 + 1);
                                                if (len2 & 1)
                                                    len2++;
                                                pb += len2;
                                                pneed = (pb + sizeof(MCHUNK));
                                                pchk2 = (PMCHUNK)pb;
                                                plchk2 = (PMLISTCHK)pb;
                                                pfcc = &pchk2->riff;
                                                tmp3 = get_fourcc(pfcc);
                                                len2 = pchk2->size;
                                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            }
                                    } else if (stream_type == ST_AUDIO) {
                                            WAVEFORMATEX * pwf = (WAVEFORMATEX *)(pchk2 + 1);
                                            offset = 0;
                                            show_wave_form(pwf, &offset, pbegin);
                                            pb2 = (PBYTE)(pwf + 1);
                                            pb3 = pb + len2 + sizeof(MCHUNK);
                                            pb4 = (PBYTE)pwf;
                                            pb4 += offset;
                                            if ( is_valid_chunk(pb4, pend) ) 
                                                pb = pb4;
                                            else if ( is_valid_chunk(pb3, pend) )
                                                pb = pb3;
                                            else if ( is_valid_chunk(pb2, pend) )
                                                pb = pb2;
                                            else {
                                                pb = (PBYTE)pwf;
                                                pb += offset;
                                                sprtf("Warning: Have NOT found NEXT CHUNK!\n");
                                            }
                                            pneed = (pb + sizeof(MCHUNK));
                                            pchk2 = (PMCHUNK)pb;
                                            plchk2 = (PMLISTCHK)pb;
                                            pfcc = &pchk2->riff;
                                            tmp3 = get_fourcc(pfcc);
                                            len2 = pchk2->size;
                                            sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            if (strcmp(tmp3,"strn") == 0) {
                                                pb = (PBYTE)(pchk2 + 1);
                                                sprtf( "Got NAME %s: len %d\n",
                                                    get_name_item( (char *)pb, len2 ), len2);
                                                // The data is always padded to nearest WORD boundary
                                                if (len2 & 1)
                                                    len2++;
                                                pb += len2;
                                                pneed = (pb + sizeof(MCHUNK));
                                                pchk2 = (PMCHUNK)pb;
                                                plchk2 = (PMLISTCHK)pb;
                                                pfcc = &pchk2->riff;
                                                tmp3 = get_fourcc(pfcc);
                                                len2 = pchk2->size;
                                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            }
                                            if (strcmp(tmp3,"strd") == 0) {
                                                pb = (PBYTE)(pchk2 + 1);
                                                if (len2 & 1)
                                                    len2++;
                                                pb += len2;
                                                pneed = (pb + sizeof(MCHUNK));
                                                pchk2 = (PMCHUNK)pb;
                                                plchk2 = (PMLISTCHK)pb;
                                                pfcc = &pchk2->riff;
                                                tmp3 = get_fourcc(pfcc);
                                                len2 = pchk2->size;
                                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                            }
                                        }
                                    }
                                }
                            } else if (strcmp(tmp2,"movi") == 0) {
                                // no pmovi  = (PBYTE)(pfcc + 1);
                                // no pmovi = pb;
                                // no pmovi = (PBYTE)(plchk2 + 1);
                                pmovi = (PBYTE)(pchk2 + 1);
                                //      LIST ('movi'
                                //            {SubChunk | LIST ('rec '
                                //             SubChunk1
                                //             SubChunk2
                                //                         ...
                                //                              )
                                //               ...
                                //             }
                                //             ...
                                //            )
                                pb  = (PBYTE)(pfcc + 1);
                                pneed = (pb + sizeof(MCHUNK));
                                pchk2 = (PMCHUNK)pb;
                                plchk2 = (PMLISTCHK)pb;
                                pfcc = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc);
                                len2 = pchk2->size;
                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                if ( is_movi_fourcc(tmp3)  ) {
                                    sprtf( " movi: %s\n", get_movi_info(tmp3) );
                                }
                                while ( strcmp(tmp3,"LIST") &&
                                    strcmp(tmp3,"idx1") ) {
                                    // NOT a LIST nor INDEX
                                    //NO pb  = (PBYTE)(pfcc + 1);
                                    pb  = (PBYTE)(pchk2 + 1);
                                    if (len2 & 1)
                                       len2++;
                                    pb += len2;
                                    pneed = (pb + sizeof(MCHUNK));
                                    pchk2 = (PMCHUNK)pb;
                                    plchk2 = (PMLISTCHK)pb;
                                    pfcc = &pchk2->riff;
                                    tmp3 = get_fourcc(pfcc);
                                    len2 = pchk2->size;
                                    sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                    if ( is_movi_fourcc(tmp3) ) {
                                        sprtf( " movi: %s\n", get_movi_info(tmp3) );
                                    }
                                }
                                if (strcmp(tmp3,"idx1") == 0) {
                                    show_indx1(pb, pbegin, pend, pmovi);
                                    pb  = (PBYTE)(pchk2 + 1);
                                    if (len2 & 1)
                                       len2++;
                                    pb += len2;
                                    pneed = (pb + sizeof(MCHUNK));
                                    pchk2 = (PMCHUNK)pb;
                                    plchk2 = (PMLISTCHK)pb;
                                    pfcc = &pchk2->riff;
                                    tmp3 = get_fourcc(pfcc);
                                    len2 = pchk2->size;
                                    sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                                }
                            } else if (strcmp(tmp2,"odml") == 0) {
                                pb  = (PBYTE)(pchk2 + 1);
                                if (len2 & 1)
                                   len2++;
                                pb += len2;
                                pneed = (pb + sizeof(MCHUNK));
                                pchk2 = (PMCHUNK)pb;
                                plchk2 = (PMLISTCHK)pb;
                                pfcc = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc);
                                len2 = pchk2->size;
                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                            }
                            while (strcmp(tmp3,"JUNK") == 0) {
                                len2 = pchk2->size;
                                pb = (PBYTE)(pchk2 + 1);
                                pb += len2;
                                pneed = (pb + sizeof(MCHUNK));
                                pchk2 = (PMCHUNK)pb;
                                plchk2 = (PMLISTCHK)pb;
                                pfcc = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc);
                                len2 = pchk2->size;
                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                            }
                            if (strcmp(tmp3,"idx1") == 0) {
                                show_indx1(pb, pbegin, pend, pmovi);
                                pb  = (PBYTE)(pchk2 + 1);
                                if (len2 & 1)
                                   len2++;
                                pb += len2;
                                pneed = (pb + sizeof(MCHUNK));
                                pchk2 = (PMCHUNK)pb;
                                plchk2 = (PMLISTCHK)pb;
                                pfcc = &pchk2->riff;
                                tmp3 = get_fourcc(pfcc);
                                len2 = pchk2->size;
                                sprtf( "%s: %s %u\n", GetHEXOffset(pb - pbegin), tmp3, len2);
                            }
                        }   // while a LIST
                    }
                } else {
                    prt( "ERROR: Did not start with 'hdr1'!\n" );
                    len = 0;
                    break;
                }
                // NO, len -= sizeof(MLISTCHK);
                // pb = (PBYTE)((PMLISTCHK)plchk + 1); // pointer to DATA
                //pb = (PBYTE)((PMCHUNK)pchk + 1); // pointer to DATA, beginning with 'type; FOURCCC
                //len -= sizeof(MCHUNK);
            } else {
                // NOT a LIST
                //sprtf( "Chunk [%s], size %u bytes\n", tmp, clen );
                //pb = (PBYTE)((PMCHUNK)pchk + 1);
                //len -= sizeof(MCHUNK);
                prt( "ERROR: Did not start with 'LIST'!\n" );
                len = 0;
                break;
            }
            //pb += clen;
            //len -= clen;
            //pchk = (PMCHUNK)pb;
        } // while len > 0

    }
    return bRet;
}



BOOL  DumpAVI( LPDFSTR lpdf )
{
    BOOL bRet = FALSE;
    PBYTE pb = (PBYTE)lpdf->lpb;
    if( !lpdf->dwmax )
        return bRet;
    pb += lpdf->dwmax;
    g_pend = pb;    // set a EOF - available ot ALL

    if( LooksLikeAVI( lpdf ) ) {
        bRet = ProcessAVI(lpdf);
    }

    return bRet;
}

#endif // ADD_AVI_FILE

#endif // WIN32
//////////////////////////////////////////////////////////////////////////////////////


// eof - DumpAVI.c
