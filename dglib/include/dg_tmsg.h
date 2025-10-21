#ifndef __dg_textmsg_included__
#define __dg_textmsg_included__
#include "dg_libcommon.h"

#define DG_MSG_TEXT_MAX 512

typedef void (*text_func_t)(const char *text);
DG_API text_func_t set_msg_handler(text_func_t phandler);

DG_API void text_msgf(const char* pformat, ...);

#endif