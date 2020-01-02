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
//	CServerManager.cpp - a tcp server class
//=========================================================================

#include "stdafx.h"
#include "CServerManager.h"



//=========================================================================
void CServerManager :: StopServer(  )
{
	SetStop();				// stop the server thread

	// stop and remove socket connections
	std::vector <CSocketManager*>::iterator Iter = m_vCSocketManagers.begin();

	while( Iter != m_vCSocketManagers.end() )
	{
		delete *Iter;
		Iter = m_vCSocketManagers.erase( Iter );
	}

	m_iPort = 0;			// forget port
}
//=========================================================================
bool CServerManager :: StartServer( const int iPort )
{
	m_iPort = iPort;	// save the port number

	return Start();		// start the server thread
}

//=========================================================================
unsigned int CServerManager :: ThreadProc()
{
	m_in.OpenTCPServer( m_iPort );

	for (;;)
	{
		if ( GetStop() ) break;

		m_vCSocketManagers.push_back( new CSocketManager( m_in.ServerAccept() ) );

		if ( GetStop() ) break;

		m_vCSocketManagers.back()->Start();

		//OutputDebugString(TEXT("Added new connection\n"));

		// remove stopped connections
		std::vector <CSocketManager*>::iterator Iter = m_vCSocketManagers.begin();

		while( Iter != m_vCSocketManagers.end() )
		{
			if ( !(*Iter)->IsStarted() )
			{
				delete *Iter;
				Iter = m_vCSocketManagers.erase( Iter );
				OutputDebugString(TEXT("Deleted connection\n"));
			}
			else
				Iter++;
		}

	}

	return 0;
}	

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

// some HTTP fixed text
const std::string CSocketManager :: HTTP_HTML_CONTENTTYPE	= "text/html"; 
const std::string CSocketManager :: HTTP_JPEG_CONTENTTYPE	= "image/jpeg"; 
const std::string CSocketManager :: HTTP_TIFF_CONTENTTYPE	= "image/tiff"; 
const std::string CSocketManager :: HTTP_BMP_CONTENTTYPE	= "image/bmp";
const std::string CSocketManager :: HTTP_PNG_CONTENTTYPE	= "image/png";
const std::string CSocketManager :: HTTP_VIDEO_CONTENTTYPE	= "multipart/x-mixed-replace;boundary=boundarydonotcross";

const std::string CSocketManager :: HTML_FILE_NOT_FOUND		= "<HTML><HEAD><TITLE>The file cannot be found</TITLE><HEAD><BODY><br><br><br><h1>altURI Image Server<br><br>HTTP Error 404 - Image not found.</h1></BODY></HTML>";
const std::string CSocketManager :: HTTP_CRLF				= "\r\n";
const std::string CSocketManager :: HTTP_RESPONSE_OK		= "HTTP/1.0 200 OK";
const std::string CSocketManager :: HTTP_RESPONSE_BAD		= "HTTP/1.0 404 Not Found";
const std::string CSocketManager :: HTTP_SERVER				= "Server: altURI/1.0";
const std::string CSocketManager :: HTTP_CONTENTTYPE_TEXT	= "Content-Type: ";
const std::string CSocketManager :: HTTP_ACCEPTRANGE_TEXT	= "Accept-Ranges: none";
const std::string CSocketManager :: HTTP_CONTENTLENGTH_TEXT	= "Content-Length: ";
const std::string CSocketManager :: HTTP_PRAGMANOCACHE_TEXT	= "Cache-Control: no-cache";
const std::string CSocketManager :: HTTP_CLOSE_TEXT			= "Connection: close";
const std::string CSocketManager :: HTTP_KEEPALIVE_TEXT		= "Connection: Keep-Alive";
const std::string CSocketManager :: HTTP_LAST_MODIFIED_TEXT	= "Last-Modified: ";
const std::string CSocketManager :: HTTP_DATE_TEXT			= "Date: ";
const std::string CSocketManager :: HTTP_GMT_TEXT			= " GMT";

const std::string CSocketManager :: HTTP_BOUNDARY_MARKER	= "--boundarydonotcross";

const std::string CSocketManager :: HTML_PAGE_TEXT = 
"<html><head><title>altURI Image</title><script>function get_arg(name){var params = location.search.substring(1).split('&');for(var i=0;i<params.length;i++){pair = params[i].split('=');if ( pair[0] == name ) return pair[1];} return 0;}var t=get_arg('r');function Start(){tmp=new Date();document.images[0].src='i.png'+location.search+'&t='+tmp.getTime();if(t>0) setTimeout('Start()', t*1000);}</script></head><body><img src=''><script>Start();</script></body></html>";

const double CSocketManager :: MAX_REGION_SIZE = 0.9;

