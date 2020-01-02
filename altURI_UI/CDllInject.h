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
//	CDllInject.h - get process list and inject dll into a process
//=========================================================================
#pragma once

#include <list>
#include "psapi.h"

#pragma comment(lib, "Psapi")

#define YES_DLL1 TEXT("d3d8.dll")
#define YES_DLL2 TEXT("d3d9.dll")
#define YES_DLL3 TEXT("opengl32.dll")
#define YES_DLL4 TEXT("d3d10.dll")
#define YES_DLL5 TEXT("d3d11.dll")


struct CInjectableProcess
{
	CInjectableProcess(){};
	~CInjectableProcess(){ m_sProcessName.clear(); };
	CInjectableProcess( const DWORD dwProcessID, TCHAR* tcProcessName )  : m_dwProcessID( dwProcessID ), m_sProcessName( tcProcessName ) {};
	DWORD	m_dwProcessID;
	tString	m_sProcessName;
};

class CDllInject
{
public:
	CDllInject(){};
	~CDllInject(){ m_lProcesses.clear(); m_tHookDll.clear(); };

	void SetHookDllNameToExclude( const tString& sName ) { m_tHookDll = sName; };

	bool LaunchSimulator(	const tString& sPath,
							const tString& sPathWithExeName,
							const tString& sParams)
	{
		STARTUPINFO si; 
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		ZeroMemory( &pi, sizeof(pi) );
		si.cb = sizeof( si );

#ifdef _UNICODE
		LPWSTR lpParams = (LPWSTR)sParams.c_str();
#else
		LPSTR lpParams = (LPSTR)sParams.c_str();
#endif

		BOOL bResult = CreateProcess(	sPathWithExeName.c_str(),
										lpParams,
										NULL,
										NULL,
										FALSE,
										CREATE_DEFAULT_ERROR_MODE,
										NULL,
										sPath.c_str(),
										&si,
										&pi );

		CloseHandle(pi.hProcess); 
		CloseHandle(pi.hThread);

		return ( bResult == TRUE );
	};

	bool FillProcesses( void )
	{
		DWORD BytesReturned, NumProcesses;

		m_lProcesses.clear();	// clear listbox

		if ( !EnumProcesses( ProcessIds, sizeof(ProcessIds), &BytesReturned ) ) return false;
  
		// Calculate how many process identifiers were returned.  
		NumProcesses = BytesReturned / sizeof(DWORD);

		// get current process id so we can exclude it
		DWORD CurrentProcessId = GetCurrentProcessId();
  
		// check each process for dlls - graphics or hook  
		for ( DWORD i = 0; i < NumProcesses; i++ )
			if ( ( ProcessIds[i] != 0 ) && ( ProcessIds[i] != CurrentProcessId)  )
			{
				TCHAR* tResult = GetProcessIfGraphicsUsed( ProcessIds[i] );

				if (tResult) m_lProcesses.push_back( CInjectableProcess( ProcessIds[i], tResult ) );
			}
		return true;
	};

	TCHAR* GetProcessIfGraphicsUsed( DWORD ProcessID )
	{
		static TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
		  
		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessID );
	  
		// Get the process name.  
		if ( hProcess != NULL )
		{
			HMODULE hMods[1024];
			DWORD BytesReturned, NumModules;
	     
			// First module is the executable, subsequent handles are its DLLs
			if ( EnumProcessModules( hProcess, &hMods[0], sizeof(hMods), &BytesReturned ) )
			{
				// save exe name
				GetModuleBaseName( hProcess, hMods[0], szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );

				NumModules = BytesReturned / sizeof(DWORD);

				bool bUsingGraphicsDlls = false;

				for ( DWORD i = 1; i < NumModules; i++ )	// loop through dll modules 
					if ( hMods[i] != 0 ) 
					{
						TCHAR szModuleName[MAX_PATH] = TEXT("<unknown>");
						GetModuleBaseName( hProcess, hMods[i], szModuleName, sizeof(szModuleName)/sizeof(TCHAR) );

						tString tModuleName( szModuleName );

						if ( !m_tHookDll.empty() )
						{
							if ( tModuleName.find( m_tHookDll ) == 0 )
							{
								CloseHandle( hProcess );
								return NULL;	// already hooked - don't return name
							}
						}

						// make all lower case for matching
						std::transform( tModuleName.begin(), tModuleName.end(), tModuleName.begin(), tolower);

						if (	( tModuleName.find( YES_DLL1 ) == 0 ) || 		// look for graphics dlls
								( tModuleName.find( YES_DLL2 ) == 0 ) ||
								( tModuleName.find( YES_DLL3 ) == 0 ) ||
								( tModuleName.find( YES_DLL4 ) == 0 ) ||
								( tModuleName.find( YES_DLL5 ) == 0 ) )
								bUsingGraphicsDlls = true;
					}
				
				CloseHandle( hProcess );
				return bUsingGraphicsDlls ? szProcessName : NULL;		// if any graphics dlls return process name
			}
		}
	  
