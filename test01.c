// The file "utf8test.txt" is a stress tester for UTF-8 decoders
// this program takes it as input and outputs "test_out.txt"
// so that results can be examined

#include <stdio.h>
#include "UTF8Buffer.h"

int main(int argc, char *argv[]) {
   FILE *f;

   tUTF8Buffer *utf8;
   utf8 = UTF8Buffer_LoadFile("utf8test.txt", UTF8_SPEC_STANDARD);
   f = fopen("test_out.txt", "wb");
   fwrite(utf8->buff, utf8->buff_size, 1, f);
   fclose(f);
   printf("count = %d\n", utf8->count);
}

