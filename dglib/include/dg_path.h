#ifndef __dg_path_h__
#define __dg_path_h__
#include "dg_libcommon.h"
#include "dg_string.h"

/**
* @brief Path structure
*/
typedef struct dg_path_s {
	size_t      size;  /*< size in elements */
	dg_wchar_t* pstring;  /*< pointer to the string */
} dg_path_t;

#ifdef _MSC_VER
#define DG_PATH_STRING(x) (L##x)
#else
#define DG_PATH_STRING(x)
#endif
#define DG_PATH_INIT_STATIC(x) { sizeof(x)/sizeof(dg_wchar_t)-1, (dg_wchar_t*)x }

/**
* @brief Initialize a static path structure
* @param pdst - Pointer to the path structure to initialize
* @param path - Static path string
* @note The path string must be a static string (e.g., a string literal)
*/
DG_API void path_init_static(dg_path_t *pdst, const dg_wchar_t *path);

/**
* @brief Allocate an empty path with a specified maximum length
* @param pdst - Pointer to the path structure to allocate
* @param maxlen - Maximum length of the path in characters
* @return 0 on success, -1 on failure
* 
* @note The allocated path will be empty (zero-length string)
*/
DG_API int  path_alloc_empty(dg_path_t* pdst, size_t maxlen);

/**
* @brief Copy a wide-character string to a path structure
* @param pdst - Pointer to the destination path structure
* @param psrc - Pointer to the source wide-character string
* @return 0 on success, -1 on failure
* 
* @note The source string is expected to be null-terminated
*/
DG_API int  path_copy_string(dg_path_t* pdst, const dg_wchar_t *psrc);

/**
* @brief Copy one path structure to another
* @param pdst - Pointer to the destination path structure
* @param psrc - Pointer to the source path structure
* @return 0 on success, -1 on failure
* @note The destination path will be allocated automatically
*/
DG_API int  path_copy(dg_path_t* pdst, const dg_path_t *psrc);

/**
* @brief Copy a wide-character string to a path structure, allocating memory
* @param pdst - Pointer to the destination path structure
* @param path - Pointer to the source wide-character string
* @return 0 on success, -1 on failure
* 
* @note The source string is expected to be null-terminated
*/
DG_API int  path_alloc_copy(dg_path_t* pdst, const dg_wchar_t* path);

/**
* @brief Free the memory allocated for a path structure
* @param pdst - Pointer to the path structure to free
* @note After calling this function, the path structure will be empty
*/
DG_API void path_free(dg_path_t* pdst);

/**
* @brief Append one path to another
*	@param pdst - Pointer to the destination path structure
* @param path - Pointer to the source path structure to append
* @return 0 on success, -1 on failure
* @note The destination path will be reallocated if necessary
*/
DG_API int  path_append(dg_path_t* pdst, const dg_path_t* path);

/**
* @brief Get the base path (directory) from a full path
* @param pdst_basepath - Pointer to the destination path structure for the base path
* @param psrc - Pointer to the source full path structure
* @return 0 on success, -1 on failure
* 
* @note The base path will be allocated automatically
*/
DG_API int  path_get_base(dg_path_t *pdst_basepath, const dg_path_t *psrc);

/**
* @brief Get the filename from a full path
* @param pdst_filename - Pointer to the destination path structure for the filename
* @param psrc - Pointer to the source full path structure
* @return 0 on success, -1 on failure
*/
DG_API int  path_get_filename(dg_path_t *pdst_filename, const dg_path_t *psrc);

/**
* @brief Fix slashes in the path to use the system's preferred slash
* @param ppath - Pointer to the path structure to fix
* @note This function modifies the path in place
*/
DG_API void path_fix_slashes(dg_path_t *ppath);

/**
* @brief Remove the file extension from a path
* @param ppath - Pointer to the path structure to modify
* @return true if the extension was removed, false if there was no extension
*/
DG_API bool path_back_folder(dg_path_t* ppath);

/**
* @brief Remove the file extension from a path string
* @param ppath - Pointer to the wide-character string representing the path
* @return Pointer to the modified string (same as input)
* @note This function modifies the string in place
*/
DG_API char* path_remove_file_ext(char* ppath);

/**
* @brief get the string from a path
* @param ppath - Pointer to the path structure
* @return Pointer to the wide-character string
* @note The returned string may be NULL if the path is not allocated
*/
static inline dg_wchar_t* path_get_string(const dg_path_t* ppath) { return ppath->pstring; }

/**
* @brief check if the path is empty
* @param ppath - Pointer to the path structure
* @return true if the path is empty, false otherwise
*/
static inline bool path_is_empty(const dg_path_t* ppath) { return !ppath->pstring || !ppath->pstring[0]; }

/**
* @brief get the length of the path
* @param ppath - Pointer to the path structure
* @return Length of the path in characters
*/
static inline size_t path_get_length(const dg_path_t* ppath) { return ppath->size; }

#endif // __dg_path_h__