//=========================================================================
// split string by delimter
//=========================================================================
template<typename _Cont>
void split( const std::string& str, _Cont& _container, const std::string& delim="," )
{
    std::string::size_type lpos = 0;
    std::string::size_type pos = str.find_first_of( delim, lpos );
    while( lpos != std::string::npos )
    {
        _container.insert( _container.end(), str.substr( lpos, pos - lpos ) );

        lpos = ( pos == std::string::npos ) ?  std::string::npos : pos + 1;
        pos = str.find_first_of( delim, lpos );
    }
}

//=========================================================================
inline void CSocketManager :: LoadHTTPSearchArguments( const std::string& sArgs )
{
	std::vector<std::string> m_vLineTokens;
	std::string sUppercaseArgs( sArgs.length(), ' ' );

	// uppercase all
	std::transform( sArgs.begin(), sArgs.end(), sUppercaseArgs.begin(), toupper);

	// load vector with arguments delimited by & (without ?)
	split( sUppercaseArgs.substr( 1, sUppercaseArgs.length() - 1 ), m_vLineTokens, "&" );

	// load argument for each recognised character argument
	for( std::vector<std::string>::iterator m_it = m_vLineTokens.begin(); m_it != m_vLineTokens.end(); ++m_it )
	{ 
		if ( (*m_it)[1] == '=' && IsValidNumber( (*m_it).substr( 2, (*m_it).length() ) ) )
			switch( (*m_it)[0] )
			{
				case HTTP_HEIGHT_PARAM:
					std::istringstream ( (*m_it).substr( 2, (*m_it).length() ) ) >> m_OutHeight;
					// don't allow image stretching ???
					if ( (UINT)m_OutHeight > g_DllState.m_SourceImageStats.m_height )
						m_OutHeight = g_DllState.m_SourceImageStats.m_height;
					break;

				case HTTP_WIDTH_PARAM:
					std::istringstream ( (*m_it).substr( 2, (*m_it).length() ) ) >> m_OutWidth;
					// don't allow image stretching ???
					if ( (UINT)m_OutWidth > g_DllState.m_SourceImageStats.m_width )
						m_OutWidth = g_DllState.m_SourceImageStats.m_width;
					break;

				case HTTP_X_PARAM:
					std::istringstream ( (*m_it).substr( 2, (*m_it).length() ) ) >> m_OutXOffset;
					//if ( (UINT)m_OutXOffset > ( g_DllState.m_SourceImageStats.m_width * MAX_REGION_SIZE ) )
					//	m_OutXOffset = int( g_DllState.m_SourceImageStats.m_width * MAX_REGION_SIZE );
					break;

				case HTTP_Y_PARAM:
					std::istringstream ( (*m_it).substr( 2, (*m_it).length() ) ) >> m_OutYOffset;
					//if ( (UINT)m_OutYOffset > ( g_DllState.m_SourceImageStats.m_height * MAX_REGION_SIZE ) )
					//	m_OutYOffset = int( g_DllState.m_SourceImageStats.m_height * MAX_REGION_SIZE );
					break;

				case HTTP_QUALITY_PARAM:
					std::istringstream ( (*m_it).substr( 2, (*m_it).length() ) ) >> m_OutCompression;
					// don't allow compression outside range
					if ( m_OutCompression > m_MaxOutCompression )
						m_OutCompression = m_MaxOutCompression;
					if ( m_OutCompression < m_MinOutCompression )
						m_OutCompression = m_MinOutCompression;
					break;
			}
	}
}

//=========================================================================
std::string CSocketManager :: GetHTTPDateTime( void )
{
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = gmtime ( &rawtime );
	std::string sTime( asctime( timeinfo ) );	// appends LF

	sTime.erase( sTime.length() - 1 );			// remove LF
	
	return sTime;
}

//=========================================================================
void CSocketManager :: SendHTTPHeader( const std::string& sResponse, const std::string& sContentType, const int iContentLength )
{
	std::string sTime( GetHTTPDateTime() );
	std::stringstream out;

	out << iContentLength;

	m_socS->SendLine(	sResponse + HTTP_CRLF +
				HTTP_CONTENTTYPE_TEXT + sContentType + HTTP_CRLF +
				HTTP_CONTENTLENGTH_TEXT + out.str() + HTTP_CRLF +
				HTTP_ACCEPTRANGE_TEXT + HTTP_CRLF +
				HTTP_SERVER + HTTP_CRLF +
				HTTP_PRAGMANOCACHE_TEXT + HTTP_CRLF +
				//( m_bKeepAlive ? HTTP_KEEPALIVE_TEXT : HTTP_CLOSE_TEXT ) + HTTP_CRLF +
				HTTP_LAST_MODIFIED_TEXT + sTime + HTTP_CRLF +
				HTTP_DATE_TEXT + sTime + HTTP_GMT_TEXT + HTTP_CRLF );		// adds another CRLF
	
	UpdateLastActionTimer( m_bKeepAlive );
}

