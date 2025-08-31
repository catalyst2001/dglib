#include "dg_string.h"
#include "dg_alloc.h"
#include "dg_linalloc.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#else
//linux
#endif

int str_compare(const char* a, const char* b, int casesensitive) {
	if (!casesensitive) {
		while (*a && *b) {
			unsigned char ca = (unsigned char)*a++;
			unsigned char cb = (unsigned char)*b++;
			if (CHR_IS_UPPER(ca))
				ca = CHR_TO_LOWER(ca);

			if (CHR_IS_UPPER(cb))
				cb = CHR_TO_LOWER(cb);

			if (ca != cb)
				return (int)ca - (int)cb;
		}
		return (int)*a - (int)*b;
	}
	else {
		while (*a && *a == *b) {
			a++;
			b++;
		}
		return (int)(unsigned char)*a - (int)(unsigned char)*b;
	}
}

bool str_copy(char* dst, size_t maxlen, const char* src) {
	if (!dst || !src || maxlen == 0)
		return false;

	size_t i = 0;
	for (; i < maxlen && src[i]; i++)
		dst[i] = src[i];

	if (i < maxlen) {
		dst[i] = '\0';
		return true;
	}
	dst[maxlen - 1] = '\0';
	return false;
}

size_t str_length(const char* str)
{
	const char* s = str;
	while (*s)
		s++;

	return (size_t)(s - str);
}

char* str_duplicate(size_t* dstlen, const char* src) {
	if (!src)
		return NULL;

	size_t len = str_length(src);
	char* dup = (char*)malloc(len + 1);
	if (!dup)
		return NULL;

	for (size_t i = 0; i < len; i++)
		dup[i] = src[i];

	dup[len] = '\0';
	if (dstlen)
		*dstlen = len;

	return dup;
}

void str_free(char* s)
{
	if (s)
		free(s);
}

char* str_to_lower(char* s)
{
	for (char* p = s; *p; p++)
		if (CHR_IS_UPPER((unsigned char)*p))
			*p = CHR_TO_LOWER((unsigned char)*p);

	return s;
}

char* str_to_upper(char* s)
{
	for (char* p = s; *p; p++)
		if (CHR_IS_LOWER((unsigned char)*p))
			*p = CHR_TO_UPPER((unsigned char)*p);

	return s;
}

size_t str_remove_char(char* s, int chr) {
	size_t write = 0, read = 0, len = 0;
	while (s[read]) {
		if (s[read] != chr) {
			s[write++] = s[read];
		}
		else {
			len++;
		}
		read++;
	}
	s[write] = '\0';
	return len;
}

size_t str_replace_char(char* s, int chr, int rep)
{
	size_t cnt = 0;
	for (size_t i = 0; s[i]; i++) {
		if (s[i] == chr) {
			s[i] = (char)rep;
			cnt++;
		}
	}
	return cnt;
}

char* str_contains(const char* s, const char* frag, int casesensitive)
{
	if (!frag[0])
		return (char*)s;

	size_t fl = str_length(frag);
	for (size_t i = 0; s[i]; i++) {
		size_t j = 0;
		for (; j < fl; j++) {
			char cs = s[i + j]; char cf = frag[j];
			if (!casesensitive) {
				if (CHR_IS_UPPER((unsigned char)cs))
					cs = CHR_TO_LOWER(cs);
				if (CHR_IS_UPPER((unsigned char)cf))
					cf = CHR_TO_LOWER(cf);
			}

			if (cs != cf)
				break;
		}
		if (j == fl)
			return (char*)&s[i];
	}
	return NULL;
}

char* str_find_char(const char* s, int chr)
{
	for (; *s; s++)
		if (*s == (char)chr)
			return (char*)s;

	return NULL;
}

char* str_rfind_char(const char* s, int chr)
{
	const char* res = NULL;
	for (size_t i = 0; s[i]; i++)
		if (s[i] == (char)chr)
			res = &s[i];

	return (char*)res;
}

size_t str_replace_string(char* s, size_t maxlen, const char* frag, const char* rep)
{
	if (!s || !frag || !rep)
		return 0;
	size_t sl = str_length(s),
		fl = str_length(frag),
		rl = str_length(rep);
	if (fl == 0)
		return sl;

	char* p = s;
	while ((p = str_contains(p, frag, 1))) {
		size_t off = (size_t)(p - s);
		size_t newl = sl - fl + rl;
		if (newl + 1 > maxlen)
			break;
		mem_move(p + rl, p + fl, sl - off - fl + 1);
		mem_copy(p, rep, rl);
		sl = newl;
		p += rl;
	}
	return sl;
}

