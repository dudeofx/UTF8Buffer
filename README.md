# UTF8Buffer
Simple library to load, check and manipulate a memory buffer with UTF-8 text

# Designed Purpose
The primary purpose for this library is to load and sanatize UTF-8 text from outside sources. Simple text editing is supported thru cut, paste & insert functions but its is not pro-efficient at it. The data is either moved around or the buffer is recreated after every modification. When dealing with very long text, modifications can be costly. Editing is only recommended for short text uses. About 64KB or less.

