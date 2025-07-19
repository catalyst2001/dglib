#include "dg_sync.h"
#include "dg_alloc.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

dg_mutex_t mutex_alloc(const char* pname)
{
  CRITICAL_SECTION* pcs = DG_NEW(CRITICAL_SECTION);
  if (!pcs)
    return NULL;

  InitializeCriticalSection(pcs);
  return (dg_mutex_t)pcs;
}

void mutex_lock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  EnterCriticalSection((LPCRITICAL_SECTION)hmutex);
}

bool mutex_try_lock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  return TryEnterCriticalSection((LPCRITICAL_SECTION)hmutex);
}

void mutex_unlock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  LeaveCriticalSection((LPCRITICAL_SECTION)hmutex);
}

void mutex_free(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  free(hmutex);
}

dg_cond_t cond_alloc(const char* pname)
{
  CONDITION_VARIABLE* pcv = DG_NEW(CONDITION_VARIABLE);
  if (pcv)
    return NULL;

  InitializeConditionVariable(pcv);
  return (dg_cond_t)pcv;
}

void cond_wait(dg_cond_t hcond, dg_mutex_t hmutex)
{
  assert(hcond && "hcond is NULL");
  SleepConditionVariableCS(
    (PCONDITION_VARIABLE)hcond, 
    (LPCRITICAL_SECTION)hmutex,
    INFINITE);
}

void cond_signal(dg_cond_t hcond)
{
  assert(hcond && "hcond is NULL");
  WakeConditionVariable((PCONDITION_VARIABLE)hcond);
}

void cond_broadcast(dg_cond_t hcond)
{
  assert(hcond && "hcond is NULL");
  WakeAllConditionVariable((PCONDITION_VARIABLE)hcond);
}

void cond_free(dg_cond_t hcond)
{
  assert(hcond && "hcond is NULL");
  //((PCONDITION_VARIABLE)hcond);
  free(hcond);
}

dg_semaphore_t semaphore_alloc(int initial_value, int max_count, const char* pname)
{
  DWORD dwerror;
  if (max_count == -1)
    max_count = LONG_MAX;

  HANDLE hsem = CreateSemaphoreA(NULL, (LONG)initial_value, (LONG)max_count, NULL);
  if (hsem == NULL) {
    dwerror = GetLastError();
    DG_ERROR("CreateSemaphoreA() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
    return NULL;
  }
  return (dg_semaphore_t)hsem;
}

void semaphore_wait(dg_semaphore_t hsem)
{
  assert(hsem && "hsem is NULL");
  WaitForSingleObject((HANDLE)hsem, INFINITE); //NOTE: K.D. WAIT_OBJECT_0 ignored here
}

int semaphore_trywait(dg_semaphore_t hsem)
{
  DWORD dwerror;
  DWORD res = WaitForSingleObject((HANDLE)hsem, 0L);
  if (res == WAIT_OBJECT_0)
    return true; //captured

  if (res == WAIT_TIMEOUT)
    return false; //counter 0 (empty)

  dwerror = GetLastError();
  DG_ERROR("WaitForSingleObject() returned %d! GetLastError()=%d (0x%x)", res, dwerror, dwerror);
  return false;
}

bool semaphore_timed_wait(dg_semaphore_t hsem, int waitms)
{
  assert(hsem && "hsem is NULL");
  return WaitForSingleObject((HANDLE)hsem, (DWORD)waitms) == WAIT_OBJECT_0;
}

bool semaphore_try_wait(dg_semaphore_t hsem)
{
  assert(hsem && "hsem is NULL");
  if (WaitForSingleObject((HANDLE)hsem, INFINITE) == WAIT_OBJECT_0) {
    ReleaseSemaphore((HANDLE)hsem, 1, NULL);
    return true;
  }
  return false;
}

void semaphore_post(dg_semaphore_t hsem)
{
  assert(hsem && "hsem is NULL");
  ReleaseSemaphore((HANDLE)hsem, 1, NULL);
}

void semaphore_free(dg_semaphore_t hsem)
{
  assert(hsem && "hsem is NULL");
  CloseHandle((HANDLE)hsem);
}

#else
dg_mutex_t mutex_alloc(const char* pname)
{
  return dg_mutex_t();
}

void mutex_lock(dg_mutex_t hmutex)
{
}

bool mutex_try_lock(dg_mutex_t hmutex)
{
  return false;
}

void mutex_unlock(dg_mutex_t hmutex)
{
}

void mutex_free(dg_mutex_t hmutex)
{
}

dg_cond_t cond_alloc(const char* pname)
{
  return dg_cond_t();
}

void cond_wait(dg_cond_t hcond, dg_mutex_t hmutex)
{
}

void cond_signal(dg_cond_t hcond)
{
}

void cond_broadcast(dg_cond_t hcond)
{
}

void cond_free(dg_cond_t hcond)
{
}

#endif