bool str_is_numeric(const char* s)
{
	for (size_t i = 0; s[i]; i++)
		if (!CHR_IS_NUMERIC(s[i]))
			return false;
	return true;
}

bool str_is_alpha(const char* s)
{
	for (size_t i = 0; s[i]; i++)
		if (!CHR_IS_ALPHA(s[i]))
			return false;

	return true;
}

size_t str_split(char* dst[], const char* src, size_t n, size_t maxlen, const char* d, int cs)
{
	size_t count = 0;
	const char* p = src;
	size_t dl = str_length(d);
	while (count < n && (p = str_contains(p, d, cs))) {
		str_copy(dst[count], maxlen, p);
		p += dl;
		count++;
	}
	return count;
}

char* str_trim(char* s)
{
	if (!s || !*s)
		return s;

	char* l = str_trim_left_fast(s);
	size_t len = str_length(l);
	str_trim_right_fast(l, len);
	if (l != s)
		mem_move(s, l, str_length(l) + 1);
	return s;
}

size_t str_chrcount(size_t* dst, size_t maxlen, const char* s, const char* syms)
{
	size_t vc = 0;
	for (size_t i = 0; i < maxlen && syms[i]; i++) {
		dst[i] = 0;
		for (size_t j = 0; s[j]; j++)
			if (s[j] == syms[i])
				dst[i]++;

		if (dst[i])
			vc++;
	}
	return vc;
}

void str_filter_bad_chars(char* s)
{
	str_remove_char(s, '%');
}

char* str_concat(char* pdst, size_t maxlen, const char* psrc)
{
	if (!pdst || !psrc || maxlen == 0)
		return NULL;

	size_t dst_len = str_length(pdst);
	size_t src_len = str_length(psrc);
	if (dst_len >= maxlen) {
		pdst[maxlen - 1] = '\0';
		return NULL;
	}

	size_t avail = maxlen - dst_len - 1;
	size_t to_copy = (src_len < avail ? src_len : avail);
	mem_copy(pdst + dst_len, psrc, to_copy);
	pdst[dst_len + to_copy] = '\0';
	if (src_len > avail)
		return NULL;

	return pdst;
}

const char* str_data(uint8_t* dst, size_t maxlen, int type, const char* src)
{
	const char* prefix = NULL; size_t plen = 0;
	if (type == STRD_HEXVAL) { prefix = "0x"; plen = 2; }
	else if (type == STRD_BINVAL) { prefix = "0b"; plen = 2; }
	else if (type == STRD_OCTVAL) { prefix = "0"; plen = 1; }
	const char* p = str_contains(src, prefix, 0);
	if (!p) return src;
	p += plen;
	size_t i = 0;
	while (p[i] && i < maxlen) {
		int v = -1; unsigned char c = (unsigned char)p[i];
		if (type == STRD_HEXVAL) {
			if (CHR_IS_NUMERIC(c)) v = c - '0';
			else if (CHR_IS_ALPHA(c)) {
				if (CHR_IS_LOWER(c)) c = CHR_TO_UPPER(c);
				if (c >= 'A' && c <= 'F') v = (c - 'A') + 10;
			}
		}
		else if (type == STRD_BINVAL) {
			if (c == '0' || c == '1') v = c - '0';
		}
		else if (type == STRD_OCTVAL) {
			if (c >= '0' && c <= '7') v = c - '0';
		}
		if (v < 0) break;
		dst[i++] = (uint8_t)v;
	}
	return p + i;
}

uint8_t* str_sig(const char* pstart, const char* pend, const char* sig, const char* mask)
{
	for (const char* p = pstart; p < pend; p++) {
		size_t i;
		for (i = 0; mask[i]; i++) {
			if (mask[i] != '?' && sig[i] != p[i]) break;
		}
		if (!mask[i]) return (uint8_t*)p;
	}
	return NULL;
}

int chr_hex_tetrade(int sym)
{
	if (CHR_IS_NUMERIC(sym))
		return sym - '0';

	unsigned char c = sym;
	if (CHR_IS_LOWER(c))
		c = CHR_TO_UPPER(c);

	if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;

	return -1;
}

