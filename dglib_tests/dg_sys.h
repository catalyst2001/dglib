/**
*/
#pragma once
#include "dg_libcommon.h"

typedef struct hdl_s {
	struct { void* punused; } s;
} hdl_t;

typedef struct dg_dlinfo_s {
	char   filename[512];
	void*  pbase;
	void*  pentrypoint;
	size_t sizeofimage;
	size_t sizeofdata;
	size_t sizeofcode;
} dg_dlinfo_t;

DG_API int sys_dlopen(hdl_t *pdst, const char *ppath);
DG_API int sys_dlclose(hdl_t *psrc);
DG_API int sys_dlinfo(dg_dlinfo_t *pdst, hdl_t hmodule);
DG_API int sys_dlsym(void **pdst, hdl_t hmodule, const char *psymname);
DG_API int sys_dlenum(hdl_t *pdsthmodules, size_t maxlen);

typedef struct sys_dl_dt_s {
	int (*psys_dlopen)(hdl_t* pdst, const char* ppath);
	int (*psys_dlclose)(hdl_t* psrc);
	int (*psys_dlinfo)(dg_dlinfo_t* pdst, hdl_t hmodule);
	int (*psys_dlsym)(void** pdst, hdl_t hmodule, const char* psymname);
	int (*psys_dlenum)(dg_dlinfo_t* pdsthmodules, size_t maxlen);
} sys_dl_dt_t;

DG_API sys_dl_dt_t* sys_get_module_api_dt();

/*
===============================
          System UI abstraction API
For different systems, it is sometimes necessary to create standard 
controls or windows to build a primitive user interface.
This API should provide all the basic functionality for working with the 
system UI to display the user interface based on the operating system UI.
===============================
*/
typedef struct dg_sysui_rect_s {
	long left, top, right, bottom;
} dg_sysui_rect_t;

typedef struct dg_sysui_size_s {
	long width, height;
} dg_sysui_size_t;

typedef dg_voidptr_t dg_sysui;

typedef struct dg_sysui_create_info_ex_s {
	dg_sysui        hparent;
	const char*     pname;
	dg_sysui_rect_t rect;
	uint32_t        id;
	void*           pexstruct;
	size_t          exstruct_size;
} dg_sysui_create_info_ex_t;

DG_API dg_sysui sysui_create_ex(const dg_sysui_create_info_ex_t *pcreateinfo);
DG_API int      sysui_set_size(dg_sysui handle, dg_sysui_size_t size);
DG_API int      sysui_get_size(dg_sysui_size_t *pdst, dg_sysui handle);
DG_API int      sysui_set_state(dg_sysui handle, int state);
DG_API int      sysui_get_state(int *pdststate, dg_sysui handle);
DG_API int      sysui_set_parent(dg_sysui handle, dg_sysui hnewparent);
DG_API int      sysui_get_parent(dg_sysui *pdsthparent, dg_sysui handle);
DG_API int      sysui_set_parameter(int param, uintptr_t value);
DG_API int      sysui_get_parameter(uintptr_t *pdstvalue, int param);
DG_API int      sysui_kill(dg_sysui handle);
DG_API int      sysui_set_minmax(dg_sysui handle, const dg_sysui_size_t min, const dg_sysui_size_t max);
DG_API int      sysui_set_userptr(dg_sysui handle, const void *ptr);
DG_API int      sysui_get_userptr(void** ptr, dg_sysui handle);
DG_API int      sysui_send_msg(dg_sysui handle, int msg, const void *pdata);

typedef struct dg_sysui_info_s {
	void* psysobj;
	void* pmonitor;
} dg_sysui_info_t;

DG_API int sysui_get_info(dg_sysui_info_t *pdst, dg_sysui handle);

/**
* @brief window event for handling
*/
enum DG_SYSUI_EVENT {
	SYSUI_EVENT_SPAWN=0, /*< window spawned */
	SYSUI_EVENT_DIE, /*< window must be die */
	SYSUI_EVENT_RESIZE, /*< window resized */
	SYSUI_EVENT_ENTERFOCUS, /*< keyboard focus entered */
	SYSUI_EVENT_LEAVEFOCUS, /*< keyboard focus leaved */
	SYSUI_EVENT_MOUSECLICK, /*< mouse clicked */
	SYSUI_EVENT_MOUSEMOVE, /*< mouse moved */
	SYSUI_EVENT_MOUSEWHEEL, /*< mouse wheel scrolled */
	SYSUI_EVENT_SCREENSTATE, /*< screen state changed */
	SYSUI_EVENT_SCREENSAVE, /*< print screen */
	SYSUI_EVENT_POWERSTATE, /*< system requested power off or reboot */
	SYSUI_EVENT_KEYDOWN, /*< keyboard or mouse key is pressed */
	SYSUI_EVENT_KEYUP, /*< keyboard or mouse key is released */
	SYSUI_EVENT_CHAR, /*< translated char symbol with current language */
	SYSUI_EVENT_STATECHANGE, /*< window state changed */
	SYSUI_EVENT_XEVENT, /* reserved */
};

/**
* @brief events structure
*/
typedef struct dg_sysui_event_s {
	uint32_t evt;
	dg_sysui hfrom;
	dg_sysui_rect_t rect;
} dg_sysui_event_t;

#define DG_SYSUI_SLONG_BITS (sizeof(long)*8-1)
#define DG_SYSUI_POS_CENTER (1<<DG_SYSUI_SLONG_BITS)

DG_API dg_sysui sysui_window_create(dg_sysui hparent, const char *ptitle, long x, long y, long width, long height);
DG_API int      sysui_window_get_title(char *pdst, size_t maxlen, dg_sysui hwindow);
DG_API int      sysui_window_set_title(dg_sysui hwindow, const char *ptitle);
DG_API void*    sysui_window_set_userdata(dg_sysui hwindow, uint32_t slot, const void *pdata);
DG_API void*    sysui_window_get_userdata(dg_sysui hwindow, uint32_t slot);

DG_API bool sysui_push_event(dg_sysui hwindow, const dg_sysui_event_t* psrc);
DG_API bool sysui_poll_events(dg_sysui_event_t *pdst);
DG_API bool sysui_wait_events(dg_sysui_event_t *pdst);