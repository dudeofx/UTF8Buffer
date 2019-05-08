// Utility Library to handle UTF-8 Text Buffers

#ifndef __UTF8BUFFER_H__
#define __UTF8BUFFER_H__

#ifndef __SUB_STRINGS__
#define __SUB_STRINGS__

typedef struct _tSubString tSubString;

struct _tSubString {
   int anchor;
   int length;
};

#endif

#define UTF8_REMOVE_INVALID   0x80
#define UTF8_SPEC_STANDARD    0x00
#define UTF8_JUST_DECODE      0x01
#define UTF8_SPEC_XML         0x02

#define UTF8_SPEC_DEFAULT  (UTF8_SPEC_STANDARD | UTF8_REMOVE_INVALID)

#define UTF8_DECODE_FAIL      0xFFFFFFFF

typedef struct _tUTF8Buffer tUTF8Buffer;

struct _tUTF8Buffer {
   tSubString cursor;
   int count;
   int spec;

   void  (*Reset)(tUTF8Buffer *);
   int   (*FindFirst)(tUTF8Buffer *, const char *);
   int   (*FindNext)(tUTF8Buffer *, const char *);
   int   (*SelectUntil)(tUTF8Buffer *, const char *);
   void  (*Copy)(tUTF8Buffer *, char *, int);
   void  (*ClearLt)(tUTF8Buffer *);
   void  (*ClearRt)(tUTF8Buffer *);

   int   buff_size;
   char  buff[1];
};

unsigned int UTF8Buffer_Decode(const char *, int);
tUTF8Buffer *UTF8Buffer_LoadFile(const char *, int);
tUTF8Buffer *UTF8Buffer_FromStr(char *, int);
void         UTF8Buffer_Destroy(tUTF8Buffer **);
int          UTF8Buffer_IsValid(char *, int);

static const unsigned char __byte_count[] = {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x00 .. 0x0F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x10 .. 0x1F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x20 .. 0x2F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x30 .. 0x3F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x40 .. 0x4F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x50 .. 0x5F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x60 .. 0x6F
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x70 .. 0x7F

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x80 .. 0x8F // error
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x90 .. 0x9F // error
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xA0 .. 0xAF // error
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xB0 .. 0xBF // error
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 0xC0 .. 0xCF // 
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 0xD0 .. 0xDF // 
   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 0xE0 .. 0xEF // 
   4, 4, 4, 4, 4, 4, 4, 4,                          // 0xF0 .. 0xF7 // 
   5, 5, 5, 5, 6, 6, 0, 0,                          // 0xF7 .. 0xFF // 
};

#endif
