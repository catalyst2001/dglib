#include "dg_tmsg.h"
#include <stdarg.h>
#include "dg_string.h"

static void emptyfunc(const char* text) {}
text_func_t glob_textfunc = emptyfunc;

text_func_t set_msg_handler(text_func_t phandler)
{
	text_func_t poldfunc = glob_textfunc;
	glob_textfunc = phandler;
	return poldfunc;
}

void text_msgf(const char* pformat, ...)
{
	char buf[DG_MSG_TEXT_MAX];
	va_list argptr;
	va_start(argptr, pformat);
	str_vformat(buf, sizeof(buf), pformat, argptr);
	va_end(argptr);
	glob_textfunc(buf);
}