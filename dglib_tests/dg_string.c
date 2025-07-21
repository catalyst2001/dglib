#include "dg_string.h"
#include "dg_alloc.h"

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

char* str_contains(const char* pstr, const char* pfrag)
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
  const char* pdelim)
{
  size_t isplit = 0;
  size_t delim_len = str_length(pdelim);
  size_t srclen = str_length(psrc);
  char* ptr = psrc;
  char* pend = ptr + srclen;
  do {
    ptr = str_contains(ptr, pdelim);
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

bool string_insert_from(dg_string_t* pdst, size_t from, const char* psrc)
{
  return false;
}

bool string_remove_chars(dg_string_t* pdst, int chr)
{
  size_t nremoved= str_remove_char(pdst->pstring, chr);
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
