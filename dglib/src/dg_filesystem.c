#include "dg_filesystem.h"
#include "dg_sync.h"
#include "dg_string.h"
#include "dg_alloc.h"
#include <wchar.h>

typedef struct searchpathinf_s {
  char      id[16];
  dg_path_t path;
} searchpathinf_t;

#define MAX_SEARCH_PATHES 16
#define MAX_FILE_HANDLES  1024
#define MAX_STOR_HANDLES  32

HA_DECL_DATA_ARRAYS(pathes_data, MAX_SEARCH_PATHES, sizeof(searchpathinf_t));
HA_DECL_DATA_ARRAYS(file_handles, MAX_FILE_HANDLES, sizeof(dg_io_handle_data_t));
HA_DECL_DATA_ARRAYS(storage_handles, MAX_STOR_HANDLES, sizeof(void*));

dg_mutex_t        glob_sp_mtx = NULL;
dg_handle_alloc_t glob_search_pathes;
dg_handle_alloc_t glob_file_handles;
dg_handle_alloc_t glob_storage_handles;

dg_io_dt_t glob_file_iodt = {
  .pio_init=NULL
};

/**
* @brief find search path by path id
* @note this is only for internally use
* 
* @return pointer to searchpathinf_t if path found, otherwise returns NULL
*/
searchpathinf_t* fsi_search_path_by_id(dg_handle_t *pdsth, const char *ppathid)
{
  searchpathinf_t* pspi;
  dg_handle_t handle = ha_get_first_handle(&glob_search_pathes, 0, false);
  if (handle.hvalue != DG_HANDLE_INVALID_VALUE) {
    while (ha_get_next_handle(&handle, &glob_search_pathes)) {
      pspi = (searchpathinf_t*)ha_get_handle_data(&glob_search_pathes, handle);
      if (pspi && !str_compare(pspi->id, ppathid, true)) {
        if (pdsth) {
          *pdsth = handle;
        }
        return pspi;
      }
    }
  }
  return NULL;
}

DG_API int fs_add_search_path(const char* ppathid, const wchar_t* ppath, int override)
{
  int                status=0;
  dg_halloc_result_t hallocres;
  searchpathinf_t*   pspinfo;
  mutex_lock(glob_sp_mtx);

  /* find already exists search path */
  pspinfo = fsi_search_path_by_id(NULL, ppathid);
  if (pspinfo) {
    /* override existing? */
    if (override) {
      path_copy_string(&pspinfo->path, ppath);
      status = 0; /* OK */
      goto __finalize;
    }
    /* already exists */
    status= 1;
    goto __finalize;
  }

  /* not found, create new */
  if (ha_alloc_handle(&hallocres, &glob_search_pathes)) {
    hallocres.new_handle; /* unused */
    pspinfo = (searchpathinf_t*)hallocres.phandle_body;
    str_copy(pspinfo->id, sizeof(pspinfo->id), ppathid);
    if (path_alloc_copy(&pspinfo->path, ppath) != 0) {
      DG_ERROR("fs_add_search_path(): path_alloc_copy() failed!");
      status= -1; /* no enough memory :( */
      goto __finalize;
    }
  }

__finalize:
  mutex_unlock(glob_sp_mtx);
  return status;
}

DG_API dg_pathid_t fs_get_path_id(const char* ppathid)
{
  dg_pathid_t path_id_handle=DG_INVALID_HANDLE;
  mutex_lock(glob_sp_mtx);
  fsi_search_path_by_id(&path_id_handle, ppathid);
  mutex_unlock(glob_sp_mtx);
  return path_id_handle;
}

bool fs_get_path_by_id(dg_path_t* pdst, dg_pathid_t pathid)
{
  searchpathinf_t* pspinfo=NULL;
  mutex_lock(glob_sp_mtx);
  pspinfo = ha_get_handle_data(&glob_search_pathes, pathid);
  if (pspinfo) {
    if (path_copy(pdst, &pspinfo->path) != 0) {
      DG_ERROR("fs_get_path_by_id(): failed to copy path");
      return false;
    }
  }
  mutex_unlock(glob_sp_mtx);
  return pspinfo != NULL;
}

