// DumpWAV.c
// FIX20081204 - Added -wav to process WAVE file
// from : http://technology.niagarac.on.ca/courses/ctec1631/WavFileFormat.html
// from : http://www.sonicspot.com/guide/wavefiles.html
#include "dump4.h"
extern void ProcessHex( PBYTE pb, DWORD len );

/* =========================================================
Wave File Header 
Wave File Chunks 
 Format Chunk - "fmt " 
 Data Chunk - "data" 
 Fact Chunk - "fact" 
 Cue Chunk - "cue " 
 Playlist Chunk - "plst" 
 Associated Data List Chunk - "list" 
 Label Chunk - "labl" 
 Labeled Text Chunk - "ltxt" 
 Note Chunk - "note" 
 Sample Chunk - "smpl" 
 Instrument Chunk - "inst" 

 Format Chunk
 Offset Size Description Value 
0x00 4 Chunk ID "fmt " (0x666D7420) 
0x04 4 Chunk Data Size 16 + extra format bytes 
0x08 2 Compression code 1 - 65,535 
0x0a 2 Number of channels 1 - 65,535 
0x0c 4 Sample rate 1 - 0xFFFFFFFF 
0x10 4 Average bytes per second 1 - 0xFFFFFFFF 
0x14 2 Block align 1 - 65,535 
0x16 2 Significant bits per sample 2 - 65,535 
0x18 2 Extra format bytes 0 - 65,535 
0x1a Extra format bytes * 
 where compression code =
 Code Description 
0 (0x0000) Unknown 
1 (0x0001) PCM/uncompressed 
2 (0x0002) Microsoft ADPCM 
6 (0x0006) ITU G.711 a-law 
7 (0x0007) ITU G.711 µ-law 
17 (0x0011) IMA ADPCM 
20 (0x0016) ITU G.723 ADPCM (Yamaha) 
49 (0x0031) GSM 6.10 
64 (0x0040) ITU G.721 ADPCM 
80 (0x0050) MPEG 
65,536 (0xFFFF) Experimental 

Time Channel Value 
0 1 (left) 0x0053 
  2 (right) 0x0024 
1 1 (left) 0x0057 
  2 (right) 0x0029 
2 1 (left) 0x0063 
  2 (right) 0x003C 
Interlaced Stereo Wave Samples

Fact Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "fact" (0x66616374) 
0x04 4 Chunk Data Size depends on format 
0x08 Format Dependant Data 

Wave List Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "slnt" (0x736C6E74) 
0x04 4 Chunk Data Size depends on size of data and slnt chunks 
0x08 List of Alternating "slnt" and "data" Chunks 

Silent Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "slnt" (0x736C6E74) 
0x04 4 Chunk Data Size 4 
0x08 4 Number of Silent Samples 0 - 0xFFFFFFFF 

Cue Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "cue " (0x63756520) 
0x04 4 Chunk Data Size depends on Num Cue Points 
0x08 4 Num Cue Points number of cue points in list 
0x0c List of Cue Points 

List of Cue Points
A list of cue points is simply a set of consecutive cue point 
descriptions that follow the format described below. 

Cue Point Format 
Offset Size Description Value 
0x00 4 ID unique identification value 
0x04 4 Position play order position 
0x08 4 Data Chunk ID RIFF ID of corresponding data chunk 
0x0c 4 Chunk Start Byte Offset of Data Chunk * 
0x10 4 Block Start Byte Offset to sample of First Channel 
0x14 4 Sample Offset Byte Offset to sample byte of First Channel 

Playlist Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "plst" (0x736C6E74) 
0x04 4 Chunk Data Size num segments * 12 
0x08 4 Number of Segments 1 - 0xFFFFFFFF 
0x0a List of Segments 

Number of Segments
This value specifies the number of following segments in the playlist chunk. 

List of Segments
A list of segments is simply a set of consecutive segment descriptions 
that follow the format described below. The segments do not have to be in 
any particular order because each segments associated cue point position 
is used to determine the play order. 

Segment Format 
Offset Size Description Value 
0x00 4 Cue Point ID 0 - 0xFFFFFFFF 
0x04 4 Length (in samples) 1 - 0xFFFFFFFF 
0x08 4 Number of Repeats 1 - 0xFFFFFFFF 

Associated Data List Chunk - "list"
An associated data list chunk is used to define text labels and names 
which are associated with the cue points to provide each text 
label or name a position. 

Associated Data List Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "list" (0x6C696E74) 
0x04 4 Chunk Data Size depends on contained text 
0x08 4 Type ID "adtl" (0x6164746C) 
0x0c List of Text Labels and Names 

Label Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "labl" (0x6C61626C) 
0x04 4 Chunk Data Size depends on contained text 
0x08 4 Cue Point ID 0 - 0xFFFFFFFF 
0x0c Text 

Note Chunk - "note"
The label chunk is always contained inside of an associated data list chunk. It 
is used to associate a text comment with a Cue Point. This information is 
stored in an identical fashion to the labels in the label chunk. 

Label Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "note" (0x6E6F7465) 
0x04 4 Chunk Data Size depends on contained text 
0x08 4 Cue Point ID 0 - 0xFFFFFFFF 
0x0C Text 

Labeled Text Chunk - "ltxt"
The labeled text chunk is always contained inside of an associated data 
list chunk. It is used to associate a text label with a region or 
section of waveform data. This information is often displayed in marked 
regions of a waveform in digital audio editors. 

Label Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "ltxt" (0x6C747874) 
0x04 4 Chunk Data Size depends on contained text 
0x08 4 Cue Point ID 0 - 0xFFFFFFFF 
0x0c 4 Sample Length 0 - 0xFFFFFFFF 
0x10 4 Purpose ID 0 - 0xFFFFFFFF 
0x12 2 Country 0 - 0xFFFF 
0x14 2 Language 0 - 0xFFFF 
0x16 2 Dialect 0 - 0xFFFF 
0x18 2 Code Page 0 - 0xFFFF 
0x1A Text 

Sampler Chunk - "smpl"
Sampler Chunk Format 
Offset Size Description Value 
0x00 4 Chunk ID "smpl" (0x736D706C) 
0x04 4 Chunk Data Size 36 + (Num Sample Loops * 24) + Sampler Data 
0x08 4 Manufacturer 0 - 0xFFFFFFFF 
0x0C 4 Product 0 - 0xFFFFFFFF 
0x10 4 Sample Period 0 - 0xFFFFFFFF 
0x14 4 MIDI Unity Note 0 - 127 
0x18 4 MIDI Pitch Fraction 0 - 0xFFFFFFFF 
0x1C 4 SMPTE Format 0, 24, 25, 29, 30 
0x20 4 SMPTE Offset 0 - 0xFFFFFFFF 
0x24 4 Num Sample Loops 0 - 0xFFFFFFFF 
0x28 4 Sampler Data 0 - 0xFFFFFFFF 
0x2C List of Sample Loops 

where -
Manufacturer
The value is stored with some extra information to enable translation 
to the value used in a MIDI System Exclusive transmission to the sampler.
The high byte indicates the number of low order bytes (1 or 3) that are 
valid for the manufacturer code. For example, the value for Digidesign 
will be 0x01000013 (0x13) and the value for Microsoft will be 
0x30000041 (0x00, 0x00, 0x41). See the MIDI Manufacturers List for a list.

from : 
The MIDI Manufacturer's Association (MMA) assigns each manufacturer 
of MIDI products a unique ID which is used in conjunction with the 
model ID (assigned by the manufacturer) to identify which product is 
the intended receiver of their system exclusive messages. This way 
other products receiving the same messages can ignore them.

Sampler Chunk Format 
Manufacturer

ID Manufacturer 
0 (0x00) Unknown 
1 (0x01) Sequential Circuits 
2 (0x02) Big Briar 
3 (0x03) Octave / Plateau 
4 (0x04) Moog 
5 (0x05) Passport Designs 
6 (0x06) Lexicon 
7 (0x07) Kurzweil 
8 (0x08) Fender 
9 (0x09) Gulbransen 
10 (0x0A) Delta Labs 
11 (0x0B) Sound Comp. 
12 (0x0C) General Electro 
13 (0x0D) Techmar 
14 (0x0E) Matthews Research 
16 (0x10) Oberheim 
17 (0x11) PAIA 
18 (0x12) Simmons 
19 (0x13) DigiDesign 
20 (0x14) Fairlight 
21 (0x15) JL Cooper 
22 (0x16) Lowery 
23 (0x17) Lin 
24 (0x18) Emu 
27 (0x1B) Peavey 
32 (0x20) Bon Tempi 
33 (0x21) S.I.E.L. 
35 (0x23) SyntheAxe 
36 (0x24) Hohner 
37 (0x25) Crumar 
38 (0x26) Solton 
39 (0x27) Jellinghaus Ms 
40 (0x28) CTS 
41 (0x29) PPG 
47 (0x2F) Elka 
54 (0x36) Cheetah 
62 (0x3E) Waldorf 
64 (0x40) Kawai 
65 (0x41) Roland 
66 (0x42) Korg 
67 (0x43) Yamaha 
68 (0x44) Casio 
70 (0x46) Kamiya Studio 
71 (0x47) Akai 
72 (0x48) Victor 
75 (0x4B) Fujitsu 
76 (0x4C) Sony 
78 (0x4E) Teac 
80 (0x50) Matsushita 
81 (0x51) Fostex 
82 (0x52) Zoom 
84 (0x54) Matsushita 
85 (0x55) Suzuki 
86 (0x56) Fuji Sound 
87 (0x57) Acoustic Technical Laboratory 

   ========================================================= */


