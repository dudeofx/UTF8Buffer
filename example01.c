// This example loads file 'example.txt' as UTF-8 text buffer
// and counts all the substrings of "apple" it finds

#include <stdio.h>
#include "UTF8Buffer.h"

//------------------------------------------------------------------------------------
int Fail(char *errstr) {
   printf("ERROR: %s\n", errstr);
   return -1;
}
//------------------------------------------------------------------------------------
int main() {
   int count = 0;
   tUTF8Buffer *txt = UTF8Buffer_LoadFile("example.txt", UTF8_SPEC_DEFAULT);
   if (txt == NULL) return Fail("unable to load file");

   int result = txt->FindFirst(txt, "apple");
   while (result > 0) {
     count++;
     result = txt->FindNext(txt, "apple");
   }

   UTF8Buffer_Destroy(&txt);

   printf("count = %d\n", count); 
}
//------------------------------------------------------------------------------------
