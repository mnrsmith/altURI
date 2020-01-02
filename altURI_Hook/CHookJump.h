/*
Copyright (c) 2010, Mark N R Smith and contributors, All rights reserved.

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
 
 Uses code derived from:
 MinHook - The Minimalistic API Hooking Library for x64/x86
 Copyright (c) 2009 Tsuda Kageyu.

 http://www.codeproject.com/KB/winsdk/LibMinHook.aspx

*/
//
// CHookJump.h
//
#pragma once

typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#pragma pack(push, 1)
struct JMP_REL
{
	uint8_t		opcode;
	uint32_t	operand;
};

struct JMP_ABS
{
	uint16_t	opcode;
	uint32_t	operand;
	uint64_t	indirect;
};
#pragma pack(pop)

struct CHookJump
{
public:
	CHookJump()	{ m_bEnabled = false; };

	bool InstallHook( LPVOID pFunc, LPVOID pFuncNew )
	{
		if ( m_bEnabled ) return false;
		if ( pFunc == NULL || pFuncNew == NULL ) return false;

		// save original function jump
		m_pBackup = AllocateBuffer( NULL, PAGE_READONLY, m_stSizeJumpRel );
		memcpy( m_pBackup, pFunc, m_stSizeJumpRel );

		// save the target function address
		m_pTarget = pFunc;

		m_pDetour = AllocateBuffer( NULL, PAGE_READONLY, m_stSizeJumpRel );

		if ( m_pDetour == NULL ) return false;

#if defined _M_X64
		m_pRelay = AllocateBuffer( pFunc, PAGE_EXECUTE_READ, m_stSizeJumpAbs );

		if ( m_pRelay == NULL ) return false;
		
		WriteAbsoluteJump( m_pRelay, pFuncNew );

		WriteRelativeJump( m_pDetour, m_pTarget, m_pRelay );
#else
		WriteRelativeJump( m_pDetour, m_pTarget, pFuncNew );
#endif
		
		// allow writing to target memory
		VirtualProtect( m_pTarget, m_stSizeJumpRel, PAGE_EXECUTE_READWRITE, &m_dwOldProtect );

		EnableHook();
		return true;
	};

	void RemoveHook( void )
	{
		DisableHook();

#if defined _M_X64
		VirtualFree( m_pRelay, 0, MEM_RELEASE );
#endif
		VirtualFree( m_pDetour, 0, MEM_RELEASE );

		VirtualFree( m_pBackup, 0, MEM_RELEASE );

		// VirtualProtect( m_pTarget, sizeof(JMP_REL), m_dwOldProtect, &m_dwOldProtect );
	};
	
	// put back saved code fragment
	inline void DisableHook( void )
	{
		if ( !m_bEnabled ) return;
		memcpy( m_pTarget, m_pBackup, m_stSizeJumpRel );
		m_bEnabled = false;
	};

	// put back JMP instruction again
	inline void EnableHook( void )
	{
		if ( m_bEnabled ) return;
		memcpy( m_pTarget, m_pDetour, m_stSizeJumpRel );
		m_bEnabled = true;
	};

private:

	void* AllocateBuffer( void* const pOrigin, DWORD protect, size_t size )
	{
		assert(("AllocateBuffer", (protect == PAGE_EXECUTE_READ || protect == PAGE_READONLY)));
		assert(("AllocateBuffer", (size > 0)));

		size = (size + TYPE_ALIGNMENT(void*) - 1) & ~(TYPE_ALIGNMENT(void*) - 1);

		LPVOID pBuffer = ReserveSuitableMemory( pOrigin, protect, size );

		if ( pBuffer )
		{
			if ( VirtualAlloc(pBuffer, size, MEM_COMMIT, protect) == NULL ) return NULL;

			DWORD oldProtect;
			// PAGE_EXECUTE_READ -> PAGE_EXECUTE_READWRITE, PAGE_READONLY -> PAGE_READWRITE
			if ( !VirtualProtect(pBuffer, size, (protect << 1), &oldProtect ) ) return NULL;
		}

		return pBuffer;
	};

	LPVOID ReserveSuitableMemory( void* const pOrigin, DWORD protect, size_t capacity )
	{
		assert(("ReserveSuitableMemory", (protect == PAGE_EXECUTE_READ || protect == PAGE_READONLY)));
		assert(("ReserveSuitableMemory", (capacity > 0)));

		LPVOID pAlloc = NULL;

#if defined _M_X64
		if (pOrigin != NULL)
		{
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			
			intptr_t minAddr = reinterpret_cast<intptr_t>(si.lpMinimumApplicationAddress);
			intptr_t maxAddr = reinterpret_cast<intptr_t>(si.lpMaximumApplicationAddress);

			// pOrigin 512MB
			minAddr = std::max<intptr_t>(minAddr, reinterpret_cast<intptr_t>(pOrigin) - 0x20000000);
			maxAddr = std::min<intptr_t>(maxAddr, reinterpret_cast<intptr_t>(pOrigin) + 0x20000000);

			intptr_t min = minAddr / capacity;
			intptr_t max = maxAddr / capacity;
			int rel = 0;
			MEMORY_BASIC_INFORMATION mi = { 0 };
			for (int i = 0; i < (max - min + 1); ++i)
			{
				rel = -rel + (i & 1);
				void* pQuery = reinterpret_cast<void*>(((min + max) / 2 + rel) * capacity);
				VirtualQuery(pQuery, &mi, sizeof(mi));
				if ( mi.State == MEM_FREE )
				{
					pAlloc = VirtualAlloc(pQuery, capacity, MEM_RESERVE, protect);
					if (pAlloc != NULL) break;
				}
			}
		}
		else
#else
		UNREFERENCED_PARAMETER(pOrigin);
#endif		// X86
		{
			pAlloc = VirtualAlloc( NULL, capacity, MEM_RESERVE, protect) ;
		}

		return pAlloc;
	};

	void WriteRelativeJump( void* pStore, void* const pFrom, void* const pTo )
	{
		JMP_REL jmp;
		jmp.opcode  = 0xE9;
		jmp.operand = static_cast<uint32_t>(reinterpret_cast<char*>(pTo) - (reinterpret_cast<char*>(pFrom) + sizeof(jmp)));

		memcpy( pStore, &jmp, sizeof(jmp) );
	};

	void WriteAbsoluteJump( void* pFrom, void* const pTo )
	{
		JMP_ABS jmp;
		jmp.opcode  = 0x25FF;
		jmp.operand = 0; // 32bit offset to 64bit pointer (below)
		
		jmp.indirect = reinterpret_cast<uint64_t>( pTo );

		memcpy( pFrom,  &jmp, sizeof(jmp) );
	}

private:
	void*				m_pTarget;		// what I am replacing
	void*				m_pBackup;		// what was there previously
	void*				m_pDetour;		// what I want to replace it with.
	bool				m_bEnabled;		// hook is active
	DWORD				m_dwOldProtect;	// protection setting when hook installed
	static const size_t	m_stSizeJumpRel = sizeof(JMP_REL);

#if defined _M_X64
	void*				m_pRelay;
	intptr_t			m_gMinAddress;
	intptr_t			m_gMaxAddress;
	static const size_t	m_stSizeJumpAbs = sizeof(JMP_ABS);
#endif

};

