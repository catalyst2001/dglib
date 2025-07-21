#pragma once
#include "dg_libcommon.h"

typedef struct dg_string_s {
	size_t   length;
	uint8_t* pstring;
} dg_string_t;

#define string_init_static(str) ((dg_string_t){\
		.length = sizeof(str)-sizeof(str[0]),\
		.pstring = str\
	})

/*
* ========================================
* STRING MACRO
* ========================================
*/
#define CHR_IS_LOWER(x) ((x) >= 'a' && (x) <= 'z')
#define CHR_TO_LOWER(x) ((x) + 32)
#define CHR_IS_UPPER(x) ((x) >= 'A' && (x) <= 'Z')
#define CHR_TO_UPPER(x) ((x) - 32)
#define CHR_IS_SPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\v' || (x) == '\f' || (x) == '\r')
#define CHR_IS_ALPHA(x) (CHR_IS_LOWER(x) || CHR_IS_UPPER(x))
#define CHR_IS_NUMERIC(x) ((x) >= '0' && (x) <= '9')
#define CHR_IS_PUNCT(c) (                                     \
    ((c) >= '!' && (c) <= '/')  /*  ! " # $ % & ' ( ) * + , - . / */ \
 || ((c) >= ':' && (c) <= '@')  /*  : ; < = > ? @ */             \
 || ((c) >= '[' && (c) <= '`')  /*  [ \ ] ^ _ ` */               \
 || ((c) >= '{' && (c) <= '~')  /*  { | } ~ */                  \
)

/*
* ========================================
* STRING UTILITY
* ========================================
*/
bool   str_copy(char *pdst, size_t maxlen, const char *psrc);
size_t str_length(const char *pstr);
char*  str_duplicate(size_t *pdstlen, const char *psrc);
void   str_free(char* pdupstr);
char*  str_to_lower(char* pstr);
char*  str_to_upper(char* pstr);
size_t str_remove_char(char *pstr, int chr);
size_t str_replace_char(char* pstr, int chr, int rep);
char*  str_contains(const char *pstr, const char *pfrag);
char*  str_find_char(const char* pstr, int chr);
char*  str_rfind_char(const char* pstr, int chr);
size_t str_replace_string(char* pstr, 
	size_t maxlen, 
	const char *pfrag, 
	const char *prep);

bool str_is_numeric(const char *pstr);

bool str_is_alpha(const char* pstr);

size_t str_split(char *pdst[],
	const char* psrc,
	size_t nsplits, 
	size_t maxlen, 	
	const char *pdelim);

void* mem_copy(void *pdst, const void *psrc, size_t count);
void* mem_move(void* pdst, const void* psrc, size_t count);

static inline char* str_trim_left_fast(char* str)
{
	while (*str && CHR_IS_SPACE(*str))
		str++;
	return str;
}

static inline char* str_trim_right_fast(char* str, size_t length)
{
	//if (!length)
	//	length = str_length(str);
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
	char* pltrimmed = str_trim_left_fast(str);
	assert(pltrimmed >= str && "trimmed address less source! size_t overflow!");
	length -= (size_t)(pltrimmed - str);
	return str_trim_right_fast(pltrimmed, length);
}

/*
* ========================================
* STRING FUNCTIONS
* ========================================
*/
bool string_copy_from(dg_string_t *pdst, const char *pstring);
bool string_copy(dg_string_t *pdst, const dg_string_t *psrc);
bool string_append(dg_string_t* pdst, const char *pstring);
bool string_insert_from(dg_string_t* pdst, size_t from, const char *psrc);
bool string_remove_chars(dg_string_t* pdst, int chr);
void string_free(dg_string_t* pdst);