//=========================================================================
bool CSocketManager :: SendHTTPStreamHeader( const std::string& sResponse )
{
	std::string sTime( GetHTTPDateTime() );

	bool temp = m_socS->SendLine(	sResponse + HTTP_CRLF +
				HTTP_CONTENTTYPE_TEXT + HTTP_VIDEO_CONTENTTYPE + HTTP_CRLF +
				HTTP_SERVER + HTTP_CRLF +
				HTTP_PRAGMANOCACHE_TEXT + HTTP_CRLF +
				( m_bKeepAlive ? HTTP_KEEPALIVE_TEXT : HTTP_CLOSE_TEXT ) + HTTP_CRLF +
				HTTP_LAST_MODIFIED_TEXT + sTime + HTTP_CRLF +
				HTTP_DATE_TEXT + sTime + HTTP_GMT_TEXT + HTTP_CRLF + HTTP_CRLF + HTTP_BOUNDARY_MARKER );

	UpdateLastActionTimer( m_bKeepAlive );
	return temp;
}

//=========================================================================
bool CSocketManager :: SendHTTPStreamIntermediateHeader( const std::string& sContentType, const int iContentLength )
{
	std::string sTime( GetHTTPDateTime() );
	std::stringstream out;
	out << iContentLength;
	bool temp = m_socS->SendLine( HTTP_CONTENTTYPE_TEXT + sContentType + HTTP_CRLF + HTTP_CONTENTLENGTH_TEXT + out.str() + HTTP_CRLF );		// adds another CRLF
	UpdateLastActionTimer( m_bKeepAlive );
	return temp;
}

//=========================================================================
bool CSocketManager :: SendHTTPStreamImageOnly( const std::string& sContentType )
{
	int iImageLength = m_buf->step * m_buf->rows;
	bool temp = SendHTTPStreamIntermediateHeader( sContentType, iImageLength );
	if (temp)
		temp = m_socS->SendBytes( (char*)m_buf->data.ptr, iImageLength );
	if (temp)
		temp = SendHTTPStreamIntermediateFooter();
	UpdateLastActionTimer( m_bKeepAlive );
	return temp;
}

//=========================================================================
bool CSocketManager :: SendHTTPStreamIntermediateFooter( void )
{
	std::string sTime( GetHTTPDateTime() );
	bool temp = m_socS->SendLine( HTTP_CRLF + HTTP_BOUNDARY_MARKER);		// adds another CRLF
	UpdateLastActionTimer( m_bKeepAlive );
	return temp;
}
//=========================================================================
void CSocketManager :: SendHTMLFileNotFoundPage( void )
{
	SendHTTPHeader( HTTP_RESPONSE_BAD, HTTP_HTML_CONTENTTYPE, HTML_FILE_NOT_FOUND.length() );
	m_socS->SendLine( HTML_FILE_NOT_FOUND );
	UpdateLastActionTimer( m_bKeepAlive );
}

//=========================================================================
void CSocketManager :: SendHTTPImageOnly( const std::string& sContentType )
{
	int iImageLength = m_buf->step * m_buf->rows;
	SendHTTPHeader( HTTP_RESPONSE_OK, sContentType, iImageLength );
	m_socS->SendBytes( (char*)m_buf->data.ptr, iImageLength );
	UpdateLastActionTimer( m_bKeepAlive );
}

//=========================================================================
void CSocketManager :: SendHTMLImagePage( void )
{
	SendHTTPHeader( HTTP_RESPONSE_OK, HTTP_HTML_CONTENTTYPE, HTML_PAGE_TEXT.length() );
	m_socS->SendLine( HTML_PAGE_TEXT );
	UpdateLastActionTimer( m_bKeepAlive );
}

//=========================================================================
void CSocketManager :: PrepareImage( IplImage* iplImage, const std::string& sExt )
{
	m_iCompressionParams[0] = m_OutCompressionParam;
	m_iCompressionParams[1] = m_OutCompression;
	m_iCompressionParams[2] = 0;

	// Prepare image to send
	m_buf = cvEncodeImage( sExt.c_str(), iplImage, m_iCompressionParams );
}

