/*
   String buffer.

   * Owns the memory.
   * Custom allocation supported. (Bring your own memory).
   * Auto grow.
   * All modify operations zero terminate the buffer. (Safe to pass to standard functions).
   * stb-like: define STRBUF_IMPLEMENTATION for implementation.

   */

#ifndef STRBUF_H
#define STRBUF_H

#include "strview.h"
#include "arena.h"
#include "assert.h"

#define STRBUF_CAPACITY_GROW_STEP 16
#define STRBUF__ZERO_TERM (1)
#define STRBUF__ALLOC_PROTOTYPE(x) void* (x) (void* ptr, size_t size, int align, void* user_data)


/// This is used for counting the number of arguments to the strbuf_cat() macro below.
/// From https://stackoverflow.com/a/35693080
#define STRBUF__PP_100TH_ARG( \
     _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
    _81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,N,...) N
#define STRBUF__PP_RSEQ_N() \
    99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80, \
    79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60, \
    59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#define STRBUF__PP_NARG_(...) STRBUF__PP_100TH_ARG(__VA_ARGS__)
#define STRBUF__PP_NARG(...)  STRBUF__PP_NARG_(__VA_ARGS__,STRBUF__PP_RSEQ_N())


typedef struct {
    int capacity;
    int size;
    union {
        char *data;
        char *cstr;
    };
    bool is_fixed;    // Only ever used in 'strbuf__resize' and 'strbuf_destroy'.
    void *allocator_user_data;
    STRBUF__ALLOC_PROTOTYPE(*allocator);
} strbuf_t;
static_assert(sizeof(strbuf_t) == 40, "");


// ↓↓↓ Public.
void strbuf_empty(strbuf_t* buf);
strbuf_t strbuf_create_with_allocator(STRBUF__ALLOC_PROTOTYPE(*allocator), void *allocator_user_data);
strbuf_t strbuf_create_fixed(char *data, int capacity);
int strbuf_grow(strbuf_t *buf, const int new_capacity);
int strbuf_shrink(strbuf_t *buf);
int strbuf_assign(strbuf_t* buf, strview_t str);
int strbuf_append(strbuf_t* buf, strview_t str);
int strbuf_prepend(strbuf_t* buf, strview_t str);
int strbuf_insert_at_index(strbuf_t *buf, const int index, strview_t str);
int strbuf_insert_before(strbuf_t *buf, strview_t pos, strview_t view);
int strbuf_insert_after(strbuf_t *buf, strview_t pos, strview_t view);
int strbuf_append_char(strbuf_t *buf, char c);
__attribute__((format(printf, 2, 0))) int strbuf_append_vprintf(strbuf_t *buf, const char *fmt, va_list p_list);
__attribute__((format(printf, 2, 3))) int strbuf_append_printf(strbuf_t *buf, const char *fmt, ...);
__attribute__((format(printf, 2, 3))) int strbuf_printf(strbuf_t *buf, const char *fmt, ...);
void strbuf_destroy(strbuf_t* buf);
#define strbuf_cat(scratch, buf, ...) strbuf__cat_variadic(scratch, buf, STRBUF__PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define strbuf_view(buf) _Generic((buf), \
    strbuf_t*: strbuf__view,             \
    strbuf_t : strbuf__view2 )(buf)

// ↓↓↓ Private.
strview_t strbuf__view(strbuf_t* buf);
strview_t strbuf__view2(const strbuf_t buf);
int strbuf__resize(strbuf_t *buf, const int new_capacity);
int strbuf__grow_to_fit(strbuf_t *buf, const int minimum_size);
int strbuf__cat_variadic(Arena *scratch, strbuf_t* buf, int view_count, ...);
int strbuf__cat(Arena *scratch, strbuf_t *buf, int view_count, strview_t *views);
STRBUF__ALLOC_PROTOTYPE(strbuf__default_allocator);


#endif
#if defined STRBUF_IMPLEMENTATION || defined __CLANGD__

#include "stdalign.h"

void strbuf_empty(strbuf_t* buf) {
    buf->size = 0;
    if (buf->capacity > 0) { buf->cstr[0] = 0; }
}

