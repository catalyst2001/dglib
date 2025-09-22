/*===============================
 "Diggy Gods" Team  Copyright (c) 2025.
 Abstract: File system abstraction layer
 Purpose: Provides basic functionality to the file system.
  Allows reading data from a file or buffer, with the ability to write.
 Author: Deryabin K.  Obidin N.
===============================*/
#pragma once
#include "dg_libcommon.h"
#include "dg_path.h"
#include "dg_bswap.h"
#include "dg_handle.h"

struct dg_io_handle_data_s; /* forward declaration */

#define DGFS_MAX_PATH 260 /*< max path length */
#define DGFS_MAX_LONG_PATH 4096 /*< max long path length */

#define DGFS_RECURSIVE (1<<0) /*< recursive flag for mkdir */

/**
* @brief I/O device type structure
*/
typedef struct dg_io_dt_s {
	int      (*pio_init)(struct dg_io_handle_data_s* phd);
	int      (*pio_deinit)(struct dg_io_handle_data_s* phd);
	int      (*pio_seek)(struct dg_io_handle_data_s* phd, int64_t pos, int32_t mode);
	uint64_t(*pio_tell)(struct dg_io_handle_data_s* phd);
	int      (*pio_write)(struct dg_io_handle_data_s* phd, const void* psrc, size_t srclen, size_t* pnbyteswrite);
	int      (*pio_read)(struct dg_io_handle_data_s* phd, void* pdst, size_t dstlen, size_t* pnbytesread);

	/* fields reading with byteswap */
	int      (*pio_read_s16)(struct dg_io_handle_data_s* phd, int32_t* pdst, int endian);
	int      (*pio_read_u16)(struct dg_io_handle_data_s* phd, uint32_t* pdst, int endian);
	int      (*pio_read_s32)(struct dg_io_handle_data_s* phd, int32_t* pdst, int endian);
	int      (*pio_read_u32)(struct dg_io_handle_data_s* phd, uint32_t* pdst, int endian);
	int      (*pio_read_s64)(struct dg_io_handle_data_s* phd, int64_t* pdst, int endian);
	int      (*pio_read_u64)(struct dg_io_handle_data_s* phd, uint64_t* pdst, int endian);

	/* fields writing with byteswap */
	int      (*pio_write_s16)(struct dg_io_handle_data_s* phd, int32_t src, int endian);
	int      (*pio_write_u16)(struct dg_io_handle_data_s* phd, uint32_t src, int endian);
	int      (*pio_write_s32)(struct dg_io_handle_data_s* phd, int32_t src, int endian);
	int      (*pio_write_u32)(struct dg_io_handle_data_s* phd, uint32_t src, int endian);
	int      (*pio_write_s64)(struct dg_io_handle_data_s* phd, int64_t src, int endian);
	int      (*pio_write_u64)(struct dg_io_handle_data_s* phd, uint64_t src, int endian);
} dg_io_dt_t;

/**
* @brief I/O handle data structure
*/
typedef struct dg_io_handle_data_s {
	uint32_t    mode; /*< FS_MODE describes it */
	uint32_t    access; /*< FS_ACCESS describes it */
	size_t      pos; /*< current position */
	size_t      size; /*< total size */
	void* pobject; /*< pointer to object (FILE*, buffer, etc) */
	dg_io_dt_t* piodt; /*< pointer to I/O device type DT structure */
	//uint8_t     data[];
} dg_io_handle_data_t;

typedef dg_handle_t dg_file_t; /*< file handle type */
typedef dg_handle_t dg_pathid_t; /*< path id handle type */

/**
* file open access
*/
enum FS_ACCESS {
	FS_R = 1 << 0, /*< read */
	FS_W = 1 << 1, /*< write */
	FS_X = 1 << 2, /*< exec */
	FS_TRUNC = 1 << 3 /*< truncate file */
};

/**
* file seek mode
*/
enum FS_SEEKM {
	FS_CUR = 0, /*< seek relative to current position */
	FS_SET, /*< seek relative to beginning of file */
	FS_END /*< seek relative to end of file */
};

/**
* file open mode
*/
enum FS_MODE {
	FS_BIN = 0, /*< binary mode */
	FS_TXT, /*< text mode */
	FS_DEVICE /*< device mode */
};

/**
* @brief add new search path to filesystem
* @param ppathid - new path string identifier
* @param ppath - basic path part
* @param override - boolean value for enable override search path if it already exists
*
* @return 0 if search path added
* @return	1 if search path already exists
* @return -1 if no available memory space for store new search path
*/
DG_API int fs_add_search_path(const char* ppathid, const wchar_t* ppath, int override);

/**
* @brief get search path handle by name
* @param ppathid - search path string identifier
*
* @return if exists, returns correct handle value of existing search path
* @return if not found, DG_INVALID_HANDLE will be returned
*/
DG_API dg_pathid_t fs_get_path_id(const char* ppathid);

