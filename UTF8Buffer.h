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

#endif
