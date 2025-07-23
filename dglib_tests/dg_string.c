#include "dg_string.h"
#include "dg_alloc.h"

//TODO: K.D. OPTIMIZE ALL STRING OPERATIONS
// 1. mem_set dst memory must be aligned to int/size_t for fast fill

int str_compare(const char* pstra, const char* pstrb, int casesensitive)
{
  return 0;
}

bool str_copy(char* pdst, size_t maxlen, const char* psrc)
{
  size_t i;
  for (i = 0; psrc[i] && i < maxlen; i++)
    pdst[i] = psrc[i];

  if (i < maxlen) {
    pdst[i] = psrc[i];
    return true;
  }

  /* clamp */
  pdst[i - 1] = '\0';
  return false;
}

size_t str_length(const char* pstr)
{
  size_t len = 0;
  while (*pstr++)
    len++;

  return len;
}

char* str_duplicate(size_t* pdstlen, const char* psrc)
{
  char*  pstr;
  size_t length = str_length(psrc);
  if (!length)
    return NULL;

  pstr = malloc(length+1);
  if (!pstr)
    return NULL;

  if (pdstlen)
    *pdstlen = length;

  str_copy(pstr, length, psrc);
  return pstr;
}

void str_free(char* pdupstr)
{
  if (pdupstr) {
    free(pdupstr);
  }
}

char* str_to_lower(char* pstr)
{
  char* psrc = pstr;
  while (*psrc++)
    if (CHR_IS_UPPER(*psrc))
      *psrc = CHR_TO_LOWER(*psrc);

  return pstr;
}

char* str_to_upper(char* pstr)
{
  char* psrc = pstr;
  while (*psrc++)
    if (CHR_IS_LOWER(*psrc))
      *psrc = CHR_TO_UPPER(*psrc);

  return pstr;
}

size_t str_remove_char(char* pstr, int chr)
{
  size_t len_to_move;
  size_t len = str_length(pstr);
  size_t src_len = len;
  if (len) {
    for (size_t i = 0; pstr[i] && i < len; i++) {
      if (pstr[i] == chr) {
        len_to_move = len - i;
        mem_move(&pstr[i], &pstr[i + 1], len_to_move);
        i = 0;
        len--;
      }
    }
    return src_len - len;
  }
  return 0;
}

size_t str_replace_char(char* pstr, int chr, int rep)
{
  size_t nreps = 0;
  for (size_t i = 0; pstr[i]; i++) {
    if (pstr[i]== chr) {
      pstr[i] = rep;
      nreps++;
    }
  }
  return nreps;
}

char* str_contains(const char* pstr, const char* pfrag, int casesensitive)
{
  assert(pstr != NULL);
  assert(pfrag != NULL);
  if (*pfrag == '\0')
    return pstr;

  for (size_t i = 0; pstr[i] != '\0'; i++) {
    size_t j = 0;
    while (pfrag[j] != '\0' && pstr[i + j] != '\0' && pstr[i + j] == pfrag[j])
      j++;
    
    if (pfrag[j] == '\0') {
      return &pstr[i];
    }
  }
  return NULL;
}

char* str_find_char(const char* pstr, int chr)
{
  while (*pstr) {
    if (*pstr == chr)
      return pstr;

    pstr++;
  }
  return NULL;
}

char* str_rfind_char(const char* pstr, int chr)
{
  char* pend;
  size_t len = str_length(pstr);
  if (!len)
    return NULL;

  len++;
  pend = pstr + len;
  while (pend > pstr) {
    if (*pend == chr) {
      return pend;
    }
    pend--;
  }
  return NULL;
}

/*
 * Replaces all (non-overlapping) occurrences of pfrag in pstr with prep,
 * without exceeding buffer size maxlen (including terminating '\0').
 * Returns the new length of the string in pstr.
 */
size_t str_replace_string(char* pstr,
  size_t maxlen,
  const char* pfrag,
  const char* prep)
{
  if (!pstr || !pfrag || !prep)
    return 0;

  size_t len = strlen(pstr);
  size_t len_frag = strlen(pfrag);
  size_t len_rep = strlen(prep);

  /* empty fragment => nothing to do */
  if (len_frag == 0)
    return len;

  char* match = pstr;
  while ((match = strstr(match, pfrag)) != NULL) {
    size_t offset = match - pstr;
    size_t new_len = len + len_rep - len_frag;

    /* not enough room for this replacement + '\0' */
    if (new_len + 1 > maxlen)
      break;

    /* shift the tail to its new position */
    memmove(match + len_rep,
      match + len_frag,
      len - offset - len_frag + 1);  /* +1 to move the '\0' too */

    /* copy replacement into place */
    mem_copy(match, prep, len_rep);

    /* update length and advance past the inserted text */
    len = new_len;
    match += len_rep;
  }

  return len;
}

bool str_is_numeric(const char* pstr)
{
  for (size_t i = 0; pstr[i]; i++)
    if (!CHR_IS_NUMERIC(pstr[i]))
      return false;

  return true;
}

bool str_is_alpha(const char* pstr)
{
  for (size_t i = 0; pstr[i]; i++)
    if (!CHR_IS_ALPHA(pstr[i]))
      return false;

  return true;
}

size_t str_split(char* pdst[], 
  const char* psrc,
  size_t nsplits,
  size_t maxlen,
  const char* pdelim,
  int casesensitive)
{
  size_t isplit = 0;
  size_t delim_len = str_length(pdelim);
  size_t srclen = str_length(psrc);
  char* ptr = psrc;
  char* pend = ptr + srclen;
  do {
    ptr = str_contains(ptr, pdelim, casesensitive);
    if (ptr) {
      str_copy(pdst[isplit], maxlen, ptr);
      ptr += delim_len;
      if (ptr > pend) {
        ptr = pend;
      }
    }
  } while (ptr);
  return isplit;
}

