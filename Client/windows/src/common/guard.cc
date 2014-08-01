

#ifndef WIN32
   #include <cstring>
   #include <cerrno>
   #include <unistd.h>
   #ifdef OSX
      #include <mach/mach_time.h>
   #endif
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #ifdef LEGACY_WIN32
      #include <wspiapi.h>
   #endif
#endif
#include "guard.h"
//
// Automatically lock in constructor
CGuard::CGuard(pthread_mutex_t& lock):
m_Mutex(lock),
m_iLocked()
{
   #ifndef WIN32
      m_iLocked = pthread_mutex_lock(&m_Mutex);
   #else
      m_iLocked = WaitForSingleObject(m_Mutex, INFINITE);
   #endif
}

// Automatically unlock in destructor
CGuard::~CGuard()
{
   #ifndef WIN32
      if (0 == m_iLocked)
         pthread_mutex_unlock(&m_Mutex);
   #else
      if (WAIT_FAILED != m_iLocked)
         ReleaseMutex(m_Mutex);
   #endif
}

void CGuard::enterCS(pthread_mutex_t& lock)
{
   #ifndef WIN32
      pthread_mutex_lock(&lock);
   #else
      WaitForSingleObject(lock, INFINITE);
   #endif
}

void CGuard::leaveCS(pthread_mutex_t& lock)
{
   #ifndef WIN32
      pthread_mutex_unlock(&lock);
   #else
      ReleaseMutex(lock);
   #endif
}

void CGuard::createMutex(pthread_mutex_t& lock)
{
   #ifndef WIN32
      pthread_mutex_init(&lock, NULL);
   #else
      lock = CreateMutex(NULL, false, NULL);
   #endif
}

void CGuard::releaseMutex(pthread_mutex_t& lock)
{
   #ifndef WIN32
      pthread_mutex_destroy(&lock);
   #else
      CloseHandle(lock);
   #endif
}

void CGuard::createCond(pthread_cond_t& cond)
{
   #ifndef WIN32
      pthread_cond_init(&cond, NULL);
   #else
      cond = CreateEvent(NULL, false, false, NULL);
   #endif
}

void CGuard::releaseCond(pthread_cond_t& cond)
{
   #ifndef WIN32
      pthread_cond_destroy(&cond);
   #else
      CloseHandle(cond);
   #endif

}