strview_t strbuf__view(strbuf_t* buf) {
    if (!buf) { return STRVIEW_INVALID; }
    return (strview_t) { .data=buf->cstr, .size=buf->size, };
}

strview_t strbuf__view2(const strbuf_t buf) {
    return (strview_t) { .data=buf.cstr, .size=buf.size, };
}

strbuf_t strbuf_create_with_allocator(STRBUF__ALLOC_PROTOTYPE(*allocator), void *allocator_user_data) {
    return (strbuf_t) { .allocator = allocator, .allocator_user_data = allocator_user_data, };
}

strbuf_t strbuf_create_fixed(char *data, int capacity) {
    return (strbuf_t) { .capacity = capacity, .cstr = data, .is_fixed = true, };
}

static inline int strbuf__int_min(int a, int b) { return a < b ? a : b; }
static inline int strbuf__int_max(int a, int b) { return a > b ? a : b; }

static bool strbuf__add_will_overflow_int(int a, int b) {
    int c = a;
    c += b;
    return ((a < 0) == (b < 0) && (a < 0) != (c < 0));
}


static int strbuf__round_up_capacity(int capacity) {
    int remainder = capacity % STRBUF_CAPACITY_GROW_STEP;
    if (remainder) {
        if (!strbuf__add_will_overflow_int(capacity, STRBUF_CAPACITY_GROW_STEP - remainder)) {
            capacity += STRBUF_CAPACITY_GROW_STEP - remainder;
        } else {
            capacity = INT_MAX;
        }
    };
    return capacity;
}


/// @Note: Can grow or shrink.
/// @Note: 'allocator' Here acts as malloc when pointer is NULL, and realloc otherwise. So no need to memmove or memcpy.
int strbuf__resize(strbuf_t *buf, const int new_capacity) {
    if (new_capacity < 0) { return -1; }
    if (buf->is_fixed) { return -1; }
    if (buf->capacity == new_capacity) { return 0; }
    STRBUF__ALLOC_PROTOTYPE(*allocator) = buf->allocator ? buf->allocator : strbuf__default_allocator;
    char *new_data = (char*)allocator(buf->cstr, (size_t)new_capacity, alignof(char), buf->allocator_user_data);
    if (!new_data) { return -1; }
    buf->capacity = new_capacity;
    buf->cstr = new_data;
    buf->size = strbuf__int_min(buf->size, new_capacity - STRBUF__ZERO_TERM);
    buf->cstr[buf->size] = 0;
    return 0;
}


/// @Note. Is different to 'strbuf_grow' that sets the capacity exactly.
int strbuf__grow_to_fit(strbuf_t *buf, const int minimum_size) {
    return strbuf__resize(buf, strbuf__round_up_capacity(minimum_size));
}

/// @Note. Is different to 'strbuf__grow_to_fit' that rounds up.
int strbuf_grow(strbuf_t *buf, const int new_capacity) {
    if (buf->capacity >= new_capacity) { return 0; }
    return strbuf__resize(buf, new_capacity);
}

int strbuf_shrink(strbuf_t *buf) {
    if (!buf) { return -1; }
    return strbuf__resize(buf, buf->size + STRBUF__ZERO_TERM);
}

int strbuf_assign(strbuf_t* buf, strview_t str) {
    if (!buf) { return -1; }
    int required_size = str.size + STRBUF__ZERO_TERM;
    if (buf->capacity < required_size) {
        int err = strbuf__grow_to_fit(buf, required_size);
        if (err) { strbuf_empty(buf); return -1; }
    }
    memmove(buf->cstr, str.data, (size_t)str.size);
    buf->size = str.size;
    buf->cstr[buf->size] = 0;
    return 0;
}

int strbuf_append(strbuf_t* buf, strview_t str) {
    if (!buf) { return -1; }
    if (str.size <= 0) { return 0; }
    int required_size = buf->size + str.size + STRBUF__ZERO_TERM;
    if (buf->capacity < required_size) {
        strview_t old_buf = strbuf_view(buf);
        int err = strbuf__grow_to_fit(buf, required_size);
        if (err) { strbuf_empty(buf); return -1; }
        if (strview_is_subview_of_view(old_buf, str)) { str.data = buf->cstr + (str.data - old_buf.data); }
    }
    memmove(buf->cstr + buf->size, str.data, (size_t)str.size);
    buf->size = buf->size + str.size;
    buf->cstr[buf->size] = 0;
    return 0;
}

