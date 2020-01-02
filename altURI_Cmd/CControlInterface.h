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
//	control_iface.h - a control class to control a simulated robot
//=========================================================================

#pragma once 

#include <vector>
#include "CIniFile.h"
#include "opencv/highgui.h"
#include "CSock.h"
#include "..\altURI_Hook\CThread.h"
#include "CEMD.h"

typedef IplImage* (__cdecl* pfGetIplImage)( const int width, const int height, const int x, const int y );

//======

struct Control_param_NEW
{
	std::string	m_sSubkey;
	float		m_fMin;
	float		m_fMax;
	float		m_fIncrement;
	float		m_fCurrent;
};	

typedef std::vector<Control_param_NEW> vparams;

struct Control_command_NEW
{
	std::string	m_sKey;
	vparams		m_vParams; 
};

typedef std::vector<Control_command_NEW> vcommands;

//======

struct Control_command
{
	tString	m_sKey;
	float	m_fMin;
	float	m_fMax;
	float	m_fIncrement;
	float	m_fCurrent;
};

struct Message_spec
{
	tString	m_sMessageName;		// value from ini file, name of usarsim message e.g. STA
	tString	m_sKeyName;			// value from ini file e.g. Battery
	int		m_iMessageNumber;	// value from ini - messageN
	int		m_iIndex;			// index of the float value e.g. Location has 0, 1 and 2
};

struct Message_value
{
	int		m_iMessageNumber;	// as above
	int		m_iIndex;			// as above
	float	m_fTime;
	float	m_fValue;
};

class CControlInterface
{
public:
	CControlInterface() { InitializeCriticalSection(&m_Linescs); InitializeCriticalSection(&m_Sensorcs); m_SocketObject.UseAnotherLock( m_Linescs ); m_SocketObject.UseAnotherLock2( m_Sensorcs ); };
	~CControlInterface(){ cvReleaseImage(&m_ImagePrevData); CloseControl(); CloseImages(); DeleteCriticalSection(&m_Linescs); DeleteCriticalSection(&m_Sensorcs); };

	bool OpenControl( const tString& sIP, const tString& sPort );
	void CloseControl( const bool bRestart = false );

	bool OpenHTMLImages( const tString& sIP, const tString& sPort );
	bool OpenDLLImages( void );
	void CloseImages( void );

	bool IsConnected( void ) { return m_SocketObject.m_socClient.IsConnected(); };
	bool IsConnectedImages( void ) { return m_SocketObjectImage.m_socClient.IsConnected(); };

	void SetCatchMessages ( const bool bSet ) { m_SocketObject.m_bCatchMessages = bSet; };
	void SetCatchLines ( const bool bSet ) { m_SocketObject.m_bCatchLines = bSet; };
	void SetCatchMessagesLines ( const bool bSet ) { m_SocketObject.m_bCatchMessageLines = bSet; };

	std::vector<Message_spec>& GetMessageSpecs( void ) { return m_SocketObject.m_vMessageSpecs; };
	std::vector<Message_value>& GetMessageValues( void ) { return m_SocketObject.m_vMessageValues; };

	bool IsVectorControlIndex( std::vector<Control_command> const& vVector, const int iIndex );

	int GetNoMessageSpecs( void ) { return m_SocketObject.m_vMessageSpecs.size(); };
	int GetNoMessageValues( void ) { return m_SocketObject.m_vMessageValues.size(); };
	int GetNoLinesReceived( void ) { return m_SocketObject.m_vLinesReceived.size(); };

	bool SendCommand( const std::string& sLine ) { return m_SocketObject.m_socClient.SendLine( sLine ); };

	IplImage* GetIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	

	tString GetLines( void );

public:
	vcommands						m_vCommandsNEW;

	std::vector<Control_command>	m_vDrives;
	tString							m_sDrivePrefix;
	std::vector<Control_command>	m_vCommands;
	tString							m_sCommandPrefix;
	bool							m_bNormalized;
	CRITICAL_SECTION				m_Linescs;
	CRITICAL_SECTION				m_Sensorcs;
	IplImage*						m_iplImageSent;
	pfGetIplImage					m_pfGetIplImage;
	CEMD							m_EMD;

private:
	class CURISocket : public CThread
	{
	public:
		CURISocket() { };
		virtual ~CURISocket() { SetStop(); m_socClient.Close(); m_vLinesReceived.clear(); };

		virtual void OnDataReceived( const std::string& sLine ) = 0;
		virtual unsigned int ThreadProc();

