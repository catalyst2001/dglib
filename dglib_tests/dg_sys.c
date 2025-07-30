#include "dg_sys.h"
#include "dg_string.h"
#include "dg_alloc.h"
#include "dg_sync.h"

#define DG_WINDOW_CLASS "diddygod"

#define DGSYS_IS_VALID_MODULE(h) ((h)->s.punused)

#define SYSUI_EVENT_QUEUE_SIZE 256
#define SYSUI_MAX_USERDATA 8
#define SYSUI_MAX_KEYS 2048

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
uint16_t           glob_fwd_keyremap[SYSUI_MAX_KEYS];
uint16_t           glob_bck_keyremap[SYSUI_MAX_KEYS];
uint8_t            glob_keyarray[SYSUI_MAX_KEYS];

/**
* @brief initialize event queue
* @return true if mutex is allocated
*/
inline bool sysui_init_events_queue(sysui_event_queue_t* pdst)
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
#include <windowsx.h>
#include <Psapi.h>

static void sysin_init_keyremap()
{
	mem_set(glob_fwd_keyremap, 0xFF, sizeof(glob_fwd_keyremap));
	mem_set(glob_bck_keyremap, 0xFF, sizeof(glob_bck_keyremap));
#define MAPKEY(vk, sk)\
	glob_fwd_keyremap[vk] = sk;\
	glob_bck_keyremap[sk] = vk;

	MAPKEY(VK_RETURN, SKEY_RETURN);
	MAPKEY(VK_ESCAPE, SKEY_ESCAPE);
	MAPKEY(VK_BACK, SKEY_BACKSPACE);
	MAPKEY(VK_TAB, SKEY_TAB);
	MAPKEY(VK_SPACE, SKEY_SPACE);
	MAPKEY(VK_LEFT, SKEY_LEFT);
	MAPKEY(VK_RIGHT, SKEY_RIGHT);
	MAPKEY(VK_UP, SKEY_UP);
	MAPKEY(VK_DOWN, SKEY_DOWN);
	MAPKEY(VK_SHIFT, SKEY_SHIFT);
	MAPKEY(VK_CONTROL, SKEY_CONTROL);
	MAPKEY(VK_MENU, SKEY_ALT);
	MAPKEY(VK_F1, SKEY_F1);
	MAPKEY(VK_F2, SKEY_F2);
	MAPKEY(VK_F3, SKEY_F3)
	MAPKEY(VK_F4, SKEY_F4);
	MAPKEY(VK_F5, SKEY_F5);
	MAPKEY(VK_F6, SKEY_F6);
	MAPKEY(VK_F7, SKEY_F7);
	MAPKEY(VK_F8, SKEY_F8);
	MAPKEY(VK_F9, SKEY_F9);
	MAPKEY(VK_F10, SKEY_F10);
	MAPKEY(VK_F11, SKEY_F11);
	MAPKEY(VK_F12, SKEY_F12);
	MAPKEY(VK_NUMPAD0, SKEY_NUM0);
	MAPKEY(VK_NUMPAD1, SKEY_NUM1);
	MAPKEY(VK_NUMPAD2, SKEY_NUM2);
	MAPKEY(VK_NUMPAD3, SKEY_NUM3);
	MAPKEY(VK_NUMPAD4, SKEY_NUM4);
	MAPKEY(VK_NUMPAD5, SKEY_NUM5);
	MAPKEY(VK_NUMPAD6, SKEY_NUM6);
	MAPKEY(VK_NUMPAD7, SKEY_NUM7);
	MAPKEY(VK_NUMPAD8, SKEY_NUM8);
	MAPKEY(VK_NUMPAD9, SKEY_NUM9);
	MAPKEY('A', SKEY_A);
	MAPKEY('B', SKEY_B);
	MAPKEY('C', SKEY_C);
	MAPKEY('D', SKEY_D);
	MAPKEY('E', SKEY_E);
	MAPKEY('F', SKEY_F);
	MAPKEY('G', SKEY_G);
	MAPKEY('H', SKEY_H);
	MAPKEY('I', SKEY_I);
	MAPKEY('J', SKEY_J);
	MAPKEY('K', SKEY_K);
	MAPKEY('L', SKEY_L);
	MAPKEY('M', SKEY_M);
	MAPKEY('N', SKEY_N);
	MAPKEY('O', SKEY_O);
	MAPKEY('P', SKEY_P);
	MAPKEY('Q', SKEY_Q);
	MAPKEY('R', SKEY_R);
	MAPKEY('S', SKEY_S);
	MAPKEY('T', SKEY_T);
	MAPKEY('U', SKEY_U);
	MAPKEY('V', SKEY_V);
	MAPKEY('W', SKEY_W);
	MAPKEY('X', SKEY_X);
	MAPKEY('Y', SKEY_Y);
	MAPKEY('Z', SKEY_Z);
	MAPKEY('0', SKEY_0);
	MAPKEY('1', SKEY_1);
	MAPKEY('2', SKEY_2);
	MAPKEY('3', SKEY_3);
	MAPKEY('4', SKEY_4);
	MAPKEY('5', SKEY_5);
	MAPKEY('6', SKEY_6);
	MAPKEY('7', SKEY_7);
	MAPKEY('8', SKEY_8);
	MAPKEY('9', SKEY_9);

	/* mouse */
	MAPKEY(VK_LBUTTON, SKEY_MOUSEL);
	MAPKEY(VK_RBUTTON, SKEY_MOUSER);
	MAPKEY(VK_MBUTTON, SKEY_MOUSEM);
	MAPKEY(VK_XBUTTON1, SKEY_MOUSEX0);
	MAPKEY(VK_XBUTTON2, SKEY_MOUSEX1);
	//MAPKEY(0xFF, SKEY_MOUSEX2);
	//MAPKEY(0xFF, SKEY_MOUSEX3);
	//MAPKEY(0xFF, SKEY_MOUSEX4);
	//MAPKEY(0xFF, SKEY_MOUSEX5);
	//MAPKEY(0xFF, SKEY_MOUSEX6);
	//MAPKEY(0xFF, SKEY_MOUSEX7);
#undef MAPKEY
}

