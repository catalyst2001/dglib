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
	uint32_t event;
	uint32_t ctlchr;
	uint32_t chr;
	char    *ptext;
} dg_tevent_t;

typedef void (*thandler)(dg_tevent_t *pevent);

thandler term_get_handler();
void     term_set_handler(thandler pproc);
int      term_open(bool with_in);
bool     term_exists();
int      term_close();
int      term_set_title(const char *psrc);
int      term_get_title(char* pdst, size_t maxlen);
int      term_update();