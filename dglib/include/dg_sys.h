/*===============================
 "Diggy Gods" Team  Copyright (c) 2025.
 Abstract: System routines abstraction layer
 Purpose: Provides basic access to system routines for loading 
  and unloading dynamic libraries, searching for symbol addresses, 
  querying basic information about a dynamic library, 
  and some functions for working with the system user interface.
 Authors: Deryabin K.
===============================*/
#pragma once
#include "dg_libcommon.h"

#define SYS_MAX_PATH 512 /*< max path length */

#ifdef _WIN32
#define DGSYS_PLATFORM_LIBRARY_EXT ".dll"
#elif __linux__
#define DGSYS_PLATFORM_LIBRARY_EXT ".so"
#elif __APPLE__
#define DGSYS_PLATFORM_LIBRARY_EXT ".dylib"
#else
#error "Unknown platform!"
#endif

/**
* @brief Dynamic library handle
*/
typedef struct hdl_s {
	struct { void* punused; } s;
} hdl_t;

/**
* @brief Dynamic library information structure
*/
typedef struct dg_dlinfo_s {
	char   filename[512]; /*< library file name */
	void* pbase; 				/*< base address where the library is loaded */
	void* pentrypoint; 	/*< entry point address */
	size_t sizeofimage; /*< size of the loaded image in memory */
	size_t sizeofdata; /*< size of initialized and uninitialized data */
	size_t sizeofcode; /*< size of code section */
} dg_dlinfo_t;

/**
* @brief load dynamic library
* @param pdst pointer to the handle that will receive the library handle
* @param ppath path to the dynamic library file
* @return 0 if successful, or a negative error code
* 
* @note The library file extension should not be specified in the path
*/
DG_API int sys_dlopen(hdl_t *pdst, const char *ppath);

/**
* @brief unload dynamic library
* @param psrc pointer to the handle of the library to be unloaded
* @return 0 if successful, or a negative error code
*/
DG_API int sys_dlclose(hdl_t *psrc);

/**
* @brief get information about the loaded dynamic library
* @param pdst pointer to the structure that will receive the library information
* @param hmodule handle of the library to query information about
* @return 0 if successful, or a negative error code
*/
DG_API int sys_dlinfo(dg_dlinfo_t *pdst, hdl_t hmodule);

/**
* @brief get the address of a symbol in the loaded dynamic library
* @param pdst pointer to the variable that will receive the symbol address
* @param hmodule handle of the library to search for the symbol
* @param psymname name of the symbol to search for
* @return 0 if successful, or a negative error code
*/
DG_API int sys_dlsym(void **pdst, hdl_t hmodule, const char *psymname);

/**
* @brief enumerate loaded dynamic libraries in current process
* @param pdstdls pointer to the array that will receive the library handles
* @param maxlen maximum number of handles that can be stored in the array
* @return number of libraries stored in the array, or a negative error code
*/
DG_API int sys_dlenum(hdl_t *pdstdls, size_t maxlen);

/**
* @brief load a dynamic library and get the address of a symbol in it
* @param ppath path to the dynamic library file
* @param pprocname name of the symbol to search for
* @return address of the symbol if successful, or NULL on failure
*/
DG_API void* sys_dlloadproc(const char* ppath, const char* pprocname);

/**
* @brief get the system API for dynamic library operations
*/
typedef struct sys_dl_dt_s {
	/**
	* @brief load dynamic library
	* @param pdst pointer to the handle that will receive the library handle
	* @param ppath path to the dynamic library file
	*	@return 0 if successful, or a negative error code
	*/
	int (*psys_dlopen)(hdl_t* pdst, const char* ppath);

	/**
	* @brief unload dynamic library
	* @param psrc pointer to the handle of the library to be unloaded
	* @return 0 if successful, or a negative error code
	*/
	int (*psys_dlclose)(hdl_t* psrc);

	/**
	* @brief get information about the loaded dynamic library
	* @param pdst pointer to the structure that will receive the library information
	* @param hmodule handle of the library to query information about
	* @return 0 if successful, or a negative error code
	*/
	int (*psys_dlinfo)(dg_dlinfo_t* pdst, hdl_t hmodule);

	/**
	* @brief get the address of a symbol in the loaded dynamic library
	* @param pdst pointer to the variable that will receive the symbol address
	* @param hmodule handle of the library to search for the symbol
	* @param psymname name of the symbol to search for
	* @return 0 if successful, or a negative error code
	*/
	int (*psys_dlsym)(void** pdst, hdl_t hmodule, const char* psymname);

	/**
	* @brief enumerate loaded dynamic libraries in current process
	* @param pdstdls pointer to the array that will receive the library handles
	* @param maxlen maximum number of handles that can be stored in the array
	* @return number of libraries stored in the array, or a negative error code
	*/
	int (*psys_dlenum)(hdl_t* pdsthmodules, size_t maxlen);
} sys_dl_dt_t;

