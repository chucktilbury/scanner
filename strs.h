#ifndef _STRS_H
#define _STRS_H

typedef void* STR;

STR create_str(const char* ptr);
STR create_str_fmt(const char* fmt, ...);
STR append_str_fmt(STR ptr, const char* fmt, ...);
STR append_str_char(STR ptr, int ch);
STR append_str_buf(STR ptr, const char* buf);
STR append_str_str(STR ptr, STR str);
const char* raw_str(STR ptr);
int index_str(STR str, int idx);
void clear_str(STR ptr);

#endif /* _STRS_H */
