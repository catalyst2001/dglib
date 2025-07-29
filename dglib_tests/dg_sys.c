#include "dg_sys.h"
#include "dg_string.h"
#include "dg_alloc.h"
#include "dg_sync.h"

#define DG_WINDOW_CLASS "diddygod"

#define DGSYS_IS_VALID_MODULE(h) ((h)->s.punused)

#define SYSUI_EVENT_QUEUE_SIZE 256
#define SYSUI_MAX_USERDATA 8

typedef struct sysui_event_queue_s {
  dg_sysui_event_t queue[SYSUI_EVENT_QUEUE_SIZE];
  size_t head;
  size_t tail;
  dg_mutex_t mutex;
} sysui_event_queue_t;

typedef struct sysui_window_data_s {
  void* puserdata[SYSUI_MAX_USERDATA];
} sysui_window_data_t;

sysui_event_queue_t glob_events_queue;

/**
* @brief initialize event queue
* @return true if mutex is allocated
*/
inline bool sysui_init_events_queue(sysui_event_queue_t *pdst)
{
  assert(pdst && "pdst was NULL");
  pdst->head = pdst->tail = 0;
  pdst->mutex = mutex_alloc("sysui_event_queue");
  return pdst->mutex != NULL;
}

inline void sysui_deinit_events_queue(sysui_event_queue_t* pdst)
{
  assert(pdst && "pdst was NULL");
  if (pdst->mutex) {
    mutex_free(pdst->mutex);
    pdst->mutex = NULL;
  }
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

int win32_dlopen(hdl_t* pdst, const char* ppath)
{
  DWORD dwerror;
  pdst->s.punused = LoadLibraryA(ppath);
  if (!DGSYS_IS_VALID_MODULE(pdst)) {
    dwerror = GetLastError();
    DG_ERROR("win32_module_load(): LoadLibraryA() failed! GetLastError()=%d (0x%x)", dwerrorl,dwerror);
    return 1;
  }
  return 0;
}

int win32_dlclose(hdl_t* psrc)
{
  if (DGSYS_IS_VALID_MODULE(psrc)) {
    FreeLibrary(psrc->s.punused);
    psrc->s.punused = NULL;
    return 0;
  }
  return 1;
}

inline PIMAGE_OPTIONAL_HEADER win32_get_PE_optional_header(PBYTE base)
{
  PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)base;
  if (pdos->e_magic != IMAGE_DOS_SIGNATURE)
    return NULL;

  PIMAGE_NT_HEADERS pnt = (PIMAGE_NT_HEADERS)(base + pdos->e_lfanew);
  if (pnt->Signature != IMAGE_NT_SIGNATURE)
    return NULL;

  return &pnt->OptionalHeader;
}

int win32_dlinfo(dg_dlinfo_t* pdst, hdl_t hmodule)
{
  PIMAGE_OPTIONAL_HEADER opthdr;
  if (!DGSYS_IS_VALID_MODULE(&hmodule))
    return 1;

  mem_set(pdst, 0, sizeof(*pdst));
  GetModuleFileNameA((HMODULE)hmodule.s.punused, pdst->filename, sizeof(pdst));
  pdst->pbase = hmodule.s.punused;
  opthdr = win32_get_PE_optional_header((PBYTE)pdst->pbase);
  if (opthdr) {
    pdst->sizeofimage = (size_t)opthdr->SizeOfImage;
    pdst->sizeofdata = (size_t)(opthdr->SizeOfInitializedData + opthdr->SizeOfUninitializedData);
    pdst->sizeofcode = (size_t)opthdr->SizeOfCode;
    pdst->pentrypoint = (void*)((PBYTE)pdst->pbase+opthdr->AddressOfEntryPoint);
  }
  return 0;
}

int win32_dlsym(void** pdst, hdl_t hmodule, const char* psymname)
{
  void* psym = GetProcAddress(hmodule.s.punused, psymname);
  if (!psym) {
    DG_ERROR("win32_dlsym(): GetProcAddress() for proc \"%s\" returned NULL", psymname);
    return 1;
  }

  if (pdst)
    *pdst = psym;

  return 0;
}

int win32_dlenum(hdl_t* pdsthmodules, size_t maxlen)
{
  DWORD   dwcount;
  HMODULE* pmods = DG_ALLOC(HMODULE, maxlen);
  if (pmods) {
    if (EnumProcessModules(GetCurrentProcess(), pmods, maxlen*sizeof(HMODULE), &dwcount)) {
      dwcount /= sizeof(DWORD);
      for (DWORD i = 0; i < dwcount; i++) {
        hdl_t* pdstdl = &pdsthmodules[i];
        pdstdl->s.punused = pmods[i];
      }
    }
    DG_FREE(pmods);
    return 0;
  }
  DG_ERROR("win32_dlenum(): HMODULE allocation in heap failed! count=%zd", maxlen);
  return 1;
}