#ifdef _WIN32
#include <Windows.h>
#else
//TODO: K.D. add here includes for linux
#endif

int fs_open_file(dg_file_t* pdst, 
  dg_pathid_t pathid, 
  const wchar_t* ppath, 
  uint32_t mode, 
  uint32_t accs)
{
  searchpathinf_t*     pspi=NULL;
  dg_io_handle_data_t* phd=NULL;
  dg_halloc_result_t   ahi; // allocated handle info
  dg_wchar_t           buf[2048];

  if (ha_alloc_handle(&ahi, &glob_file_handles)) {
    phd = (dg_io_handle_data_t *)ahi.phandle_body;
    if (!phd) {
      DG_ERROR("fs_open_file(): failed! phandle_body is NULL");
      ha_free_handle(&glob_file_handles, ahi.new_handle);
      return 1;
    }
    *pdst = ahi.new_handle;

    /* get search path */
    pspi = (searchpathinf_t*)ha_get_handle_data(&glob_search_pathes, pathid);
    if (pspi) {
#ifdef _WIN32
      swprintf(buf, sizeof(buf)/sizeof(*buf), L"%ws\\%ws", pspi->path.pstring, ppath);
#else
      //TODO: K.D. implement this for linux
#error "not implemented"
#endif
    }
    else {
      wcs_copy(buf, sizeof(buf), ppath, 0);
    }

    /* fill */
    phd->mode = mode;
    phd->access = accs;
    phd->pos = phd->size = 0;
    phd->piodt = &glob_file_iodt;
    return phd->piodt->pio_init(phd);
  }
  return 0;
}

int fs_open_buffer(dg_file_t* pdst, void* pbuffer, size_t size, uint32_t mode,
  uint32_t accs)
{
  //TODO: IMPL
  return 0;
}

#define DGFS_CALL_HANDLER(hfile, handler, ...)\
  dg_io_handle_data_t *phd = ha_get_handle_data(&glob_file_handles, hfile);\
  if (!phd)\
    return 2;\
  assert(phd->piodt && "phd->piodt was NULL");\
  assert(phd->piodt->handler && "phd->piodt->" DG_TO_STRING(handler) " was NULL");\
  return phd->piodt->handler(phd, __VA_ARGS__);

#define DGFS_CALL_HANDLER_NO_ARGS(hfile, handler)\
  dg_io_handle_data_t *phd = ha_get_handle_data(&glob_file_handles, hfile);\
  if (!phd)\
    return 2;\
  assert(phd->piodt && "phd->piodt was NULL");\
  assert(phd->piodt->handler && "phd->piodt->" DG_TO_STRING(handler) " was NULL");\
  return phd->piodt->handler(phd);

int fs_write(dg_file_t hfile, const void* psrc, size_t srclen, size_t* pnbyteswrite)
{
  DGFS_CALL_HANDLER(hfile, pio_write, psrc, srclen, pnbyteswrite)
}

int fs_read(void* pdst, size_t dstlen, dg_file_t hfile, size_t* pnbytesread)
{
  DGFS_CALL_HANDLER(hfile, pio_read, pdst, dstlen, pnbytesread)
}

int fs_write_string(dg_file_t hfile, const char* psrc, size_t length)
{
  //TODO: impl with pio_write
  return 0;
}

const char* fs_read_string(char* pdst, size_t maxlen, size_t length, dg_file_t hfile)
{
  //TODO: impl with pio_read
  return NULL;
}

int fs_read_s16(int32_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s16, pdst, endian)
}

int fs_read_u16(uint32_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u16, pdst, endian)
}

int fs_read_s32(int32_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s32, pdst, endian)
}

int fs_read_u32(uint32_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u32, pdst, endian)
}

int fs_read_s64(int64_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s64, pdst, endian)
}