/**
* @brief get the system API for dynamic library operations
*/
DG_API sys_dl_dt_t* sys_get_module_api_dt();

/**
* @brief get the command line string
* @return pointer to the command line string
*/
DG_API const char* sys_get_cmdline();

/**
* @brief terminate the process with the specified exit code
* @param code exit code
* @note This function does not return
*/
DG_API void sys_pexit(int code);

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

typedef struct dg_sysui_point_s {
	long x, y;
} dg_sysui_point_t;

typedef dg_voidptr_t dg_sysui;

typedef struct dg_sysui_create_info_ex_s {
	dg_sysui        hparent;
	const char*     pname;
	dg_sysui_rect_t rect;
	uint32_t        id;
	uint32_t        widgetid;
	void*           pexstruct;
	size_t          exstruct_size;
} dg_sysui_create_info_ex_t;

#define SYS_ID_NONE (0)
#define SYS_ID_OK (1<<0)
#define SYS_ID_CANCEL (1<<1)
#define SYS_ID_RETRY (1<<2)

#define SYS_MSGBOX_ERROR (1<<4)
#define SYS_MSGBOX_INFO (1<<5)

DG_API int sys_show_msgbox(dg_sysui parent_window, const char* pcaption, const char* ptext, int flags);

DG_API dg_sysui sysui_create_ex(const dg_sysui_create_info_ex_t *pcreateinfo);

/*
===============================
 label
===============================
*/
DG_API dg_sysui sysui_label_create(dg_sysui hparent, long x, long y, long width, long height, uint32_t id, const char* ptext, ...);
DG_API void     sysui_label_set_text(dg_sysui hlabel, const char *ptext);
DG_API void     sysui_label_get_text(char* pdst, size_t dstlen, dg_sysui hlabel);

DG_API dg_sysui sysui_image_create(dg_sysui hparent, long x, long y, long width, long height, uint32_t id, const char* ppath);
DG_API dg_sysui sysui_button_create(dg_sysui hparent, long x, long y, long width, long height, uint32_t id, const char* name);
DG_API dg_sysui sysui_editbox_create(dg_sysui hparent, long x, long y, long width, long height, uint32_t id, const char* name);
DG_API dg_sysui sysui_textbox_create(dg_sysui hparent, long x, long y, long width, long height, uint32_t id, const char* name);

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
	SYSUI_EVENT_CLOSE, /*< window is closing */
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
	SYSUI_EVENT_DRAGFILE, /* < window received file in drag-n-drop */
	SYSUI_EVENT_XEVENT, /* reserved */
};

DG_API const char* sysui_event_to_string(uint32_t event);

typedef struct dg_sysui_keyevt_s {
	uint32_t scancode;
	uint32_t keycode;
	uint32_t modifiers;
} dg_sysui_keyevt_t;

typedef struct dg_sysui_mouseevt_s {
	uint32_t         key;
	uint32_t         kstate;
	uint32_t         wheeldelta;
	dg_sysui_point_t point;
} dg_sysui_mouseevt_t;

/**
* @brief events structure
*/
typedef struct dg_sysui_event_s {
	uint32_t evt;
	dg_sysui hfrom;
	union { 
		dg_sysui_keyevt_t   key;
		dg_sysui_mouseevt_t mouse;
		dg_sysui_size_t     size;
		dg_sysui_rect_t     rect;
	};
} dg_sysui_event_t;

#define DG_SYSUI_SLONG_BITS (sizeof(long)*8-1)
#define DG_SYSUI_POS_CENTER (1<<DG_SYSUI_SLONG_BITS)

DG_API dg_sysui sysui_window_create(dg_sysui hparent, const char *ptitle, long x, long y, long width, long height);
DG_API int      sysui_window_get_title(char *pdst, size_t maxlen, dg_sysui hwindow);
DG_API int      sysui_window_set_title(dg_sysui hwindow, const char *ptitle);
DG_API void*    sysui_window_set_userdata(dg_sysui hwindow, uint32_t slot, const void *pdata);
DG_API void*    sysui_window_get_userdata(dg_sysui hwindow, uint32_t slot);

DG_API bool     sysui_push_event(dg_sysui hwindow, const dg_sysui_event_t* psrc);
DG_API bool     sysui_poll_events(dg_sysui_event_t *pdst);
DG_API bool     sysui_wait_events(dg_sysui_event_t *pdst);