/* RIFF Format
   from : http://netghost.narod.ru/gff/graphics/summary/micriff.htm

   Also Known As: RIFF, 
   Resource Interchange File Format, RIFX, .WAV, .AVI, .BND, .RMI, .RDI 

Examples of data that may be stored in RIFF files are: 
Audio/visual interleaved data (.AVI) 
Waveform data (.WAV) 
Bitmapped data (.RDI) 
MIDI information (.RMI) 
A bundle of other RIFF files (.BND) 

 Every RIFF chunk has the following basic structure: 
typedef struct _Chunk
{
    DWORD ChunkId;              /+* Chunk ID marker *+/
    DWORD ChunkSize;            /+* Size of the chunk data in bytes *+/
    BYTE ChunkData[ChunkSize];  /+* The chunk data *+/
} CHUNK;

ChunkId contains four ASCII characters that identify the data the chunk contains.
For example, the characters RIFF are used to identify chunks containing RIFF data.
If an ID is smaller than four characters, it is padded on the right using 
spaces (ASCII 32). Note that RIFF files are written in little-endian byte order.
Files written using the big-endian byte ordering scheme have the identifier RIFX. 

struct _RIFF   /+* "RIFF" *+/
{
    struct _AVICHUNK   /+* "AVI " *+/
    {
        struct _LISTHEADERCHUNK   /+* "hdrl" *+/
        {
            AVIHEADER AviHeader;     /+* "avih" *+/
            struct _LISTHEADERCHUNK  /+* "strl" *+/
            {
                AVISTREAMHEADER	StreamHeader; /+* "strh" *+/
                AVISTREAMFORMAT	StreamFormat; /+* "strf" *+/
                AVISTREAMDATA	StreamData;   /+* "strd" *+/
            }
        }
        struct _LISTMOVIECHUNK  /+* "movi" *+/
        {
            struct _LISTRECORDCHUNK  /+* "rec " *+/
            {
                /+* Subchunk 1 *+/
                /+* Subchunk 2 *+/
                /+* Subchunk N *+/
            }
        }
        struct _AVIINDEXCHUNK  /+* "idx1" *+/
        {
            /+* Index data *+/
        }
    }
}

typedef struct _StreamHeader
{
    char  DataType[4];           /+* Chunk identifier ("strl") *+/
    char  DataHandler[4];        /+* Device handler identifier *+/
    DWORD Flags;                 /+* Data parameters *+/
    DWORD Priority;              /+* Set to 0 *+/
    DWORD InitialFrames;         /+* Number of initial audio frames *+/
    DWORD TimeScale;             /+* Unit used to measure time *+/
    DWORD DataRate;              /+* Data rate of playback *+/
    DWORD StartTime;             /+* Starting time of AVI data *+/
    DWORD DataLength;            /+* Size of AVI data chunk *+/
    DWORD SuggestedBufferSize;   /+* Minimum playback buffer size *+/
    DWORD Quality;               /+* Sample quailty factor *+/
    DWORD SampleSize;            /+* Size of the sample in bytes *+/
} STREAMHEADER;

DataType contains a 4-character identifier indicating the type of data the 
stream header refers to. Identifiers supported by the current version 
of the RIFF format are: vids for video data and auds for audio data. 

typedef struct _AviIndex
{
    DWORD Identifier;    /+* Chunk identifier reference *+/
    DWORD Flags;         /+* Type of chunk referenced *+/
    DWORD Offset;        /+* Position of chunk in file *+/
    DWORD Length;        /+* Length of chunk in bytes *+/
} AVIINDEX;

Identifier contains the 4-byte identifier of the chunk it
references (strh, strf, strd, and so on). 

JUNK Chunk
One other type of chunk that is commonly encountered in an AVI chunk
is the padding or JUNK chunk (so named because its chunk identifier is JUNK).
This chunk is used to pad data out to specific boundaries 
(for example, CD-ROMs use 2048-byte boundaries). The size of the chunk 
is the number of bytes of padding it contains. If you are reading AVI 
data, do not use use the data in the JUNK chunk. Skip it when reading 
and preserve it when writing. The JUNK chunk uses the standard chunk structure: 

typedef struct _JunkChunk
{
    DWORD ChunkId;             /+* Chunk ID marker (JUNK)*+/
    DWORD PaggingSize;         /+* Size of the padding in bytes *+/
    BYTE Padding[ChunkSize];   /+* Padding *+/
} JUNKCHUNK;


 * ------------------------------------------------------------- */