int str_hex_byte(const char* hex)
{
	return (chr_hex_tetrade(hex[0]) << 4) | chr_hex_tetrade(hex[1]);
}

void* mem_set(void* dst, int value, size_t count)
{
	unsigned char* d = (unsigned char*)dst;
	for (size_t i = 0; i < count; i++)
		d[i] = (unsigned char)value;
	return dst;
}

void* mem_copy(void* dst, const void* src, size_t count)
{
	unsigned char* d = (unsigned char*)dst;
	const unsigned char* s = (const unsigned char*)src;
	for (size_t i = 0; i < count; i++)
		d[i] = s[i];
	return dst;
}

void* mem_move(void* dst, const void* src, size_t count)
{
	unsigned char* d = (unsigned char*)dst;
	const unsigned char* s = (const unsigned char*)src;
	if (d < s) {
		for (size_t i = 0; i < count; i++)
			d[i] = s[i];
	}
	else {
		for (size_t i = count; i > 0; i--)
			d[i - 1] = s[i - 1];
	}
	return dst;
}

int mem_compare(const void* a, const void* b, size_t count)
{
	const unsigned char* x = (const unsigned char*)a;
	const unsigned char* y = (const unsigned char*)b;
	for (; count; count--) {
		if (*x != *y)
			return *x - *y;
		x++; y++;
	}
	return 0;
}

const char* str_format(char* pdst, size_t dstlen, const char* pformat, ...)
{
	va_list argptr;
	const char* pformated;
	va_start(argptr, pformat);
	pformated = str_vformat(pdst, dstlen, pformat, argptr);
	va_end(argptr);
	return pformated;
}

const char* str_vformat(char* pdst, size_t dstlen, const char* pformat, va_list argptr)
{
#ifdef _WIN32
	/* dst buffer present? */
	if (!pdst) {
		pdst = la_hunk_alloc(dstlen, true);
		if (!pdst) {
			/* failed to alloc buffer */
			return "";
		}
	}
	vsnprintf(pdst, dstlen, pformat, argptr);
	return pdst;
#else
	//TODO: K.D. "str_format" implement me
	return "";
#endif
}

bool dgstr_copy_from(dg_string_t* dst, const char* src) {
	if (!dst)
		return false;

	dst->pstring = (uint8_t*)str_duplicate(&dst->length, src);
	return dst->pstring != NULL;
}

bool dgstr_copy(dg_string_t* dst, const dg_string_t* src) {
	if (!dst || !src)
		return false;

	dst->pstring = (uint8_t*)str_duplicate(&dst->length, (char*)src->pstring);
	return dst->pstring != NULL;
}

bool dgstr_append(dg_string_t* dst, const char* src, size_t length) {
	if (!dst || !src)
		return false;

	size_t sl = str_length(src);
	if (length > sl)
		length = sl;

	size_t nl = dst->length + length;
	uint8_t* buf = (uint8_t*)realloc(dst->pstring, nl + 1);
	if (!buf)
		return false;

	mem_copy(buf + dst->length, src, length);
	buf[nl] = '\0';
	dst->pstring = buf;
	dst->length = nl;
	return true;
}

bool dgstr_insert_from(dg_string_t* dst, size_t pos, const char* src) {
	if (!dst || !src)
		return false;

	size_t sl = str_length(src);
	if (sl == 0)
		return true;

	if (pos > dst->length)
		pos = dst->length;

	size_t nl = dst->length + sl;
	uint8_t* buf = (uint8_t*)realloc(dst->pstring, nl + 1);
	if (!buf)
		return false;

	mem_move(buf + pos + sl, buf + pos, dst->length - pos + 1);
	mem_copy(buf + pos, src, sl);
	buf[nl] = '\0'; dst->pstring = buf; dst->length = nl;
	return true;
}

bool dgstr_remove_chars(dg_string_t* dst, int chr) {
	if (!dst || !dst->pstring)
		return false;

	size_t n = str_remove_char((char*)dst->pstring, chr);
	if (n)
		dst->length -= n;

	return n > 0;
}

void dgstr_free(dg_string_t* dst)
{
	if (dst && dst->pstring)
		free(dst->pstring);
	if (dst)
		dst->length = 0;
}

size_t wcs_length(const dg_wchar_t* s)
{
	const dg_wchar_t* p = s;
	size_t len = 0;
	while (p && *p) {
		++len;
		++p;
	}
	return len;
}

