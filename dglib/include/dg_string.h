#pragma once
#include "dg_libcommon.h"

typedef struct dg_string_s {
	size_t   length;
	uint8_t* pstring;
} dg_string_t;

typedef const char* dg_cstr_t;

/**
* @brief Initialize a static dg_string_t from a string literal or a char array
* @param str - String literal or char array
*/
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

/**
* @brief compare two strings wiht option for case sensitivity
* @param a - first string
* @param b - second string
* @param casesensitive - if non-zero, comparison is case-sensitive
* @returns 0 if strings are equal, <0 if a < b, >0 if a > b
*/
DG_API int str_compare(const char* a, const char* b, int casesensitive);

/**
* @brief copy string from src to dst with max length
* @param dst - destination buffer
* @param maxlen - maximum length of the destination buffer
* @param src - source string
* @returns true on success, false if the source string is too long
*/
DG_API bool str_copy(char* dst, size_t maxlen, const char* src);

/**
* @brief string length calculation
* @param str - input string
* @returns length of the string (not including the null terminator)
*/
DG_API size_t str_length(const char* str);

/**
* @brief duplicate a string, allocating memory for it
* @param dstlen - pointer to size_t to receive the length of the duplicated string
* @param src - source string to duplicate
* @returns pointer to the duplicated string, or NULL on failure
*/
DG_API char* str_duplicate(size_t* dstlen, const char* src);

/**
* @brief free a duplicated string
* @param s - pointer to the string to free
* @returns nothing
* 
* @note the string must have been allocated by str_duplicate()
*/
DG_API void str_free(char* s);

/**
* @brief convert string to lower case
* @param s - string to convert
* @returns pointer to the converted string
* 
* @note the conversion is done in-place
*/
DG_API char* str_to_lower(char* s);

/**
* @brief convert string to upper case
* @param s - string to convert
* @returns pointer to the converted string
* 
* @note the conversion is done in-place
*/
DG_API char* str_to_upper(char* s);

/**
* @brief remove all occurrences of a character from a string
* @param s - string to modify
* @param chr - character to remove
* @returns number of characters removed
*/
DG_API size_t str_remove_char(char* s, int chr);

/**
* @brief replace all occurrences of a character in a string with another character
* @param s - string to modify
* @param chr - character to replace
* @param rep - replacement character
* @returns number of characters replaced
*/
DG_API size_t str_replace_char(char* s, int chr, int rep);

/**
* @brief check if a string contains a substring wiht option for case sensitivity
* @param s - string to search in
* @param frag - substring to search for
* @param casesensitive - if non-zero, search is case-sensitive
* @returns pointer to the first occurrence of the substring, or NULL if not found
*/
DG_API char* str_contains(const char* s, const char* frag, int casesensitive);

/**
* @brief find first occurrence of a character in a string
* @param s - string to search in
* @param chr - character to search for
* @returns pointer to the first occurrence of the character, or NULL if not found
*/
DG_API char* str_find_char(const char* s, int chr);

/**
* @brief find last occurrence of a character in a string
* @param s - string to search in
* @param chr - character to search for
* @returns pointer to the last occurrence of the character, or NULL if not found
*/
DG_API char* str_rfind_char(const char* s, int chr);

/**
* @brief replace all occurrences of a substring in a string with another substring
* @param s - string to modify
* @param maxlen - maximum length of the string buffer
* @param frag - substring to replace
* @param rep - replacement substring
* @returns number of replacements made
*/
DG_API size_t str_replace_string(char* s, size_t maxlen, const char* frag, const char* rep);

/**
* @brief check if a string is numeric (only digits)
* @param s - string to check
* @returns true if the string is numeric, false otherwise
*/
DG_API bool str_is_numeric(const char* s);

/**
* @brief check if a string is alphabetic (only letters)
* @param s - string to check
* @returns true if the string is alphabetic, false otherwise
*
*/
DG_API bool str_is_alpha(const char* s);

/**
* @brief split a string into tokens based on delimiters
* @param dst - array of pointers to receive the tokens
* @param src - source string to split
* @param n - maximum number of tokens to extract
* @param maxlen - maximum length of each token
* @param delim - string of delimiter characters
* @param casesensitive - if non-zero, tokenization is case-sensitive
* @returns number of tokens extracted
*/
DG_API size_t str_split(char* dst[], const char* src, size_t n, size_t maxlen, const char* delim, int casesensitive);

/**
* @brief trim leading and trailing whitespace from a string
* @param s - string to trim
* @returns pointer to the trimmed string
*/
DG_API char* str_trim(char* s);