#pragma pack(push,1)
// Header: 12 bytes
typedef struct tagWAVHEADER {
   char chRiff[4];   // 0 - 3  "RIFF" (ASCII Characters)
   DWORD dwFileLen;  // 4 - 7  Total Length Of Package To Follow (Binary, little endian)
   char chWave[4];   // 8 - 11 "WAVE" (ASCII Characters)
}WAVHEADER, * PWAVHEADER;

typedef struct _Chunk
{
    char  ChunkId[4];           /* Chunk ID marker */
    DWORD ChunkSize;            /* Size of the chunk data in bytes */
    // BYTE ChunkData[ChunkSize];  /* The chunk data */
} CHUNK, * PCHUNK;

// CHUNK TYPES
#define  CH_NONE  0
#define  CH_RIFF  1
#define  CH_fmt   2   // Format Chunk - "fmt " 
#define  CH_data  3   // Data Chunk - "data" 
#define  CH_fact  4   // Fact Chunk - "fact" 
#define  CH_cue   5   // Cue Chunk - "cue " 
#define  CH_plst  6   // Playlist Chunk - "plst" 
#define  CH_list  7   // Associated Data List Chunk - "list" 
#define  CH_labl  8   // Label Chunk - "labl" 
#define  CH_ltxt  9   // Labeled Text Chunk - "ltxt"
#define  CH_note  10  // Note Chunk - "note" 
#define  CH_smpl  11  // Sample Chunk - "smpl" 
#define  CH_inst  12  // Instrument Chunk - "inst" 


/* -----------------------------------------
Offset Size Description Value 
0x00 4 Chunk ID "fmt " (0x666D7420) 
0x04 4 Chunk Data Size 16 + extra format bytes 
0x08 2 Compression code 1 - 65,535 
0x0a 2 Number of channels 1 - 65,535 
0x0c 4 Sample rate 1 - 0xFFFFFFFF 
0x10 4 Average bytes per second 1 - 0xFFFFFFFF 
0x14 2 Block align 1 - 65,535 
0x16 2 Significant bits per sample 2 - 65,535 

0x18 2 Extra format bytes 0 - 65,535 
0x1a Extra format bytes * 
OR
Next, the fmt chunk describes the sample format:
  12      4 bytes  'fmt '
  16      4 bytes  0x00000010     // Length of the fmt data (16 bytes)
  20      2 bytes  0x0001         // Format tag: 1 = PCM
  22      2 bytes  <channels>     // Channels: 1 = mono, 2 = stereo
  24      4 bytes  <sample rate>  // Samples per second: e.g., 44100
  28      4 bytes  <bytes/second> // sample rate * block align
  32      2 bytes  <block align>  // channels * bits/sample / 8
  34      2 bytes  <bits/sample>  // 8 or 16

   ----------------------------------------- */
// Format: 24 bytes
typedef struct tagWAVFORMAT {
   char chFmt[4];    // 0 - 3   "fmt " (ASCII Characters)
   DWORD dwFormLen;  // 4 - 7   Length Of FORMAT Chunk (Binary, usually 0x10)
   WORD wComprssion; // 8 - 9   Compression code 1 - 65,535, usually/always 0x01
   WORD wChannels;   // 10 - 11 Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
   DWORD dwRate;     // 12 - 15 Sample Rate (Binary, in Hz)
   DWORD dwBperS;    // 16 - 19 Bytes Per Second
   WORD  wBperS;     // 20 - 21 Bytes Per Sample
   WORD  wBitsperSample;   // 22 - 23 Bits Per Sample
} WAVFORMAT, * PWAVFORMAT;

//Data: 8 bytes + sound data
typedef struct tagWAVDATA {
   char chData[4];   // 0 - 3   "data" (ASCII Characters)
   DWORD dwLen;      // 4 - 7   Length Of Data To Follow
//   char  data[1];    // 8 - end  Data (Samples)
}WAVDATA, * PWAVDATA;