int win32_dlopen(hdl_t* pdst, const char* ppath)
{
	DWORD dwerror;
	pdst->s.punused = LoadLibraryA(ppath);
	if (!DGSYS_IS_VALID_MODULE(pdst)) {
		dwerror = GetLastError();
		DG_ERROR("win32_module_load(): LoadLibraryA() failed! GetLastError()=%d (0x%x)", dwerrorl, dwerror);
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
		pdst->pentrypoint = (void*)((PBYTE)pdst->pbase + opthdr->AddressOfEntryPoint);
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
		if (EnumProcessModules(GetCurrentProcess(), pmods, maxlen * sizeof(HMODULE), &dwcount)) {
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
	dg_sysui_event_t ev = { 0 };
	ev.hfrom = (dg_sysui)hwnd;
	switch (msg) {
	case WM_KEYDOWN:
		ev.evt = SYSUI_EVENT_KEYDOWN;
		ev.key.keycode = (uint32_t)glob_fwd_keyremap[wparam];
		ev.key.scancode = (lparam >> 16) & 0xFF;
		ev.key.modifiers = 0;

		if (lparam & ((LPARAM)1 << 30)) // key is repeating
			ev.key.modifiers |= SYSKS_REPEAT;

		glob_keyarray[ev.key.keycode] = SYSKS_PRESSED | (ev.key.modifiers & SYSKS_REPEAT);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_KEYUP:
		ev.evt = SYSUI_EVENT_KEYUP;
		ev.key.keycode = (uint32_t)glob_fwd_keyremap[wparam];
		ev.key.scancode = (lparam >> 16) & 0xFF;
		ev.key.modifiers = SYSKS_RELEASED;
		glob_keyarray[ev.key.keycode] = SYSKS_RELEASED;
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_CHAR:
		ev.evt = SYSUI_EVENT_CHAR;
		ev.key.keycode = (uint32_t)wparam;
		return sysui_push_event(ev.hfrom, &ev);

	case WM_CLOSE:
		ev.evt = SYSUI_EVENT_DIE;
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_SIZE:
		ev.evt = SYSUI_EVENT_RESIZE;
		ev.size.size.width = LOWORD(lparam);
		ev.size.size.height = HIWORD(lparam);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_MOUSEMOVE:
		ev.evt = SYSUI_EVENT_MOUSEMOVE;
		ev.mouse.point.x = GET_X_LPARAM(lparam);
		ev.mouse.point.y = GET_Y_LPARAM(lparam);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_MOUSEWHEEL:
		ev.evt = SYSUI_EVENT_MOUSEWHEEL;
		ev.mouse.wheeldelta = GET_WHEEL_DELTA_WPARAM(wparam);
		ev.mouse.point.x = GET_X_LPARAM(lparam);
		ev.mouse.point.y = GET_Y_LPARAM(lparam);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	//TODO: K.D. add handling double click
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		ev.evt = SYSUI_EVENT_MOUSECLICK;
		ev.mouse.key = msg; //TODO: K.D. fix here
		ev.mouse.kstate = SYSKS_PRESSED;
		ev.mouse.point.x = GET_X_LPARAM(lparam);
		ev.mouse.point.y = GET_Y_LPARAM(lparam);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		ev.evt = SYSUI_EVENT_MOUSECLICK;
		ev.mouse.kstate = SYSKS_RELEASED;
		ev.mouse.key = msg; //TODO: K.D. fix here
		ev.mouse.point.x = GET_X_LPARAM(lparam);
		ev.mouse.point.y = GET_Y_LPARAM(lparam);
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_SETFOCUS:
		ev.evt = SYSUI_EVENT_ENTERFOCUS;
		sysui_push_event(ev.hfrom, &ev);
		return 0;

	case WM_KILLFOCUS:
		ev.evt = SYSUI_EVENT_LEAVEFOCUS;
		sysui_push_event(ev.hfrom, &ev);
		return 0;

		//TODO: K.D. add handling new events here
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
	dg_sysui hwindow = NULL;
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
	sysui_window_data_t* pwindowptr = NULL;
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

	if (slot >= SYSUI_MAX_USERDATA)
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

uint8_t* sysin_get_keys_array(size_t* pdstsize)
{
	if (pdstsize)
		*pdstsize = sizeof(glob_keyarray) / sizeof(glob_keyarray[0]);

	return glob_keyarray;
}

int sysin_get_key_state(int keycode)
{
	if (keycode < 0 || keycode >= SYSUI_MAX_KEYS)
		return 0;

	return glob_keyarray[keycode];
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
	if (IsWindow((HWND)handle)) {
		DestroyWindow((HWND)handle);
		return 0;
	}
	return 1;
}

int sysui_set_minmax(
	dg_sysui handle,
	const dg_sysui_size_t min,
	const dg_sysui_size_t max)
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
	sysin_init_keyremap();
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