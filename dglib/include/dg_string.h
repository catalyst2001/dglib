#pragma once
#include "dg_libcommon.h"

typedef struct dg_string_s {
	size_t   length;
	uint8_t* pstring;
} dg_string_t;

typedef const char* dg_cstr_t;

#define string_init_static(str) ((dg_string_t){             \
        .length = sizeof(str)-sizeof(str[0]),               \
        .pstring = (uint8_t*)(str)                          \
    })

#define CHR_IS_LOWER(x) ((x) >= 'a' && (x) <= 'z')
#define CHR_TO_LOWER(x) ((x) + 32)
#define CHR_IS_UPPER(x) ((x) >= 'A' && (x) <= 'Z')
#define CHR_TO_UPPER(x) ((x) - 32)
#define CHR_IS_SPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\v' || (x) == '\f' || (x) == '\r')
#define CHR_IS_ALPHA(x) (CHR_IS_LOWER(x) || CHR_IS_UPPER(x))
#define CHR_IS_NUMERIC(x) ((x) >= '0' && (x) <= '9')
#define CHR_IS_PUNCT(c)  (                                    \
    ((c) >= '!' && (c) <= '/')  /*  ! " # $ % & ' ( ) * + , - . / */ \
 || ((c) >= ':' && (c) <= '@')  /*  : ; < = > ? @ */           \
 || ((c) >= '[' && (c) <= '`')  /*  [ \\ ] ^ _ ` */             \
 || ((c) >= '{' && (c) <= '~')  /*  { | } ~ */                  \
)

// PROTOTYPES
DG_API int      str_compare(const char* a, const char* b, int casesensitive);
DG_API bool     str_copy(char* dst, size_t maxlen, const char* src);
DG_API size_t   str_length(const char* str);
DG_API char*    str_duplicate(size_t* dstlen, const char* src);
DG_API void     str_free(char* s);
DG_API char*    str_to_lower(char* s);
DG_API char*    str_to_upper(char* s);
DG_API size_t   str_remove_char(char* s, int chr);
DG_API size_t   str_replace_char(char* s, int chr, int rep);
DG_API char*    str_contains(const char* s, const char* frag, int casesensitive);
DG_API char*    str_find_char(const char* s, int chr);
DG_API char*    str_rfind_char(const char* s, int chr);
DG_API size_t   str_replace_string(char* s, size_t maxlen, const char* frag, const char* rep);
DG_API bool     str_is_numeric(const char* s);
DG_API bool     str_is_alpha(const char* s);
DG_API size_t   str_split(char* dst[], const char* src, size_t n, size_t maxlen, const char* delim, int casesensitive);
DG_API char* str_trim(char* s);
DG_API size_t   str_chrcount(size_t* dst, size_t maxlen, const char* s, const char* syms);
DG_API void     str_filter_bad_chars(char* s);
DG_API char*    str_concat(char *pdst, size_t maxlen, const char *psrc);

enum STRD_TYPE {
	STRD_HEXVAL = 0,
	STRD_BINVAL,
	STRD_OCTVAL 
};
DG_API const char* str_data(uint8_t* dst, size_t maxlen, int type, const char* src);

DG_API uint8_t* str_sig(const char* pstart, const char* pend, const char* sig, const char* mask);
DG_API int      chr_hex_tetrade(int sym);
DG_API int      str_hex_byte(const char* hex);
DG_API void*    mem_set(void* dst, int value, size_t count);
DG_API void*    mem_copy(void* dst, const void* src, size_t count);
DG_API void*    mem_move(void* dst, const void* src, size_t count);
DG_API int      mem_compare(const void* a, const void* b, size_t count);

DG_API const char* str_format(char *pdst, size_t dstlen, const char *pformat, ...);
DG_API const char* str_vformat(char *pdst, size_t dstlen, const char* pformat, va_list argptr);

DG_API bool     dgstr_copy_from(dg_string_t* dst, const char* src);
DG_API bool     dgstr_copy(dg_string_t* dst, const dg_string_t* src);
DG_API bool     dgstr_append(dg_string_t* dst, const char* src, size_t length);
DG_API bool     dgstr_insert_from(dg_string_t* dst, size_t pos, const char* src);
DG_API bool     dgstr_remove_chars(dg_string_t* dst, int chr);
DG_API void     dgstr_free(dg_string_t* dst);

static inline char* str_trim_left_fast(char* str)
{
	while (*str && CHR_IS_SPACE(*str))
		str++;
	return str;
}

static inline char* str_trim_right_fast(char* str, size_t length)
{
	if (length) {
		char* ptr = &str[length - 1];
		while (ptr > str && CHR_IS_SPACE(*ptr)) {
			*ptr = '\0';
			ptr--;
		}
	}
	return str;
}

static inline char* str_trim_fast(char* str, size_t length) {
	if (!length)
		length = str_length(str);
	char* pl = str_trim_left_fast(str);
	length -= (size_t)(pl - str);
	return str_trim_right_fast(pl, length);
}

/*
==========================
 wide char string functions
==========================
*/
#ifdef _MSC_VER
typedef wchar_t dg_wchar_t;
#else
typedef unsigned short dg_wchar_t;
#endif

DG_API int         ansi_to_wide(dg_wchar_t *pdst, size_t dstcount, const char *psrc);
DG_API int         wide_to_ansi(char *pdst, size_t dstcount, const dg_wchar_t *psrc);

DG_API size_t      wcs_length(const dg_wchar_t* s);
DG_API dg_wchar_t* wcs_duplicate(size_t* dstlen, const dg_wchar_t* s);
DG_API void        wcs_free(const dg_wchar_t* s);
DG_API dg_wchar_t* wcs_copy(dg_wchar_t *pdst, size_t dstlen, const dg_wchar_t *psrc, size_t count);