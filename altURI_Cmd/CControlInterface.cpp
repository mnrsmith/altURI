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
//	control_iface.cpp - a control class to control a simulated robot
//=========================================================================

#include "stdafx.h"
#include "CControlInterface.h"
#include "XTrace.h"

extern HMODULE hCurrentModule;


//=========================================================================
tString CControlInterface :: GetLines( void )
{
	tString r = TEXT("");

	EnterCriticalSection( &m_Linescs );
	{
		// unload lines from vector
		std::vector <tString>::iterator Iter = m_SocketObject.m_vLinesReceived.begin();

		while( Iter != m_SocketObject.m_vLinesReceived.end() )
		{
			r += *Iter;
			Iter = m_SocketObject.m_vLinesReceived.erase( Iter );
		}
	}
	LeaveCriticalSection( &m_Linescs );

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
bool CControlInterface :: OpenControl( const tString& sIP, const tString& sPort )
{
	if ( !IsValidIP4( sIP ) ) return false;

	m_SocketObject.m_sIP = sIP;
	m_SocketObject.m_sPort = sPort;

	// reset everything - but not drives or commands
	CloseControl( true );

	return m_SocketObject.ConnectClient( true );
}

//=========================================================================
void CControlInterface :: CloseControl( const bool bRestart )
{
	SetCatchMessages ( false );
	SetCatchLines ( false );
	SetCatchMessagesLines ( false );

	m_SocketObject.SetStop();
	m_SocketObject.m_socClient.Close();

	m_SocketObject.m_vMessageValues.clear();
	m_SocketObject.m_vLinesReceived.clear();

	if ( bRestart ) return;

	m_vDrives.clear();
	m_vCommands.clear();
}

//=========================================================================
bool CControlInterface :: OpenHTMLImages( const tString& sIP, const tString& sPort )
{
	if ( !IsValidIP4( sIP ) ) return false;

	m_SocketObjectImage.m_sIP = sIP;
	m_SocketObjectImage.m_sPort = sPort;

	CloseImages();

	bool bRes = m_SocketObjectImage.ConnectClient( false );

	m_bLocalImage = false;

	return bRes;
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
bool CControlInterface :: OpenDLLImages( void )
{

#if defined _M_X64
	tString sDll = ExtractDllPath() + TEXT("altURI_Hook_x64.dll");
#else
	tString sDll = ExtractDllPath() + TEXT("altURI_Hook.dll");
#endif

	m_pfGetIplImage = NULL;

	m_altHookDLL = LoadLibrary( sDll.c_str() );

	if ( m_altHookDLL ) m_pfGetIplImage = (pfGetIplImage)GetProcAddress( m_altHookDLL, "GetIplImage" );

	m_bLocalImage = true;

	return ( m_pfGetIplImage != NULL );
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
std::string CControlInterface :: MakeHTMLArgs( const int width, const int height, const int x, const int y )
{
	std::string sRet = "";
	std::string sSeparator = HTTP_QUESTIONMARK_ARG;
	std::ostringstream osResult;

	if ( width > 0 )
	{
		osResult << width;
		sRet = sSeparator + HTTP_WIDTH_ARG + osResult.str();
		sSeparator = HTTP_AMPERSAND_ARG;	// for next time (if any)
		osResult.flush();
	}

	if ( height > 0 )
	{
		osResult << height;
		sRet += sSeparator + HTTP_HEIGHT_ARG + osResult.str();
		sSeparator = HTTP_AMPERSAND_ARG;	// for next time (if any)
		osResult.flush();
	}

	if ( x > 0 )
	{
		osResult << x;
		sRet += sSeparator + HTTP_X_ARG + osResult.str();
		sSeparator = HTTP_AMPERSAND_ARG;	// for next time (if any)
		osResult.flush();
	}

	if ( y > 0 )
	{
		osResult << y;
		sRet += sSeparator + HTTP_Y_ARG + osResult.str();
	}

	return sRet;
}

//=========================================================================
IplImage* CControlInterface :: GetIplImage( const int width, const int height, const int x, const int y )
{
	if ( m_bLocalImage )
		return GetLocalIplImage( width, height, x, y );
	else
		return GetHTMLIplImage( width, height, x, y );
}


//=========================================================================
IplImage* CControlInterface :: GetLocalIplImage( const int width, const int height, const int x, const int y )
{
	return m_pfGetIplImage( width, height, x, y );
}

//=========================================================================
IplImage* CControlInterface :: GetHTMLIplImage( const int width, const int height, const int x, const int y )
{
	std::string sIn;
	std::string sImage;
	std::string sHTMLArgs = "";
	int iImageLength = 0;

	// if we are using some size parameters make appropriate html search arguments
	if ( width + height + x + y > 0 )
		sHTMLArgs = MakeHTMLArgs( width, height, x, y );

	// try to send http request
	if ( m_SocketObjectImage.SendGetHTTPImage( sHTMLArgs ) )
	{
		// get first bytes
		std::string sTemp = m_SocketObjectImage.m_socClient.ReceiveBytes();

		// get more until no more
		while ( sTemp.size() > 0 )
		{
			sIn += sTemp;
			sTemp = m_SocketObjectImage.m_socClient.ReceiveBytes();
		}

		// find http content length
		std::string::size_type m_stPos = sIn.find( m_SocketObjectImage.HTTP_CONTENTLENGTH_TEXT );

		// if content length found
		if ( m_stPos != std::string::npos )
		{
			// make a string containing the length
			std::string sImageLength = sIn.substr( m_stPos + m_SocketObjectImage.HTTP_CONTENTLENGTH_TEXT.length() , sIn.length() );

			m_stPos = sImageLength.find( CRLF );

			sImageLength = sImageLength.substr( 0, m_stPos );

			// make an integer
			if ( IsValidNumber( sImageLength ) ) std::istringstream ( sImageLength ) >> iImageLength;

			// is some length
			if ( iImageLength > 0 )
			{
				// look for end of http header
				m_stPos = sIn.find( m_SocketObjectImage.HTTP_GMT_TEXT );

				// if header found and string is at least iImageLength
				if (	( m_stPos != std::string::npos ) &&
						( sIn.size() >= m_stPos + m_SocketObjectImage.HTTP_GMT_TEXT.size() + iImageLength ) )
				{
					// make a new string
					sImage.assign( sIn.substr( m_stPos + m_SocketObjectImage.HTTP_GMT_TEXT.size(), iImageLength ) );
					// make a mat and decode it to an iplimage
					CvMat cvmImageSent = cvMat(1, iImageLength, CV_8UC3, (void*)sImage.data() );
					if ( m_iplImageSent ) cvReleaseImage( &m_iplImageSent );
					m_iplImageSent = cvDecodeImage( &cvmImageSent );

				}
			}
		}
	}
	else
	{
		// try to connect again
		ConnectImagesIfNot();
		return NULL;
	}

	return m_iplImageSent;
}

//=========================================================================
void CControlInterface :: CloseImages( void )
{
	if ( m_bLocalImage )
	{
		if ( m_altHookDLL ) FreeLibrary( m_altHookDLL );
	}
	else
	{
		m_SocketObjectImage.m_socClient.Close();
		m_SocketObject.m_vLinesReceived.clear();
	}

	if ( m_iplImageSent ) cvReleaseImage( &m_iplImageSent );
}


//=========================================================================
const std::string CControlInterface :: HTTP_WIDTH_ARG = "w=";
const std::string CControlInterface :: HTTP_HEIGHT_ARG = "h=";
const std::string CControlInterface :: HTTP_X_ARG = "x=";
const std::string CControlInterface :: HTTP_Y_ARG = "y=";
const std::string CControlInterface :: HTTP_QUESTIONMARK_ARG = "?";
const std::string CControlInterface :: HTTP_AMPERSAND_ARG = "&";
const std::string CControlInterface :: CRLF = "\r\n";

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
float CControlInterface :: CCmdSocket :: GetTime( const std::string& sLine )
{
	std::string::size_type iStartText =  sLine.find( ITEM_START_TIME ) + ITEM_START_TIME.length() + 1;

	std::string::size_type iEndText =  sLine.find( ITEM_END, iStartText ) - 1;

	std::string sTime = sLine.substr( iStartText, iEndText );

	return static_cast<float>( atof( sTime.c_str() ) );
}

//=========================================================================
float CControlInterface :: CCmdSocket :: GetValue( const std::string& sLine,const std::string& sKey, const int index )
{
	std::string sStartText = ITEM_START + sKey + TEXT(" ");

	std::string::size_type iStartText =  sLine.find( sStartText ) + sStartText.size();

	std::string::size_type iEndText =  sLine.find( ITEM_END, iStartText ) - 1;

	std::string sText = sLine.substr( iStartText, iEndText );

	std::vector<std::string> tokens;

	split( sText, tokens );

	return static_cast<float>( atof( tokens[index].c_str() ) );
}

//=========================================================================
void CControlInterface :: CCmdSocket :: OnDataReceived( const std::string& sLine )
{
	// if not processing replies or zero length reply then exit
	if ( !( m_bCatchLines || m_bCatchMessages ) ) return;
	
	bool bMessageLine = false;
	
	if ( m_bCatchMessages )
	{
		// look for all the messages specified in the ini file
		for ( std::vector<Message_spec>::const_iterator it = m_vMessageSpecs.begin(); it != m_vMessageSpecs.end(); ++it )
		{
			// if line begins with message name and contains the key
			if ( sLine.find( it->m_sMessageName ) == 0 &&  sLine.find( it->m_sKeyName ) != std::string::npos )
			{
				Message_value value;
				value.m_iMessageNumber = it->m_iMessageNumber;
				value.m_iIndex = it->m_iIndex;
				value.m_fTime = GetTime( sLine );
				value.m_fValue = GetValue( sLine, it->m_sKeyName, it->m_iIndex );
				Lock2();
				m_vMessageValues.push_back( value );
				Unlock2();
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
const std::string CControlInterface :: CCmdSocket :: ITEM_START		= " {";
const std::string CControlInterface :: CCmdSocket :: ITEM_END		= "} ";
const std::string CControlInterface :: CCmdSocket :: ITEM_START_TIME	= ITEM_START + "Time ";


//=========================================================================
unsigned int CControlInterface :: CURISocket :: ThreadProc()
{
	std::string sIn;

	for (;;)
	{
		if ( GetStop() ) break;
		
		sIn = m_socClient.ReceiveLine();

		if ( !sIn.empty() ) OnDataReceived( sIn );
	}

	return 0;
}

//=========================================================================
const std::string CControlInterface :: CImageSocket :: HTTP_GET_IMAGE_TEXT		= "GET /i.png HTTP/1.1";
const std::string CControlInterface :: CImageSocket :: HTTP_RESPONSE_OK			= "HTTP/1.1 200 OK" + CRLF;
const std::string CControlInterface :: CImageSocket :: HTTP_CONTENTLENGTH_TEXT	= "Content-Length: ";
const std::string CControlInterface :: CImageSocket :: HTTP_GMT_TEXT			= " GMT" + CRLF + CRLF;


