#ifndef _FILEIO_H
#define _FILEIO_H

#include "strs.h"

typedef void* FPTR;
typedef void* FSTK;

FSTK create_file_stk(const char* fname);
int consume_char(FSTK stk);
int get_char(FSTK stk);

FPTR open_input_file(FSTK stk, const char* fname);
int _consume_char(FPTR fp);
int _get_char(FPTR fp);

FPTR open_output_file(const char* fname);
void emit_buf(FPTR fp, const char* str);
void emit_fmt(FPTR fp, const char* fmt, ...);
void emit_str(FPTR fp, STR str);

void close_file(FPTR fp);

const char* get_fname(FSTK ptr);
int get_line_no(FSTK ptr);
int get_col_no(FSTK ptr);

#endif /* _FILEIO_H */