int fs_read_u64(uint64_t* pdst, dg_file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u64, pdst, endian)
}

int fs_write_s16(dg_file_t hfile, int32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s16, src, endian)
}

int fs_write_u16(dg_file_t hfile, uint32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u16, src, endian)
}

int fs_write_s32(dg_file_t hfile, int32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s32, src, endian)
}

int fs_write_u32(dg_file_t hfile, uint32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u32, src, endian)
}

int fs_write_s64(dg_file_t hfile, int64_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s64, src, endian)
}

int fs_write_u64(dg_file_t hfile, uint64_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u64, src, endian)
}

void* fs_ftomem(size_t* pdst, dg_pathid_t pathid, const char* pfilename)
{
  dg_file_t fh;
  if (!pdst || !pfilename)
    return NULL;

  if (fs_open_file(&fh, pathid, (const wchar_t*)pfilename, FS_BIN, FS_R) != 0)
    return NULL;

  fs_seek(fh, 0, FS_END);
  int64_t sz = fs_tell(fh);
  fs_seek(fh, 0, FS_SET);
  if (sz <= 0) {
    fs_close(fh);
    return NULL;
  }
  void* buf = malloc((size_t)sz);
  if (!buf) {
    fs_close(fh);
    return NULL;
  }
  fs_read(buf, (size_t)sz, fh, pdst);
  fs_close(fh);
  return buf;
}

int fs_memtof(dg_pathid_t pathid, const char* pfilename, const void* psrc, size_t length)
{
  dg_file_t fh;
  if (!psrc || !pfilename)
    return 1;

  if (fs_open_file(&fh, pathid, (const wchar_t*)pfilename, FS_BIN, FS_W|FS_TRUNC) != 0)
    return 1;

  size_t wr;
  int err = fs_write(fh, psrc, length, &wr);
  fs_close(fh);
  return err;
}

int fs_finfo(fs_finfo_t* pdst, dg_file_t hfile)
{
  dg_io_handle_data_t* phd = ha_get_handle_data(&glob_file_handles, hfile);
  if (!phd)
    return 2;

  pdst->access = phd->access;
  pdst->mode = phd->mode;
  return 0;
}

int fs_seek(dg_file_t hfile, int64_t pos, int32_t mode)
{
  DGFS_CALL_HANDLER(hfile, pio_seek, pos, mode)
}

int64_t fs_tell(dg_file_t hfile)
{
  DGFS_CALL_HANDLER_NO_ARGS(hfile, pio_tell)
}

void fs_close(dg_file_t hfile)
{
  dg_io_handle_data_t* phd = ha_get_handle_data(&glob_file_handles, hfile);
  if (!phd)
    return;

  assert(phd->piodt && "phd->piodt was NULL");
  assert(phd->piodt->pio_deinit && "phd->piodt->pio_deinit was NULL");
  phd->piodt->pio_deinit(phd);
  if (!ha_free_handle(&glob_file_handles, hfile)) {
    DG_ERROR("fs_close(): ha_free_handle failed!");
  }
}


/**
* INTERNAL FILESYSTEM INITIALIZATION
*/
int initialize_filesystem()
{
  DG_LOG("initialize_filesystem(): initializing filesystem...");
  if (glob_sp_mtx) {
    DG_ERROR("initialize_filesystem(): already initialized");
    return 0;
  }

  glob_sp_mtx = mutex_alloc("dg_filesystem:searchpath");
  if (!glob_sp_mtx) {
    DG_ERROR("initialize_filesystem(): mutex allocation failed");
    return 0;
  }

  ha_init_static(&glob_search_pathes,
    sizeof(dg_path_t),
    MAX_SEARCH_PATHES,
    pathes_data.blocks,
    pathes_data.generations);

  ha_init_static(&glob_file_handles,
    sizeof(dg_io_handle_data_t),
    MAX_FILE_HANDLES,
    file_handles.blocks,
    file_handles.generations);

  ha_init_static(&glob_storage_handles,
    sizeof(dg_io_handle_data_t),
    MAX_STOR_HANDLES,
    storage_handles.blocks,
    storage_handles.generations);

  return 1;
}

