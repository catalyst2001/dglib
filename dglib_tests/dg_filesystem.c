#include "dg_filesystem.h"
#include "dg_sync.h"
#include "dg_string.h"
#include <wchar.h>

typedef struct searchpathinf_s {
  char      id[16];
  dg_path_t path;
} searchpathinf_t;

#define MAX_SEARCH_PATHES 16
#define MAX_FILE_HANDLES  1024

HA_DECL_DATA_ARRAYS(pathes_data, MAX_SEARCH_PATHES, sizeof(searchpathinf_t));
HA_DECL_DATA_ARRAYS(file_handles, MAX_FILE_HANDLES, sizeof(dg_io_handle_data_t));

dg_mutex_t        glob_sp_mtx = NULL;
dg_handle_alloc_t glob_search_pathes;
dg_handle_alloc_t glob_file_handles;

dg_io_dt_t glob_file_iodt = {
  .pio_init=NULL
};

/**
* @brief find search path by path id
* @note this is only for internally use
* 
* @return pointer to searchpathinf_t if path found, otherwise returns NULL
*/
searchpathinf_t* fsi_search_path_by_id(handle_t *pdsth, const char *ppathid)
{
  searchpathinf_t* pspi;
  handle_t handle = ha_get_first_handle(&glob_search_pathes, 0, false);
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

DG_API pathid_t fs_get_path_id(const char* ppathid)
{
  pathid_t path_id_handle=DG_INVALID_HANDLE;
  mutex_lock(glob_sp_mtx);
  fsi_search_path_by_id(&path_id_handle, ppathid);
  mutex_unlock(glob_sp_mtx);
  return path_id_handle;
}

bool fs_get_path_by_id(dg_path_t* pdst, pathid_t pathid)
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

int fs_open_file(file_t* pdst, 
  pathid_t pathid, 
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

int fs_open_buffer(file_t* pdst, void* pbuffer, size_t size, uint32_t mode,
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

int fs_write(file_t hfile, const void* psrc, size_t srclen, size_t* pnbyteswrite)
{
  DGFS_CALL_HANDLER(hfile, pio_write, psrc, srclen, pnbyteswrite)
}

int fs_read(void* pdst, size_t dstlen, file_t hfile, size_t* pnbytesread)
{
  DGFS_CALL_HANDLER(hfile, pio_read, pdst, dstlen, pnbytesread)
}

int fs_write_string(file_t hfile, const char* psrc, size_t length)
{
  //TODO: impl with pio_write
  return 0;
}

const char* fs_read_string(char* pdst, size_t maxlen, size_t length, file_t hfile)
{
  //TODO: impl with pio_read
  return NULL;
}

int fs_read_s16(int32_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s16, pdst, endian)
}

int fs_read_u16(uint32_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u16, pdst, endian)
}

int fs_read_s32(int32_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s32, pdst, endian)
}

int fs_read_u32(uint32_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u32, pdst, endian)
}

int fs_read_s64(int64_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_s64, pdst, endian)
}

int fs_read_u64(uint64_t* pdst, file_t hfile, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_read_u64, pdst, endian)
}

int fs_write_s16(file_t hfile, int32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s16, src, endian)
}

int fs_write_u16(file_t hfile, uint32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u16, src, endian)
}

int fs_write_s32(file_t hfile, int32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s32, src, endian)
}

int fs_write_u32(file_t hfile, uint32_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u32, src, endian)
}

int fs_write_s64(file_t hfile, int64_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_s64, src, endian)
}

int fs_write_u64(file_t hfile, uint64_t src, int endian)
{
  DGFS_CALL_HANDLER(hfile, pio_write_u64, src, endian)
}

int fs_finfo(fs_finfo_t* pdst, file_t hfile)
{
  dg_io_handle_data_t* phd = ha_get_handle_data(&glob_file_handles, hfile);
  if (!phd)
    return 2;

  pdst->access = phd->access;
  pdst->mode = phd->mode;
  return 0;
}

int fs_seek(file_t hfile, int64_t pos, int32_t mode)
{
  DGFS_CALL_HANDLER(hfile, pio_seek, pos, mode)
}

int64_t fs_tell(file_t hfile)
{
  DGFS_CALL_HANDLER_NO_ARGS(hfile, pio_tell)
}

void fs_close(file_t hfile)
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
bool initialize_filesystem()
{
  if (glob_sp_mtx) {
    DG_ERROR("initialize_filesystem(): already initialized");
    return false;
  }

  glob_sp_mtx = mutex_alloc("dg_filesystem:searchpath");
  if (!glob_sp_mtx) {
    DG_ERROR("initialize_filesystem(): mutex allocation failed");
    return false;
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

  return true;
}

/**
* INTERNAL FILESYSTEM DEINITIALIZATION
*/
void deinitialize_filesystem()
{
  if (glob_sp_mtx) {
    /* deinitialize */  
    mutex_free(glob_sp_mtx);
    glob_sp_mtx = NULL;
  }
}