//=========================================================================
void CSocketManager :: OnDataReceived( const std::string& sLine )
{
	// eg. http://127.0.0.1/image.jpg?h=300&w=400&x=100&y=200&q=50 or any combination of

	std::string m_sLine( sLine );

	if ( ( m_sLine.compare( 0, 4, "GET " ) == 0 ) || ( m_sLine.compare( 0, 4, "get " ) == 0 ) )
	{
		m_bKeepAlive =  ( m_sLine.find( HTTP_CLOSE_TEXT, 5 ) == std::string::npos );
		
		UpdateLastActionTimer( m_bKeepAlive );

		// find second space - end of url
		std::string::size_type m_stPos = m_sLine.find( " ", 5 );

		// extract url if second space
		if ( m_stPos != std::string::npos )
			m_sLine = m_sLine.substr( 5, m_stPos - 5 );

		// assume png whatever
		std::string m_sContentExt = HTTP_PNG_CONTENTTYPE;
		std::string m_sContentExtTest = ".png";
		m_MinOutCompression = 0;		// =none
		m_MaxOutCompression = 9;
		m_OutCompression = 4;
		m_OutCompressionParam = CV_IMWRITE_PNG_COMPRESSION;

		// find start of extension
		m_stPos = m_sLine.find( "." );

		if ( m_stPos != std::string::npos )
		{
			m_sContentExtTest = m_sLine.substr( m_stPos, 4 );

			std::transform( m_sContentExtTest.begin(), m_sContentExtTest.end(), m_sContentExtTest.begin(), tolower);

			if ( m_sContentExtTest.compare( ".htm" ) == 0 )
				m_sContentExt = HTTP_HTML_CONTENTTYPE;
			else if ( m_sContentExtTest.compare( ".jpg" ) == 0 )
			{
				m_sContentExt = HTTP_JPEG_CONTENTTYPE;
				m_MinOutCompression = 0;	// max is size not compression
				m_MaxOutCompression = 100;
				m_OutCompression = 30;		// i.e. no compression at all
				m_OutCompressionParam = CV_IMWRITE_JPEG_QUALITY;
			}
			else if ( m_sContentExtTest.compare( ".tif" ) == 0 )
			{
			 	m_sContentExt = HTTP_TIFF_CONTENTTYPE;
				m_OutCompressionParam = 0;
			}
			else if ( m_sContentExtTest.compare( ".bmp" ) == 0 )
			{
			 	m_sContentExt = HTTP_BMP_CONTENTTYPE;
				m_OutCompressionParam = 0;
			}
			else if ( m_sLine.length() > 5 && m_sLine.substr( 0, 6 ).compare( "video." ) == 0 )
			{
				m_sContentExt = HTTP_VIDEO_CONTENTTYPE;
				m_MinOutCompression = 0;	// max is size not compression
				m_MaxOutCompression = 100;
				m_OutCompression = 30;		// i.e. no compression at all
				m_OutCompressionParam = CV_IMWRITE_JPEG_QUALITY;
			}
			else	// none of these so set back to png
			{
				m_sContentExtTest = ".png";
			}
		}

		// if html send image html
		if ( m_sContentExt.compare( HTTP_HTML_CONTENTTYPE ) == 0 )
			SendHTMLImagePage();
		else
		{
			// initialise values
			m_OutHeight			= 0;
			m_OutWidth			= 0;
			m_OutXOffset		= 0;
			m_OutYOffset		= 0;

			// any parameters in url?
			m_stPos = m_sLine.find( "?" );

			// get params if any
			if ( m_stPos != std::string::npos )
				LoadHTTPSearchArguments( m_sLine.substr( m_stPos, m_sLine.length() ) );

			bool bVideo = ( m_sContentExt.compare( HTTP_VIDEO_CONTENTTYPE ) == 0 );

			if ( bVideo )
			{
				SendHTTPStreamHeader( HTTP_RESPONSE_OK );
				m_sContentExt = HTTP_JPEG_CONTENTTYPE;
				m_sContentExtTest = ".jpg";
			}

			while ( true )
			{
				g_MyImage.Lock();
			
				IplImage* iplImage = GetIplImage( m_OutWidth, m_OutHeight, m_OutXOffset, m_OutYOffset );
				
				if ( CV_IS_IMAGE( iplImage ) )
				{
					PrepareImage( iplImage, m_sContentExtTest );	
					g_MyImage.Unlock();
					if ( bVideo )
						bVideo = SendHTTPStreamImageOnly( m_sContentExt );
					else
						SendHTTPImageOnly( m_sContentExt ) ;
					cvReleaseMat( &m_buf );			// release buffer
				}
				else
				{
					g_MyImage.Unlock();
					SendHTMLFileNotFoundPage();		// if no image send error 404
				}

				if ( !bVideo ) return;
			}
		}
	}
}

//=========================================================================
unsigned int CSocketManager :: ThreadProc()
{
	UpdateLastActionTimer( true );

	while ( !TimedOut() )
	{
		OnDataReceived( m_socS->ReceiveLine() );
		if ( GetStop() ) break;
	}

	return 0;
}
