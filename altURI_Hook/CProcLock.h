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
//	CProcLock.h - interprocess synchronisation for readers and writers
//=========================================================================

// No writing when reading or vice-versa. Only one concurrent writer but
// multiple readers allowed. Favours writers.
//
// See: http://msdn.microsoft.com/en-us/library/ms810427.aspx

#include "stdafx.h"

#pragma once

// define some names for synchronisation objects to give them ALL_ACCESS rights
#define CPL_RDR_EVENT_NAME TEXT("Cpl_Rdr_Event_Name")
#define CPL_EVENT_NAME TEXT("Cpl_Event_Name")
#define CPL_MUTEX_NAME TEXT("Cpl_Mutex_Name")


//=========================================================================
class CProcLock
{
//===============================================
public:
	enum LOCK_TYPE { NONE = 0L, READ = 1L, WRITE = 2L };

	// structure must be in shared memory
	//===========================================
	struct LIBSPEC LOCK_STATS
	{
		LOCK_TYPE	m_ltLock;
		LONG		m_lCounter;
	};

	//===========================================
	CProcLock( LOCK_STATS* plstats )
	{
		m_plstats = plstats;
	};

	//===========================================
	void CreateAndOpenObjects()
	{
		::CreateEvent( NULL, TRUE, FALSE, CPL_RDR_EVENT_NAME );
		::CreateEvent( NULL, FALSE, TRUE, CPL_EVENT_NAME );
		::CreateMutex( NULL, FALSE, CPL_MUTEX_NAME );
		OpenObjects();
	}

	//===========================================
	void OpenObjects()
	{
		m_hReaderEvent = ::OpenEvent( SYNCHRONIZE, FALSE, CPL_RDR_EVENT_NAME );
		m_hEvent = ::OpenEvent( SYNCHRONIZE, FALSE, CPL_EVENT_NAME );
		m_hWriterMutex = ::OpenMutex( SYNCHRONIZE, FALSE, CPL_MUTEX_NAME );
	}

	//===========================================
	void Lock( LOCK_TYPE ltLock )
	{
		assert( ltLock != NONE );
		assert( m_hWriterMutex && m_hEvent && m_hReaderEvent );
		if ( ltLock == WRITE )
		{  
			::WaitForSingleObject( m_hWriterMutex, INFINITE );
			::WaitForSingleObject( m_hEvent, INFINITE );
			InterlockedExchange( (LONG*)&m_plstats->m_ltLock, WRITE );
		}
		else
		{   
			if ( InterlockedIncrement( &m_plstats->m_lCounter ) == 0 )
			{
				::WaitForSingleObject( m_hEvent, INFINITE );
				::SetEvent( m_hReaderEvent );
			}
			::WaitForSingleObject( m_hReaderEvent, INFINITE );
			InterlockedExchange( (LONG*)&m_plstats->m_ltLock, READ );
		}
	};

	//===========================================
	void Unlock( LOCK_TYPE ltLock )
	{
		assert( ltLock != NONE );
		if ( ltLock == WRITE )
		{ 
			::SetEvent( m_hEvent );
			::ReleaseMutex( m_hWriterMutex );
		}
		else if ( InterlockedDecrement( &m_plstats->m_lCounter ) < 0 )
		{ 
			::ResetEvent( m_hReaderEvent );
			::SetEvent( m_hEvent );
		}
		InterlockedExchange( (LONG*)&m_plstats->m_ltLock, NONE );
	};

	//===========================================
	bool const IsReadLocked() const
	{
		return ( m_plstats->m_ltLock == READ );
	};

	//===========================================
	bool const IsWriteLocked() const
	{
		return ( m_plstats-> m_ltLock == WRITE );
	};

//===============================================
private:
	LOCK_STATS*	m_plstats;
	HANDLE		m_hReaderEvent;
	HANDLE		m_hEvent;
	HANDLE		m_hWriterMutex;
};


//=========================================================================

class CReadLock
{
public:
    CReadLock( CProcLock* pProcLock ) : m_pProcLock(pProcLock)
    {
        assert( m_pProcLock );
        assert( !m_pProcLock->IsReadLocked() );
        m_pProcLock->Lock( CProcLock :: READ );
    };

    virtual ~CReadLock()
	{
		assert( m_pProcLock->IsReadLocked() );
		m_pProcLock->Unlock( CProcLock :: READ );
    };

private:
	CProcLock*   m_pProcLock;
};


//=========================================================================
class CWriteLock
{
public:
    CWriteLock( CProcLock* pProcLock ) : m_pProcLock(pProcLock)
    {
        assert( m_pProcLock );
        assert( !m_pProcLock->IsWriteLocked() );
        m_pProcLock->Lock( CProcLock :: WRITE );
    };

    virtual ~CWriteLock()
	{
		assert( m_pProcLock->IsWriteLocked() );
		m_pProcLock->Unlock( CProcLock :: WRITE );
    };

private:
	CProcLock*   m_pProcLock;
};
