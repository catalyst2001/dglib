#include "dg_term.h"

thandler global_phandler = NULL;

thandler term_get_handler()
{
  return global_phandler;
}

void term_set_handler(thandler pproc)
{
  global_phandler = pproc;
}

#ifdef _WIN32
#include <windows.h>

HANDLE global_stdin = INVALID_HANDLE_VALUE;
HANDLE global_stdout = INVALID_HANDLE_VALUE;
HANDLE global_stderr = INVALID_HANDLE_VALUE;

int term_open(bool with_in)
{
  DWORD dwerror;
  if (!term_exists()) {
    if (!AllocConsole()) {
      dwerror = GetLastError();
      DG_ERROR("AllocConsole() failed! GetLastError()=%d (0x%x)",
        dwerror, dwerror);
      return DGERR_UNKNOWN_ERROR;
    }
  }

  global_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
  if (global_stdout == INVALID_HANDLE_VALUE) {
    DG_ERROR("term_open(): GetStdHandle(STD_OUTPUT_HANDLE) returned INVALID_HANDLE_VALUE");
    return DGERR_FAILED;
  }

  global_stderr = GetStdHandle(STD_ERROR_HANDLE);
  if (global_stderr == INVALID_HANDLE_VALUE) {
    DG_ERROR("term_open(): GetStdHandle(STD_ERROR_HANDLE) returned INVALID_HANDLE_VALUE");
    return DGERR_FAILED;
  }

  if (with_in) {
    global_stdin = GetStdHandle(STD_INPUT_HANDLE);
    if (global_stdin == INVALID_HANDLE_VALUE) {
      DG_ERROR("term_open(): GetStdHandle(STD_INPUT_HANDLE) returned INVALID_HANDLE_VALUE");
      return DGERR_FAILED;
    }
  }
  return DGERR_SUCCESS;
}

bool term_exists()
{
  return !!GetConsoleWindow();
}

int term_close()
{
  return 0;
}

int term_set_title(const char* psrc)
{
  return 0;
}

int term_get_title(char* pdst, size_t maxlen)
{
  return 0;
}

int term_update()
{
  return 0;
}

#endif