#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>

#include "greatest.h"
#include "strbuf.h"
#include "strview.h"
#include "strnum.h"

#define DBG(_fmtarg, ...) printf("%s:%.4i - "_fmtarg"\n" , __FILE__, __LINE__ ,##__VA_ARGS__)

GREATEST_MAIN_DEFS();

#define STATIC_BUFFER_SIZE	200
static char static_buf[STATIC_BUFFER_SIZE] __attribute__ ((aligned));

/*static void* allocator(struct strbuf_allocator_t* this_allocator, void* ptr_to_free, size_t size);*/

SUITE(suite_strbuf);
TEST test_strbuf_create_using_malloc(void);
TEST test_strbuf_create_using_allocator(void);
TEST test_strbuf_create_static(void);
TEST test_strbuf_create_init(void);
TEST test_strbuf_strcat(void);
TEST test_strbuf_shrink(void);
TEST test_strbuf_printf(void);
TEST test_strbuf_append_printf(void);
/*TEST test_strbuf_prnf(void);*/
/*TEST test_strbuf_append_prnf(void);*/
TEST test_strbuf_assign(void);
TEST test_strbuf_append(void);
TEST test_strbuf_prepend(void);
TEST test_strbuf_insert_at_index(void);
TEST test_strbuf_insert_before(void);
TEST test_strbuf_insert_after(void);
/*TEST test_strbuf_to_cstr(void); // <-- WONT FIX.*/
/*TEST test_strbuf_terminate_views(void);*/
TEST test_strbuf_empty(void);


ArenaRoot arenaroot;

int main(int argc, const char* argv[])
{
	arenaroot = ArenaRoot_create(1 << 20);
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(suite_strbuf);
	GREATEST_MAIN_END();
	ArenaRoot_free(&arenaroot);
	return 0;
}

//********************************************************************************************************
// Private functions
//********************************************************************************************************

static void* my_allocator (void* ptr, size_t size, int align, void* user_data) {
	// TODO: Use an arena instead.
	(void)align; (void) user_data;
	void* result;
	result = realloc(ptr, size);
	assert(size==0 || result);	// You need to catch a failed allocation here.
	return result;
}

SUITE(suite_strbuf)
{
	RUN_TEST(test_strbuf_create_using_malloc);
	RUN_TEST(test_strbuf_create_init);
	RUN_TEST(test_strbuf_create_using_allocator);
	RUN_TEST(test_strbuf_create_static);
	RUN_TEST(test_strbuf_strcat);
	RUN_TEST(test_strbuf_shrink);
	RUN_TEST(test_strbuf_printf);
	RUN_TEST(test_strbuf_append_printf);
	/*RUN_TEST(test_strbuf_prnf);*/
	/*RUN_TEST(test_strbuf_append_prnf);*/
	RUN_TEST(test_strbuf_assign);
	RUN_TEST(test_strbuf_append);
	RUN_TEST(test_strbuf_prepend);
	RUN_TEST(test_strbuf_insert_at_index);
	RUN_TEST(test_strbuf_insert_before);
	RUN_TEST(test_strbuf_insert_after);
	/*RUN_TEST(test_strbuf_to_cstr); /// <--- WONT FIX.*/
	/*RUN_TEST(test_strbuf_terminate_views);*/
	RUN_TEST(test_strbuf_empty);
}


TEST test_strbuf_create_using_malloc(void)
{
	#define INITIAL_BUF_CAPACITY 16
	strbuf_t buf = { 0 };
	strbuf_grow(&buf, INITIAL_BUF_CAPACITY);

	/*buf = strbuf_create(INITIAL_BUF_CAPACITY, NULL);*/

	/*ASSERT(buf);*/
	ASSERT(buf.cstr);
	ASSERT(buf.size == 0);
	ASSERT(buf.capacity == INITIAL_BUF_CAPACITY);
	ASSERT(buf.cstr[0] == 0);
	strbuf_destroy(&buf);
	ASSERT(!buf.cstr);

	PASS();
	#undef INITIAL_BUF_CAPACITY
}

