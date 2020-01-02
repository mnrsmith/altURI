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
//	CServerManager.h - a tcp server class
//=========================================================================

#pragma once 

#include <vector>
#include <ctype.h>
#include <ctime>

#include "..\altURI_Cmd\CSock.h"
#include "CThread.h"


//=========================================================================
class CSocketManager : public CThread
{
public:
	CSocketManager( CSock* socS )  : m_socS( socS ) {};

	virtual ~CSocketManager() {	SetStop(); };

	void OnDataReceived( const std::string& sLine );

	inline bool TimedOut() { return ( clock() - m_ctLastAction ) > ( KEEP_ALIVE_TIMEOUT * CLOCKS_PER_SEC );  }; 
	
	virtual unsigned int ThreadProc();

private:
	inline void UpdateLastActionTimer( const bool& bKeepingAlive ) { if ( bKeepingAlive ) { m_ctLastAction = clock(); } else m_ctLastAction = 0; };

	inline bool IsValidNumber( const std::string& s )	{	for ( std::string::size_type i = 0; i < s.length(); i++ )
														if ( !isdigit( s[i] ) ) return false;
													return true; };

	inline void LoadHTTPSearchArguments( const std::string& sArgs );
	void SendHTTPHeader( const std::string& sResponse, const std::string& sContentType, const int iContentLength );

	bool SendHTTPStreamHeader( const std::string& sResponse);
	bool SendHTTPStreamIntermediateHeader( const std::string& sContentType, const int iContentLength );
	bool SendHTTPStreamImageOnly( const std::string& sContentType );
	bool SendHTTPStreamIntermediateFooter( void);

	void SendHTMLFileNotFoundPage( void );
	void SendHTTPImageOnly( const std::string& sContentType );
	void SendHTMLImagePage( void );

	std::string GetHTTPDateTime( void );

	void PrepareImage( IplImage* iplImageInitial, const std::string& sExt );

private:
	CSock*								m_socS;
	bool								m_bKeepAlive;
	clock_t								m_ctLastAction;
	CvMat*								m_buf;

	int									m_OutHeight;
	int									m_OutWidth;
	int									m_OutXOffset;
	int									m_OutYOffset;
	int									m_OutCompression;
	int									m_MaxOutCompression;
	int									m_MinOutCompression;
	int									m_OutCompressionParam;
	int									m_iCompressionParams[3];
	UINT								m_uLastEvent;

private:
	static const int					KEEP_ALIVE_TIMEOUT	= 60;	// seconds
	static const char					HTTP_HEIGHT_PARAM	= 'H';
	static const char					HTTP_WIDTH_PARAM	= 'W';
	static const char					HTTP_X_PARAM		= 'X';
	static const char					HTTP_Y_PARAM		= 'Y';
	static const char					HTTP_QUALITY_PARAM	= 'Q';
	static const double					MAX_REGION_SIZE;

	static const std::string			HTML_FILE_NOT_FOUND;

	static const std::string			HTTP_JPEG_CONTENTTYPE;
	static const std::string			HTTP_HTML_CONTENTTYPE;
	static const std::string			HTTP_TIFF_CONTENTTYPE;
	static const std::string			HTTP_BMP_CONTENTTYPE;
	static const std::string			HTTP_PNG_CONTENTTYPE;
	static const std::string			HTTP_VIDEO_CONTENTTYPE;

	static const std::string			HTTP_CRLF;

	static const std::string			HTTP_RESPONSE_OK;
	static const std::string			HTTP_RESPONSE_BAD;
	static const std::string			HTTP_SERVER;
	static const std::string			HTTP_CONTENTTYPE_TEXT;
	static const std::string			HTTP_ACCEPTRANGE_TEXT;
	static const std::string			HTTP_CONTENTLENGTH_TEXT;
	static const std::string			HTTP_PRAGMANOCACHE_TEXT;
	static const std::string			HTTP_LAST_MODIFIED_TEXT;
	static const std::string			HTTP_CLOSE_TEXT;
	static const std::string			HTTP_KEEPALIVE_TEXT;
	static const std::string			HTTP_DATE_TEXT;
	static const std::string			HTTP_GMT_TEXT;
	static const std::string			HTML_PAGE_TEXT;
	static const std::string			HTTP_BOUNDARY_MARKER;
};


//=========================================================================
class CServerManager : public CThread
{
public :
	CServerManager() { };
	virtual ~CServerManager() { StopServer(); };

	bool StartServer( const int iPort );
	void StopServer();

	virtual unsigned int ThreadProc();

private:
	CSock							m_in;
	int								m_iPort;

	std::vector<CSocketManager*>	m_vCSocketManagers;
};

IplImage* GetResizedIplCopy( IplImage* iplIn, const int width, const int height, const int x, const int y );