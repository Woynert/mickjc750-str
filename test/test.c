#include "stdio.h"
#include "strbuf.h"
#include "strview.h"

int main(void) {
	strbuf_t mimo = { 0 };
	strbuf_assign(&mimo, cstr_SL("Hello"));
	printf("[%s]\n", mimo.cstr);
	strbuf_destroy(&mimo);
	return 0;
}