/**
* @brief count occurrences of characters from a set in a string
* @param dst - pointer to size_t to receive the count
* @param maxlen - maximum length of the string to check
* @param s - string to check
* @param syms - string of characters to count
* @returns number of characters counted
*/
DG_API size_t   str_chrcount(size_t* dst, size_t maxlen, const char* s, const char* syms);

/**
* @brief filter out bad characters from a string (non-printable and non-ASCII)
* @param s - string to filter
* @returns nothing
*/
DG_API void     str_filter_bad_chars(char* s);

/**
* @brief concatenate two strings with maximum length
* @param pdst - destination string buffer
* @param maxlen - maximum length of the destination buffer
* @param psrc - source string to append
* @returns pointer to the destination string, or NULL if the result would exceed maxlen
* 
* @note the destination string must be null-terminated
*/
DG_API char*    str_concat(char *pdst, size_t maxlen, const char *psrc);

enum STRD_TYPE {
	STRD_HEXVAL = 0,
	STRD_BINVAL,
	STRD_OCTVAL 
};

/**
* @brief convert binary, hex or octal string to byte array
* @param dst - destination byte array
* @param maxlen - maximum length of the destination array
* @param type - type of the source string (STRD_HEXVAL, STRD_BINVAL, STRD_OCTVAL)
* @param src - source string to convert
* @returns pointer to the source string if successful, NULL on failure
*/
DG_API const char* str_data(uint8_t* dst, size_t maxlen, int type, const char* src);

/**
* @brief find a signature in a memory block
* @param pstart - start of the memory block
* @param pend - end of the memory block
* @param sig - signature to search for
* @param mask - mask for the signature (use '?' for wildcard)
* @returns pointer to the start of the signature if found, NULL if not found
*/
DG_API uint8_t* str_sig(const char* pstart, const char* pend, const char* sig, const char* mask);

/**
* @brief convert a hex character to its integer value
* @param sym - hex character (0-9, a-f, A-F)
* @returns integer value of the hex character, or -1 if invalid
*/
DG_API int chr_hex_tetrade(int sym);

/**
* @brief convert a hex string to its byte value
* @param hex - hex string (at least two characters)
* @returns byte value of the hex string, or -1 if invalid
* 
* @note the hex string must be at least two characters long
*/
DG_API int str_hex_byte(const char* hex);

/**
* @brief fill a memory block bytes with a value
* @param dst - destination memory block
* @param value - value to fill with
* @param count - number of bytes to fill
* @returns pointer to the destination memory block
*/
DG_API void* mem_set(void* dst, int value, size_t count);

/**
* @brief copy memory from source to destination
* @param dst - destination memory block
* @param src - source memory block
* @param count - number of bytes to copy
* @returns pointer to the destination memory block
*/
DG_API void* mem_copy(void* dst, const void* src, size_t count);

/**
* @brief move memory from source to destination (handles overlapping regions)
* @param dst - destination memory block
* @param src - source memory block
* @param count - number of bytes to move
* @returns pointer to the destination memory block
*/
DG_API void* mem_move(void* dst, const void* src, size_t count);

/**
* @brief compare two memory blocks
* @param a - first memory block
* @param b - second memory block
* @param count - number of bytes to compare
* @returns 0 if equal, <0 if a < b, >0 if a > b
*/
DG_API int mem_compare(const void* a, const void* b, size_t count);

/**
* @brief format a string with variable arguments
* @param pdst - destination buffer
* @param dstlen - length of the destination buffer
* @param pformat - format string
* @param ... - variable arguments
* @returns pointer to the destination buffer
* 
* @note if dst is NULL, the function calling la_hunkalloc to allocate memory for the formatted string, size of memory block is returned in dstlen
* @note if dst is NULL and dstlen is 0, the function returns NULL
*/
DG_API const char* str_format(char *pdst, size_t dstlen, const char *pformat, ...);

/**
* @brief format a string with variable arguments list
* @param pdst - destination buffer
* @param dstlen - length of the destination buffer
* @param pformat - format string
* @param argptr - variable arguments list
* @returns pointer to the destination buffer
* 
* @note if dst is NULL, the function calling la_hunkalloc to allocate memory for the formatted string, size of memory block is returned in dstlen
* @note if dst is NULL and dstlen is 0, the function returns NULL
*/
DG_API const char* str_vformat(char *pdst, size_t dstlen, const char* pformat, va_list argptr);