LRESULT CALLBACK win32_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {

  }
  return DefWindowProcA(hwnd, msg, wparam, lparam);
}

/**
* K.D. 
* win32 specific defined here
*/
bool win32_create_window_class()
{
  DWORD       dwerr;
  WNDCLASSEXA wndcls;
  mem_set(&wndcls, 0, sizeof(wndcls));
  wndcls.cbSize = sizeof(wndcls);
  wndcls.cbWndExtra = sizeof(void*);
  wndcls.style = CS_OWNDC;
  wndcls.hInstance = GetModuleHandleW(NULL);
  wndcls.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
  wndcls.lpszClassName = DG_WINDOW_CLASS;
  wndcls.lpfnWndProc = win32_wndproc;
  if (!RegisterClassExA(&wndcls)) {
    dwerr = GetLastError();
    DG_ERROR("win32_create_window_class(): RegisterClassExA() failed! GetLastError()=%d (0x%x)", dwerr, dwerr);
    return false;
  }
  return true;
}

void win32_remove_window_class()
{
  UnregisterClassA(DG_WINDOW_CLASS, GetModuleHandleW(NULL));
}

#else

//TODO: K.D. linux impls must be here!

#endif

static sys_dl_dt_t glob_moduledt = {
#ifdef _WIN32
  .psys_dlopen = win32_dlopen,
  .psys_dlclose = win32_dlclose,
  .psys_dlsym = win32_dlsym,
  .psys_dlinfo = win32_dlinfo,
  .psys_dlenum = win32_dlenum
#else

#endif
};

dg_sysui sysui_window_create(dg_sysui hparent, const char* ptitle, long x, long y, long width, long height)
{
  dg_sysui hwindow=NULL;
#ifdef _WIN32
  DWORD dwerror;
  DWORD dw_exstyle = 0;
  DWORD dw_style = WS_VISIBLE;

  if (x == DG_SYSUI_POS_CENTER)
    x = (GetSystemMetrics(SM_CXSCREEN) - width) >> 1;
  if (y == DG_SYSUI_POS_CENTER)
    y = (GetSystemMetrics(SM_CYSCREEN) - height) >> 1;

  hwindow = (dg_sysui)CreateWindowExA(0, DG_WINDOW_CLASS,
    ptitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW, x, y, width, height,
    (HWND)hparent,
    (HMENU)0,
    GetModuleHandleW(NULL),
    NULL);
  if (!hwindow) {
    dwerror = GetLastError();
    DG_ERROR("sysui_window_create(): CreateWindowExA() failed! GetLastError()=%d (0x%x)",
      dwerror, dwerror);
    return NULL;
  }

  UpdateWindow((HWND)hwindow);
  ShowWindow((HWND)hwindow, SW_SHOW);
#else
  //TODO: K.D. linux impls must be here

#endif
  return hwindow;
}

int sysui_window_get_title(char* pdst, size_t maxlen, dg_sysui hwindow)
{
  GetWindowTextA((HWND)hwindow, pdst, (int)maxlen);
  return 0;
}

int sysui_window_set_title(dg_sysui hwindow, const char* ptitle)
{
  SetWindowTextA((HWND)hwindow, ptitle);
  return 0;
}

inline sysui_window_data_t* win32_window_data(dg_sysui hwindow)
{
  return (sysui_window_data_t*)GetWindowLongPtr(hwindow, 0);
}

void* sysui_window_set_userdata(dg_sysui hwindow, uint32_t slot, const void* pdata)
{
  void* poldptr = NULL;
  sysui_window_data_t* pwindowptr=NULL;
  if (!hwindow)
    return NULL;

  if (slot >= SYSUI_MAX_USERDATA)
    return NULL;

  pwindowptr = win32_window_data(hwindow);
  if (!pwindowptr) {
    DG_ERROR("sysui_window_set_userdata(): GetWindowLongPtr() returned NULL!");
    return NULL;
  }
  poldptr = pwindowptr->puserdata[slot];
  pwindowptr->puserdata[slot] = pdata;
  return poldptr;
}

void* sysui_window_get_userdata(dg_sysui hwindow, uint32_t slot)
{
  sysui_window_data_t* pwindowptr = NULL;
  if (!hwindow)
    return NULL;

  if (slot > SYSUI_MAX_USERDATA)
    return NULL;

  pwindowptr = win32_window_data(hwindow);
  if (!pwindowptr) {
    DG_ERROR("sysui_window_set_userdata(): GetWindowLongPtr() returned NULL!");
    return NULL;
  }
  return pwindowptr->puserdata[slot];
}