// there are other optional chunks
// Associated Data List Chunk - "list"
// An associated data list chunk is used to define text labels
// and names which are associated with the cue points to provide
// each text label or name a position. 
typedef struct tagWAVLIST {
   // Offset Size Description Value 
   char chList[4];   // 0x00 4 Chunk ID "list" (0x6C696E74) 
   DWORD dwlen;      // 0x04 4 Chunk Data Size depends on contained text 
   char chID[4];     // 0x08 4 Type ID "adtl" (0x6164746C) 
   // 0x0c List of Text Labels and Names 
}WAVLIST, * PWAVLIST;

// Label Chunk Format 
typedef struct tagWAVLTXT {
   // Offset Size Description Value 
   // 0x00 4 Chunk ID "ltxt" (0x6C747874) 
   DWORD Chunk_ID;
   //0x04 4 Chunk Data Size depends on contained text 
   DWORD Chunk_Size;
   // 0x08 4 Cue Point ID 0 - 0xFFFFFFFF 
   DWORD Cue_ID;
   //0x0c 4 Sample Length 0 - 0xFFFFFFFF 
   DWORD Length;
   //0x10 4 Purpose ID 0 - 0xFFFFFFFF 
   DWORD Purpose_ID;
   //0x12 2 Country 0 - 0xFFFF 
   WORD Country;
   //0x14 2 Language 0 - 0xFFFF 
   WORD Language;
   //0x16 2 Dialect 0 - 0xFFFF 
   WORD Dialect;
   //0x18 2 Code Page 0 - 0xFFFF 
   WORD  Code_page;
   //0x1A Text 
}WAVLTXT, * PWAVLTXT;

typedef struct tagWAVFILE {
   WAVHEADER   sWavHeader;
   WAVFORMAT   sWavFormat;
   WAVDATA     sWavData;
}WAVFILE, * PWAVFILE;

/* AVI Header Subchunk
   The first mandatory LIST chunk contains the main AVI header 
   subchunk and has the identifier hdrl. The information in the header 
   subchunk defines the format of the entire AVI chunk. The hdrl 
   chunk must appear as the first chunk within the AVI chunk. 
   The format of the header subchunk is the following: 
 * ------------------------------------------------------------- */
typedef struct _AVIHeader
{
    DWORD TimeBetweenFrames;     /* Time delay between frames */
    DWORD MaximumDataRate;       /* Data rate of AVI data */
    DWORD PaddingGranularity;    /* Size of single unit of padding */
    DWORD Flags;                 /* Data parameters */
    DWORD TotalNumberOfFrames;   /* Number of video frame stored */
    DWORD NumberOfInitialFrames; /* Number of preview frames */
    DWORD NumberOfStreams;       /* Number of data streams in chunk*/
    DWORD SuggestedBufferSize;   /* Minimum playback buffer size */
    DWORD Width;                 /* Width of video frame in pixels */
    DWORD Height;                /* Height of video frame in pixels*/
    DWORD TimeScale;             /* Unit used to measure time */
    DWORD DataRate;              /* Data rate of playback */
    DWORD StartTime;             /* Starting time of AVI data */
    DWORD DataLength;            /* Size of AVI data chunk */
} AVIHEADER;


// Sampler Chunk - "smpl"
// Sampler Chunk Format 
typedef struct _WAVSMPL {
   DWORD Chunk_ID;   // "smpl" (0x736D706C) 
   DWORD Chunk_Size; // 0x04 4 Chunk Data Size 36 + (Num Sample Loops * 24) + Sampler Data 
   DWORD Manufacturer;  // 0x08 4 Manufacturer 0 - 0xFFFFFFFF 
   DWORD Product;    // 0x0C 4 Product 0 - 0xFFFFFFFF 
   DWORD Samp_Period;   // 0x10 4 Sample Period 0 - 0xFFFFFFFF 
   DWORD MIDI_Unit;  // 0x14 4 MIDI Unity Note 0 - 127 
   DWORD MIDI_Pitch; // 0x18 4 MIDI Pitch Fraction 0 - 0xFFFFFFFF 
   DWORD SMPTE_Form; // 0x1C 4 SMPTE Format 0, 24, 25, 29, 30 
   DWORD SMPTE_Off;  // 0x20 4 SMPTE Offset 0 - 0xFFFFFFFF 
   DWORD Samp_Loops; // 0x24 4 Num Sample Loops 0 - 0xFFFFFFFF 
   DWORD Samp_Data;  // 0x28 4 Sampler Data 0 - 0xFFFFFFFF 
   // 0x2C List of Sample Loops 
}WAVSMPL, * PWAVSMPL;

#pragma pack(pop)

#define  wavChan_Mono   1
#define  wavChan_Stereo 2


typedef struct tagVAL2STG {
   DWORD value;
   char * stg;
}VAL2STG, * PVAL2STG;

VAL2STG sChan2Stg[] = {
   { wavChan_Mono, "Mono" },
   { wavChan_Stereo, "Stereo" },
   { 0 , 0 }
};

char * Get_Channel_String( DWORD val )
{
   PVAL2STG pv2s = &sChan2Stg[0];
   LPTSTR lptmp = &gszTmpOut[0];
   while(pv2s->stg)
   {
      if(pv2s->value == val )
         return pv2s->stg;

      pv2s++;
   }
   sprintf(lptmp, "Unknown %d", val);
   return lptmp;
}

/* ===============================
   THIS DOES NOT SEEM CORRECT!!!
// 20 - 21 Bytes Per Sample:
#define  wav_8_bit_Mono    1
#define  wav_8_bit_Stereo  2
#define  wav_16_bit_Mono   2
#define  wav_16_bit_Stereo 4

// 20 - 21 Bytes Per Sample:
VAL2STG  sBPS[] = {
   { wav_8_bit_Mono,  "8-bit Mono" },
   { wav_8_bit_Stereo,"8-bit Stereo" },
   { wav_16_bit_Mono, "16-bit Mono" },
   { wav_16_bit_Stereo, "16-bit Stereo" },
   { 0,  0 }
};

char * Get_BPS_String( DWORD val )
{
   PVAL2STG pv2s = &sBPS[0];
   LPTSTR lptmp = &gszTmpOut[0];
   while(pv2s->stg)
   {
      if(pv2s->value == val )
         return pv2s->stg;

      pv2s++;
   }
   sprintf(lptmp, "Unknown %d", val);
   return lptmp;
}
  ================================================= */

