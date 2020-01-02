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
//
// altURI_LoadDLL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\altURI_ui\CDllInject.h"

CDllInject myInjection;
DWORD dwProcessID = 0;

const tString DOT_EXE = TEXT( ".exe" );
const tString DOT_DLL = TEXT( ".dll" );
const tString SLASHES = TEXT( "\\" );

//==================================================================================================
tString ToLower(const tString& sString)
{
	tString sReturned( sString );
	std::transform( sReturned.begin(), sReturned.end(), sReturned.begin(), tolower );
	return sReturned;
}

//==================================================================================================
//tString ExtractAppPath( const tString& sDrivePathFileName )
//{ 
//	tString tsDrive;
//	tsDrive.resize( _MAX_DRIVE );
//	tString tsFolder;
//	tsFolder.resize( _MAX_DIR );
//	SPLITPATH( (TCHAR*)sDrivePathFileName.c_str(), (TCHAR*)tsDrive.c_str(), (TCHAR*)tsFolder.c_str(), NULL, NULL );
//
//	tString tsAppPath( tsDrive.substr(0, 2) + tsFolder );
//	tsAppPath.resize( tsAppPath.find_last_of( TEXT("\\") ) + 1 );
//
//	return tsAppPath;
//}

//==================================================================================================
tString GetPath( const tString& sExeNamePlusParams )
{
	tString sLowercaseExeNamePlusParams( ToLower( sExeNamePlusParams ) );
	tString sReturned = sExeNamePlusParams.substr( 0, sLowercaseExeNamePlusParams.find( DOT_EXE ) );
	sReturned = sReturned.substr( 0, sReturned.rfind( SLASHES ) == -1 ? 0 : sReturned.rfind( SLASHES ) );
	return sReturned;
}

//==================================================================================================
tString GetParams( const tString& sExeNamePlusParams )
{
	tString sLowercaseExeNamePlusParams( ToLower( sExeNamePlusParams ) );
	tString sReturned = sExeNamePlusParams.substr( sLowercaseExeNamePlusParams.find( DOT_EXE ) + 4 );
	return sReturned;
}

//==================================================================================================
tString GetExeParams( const tString& sExeNamePlusParams )
{
	tString sLowercaseExeNamePlusParams( ToLower( sExeNamePlusParams ) );
	tString sReturned = sExeNamePlusParams.substr( 0, sLowercaseExeNamePlusParams.find( DOT_EXE ) + 5 );
	return sReturned;
}
 
//==================================================================================================
tString GetExe( const tString& sExeNamePlusParams )
{
	tString sExePath = GetPath( sExeNamePlusParams );
	tString sTemp = sExeNamePlusParams.substr( sExePath.length() + 1 );
	tString sLowercaseTemp( ToLower( sTemp ) );
	tString sReturned = sTemp.substr( 0, sLowercaseTemp.find( DOT_EXE ) + 4 );
	return sReturned;
}

//==================================================================================================
tString GetDllName( const tString& sDllPathName )
{
	tString sDllPath = GetPath( sDllPathName );
	tString sTemp = sDllPathName.substr( sDllPath.length() + 1 );
	tString sLowercaseTemp( ToLower( sTemp ) );
	tString sReturned = sTemp.substr( 0, sLowercaseTemp.find( DOT_DLL ) + 4 );
	return sReturned;
}

//==================================================================================================
bool fileExists( const tString& sFileName )
{
	return ( GetFileAttributes( sFileName.c_str() ) != 0xFFFFFFFF );
}

//==================================================================================================
inline bool IsValidNumber( const tString& s )
{
	for ( tString::size_type i = 0; i < s.length(); i++ )
		if ( !isdigit( s[i] ) ) return false;
	return true;
};

//==================================================================================================
void ShowHelp()
{
      ConOut << "\n\naltURI_LoadDLL - start a program with an added dll\n\nCopyright (c) 2010, Mark N R Smith" << std::endl; 
      ConOut << "\n\nUsage: altURI_LoadDLL.exe dll_path+file program_comand_line delay_seconds(optional)" << std::endl; 
}

//==================================================================================================
//==================================================================================================
//==================================================================================================
int _tmain(int argc, _TCHAR* argv[])
{
	// do we have two or three command line args?
	if ( !( argc == 3 || argc == 4 ) )
	{
		ShowHelp();
		exit(1);
	}

	int iDelay;
	
	tString sDllFile( argv[1] );
	tString sHostCommand( argv[2] );
	tString tDelay;

	// get value or set default 15 seconds
	if ( argc == 4 )
		tDelay = argv[3];
	else
		tDelay = TEXT( "15" );

	tString sHostPathExe = GetExeParams( sHostCommand );
	tString sHostParams = GetParams( sHostCommand );
	tString sHostPath = GetPath( sHostCommand );
	tString sHostExe = GetExe( sHostCommand );
	tString sDllName = ToLower( GetDllName( sDllFile ) );

	// check files are there
	if ( !fileExists( sDllFile ) )
	{
		ConOut << "\n\nERROR: Cannot find .dll file: " << sDllFile << std::endl; 
		ShowHelp();
		exit(1);
	}

	if ( !fileExists( sHostPathExe ) )
    {
		ConOut << "\n\nERROR: Cannot find .exe file: " << sHostCommand << std::endl; 
		ShowHelp();
        exit(1);
    }

	// convert delay parameter
	if ( IsValidNumber( tDelay ) )
		iDelay = TOI( tDelay.c_str() ) * 1000;
	else
	{
		ConOut << "\n\nERROR: Value for delay (seconds) not recognised as a number: " << tDelay << std::endl; 
		ShowHelp();
        exit(1);
	}

	if ( !myInjection.LaunchSimulator( sHostPath, sHostPathExe, sHostParams ) )
	{
		ConOut << "\n\nERROR: Unable to launch: " << sHostCommand << ", GetLastError=" << GetLastError() << std::endl; 
        exit(1);
	}

	// wait for steam or whatever
	Sleep( iDelay );

	myInjection.SetHookDllNameToExclude( sDllName );

	// get list of processes that should contain the loaded .exe
	if ( myInjection.FillProcesses() )
	{
		std::list <CInjectableProcess>::iterator Iter = myInjection.m_lProcesses.begin();

		// look through list for .exe we loaded
		while( Iter != myInjection.m_lProcesses.end() )
		{
			if ( ToLower( Iter->m_sProcessName ).compare( ToLower( sHostExe ) ) == 0 )
			{
				dwProcessID = Iter->m_dwProcessID;
				break;
			}
			++Iter;
		}
		
		// if .exe process id found try to inject into it
		if ( dwProcessID != 0 &&  myInjection.DoInject( dwProcessID, sDllFile ) )		// right name ???
			ConOut << "\n\nSUCCESS: " << sDllName << " loaded into " << sHostExe << std::endl; 
		else
		{
			ConOut << "\n\nERROR: Unable to load DLL: " << sDllFile << std::endl; 
			exit(1);
		}
	}

	return 0;
}