TEST test_strbuf_create_init(void)
{
	#define TEST_STRING	"The quick brown fox jumped over the lazy dog"
	strbuf_t buf = { 0 };
	strbuf_assign(&buf, cstr(TEST_STRING));
	/*buf = strbuf_create(cstr(TEST_STRING), NULL);*/

	/*ASSERT(buf);*/
	ASSERT(buf.cstr);
	ASSERT(buf.size == sizeof(TEST_STRING)-1);
	ASSERT(buf.cstr[sizeof(TEST_STRING)-1] == 0);
	strbuf_destroy(&buf);
	ASSERT(!buf.cstr);

	PASS();
	#undef TEST_STRING
}

TEST test_strbuf_create_using_allocator(void)
{
	#define INITIAL_BUF_CAPACITY 16
	/*strbuf_allocator_t strbuf_allocator = {.allocator = allocator};*/
	/*strbuf_t* buf;*/
	/*buf = strbuf_create(INITIAL_BUF_CAPACITY, &strbuf_allocator);*/
	strbuf_t buf = strbuf_create_with_allocator(my_allocator, NULL);
	strbuf_grow(&buf, INITIAL_BUF_CAPACITY);

	/*ASSERT(buf);*/
	ASSERT(buf.cstr);
	ASSERT(buf.size == 0);
	ASSERT(buf.capacity == INITIAL_BUF_CAPACITY);
	ASSERT(buf.cstr[0] == 0);
	strbuf_destroy(&buf);
	ASSERT(!buf.cstr);

	PASS();
	#undef INITIAL_BUF_CAPACITY
}

TEST test_strbuf_create_static(void)
{
	/*strbuf_t* buf;*/

	/*ASSERT(!strbuf_create_fixed(static_buf+3, STATIC_BUFFER_SIZE));	//must fail due to badly aligned address*/
	/*ASSERT(!strbuf_create_fixed(static_buf, sizeof(strbuf_t)));		//must fail due to insufficient space*/

	strbuf_t buf = strbuf_create_fixed(static_buf, STATIC_BUFFER_SIZE);
	/*ASSERT(buf);*/
	ASSERT(buf.cstr);
	ASSERT(buf.size == 0);
	ASSERT(buf.capacity == STATIC_BUFFER_SIZE);
	ASSERT(buf.cstr[0] == 0);
	/*strbuf_destroy(&buf);*/
	/*ASSERT(!buf.cstr);*/

	PASS();
}