		bool ConnectClient( const bool bUseThread ){	m_vLinesReceived.clear();
								if ( m_socClient.OpenTCPClient( m_sIP, m_sPort ) )
									return ( bUseThread ? Start() : true );
								else
									return false; };

	public:
		CSock						m_socClient;
		tString						m_sIP;
		tString						m_sPort;
		std::vector<tString>		m_vLinesReceived;
	};

	class CCmdSocket : public CURISocket
	{
		// Operations
	public:
		CCmdSocket() { };
		virtual ~CCmdSocket() { m_vMessageSpecs.clear(); m_vMessageValues.clear(); };
		virtual void OnDataReceived( const std::string& sLine );
		inline void UseAnotherLock2( const CRITICAL_SECTION& cs ) { m_cs = cs; }
		inline void Lock2() { EnterCriticalSection( &m_cs ); };
		inline void Unlock2() { LeaveCriticalSection( &m_cs ); };

	public:
		//std::map<std::string, Message_spec>	m_mMessageSpecs;
		std::vector<Message_spec>			m_vMessageSpecs;
		std::vector<Message_value>			m_vMessageValues;
		bool								m_bCatchMessages;
		bool								m_bCatchLines;
		bool								m_bCatchMessageLines;

	private:
		float GetTime( const std::string& sLine );
		float GetValue( const std::string& sLine, const std::string& sKey, const int index );

		CRITICAL_SECTION					m_cs;

		static const std::string			ITEM_START;
		static const std::string			ITEM_END;
		static const std::string			ITEM_START_TIME;
	};

	class CImageSocket : public CURISocket
	{
	public:
		CImageSocket() { };
		virtual ~CImageSocket() {};
		virtual void OnDataReceived( const std::string& sLine ) { sLine;};

		bool SendGetHTTPImage( const std::string& sArgs ){ return m_socClient.SendLine( HTTP_GET_IMAGE_TEXT + sArgs ); };

		static const std::string HTTP_GET_IMAGE_TEXT;
		static const std::string HTTP_RESPONSE_OK;
		static const std::string HTTP_CONTENTLENGTH_TEXT;
		static const std::string HTTP_GMT_TEXT;

	private:
		std::string sIn;
	};

private:

	void AllocateImageMats( const int width, const int height, const int orgcode, const int newcode);
	void CalculateAndStoreHPFnLPF( size_t stSourceAddress, size_t stDestAddress );

	IplImage* GetHTMLIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	
	IplImage* GetLocalIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	
	bool IsValidIP4( const tString& ip );
	bool ConnectImagesIfNot() { return m_SocketObjectImage.ConnectClient( false ); };
	std::string MakeHTMLArgs( const int width, const int height, const int x, const int y );

	static const std::string	HTTP_WIDTH_ARG;
	static const std::string	HTTP_HEIGHT_ARG;
	static const std::string	HTTP_X_ARG;
	static const std::string	HTTP_Y_ARG;
	static const std::string	HTTP_QUESTIONMARK_ARG;
	static const std::string	HTTP_AMPERSAND_ARG;
	static const std::string	CRLF;

private:
	CCmdSocket		m_SocketObject;
	CImageSocket	m_SocketObjectImage;
	HINSTANCE		m_altHookDLL;
	bool			m_bLocalImage;

	IplImage*		m_ImagePrevData;

	// new version
	IplImage*		m_ImageTemp;
	IplImage*		m_ImagePrev;
	IplImage*		m_ImageHPFPrev;
	IplImage*		m_ImageEMDPrev;

	cv::Range		m_rcolShiftX;
	cv::Range		m_rcolNormalX;
	cv::Range		m_rrowShiftY;
	cv::Range		m_rrowNormalY;
	cv::Range		m_rAll;

	cv::Mat			m_matImage;
	cv::Mat			m_matGrayInitImage;
	cv::Mat			m_matGrayImage;
	cv::Mat			m_matGrayImagePrev;
	cv::Mat			m_matHPF;
	cv::Mat			m_matLPF;
	cv::Mat			m_matHPFPrev;
	cv::Mat			m_matEMDX;
	cv::Mat			m_matEMDY;
	cv::Mat			m_matEMDXYSquared;
	cv::Mat			m_matEMD;
	cv::Mat			m_matEMDPrev;
	cv::Mat			m_matEMDRGB;
	cv::Mat			m_matEMDRGB_ORG;
	IplImage		m_ImageEMD;

	size_t			m_stSourceStep;
	size_t			m_stSourceElementSize;
	size_t			m_stDestStep;
	size_t			m_stDestElementSize;
};