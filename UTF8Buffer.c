#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "UTF8Buffer.h"

static const unsigned int __lower_limit[] = {
   0xFFFFFFFF, // invalid should never happen
   0x00000000,
   0x00000080,
   0x00000800,
   0x00010000,
   0x00200000,
   0x04000000,
};

static const unsigned char __mask[] = {
   0x00, // invalid should never happen
   0x7F,
   0x1F,
   0x0F,
   0x07,
   0x03,
   0x01,
};

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


static const unsigned char __invalid_utf8_char[] = { 0xef, 0xbf, 0xbd };

//======================================================================================
// Private Functions
//--------------------------------------------------------------------------------------
// returns:
//     1 if its a valid codepoint
//    -1 if its invalid
static int ValidXMLCodePoint(int codept){
   static const unsigned int _valid_singles[4] = { 0x9, 0xA, 0xD, 0x85 };
   static const unsigned int _valid_ranges[8] = {
      0x0020,   0x007E,
      0x00A0,   0x07FF,
      0xE000,   0xFDCF,
      0xFDF0,   0xFFFD,
   };
   int i;
   int lower_bound;
   int upper_bound;
   
   for (i = 0; i < 4; i++) {
      if (_valid_singles[i] == codept) return 1;
   }
   
   for (i = 0; i < 4; i++) {
      if ((codept >= _valid_ranges[i*2]) && (codept <= _valid_ranges[i*2+1])) return 1;
   }
   
   lower_bound = 0x010000;
   upper_bound = 0x01FFFD;
   for (i = 0; i < 16; i++) {
      if ((codept >= lower_bound) && (codept <= upper_bound)) return 1;
      lower_bound += 0x010000;
      upper_bound += 0x010000;
   }
   
   return -1;
}
//--------------------------------------------------------------------------------------
static int Examine(unsigned char *st, int size, int spec) {
   // Checks string if its malformed based on spec
   // returns recommended new size of buffer to store a sanatized version of it
   unsigned char *src = st;
   int new_size = 0;
   int byte_count = 0;
   int byte_size = 0;
   int err_count = 0;
   int verdict = 0;
   unsigned int codept = 0;
   int insert_error_char = 0;
   
   if ((spec & UTF8_REMOVE_INVALID) == 0) insert_error_char = 1;
   spec &= 0x7f;

   while (size != 0) {
      if (byte_count == 0) {
         byte_size = byte_count = __byte_count[*src];
         if (byte_count == 0) { // error found, orphaned continuation byte, drop it
            err_count++;
         } else {
            codept = (*src & __mask[byte_size]);
            byte_count--;
            if (byte_count == 0) {
               verdict = 1;
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                     break;
               }

               if (verdict > 0) {
                  new_size++;
               } else {
                  err_count++;
               }
            }
         }
      } else {
         if ((0xC0 & *src) == 0x80) {
            codept = ((codept << 6) | (0x3F & *src)) ;
            byte_count--;
            verdict = 1;
            if (byte_count == 0) {
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                  case UTF8_SPEC_STANDARD:
                     if (codept > 0x10FFFF) verdict = -1;
                     if ((codept >= 0xD800) && (codept <= 0xDFFF)) verdict = -1;
                  case UTF8_JUST_DECODE:
                     if (codept < __lower_limit[byte_size]) verdict = -1;
                     break;
               }

               if (verdict > 0) {
                  new_size += byte_size;
               } else {
                  err_count++;
               }
            }
         } else {  // error found: expected more continuation bytes
            err_count++;
            byte_count = 0;
            continue; // do not increment fowards retry current byte as new sequence
         }
      }
      src++;
      size--;
   }

   // new buffer will be padded with a null byte hence the +1
   if (insert_error_char != 0) new_size += err_count*3;
   return new_size+1;  
}
//--------------------------------------------------------------------------------------
static int Sanatize(unsigned char *dst, int dst_size, unsigned char *src, int src_size, int spec) {
   // produces a sanatized version of a possibly malformed string
   // returns number of codepoints
   int byte_count = 0;
   int byte_size = 0;
   int verdict = 0;
   unsigned int codept = 0;
   unsigned char *mrk = src;
   int insert_error_char = 0;
   int num_codept = 0;
   int i;
   
   if ((spec & UTF8_REMOVE_INVALID) == 0) insert_error_char = 1;
   spec &= 0x7f;
   
   if (dst_size <= 1) return -1;
   dst_size--; // we will put a null terminator no matter what

   while (src_size != 0) {
      if (byte_count == 0) {
         mrk = src;
         byte_size = byte_count = __byte_count[*src];
         if (byte_count == 0) { // error found, orphaned continuation byte, drop it and insert invalid
            if (insert_error_char != 0) {
               if (dst_size < 3) break;
               dst_size -= 3;
               for (i = 0; i < 3; i++) *(dst++) = __invalid_utf8_char[i];
            }
         } else {
            codept = (*src & __mask[byte_size]);
            byte_count--;
            if (byte_count == 0) {
               verdict = 1;
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                     break;
               }

               if (verdict > 0) {
                  if (dst_size < 1) break;
                  dst_size--;
                  *(dst++) = *(mrk++);
                  num_codept++;
               } else {
                  if (insert_error_char != 0) {
                     if (dst_size < 3) break;
                     dst_size -= 3;
                     for (i = 0; i < 3; i++) *(dst++) = __invalid_utf8_char[i];
                  }
               }
            }
         }
      } else {
         if ((0xC0 & *src) == 0x80) {
            codept = ((codept << 6) | (0x3F & *src)) ;
            byte_count--;
            if (byte_count == 0) {
               verdict = 1;
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                  case UTF8_SPEC_STANDARD:
                     if (codept > 0x10FFFF) verdict = -1;
                     if ((codept >= 0xD800) && (codept <= 0xDFFF)) verdict = -1;
                  case UTF8_JUST_DECODE:
                     if (codept < __lower_limit[byte_size]) verdict = -1;
                     break;
               }
               if (verdict > 0) {
                  if (dst_size < byte_size) break;
                  dst_size -= byte_size;               
                  for (i = 0; i < byte_size; i++) *(dst++) = *(mrk++);
                  num_codept++;
               } else {
                  if (insert_error_char != 0) {
                     if (dst_size < 3) break;
                     dst_size -= 3;
                     for (i = 0; i < 3; i++) *(dst++) = __invalid_utf8_char[i];
                  }
               }
            }
         } else {  // error found: expected more continuation bytes
            if (insert_error_char != 0) {
               if (dst_size < 3) break;
               dst_size -= 3;
               for (i = 0; i < 3; i++) *(dst++) = __invalid_utf8_char[i];
            }
            byte_count = 0;
            continue; // do not increment fowards retry current byte as new sequence
         }
      }
      src++;
      src_size--;
   }
   *(dst++) = 0; // NULL terminator
   return num_codept;
}
//======================================================================================
// Object Methods
//--------------------------------------------------------------------------------------
// Resets the object to default *back to a fresh start*
static void _UTF8Buffer_Reset(tUTF8Buffer *obj) {
   obj->cursor.anchor = 0;
   obj->cursor.length = 0;
}
//--------------------------------------------------------------------------------------
static int _UTF8Buffer_FindFirst(tUTF8Buffer *obj, const char *st) {
   // finds the *first* instance of a null terminated string within buffer
   // returns
   //     1 : success
   //    -1 : fail
   int i, j;
   int len = strlen(st);
   int count = obj->buff_size - len;
   unsigned char *buff = (unsigned char *) obj->buff;
   unsigned char *s1;
   unsigned char *s2;
   
   for (i = 0; i < count; i++) {
      if (*buff == *st) {
         s1 = buff+1;
         s2 = (unsigned char *) st+1;
         for (j = 1; j < len; j++) if (*(s1++) != *(s2++)) break;
         if (j == len) {
            obj->cursor.anchor = i;
            obj->cursor.length = len;
            return 1;
         }
      }
      buff++;
   }
   return -1;
}
//--------------------------------------------------------------------------------------
static int _UTF8Buffer_FindNext(tUTF8Buffer *obj, const char *st) {
   // finds the *next* instance of a null terminated string within buffer
   // returns
   //     1 : success
   //    -1 : fail
   int i, j;
   int len = strlen(st);
   unsigned char *s1;
   unsigned char *s2;
   int start_from = obj->cursor.anchor+1;
   int count = obj->buff_size - len;
   unsigned char *buff = (unsigned char *) obj->buff + start_from;

   for (i = start_from; i < count; i++) {
      if (*buff == *st) {
         s1 = buff+1;
         s2 = (unsigned char *) st+1;
         for (j = 1; j < len; j++) if (*(s1++) != *(s2++)) break;
         if (j == len) {
            obj->cursor.anchor = i;
            obj->cursor.length = len;
            return 1;
         }
      }
      buff++;
   }
   return -1;
}
//--------------------------------------------------------------------------------------
static int _UTF8Buffer_SelectUntil(tUTF8Buffer *obj, const char *st) {
   // returns
   //     1 : success
   //    -1 : fail
   int i, j;
   int len = strlen(st);
   int count = obj->buff_size - len - 1;
   unsigned char *s1;
   unsigned char *s2;
   int start_from = obj->cursor.anchor;
   unsigned char *buff = (unsigned char *) obj->buff + start_from;
   
   for (i = start_from; i < count; i++) {
      if (*buff == *st) {
         s1 = buff+1;
         s2 = (unsigned char *) st+1;
         for (j = 1; j < len; j++) if (*(s1++) != *(s2++)) break;
         if (j == len) {
            obj->cursor.anchor = start_from;
            obj->cursor.length = i - start_from;
            return 1;
         }
      }
      buff++;
   }
   return -1;
}
//--------------------------------------------------------------------------------------
static void _UTF8Buffer_Copy(tUTF8Buffer *obj, char *dst, int dst_size) {
   // copies whatever is selected onto destination buffer

   char *src = obj->buff + obj->cursor.anchor;
   int src_size = obj->cursor.length;
   Sanatize((unsigned char *) dst, dst_size, (unsigned char *) src, src_size, obj->spec);
   
}
//--------------------------------------------------------------------------------------
static void _UTF8Buffer_ClearLt(tUTF8Buffer *obj) {
   // sets cursor to the left of selection and clears it
   obj->cursor.length = 0;
}
//--------------------------------------------------------------------------------------
static void _UTF8Buffer_ClearRt(tUTF8Buffer *obj) {
   // sets cursor to the right of selection and clears it
   obj->cursor.anchor += obj->cursor.length;
   obj->cursor.length = 0;
}
//--------------------------------------------------------------------------------------
static void SetMethodPointers(tUTF8Buffer *obj) {
   obj->Reset = _UTF8Buffer_Reset;
   obj->FindFirst = _UTF8Buffer_FindFirst;
   obj->FindNext = _UTF8Buffer_FindNext;
   obj->SelectUntil = _UTF8Buffer_SelectUntil;
   obj->Copy = _UTF8Buffer_Copy;
   obj->ClearLt = _UTF8Buffer_ClearLt;
   obj->ClearRt = _UTF8Buffer_ClearRt;
}
//======================================================================================
// Public Functions
//--------------------------------------------------------------------------------------
unsigned int UTF8Buffer_Decode(const char *st, int size) {
   // returns codepoint
   // does not check if codepoint is in a valid range it just decodes
   // should the sequence be malformed it returns UTF8_DECODE_FAIL

   int byte_count = 0; 
   int byte_size = 0;
   unsigned int codept = 0; 
   
   byte_size = byte_count = __byte_count[(unsigned char) *st];
   if (byte_count == 0) return UTF8_DECODE_FAIL; // error: orphaned continuation byte 
   
   codept = (*(st++) & __mask[byte_size]);
   byte_count--;
   if (byte_count == 0) return codept;
   size--;
   while (size > 0) {
      if ((0xC0 & *st) != 0x80) return UTF8_DECODE_FAIL; // error: expected a continuation byte
      codept = ((codept << 6) | (0x3F & *st)) ;
      byte_count--;
      if (byte_count == 0) {
         if (codept < __lower_limit[byte_size]) return UTF8_DECODE_FAIL; // overlong check just curtisy
         return codept;
      }
      st++;
      size--;
   };
   
   return UTF8_DECODE_FAIL; // if it gets here it means data is truncated
}
//--------------------------------------------------------------------------------------
int UTF8Buffer_IsValid(char *st, int spec) {
   // Checks string if its malformed based on spec
   // returns recommended new size of buffer to store a sanatized version of it
   unsigned char *src = (unsigned char *) st;
   int new_size = 0;
   int byte_count = 0;
   int byte_size = 0;
   int verdict = 0;
   unsigned int codept = 0;
   
   if (st == NULL) return -1;
   spec &= 0x7f;

   while (*src != 0) {
      if (byte_count == 0) {
         byte_size = byte_count = __byte_count[*src];
         if (byte_count == 0) { // error found, orphaned continuation byte, drop it
            return -1;
         } else {
            codept = (*src & __mask[byte_size]);
            byte_count--;
            if (byte_count == 0) {
               verdict = 1;
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                     break;
               }

               if (verdict > 0) {
                  new_size++;
               } else {
                  return -1;
               }
            }
         }
      } else {
         if ((0xC0 & *src) == 0x80) {
            codept = ((codept << 6) | (0x3F & *src)) ;
            byte_count--;
            verdict = 1;
            if (byte_count == 0) {
               switch (spec) {
                  case UTF8_SPEC_XML:
                     if (ValidXMLCodePoint(codept) < 0) verdict = -1;
                  case UTF8_SPEC_STANDARD:
                     if (codept > 0x10FFFF) verdict = -1;
                     if ((codept >= 0xD800) && (codept <= 0xDFFF)) verdict = -1;
                  case UTF8_JUST_DECODE:
                     if (codept < __lower_limit[byte_size]) verdict = -1;
                     break;
               }

               if (verdict > 0) {
                  new_size += byte_size;
               } else {
                  return -1;
               }
            }
         } else {  // error found: expected more continuation bytes
            return -1;
         }
      }
      src++;
   }

   return 1;  
}
//--------------------------------------------------------------------------------------
tUTF8Buffer *UTF8Buffer_LoadFile(const char *filename, int spec) {
   // Loads the whole file onto memory creates a UTF8 Buffer structure
   int            file_size;
   int            buff_size;
   FILE          *f;
   tUTF8Buffer   *obj;
   unsigned char *tmp;
   
   f = fopen(filename, "rb");
   if (f == NULL) return NULL;

   fseek(f, 0, SEEK_END);
   file_size = ftell(f);

   tmp = (unsigned char *) malloc(file_size);
   if (tmp == NULL) {
      fclose(f);
      return NULL;
   }

   fseek(f, 0, SEEK_SET);
   fread(tmp, 1, file_size, f);
   fclose(f);   

   buff_size = Examine(tmp, file_size, spec);

   obj = (tUTF8Buffer *) calloc(sizeof(tUTF8Buffer) + buff_size, 1);
   if (obj == NULL) {
      free(tmp);
      fclose(f);
      return NULL;
   }

   SetMethodPointers(obj);

   obj->buff_size = buff_size;
   obj->spec = spec;

   obj->count = Sanatize((unsigned char *)obj->buff, buff_size, tmp, file_size, spec);
   _UTF8Buffer_Reset(obj);
   free(tmp);

  return obj;
}
//--------------------------------------------------------------------------------------
tUTF8Buffer *UTF8Buffer_FromStr(char *st, int spec) {
   // creates a UTF8 Buffer structure from null terminated string
   int            st_size;
   int            buff_size;
   tUTF8Buffer   *obj;
   
   if (st == NULL) return NULL;

   st_size = strlen(st);
   buff_size = Examine((unsigned char *) st, st_size, spec);

   obj = (tUTF8Buffer *) calloc(sizeof(tUTF8Buffer) + buff_size, 1);
   if (obj == NULL) return NULL;

   SetMethodPointers(obj);

   obj->buff_size = buff_size;
   obj->spec = spec;

   obj->count = Sanatize((unsigned char *)obj->buff, buff_size, (unsigned char *) st, st_size, spec);
   _UTF8Buffer_Reset(obj);

  return obj;
}
//--------------------------------------------------------------------------------------
void UTF8Buffer_Destroy(tUTF8Buffer **obj) {
   // makes sure object is properly destroyed
   if (*obj == NULL) return;
   free(*obj);
   *obj = NULL;
}
//--------------------------------------------------------------------------------------
