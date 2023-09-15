#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "mem.h"
#include "strs.h"

struct _string_ {
    char* buffer;
    int len;
    int cap;
};

#define CONVERT(p)      ((struct _string_*)(p))
#define MKLOCAL(n, p)   struct _string_*(n)=CONVERT(p)
#define RETURN_STR(p)   return (void*)(p)

STR create_str(const char* str) {

    struct _string_* ptr = _alloc_obj(struct _string_);

    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->buffer = _alloc_array(char, ptr->len);

    if(str != NULL)
        append_str_buf(ptr, str);

    RETURN_STR(ptr);
}

STR create_str_fmt(const char* fmt, ...) {

    assert(fmt != NULL);

    va_list args;
    size_t len;

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* str = _alloc(len + sizeof(int));

    va_start(args, fmt);
    vsnprintf(str, len, fmt, args);
    va_end(args);

    STR ptr = create_str(str);
    _free(str);

    return ptr;
}

STR append_str_fmt(STR s, const char* fmt, ...) {

    assert(s != NULL);
    assert(fmt != NULL);

    va_list args;
    size_t len;
    MKLOCAL(p, s);

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* str = _alloc(len + sizeof(int));

    va_start(args, fmt);
    vsnprintf(str, len, fmt, args);
    va_end(args);

    STR ptr = append_str_buf(p, str);
    _free(str);

    return ptr;
}

STR append_str_char(STR ptr, int ch) {

    assert(ptr != NULL);

    MKLOCAL(p, ptr);
    if(p->len+1 > p->cap) {
        p->cap <<= 1;
        p->buffer = _realloc_array(p->buffer, char, p->cap);
    }

    p->buffer[p->len] = ch;
    p->len++;
    p->buffer[p->len] = '\0';

    return ptr;
}

STR append_str_buf(STR ptr, const char* buf) {

    assert(ptr != NULL);
    assert(buf != NULL);

    MKLOCAL(p, ptr);
    int len = strlen(buf);

    if(p->len+len+1 > p->cap) {
        while(p->len+len+1 > p->cap)
            p->cap <<= 1;
        p->buffer = _realloc_array(p->buffer, char, p->cap);
    }

    // copy the ending null, as well.
    memcpy(&p->buffer[p->len], buf, len+1);
    p->len += len;

    return ptr;
}

STR append_str_str(STR ptr, STR str) {

    assert(ptr != NULL);

    return append_str_buf(ptr, CONVERT(str)->buffer);
}

const char* raw_str(STR ptr) {

    assert(ptr != NULL);

    return CONVERT(ptr)->buffer;
}

int index_str(STR str, int idx) {

    assert(str != NULL);

    MKLOCAL(p, str);
    if(idx < p->len)
        return p->buffer[idx];
    else
        return 0;
}

void clear_str(STR ptr) {

    assert(ptr != NULL);

    MKLOCAL(p, ptr);
    p->len = 0;
    p->buffer[0] = 0;
}