TEST test_strbuf_strcat(void)
{
	#define INITIAL_BUF_CAPACITY 16
	Arena arena = ArenaRoot_get_arena(arenaroot);
	strview_t str1;
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;
	/*buf = strbuf_create(INITIAL_BUF_CAPACITY, NULL);*/
	strbuf_grow(buf, INITIAL_BUF_CAPACITY);
	ASSERT(buf->cstr);

	//Concatenating "AAAAAAAAAA", "BBBBBBBBBB", "CCCCCCCCCC"
	strbuf_cat(&arena, buf, cstr("AAAAAAAAAA"), cstr("BBBBBBBBBB"), cstr("CCCCCCCCCC"));
	ASSERT(buf->capacity >= 30);
	ASSERT(buf->size == 30);
	ASSERT(!strcmp(buf->cstr, "AAAAAAAAAABBBBBBBBBBCCCCCCCCCC"));

	//Appending -AFTER to existing buffer
	strbuf_cat(&arena, buf, strbuf_view(buf), cstr("-AFTER"));
	ASSERT(!strcmp(buf->cstr, "AAAAAAAAAABBBBBBBBBBCCCCCCCCCC-AFTER"));

	//Prepending BEFORE- to existing buffer
	strbuf_cat(&arena, buf, cstr("BEFORE-"), strbuf_view(buf));
	/*printf("["PRIstr"]\n", PRIstrarg(strbuf_view(buf)));*/
	ASSERT(!strcmp(buf->cstr, "BEFORE-AAAAAAAAAABBBBBBBBBBCCCCCCCCCC-AFTER"));

	// Testing strbuf_cat() with a single invalid str
	// strbuf_cat() should return a valid string of length 0
	str1 = STRVIEW_INVALID;
	strbuf_cat(&arena, buf, str1);
	str1 = strbuf_view(buf);
	ASSERT(str1.size == 0);
	ASSERT(str1.data);

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);

	//Now perform some operations on a static buffer
	strbuf_t buf_static = strbuf_create_fixed(static_buf, STATIC_BUFFER_SIZE);
	buf = &buf_static;
	ASSERT(buf);
	ASSERT(buf->capacity == STATIC_BUFFER_SIZE);

	// Concatenating DDDDDDDDDD EEEEEEEEEE FFFFFFFFFF
	strbuf_cat(&arena, buf, cstr("DDDDDDDDDD"), cstr("EEEEEEEEEE"), cstr("FFFFFFFFFF"));
	ASSERT(buf->capacity == STATIC_BUFFER_SIZE);	//capacity should not change
	ASSERT(buf->size == 30);
	ASSERT(!strcmp(buf->cstr, "DDDDDDDDDDEEEEEEEEEEFFFFFFFFFF"));

	// **** This is no longer a limitation with arenas: ↓↓↓↓↓
	//      >> "Trying to pass data from the destination buffer into strbuf_cat() without a dynamic buffer (should fail and return empty buffer"
	// **** This is no longer a limitation with arenas: ↑↑↑↑↑
	str1 = strbuf_view(buf);
	str1 = strview_sub(str1, 5, 10);
	strbuf_cat(&arena, buf, cstr("never "), str1, cstr(" seen"));
	/*printf("["PRIstr"]\n", PRIstrarg(strbuf_view(buf)));*/
	/*ASSERT(buf->size == 0);*/

	// Trying append too much data to the buffer (should fail and return empty buffer
	strbuf_assign(buf, cstr(""));
	//this much should fit
	strbuf_append(buf, cstr("BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH  "));
	/*//this much should empty the buffer*/
	strbuf_append(buf, cstr("BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH BLAH  "));
	ASSERT(buf->size == 0);

	/*strbuf_destroy(buf);*/
	/*ASSERT(!buf);*/

	PASS();
}


TEST test_strbuf_shrink(void)
{
	/*strbuf_t* buf = strbuf_create(200, NULL);*/
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;
	int err;
	err = strbuf_grow(buf, 200);
	ASSERT(!err);

	err = strbuf_assign(buf, cstr_SL("hello-test"));
	/*printf("((%d))\n", buf->capacity);*/
	/*ASSERT(buf->capacity == 200);*/
	ASSERT(!err);
	err = strbuf_shrink(buf);
	ASSERT(!err);
	ASSERT(buf->size == 10);
	ASSERT(!memcmp("hello-test", buf->cstr, 10));
	/*printf("(%d)\n", buf->capacity);*/
	ASSERT(buf->capacity == 11);

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);

	strbuf_t buf_fixed = strbuf_create_fixed(static_buf, STATIC_BUFFER_SIZE);
	buf = &buf_fixed;
	ASSERT(buf->capacity == STATIC_BUFFER_SIZE);
	strbuf_assign(buf, cstr("hello-test"));
	strbuf_shrink(buf);
	ASSERT(buf->size == 10);
	ASSERT(!memcmp("hello-test", buf->cstr, 10));
	ASSERT(buf->capacity == STATIC_BUFFER_SIZE);	//strbuf_shrink should not change a static buffers capacity

	/*strbuf_destroy(buf);*/
	/*ASSERT(!buf->cstr);*/

	PASS();
}