/**
* @brief get path copy by valid pathid handle
* @param pdst - address of dg_path_t struct for store copy of search path
* @param pathid - valid handle of search path
*
* @return true - if pathid handle is valid, otherwise return false
*/
DG_API bool fs_get_path_by_id(dg_path_t* pdst, dg_pathid_t pathid);

/**
* @brief creates and opens a file or device
* @param pdst - address of file_t var for store opened file handle
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @param mode - open file mode (described by FS_MODE enumeration)
* @param accs - file open access (described by FS_ACCESS enumeration)
*
* @return 0 - operation succesfully completed
* @return 1 - access violation
*/
DG_API int fs_open_file(dg_file_t* pdst,
	dg_pathid_t pathid,
	const wchar_t* ppath,
	uint32_t mode,
	uint32_t accs);

/**
* @brief open memory buffer as file
*
* @param pdst - address of file_t var for store opened file handle
* @param pbuffer - start address of buffer for store or read data
* @param size - size of buffer
* @param mode - open file mode (described by FS_MODE enumeration)
* @param accs - file open access (described by FS_ACCESS enumeration)
*
* @return 0 - operation succesfully completed
* @return 1 - invalid parameter
*/
DG_API int fs_open_buffer(dg_file_t* pdst,
	void* pbuffer,
	size_t size,
	uint32_t mode,
	uint32_t accs);

/**
* @brief get information by file handle
* @param pdst - address of fs_finfo_t structure instance for filling by information
* @param hfile - opened file handle
*
* @return 0 - success
* @return 1 - invalid parameter
* @return 2 - invalid handle
*/
typedef struct fs_finfo_s {
	uint32_t mode;
	uint32_t access;
} fs_finfo_t;

DG_API int fs_finfo(fs_finfo_t* pdst, dg_file_t hfile);

/**
* @brief changes file pointer
* @param hfile - opened file handle
* @param pos - relative or absolute position (dependent of mode)
* @param mode - seek mode (described by FS_SEEKM enumeration)
*
* @return 0 - success
* @return 1 - invalid parameter
* @return 2 - invalid handle
*/
DG_API int fs_seek(dg_file_t hfile, int64_t pos, int32_t mode);

/**
* @brief returns current file pointer
* @param hfile - file handle
*
* @return absolute file position
*/
DG_API int64_t fs_tell(dg_file_t hfile);

/**
* @brief closes file or device
* @param hfile - file handle
* @return nothing
*/
DG_API void fs_close(dg_file_t hfile);

enum FS_ENDIAN {
	FS_LE = 0, /*< little endian */
	FS_BE /* big endian */
};

/**
* @brief write data to file
* @param hfile - file handle
* @param psrc - pointer to source data for writing
* @param srclen - num bytes to write
* @param pnbyteswrite - pointer to size_t var for store num writed bytes (may be NULL if not needed)
*
* @return 0 - success
* @return 1 - invalid parameter
* @return 2 - invalid file handle
*/
DG_API int fs_write(dg_file_t hfile, const void* psrc, size_t srclen, size_t* pnbyteswrite);
DG_API int fs_read(void* pdst, size_t dstlen, dg_file_t hfile, size_t* pnbytesread);
//static inline int fs_write_char(file_t hfile, int c) { return fs_write_s8(hfile, c); }
//static inline int fs_read_char(int* pdstc, file_t hfile) { return fs_read_s8(pdstc, hfile); }
DG_API int fs_write_string(dg_file_t hfile, const char* psrc, size_t length);
DG_API const char* fs_read_string(char* pdst, size_t maxlen, size_t length, dg_file_t hfile);

/*
===============================
	Automatic byteswapped reading/writing
===============================
*/

/**
* @brief read/write functions with automatic byteswap
* @param pdst/src - pointer to destination/source variable
* @param hfile - file handle
* @param endian - FS_ENDIAN enumeration value
* @return 0 - success
*/
DG_API int fs_read_s16(int32_t* pdst, dg_file_t hfile, int endian);
DG_API int fs_read_u16(uint32_t* pdst, dg_file_t hfile, int endian);
DG_API int fs_read_s32(int32_t* pdst, dg_file_t hfile, int endian);
DG_API int fs_read_u32(uint32_t* pdst, dg_file_t hfile, int endian);
DG_API int fs_read_s64(int64_t* pdst, dg_file_t hfile, int endian);
DG_API int fs_read_u64(uint64_t* pdst, dg_file_t hfile, int endian);

