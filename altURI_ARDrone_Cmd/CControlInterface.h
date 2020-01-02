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
//	control_iface.h - a control class to control a simulated robot
//=========================================================================

#pragma once 

#include <vector>
#include <bitset>
#include "..\altURI_Cmd\CIniFile.h"
#include "opencv/highgui.h"
#include "..\altURI_Cmd\CSock.h"
#include "..\altURI_Hook\CThread.h"
#include "..\altURI_Cmd\CEMD.h"
#include "CARDroneImageDecoder.h"
#include "..\altURI_Cmd\XTrace.h"

typedef IplImage* (__cdecl* pfGetIplImage)( const int width, const int height, const int x, const int y );

//=======

class ARDrone_state
{
public:
	ARDrone_state() : m_lastState( 0xFFFFFFFF ) {};

private:
	bool bFlying;
	bool bVideoEnabled;
	bool bVisionEnabled;
	bool bControlAlgorithm;
	bool bAltitudeControlActive;
	bool bUserFeedbackOn;
	bool bControlReceived;
	bool bTrimReceived ;
	bool bTrimRunning;
	bool bTrimSucceeded;
	bool bNavDataDemoOnly;
	bool bNavDataBootstrap;
	bool bMotorsDown;
	bool bGyrometersDown;
	bool bBatteryTooLow;
	bool bBatteryTooHigh;
	bool bTimerElapsed;
	bool bNotEnoughPower;
	bool bAnglesOutOfRange;
	bool bTooMuchWind;
	bool bUltrasonicSensorDeaf;
	bool bCutoutSystemDetected;
	bool bPICVersionNumberOK;
	bool bATCodedThreadOn;
	bool bNavDataThreadOn;
	bool bVideoThreadOn;
	bool bAcquisitionThreadOn;
	bool bControlWatchdogDelayed;
	bool bADCWatchdogDelayed;
	bool bCommunicationWatchdogProblem;
	bool bEmergency;

public:
	void SetState( const int iState ) {	if ( iState != m_lastState )	// only set if changed
										{
											std::bitset<sizeof( iState ) * 8> bsBits( iState );
											for ( int i = 0; i < 32; i++ )
												SetSingleState( i, bsBits.test( i ) );
											m_lastState = iState;
											//std::stringstream ss;
											//ss << bsBits << " ";
											//TRACE( TEXT( "State (MSB->LSB) = %s\n" ), ss.str().c_str() );
											TRACE( TEXT( "State: %s\n" ), GetString().c_str() );
										}	
									};

private:
	std::string GetString( void )
	{
		char cTemp[1024];

		sprintf( cTemp, "Flying=%i,Video=%i,Vision=%i,CAlg=%i,AltC=%i,UserFB=%i,ACK=%i,TrimRec=%i,TrimRun=%i,TrimOK=%i,NavDemo=%i,NavBoot=%i,MotorBad=%i,GyroBad=%i,BatLow=%i,BatHigh=%i,TimerE=%i,BadPower=%i,AnglesBad=%i,WindBad=%i,UltraBad=%i,Cutout=%i,PICVOK=%i,ATTOn=%i,VidTOn=%i,AcqTOn=%i,CWDGDelay=%i,ADCWDGDelay=%i,CommWDGBad=%i,Emergency=%i",\
								bFlying, bVideoEnabled, bVisionEnabled, bControlAlgorithm, bAltitudeControlActive, bUserFeedbackOn, bControlReceived,\
								bTrimReceived, bTrimRunning, bTrimSucceeded, bNavDataDemoOnly, bNavDataBootstrap, bMotorsDown, bGyrometersDown, \
								bBatteryTooLow, bBatteryTooHigh, bTimerElapsed, bNotEnoughPower, bAnglesOutOfRange, bTooMuchWind, bUltrasonicSensorDeaf,\
								bCutoutSystemDetected, bPICVersionNumberOK, bATCodedThreadOn, bNavDataThreadOn, bVideoThreadOn, bAcquisitionThreadOn,\
								bControlWatchdogDelayed, bADCWatchdogDelayed, bCommunicationWatchdogProblem, bEmergency );

		return std::string( cTemp );
	}
	