/**
* INTERNAL FILESYSTEM DEINITIALIZATION
*/
void deinitialize_filesystem()
{
  DG_LOG("deinitialize_filesystem(): deinitializing filesystem...");
  if (glob_sp_mtx) {
    /* deinitialize */  
    mutex_free(glob_sp_mtx);
    glob_sp_mtx = NULL;
  }
}

#ifdef _WIN32

#define DGFS_WIN32_DIR_NONE (0)
#define DGFS_WIN32_DIR_FIRSTINFO (1 << 1)

typedef struct win32_dirdata_s {
  uint32_t         flags;
  HANDLE           h_find;
  WIN32_FIND_DATAW find_data;
} win32_dirdata_t;
#endif

int fs_dir_open(hdir_t* pdst, dg_pathid_t pathid, const dg_wchar_t* ppath)
{
#ifdef _WIN32
  if (!pdst)
    return 1;

  searchpathinf_t* pspi = (searchpathinf_t*)ha_get_handle_data(&glob_search_pathes, pathid);
  DWORD            dwerror;
  wchar_t          buf[_MAX_PATH];
  win32_dirdata_t* pdirdata = DG_NEW(win32_dirdata_t);
  if (!pdirdata) {
    DG_ERROR("win32_dirdata_t allocation failed!");
    return DGERR_OUT_OF_MEMORY;
  }

  swprintf(buf, MAX_PATH, L"%ws\\%ws\\*", !pspi ? L"." : path_get_string(&pspi->path), ppath);
  pdirdata->flags = DGFS_WIN32_DIR_FIRSTINFO;
  pdirdata->h_find = FindFirstFileW(buf, &pdirdata->find_data);
  if (pdirdata->h_find == INVALID_HANDLE_VALUE) {
    dwerror = GetLastError();
    DG_ERROR("fs_dir_open(): FindFirstFileW() returned INVALID_HANDLE_VALUE. GetLastError()=%d (0x%x)",
      dwerror, dwerror);
    return DGERR_INTERNAL_ERROR;
  }
  *pdst = pdirdata;
  return DGERR_SUCCESS;
#else
  return DGERR_UNIMPLEMENTED;
#endif
}

