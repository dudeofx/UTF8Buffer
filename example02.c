// this example shows a quick and dirty way of extracting
// data from a string that resembles XML

#include <stdio.h>
#include "UTF8Buffer.h"


int main() {
   char phone_number[128];
   char st[] = "<contact><name>John Doe</name><phone>123-456-7890</phone><sex>male</sex></contact>";
   tUTF8Buffer *txt = UTF8Buffer_FromStr(st,  UTF8_SPEC_DEFAULT);

   txt->FindFirst(txt, "<phone>");
   txt->ClearRt(txt);
   txt->SelectUntil(txt, "</phone>");
   txt->Copy(txt, phone_number, 128);

   UTF8Buffer_Destroy(&txt);

   printf("Phone Number: %s\n", phone_number);
}
