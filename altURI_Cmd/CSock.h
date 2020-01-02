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
// CSock.h : a socket class
//

#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")

#include <string>

//=========================================================================
class CSock
{
public:
	CSock( SOCKET s = INVALID_SOCKET ) : m_s( s ), m_iCurrentReceiveBlockSize( DEFAULT_RECEIVE_BLOCK_SIZE ), m_buf( NULL ) { SetLineTerminator( CSock::CRLF ); if ( !m_iInstanceCounter ) { WSADATA ws; WSAStartup( 0x0202, &ws ); } m_iInstanceCounter++; SetReceiveBlockSize( m_iCurrentReceiveBlockSize ); };

	virtual ~CSock() { if ( IsConnected() ) Close(); m_iInstanceCounter--; if ( m_iInstanceCounter < 1 ) WSACleanup(); delete[] m_buf; };

	bool OpenTCPClient( const std::string& sHost, const std::string& sPort ){ return OpenClient( sHost, sPort, IPPROTO_TCP ); };

	bool OpenUDPClient( const std::string& sHost, const std::string& sPort ){ return OpenClient( sHost, sPort, IPPROTO_UDP ); };
	
	bool OpenTCPServer( const int iPort );
	
	CSock* ServerAccept();
	
	std::string ReceiveLine();

	std::string ReceiveBytes();

	bool SendBytes( const char* buf, const int iLength);

	bool SendLine( const std::string& sData );

	bool IsWriteable();

	bool IsReadable();

	void SetReceiveBlockSize( const int iNewSize ){ char* old = m_buf;
													m_iCurrentReceiveBlockSize = iNewSize;
													m_buf = new char[ m_iCurrentReceiveBlockSize ];
													delete[] old;
													m_sRet.reserve( m_iCurrentReceiveBlockSize );
												};

	void Close(){ shutdown( m_s, SD_BOTH ); closesocket( m_s ); m_s = INVALID_SOCKET; m_iProtocol = -1; };

	//bool IsConnected() { return ( m_s != INVALID_SOCKET && IsWriteable() && IsReadable() ); } ;
	bool IsConnected() { return ( m_s != INVALID_SOCKET ); } ;

	void SetLineTerminator( const std::string& sNew ) { assert ( sNew.compare(CRLF) == 0 || sNew.compare(CR) || sNew.compare(LF) );  m_sEndOfLine = sNew; }

public:
	static const std::string CRLF;
	static const std::string CR;
	static const std::string LF;

private:
	// prevent copy
	CSock( const CSock& );
	CSock& operator = ( const CSock& );

	bool OpenClient( const std::string& sHost, const std::string& sPort, const int iProtocol );

	bool SetSectionOptions( const bool bReuse );

private:
	static int m_iInstanceCounter;
	static const int DEFAULT_RECEIVE_BLOCK_SIZE = 16484;

	SOCKET		m_s;
	int			m_iCurrentReceiveBlockSize;
	char*		m_buf;
	std::string m_sRet;
	int			m_iProtocol;						// TCP or UDP
	std::string	m_sEndOfLine;						// LF or CRLF
};