/*
===============================
					System input abstraction API
To unify virtual key codes, key constants defined by this 
abstraction are used, which try to follow the most common majority of virtual codes.
===============================
*/
DG_API uint8_t*     sysin_get_keys_array(size_t *pdstsize);
DG_API int          sysin_get_key_state(int keycode);
DG_API void         sysin_get_cursor_pos(int* px, int* py);
DG_API void         sysin_set_cursor_pos(int x, int y);

/* CURSOR API */
typedef dg_voidptr_t dg_syscursor;

enum SYSCUR_TYPE {
	SYSCUR_ARROW = 0,
	SYSCUR_IBEAM,
	SYSCUR_WAIT,
	SYSCUR_CROSS,
	SYSCUR_UPARROW,
	SYSCUR_SIZE_NWSE,
	SYSCUR_SIZE_NESW,
	SYSCUR_SIZE_WE,
	SYSCUR_SIZE_NS,
	SYSCUR_SIZE_ALL,
	SYSCUR_NO,
	SYSCUR_HAND,

	SYSCUR_TOTAL_CURSORS
};

DG_API dg_syscursor sys_create_native_cursor(enum SYSCUR_TYPE cursor_type);
DG_API dg_syscursor sys_create_custom_cursor(const char* ppath, int hotx, int hoty);
DG_API void         sys_destroy_cursor(dg_syscursor hcursor);
DG_API void         sys_set_cursor(dg_syscursor hcursor);
DG_API dg_syscursor sys_get_cursor();

typedef struct dg_cursor_mode_s {
	int mode;
} dg_cursor_mode_t;

DG_API int          sys_set_cursor_mode(const dg_cursor_mode_t *pmode);
DG_API int          sys_get_cursor_mode(dg_cursor_mode_t *pmode);

DG_API void         sys_show_cursor(bool show);
DG_API bool         sys_is_cursor_visible();

/* CLIPBOARD */
enum SYS_CLIPBOARD_DATA {
	SYS_CLIPBOARD_DATA_NONE = 0,
	SYS_CLIPBOARD_DATA_TEXT,
	SYS_CLIPBOARD_DATA_IMAGE,
};

DG_API int          sys_set_clipboard_text(const char* ptext);
DG_API int          sys_get_clipboard_text(char* pdst, size_t maxlen);
DG_API int          sys_get_clipboard_data_type();

#define SYSKS_RELEASED  (0) /*< key is released */
#define SYSKS_PRESSED   (1 << 1) /*< key is pressed */
#define SYSKS_REPEAT    (1 << 0) /*< key repeat */
#define SYSKS_RIGHT     (1 << 2) /*< right or left key */

enum DG_SYSKEY {
	/* special */
	SKEY_RETURN, SKEY_ESCAPE, SKEY_BACKSPACE, SKEY_TAB, SKEY_SPACE,
	SKEY_LEFT, SKEY_RIGHT, SKEY_UP, SKEY_DOWN,
	SKEY_INSERT, SKEY_DELETE, SKEY_HOME, SKEY_END, SKEY_PAGEUP, SKEY_PAGEDOWN,
	SKEY_SHIFT, SKEY_CONTROL, SKEY_ALT,
	SKEY_F1, SKEY_F2, SKEY_F3, SKEY_F4, SKEY_F5, SKEY_F6,
	SKEY_F7, SKEY_F8, SKEY_F9, SKEY_F10, SKEY_F11, SKEY_F12,

	/* mouse keys */
	SKEY_MOUSEL, SKEY_MOUSER, SKEY_MOUSEM,
	SKEY_MOUSEX0, SKEY_MOUSEX1, SKEY_MOUSEX2, 
	SKEY_MOUSEX3, SKEY_MOUSEX4, SKEY_MOUSEX5,
	SKEY_MOUSEX6, SKEY_MOUSEX7,

	/* numbers */
	SKEY_0='0', SKEY_1, SKEY_2, SKEY_3, SKEY_4, SKEY_5, SKEY_6,
	SKEY_7, SKEY_8, SKEY_9,

	/* alpha */
	SKEY_A = 'A', SKEY_B, SKEY_C, SKEY_D, SKEY_E, SKEY_F, SKEY_G, SKEY_H, SKEY_I, SKEY_J,
	SKEY_K, SKEY_L, SKEY_M, SKEY_N, SKEY_O, SKEY_P, SKEY_Q, SKEY_R, SKEY_S, SKEY_T,
	SKEY_U, SKEY_V, SKEY_W, SKEY_X, SKEY_Y, SKEY_Z,

	/* numpad */
	SKEY_NUM0, SKEY_NUM1, SKEY_NUM2, SKEY_NUM3, SKEY_NUM4,
	SKEY_NUM5, SKEY_NUM6, SKEY_NUM7, SKEY_NUM8, SKEY_NUM9,
	SKEY_TOTAL,
	SKEY_UNKNOWN = SKEY_TOTAL
};
