#pragma once
#include "dg_libcommon.h"

typedef struct dg_string_s {
	size_t   length;
	uint8_t* pstring;
} dg_string_t;

#define string_init_static(str) ((dg_string_t){\
		.length = sizeof(str)-1,\
		.pstring = str\
	})

#define CHR_IS_LOWER(x) ((x) >= 'a' && (x) <= 'z')
#define CHR_TO_LOWER(x) ((x) + 32)
#define CHR_IS_UPPER(x) ((x) >= 'A' && (x) <= 'Z')
#define CHR_TO_UPPER(x) ((x) - 32)
#define CHR_IS_SPACE(x) ((x) == 0xFF || (x) == ' ' || (x) == '\n' || (x) == '\r')

bool   str_copy(char *pdst, size_t maxlen, const char *psrc);
size_t str_length(const char *pstr);
char*  str_duplicate_string(size_t *pdstlen, const char *psrc);
char*  str_to_lower(char* pstr);
char*  str_to_upper(char* pstr);

bool string_copy_from(dg_string_t *pdst, const char *pstring);
bool string_copy(dg_string_t *pdst, const dg_string_t *psrc);
void string_free(dg_string_t* pdst);