bool sysui_push_event(dg_sysui hwindow, const dg_sysui_event_t* psrc)
{
  bool result = false;
  mutex_lock(glob_events_queue.mutex);
  size_t next_tail = (glob_events_queue.tail + 1) % SYSUI_EVENT_QUEUE_SIZE;
  if (next_tail != glob_events_queue.head) {
    glob_events_queue.queue[glob_events_queue.tail] = *psrc;
    glob_events_queue.queue[glob_events_queue.tail].hfrom = hwindow;
    glob_events_queue.tail = next_tail;
    result = true;
  }
  mutex_unlock(glob_events_queue.mutex);
  return result;
}

bool sysui_get_queue_event(dg_sysui_event_t* pdst)
{
  bool result = false;
  mutex_lock(glob_events_queue.mutex);
  if (glob_events_queue.head != glob_events_queue.tail) {
    *pdst = glob_events_queue.queue[glob_events_queue.head];
    glob_events_queue.head = (glob_events_queue.head + 1) % SYSUI_EVENT_QUEUE_SIZE;
    result = true;
  }
  mutex_unlock(glob_events_queue.mutex);
  return result;
}

bool sysui_poll_events(dg_sysui_event_t* pdst)
{
  MSG msg;
  while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  return sysui_get_queue_event(pdst);
}

bool sysui_wait_events(dg_sysui_event_t* pdst)
{
  MSG msg;
  if (GetMessageA(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  return sysui_get_queue_event(pdst);
}

int sys_dlopen(hdl_t* pdst, const char* ppath)
{
  return glob_moduledt.psys_dlopen(pdst, ppath);
}

int sys_dlclose(hdl_t* psrc)
{
  return glob_moduledt.psys_dlclose(psrc);
}

int sys_dlinfo(dg_dlinfo_t* pdst, hdl_t hmodule)
{
  return glob_moduledt.psys_dlinfo(pdst, hmodule);
}

int sys_dlsym(void** pdst, hdl_t hmodule, const char* psymname)
{
  return glob_moduledt.psys_dlsym(pdst, hmodule, psymname);
}

int sys_dlenum(hdl_t* pdsthmodules, size_t maxlen)
{
  return glob_moduledt.psys_dlenum(pdsthmodules, maxlen);
}

sys_dl_dt_t* sys_get_module_api_dt()
{
  return &glob_moduledt;
}

dg_sysui sysui_create_ex(const dg_sysui_create_info_ex_t* pcreateinfo)
{
  return NULL;
}

int sysui_set_size(dg_sysui handle, dg_sysui_size_t size)
{
  return 0;
}

int sysui_get_size(dg_sysui_size_t* pdst, dg_sysui handle)
{
  return 0;
}

int sysui_set_state(dg_sysui handle, int state)
{
  return 0;
}

int sysui_get_state(int* pdststate, dg_sysui handle)
{
  return 0;
}

int sysui_set_parent(dg_sysui handle, dg_sysui hnewparent)
{
  return 0;
}

int sysui_get_parent(dg_sysui* pdsthparent, dg_sysui handle)
{
  return 0;
}

int sysui_set_parameter(int param, uintptr_t value)
{
  return 0;
}

int sysui_get_parameter(uintptr_t* pdstvalue, int param)
{
  return 0;
}

int sysui_kill(dg_sysui handle)
{
  return 0;
}

int sysui_set_minmax(dg_sysui handle, const dg_sysui_size_t min, const dg_sysui_size_t max)
{
  return 0;
}

int sysui_set_userptr(dg_sysui handle, const void* ptr)
{
  return 0;
}

int sysui_get_userptr(void** ptr, dg_sysui handle)
{
  return 0;
}

int sysui_send_msg(dg_sysui handle, int msg, const void* pdata)
{
  return 0;
}

int sysui_get_info(dg_sysui_info_t* pdst, dg_sysui handle)
{
  return 0;
}

/**
* @brief initializing system UI abstaction subsystem
*/
int initialize_sysui()
{
  DG_LOG("initialize_sysui(): initializing system UI abstraction subsystem...");
  /* initialize events queue */
  if (!sysui_init_events_queue(&glob_events_queue)) {
    DG_ERROR("sysui_initialize(): sysui_init_events_queue() failed!");
    return 0;
  }

  if (!win32_create_window_class()) {
    DG_ERROR("sysui_initialize(): win32_create_window_class() failed!");
    return 0;
  }
  return 1;
}

/**
* @brief 
*/
void deinitialize_sysui()
{
  DG_LOG("deinitialize_sysui(): deinitializing system UI abstraction subsystem");
  sysui_deinit_events_queue(&glob_events_queue);
}