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

char* str_duplicate_string(size_t* pdstlen, const char* psrc)
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

bool string_copy_from(dg_string_t* pdst, const char* pstring)
{
  pdst->pstring = str_duplicate_string(&pdst->length, pstring);
  return pdst->pstring != NULL;
}

bool string_copy(dg_string_t* pdst, const dg_string_t* psrc)
{
  pdst->length = psrc->length;
  pdst->pstring = str_duplicate_string(NULL, psrc->pstring);
  return pdst->pstring != NULL;
}

void string_free(dg_string_t* pdst)
{
  pdst->length = 0;
  if (pdst->pstring) {
    free(pdst->pstring);
    pdst->pstring = NULL;
  }
}