TEST test_strbuf_printf(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	/*buf = strbuf_create(0, NULL);*/
	int err = strbuf_printf(buf, "Hello from printf! have some numbers... %i %i %i %i %i %i %i", 6246456, 3466765, 435234, 4598756, 94572, 69, 42597);
	ASSERT(!err);
	str1 = strbuf_view(buf);

	/*printf("->->["PRIstr"]\n", PRIstrarg(str1));*/
	ASSERT(!memcmp(str1.data, "Hello from printf! have some numbers... 6246456 3466765 435234 4598756 94572 69 42597", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_append_printf(void)
{
	strview_t str1;
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	/*buf = strbuf_create(0, NULL);*/
	strbuf_assign(buf, cstr("Hello"));
	int err = strbuf_append_printf(buf, " Appending one more number %i", 748921);
	ASSERT(!err);
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "Hello Appending one more number 748921", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}


TEST test_strbuf_assign(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;

	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;
	/*ASSERT(buf);*/

	// Testing strbuf_assign() source outside of the destination
	strbuf_assign(buf, cstr("***Hello test***"));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "***Hello test***", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	// Testing strbuf_assign() source from the destination
	strbuf_assign(buf, strview_trim(strbuf_view(buf), cstr("*")));
	strbuf_assign(buf, cstr("Hello test"));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "Hello test", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_append(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;

	/*buf = strbuf_create(0, NULL);*/
	/*ASSERT(buf);*/
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	// Testing strbuf_append(), with source from the destination
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_append(buf, strbuf_view(buf));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "{Hello-testing-some-string}{Hello-testing-some-string}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	// Testing strbuf_append()
	strbuf_assign(buf, cstr("{Hello-Fred}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_append(buf, cstr("{Hello-Bob}"));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "{Hello-Fred}{Hello-Bob}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_prepend(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;

	/*buf = strbuf_create(0, NULL);*/
	/*ASSERT(buf);*/
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	// Testing strbuf_prepend(), with source from the destination
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_prepend(buf, strbuf_view(buf));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "{Hello-testing-some-string}{Hello-testing-some-string}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	// Testing strbuf_prepend()
	strbuf_assign(buf, cstr("{Hello-Fred}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_prepend(buf, cstr("{Hello-Bob}"));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "{Hello-Bob}{Hello-Fred}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_insert_at_index(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;

	/*buf = strbuf_create(0, NULL);*/
	/*ASSERT(buf);*/
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	// Testing strbuf_insert_at_index(), with source from the destination
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_insert_at_index(buf, 6, strbuf_view(buf));
	str1 = strbuf_view(buf);
	/*printf("\n");*/
	/*printf("["PRIstr"]\n", PRIstrarg(str1));*/
	/*printf("\n");*/
	ASSERT(!memcmp(str1.data, "{Hello{Hello-testing-some-string}-testing-some-string}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	// Testing strbuf_insert_at_index(), with source from the destination before index.
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	strbuf_insert_at_index(buf, 7, strview_sub(strbuf_view(buf), 1, 1+5)); // -> Gets "Hello".
	str1 = strbuf_view(buf);
	/*printf("["PRIstr"]\n", PRIstrarg(str1));*/
	ASSERT(!memcmp(str1.data, "{Hello-Hellotesting-some-string}", (size_t)str1.size));

	// Testing strbuf_insert_at_index(), with source from the destination after index.
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	strbuf_insert_at_index(buf, 6, strview_sub(strbuf_view(buf), 19, 19+7)); // -> Gets "-string".
	str1 = strbuf_view(buf);
	/*printf("->["PRIstr"]\n", PRIstrarg(str1));*/
	ASSERT(!memcmp(str1.data, "{Hello-string-testing-some-string}", (size_t)str1.size));

	// Testing strbuf_insert_at_index(), with source from the destination
	strbuf_assign(buf, cstr("{Hello-testing-some-string}"));
	str1 = strbuf_view(buf);
	strbuf_shrink(buf);
	strbuf_insert_at_index(buf, 6, cstr_SL("(random text)"));
	str1 = strbuf_view(buf);
	ASSERT(!memcmp(str1.data, "{Hello(random text)-testing-some-string}", (size_t)str1.size));
	ASSERT(buf->cstr[buf->size] == 0);	//check the 0 terminator is in place

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}


TEST test_strbuf_insert_before(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	/*buf = strbuf_create(0, NULL);*/
	/*ASSERT(buf);*/

	strbuf_assign(buf, cstr("Hello"));
	str1 = strview_find_last(strbuf_view(buf), cstr(""));
	strbuf_insert_before(buf, str1, cstr("-test"));
	ASSERT(!memcmp(buf->cstr, "Hello-test", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_last(strbuf_view(buf), cstr("ll"));
	strbuf_insert_before(buf, str1, cstr("..."));
	ASSERT(!memcmp(buf->cstr, "Hello Me...llow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_first(strbuf_view(buf), cstr("ll"));
	strbuf_insert_before(buf, str1, cstr("---"));
	ASSERT(!memcmp(buf->cstr, "He---llo Mellow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_first(strbuf_view(buf), cstr(""));
	strbuf_insert_before(buf, str1, cstr("***"));
	ASSERT(!memcmp(buf->cstr, "***Hello Mellow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_last(strbuf_view(buf), "ll");
	strbuf_insert_before(buf, str1, cstr_SL("..."));
	ASSERT(!memcmp(buf->cstr, "Hello Me...llow", (size_t)buf->size));

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_insert_after(void)
{
	/*strbuf_t* buf;*/
	strview_t str1;

	/*buf = strbuf_create(0, NULL);*/
	/*ASSERT(buf);*/
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;

	strbuf_assign(buf, cstr("Hello"));
	str1 = strview_find_last(strbuf_view(buf), cstr(""));
	strbuf_insert_after(buf, str1, cstr("-test"));
	ASSERT(!memcmp(buf->cstr, "Hello-test", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_last(strbuf_view(buf), cstr("ll"));
	strbuf_insert_after(buf, str1, cstr("..."));
	/*printf("["PRIstr"]\n", PRIstrarg(strbuf_view(buf)));*/
	ASSERT(!memcmp(buf->cstr, "Hello Mell...ow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_last(strbuf_view(buf), cstr("ll"));
	strbuf_insert_after(buf, str1, cstr("<.........>"));
	/*printf("["PRIstr"]\n", PRIstrarg(strbuf_view(buf)));*/
	ASSERT(!memcmp(buf->cstr, "Hello Mell<.........>ow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_first(strbuf_view(buf), cstr("ll"));
	strbuf_insert_after(buf, str1, cstr("---"));
	ASSERT(!memcmp(buf->cstr, "Hell---o Mellow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_first(strbuf_view(buf), cstr(""));
	strbuf_insert_after(buf, str1, cstr("***"));
	ASSERT(!memcmp(buf->cstr, "***Hello Mellow", (size_t)buf->size));

	strbuf_assign(buf, cstr("Hello Mellow"));
	str1 = strview_find_last(strbuf_view(buf), "ll");
	strbuf_insert_after(buf, str1, cstr_SL("..."));
	ASSERT(!memcmp(buf->cstr, "Hello Mell...ow", (size_t)buf->size));

	strbuf_destroy(buf);
	ASSERT(!buf->cstr);
	PASS();
}

TEST test_strbuf_empty(void)
{
	strbuf_t buf_instance = { 0 };
	strbuf_t *buf = &buf_instance;
	strbuf_empty(buf);
	strbuf_assign(buf, cstr("Hello"));
	ASSERT(buf->cstr[0] != 0 && buf->size > 0);
	ASSERT(strview_equal(strbuf_view(buf), cstr("Hello")));
	strbuf_empty(buf);
	ASSERT(buf->cstr[0] == 0 && buf->size == 0);
	strbuf_destroy(buf);
	PASS();
}

