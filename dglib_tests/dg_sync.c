#include "dg_sync.h"
#include "dg_alloc.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

dg_mutex_t mtx_alloc(const char* pname)
{
  CRITICAL_SECTION* pcs = DG_NEW(CRITICAL_SECTION);
  if (!pcs)
    return NULL;

  InitializeCriticalSection(pcs);
  return (dg_mutex_t)pcs;
}

void mtx_lock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  EnterCriticalSection((LPCRITICAL_SECTION)hmutex);
}

bool mtx_try_lock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  return TryEnterCriticalSection((LPCRITICAL_SECTION)hmutex);
}

void mtx_unlock(dg_mutex_t hmutex)
{
  assert(hmutex && "hmutex is NULL");
  LeaveCriticalSection((LPCRITICAL_SECTION)hmutex);
}

void mtx_free(dg_mutex_t hmutex)
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

#else
dg_mutex_t mtx_alloc(const char* pname)
{
  return dg_mutex_t();
}

void mtx_lock(dg_mutex_t hmutex)
{
}

bool mtx_try_lock(dg_mutex_t hmutex)
{
  return false;
}

void mtx_unlock(dg_mutex_t hmutex)
{
}

void mtx_free(dg_mutex_t hmutex)
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