	void SetSingleState( const int iBit, const bool bBitValue )
	{
		switch( iBit )
		{
			case 0: bFlying = bBitValue; break;
			case 1: bVideoEnabled = bBitValue; break;
			case 2: bVisionEnabled = bBitValue; break;
			case 3: bControlAlgorithm = bBitValue; break;
			case 4: bAltitudeControlActive = bBitValue; break;
			case 5: bUserFeedbackOn = bBitValue; break;
			case 6: bControlReceived = bBitValue; break;
			case 7: bTrimReceived = bBitValue; break;
			case 8: bTrimRunning = bBitValue; break;
			case 9: bTrimSucceeded = bBitValue; break;
			case 10: bNavDataDemoOnly = bBitValue; break;
			case 11: bNavDataBootstrap = bBitValue; break;
			case 12: bMotorsDown = bBitValue; break;
			case 13: bGyrometersDown = bBitValue;break;
			case 14: break;
			case 15: bBatteryTooLow = bBitValue; break;
			case 16: bBatteryTooHigh = bBitValue; break;
			case 17: bTimerElapsed = bBitValue; break;
			case 18: bNotEnoughPower = bBitValue; break;
			case 19: bAnglesOutOfRange = bBitValue; break;
			case 20: break;
			case 21: bTooMuchWind = bBitValue; break;
			case 22: bUltrasonicSensorDeaf = bBitValue; break;
			case 23: bCutoutSystemDetected = bBitValue; break;
			case 24: bPICVersionNumberOK = bBitValue; break;
			case 25: bATCodedThreadOn = bBitValue; break;
			case 26: bVideoThreadOn = bBitValue; break;
			case 27: bAcquisitionThreadOn = bBitValue; break;
			case 28: bControlWatchdogDelayed = bBitValue; break;
			case 29: bADCWatchdogDelayed = bBitValue; break;
			case 30: bCommunicationWatchdogProblem = bBitValue; break;
			case 31: bEmergency = bBitValue; break;
			default: assert( false );
		}
	}
private:
	int m_lastState;;
};

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
	CControlInterface() {  InitializeCriticalSection(&m_Sensorcs); InitializeCriticalSection(&m_UDPcs); m_SocketObjectSensor.UseAnotherLock( m_Sensorcs ); };
	~CControlInterface(){ /*cvReleaseImage(&m_ImagePrevData);*/ CloseControl(); CloseImages(); DeleteCriticalSection(&m_Sensorcs);  DeleteCriticalSection(&m_UDPcs); };

	bool OpenControl( const tString& sIP, const tString& sCommandPort, const tString& sSensorPort );
	void CloseControl( const bool bRestart = false );

	bool OpenDroneImages( const tString& sIP, const tString& sPort );
	void CloseImages( void );

	bool IsConnected( void ) { return m_SocketObjectCmd.m_socClient.IsConnected(); };
	bool IsConnectedImages( void ) { return m_SocketObjectImage.m_socClient.IsConnected(); };

	void SetCatchMessages ( const bool bSet ) { m_SocketObjectSensor.m_bCatchMessages = bSet; };
	void SetCatchLines ( const bool bSet ) { m_SocketObjectSensor.m_bCatchLines = bSet; };
	void SetCatchMessagesLines ( const bool bSet ) { m_SocketObjectSensor.m_bCatchMessageLines = bSet; };

	std::vector<Message_spec>& GetMessageSpecs( void ) { return m_SocketObjectSensor.m_vMessageSpecs; };
	std::vector<Message_value>& GetMessageValues( void ) { return m_SocketObjectSensor.m_vMessageValues; };

	bool IsVectorControlIndex( std::vector<Control_command> const& vVector, const int iIndex );

	int GetNoMessageSpecs( void ) { return m_SocketObjectSensor.m_vMessageSpecs.size(); };
	int GetNoMessageValues( void ) { return m_SocketObjectSensor.m_vMessageValues.size(); };
	int GetNoLinesReceived( void ) { return m_SocketObjectSensor.m_vLinesReceived.size(); };

	bool SendCommand( const std::string& sLine ) { m_SocketObjectCmd.AddCommand( sLine ); return true; };

	IplImage* GetIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	

	tString GetLines( void );

public:
	vcommands						m_vCommandsNEW;

	std::vector<Control_command>	m_vDrives;
	tString							m_sDrivePrefix;
	std::vector<Control_command>	m_vCommands;
	tString							m_sCommandPrefix;
	bool							m_bNormalized;
	CRITICAL_SECTION				m_Sensorcs;
	CRITICAL_SECTION				m_UDPcs;
	IplImage						m_iplImageSent;
	//pfGetIplImage					m_pfGetIplImage;
	CEMD							m_EMD;