char* str_trim(char* pstr)
{
  size_t len;
  char* psrc = pstr;
  if (!pstr || !pstr[0])
    return pstr; //skip empty string

  //TODO: K.D. optimize string length computation
  pstr = str_trim_left_fast(pstr);
  len = str_length(pstr);
  pstr = str_trim_right_fast(pstr, len);
  len = str_length(pstr);
  if (psrc != pstr && len)
    mem_move(psrc, pstr, len+1); //len+1 for copy '\0'
  
  return psrc;
}

size_t str_chrcount(size_t* pdst, 
  size_t maxlen, 
  const char* pstr, 
  const char* psymsstr)
{
  size_t vcount = 0;
  for (size_t i = 0; i < maxlen 
    && psymsstr[i]; i++) {
    pdst[i] = 0;
    for (size_t j = 0; pstr[j]; j++) {
      if (pstr[j] == psymsstr[i]) {
        /* sym found */
        pdst[i]++;
      }
    }

    if (pdst[i]) {
      vcount++;
    }
  }
  return vcount;
}

void str_filter_bad_chars(char* pstr)
{
  //TODO: K.D. add other format specifiers here
  str_remove_char(pstr, '%');
}

static inline int read_tetrade_inl(int sym)
{
  if (DG_INRANGE(sym, '0', '9'))
    return sym - '0';

  sym = CHR_TO_LOWER(sym);
  if (DG_INRANGE(sym, 'a', 'f'))
    return (sym - 'a')+10;
  
  return -1;
}

static inline int read_hex_byte_inl(const char* pbyte)
{
  int low = read_tetrade_inl(pbyte[0]);
  int high = read_tetrade_inl(pbyte[1]);
  return (low << 4) | high;
}

const char* str_data(uint8_t* pdst, size_t maxlen, int strd_type, const char* psrc)
{
#define BINPREFIX "0b"
  DG_CONST_STRLEN(BINPREF_LENGTH, BINPREFIX);
  const char* pstarthex = str_contains(psrc, BINPREFIX, 0);
  if (!pstarthex)
    return psrc; //bin pref not found, return src address

  /* found bin prefix */
  pstarthex += BINPREF_LENGTH;

  //TODO: K.D. CONTINUE

}

uint8_t* str_sig(const char* pstart,
  const char* pend, 
  const char* psig, 
  const char* pmask)
{
  for (const char *paddr = pstart; paddr < pend; paddr++) {
    for (size_t i = 0; pmask[i]; i++) {
      if (pmask[i] != '?' && psig[i] != paddr[i]) {
        /* not found */
        break;
      }
    }
    /* found */
    return paddr;
  }
  return NULL;
}

int chr_hex_tetrade(int sym)
{
  return read_tetrade_inl(sym);
}

int str_hex_byte(const char* phexbyte)
{
  return read_hex_byte_inl(phexbyte);
}

void* mem_set(void* pdst, int value, size_t count)
{
  char* pbdst = (char*)pdst;
  for (size_t i = 0; i < count; i++)
    pbdst[i] = 0;

  return pdst;
}

void* mem_copy(void* pdst, const void* psrc, size_t count)
{
  char* pcdst = (char*)pdst;
  char* pcsrc = (char*)psrc;
  if (count == 0 || pdst == psrc)
    return pdst;

  for (size_t i = 0; i < count; i++)
    pcdst[i] = pcsrc[i];

  return pdst;
}

void* mem_move(void* pdst, const void* psrc, size_t count)
{
  size_t i;
  if (count == 0 || pdst == psrc)
    return pdst;

  char* pcdst = (char*)pdst;
  char* pcsrc = (char*)psrc;
  if (pcdst < pcsrc) {
    for (i = 0; i < count; i++) {
      pcdst[i] = pcsrc[i];
    }
  }
  else {
    for (i = count; i > 0; i--) {
      pcdst[i-1] = pcsrc[i-1];
    }
  }
  return pdst;
}

int mem_compare(const void* psrca, const void* psrcb, size_t count)
{
  char* pbsrca = (char*)psrca;
  char* pbsrcb = (char*)psrcb;
  while (count--) {
    if (*pbsrca != *pbsrcb) {
      return *pbsrca - *pbsrcb;
    }
    pbsrca++, pbsrcb++;
  }
  return 0;
}

bool string_copy_from(dg_string_t* pdst, const char* pstring)
{
  pdst->pstring = str_duplicate(&pdst->length, pstring);
  return pdst->pstring != NULL;
}

bool string_copy(dg_string_t* pdst, const dg_string_t* psrc)
{
  pdst->length = psrc->length;
  pdst->pstring = str_duplicate(NULL, psrc->pstring);
  return pdst->pstring != NULL;
}

bool string_append(
  dg_string_t* pdst, 
  const char* pstring, 
  size_t length)
{



  return false;
}

bool string_insert_from(dg_string_t* pdst, 
  size_t from, 
  const char* psrc)
{
  return false;
}

bool string_remove_chars(dg_string_t* pdst, int chr)
{
  size_t nremoved = str_remove_char(pdst->pstring, chr);
  pdst->length -= nremoved;
  return nremoved != 0;
}

void string_free(dg_string_t* pdst)
{
  pdst->length = 0;
  if (pdst->pstring) {
    free(pdst->pstring);
    pdst->pstring = NULL;
  }
}
