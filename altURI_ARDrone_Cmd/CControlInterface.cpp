/*
Copyright (c) 2012, Mark N R Smith, All rights reserved.

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
//	control_iface.cpp - a control class to control the Parrot ARDrone
//=========================================================================

#include "stdafx.h"
#include "CControlInterface.h"
#include "..\altURI_Cmd\XTrace.h"

extern HMODULE hCurrentModule;


//=========================================================================
tString CControlInterface :: GetLines( void )
{
	tString r = TEXT("");

	EnterCriticalSection( &m_Sensorcs );
	{
		// unload lines from vector
		std::vector <tString>::iterator Iter = m_SocketObjectSensor.m_vLinesReceived.begin();

		while( Iter != m_SocketObjectSensor.m_vLinesReceived.end() )
		{
			r += *Iter;
			Iter = m_SocketObjectSensor.m_vLinesReceived.erase( Iter );
		}
	}
	LeaveCriticalSection( &m_Sensorcs );

	return r;
}

//=========================================================================
bool CControlInterface :: IsValidIP4( const tString& ip )
{ 
	unsigned b1, b2, b3, b4;
	unsigned char c;

	if ( sscanf( ip.c_str(), "%3u.%3u.%3u.%3u%c", &b1, &b2, &b3, &b4, &c) != 4 )
		return false;

	if ( (b1 | b2 | b3 | b4) > 255 ) return false;

	if ( strspn( ip.c_str(), "0123456789.") < strlen( ip.c_str() ) ) return false;

	return true;
}

//=========================================================================
bool CControlInterface :: IsVectorControlIndex( std::vector<Control_command> const& vVector, const int iIndex )
{
	return ( iIndex > -1 && iIndex < (int)vVector.size() );
}

//=========================================================================
bool CControlInterface :: OpenControl( const tString& sIP, const tString& sCommandPort, const tString& sSensorPort )
{
	if ( !IsValidIP4( sIP ) ) return false;

	m_SocketObjectCmd.m_sIP = sIP;
	m_SocketObjectCmd.m_sPort = sCommandPort;

	m_SocketObjectSensor.m_sIP = sIP;
	m_SocketObjectSensor.m_sPort = sSensorPort;

	// reset everything - but not drives or commands
	CloseControl( true );

	if ( m_SocketObjectCmd.ConnectUDPClient( true )  )
	{
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:session_id\",\"0ce000d1\"" );
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:application_id\",\"0ce000d2\"" );
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:profile_id\",\"0ce000d3\"" );
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:session_desc\",\"Session 1\"" );
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:application_desc\",\"altURI_ARDrone_Cmd\"" );
		//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"custom:profile_desc\",\"altURIv2\"" );

		// m_SocketObjectCmd.AddCommand( "AT*CONFIG_IDS=%d,\"0ce000d1\",\"0ce000d3\",\"0ce000d2\"" );

		m_SocketObjectCmd.SendDefault();	// AT*COMWDG=1

		// Send emergency shutdown/reset command
		m_SocketObjectCmd.AddCommand( "AT*REF=%d,290717952" );

		m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"general:navdata_demo\",\"TRUE\"" );
		m_SocketObjectCmd.AddCommand( "AT*CTRL=%d,5,0" );

		m_SocketObjectCmd.AddCommand( "AT*FTRIM=%d" );

		m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"CONTROL:altitude_max\",\"1500\"" );
		m_SocketObjectCmd.AddCommand( "AT*CTRL=%d,5,0" );
		
		// Send emergency shutdown/reset command
		//m_SocketObjectCmd.AddCommand( "AT*REF=%d,290717952" );

		if ( m_SocketObjectSensor.ConnectUDPClient( true ) )
		{

			// take off command - twice
			//m_SocketObjectCmd.AddCommand( "AT*REF=%d,290718208" );

			//m_SocketObjectCmd.AddCommand( "AT*REF=%d,290718208" );


			//m_SocketObjectCmd.AddCommand( "AT*CONFIG=&d,\"video:video_codec\",\"64\"" );

			// enable adaptive video ???
			//m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"video:bitrate_ctrl_mode\",\"1\"" );

			m_SocketObjectCmd.AddCommand( "AT*LED=%d,2,1056964608,3" );
	

			return true;
		}
		else
			return false;
	}
	else
		return false;
}

//=========================================================================
void CControlInterface :: CloseControl( const bool bRestart )
{
	SetCatchMessages ( false );
	SetCatchLines ( false );
	SetCatchMessagesLines ( false );

	m_SocketObjectCmd.SetStop();
	m_SocketObjectCmd.m_socClient.Close();
	m_SocketObjectCmd.m_vsCommandQueue.clear();

	m_SocketObjectSensor.SetStop();
	m_SocketObjectSensor.m_socClient.Close();
	
	m_SocketObjectSensor.m_vMessageValues.clear();
	m_SocketObjectSensor.m_vLinesReceived.clear();

	if ( bRestart ) return;

	m_vDrives.clear();
	m_vCommands.clear();
}

//=========================================================================
bool CControlInterface :: OpenDroneImages( const tString& sIP, const tString& sPort )
{
	if ( !IsValidIP4( sIP ) ) return false;

	m_SocketObjectImage.m_sIP = sIP;
	m_SocketObjectImage.m_sPort = sPort;

	CloseImages();

	//m_SocketObjectCmd.AddCommand( "AT*CONFIG_IDS=%d,\"0ce000d1\",\"0ce000d3\",\"0ce000d2\"" );
	m_SocketObjectCmd.AddCommand( "AT*CONFIG=%d,\"general:video_enable\",\"TRUE\"" );

	// reset ack bit (after every AT*CONFIG command)
	m_SocketObjectCmd.AddCommand( "AT*CTRL=%d,5,0" );
	
	return m_SocketObjectImage.ConnectUDPClient( true );
}
//=========================================================================
const tString ExtractDllPath()
{ 
	TCHAR szPath[MAX_PATH];

	GetModuleFileName( hCurrentModule, szPath, MAX_PATH );

	tString tsPath( szPath );
	tString tsDrive;
	tsDrive.resize(_MAX_DRIVE);
	tString tsFolder;
	tsFolder.resize(_MAX_DIR);
	_splitpath( (TCHAR*)tsPath.c_str(), (TCHAR*)tsDrive.c_str(), (TCHAR*)tsFolder.c_str(), NULL, NULL );

	tString tsDllPath;

	// cope with unc shares
	if ( tsDrive.at(0) != 0 )
		tsDllPath = tsDrive.substr(0, 2) + tsFolder;
	else
		tsDllPath = tsFolder;

	tsDllPath.resize( tsDllPath.find_last_of( TEXT("\\") ) + 1 );

	return tsDllPath;
}

//=========================================================================
inline bool IsValidNumber( const std::string& s )
{	
	for ( std::string::size_type i = 0; i < s.length(); i++ )
		if ( !isdigit( s[i] ) )
			return false;

	return true;
}

//=========================================================================
IplImage* CControlInterface :: GetIplImage( const int width, const int height, const int x, const int y )
{
	return m_SocketObjectImage.m_CARDroneImageDecoder.GetIplImage( width, height, x, y );
}

//=========================================================================
void CControlInterface :: CloseImages( void )
{
	//m_SocketObjectCmd.AddCommand( "AT*CONFIG_IDS=%d,\"0ce000d1\",\"0ce000d3\",\"0ce000d2\"" );
	//m_SocketObjectCmd.AddCommand( "AT*CONFIG=&d,\"general:video_enable\",\"FALSE\"" );

	// reset ack bit (after every AT*CONFIG command)
	// m_SocketObjectCmd.AddCommand( "AT*CTRL=%d,5,0" );

	m_SocketObjectImage.SetStop();
	m_SocketObjectImage.m_socClient.Close();
}

//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
// split string by delimiter
//=========================================================================
template<typename _Cont>
void split(const std::string& str, _Cont& _container, const std::string& delim=",")
{
    std::string::size_type lpos = 0;
    std::string::size_type pos = str.find_first_of( delim, lpos );
    while( lpos != tString::npos )
    {
        _container.insert( _container.end(), str.substr( lpos, pos - lpos ) );

        lpos = ( pos == tString::npos ) ?  tString::npos : pos + 1;
        pos = str.find_first_of( delim, lpos );
    }
}

//=========================================================================
void CControlInterface :: CSensorSocket :: OnDataReceived( const std::string& sLine )
{
	int iOffset = 0;

	// check for navdata header 0x55667788
	if ( makeIntFromBytes( sLine.data(), iOffset ) != 0x55667788 ) return;

	iOffset = 4;

	int iState = makeIntFromBytes( sLine.data(), iOffset );

	m_ARDroneState.SetState( iState );

	// if not processing replies or zero length reply then exit
	if ( !( m_bCatchLines || m_bCatchMessages ) ) return;
	
	bool bMessageLine = false;
	
	if ( m_bCatchMessages )
	{
		// look for all the messages specified in the ini file
		for ( std::vector<Message_spec>::const_iterator it = m_vMessageSpecs.begin(); it != m_vMessageSpecs.end(); ++it )
		{
			// if line begins with message name and contains the key
			if ( sLine.find( it->m_sMessageName ) == 0 && sLine.find( it->m_sKeyName ) != std::string::npos )
			{
				Message_value value;
				value.m_iMessageNumber = it->m_iMessageNumber;
				value.m_iIndex = it->m_iIndex;
				//value.m_fTime = GetTime( sLine );
				//value.m_fValue = GetValue( sLine, it->m_sKeyName, it->m_iIndex );
				//Lock2();
				m_vMessageValues.push_back( value );
				//Unlock2();
				bMessageLine = true;
			}
		}
	}

	// catch lines if required but ignore message lines if not requested
	if ( m_bCatchLines )
		if ( !( !m_bCatchMessageLines && bMessageLine ) )
		{
			Lock();
			m_vLinesReceived.push_back( sLine );
			Unlock();
		}

}

//=========================================================================
unsigned int CControlInterface :: CCmdSocket :: ThreadProc()
{
	std::string sOut;
	std::vector <std::string>::iterator Iter;
	int iKeepAliveCounter = 0;

	for (;;)
	{
		if ( GetStop() ) break;

		if ( m_vsCommandQueue.empty() && iKeepAliveCounter > m_iKEEP_ALIVE_DELAY ) AddCommand( m_DefaultSend );
		
		Lock();
		Iter = m_vsCommandQueue.begin();

		if ( Iter != m_vsCommandQueue.end() )
		{
			sOut = *Iter;
			m_vsCommandQueue.erase( Iter );		// repeat commands?
			Unlock();
			SendWithSequenceNumber( sOut );
			iKeepAliveCounter = 0;
		}
		else
			Unlock();

		if ( GetStop() ) break;

		iKeepAliveCounter += m_iCOMMAND_DELAY;

		Sleep( m_iCOMMAND_DELAY );
	}

	return 0;
}

//=========================================================================
unsigned int CControlInterface :: CImageSocket :: ThreadProc()
{
	std::string sTemp;

	SendDefault();

	for (;;)
	{
		if ( GetStop() ) break;

		sTemp = m_socClient.ReceiveBytes();

		if ( GetStop() ) break;

		if ( !sTemp.empty() )
		{
			OnDataReceived( sTemp );
		}
	}

	return 0;
}

//=========================================================================
unsigned int CControlInterface :: CSensorSocket :: ThreadProc()
{
	std::string sTemp;

	SendDefault();

	for (;;)
	{
		if ( GetStop() ) break;

		sTemp = m_socClient.ReceiveBytes();

		if ( GetStop() ) break;

		if ( !sTemp.empty() ) 
		{
				OnDataReceived( sTemp );
		}

	}

	return 0;
}