		CloseHandle( hProcess );
		return NULL;
	};

	bool DoInject( DWORD ProcessID, const tString& sDllFile )
	{
		bool bRes = false;

		//if ( !SetDebugPrivilege( TRUE ) )
		//	return false;

		HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, ProcessID );

		// allocate memory in the remote process for max size of dll path+name
		LPVOID pLibRemote = VirtualAllocEx( hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		if( !pLibRemote )
		{
			// std::cout << "ERROR: Unable to VirtualAllocEx: " << MAX_PATH << ", GetLastError=" << GetLastError() << std::endl; 
			CloseHandle( hProcess );
			return false;
		}

		// covert from wide-chars for unicode
		std::string sTemp( sDllFile.begin(), sDllFile.end() );

		// copy dll name and path to the allocated memory
		if ( WriteProcessMemory( hProcess, pLibRemote, (void*)sTemp.c_str(), sTemp.size(), NULL ) == TRUE )
			bRes = RemoteLoadLibrary( hProcess, pLibRemote );
		
		int b;

		if (!bRes)
			//std::cout << "ERROR: Unable to WriteProcessMemory/RemoteLoadLibrary: " << MAX_PATH << ", GetLastError=" << GetLastError() << std::endl; 
			b = GetLastError();

		VirtualFreeEx( hProcess, pLibRemote, 0, MEM_RELEASE );

		CloseHandle( hProcess );

		return bRes;
	}

public:
	std::list <CInjectableProcess> m_lProcesses;

private:

	bool RemoteLoadLibrary( HANDLE hProcess, LPVOID pLibRemote, DWORD dwDelay = 10000 )
	{
		// load dll into the remote process via CreateRemoteThread & LoadLibrary
		HANDLE hThread = CreateRemoteThread(	hProcess,
												NULL,
												0,    
												(LPTHREAD_START_ROUTINE) GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "LoadLibraryA" ), 
												pLibRemote,
												0,
												NULL );

		if ( !hThread ) return false;

		WaitForSingleObject( hThread, dwDelay );

		// check exit code
		DWORD  lpExitCode = 0;
		GetExitCodeThread( hThread, &lpExitCode );

		CloseHandle( hThread );

		return ( lpExitCode != 0 );
	};

	BOOL SetPrivilege(	HANDLE hToken, LPCTSTR lpPrivilege, BOOL bEnablePrivilege )
	{
		// Initializing variables
		TOKEN_PRIVILEGES	tkp			= {0};
		LUID				luid		= {0};
		TOKEN_PRIVILEGES	tkpPrevious = {0};
		DWORD				cbPrevious  = 0;

		// Check the parameters passed to the function
		if ( !hToken || !lpPrivilege ) return FALSE;

		if ( !LookupPrivilegeValue( NULL, lpPrivilege, &luid ) ) return FALSE;

		tkp.PrivilegeCount			 = 1;
		tkp.Privileges[0].Luid		 = luid;
		tkp.Privileges[0].Attributes = 0;

		cbPrevious = sizeof( TOKEN_PRIVILEGES );
		
		if ( AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof( TOKEN_PRIVILEGES ), &tkpPrevious, &cbPrevious ) != ERROR_SUCCESS )
		{
			int e = GetLastError();
			e = e + 1;
			return FALSE;
		}
		
		tkpPrevious.PrivilegeCount		= 1;
		tkpPrevious.Privileges[0].Luid  = luid;

		if ( bEnablePrivilege )
			tkpPrevious.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
		else
			tkpPrevious.Privileges[0].Attributes ^= ( SE_PRIVILEGE_ENABLED & tkpPrevious.Privileges[0].Attributes );

		if ( AdjustTokenPrivileges( hToken, FALSE, &tkpPrevious, cbPrevious, NULL, NULL ) != ERROR_SUCCESS )
		{
			int e = GetLastError();
			e = e + 1;
			return FALSE;
		}

		return TRUE;

		//TOKEN_PRIVILEGES pTokenPrivileges;
		//LUID luid;

		//if( !LookupPrivilegeValue( NULL, lpPrivilege, &luid ) ) return FALSE;

		//pTokenPrivileges.PrivilegeCount = 1;
		//pTokenPrivileges.Privileges[ 0 ].Luid = luid;

		//if( bEnablePrivilege )
		//{	
		//	pTokenPrivileges.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
		//}
		//else 
		//{	
		//	pTokenPrivileges.Privileges[ 0 ].Attributes = 0;
		//}

		//if( !AdjustTokenPrivileges( hToken, FALSE, &pTokenPrivileges, sizeof( TOKEN_PRIVILEGES ), NULL, NULL ) ) return FALSE;

		//if( GetLastError( ) == ERROR_NOT_ALL_ASSIGNED ) return FALSE;

		//return TRUE;
	}

	BOOL SetDebugPrivilege( BOOL bEnable )
	{
		HANDLE hToken = NULL;

		if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) ) // | TOKEN_QUERY
			return FALSE;

		// Enable/Disable Debug Privilege
		if ( !SetPrivilege( hToken, SE_DEBUG_NAME, bEnable ) )
		{
			CloseHandle( hToken );
			return FALSE;
		}

		CloseHandle( hToken );

		return TRUE;
	}

	BOOL IsUserAdmin(VOID)
	/*++ 
	Routine Description: This routine returns TRUE if the caller's
	process is a member of the Administrators local group. Caller is NOT
	expected to be impersonating anyone and is expected to be able to
	open its own process and process token. 
	Arguments: None. 
	Return Value: 
	   TRUE - Caller has Administrators local group. 
	   FALSE - Caller does not have Administrators local group. --
	*/ 
	{
		BOOL b;
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;

		b = AllocateAndInitializeSid(	&NtAuthority,
										2,
										SECURITY_BUILTIN_DOMAIN_RID,
										DOMAIN_ALIAS_RID_ADMINS,
										0, 0, 0, 0, 0, 0,
										&AdministratorsGroup); 

		if(b) 
		{
			if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
			{
				 b = FALSE;
			} 
			FreeSid(AdministratorsGroup); 
		}

		return(b);
	}


private:
	DWORD	ProcessIds[1024];
	tString	m_tHookDll;
};