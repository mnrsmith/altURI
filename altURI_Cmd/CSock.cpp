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
// CSock.cpp : socket class methods
//

#include "stdafx.h"
#include "CSock.h"

const std::string CSock :: CRLF = "\r\n";
const std::string CSock :: CR = "\r";
const std::string CSock :: LF = "\n";

int CSock :: m_iInstanceCounter = 0;

//=========================================================================
bool CSock :: OpenClient( const std::string& sHost, const std::string& sPort, const int iProtocol )
{
	if ( IsConnected() ) Close();	// re-open

	m_iProtocol = iProtocol;

	int iType = ( m_iProtocol == IPPROTO_TCP ? SOCK_STREAM : SOCK_DGRAM );

	m_s = socket( AF_INET, iType, m_iProtocol );

	if ( m_s == INVALID_SOCKET ) return false;

	addrinfo hints, *resultTarget = NULL;

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_socktype = iType;
	hints.ai_family = AF_INET;
	hints.ai_protocol = m_iProtocol;

	if ( getaddrinfo( sHost.c_str(), sPort.c_str(), &hints, &resultTarget ) != 0 )
	{
        Close();
        return false;
	}

	if ( connect( m_s, resultTarget->ai_addr, resultTarget->ai_addrlen ) == SOCKET_ERROR )
	{
		int iError = WSAGetLastError();
		freeaddrinfo( resultTarget );
        Close();
        return false;
	}

	if ( !SetSectionOptions( false ) )
	{
		freeaddrinfo( resultTarget );
		Close();
		return false;
	}

	freeaddrinfo( resultTarget );
	return true;
}

//=========================================================================
bool CSock :: SetSectionOptions( const bool bReuse )
{
	BOOL bOptReuseAddress = TRUE;
	int iOptTimeout = 2000;

	if ( bReuse )
		if ( setsockopt( m_s, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptReuseAddress, sizeof(BOOL) ) == SOCKET_ERROR )
			return false;

	if ( setsockopt( m_s, SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptTimeout, sizeof(int) ) == SOCKET_ERROR )
		return false;

	if ( setsockopt( m_s, SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptTimeout, sizeof(int) ) == SOCKET_ERROR )
		return false;

	if ( setsockopt( m_s, SOL_SOCKET, SO_RCVBUF, (char*)&m_iCurrentReceiveBlockSize, sizeof(int) ) == SOCKET_ERROR )
		return false;

	return true;
}

//=========================================================================
bool CSock :: OpenTCPServer( const int iPort )
{
	if ( IsConnected() ) Close();		// re-open

	m_iProtocol = IPPROTO_TCP;

	m_s = socket( AF_INET, SOCK_STREAM, m_iProtocol );

	if ( m_s == INVALID_SOCKET ) return false;

	sockaddr_in socketAddress;

	memset( &socketAddress, 0, sizeof( socketAddress ) );

	socketAddress.sin_family = PF_INET;             
	socketAddress.sin_port = htons( (u_short)iPort );

	if ( bind( m_s, (sockaddr *)&socketAddress, sizeof( socketAddress ) ) == SOCKET_ERROR )
	{
		Close();
		return false;
	}

	if ( !SetSectionOptions( true ) )
	{
		Close();
		return false;
	}

	return ( listen( m_s, SOMAXCONN ) != SOCKET_ERROR  );
}

//=========================================================================
CSock* CSock :: ServerAccept()
{
	SOCKET new_sock = accept( m_s, 0, 0 );

	if ( new_sock == INVALID_SOCKET ) return NULL;

	return new CSock( new_sock );
}

//=========================================================================
std::string CSock :: ReceiveLine()
{
	char r;

	m_sRet = "";

	for (;;)
	{
		switch ( recv( m_s, &r, 1, 0) )
		{
			case SOCKET_ERROR:
			case 0:
				return m_sRet;
		}

		m_sRet += r;
		if ( r == '\n' )  return m_sRet;
	}
}

//=========================================================================
std::string CSock :: ReceiveBytes()
{
	int iReceived;
	u_long ulArg;

	m_sRet = "";
	
	for (;;) 
	{
		ulArg = 0;

		if ( ioctlsocket( m_s, FIONREAD, &ulArg ) == SOCKET_ERROR || ulArg == 0 ) break;

		if ( ulArg > m_iCurrentReceiveBlockSize ) ulArg = m_iCurrentReceiveBlockSize;

		iReceived = recv( m_s, m_buf, ulArg, 0 );

		if ( iReceived < 1 )
		{
			//int iError = WSAGetLastError();
			//if ( !( iError == WSAECONNRESET && m_iProtocol == IPPROTO_UDP ) ) break;
			break;
		}

		m_sRet += std::string( m_buf, iReceived );
	}

	return m_sRet;
}

//=========================================================================
bool CSock :: SendBytes( const char* buf, const int iLength )
{
	if ( send( m_s, buf, iLength, 0 ) == iLength ) return true;

	//int iError = WSAGetLastError();

	Close();
	return false;
}

//=========================================================================
bool CSock :: SendLine( const std::string& sData )
{
	std::string sLine;

	if ( sData.at( sData.length() - 1 ) == '\0' )
		sLine = sData.substr( 0, sData.size() - 1 );
	else
		sLine = sData.substr( 0, sData.size() );

	sLine += m_sEndOfLine;

	return SendBytes( sLine.c_str(), sLine.length() );
}


