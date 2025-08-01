#pragma once
#include "dg_libcommon.h"

enum DGTERM_EVENT {
	TERMEVT_OPEN = 0,
	TERMEVT_CLOSE,
	TERMEVT_CHAR,
	TERMEVT_CTRLCHAR,
	TERMEVT_READYTEXT
};

typedef struct dg_tevent_s {
	uint32_t evt;
	uint32_t ctlchr;
	uint32_t chr;
	char    *ptext;
} dg_tevent_t;

typedef void (*thandler)(dg_tevent_t *pevent);

DG_API thandler term_get_handler();
DG_API void     term_set_handler(thandler pproc);
DG_API int      term_open(bool with_in);
DG_API bool     term_exists();
DG_API int      term_close();
DG_API int      term_set_title(const char *psrc);
DG_API int      term_get_title(char* pdst, size_t maxlen);
DG_API int      term_update();