/*http://www.codeproject.com/Articles/146617/Simple-C-Timer-Wrapper*/

#pragma once
#include <atlbase.h>

namespace MyTimer
{

static void CALLBACK TimerProc(void*, BOOLEAN);
static void CALLBACK TimerProc_1(void*, BOOLEAN);

///////////////////////////////////////////////////////////////////////////////
//
// class CTimer
//
class CTimer
{
public:
    CTimer()
    {
        m_hTimer = NULL;
        m_mutexCount = 0;
    }

    virtual ~CTimer()
    {
        Stop();
    }

    bool Start(unsigned int interval,   // interval in ms
               bool immediately = false,// true to call first event immediately
               bool once = false)       // true to call timed event only once
    {
        if( m_hTimer )
        {
            return false;
        }

        SetCount(0);

        BOOL success = CreateTimerQueueTimer( &m_hTimer,
                                              NULL,
                                              TimerProc,
                                              this,
                                              immediately ? 0 : interval,
                                              once ? 0 : interval,
                                              WT_EXECUTEINTIMERTHREAD);

        return( success != 0 );
    }

	bool Start_1(unsigned int interval,   // interval in ms
		bool immediately = false,// true to call first event immediately
		bool once = false)       // true to call timed event only once
	{
		if( m_hTimer )
		{
			return false;
		}

		SetCount(0);

		BOOL success = CreateTimerQueueTimer( &m_hTimer,
			NULL,
			TimerProc_1,
			this,
			immediately ? 0 : interval,
			once ? 0 : interval,
			WT_EXECUTEINTIMERTHREAD);

		return( success != 0 );
	}

    void Stop()
    {
        DeleteTimerQueueTimer( NULL, m_hTimer, NULL );
        m_hTimer = NULL ;
    }

    virtual void OnTimedEvent()
    {
        // Override in derived class
    }

	virtual void OnTimedEvent_1()
	{
		// Override in derived class
	}

    void SetCount(int value)
    {
        InterlockedExchange( &m_mutexCount, value );
    }

    int GetCount()
    {
        return InterlockedExchangeAdd( &m_mutexCount, 0 );
    }

private:
    HANDLE m_hTimer;
    long m_mutexCount;
};

///////////////////////////////////////////////////////////////////////////////
//
// TimerProc
//
void CALLBACK TimerProc(void* param, BOOLEAN timerCalled)
{
    CTimer* timer = static_cast<CTimer*>(param);
    timer->SetCount( timer->GetCount()+1 );
    timer->OnTimedEvent();
};

void CALLBACK TimerProc_1(void* param, BOOLEAN timerCalled)
{
	CTimer* timer = static_cast<CTimer*>(param);
	timer->SetCount( timer->GetCount()+1 );
	timer->OnTimedEvent_1();
};

///////////////////////////////////////////////////////////////////////////////
//
// template class TTimer
//
template <class T> class TTimer : public CTimer
{
public:
    typedef private void (T::*TimedFunction)(void);
	typedef private void (T::*TimedFunction_1)(const std::string& param);

    TTimer()
    {
        m_pTimedFunction = NULL;
        m_pClass = NULL;
    }

    void SetTimedEvent(T *pClass, TimedFunction pFunc)
    {
        m_pClass         = pClass;
        m_pTimedFunction = pFunc;
    }

	void SetTimedEvent_1(T *pClass, TimedFunction_1 pFunc_1, const std::string& param)
	{
		m_pClass			= pClass;
		m_pTimedFunction_1	= pFunc_1;
		m_param				= param;
	}

protected:
    void OnTimedEvent()  
    {
        if (m_pTimedFunction && m_pClass)
        {
            (m_pClass->*m_pTimedFunction)();
        }
    }

	void OnTimedEvent_1()  
	{
		if (m_pTimedFunction_1 && m_pClass)
		{
			(m_pClass->*m_pTimedFunction_1)(m_param);
		}
	}

private:
    T *m_pClass;
    TimedFunction m_pTimedFunction;
	TimedFunction_1 m_pTimedFunction_1;
	std::string m_param;
};

}