dg_wchar_t* wcs_duplicate(size_t* dstlen, const dg_wchar_t* src)
{
	if (!src)
		return NULL;

	size_t len = wcs_length(src);
	dg_wchar_t* dup = (dg_wchar_t*)malloc(sizeof(dg_wchar_t) * (len + 1));
	if (!dup)
		return NULL;

	for (size_t i = 0; i < len; i++)
		dup[i] = src[i];

	dup[len] = '\0';
	if (dstlen)
		*dstlen = len;

	return dup;
}

void wcs_free(const dg_wchar_t* src)
{
	if (src) {
		free(src);
	}
}

dg_wchar_t* wcs_copy(dg_wchar_t* pdst, size_t dstlen, const dg_wchar_t* psrc, size_t count)
{
	if (!pdst || !psrc || dstlen == 0)
		return NULL;

	if (!count)
		count = wcs_length(psrc);

	if (count > dstlen - 1)
		count = dstlen - 1;

	mem_copy(pdst, psrc, count * sizeof(*psrc));
	pdst[count] = 0x0000;
	return pdst;
}

#ifndef _WIN32
static inline uint32_t utf8_decode_cp(const unsigned char* s, size_t* adv)
{
	uint32_t cp;
	unsigned char c = s[0];

	if (c < 0x80) { *adv = 1; return c; }

	// 2-byte
	if ((c & 0xE0) == 0xC0) {
		unsigned char c1 = s[1];
		if ((c1 & 0xC0) != 0x80) { *adv = 1; return 0xFFFD; }
		cp = ((c & 0x1F) << 6) | (c1 & 0x3F);
		if (cp < 0x80) { *adv = 1; return 0xFFFD; } // overlong
		*adv = 2; return cp;
	}

	// 3-byte
	if ((c & 0xF0) == 0xE0) {
		unsigned char c1 = s[1], c2 = s[2];
		if (((c1 | c2) & 0xC0) != 0x80) { *adv = 1; return 0xFFFD; }
		cp = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
		if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) { *adv = 1; return 0xFFFD; } // overlong/surrogate
		*adv = 3; return cp;
	}

	// 4-byte
	if ((c & 0xF8) == 0xF0) {
		unsigned char c1 = s[1], c2 = s[2], c3 = s[3];
		if (((c1 | c2 | c3) & 0xC0) != 0x80) { *adv = 1; return 0xFFFD; }
		cp = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
		if (cp < 0x10000 || cp > 0x10FFFF) { *adv = 1; return 0xFFFD; } // overlong/out of range
		*adv = 4; return cp;
	}

	*adv = 1; return 0xFFFD;
}

static inline size_t utf16_units_for_cp(uint32_t cp) {
	return (cp <= 0xFFFF) ? 1u : 2u;
}

static inline size_t utf8_bytes_for_cp(uint32_t cp) {
	if (cp <= 0x7F) return 1;
	if (cp <= 0x7FF) return 2;
	if (cp <= 0xFFFF) return 3;
	return 4;
}

static inline void utf16_write_cp(dg_wchar_t* dst, size_t* out, size_t cap, uint32_t cp)
{
	if (cp <= 0xFFFF) {
		if (*out < cap) dst[(*out)++] = (dg_wchar_t)cp;
	}
	else {
		if (*out + 1 < cap) {
			cp -= 0x10000;
			dst[(*out)++] = (dg_wchar_t)(0xD800 | (cp >> 10));
			dst[(*out)++] = (dg_wchar_t)(0xDC00 | (cp & 0x3FF));
		}
	}
}

static inline uint32_t utf16_read_cp(const dg_wchar_t* s, size_t len, size_t* adv)
{
	if (*adv >= len) return 0;
	uint32_t w1 = s[*adv];
	(*adv)++;
	if (w1 >= 0xD800 && w1 <= 0xDBFF) { // high surrogate
		if (*adv < len) {
			uint32_t w2 = s[*adv];
			if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
				(*adv)++;
				uint32_t cp = 0x10000 + (((w1 - 0xD800) << 10) | (w2 - 0xDC00));
				return cp;
			}
		}
		return 0xFFFD; // unmatched high surrogate
	}
	if (w1 >= 0xDC00 && w1 <= 0xDFFF) {
		return 0xFFFD; // stray low surrogate
	}
	return w1;
}