#define fs_read_s16_LE(pdst, hfile) fs_read_s16(pdst, hfile, FS_LE)
#define fs_read_s16_BE(pdst, hfile) fs_read_s16(pdst, hfile, FS_BE)
#define fs_read_u16_LE(pdst, hfile) fs_read_u16(pdst, hfile, FS_LE)
#define fs_read_u16_BE(pdst, hfile) fs_read_u16(pdst, hfile, FS_BE)

#define fs_read_s32_LE(pdst, hfile) fs_read_s32(pdst, hfile, FS_LE)
#define fs_read_s32_BE(pdst, hfile) fs_read_s32(pdst, hfile, FS_BE)
#define fs_read_u32_LE(pdst, hfile) fs_read_u32(pdst, hfile, FS_LE)
#define fs_read_u32_BE(pdst, hfile) fs_read_u32(pdst, hfile, FS_BE)

#define fs_read_s64_LE(pdst, hfile) fs_read_s64(pdst, hfile, FS_LE)
#define fs_read_s64_BE(pdst, hfile) fs_read_s64(pdst, hfile, FS_BE)
#define fs_read_u64_LE(pdst, hfile) fs_read_u64(pdst, hfile, FS_LE)
#define fs_read_u64_BE(pdst, hfile) fs_read_u64(pdst, hfile, FS_BE)

/**
* @brief read/write functions with automatic byteswap
* @param src - source variable
* @param hfile - file handle
* @param endian - FS_ENDIAN enumeration value
* @return 0 - success
*/
DG_API int fs_write_s16(dg_file_t hfile, int32_t src, int endian);
DG_API int fs_write_u16(dg_file_t hfile, uint32_t src, int endian);
DG_API int fs_write_s32(dg_file_t hfile, int32_t src, int endian);
DG_API int fs_write_u32(dg_file_t hfile, uint32_t src, int endian);
DG_API int fs_write_s64(dg_file_t hfile, int64_t src, int endian);
DG_API int fs_write_u64(dg_file_t hfile, uint64_t src, int endian);

#define fs_write_s16_LE(src, hfile) fs_write_s16(hfile, src, FS_LE)
#define fs_write_s16_BE(src, hfile) fs_write_s16(hfile, src, FS_BE)
#define fs_write_u16_LE(src, hfile) fs_write_u16(hfile, src, FS_LE)
#define fs_write_u16_BE(src, hfile) fs_write_u16(hfile, src, FS_BE)

#define fs_write_s32_LE(src, hfile) fs_write_s32(hfile, src, FS_LE)
#define fs_write_s32_BE(src, hfile) fs_write_s32(hfile, src, FS_BE)
#define fs_write_u32_LE(src, hfile) fs_write_u32(hfile, src, FS_LE)
#define fs_write_u32_BE(src, hfile) fs_write_u32(hfile, src, FS_BE)

#define fs_write_s64_LE(src, hfile) fs_write_s64(hfile, src, FS_LE)
#define fs_write_s64_BE(src, hfile) fs_write_s64(hfile, src, FS_BE)
#define fs_write_u64_LE(src, hfile) fs_write_u64(hfile, src, FS_LE)
#define fs_write_u64_BE(src, hfile) fs_write_u64(hfile, src, FS_BE)

/**
* @brief load file to memory buffer or save memory buffer to file
* @param pdst - address of size_t var for store size of allocated buffer
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param pfilename - file name relative pathid
* @return pointer to allocated memory buffer with file data, or NULL if failed
* 
*	@note returned buffer must be freed by user with ma_free()
*/
DG_API void* fs_ftomem(size_t* pdst, dg_pathid_t pathid, const char* pfilename);

/**
* @brief save memory buffer to file
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param pfilename - file name relative pathid
* @param psrc - pointer to source memory buffer
* @param length - size of source memory buffer
* @return 0 - success
*/
DG_API int   fs_memtof(dg_pathid_t pathid, const char* pfilename, const void* psrc, size_t length);

typedef dg_voidptr_t hdir_t; /*< directory handle type */
typedef uint64_t dg_fstime_t; /*< file time type in 100-nanosecond intervals since January 1, 1601 (UTC) */

enum FS_FILEATTRIB {
	DGFS_ATTRIB_NONE = 0, /*< no attribs */
	DGFS_ATTRIB_DIRECTORY = 1 << 0, /*< directory */
	DGFS_ATTRIB_DEVICE = 1 << 1, /*< device */
	DGFS_ATTRIB_HIDDEN = 1 << 2 /*< hidden */
};

typedef struct dg_dirent_s {
	uint32_t    faccess; /*< FS_ACCESS describes it */
	uint32_t    fattribs; /*< FS_FILEATTRIB describes it */
	dg_wchar_t  filename[DGFS_MAX_PATH]; /*< directory or filename */
	dg_fstime_t creation; /*< creation time */
	dg_fstime_t last_access; /*< last access time */
	dg_fstime_t last_change; /*< last change time */
} dg_dirent_t;