/**
* @brief copy string from src to a dg_string_t
* @param dst - destination dg_string_t
* @param src - source string
* @returns true on success, false on failure (e.g. memory allocation failure)
*/
DG_API bool dgstr_copy_from(dg_string_t* dst, const char* src);

/**
* @brief duplicate a dg_string_t, allocating memory for it
* @param dstlen - pointer to size_t to receive the length of the duplicated string
* @param src - source dg_string_t to duplicate
* @returns pointer to the duplicated string, or NULL on failure
*/
DG_API bool dgstr_copy(dg_string_t* dst, const dg_string_t* src);

/**
* @brief append a string to a dg_string_t
* @param dst - destination dg_string_t
* @param src - source string to append
* @param length - length of the source string to append (if 0, the function calculates the length)
* @returns true on success, false on failure (e.g. memory allocation failure)
*/
DG_API bool     dgstr_append(dg_string_t* dst, const char* src, size_t length);

/**
* @brief insert a string into a dg_string_t at a specified position
* @param dst - destination dg_string_t
* @param pos - position to insert at (0 <= pos <= dst->length)
* @param src - source string to insert
* @returns true on success, false on failure (e.g. memory allocation failure)
*/
DG_API bool     dgstr_insert_from(dg_string_t* dst, size_t pos, const char* src);

/**
* remove all occurrences of a character from a dg_string_t
* @param dst - destination dg_string_t
* @param chr - character to remove
* @returns true on success, false if invalid parameters (e.g. dst is NULL)
*/
DG_API bool     dgstr_remove_chars(dg_string_t* dst, int chr);

/**
* @brief free a dg_string_t
* @param dst - pointer to the dg_string_t to free
* @returns nothing
*/
DG_API void     dgstr_free(dg_string_t* dst);

/**
* @brief trim leading and trailing whitespace from a dg_string_t
* @param dst - pointer to the dg_string_t to trim
* @returns return adjusted pointer
*/
static inline char* str_trim_left_fast(char* str)
{
	while (*str && CHR_IS_SPACE(*str))
		str++;
	return str;
}

/**
* @brief trim trailing whitespace from a string with known length
* @param str - string to trim
* @param length - length of the string
* @returns pointer to the trimmed string
* 
* @note the trimming is done in-place
*/
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

/**
* @brief trim leading and trailing whitespace from a string with known length
* @param str - string to trim
* @param length - length of the string (if 0, the function calculates the length)
* @returns pointer to the trimmed string
*/
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

/**
* @brief convert ANSI string to wide char string
* @param pdst - destination wide char string buffer
* @param dstcount - number of wide characters in the destination buffer
* @param psrc - source ANSI string
* @returns number of wide characters written (not including the null terminator), or -1 on failure
*/
DG_API int ansi_to_wide(dg_wchar_t *pdst, size_t dstcount, const char *psrc);

/**
* @brief convert wide char string to ANSI string
* @param pdst - destination ANSI string buffer
* @param dstcount - number of characters in the destination buffer
* @param psrc - source wide char string
* @returns number of characters written (not including the null terminator), or -1 on failure
*/
DG_API int wide_to_ansi(char *pdst, size_t dstcount, const dg_wchar_t *psrc);

/**
* @brief compute length of a wide char string
* @param s - input wide char string
* @returns length of the wide char string (not including the null terminator)
*/
DG_API size_t wcs_length(const dg_wchar_t* s);

/**
* @brief duplicate a wide char string, allocating memory for it
* @param dstlen - optional pointer to size_t to receive the length of the duplicated string, may be NULL
* @param s - source wide char string to duplicate
* @returns pointer to the duplicated wide char string, or NULL on failure
*/
DG_API dg_wchar_t* wcs_duplicate(size_t* dstlen, const dg_wchar_t* s);

/**
* @brief free a duplicated wide char string
* @param s - pointer to the wide char string to free
* @returns nothing
* 
* @note the wide char string must have been allocated by wcs_duplicate()
*/
DG_API void wcs_free(const dg_wchar_t* s);

/**
* @brief copy wide char string from src to dst with max length
* @param pdst - destination wide char string buffer
* @param dstlen - number of wide characters in the destination buffer
* @param psrc - source wide char string
* @param count - maximum number of wide characters to copy from source (if 0, the function calculates the length)
* @returns pointer to the destination wide char string, or NULL if the source string is too long
*/
DG_API dg_wchar_t* wcs_copy(dg_wchar_t *pdst, size_t dstlen, const dg_wchar_t *psrc, size_t count);