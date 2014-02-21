#ifndef __TIMER_H__
#define __TIMER_H__
//=======================================================================================================================
//  Timer ���
//	@history- 2005.2.28 : Timer based time ���� �ٲپ���
//=======================================================================================================================
#pragma once

namespace util
{
class Timer
{
public: 
	Timer():m_dwExpireTime(0),m_dwIntervalTime(0),m_bCheckTime(false) {}
	~Timer(){}

public: 

	inline bool IsAlive() const { return m_bCheckTime ; }
	inline void SetTimer( DWORD dwIntervalTime )
	{
		m_dwIntervalTime	= dwIntervalTime;
		Reset();
	}
	inline void SetIntervalTime(DWORD dwIntervalTime)
	{
		m_dwIntervalTime	= dwIntervalTime;
	}
	inline DWORD GetIntervalTime() { return m_dwIntervalTime;	}
	DWORD GetRemainTime()
	{
		if (! IsExpired(false))
		{
			DWORD dwProcess = GetProcessTime();
			CHECKZERO(m_dwIntervalTime > dwProcess);
			return m_dwIntervalTime- dwProcess;
		}
		
		return 0;
	}

	DWORD  GetProcessTime()
	{
		DWORD dwCurTick = GetTickCount();
 		CHECKZERO( m_dwExpireTime > 0 && m_dwExpireTime >= m_dwIntervalTime );
 		CHECKZERO( dwCurTick >= (m_dwExpireTime - m_dwIntervalTime) );
		return dwCurTick >= m_dwExpireTime ? m_dwIntervalTime :( dwCurTick - (m_dwExpireTime - m_dwIntervalTime) );
	}
	// m_dwIntervalTime �ð� ������ �״�� �����ϰ� �ٽ� ��ʱ���� ������.
	inline void Reset(/*m_dwIntervalTime*/)
	{
		m_dwExpireTime		= GetTickCount() + m_dwIntervalTime;
		EnableCheckTime();
	}

	// ��ʱ���� resume
	inline void EnableCheckTime()
	{
		m_bCheckTime = true;
	}

	// ��ʱ���� �����Ѵ�.
	inline void DisableCheckTime()
	{
		m_bCheckTime = false;
	}
	inline void ResetReCalcTimer(int iValue)
	{
		CHECK(safe_cast<int>(m_dwExpireTime) >= abs(iValue));
		if(!IsExpired(FALSE))
		{
			m_dwExpireTime	= m_dwExpireTime + iValue;	
		}
		
	}
	inline void DecreaseExpireTime( DWORD dwExpireTime )
	{
		CHECK( m_dwExpireTime >= dwExpireTime );
		m_dwExpireTime -= dwExpireTime;
	}
	// ����Ÿ���� �Ǳ������ ��ǰ �����
	 float GetProgressRatio()
	{
		if(m_dwIntervalTime == 0)
			return 1.0f;
		//int dwProgressTime = GetTickCount() - (m_dwExpireTime - m_dwIntervalTime);
		return min(1.0f,((float)GetProcessTime() / (float)m_dwIntervalTime));
	}

	// problem : �� ������ ���� ȣ���� ��� IsExpired() == TRUE�� �Ǵ� ��찡 �����.
	// �ֳ��� 1 Tick�̶� ������ ������ �ʱ� ���� IsExpired()ȣ���� �ð��� �������� �ʴ´�.
	// Therefore, �̷���� IsExpired() == TRUE�� �Ŀ� Reset()�� �ѹ� ȣ���� �ش�!
	// �Ƿ�ʱ [2010-3-4 10:21:49 DZY]
	inline BOOL IsExpired( BOOL bReset=TRUE )
	{
		if( m_bCheckTime && GetTickCount() >= m_dwExpireTime ) 
		{
			if( bReset )
			{
				m_dwExpireTime = GetTickCount() + m_dwIntervalTime;
			}
			return TRUE;
		}
		else 
			return FALSE;
	}

	// �ѹ��� ��ʱ���� ������ ����
	inline BOOL IsExpiredManual( BOOL bReset=TRUE )
	{
		DWORD dwCurTime;
		if( m_bCheckTime && ( dwCurTime = GetTickCount() ) >= m_dwExpireTime ) 
		{
			if( bReset )
			{
				m_dwExpireTime = dwCurTime + m_dwIntervalTime;
			}
			DisableCheckTime();
			return TRUE;
		}
		else 
			return FALSE;
	}

	void InitTime()
	{
		m_dwExpireTime = 0;
	}