/**
* @brief directory iteration functions
* @param pdst - address of hdir_t var for store opened directory handle
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @return 0 - operation succesfully completed
*/
DG_API int fs_dir_open(hdir_t* pdst, dg_pathid_t pathid, const dg_wchar_t* ppath);

/**
* @brief read next directory entry
* @param pdst - address of dg_dirent_t structure instance for filling by information
* @param hdir - opened directory handle
* @return 0 - success
*/
DG_API int fs_dir_read(dg_dirent_t* pdst, hdir_t hdir);

/**
* @brief closes directory handle
* @param phdir - address of hdir_t var for store opened directory handle
* @return 0 - success
*/
DG_API int fs_dir_close(hdir_t* phdir);

/**
* @brief file status structure
*/
typedef struct dg_stat_s {
	uint32_t    faccess; /*< FS_ACCESS describes it */
	uint32_t    fattribs; /*< FS_FILEATTRIB describes it */
	dg_fstime_t creation; /*< creation time */
	dg_fstime_t last_access; /*< last access time */
	dg_fstime_t last_change; /*< last change time */
	uint64_t    filesize; /*< file size */
} dg_stat_t;

/**
* @brief build full path string from pathid and relative path
* @param pdst - pointer to destination buffer for store full path string
* @param dstlen - size of destination buffer in characters
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @return pointer to destination buffer (pdst)
*/
DG_API dg_wchar_t* fs_build_path(dg_wchar_t *pdst, size_t dstlen, 
	dg_pathid_t pathid, 
	const dg_wchar_t *ppath);

/**
* @brief file system functions
* @param pdst - address of dg_stat_t structure instance for filling by information
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @return 0 - success
*/
DG_API int fs_stat(dg_stat_t* pdst, dg_pathid_t pathid, const dg_wchar_t* ppath);

/**
* @brief check file existence
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @param chk - FS_ACCESS flag for check access
* @return 0 - if file exists and access is allowed
*/
DG_API int fs_access(dg_pathid_t pathid, const dg_wchar_t* ppath, int chk);

/**
* @brief create directory, touch file or remove file
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param flags - DGFS_RECURSIVE flag for mkdir, 0 for touch and remove
* @param ppath - path relative of pathid
* @return 0 - success
*/
DG_API int fs_mkdir(dg_pathid_t pathid, int flags, const dg_wchar_t* ppath);

/**
*	@brief create empty file or remove file
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @return 0 - success
*/
DG_API int fs_touch(dg_pathid_t pathid, const dg_wchar_t* ppath);

/**
* @brief remove file or empty directory
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param ppath - path relative of pathid
* @return 0 - success
*/
DG_API int fs_remove(dg_pathid_t pathid, const dg_wchar_t* ppath);

typedef dg_handle_t dg_hstor_t; /*< storage handle type */

/**
* @brief simple storage functions
* @param pdst - address of dg_hstor_t var for store opened storage handle
* @param pathid - valid path id handle or DG_INVALID_HANDLE, if needed path relative executable path
* @param pstrorname - storage name (file name)
* @return 0 - operation succesfully completed
*/
DG_API int fs_stor_open(dg_hstor_t* pdst, 
	dg_pathid_t pathid, 
	const char* pstrorname);

/**
* @brief set value in storage by name
* @param hstor - opened storage handle
* @param pvalname - value name in storage
* @param psrc - pointer to source data for writing
* @return 0 - success
*/
DG_API int fs_stor_set_str(dg_hstor_t hstor,
	const char* pvalname,
	const char* psrc);

/**
* @brief set binary value in storage by name
* @param hstor - opened storage handle
* @param pvalname - value name in storage
* @param psrc - pointer to source data for writing
* @param length - size of source data
* @return 0 - success
*/
DG_API int fs_stor_set_bin(dg_hstor_t hstor,
	const char* pvalname,
	const void* psrc,
	size_t length);

/**
* @brief get string value from storage by name
* @param pdst - pointer to destination buffer for store string value
* @param dstlen - size of destination buffer in characters
* @param outSize - address of size_t var for store real size of string value (may be NULL if not needed)
* @param hstor - opened storage handle
* @param pvalname - value name in storage
* @return 0 - success
*/
DG_API int fs_stor_get(void* pdst, 
	size_t dstlen, 
	size_t* outSize, 
	dg_hstor_t hstor,
	const char* pvalname);

/**
* @brief delete value from storage by name
* @param hstor - opened storage handle
* @param pvalname - value name in storage
* @return 0 - success
*/
DG_API int fs_stor_delete(dg_hstor_t hstor,
	const char* pvalname);

/**
* @brief close storage handle
* @param hstor - opened storage handle
* @return 0 - success
*/
DG_API int fs_stor_close(dg_hstor_t hstor);