VAL2STG  sCompression[] = {
   { 0, "Unknown" },
   { 1, "PCM/uncompressed" },
   { 2, "Microsoft ADPCM" },
   { 6, "ITU G.711 a-law" },
   { 7, "ITU G.711 µ-law" },
   { 17, "IMA ADPCM" },
   { 20, "ITU G.723 ADPCM (Yamaha)" },
   { 49, "GSM 6.10" },
   { 64, "ITU G.721 ADPCM" },
   { 80, "MPEG" },
   { 0xFFFF, "Experimental" },
   { 0, 0 }
};

char * Get_Compression_String( DWORD val )
{
   PVAL2STG pv2s = &sCompression[0];
   LPTSTR lptmp = &gszTmpOut[0];
   while(pv2s->stg)
   {
      if(pv2s->value == val )
         return pv2s->stg;
      pv2s++;
   }
   sprintf(lptmp, "Undefined %d", val);
   return lptmp;
}

// Sampler Chunk Format 
// Manufacturer
VAL2STG  sManufacturer[] = {
   { 0, "Unknown" },
   { 1, "Sequential Circuits" },
   { 2, "Big Briar" },
   { 3, "Octave/Plateau" },
   { 4, "Moog" },
   { 5, "Passport Designs" },
   { 6, "Lexicon" },
   { 7, "Kurzweil" },
   { 8, "Fender" },
   { 9, "Gulbransen" },
   { 10, "Delta Labs" },
   { 11, "Sound Comp." },
   { 12, "General Electro" },
   { 13, "Techmar" },
   { 14, "Matthews Research" },
   { 16, "Oberheim" },
   { 17, "PAIA" },
   { 18, "Simmons" },
   { 19, "DigiDesign" },
   { 20, "Fairlight" },
   { 21, "JL Cooper" },
   { 22, "Lowery" },
   { 23, "Lin" },
   { 24, "Emu" },
   { 27, "Peavey" },
   { 32, "Bon Tempi" },
   { 33, "S.I.E.L." },
   { 35, "SyntheAxe" },
   { 36, "Hohner" },
   { 37, "Crumar" },
   { 38, "Solton" },
   { 39, "Jellinghaus Ms" },
   { 40, "CTS" },
   { 41, "PPG" },
   { 47, "Elka" },
   { 54, "Cheetah" },
   { 62, "Waldorf" },
   { 64, "Kawai" },
   { 65, "Roland" },
   { 66, "Korg" },
   { 67, "Yamaha" },
   { 68, "Casio" },
   { 70, "Kamiya Studio" },
   { 71, "Akai" },
   { 72, "Victor" },
   { 75, "Fujitsu" },
   { 76, "Sony" },
   { 78, "Teac" },
   { 80, "Matsushita" },
   { 81, "Fostex" },
   { 82, "Zoom" },
   { 84, "Matsushita" },
   { 85, "Suzuki" },
   { 86, "Fuji Sound" },
   { 87, "Acoustic Technical Laboratory" },
   { 0, 0 }
};

char * Get_Manufacturer_String( DWORD val )
{
   PVAL2STG pv2s = &sManufacturer[0];
   LPTSTR lptmp = &gszTmpOut[0];
   while(pv2s->stg)
   {
      if(pv2s->value == val )
         return pv2s->stg;
      pv2s++;
   }
   sprintf(lptmp, "Unlisted value %d (%#X)", val, val);
   return lptmp;
}

int  Known_Chunk_ID( void * pv )
{
   PCHUNK pc = (PCHUNK)pv;
   if((pc->ChunkId[0] == 'R')&&
      (pc->ChunkId[1] == 'I')&&
      (pc->ChunkId[2] == 'F')&&
      (pc->ChunkId[3] == 'F'))
      return CH_RIFF;
   // Format Chunk - "fmt " 
   else if((pc->ChunkId[0] == 'f')&&
      (pc->ChunkId[1] == 'm')&&
      (pc->ChunkId[2] == 't'))
      return CH_fmt;
   // Data Chunk - "data" 
   else if((pc->ChunkId[0] == 'd')&&
      (pc->ChunkId[1] == 'a')&&
      (pc->ChunkId[2] == 't')&&
      (pc->ChunkId[3] == 'a'))
      return CH_data;
   // Fact Chunk - "fact" 
   else if((pc->ChunkId[0] == 'f')&&
      (pc->ChunkId[1] == 'a')&&
      (pc->ChunkId[2] == 'c')&&
      (pc->ChunkId[3] == 't'))
      return CH_fact;
   // Cue Chunk - "cue " 
   else if((pc->ChunkId[0] == 'c')&&
      (pc->ChunkId[1] == 'u')&&
      (pc->ChunkId[2] == 'e'))
      return CH_cue;
   // Playlist Chunk - "plst" 
   else if((pc->ChunkId[0] == 'p')&&
      (pc->ChunkId[1] == 'l')&&
      (pc->ChunkId[2] == 's')&&
      (pc->ChunkId[3] == 't'))
      return CH_plst;
   // Associated Data List Chunk - "list" 
   else if((pc->ChunkId[0] == 'l')&&
      (pc->ChunkId[1] == 'i')&&
      (pc->ChunkId[2] == 's')&&
      (pc->ChunkId[3] == 't'))
      return CH_list;
   // Label Chunk - "labl" 
   else if((pc->ChunkId[0] == 'l')&&
      (pc->ChunkId[1] == 'a')&&
      (pc->ChunkId[2] == 'b')&&
      (pc->ChunkId[3] == 'l'))
      return CH_labl;
   // Labeled Text Chunk - "ltxt"
   else if((pc->ChunkId[0] == 'l')&&
      (pc->ChunkId[1] == 't')&&
      (pc->ChunkId[2] == 'x')&&
      (pc->ChunkId[3] == 't'))
      return CH_ltxt;
   // Note Chunk - "note" 
   else if((pc->ChunkId[0] == 'n')&&
      (pc->ChunkId[1] == 'o')&&
      (pc->ChunkId[2] == 't')&&
      (pc->ChunkId[3] == 'e'))
      return CH_note;
   // Sample Chunk - "smpl" 
   else if((pc->ChunkId[0] == 's')&&
      (pc->ChunkId[1] == 'm')&&
      (pc->ChunkId[2] == 'p')&&
      (pc->ChunkId[3] == 'l'))
      return CH_smpl;
   // Instrument Chunk - "inst" 
   else if((pc->ChunkId[0] == 'i')&&
      (pc->ChunkId[1] == 'n')&&
      (pc->ChunkId[2] == 's')&&
      (pc->ChunkId[3] == 't'))
      return CH_inst;

   return CH_NONE;
}