int strbuf_prepend(strbuf_t* buf, strview_t str) {
    if (!buf) { return -1; }
    if (str.size <= 0) { return 0; }
    int required_size = buf->size + str.size + STRBUF__ZERO_TERM;
    if (buf->capacity < required_size) {
        strview_t old_buf = strbuf_view(buf);
        int err = strbuf__grow_to_fit(buf, required_size);
        if (err) { strbuf_empty(buf); return -1; }
        if (strview_is_subview_of_view(old_buf, str)) { str.data = buf->cstr + (str.data - old_buf.data); }
    }
    memmove(buf->cstr + str.size, buf->cstr, (size_t)buf->size);
    memmove(buf->cstr, str.data, (size_t)str.size);
    buf->size = buf->size + str.size;
    buf->cstr[buf->size] = 0;
    return 0;
}

int strbuf_insert_at_index(strbuf_t *buf, const int index, strview_t str) {
    if (!buf) { return -1; }
    if (str.size <= 0) { return 0; }
    if (index < 0 || index > buf->size) { return -1; }
    int new_size = buf->size + str.size;
    int required_cap = new_size + STRBUF__ZERO_TERM;
    strview_t old_buf = strbuf_view(buf);
    bool is_inside = strview_is_subview_of_view(old_buf, str);
    if (buf->capacity < required_cap) {
        int err = strbuf__grow_to_fit(buf, required_cap);
        if (err) { strbuf_empty(buf); return -1; }
        if (is_inside) { str.data = buf->cstr + (str.data - old_buf.data); }
    }
    memmove(buf->cstr + index + str.size, buf->cstr + index, (size_t)(buf->size - index));
    if (is_inside) {
        int cut = index;
        int pos = (int)(str.data - buf->cstr);
        if (pos < cut) {
            strview_t left_half = { .data = str.data, .size = strbuf__int_min(str.size, cut-pos), };
            strview_t right_half = { .data = buf->cstr + index + str.size, .size = strbuf__int_max(0, str.size - left_half.size) };
            //printf("\nhalfs["PRIstr"]["PRIstr"]\n", PRIstrarg(left_half), PRIstrarg(right_half));
            memmove(buf->cstr + index, left_half.data, (size_t)left_half.size);
            memmove(buf->cstr + index + left_half.size, right_half.data, (size_t)right_half.size);
        } else {
            str.data += str.size;
            memmove(buf->cstr + index, str.data, (size_t)str.size);
        }
    } else {
        memmove(buf->cstr + index, str.data, (size_t)str.size);
    }
    buf->size = new_size;
    buf->cstr[buf->size] = 0;
    return 0;
}

int strbuf_insert_before(strbuf_t *buf, strview_t pos, strview_t view) {
    if (!strview_is_subview_of_view(strbuf_view(buf), pos)) { return -1; }
    return strbuf_insert_at_index(buf, (int)(pos.data - buf->cstr), view);
}

int strbuf_insert_after(strbuf_t *buf, strview_t pos, strview_t view) {
    if (!strview_is_subview_of_view(strbuf_view(buf), pos)) { return -1; }
    return strbuf_insert_at_index(buf, (int)(pos.data - buf->cstr) + pos.size, view);
}

int strbuf_append_char(strbuf_t *buf, char c) {
    return strbuf_append(buf, (strview_t){.data=&c, .size=1});
}

int strbuf__cat_variadic(Arena *scratch, strbuf_t* buf, int view_count, ...) {
    va_list ap;
    va_start(ap, view_count);
    strview_t views[view_count];
    for (int i = 0; i < view_count; i++) {
        views[i] = va_arg(ap, strview_t);
    }
    va_end(ap);
    return strbuf__cat(scratch, buf, view_count, views);
}