	inline DWORD GetExpireTime()	{ return m_dwExpireTime; }

private:
	bool		m_bCheckTime;
	DWORD		m_dwExpireTime;
	DWORD		m_dwIntervalTime;

};


/************************************************************************
��ѡһ�Ĳ���
bool TimeOut()	��ʱ�󷵻�TURE���Ժ���Ҳ������TURE, ���������������5����һ��������Ϣ���Ժ���Ҳ������
bool TimeOver()	��ʱ����Զ����TURE�������ԣ�����30���Ӻ󣬻��0����
bool NextTime() ��ʱ��ͬ���г�����һ�η���TURE.�����ж���ÿ5���һ��Ѫ
	��NextTime��ʱ��	����¼��Ѿ�������ÿ�ε����ʱ����ʣ���ʱ�����ۼƵ���һ��
	��NextTime��ʱ��	����¼��Ѿ�������ÿ�ε����ʱ����ʣ���ʱ�����ۼƵ���һ��

��ѡһ
��õ�ǰʱ���ʱ��	���Լ�����bool TimeOut()
�ⲿ����ʱ��TimeOut( DWORD dwCurTick )

����Ķ���֧��
����һ��ķ�ʽ	24Сʱһ��
��ҹ֮�����µ�һ��

ʱ�䵥λ	��		������	SetTimerSec() SetTimerTick SetTimerDay()
����	����ʱ��
��		ÿ��ֻ������ε�����	
************************************************************************/
struct TimeOutBev
{
	static void ResetInterval( const DWORD & /*dwCurTick*/ , DWORD & /*dwLastTick*/ , DWORD & dwIntervalTick )
	{
		dwIntervalTick = 0;
	}
};

struct TimeOverBev
{
	static void ResetInterval( const DWORD & /*dwCurTick*/ , DWORD & /*dwLastTick*/ ,DWORD & /*dwIntervalTick*/ ){}
};

struct UseAccumulate
{
	static void CalRemainTick( const DWORD &dwCurTick , DWORD &dwLastTick ,DWORD &dwIntervalTick )
	{
		CHECK( dwCurTick > dwLastTick );
		CHECK( 0 != dwIntervalTick );
		dwLastTick = dwLastTick+dwIntervalTick;
	}
};

struct NotUseAccumulate
{
	static void CalRemainTick( const DWORD &dwCurTick , DWORD &dwLastTick ,DWORD & /*dwIntervalTick*/  )
	{
		dwLastTick = dwCurTick ;
	}
};


template< class T_CalAcount>
struct TimeNextTime
{
	static void ResetInterval( const DWORD &dwCurTick , DWORD &dwLastTick ,DWORD &dwIntervalTick )
	{
		T_CalAcount::CalRemainTick( dwCurTick , dwLastTick , dwIntervalTick );
	}
};


template<class T_ResetIntervalBev >
class RecvTickTimer
{
public:
	RecvTickTimer()
	{
		m_dwLastTick = 0;
		m_dwIntervalTick = 0;
	}
	bool Init( DWORD dwIntervalTick , DWORD dwCurTick )
	{
		CHECKF( dwIntervalTick > 0 );
		m_dwIntervalTick = dwIntervalTick;
		m_dwLastTick = dwCurTick ;
		return true;
	}
	bool TimeEvent( DWORD dwCurTick )
	{
		if( 0 == m_dwIntervalTick )
			return false;

		ASSERT( dwCurTick >= m_dwLastTick );
		ASSERT( m_dwIntervalTick > 0 );
		if( dwCurTick - m_dwLastTick < m_dwIntervalTick )
			return false;

		T_ResetIntervalBev::ResetInterval( dwCurTick ,m_dwLastTick , m_dwIntervalTick );
		return true;
	}
private:
	DWORD m_dwLastTick ;
	DWORD m_dwIntervalTick ;
};

// ����
template<class T_ResetIntervalBev >
class AutoTimer
{
public:
	bool InitTicks( DWORD dwIntervalTick )
	{
		return m_TimerRecvTick.Init( dwIntervalTick , GetTickCount() ) ;
	}
	bool TimeEvent()
	{
		return m_TimerRecvTick.TimeEvent( GetTickCount() ) ;
	}
private:
	RecvTickTimer<T_ResetIntervalBev> m_TimerRecvTick ;
};

template<class T_ResetIntervalBev >
class AutoTimerSecs
{
public:
	bool InitSecs( DWORD dwSecs )
	{
		return m_TimerRecvTick.InitTicks( dwSecs * CLOCKS_PER_SEC ) ;
	}
	bool TimeEvent()
	{
		return m_TimerRecvTick.TimeEvent() ;
	}
private:
	AutoTimer<T_ResetIntervalBev>  m_TimerRecvTick ;
};

// ��ʱ��ֻ����һ�� [2009-12-14 14:53:54  ]
typedef AutoTimer<TimeOutBev> TimeOutTimer;//ʹ�ú���
typedef AutoTimerSecs<TimeOutBev> TimeOutTimerSecs;//ʹ����
typedef AutoTimerSecs<TimeNextTime<UseAccumulate> > TimeNextTimeSecsAccumulate;// ���۴���������ʹ���� [2010-3-30 11:27:52  ]
typedef AutoTimerSecs<TimeNextTime<NotUseAccumulate> > TimeNextTimeSecsNoAccumulate;// �����۴���������ʹ���� [2010-3-30 11:27:52  ]
}//End of namespace util





#endif //__TIMER_H__