private:
	class CURISocket : public CThread
	{
		friend class CControlInterface;

	public:
		CURISocket() { m_socClient.SetLineTerminator( CSock::CR ); };
		virtual ~CURISocket() { SetStop(); m_socClient.Close(); m_vLinesReceived.clear(); };

		//virtual void OnDataReceived( const std::string& sLine ) = 0;

		bool ConnectUDPClient( const bool bUseThread ){	m_vLinesReceived.clear();
								if ( m_socClient.OpenUDPClient( m_sIP, m_sPort ) )
									return ( bUseThread ? Start() : true );
								else
									return false; };

		//void SendDefault( void ){ m_socClient.SendBytes( m_DefaultSend.data(), m_DefaultSend.length() ); };	// send without LF or null terminator

	public:
		CSock						m_socClient;
		tString						m_sIP;
		tString						m_sPort;
		std::vector<tString>		m_vLinesReceived;

	protected:
		std::string					m_DefaultSend;

	};

	class CSensorSocket : public CURISocket
	{
		friend class CControlInterface; 
		// Operations
	public:
		CSensorSocket() { m_DefaultSend = std::string( "\1\0\0\0", 4 ); };		// unicast, use 2 for multicast
		~CSensorSocket() { m_vMessageSpecs.clear(); m_vMessageValues.clear(); };
		void OnDataReceived( const std::string& sLine );

		unsigned int ThreadProc();

	private:
		void SendDefault( void ){ m_socClient.SendBytes( m_DefaultSend.data(), m_DefaultSend.length() ); };	// send without LF or null terminator

	public:
		//std::map<std::string, Message_spec>	m_mMessageSpecs;
		std::vector<Message_spec>			m_vMessageSpecs;
		std::vector<Message_value>			m_vMessageValues;
		bool								m_bCatchMessages;
		bool								m_bCatchLines;
		bool								m_bCatchMessageLines;
	
	private:
		ARDrone_state						m_ARDroneState;
	};

	class CCmdSocket : public CURISocket
	{
	public:
		CCmdSocket() { m_DefaultSend = "AT*COMWDG=%d"; m_sequence_no = 1; };
		~CCmdSocket() { m_vsCommandQueue.clear(); };

		unsigned int ThreadProc();

		void OnDataReceived( const std::string& sLine ) { sLine;};

		void AddCommand( const std::string& sLine ) {	Lock(); 
														m_vsCommandQueue.push_back( sLine );
														Unlock(); };

		void SendDefault( void ){ SendWithSequenceNumber( m_DefaultSend ); };

	private:
		inline std::string stdFormatCommand( const std::string& sKey, const int Value ){	char* tcTemp = new char[sKey.size()];
																							sprintf( tcTemp, sKey.c_str(), Value );
																							return std::string( tcTemp ); };

		inline void SendWithSequenceNumber( const std::string& sCommand ){	Lock();
																			std::string sOut = stdFormatCommand( sCommand, m_sequence_no++ );
																			//TRACE( TEXT( "Sending: %s\n" ), sOut.c_str() );
																			Unlock();
																			m_socClient.SendLine( sOut ); };

	public:
		std::vector<std::string>		m_vsCommandQueue;
	private:
		std::string						sIn;
		int								m_sequence_no;

		const static int				m_iCOMMAND_DELAY = 30;
		const static int				m_iKEEP_ALIVE_DELAY = 100;
	};

	class CImageSocket : public CURISocket
	{
	public:
		CImageSocket() { m_DefaultSend = std::string( "\1\0\0\0", 4 ); m_socClient.SetReceiveBlockSize( RECEIVE_BLOCK_SIZE ); /* big buffer for image */ };		// m_DefaultSend=unicast, use 2 for multicast
		~CImageSocket() {};
		void OnDataReceived( const std::string& sLine ) { m_CARDroneImageDecoder.decodeImage( (unsigned char*)sLine.data(), sLine.length() ) ; };

		//bool SendGetHTTPImage( const std::string& sArgs ){ return m_socClient.SendLine( HTTP_GET_IMAGE_TEXT + sArgs ); };

		unsigned int ThreadProc();

		CARDroneImageDecoder m_CARDroneImageDecoder;

	private:
		void SendDefault( void ){ m_socClient.SendBytes( m_DefaultSend.data(), m_DefaultSend.length() ); };	// send without LF or null terminator

	private:
		static const int RECEIVE_BLOCK_SIZE = 131072;
	};

private:

	void AllocateImageMats( const int width, const int height, const int orgcode, const int newcode);
	void CalculateAndStoreHPFnLPF( size_t stSourceAddress, size_t stDestAddress );

	IplImage* GetHTMLIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	
	IplImage* GetLocalIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );	
	bool IsValidIP4( const tString& ip );
	bool ConnectImagesIfNot() { return m_SocketObjectImage.ConnectUDPClient( false ); };
	std::string MakeHTMLArgs( const int width, const int height, const int x, const int y );

	static inline unsigned int makeIntFromBytes( const char* buffer, int index ) { return *((unsigned int *)(buffer + index)); };

	inline void UDPLock() { EnterCriticalSection( &m_UDPcs ); };
	inline void UDPUnlock() { LeaveCriticalSection( &m_UDPcs ); };

	//inline void Lock() { EnterCriticalSection( &m_cs ); };
	//inline void Unlock() { LeaveCriticalSection( &m_cs ); };

private:
	CCmdSocket		m_SocketObjectCmd;
	CImageSocket	m_SocketObjectImage;
	CSensorSocket	m_SocketObjectSensor;
	//HINSTANCE		m_altHookDLL;
	//bool			m_bLocalImage;

	//IplImage*		m_ImagePrevData;

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