static inline uint64_t win32_filetime_to_uint64(FILETIME ft) {
  // FILETIME stores a 64-bit value as two 32-bit parts:
  //   dwHighDateTime = high 32 bits
  //   dwLowDateTime  = low  32 bits
  return ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

static inline uint64_t win32_filesize_to_uint64(DWORD lowpart, DWORD hightpart) {
  return ((uint64_t)hightpart << 32) | lowpart;
}

static inline uint32_t win32_attribs_to_fsattribs(DWORD dwattribs) {
  uint32_t attribs = DGFS_ATTRIB_NONE;
  if (dwattribs & FILE_ATTRIBUTE_DIRECTORY)
    attribs |= DGFS_ATTRIB_DIRECTORY;
  if (dwattribs & FILE_ATTRIBUTE_DEVICE)
    attribs |= DGFS_ATTRIB_DEVICE;
  if (dwattribs & FILE_ATTRIBUTE_HIDDEN)
    attribs |= DGFS_ATTRIB_HIDDEN;
}

static inline uint32_t win32_attribs_to_access(DWORD dwattribs) {
  uint32_t faccess = FS_R|FS_X;
  if (!(dwattribs & FILE_ATTRIBUTE_READONLY))
    faccess |= FS_W;
  return faccess;
}

int fs_dir_read(dg_dirent_t* pdst, hdir_t hdir)
{
  if (!hdir)
    return 2;

#ifdef _WIN32
  DWORD dwerror;
  win32_dirdata_t* pdirdata = (win32_dirdata_t*)hdir;
  if (!(pdirdata->flags & DGFS_WIN32_DIR_FIRSTINFO)) {
    if (!FindNextFileW(pdirdata->h_find, &pdirdata->find_data)) {
      dwerror = GetLastError();
      DG_ERROR("fs_dir_read(): FindNextFileW() failed! GetLastError()=%d (0x%x)",
        dwerror, dwerror);
      return DGERR_INTERNAL_ERROR;
    }
  }
  else {
    /* skip previous info mark */
    pdirdata->flags &= ~DGFS_WIN32_DIR_FIRSTINFO;
  }

  /* copy information to target struct */
  wcs_copy(pdst->filename, sizeof(pdst->filename), pdirdata->find_data.cFileName, 0);
  pdst->creation = win32_filetime_to_uint64(pdirdata->find_data.ftCreationTime);
  pdst->last_access = win32_filetime_to_uint64(pdirdata->find_data.ftLastAccessTime);
  pdst->last_change = win32_filetime_to_uint64(pdirdata->find_data.ftLastWriteTime);
  pdst->fattribs = win32_attribs_to_fsattribs(pdirdata->find_data.dwFileAttributes);
  pdst->faccess = win32_attribs_to_access(pdirdata->find_data.dwFileAttributes);
  return DGERR_SUCCESS;
#else
  //TODO: K.D. implement for linux
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_dir_close(hdir_t* phdir)
{
  /* check param for invalid */
  if (!phdir)
    return DGERR_INVALID_PARAM;

#ifdef _WIN32
  win32_dirdata_t* pdirdata = (win32_dirdata_t*)*phdir;
  FindClose(pdirdata->h_find);
  DG_FREE(pdirdata);
#else
  //TODO: K.D. implement this for linux
#endif
  *phdir = NULL;
  return DGERR_SUCCESS;
}

dg_wchar_t* fs_build_path(dg_wchar_t* pdst, size_t dstlen, dg_pathid_t pathid, const dg_wchar_t* ppath)
{
  searchpathinf_t* pspinfo = ha_get_handle_data(&glob_search_pathes, pathid);
  if (pspinfo) {
    swprintf(pdst, dstlen, L"%ws\\%ws", path_get_string(&pspinfo->path), ppath);
    return pdst;
  }
  return ppath;
}

int fs_stat(dg_stat_t* pdst, dg_pathid_t pathid, const dg_wchar_t* ppath)
{
#ifdef _WIN32
  DWORD      dwerror;
  dg_wchar_t fullpath[DGFS_MAX_LONG_PATH];
  WIN32_FILE_ATTRIBUTE_DATA win32_fileattribs;
  dg_wchar_t* pbuffer = fs_build_path(fullpath, DG_ARRSIZE(fullpath), pathid, ppath);
  /* query file information */
  if (!GetFileAttributesExA(pbuffer, GetFileExInfoStandard, &win32_fileattribs)) {
    dwerror = GetLastError();
    DG_ERROR("fs_stat(): GetFileAttributesExA() failed! GetLastError()=%d (0x%x)",
      dwerror, dwerror);
    return DGERR_INTERNAL_ERROR;
  }

  /* copy data */
  pdst->creation = win32_filetime_to_uint64(win32_fileattribs.ftCreationTime);
  pdst->last_access = win32_filetime_to_uint64(win32_fileattribs.ftLastAccessTime);
  pdst->last_change = win32_filetime_to_uint64(win32_fileattribs.ftLastWriteTime);
  pdst->fattribs = win32_attribs_to_fsattribs(win32_fileattribs.dwFileAttributes);
  pdst->faccess = win32_attribs_to_access(win32_fileattribs.dwFileAttributes);
  pdst->filesize = win32_filesize_to_uint64(win32_fileattribs.nFileSizeLow, win32_fileattribs.nFileSizeHigh);
  return DGERR_SUCCESS;
#else
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_access(dg_pathid_t pathid, const dg_wchar_t* ppath, int chk)
{
#ifdef _WIN32
  int        result = 0;
  DWORD      dwError;
  dg_wchar_t fullpath[MAX_PATH];
  WIN32_FILE_ATTRIBUTE_DATA fa;
  searchpathinf_t* pspinfo = ha_get_handle_data(&glob_search_pathes, pathid);
  dg_wchar_t* pbuffer = fs_build_path(fullpath, DG_ARRSIZE(fullpath), pathid, ppath);
  if (!GetFileAttributesExW(pbuffer, GetFileExInfoStandard, &fa)) {
    dwError = GetLastError();
    DG_ERROR("fs_access(): GetFileAttributesExW failed, error=%u", dwError);
    return DGERR_INTERNAL_ERROR;
  }

  if (chk & FS_R)
    result |= FS_R;

  if ((chk & FS_W) && !(fa.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    result |= FS_W;

  if (chk & FS_X)
    result |= FS_X;

  return result;
#else
  //TODO: K.D. implement for linux
  return 0;
#endif
}

int fs_mkdir(dg_pathid_t pathid, int flags, const dg_wchar_t* ppath)
{
#ifdef _WIN32
  dg_wchar_t fullpath[MAX_PATH];
  dg_wchar_t* p = fs_build_path(fullpath, DG_ARRSIZE(fullpath), pathid, ppath);

  // If directory already exists, treat as success.
  DWORD fa = GetFileAttributesW(p);
  if (fa != INVALID_FILE_ATTRIBUTES && (fa & FILE_ATTRIBUTE_DIRECTORY))
    return DGERR_SUCCESS;

  // recursive creation requested?
  if (flags & DGFS_RECURSIVE) {
    dg_wchar_t tmp[MAX_PATH];
    size_t len = wcslen(p);
    // walk through each separator and create intermediate
    for (size_t i = 0; i < len; ++i) {
      if (p[i] == L'\\' || p[i] == L'/') {
        wcsncpy(tmp, p, i);
        tmp[i] = L'\0';
        CreateDirectoryW(tmp, NULL);
      }
    }
  }

  if (!CreateDirectoryW(p, NULL)) {
    DWORD err = GetLastError();
    if (err == ERROR_PATH_NOT_FOUND)
      return DGERR_NOT_FOUND;

    DG_ERROR("fs_mkdir(): CreateDirectoryW() failed! GetLastError()=%d (0x%x)", err, err);
    return DGERR_INTERNAL_ERROR;
  }
  return DGERR_SUCCESS;
#else
  //TODO: K.D. implement for linux
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_touch(dg_pathid_t pathid, const dg_wchar_t* ppath)
{
#ifdef _WIN32
  DWORD      dwerror;
  dg_wchar_t fullpath[MAX_PATH];
  dg_wchar_t* p = fs_build_path(fullpath, DG_ARRSIZE(fullpath), pathid, ppath);

  // OPEN_ALWAYS: create if not exists, else open
  HANDLE h = CreateFileW(p,
    GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  if (h == INVALID_HANDLE_VALUE) {
    dwerror = GetLastError();
    DG_ERROR("fs_touch(): CreateFileW() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
    return DGERR_INTERNAL_ERROR;
  }

  // update access & write times to now
  FILETIME now;
  GetSystemTimeAsFileTime(&now);
  if (!SetFileTime(h, NULL, &now, &now)) {
    CloseHandle(h);
    dwerror = GetLastError();
    DG_ERROR("fs_touch(): SetFileTime() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
    return DGERR_INTERNAL_ERROR;
  }
  CloseHandle(h);
  return DGERR_SUCCESS;
#else
  //TODO: K.D. implement for linux
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_remove(dg_pathid_t pathid, const dg_wchar_t* ppath)
{
#ifdef _WIN32
  DWORD dwerror;
  dg_wchar_t fullpath[MAX_PATH];
  dg_wchar_t* p = fs_build_path(fullpath, DG_ARRSIZE(fullpath), pathid, ppath);

  // determine if file or directory
  DWORD fa = GetFileAttributesW(p);
  if (fa == INVALID_FILE_ATTRIBUTES)
    return DGERR_NOT_FOUND;

  BOOL ok;
  if (fa & FILE_ATTRIBUTE_DIRECTORY) {
    ok = RemoveDirectoryW(p);
  }
  else {
    ok = DeleteFileW(p);
  }

  if (!ok) {
    dwerror = GetLastError();
    if (dwerror == ERROR_FILE_NOT_FOUND || dwerror == ERROR_PATH_NOT_FOUND)
      return DGERR_NOT_FOUND;

    const char* pfunc = (fa & FILE_ATTRIBUTE_DIRECTORY) ? "RemoveDirectoryW()" : "DeleteFileW()";
    DG_ERROR("fs_touch(): %s failed! GetLastError()=%d (0x%x)", pfunc, dwerror, dwerror);
    return DGERR_INTERNAL_ERROR;
  }
  return DGERR_SUCCESS;
#else
  //TODO: K.D. implement for linux
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_stor_open(dg_hstor_t* pdst, dg_pathid_t pathid, const char* pstrorname)
{
#ifdef _WIN32
  dg_halloc_result_t hallocres;
  HKEY               h_key;
  wchar_t            subKey[DGFS_MAX_LONG_PATH];
  if (!pdst || !pstrorname)
    return DGERR_INVALID_PARAM;

  // determine base registry subkey (default "Software")
  dg_path_t base = { 0, NULL };
  const dg_wchar_t* pbase_path = L"Software";
  if (fs_get_path_by_id(&base, pathid))
    pbase_path = base.pstring;

  // cvt pstrorname to wide
  wchar_t wide_name[DGFS_MAX_LONG_PATH];
  int wlen = MultiByteToWideChar(CP_UTF8, 0,
    pstrorname, -1, wide_name, DGFS_MAX_LONG_PATH);

  if (wlen == 0) {
    path_free(&base);
    return DGERR_INTERNAL_ERROR;
  }

  // build full key: e.g. "Software\MyApp\Settings"
  swprintf(subKey, DG_ARRSIZE(subKey), L"%ws\\%ws", pbase_path, wide_name);
  path_free(&base);

  LONG res = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    subKey,
    0, NULL,
    REG_OPTION_NON_VOLATILE,
    KEY_READ | KEY_WRITE,
    NULL,
    &h_key,
    NULL
  );
  if (res != ERROR_SUCCESS)
    return DGERR_INTERNAL_ERROR;

  if (!ha_alloc_handle(&hallocres, &storage_handles)) {
    DG_ERROR("fs_stor_open(): ha_alloc_handle() failed!  no free handles!");
    return DGERR_LIMIT_EXCEEDED;
  }
  *((void**)hallocres.phandle_body) = h_key;
  *pdst = hallocres.new_handle;
  return DGERR_SUCCESS;
#else
  (void)pdst; (void)pathid; (void)pstrorname;
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_stor_set_str(dg_hstor_t hstor, const char* pvalname, const char* psrc)
{
  HKEY hKey = (HKEY)ha_get_handle_data(&glob_storage_handles, hstor);
  if (!hKey) {
    DG_ERROR("fs_stor_set_str(): invalid handle");
    return DGERR_INVALID_HANDLE;
  }

  if (!pvalname || !psrc) {
    DG_ERROR("fs_stor_set_str(): invalid parameter");
    return DGERR_INVALID_PARAM;
  }

  DWORD dataSize = (DWORD)(str_length(psrc) + 1); //includes \0
  LONG res = RegSetValueExA(
    hKey,
    pvalname,
    0,
    REG_SZ,
    (const BYTE*)psrc,
    dataSize
  );

  if (res != ERROR_SUCCESS) {
    DG_ERROR("fs_stor_set_str(): RegSetValueExA() failed!");
    return DGERR_INTERNAL_ERROR;
  }
  return DGERR_SUCCESS;
}

int fs_stor_set_bin(dg_hstor_t hstor, const char* pvalname, const void* psrc, size_t length)
{
  HKEY hKey = (HKEY)ha_get_handle_data(&glob_storage_handles, hstor);
  if (!hKey) {
    DG_ERROR("fs_stor_set_bin(): invalid handle");
    return DGERR_INVALID_HANDLE;
  }

  if (!pvalname || !psrc) {
    DG_ERROR("fs_stor_set_bin(): invalid parameter");
    return DGERR_INVALID_PARAM;
  }

  LONG res = RegSetValueExA(
    hKey,
    pvalname,
    0,
    REG_BINARY,
    (const BYTE*)psrc,
    (DWORD)length
  );

  if (res != ERROR_SUCCESS) {
    DG_ERROR("fs_stor_set_bin(): RegSetValueExA() failed!");
    return DGERR_INTERNAL_ERROR;
  }
  return DGERR_SUCCESS;
}

int fs_stor_get(void* pdst, size_t dstlen, size_t* outSize, dg_hstor_t hstor, const char* pvalname)
{
  HKEY hKey;
  DWORD type = 0;
  DWORD dataSz = (DWORD)dstlen;
  LONG  res;

  hKey = (HKEY)ha_get_handle_data(&glob_storage_handles, hstor);
  if (!hKey) {
    DG_ERROR("fs_stor_get(): invalid handle");
    return DGERR_INVALID_HANDLE;
  }

  if (!pvalname || !pdst) {
    DG_ERROR("fs_stor_get(): invalid parameter");
    return DGERR_INVALID_PARAM;
  }

  res = RegQueryValueExA(
    hKey,
    pvalname,
    NULL,
    &type,
    (LPBYTE)pdst,
    &dataSz
  );

  if (res == ERROR_FILE_NOT_FOUND)
    return DGERR_NOT_FOUND;

  if (res == ERROR_MORE_DATA) {
    if (outSize)
      *outSize = dataSz;
    return DGERR_BUFFER_TOO_SMALL;
  }

  if (res != ERROR_SUCCESS) {
    DG_ERROR("fs_stor_get(): RegQueryValueExA() failed!");
    return DGERR_INTERNAL_ERROR;
  }

  if (outSize)
    *outSize = dataSz;

  if (type == REG_SZ && dataSz > 0) {
    char* s = (char*)pdst;
    if (dataSz == 0 || s[dataSz - 1] != '\0') {
      if ((size_t)dataSz < dstlen)
        s[dataSz] = '\0';
      else
        s[dstlen - 1] = '\0';
    }
  }
  return DGERR_SUCCESS;
}

int fs_stor_delete(dg_hstor_t hstor, const char* pvalname)
{
#ifdef _WIN32
  HKEY hKey = (HKEY)ha_get_handle_data(&glob_storage_handles, hstor);
  if (!hKey) {
    DG_ERROR("fs_stor_delete(): invalid handle");
    return DGERR_INVALID_HANDLE;
  }

  if (!pvalname) {
    DG_ERROR("fs_stor_delete(): pvalname is NULL");
    return DGERR_INVALID_PARAM;
  }

  LONG res = RegDeleteValueA(hKey, pvalname);
  if (res == ERROR_FILE_NOT_FOUND)
    return DGERR_NOT_FOUND;

  if (res != ERROR_SUCCESS) {
    DG_ERROR("fs_stor_delete(): RegDeleteValueA() failed!  !!TODO!! K.D. add errors detail to output!");
    return DGERR_INTERNAL_ERROR;
  }

  return DGERR_SUCCESS;
#else
  return DGERR_UNIMPLEMENTED;
#endif
}

int fs_stor_close(dg_hstor_t hstor)
{
#ifdef _WIN32
  HKEY hKey = (HKEY)ha_get_handle_data(&glob_storage_handles, hstor);
  if (!hKey) {
    DG_ERROR("fs_stor_close(): invalid handle");
    return DGERR_INVALID_HANDLE;
  }
  RegCloseKey(hKey);
  ha_free_handle(&glob_storage_handles, hstor);
  return DGERR_SUCCESS;
#else
  (void)hstor;
  return DGERR_UNIMPLEMENTED;
#endif
}
