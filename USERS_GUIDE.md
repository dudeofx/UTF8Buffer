# Dependencies

Just include the following files on your project:

~~~
UTF8Buffer.h
UTF8Buffer.c
~~~

---

# Initialization and Destruction

This library defines the object type `tUTF8Buffer`

You can create the object by providing a null terminated string like this:  
`tUTF8Buffer *obj = UTF8Buffer_FromStr("C style string", flags);`  

or you can load a text file into memory like this:  
`tUTF8Buffer *obj = UTF8Buffer_LoadFile("path/filename", flags);`


The flags can be one of the following:

- `UTF8_SPEC_STANDARD` Follows the UTF8 specifications for validity
- `UTF8_JUST_DECODE` It simply tries to decode it doesn't worry about valid code points
- `UTF8_SPEC_XML` Follows the specs for valid XML code points.
- `UTF8_REMOVE_INVALID` This flag is meant to be OR'ed with the other flags in case you want to remove invalid code points from the buffer. UTF8 Specifies to insert the invalid character for invalid code points. They have their reasons but sometimes you just want to remove them.
- `UTF8_SPEC_DEFAULT` Follow the developer recommended defaults. It is the equivalent to `(UTF8_SPEC_STANDARD | UTF8_REMOVE_INVALID)`

Once you are done with the object you can destroy it like this:  
`UTF8Buffer_Destroy(&obj);`

If you just want to examine a string if its valid UTF-8:  
`UTF8Buffer_Examine(c_string_to_examine, flags);`  
It returns 1 if its valid or -1 if its not.

---

# Object Methods

The object contains the methods to do searches and extracting substrings. It maintains an internal cursor and the methods below are used to navigate the buffer.

---

`void Reset(tUTF8Buffer *self);`  

Sets the cursor to the beginning

---

`int FindFirst(tUTF8Buffer *self, const char *st);`  

Start from the beginning and find the sub-string st and select it  

  Return:  
-1 = Fail  
1 = Success

---

`int FindNext(tUTF8Buffer *self, const char *st);`  

From the current position find the sub-string st and select it  

  Return:  
-1 = Fail  
1 = Success

---

`int SelectUntil(tUTF8Buffer *self, const char *st);`  

Select everything between the current position until the occurrence of sub-string st. The sub-string is not included in the selection

  Return:  
-1 = Fail  
1 = Success

---

`void Copy(tUTF8Buffer *self, char *dst, int dst_size);`  

Copy the selected text unto dst. Apply a null termination onto dst. If selection is larger then dst_size then only part of the selection is copied. A null is still applied at the end of dst.

---

`void ClearLt(tUTF8Buffer *self);`  

Clear selection. Set cursor to the left of what was selected.

---

`void ClearRt(tUTF8Buffer *self);`  

Clear selection. Set cursor to the right of what was selected.

---

# Quick Examples

The following example counts the instances of the string "apple" within the text file example.txt

~~~
   int count = 0;
   tUTF8Buffer *txt = UTF8Buffer_LoadFile("example.txt", UTF8_SPEC_DEFAULT);

   int result = txt->FindFirst(txt, "apple");
   while (result > 0) {
     count++;
     result = txt->FindNext(txt, "apple");
   }

   UTF8Buffer_Destroy(&txt);

   printf("count = %d\n", count); 
~~~

---

The following example extracts the phone number from a string that resembles XML.

~~~
   char phone_number[128];
   char st[] = "<contact><name>John Doe</name><phone>123-456-7890</phone><sex>male</sex></contact>";
   tUTF8Buffer *txt = UTF8Buffer_FromStr(st,  UTF8_SPEC_DEFAULT);

   txt->FindFirst(txt, "<phone>");
   txt->ClearRt(txt);
   txt->SelectUntil(txt, "</phone>");
   txt->Copy(txt, phone_number, 128);

   UTF8Buffer_Destroy(&txt);

   printf("Phone Number: %s\n", phone_number);
~~~

---