/// @Note: If combined size of all views overflows INT_MAX, will return error.
/// @Note: Views may be part of buf only if not fixed.
int strbuf__cat(Arena *scratch, strbuf_t *buf, int view_count, strview_t *views) {
    bool contains_itself = false;
    int required_size = STRBUF__ZERO_TERM;
    const strview_t og_view = strbuf_view(buf);
    {
        for (int i = 0; i < view_count; ++i) {
            //printf("%d:["PRIstr"]\n", i, PRIstrarg(views[i]));
            if (strbuf__add_will_overflow_int(required_size, views[i].size)) { return -1; }
            if (strview_is_subview_of_view(og_view, views[i])) { contains_itself = true; }
            required_size += views[i].size;
        }
    }
    if (buf->capacity < required_size) {
        int err = strbuf__grow_to_fit(buf, required_size);
        if (err) { strbuf_empty(buf); return -1; }
    }
    if (contains_itself) {
        strbuf_t temp = strbuf_create_with_allocator(arena_allocator, scratch);
        int err = strbuf_assign(&temp, strbuf_view(buf));
        //printf("->["PRIstr"]\n", PRIstrarg(strbuf_view(buf)));
        //printf("->["PRIstr"]\n", PRIstrarg(strbuf_view(&temp)));
        if (err) { return -1; }
        for (int i = 0; i < view_count; ++i) {
            if (strview_is_subview_of_view(og_view, views[i])) {
                views[i].data = temp.cstr + (views[i].data - og_view.data);
            }
        }
    }
    buf->size = 0;
    for (int i = 0; i < view_count; ++i) {
        if (views[i].size <= 0) { continue; }
        strbuf_append(buf, views[i]);
    }
    return 0;
}


__attribute__((format(printf, 2, 0)))
int strbuf_append_vprintf(strbuf_t *buf, const char *fmt, va_list p_list) {
    int err;
    va_list list;
    va_copy(list, p_list);
    const int printf_length = vsnprintf(NULL, 0, fmt, list);
    va_end(list);
    if (printf_length < 0) { return -1; }
    if (printf_length == 0) { return 0; }
    const int required_cap = buf->size + printf_length + STRBUF__ZERO_TERM;
    if (buf->capacity < required_cap) {
        err = strbuf__grow_to_fit(buf, required_cap);
        if (err) { strbuf_empty(buf); return -1; }
    }
    va_copy(list, p_list);
    // See man vsnprintf for example.
    err = vsnprintf(buf->cstr + buf->size, (size_t)printf_length + STRBUF__ZERO_TERM, fmt, list);
    va_end(list);
    buf->size = buf->size + printf_length;
    buf->cstr[buf->size] = 0;
    if (err < 0) { return -1; }
    return 0;
}

__attribute__((format(printf, 2, 3)))
int strbuf_append_printf(strbuf_t *buf, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    int err = strbuf_append_vprintf(buf, fmt, list);
    va_end(list);
    return err;
}

/// @Note: If appending always preffer 'strbuf_append_printf' instead, because
///        According to 'man snprintf' dest should not be same as source.
__attribute__((format(printf, 2, 3)))
int strbuf_printf(strbuf_t *buf, const char *fmt, ...) {
    strbuf_empty(buf);
    va_list list;
    va_start(list, fmt);
    int err = strbuf_append_vprintf(buf, fmt, list);
    va_end(list);
    return err;
}

/// @Note: here 'allocator' acts as free.
void strbuf_destroy(strbuf_t* buf) {
    if (!buf) { return; }
    if (buf->is_fixed) { return; }
    if (buf->cstr) {
        STRBUF__ALLOC_PROTOTYPE(*allocator) = buf->allocator ? buf->allocator : strbuf__default_allocator;
        allocator(buf->cstr, 0, 0, buf->allocator_user_data);
    }
    *buf = (strbuf_t) { 0 };
    //buf->capacity = 0;
    //buf->size = 0;
    //buf->cstr = NULL;
}

STRBUF__ALLOC_PROTOTYPE(strbuf__default_allocator) {
    // New allocation: ptr == NULL && size > 0
    // Reallocation:   ptr != NULL && size > 0
    // Free:           ptr != NULL && size == 0
    (void)align; (void)user_data; // Unused: malloc guarantees alignment.
    void* result = NULL;
    if      (size == 0)   { free(ptr);                   }
    else if (ptr == NULL) { result = malloc(size);       }
    else                  { result = realloc(ptr, size); }
    return result;
}

#endif