BOOL  Valid_WAV_data( PWAVDATA pwd )
{
   if(( pwd->chData[0] == 'd' )&&
      ( pwd->chData[1] == 'a' )&&
      ( pwd->chData[2] == 't' )&&
      ( pwd->chData[3] == 'a' ))
   {
      return TRUE;
   }
   return FALSE;
}

BOOL  Valid_WAV_format( PWAVFORMAT pwf )
{
   if(( pwf->chFmt[0] == 'f' )&&
      ( pwf->chFmt[1] == 'm' )&&
      ( pwf->chFmt[2] == 't' )&&
      ( pwf->dwFormLen >= 0x10))
      // ( pwf->wCompression == 1) MS PCM)
   {
      return TRUE;
   }
   return FALSE;
}

BOOL  Valid_WAV_header( PWAVHEADER pwh )
{
   if(( pwh->chRiff[0] == 'R' )&&
      ( pwh->chRiff[1] == 'I' )&&
      ( pwh->chRiff[2] == 'F' )&&
      ( pwh->chRiff[3] == 'F' )&&
      ( pwh->chWave[0] == 'W' )&&
      ( pwh->chWave[1] == 'A' )&&
      ( pwh->chWave[2] == 'V' )&&
      ( pwh->chWave[3] == 'E' ))
   {
      return TRUE;
   }
   return FALSE;
}

int  IsWAVFile( LPDFSTR lpdf, LPTSTR lpf, PBYTE pb, DWORD dwmax )
{
   int  bRet = 0;
   PWAVHEADER pwh = (PWAVHEADER)pb;
   PWAVFORMAT pwf = (PWAVFORMAT)( pwh + 1 );
   PWAVDATA   pwd = (PWAVDATA)( pwf + 1 );
   DWORD    diff;
   PBYTE    ptmp = pb;
   PBYTE    tmp;
   if( dwmax > (sizeof(WAVFILE)) ) {
      int   chid = Known_Chunk_ID(ptmp);
      if( (chid == CH_RIFF) && Valid_WAV_header( pwh ) )
      {
         ptmp = (PBYTE)((PWAVHEADER)pwh + 1);
         chid = Known_Chunk_ID(ptmp);
         if(VERB) {
            sprtf( "Got 'RIFF'/'WAVE' header, File length = %d",
               pwh->dwFileLen );
            if( pwh->dwFileLen == (dwmax - 8) )
               sprtf(" ok\n");
            else
               sprtf( ", versus = %d\n", (dwmax - 8) );
         }
         if( (chid == CH_fmt) && Valid_WAV_format( pwf ) )
         {
            ptmp = (PBYTE)((PWAVFORMAT)pwf + 1);
            if(VERB)
               sprtf( "Got 'fmt' block of length %#x, with reserved=1\n",
               pwf->dwFormLen );
            if( pwf->dwFormLen > 0x10 )
            {
               tmp = (PBYTE)pwd;
               diff = pwf->dwFormLen - 0x10;
               tmp += diff;
               if(VERB9) {
                  sprtf( "Got 'fmt' dwFormLen of %d (%#x), instead of %d (%#x)\n",
                     pwf->dwFormLen, pwf->dwFormLen, 0x10, 0x10 );
                  ProcessHex((PBYTE)pwf, sizeof(WAVFORMAT));
                  sprtf( "Followed by ...\n" );
                  ProcessHex((PBYTE)pwd, diff);
               }
               pwd = (PWAVDATA)tmp;
               ptmp = tmp;
            }
            chid = Known_Chunk_ID(ptmp);
Do_DATA:
            if( (chid == CH_data) && Valid_WAV_data( pwd ) ) {
               if(VERB) {
                  sprtf( "Got 'data' block, of length %d ...\n",
                     pwd->dwLen );
               }
               bRet = 1;
            } else {
               while( chid )
               {
                  PCHUNK pch = (PCHUNK)ptmp;
                  diff = pch->ChunkSize + sizeof(CHUNK);
                  if(VERB) {
                     sprtf( "Got 'chunk', of length %d ...\n", diff );
                     ProcessHex((PBYTE)pwd, diff);
                  }
                  ptmp += diff;
                  chid = Known_Chunk_ID(ptmp);
                  if( chid == CH_data ) {
                     pwd = (PWAVDATA)ptmp;
                     goto Do_DATA;
                  }
               }
               if(VERB) {
                  sprtf( "Failed on data block - got -\n" );
                  ProcessHex((PBYTE)pwd, sizeof(WAVDATA));
               }
            }
         }
         else
         {
            if(VERB) {
               sprtf( "Failed on 'format' block - got -\n" );
               ProcessHex((PBYTE)pwf, sizeof(WAVFORMAT));
            }
         }
      }
      else
      {
         if(VERB) {
            sprtf( "Failed on 'header' block - got -\n" );
            ProcessHex((PBYTE)pwh, sizeof(WAVHEADER));
         }
      }
   }
   else
   {
      if(VERB) {
         sprtf( "Failed on size - got %d, need more than %d!\n",
            dwmax, sizeof(WAVFILE) );
      }
   }
   if( !bRet && VERB)
      sprtf( "Reverting to simple HEX output ...\n" );
   return bRet;
}