static inline void utf8_write_cp(char* dst, size_t* out, size_t cap, uint32_t cp)
{
	if (cp <= 0x7F) {
		if (*out < cap) dst[(*out)++] = (char)cp;
	}
	else if (cp <= 0x7FF) {
		if (*out + 1 < cap) {
			dst[(*out)++] = (char)(0xC0 | (cp >> 6));
			dst[(*out)++] = (char)(0x80 | (cp & 0x3F));
		}
	}
	else if (cp <= 0xFFFF) {
		if (*out + 2 < cap) {
			dst[(*out)++] = (char)(0xE0 | (cp >> 12));
			dst[(*out)++] = (char)(0x80 | ((cp >> 6) & 0x3F));
			dst[(*out)++] = (char)(0x80 | (cp & 0x3F));
		}
	}
	else {
		if (*out + 3 < cap) {
			dst[(*out)++] = (char)(0xF0 | (cp >> 18));
			dst[(*out)++] = (char)(0x80 | ((cp >> 12) & 0x3F));
			dst[(*out)++] = (char)(0x80 | ((cp >> 6) & 0x3F));
			dst[(*out)++] = (char)(0x80 | (cp & 0x3F));
		}
	}
}
#endif /* !_WIN32 */

int ansi_to_wide(dg_wchar_t* pdst, size_t dstcount, const char* psrc)
{
	if (!psrc)
		return -1;

#ifdef _WIN32
	// required (includes NUL when cbMultiByte == -1)
	int need = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, psrc, -1, NULL, 0);
	if (need <= 0)
		return -1;

	if (!pdst || dstcount == 0)
		return need - 1; // without NUL

	int written = MultiByteToWideChar(CP_ACP, 0, psrc, -1, (LPWSTR)pdst, (int)dstcount);
	if (written <= 0)
		return -1;

	return written - 1; // exclude NUL
#else
	// treat "ANSI" as UTF-8
	const unsigned char* s = (const unsigned char*)psrc;
	// 1) measure
	size_t need_units = 0;
	while (*s) {
		size_t adv = 0;
		uint32_t cp = utf8_decode_cp(s, &adv);
		need_units += utf16_units_for_cp(cp);
		s += adv;
	}
	if (!pdst || dstcount == 0) return (int)need_units;

	// 2) convert
	size_t out = 0;
	s = (const unsigned char*)psrc;
	while (*s) {
		if (out + 1 >= dstcount) break; // keep space for NUL (or room for surrogate pair below)
		size_t adv = 0;
		uint32_t cp = utf8_decode_cp(s, &adv);
		size_t units = utf16_units_for_cp(cp);
		if (out + units >= dstcount) break; // not enough room for full code point
		utf16_write_cp(pdst, &out, dstcount - 1, cp); // -1 to guarantee room for NUL
		s += adv;
	}
	pdst[out] = 0;
	return (int)out;
#endif
}

int wide_to_ansi(char* pdst, size_t dstcount, const dg_wchar_t* psrc)
{
	if (!psrc)
		return -1;

#ifdef _WIN32
	int need = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)psrc, -1, NULL, 0, NULL, NULL);
	if (need <= 0)
		return -1;

	if (!pdst || dstcount == 0)
		return need - 1;

	int written = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)psrc, -1, pdst, (int)dstcount, NULL, NULL);
	if (written <= 0)
		return -1;

	return written - 1;
#else
	// treat psrc as UTF-16, encode to UTF-8
	// 1) measure
	size_t len16 = 0; while (psrc[len16] != 0) ++len16;
	size_t i = 0, need_bytes = 0;
	while (i < len16) {
		size_t adv = i;
		uint32_t cp = utf16_read_cp(psrc, len16, &adv);
		i = adv;
		need_bytes += utf8_bytes_for_cp(cp);
	}
	if (!pdst || dstcount == 0) return (int)need_bytes;

	// 2) convert
	size_t out = 0; i = 0;
	while (psrc[i] && out + 1 < dstcount) {
		size_t adv = i;
		uint32_t cp = utf16_read_cp(psrc, (size_t)-1, &adv); // len not needed since we check NUL
		i = adv;
		size_t need = utf8_bytes_for_cp(cp);
		if (out + need >= dstcount) break; // leave space for NUL or avoid overflow
		utf8_write_cp(pdst, &out, dstcount - 1, cp);
	}
	pdst[out] = '\0';
	return (int)out;
#endif
}
