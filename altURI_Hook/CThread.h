/*
Copyright (c) 2010, Mark N R Smith, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer. Redistributions in binary form must
reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the
distribution. Neither the name of the author nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//=========================================================================
//	CThread.h - simple thread class
//=========================================================================
#pragma once

#include <process.h>


class CThread
{
public:
	static enum Priority
	{
		Idle = THREAD_PRIORITY_IDLE,
		Lowest = THREAD_PRIORITY_LOWEST,
		BelowNormal = THREAD_PRIORITY_BELOW_NORMAL,
		Normal = THREAD_PRIORITY_NORMAL,
		AboveNormal = THREAD_PRIORITY_ABOVE_NORMAL,
		Highest = THREAD_PRIORITY_HIGHEST,
		TimeCritical = THREAD_PRIORITY_TIME_CRITICAL
	};

	inline CThread( void ) : m_hThread(NULL), m_bRunning(false), m_bStop(false), m_bUsingOuterCS(false) { InitializeCriticalSection(&m_cs); }

	inline virtual ~CThread( void ) { if (!m_bUsingOuterCS) DeleteCriticalSection(&m_cs); }
	bool Start( const void* data = NULL, const bool suspended = false)
	{
		if ( m_hThread ) return false;	// already started

		//start the thread
		this->m_pData = data;
		m_bStop = false;	// not stopping yet
		unsigned int id;
		m_hThread = (HANDLE)_beginthreadex( NULL, 0 ,_ThreadProc, this, (suspended)?CREATE_SUSPENDED:0, &id );
		
		m_bRunning = ( m_hThread != 0 );
		return m_bRunning;
	}
	inline void SetStop( const bool bStop = true ) { this->m_bStop = bStop; }
	inline bool GetStop() { return m_bStop; }
	bool Terminate( const int ret = 0 )
	{
		if(!m_hThread) return false;
		
		m_bRunning = !TerminateThread( m_hThread, ret );
		if ( !m_bRunning )
		{
			m_bStop = false;
			CloseHandle( m_hThread );
			m_hThread = 0;
		}

		return !m_bRunning;
	}
	int Suspend()
	{
		int iRet = SuspendThread( m_hThread );
		m_bRunning = ( iRet <= 0 );
		return iRet;
	}
	int Resume()
	{
		int iRet = ResumeThread( m_hThread );
		m_bRunning = ( iRet<=0 );
		return iRet;
	}
	inline bool IsRunning() { return m_bRunning; }
	inline bool IsStarted() { return ( m_hThread != NULL ) ; }
	inline const void* GetData() { return m_pData; }
	inline bool SetPriority(Priority priority) { return ( SetThreadPriority( m_hThread, priority ) == 0 ); }
	inline Priority GetPriority() { return (Priority)GetThreadPriority( m_hThread ); }
	inline operator HANDLE() { return m_hThread; }

	inline void Lock() { EnterCriticalSection( &m_cs ); };
	inline void Unlock() { LeaveCriticalSection( &m_cs ); };
	inline void UseAnotherLock( const CRITICAL_SECTION& cs ) { DeleteCriticalSection(&m_cs); m_cs = cs; m_bUsingOuterCS = true; }

protected:
	virtual unsigned int ThreadProc() = 0;

	static unsigned int __stdcall _ThreadProc(void* param)
	{
		// invoke the virtual thread procedure
		CThread* pThread = reinterpret_cast<CThread*>( param );
		unsigned int iRet = pThread->ThreadProc();

		// set state
		CloseHandle( pThread->m_hThread );
		pThread->m_hThread = NULL;
		pThread->SetStop( false );

		// terminate
		_endthreadex( iRet );
		return iRet; //we never get here
	}

protected:
	HANDLE						m_hThread;
	const void*					m_pData;
	bool						m_bRunning;
	bool						m_bStop;
	CRITICAL_SECTION			m_cs;

private:
	bool						m_bUsingOuterCS;
};



