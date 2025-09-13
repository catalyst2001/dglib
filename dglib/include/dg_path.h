#pragma once
#include "dg_libcommon.h"
#include "dg_string.h"

typedef struct dg_path_s {
	size_t      size;
	dg_wchar_t* pstring;
} dg_path_t;

#ifdef _MSC_VER
#define DG_PATH_STRING(x) (L##x)
#else
#define DG_PATH_STRING(x)
#endif

DG_API void path_init_static(dg_path_t *pdst, const dg_wchar_t *path);
DG_API int  path_alloc_empty(dg_path_t* pdst, size_t maxlen);
DG_API int  path_copy_string(dg_path_t* pdst, const dg_wchar_t *psrc);
DG_API int  path_copy(dg_path_t* pdst, const dg_path_t *psrc);
DG_API int  path_alloc_copy(dg_path_t* pdst, const dg_wchar_t* path);
DG_API void path_free(dg_path_t* pdst);
DG_API int  path_append(dg_path_t* pdst, const dg_path_t* path);
DG_API int  path_get_base(dg_path_t *pdst_basepath, const dg_path_t *psrc);
DG_API int  path_get_filename(dg_path_t *pdst_filename, const dg_path_t *psrc);
DG_API void path_fix_slashes(dg_path_t *ppath);
DG_API bool path_back_folder(dg_path_t* ppath);

DG_API char* path_remove_file_ext(char* ppath);

static inline dg_wchar_t* path_get_string(const dg_path_t* ppath) { return ppath->pstring; }
static inline bool path_is_empty(const dg_path_t* ppath) { return !ppath->pstring || !ppath->pstring[0]; }
static inline size_t path_get_length(const dg_path_t* ppath) { return ppath->size; }