/* For byte swapping on little-endian systems (GSHHS is defined to be bigendian) */
#define swabi4(i4) (((i4) >> 24) + (((i4) >> 8) & 65280) + (((i4) & 65280) << 8) + (((i4) & 255) << 24))

VOID  DumpSMPL( PWAVSMPL pws )
{
   DWORD dwi, val;
   WORD  wi;
   BYTE  b;
   dwi = swabi4(pws->Chunk_ID);
   sprtf("Chunk ID        : %#X (smpl=0x736D706C)\n", dwi ); // "smpl" (0x736D706C) 
   sprtf("Chunk Data Size : %d\n", pws->Chunk_Size ); // 0x04 4 Chunk Data Size 36 + (Num Sample Loops * 24) + Sampler Data 
   // The value is stored with some extra information to enable translation
   // to the value used in a MIDI System Exclusive transmission to the sampler.
   // The high byte indicates the number of low order bytes (1 or 3) that are 
   // valid for the manufacturer code. 
   // For example, the value for Digidesign will be 0x01000013 (0x13) and 
   // the value for Microsoft will be 0x30000041 (0x00, 0x00, 0x41).
   // See the MIDI Manufacturers List for a list.
   // Assume bigendians and swap to little-endiands
   dwi = swabi4(pws->Manufacturer);
   wi = HIWORD(dwi);
   b = HIBYTE(wi);
   if(b == 1)
      val = dwi & 0x0ff;
   else if(b == 2)
      val = dwi & 0x0ffff;
   else if(b == 3)
      val = dwi & 0x0ffffff;
   else
      val = dwi & 0x0ff;   // ok, default to a BYTE only

   sprtf("Manufacturer    : %s (%#x)\n", Get_Manufacturer_String(val), dwi ); // 0x08 4 Manufacturer 0 - 0xFFFFFFFF 
   // DWORD Product;    // 0x0C 4 Product 0 - 0xFFFFFFFF 
   sprtf("Product         : %d\n", pws->Product );
   //DWORD Samp_Period;   // 0x10 4 Sample Period 0 - 0xFFFFFFFF 
   sprtf("Sample Period   : %d\n", pws->Samp_Period );
   //DWORD MIDI_Unit;  // 0x14 4 MIDI Unity Note 0 - 127 
   sprtf("MIDI Unit Note  : %d\n", pws->MIDI_Unit );
   //DWORD MIDI_Pitch; // 0x18 4 MIDI Pitch Fraction 0 - 0xFFFFFFFF 
   sprtf("MIDI Pitch      : %d\n", pws->MIDI_Pitch );
   //DWORD SMPTE_Form; // 0x1C 4 SMPTE Format 0, 24, 25, 29, 30 
   sprtf("SMPTE Format    : %d\n", pws->SMPTE_Form );
   //DWORD SMPTE_Off;  // 0x20 4 SMPTE Offset 0 - 0xFFFFFFFF 
   sprtf("SMPTE Offset    : %d\n", pws->SMPTE_Off );
   //DWORD Samp_Loops; // 0x24 4 Num Sample Loops 0 - 0xFFFFFFFF 
   sprtf("Sample Loops    : %d\n", pws->Samp_Loops );
   //DWORD Samp_Data;  // 0x28 4 Sampler Data 0 - 0xFFFFFFFF 
   sprtf("Sample Data     : %d\n", pws->Samp_Data );
   // 0x2C List of Sample Loops 

}

void  DumpLTXT( PWAVLTXT plt )
{
   DWORD dwi;
   WORD  wi;
   // Label Chunk Format 
   // typedef struct tagWAVLTXT {
   // Offset Size Description Value 
   // 0x00 4 Chunk ID "ltxt" (0x6C747874) 
   dwi = swabi4(plt->Chunk_ID);
   sprtf("Chunk ID        : %#X (ltxt=0x6C747874)\n", dwi );
   //0x04 4 Chunk Data Size depends on contained text 
   dwi = plt->Chunk_Size;
   sprtf("Chunk Size      : %d\n", dwi );
   // 0x08 4 Cue Point ID 0 - 0xFFFFFFFF 
   dwi = plt->Cue_ID;
   sprtf("Cue Point ID    : %d\n", dwi );
   //0x0c 4 Sample Length 0 - 0xFFFFFFFF 
   dwi = plt->Length;
   sprtf("Length          : %d\n", dwi );
   //0x10 4 Purpose ID 0 - 0xFFFFFFFF 
   dwi = plt->Purpose_ID;
   sprtf("Purpose ID      : %d\n", dwi );
   //0x12 2 Country 0 - 0xFFFF 
   wi = plt->Country;
   sprtf("Country         : %d\n", wi );
   //0x14 2 Language 0 - 0xFFFF 
   wi = plt->Language;
   sprtf("Language        : %d\n", wi );
   //0x16 2 Dialect 0 - 0xFFFF 
   wi = plt->Dialect;
   sprtf("Dialect         : %d\n", wi );
   //0x18 2 Code Page 0 - 0xFFFF 
   wi = plt->Code_page;
   sprtf("Code Page       : %d\n", wi );
   //0x1A Text 
}  // WAVLTXT, * PWAVLTXT;

