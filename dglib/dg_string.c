#include "dg_string.h"
#include "dg_alloc.h"

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

	size_t src_len = wcs_length(psrc);
	size_t to_copy = (count == 0 ? src_len : count);
	if (to_copy > dstlen - 1)
		to_copy = dstlen - 1;

	mem_copy(pdst, psrc, to_copy * sizeof(*psrc));
	pdst[to_copy] = 0x0000;
	return pdst;
}
