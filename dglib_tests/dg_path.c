#include "dg_path.h"

void path_init_static(dg_path_t* pdst, const dg_wchar_t* path)
{
  if (!pdst)
    return;

  pdst->pstring = (dg_wchar_t*)path;
  pdst->size = path ? wcs_length(path) : 0;
}

int path_alloc_empty(dg_path_t* pdst, size_t maxlen)
{
  if (!pdst) return -1;
  dg_wchar_t* str = (dg_wchar_t*)malloc((maxlen + 1) * sizeof(dg_wchar_t));
  if (!str) return -1;
  str[0] = 0;
  pdst->pstring = str;
  pdst->size = 0;
  return 0;
}

int path_copy_string(dg_path_t* pdst, const dg_wchar_t* psrc)
{
  if (!pdst) 
    return -1;

  // Clean up existing
  if (pdst->pstring) {
    free(pdst->pstring);
    pdst->pstring = NULL;
    pdst->size = 0;
  }

  if (!psrc)
    return 0;
  
  size_t len = wcs_length(psrc);
  dg_wchar_t* str = (dg_wchar_t*)malloc((len + 1) * sizeof(dg_wchar_t));
  if (!str)
    return -1;

  for (size_t i = 0; i <= len; ++i)
    str[i] = psrc[i];
  
  pdst->pstring = str;
  pdst->size = len;
  return 0;
}

int path_copy(dg_path_t* pdst, const dg_path_t* psrc)
{
  size_t length;
  /* empty source? */
  if (!psrc->pstring) {
    pdst->size = 0;
    pdst->pstring = NULL;
    return 1;
  }

  length = wcs_length(psrc->pstring);
  if (!length)
    return 1;
  
  length++; //copy with '\0'
  pdst->pstring = (dg_wchar_t*)malloc(length*sizeof(dg_wchar_t));
  if (!pdst->pstring)
    return -1;

  mem_copy(pdst->pstring, psrc->pstring, length);
  return 0; /* OK */
}

int path_alloc_copy(dg_path_t* pdst, const dg_wchar_t* path)
{
  return path_copy_string(pdst, path);
}

void path_free(dg_path_t* pdst)
{
  if (!pdst)
    return;

  if (pdst->pstring) {
    free(pdst->pstring);
    pdst->pstring = NULL;
  }
  pdst->size = 0;
}

int path_append(dg_path_t* pdst, const dg_path_t* path)
{
  if (!pdst || !path || !path->pstring)
    return -1;

  size_t base_len = pdst->size;
  size_t add_len = path->size;
  bool need_sep = false;
  if (base_len > 0 && add_len > 0) {
    dg_wchar_t last = pdst->pstring[base_len - 1];
    dg_wchar_t first = path->pstring[0];
    if (last != (dg_wchar_t)'/' && last != (dg_wchar_t)'\\' &&
      first != (dg_wchar_t)'/' && first != (dg_wchar_t)'\\') {
      need_sep = true;
    }
  }
  size_t new_len = base_len + (need_sep ? 1 : 0) + add_len;
  dg_wchar_t* new_str = (dg_wchar_t*)malloc((new_len + 1) * sizeof(dg_wchar_t));
  if (!new_str)
    return -1;

  // Copy base
  if (base_len > 0 && pdst->pstring)
    memcpy(new_str, pdst->pstring, base_len * sizeof(dg_wchar_t));
  
  // Insert separator if needed
  size_t offset = base_len;
  if (need_sep)
    new_str[offset++] = (dg_wchar_t)'/';
  
  // Copy appended path
  if (add_len > 0) {
    memcpy(new_str + offset, path->pstring, add_len * sizeof(dg_wchar_t));
    offset += add_len;
  }

  new_str[offset] = 0;

  // Replace old
  if (pdst->pstring)
    free(pdst->pstring);

  pdst->pstring = new_str;
  pdst->size = new_len;
  return 0;
}

int path_get_base(dg_path_t* pdst_basepath, const dg_path_t* psrc) {
  if (!pdst_basepath || !psrc || !psrc->pstring)
    return -1;

  const dg_wchar_t* str = psrc->pstring;
  size_t len = psrc->size;

  // Trim trailing slashes
  size_t end = len;
  while (end > 0 && (str[end - 1] == (dg_wchar_t)'/' || str[end - 1] == (dg_wchar_t)'\\')) {
    --end;
  }
  // Find last separator
  size_t pos = end;
  while (pos > 0 && str[pos - 1] != (dg_wchar_t)'/' && str[pos - 1] != (dg_wchar_t)'\\') {
    --pos;
  }
  // pos is index of separator or 0
  if (pos == 0) {
    // No base, return empty
    return path_alloc_empty(pdst_basepath, 0);
  }
  // Copy [0, pos)
  dg_wchar_t* buf = (dg_wchar_t*)malloc((pos + 1) * sizeof(dg_wchar_t));
  if (!buf)
    return -1;

  memcpy(buf, str, pos * sizeof(dg_wchar_t));
  buf[pos] = 0;

  // Free existing and assign
  if (pdst_basepath->pstring)
    free(pdst_basepath->pstring);

  pdst_basepath->pstring = buf;
  pdst_basepath->size = pos;
  return 0;
}

int path_get_filename(dg_path_t* pdst_filename, const dg_path_t* psrc) {
  if (!pdst_filename || !psrc || !psrc->pstring)
    return -1;

  const dg_wchar_t* str = psrc->pstring;
  size_t len = psrc->size;
  // Trim trailing slashes
  size_t end = len;
  while (end > 0 && (str[end - 1] == (dg_wchar_t)'/' || str[end - 1] == (dg_wchar_t)'\\')) {
    --end;
  }
  // Find start of filename
  size_t pos = end;
  while (pos > 0 && str[pos - 1] != (dg_wchar_t)'/' && str[pos - 1] != (dg_wchar_t)'\\') {
    --pos;
  }
  size_t fname_len = end - pos;
  dg_wchar_t* buf = (dg_wchar_t*)malloc((fname_len + 1) * sizeof(dg_wchar_t));
  if (!buf)
    return -1;

  memcpy(buf, str + pos, fname_len * sizeof(dg_wchar_t));
  buf[fname_len] = 0;

  if (pdst_filename->pstring)
    free(pdst_filename->pstring);

  pdst_filename->pstring = buf;
  pdst_filename->size = fname_len;
  return 0;
}

void path_fix_slashes(dg_path_t* ppath) {
  if (!ppath || !ppath->pstring)
    return;

  for (size_t i = 0; i < ppath->size; ++i) {
    if (ppath->pstring[i] == (dg_wchar_t)'\\') {
      ppath->pstring[i] = (dg_wchar_t)'/';
    }
  }
}

bool path_back_folder(dg_path_t* ppath) {
  if (!ppath || !ppath->pstring)
    return false;

  dg_wchar_t* str = ppath->pstring;
  size_t len = ppath->size;
  // Remove trailing slashes
  size_t end = len;
  while (end > 0 && (str[end - 1] == (dg_wchar_t)'/' || str[end - 1] == (dg_wchar_t)'\\')) {
    --end;
  }
  if (end == 0) {
    // Nothing to back up
    return false;
  }
  // Find previous slash
  size_t pos = end;
  while (pos > 0 && str[pos - 1] != (dg_wchar_t)'/' && str[pos - 1] != (dg_wchar_t)'\\') {
    --pos;
  }
  if (pos == 0) {
    // Root reached, clear
    str[0] = 0;
    ppath->size = 0;
    return false;
  }
  // Truncate at pos
  str[pos] = 0;
  ppath->size = pos;
  return true;
}