BOOL  DumpWAV( LPDFSTR lpdf )
{
   BOOL  bRet = TRUE;
   BYTE * p = (BYTE *)lpdf->df_pVoid;
   DWORD dwm = lpdf->dwmax;   // FULL file size
   PWAVHEADER pwh = (PWAVHEADER)p;
   PWAVFORMAT pwf = (PWAVFORMAT)( pwh + 1 );
   PWAVDATA   pwd = (PWAVDATA)( pwf + 1 );
   DWORD dwmax = dwm - sizeof(WAVFILE);  // size already checked to be greater
   DWORD len = pwd->dwLen;
   BYTE  maxc, minc, c;
   DWORD * pdw = &gdwColCount[0];
   DWORD dwi = pwf->dwFormLen - 0x10;
   double   tm;
   int   chid;
   PBYTE tmp;
   PCHUNK   pch;
   DWORD    diff;

   // if there are EXTRA format bytes, then ...
   if(dwi) {
      tmp = (PBYTE)pwd;
      tmp += dwi;
      pwd = (PWAVDATA)tmp;
      len = pwd->dwLen;
   }

   sprtf("Compression     : %s\n", Get_Compression_String( pwf->wComprssion ));
   //WORD wChannels;   // 10 - 11  Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
   sprtf("Channels        : %s\n", Get_Channel_String( pwf->wChannels )); //= wavChan_Mono;
   // DWORD dwRate;     // 12 - 15 Sample Rate (Binary, in Hz)
   sprtf("Sample Rate     : %u Hz.\n", pwf->dwRate );
   // DWORD dwBperS;    // 16 - 19 Bytes Per Second
   sprtf("Bytes per Second: %d.\n", pwf->dwBperS );
   //WORD  wBperS;     // 20 - 21 Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo
   // THIS DOES NOT APPEAR CORRECT, so just put out the value
   //sprtf("Bytes per Sample: %s.\n", Get_BPS_String(pwf->wBperS) );
   sprtf("Bytes per Sample: %d.\n", pwf->wBperS );
   // WORD  wBitsperSample;   // 22 - 23 Bits Per Sample
   sprtf("Bits Per Sample : %d.\n", pwf->wBitsperSample );

   if( dwi )
      sprtf("Additional fmt  : %d.\n", dwi );

   tmp = (PBYTE)pwd;
   chid = Known_Chunk_ID(tmp);
   while( chid )
   {
      if( chid == CH_data ) {
         break;
      } else if( chid == CH_smpl ) {
         if(VERB) {
            sprtf("Dump of 'smpl' chunk ...\n" );
            DumpSMPL( (PWAVSMPL) tmp );
         }
      } else if( chid == CH_ltxt ) {
         if(VERB) {
            sprtf("Dump of 'ltxt' chunk ...\n" );
            DumpLTXT( (PWAVLTXT) tmp );
         }
      }
      pch = (PCHUNK)tmp;
      diff = pch->ChunkSize + sizeof(CHUNK);
      tmp += diff;
      chid = Known_Chunk_ID(tmp);
      if( chid == CH_data ) {
         pwd = (PWAVDATA)tmp;
         len = pwd->dwLen;
      }
   }

   sprtf("Followed by %d bytes of data. %s\n", pwd->dwLen,
      ( VERB2 ? "" : "(add -v2+ to view)" ));
   if( pwf->dwBperS != 0 ) {
      tm = (double)pwd->dwLen / (double)pwf->dwBperS;
      sprtf("Estimates Time  : %f seconds.\n", tm );
   } else {
      sprtf("Estimates Time  : Bytes per Second is ZERO!\n" );
   }

   p = (BYTE *)( pwd + 1 );   // get data pointer
   if( len > dwmax ) {
      sprtf("Indicated length of data %d GREATER than remainder %d bytes!\n",
         len, dwmax );
      len = dwmax;
   }
   if(VERB2) {
      sprtf("Hex output of data %d bytes ... \n", len );
      if(VERB3) {
         minc = 0xff;
         maxc = 0;
         for(dwi = 0; dwi < 256; dwi++)
            pdw[dwi] = 0;

         for(dwi = 0; dwi < len; dwi++) {
            c = p[dwi];
            if(c > maxc)
               maxc = c;
            if(c < minc)
               minc = c;
            pdw[c]++;
         }
         sprtf("Data has range %x (%d) to %x (%d) ...\n",
            minc, minc, maxc, maxc );
      }
      ProcessHex( p, len );
      if( dwmax > len ) {
         p += len;
         len = dwmax - len;
         sprtf("Hex of remainder of file %d bytes ... \n", len );
         ProcessHex( p, len );
      }
   }
   if(VERB8) {
      DWORD dwmin = -1;
      DWORD dwmax = 0;
      DWORD dwmin2 = -1;
      DWORD dwmax2 = 0;
      DWORD dwcnt;
      sprtf( "Byte value frequency table ... " );
      if(VERB9)
         sprtf("all\n");
      else {
         len = 0;
         for(dwi = 0; dwi < 256; dwi++) {
            dwcnt = pdw[dwi];
            if(dwcnt)
               len++;
         }
         sprtf( "%d non zero only.\n", len);
      }
      len = 0;
      for(dwi = 0; dwi < 256; dwi++) {
         dwcnt = pdw[dwi];
         if(dwcnt > dwmax)
            dwmax = dwcnt;
         if(dwcnt < dwmin)
            dwmin = dwcnt;
         if(VERB9 || dwcnt) {
            sprtf( "%02X: %06u ", dwi, dwcnt );
            len++;
         }
         if(len == 8) {
            len = 0;
            sprtf("\n");
         }
      }
      if(len)
         sprtf("\n");
      for(dwi = 0; dwi < 256; dwi++) {
         dwcnt = pdw[dwi];
         if((dwcnt > dwmax2)&&(dwcnt < dwmax))
            dwmax2 = dwcnt;
         if((dwcnt < dwmin2)&&(dwcnt > dwmin))
            dwmin2 = dwcnt;
      }
      sprtf("Max frequency = %u, %u, min = %u, %u\n", dwmax, dwmax2, dwmin2, dwmin);
   }
   return bRet;
}

BOOL  ProcessWAV( LPDFSTR lpdf )
{
   BOOL  bRet = FALSE;
   if( IsWAVFile( lpdf, lpdf->fn, lpdf->lpb, lpdf->dwmax ) )
   {
      bRet = TRUE;
      if(VERB)
         bRet = DumpWAV( lpdf );
   }
   return bRet;
}

// eof - DumpWAV.c
