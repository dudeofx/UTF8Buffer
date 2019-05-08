# Design notes:
- Does not do case sensitive searches

---

# Security notes:
- The only time UTF8Buffer checks for valid UTF-8 is when creating the object. When doing the string searches or text manipulation the input is not checked for validity. If the input is coming from untrusted sources it should be examined first. 




