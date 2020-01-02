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
//CaltURI_plus.h : a wrapper class for the robot interface
//

#pragma once 

#include <map>

typedef bool (__cdecl* pfSetupControl)( const char* szIniFile );
typedef bool (__cdecl* pfSendRobot)( const char* szAnyCommand );
typedef bool (__cdecl* pfGetDriveMinValue)( const int iDriveNo );
typedef bool (__cdecl* pfGetDriveMaxValue)( const int iDriveNo );
typedef bool (__cdecl* pfGetDriveIncValue)( const int iDriveNo );
typedef bool (__cdecl* pfGetDriveCurValue)( const int iDriveNo );
typedef bool (__cdecl* pfCommandDrive)( const int iDriveNo, const int iIncrements );
typedef bool (__cdecl* pfCommandDriveAllStop)( void );
typedef bool (__cdecl* pfCommandDriveAll)(	const int iIncrements1,
											const int iIncrements2,
											const int iIncrements3,
											const int iIncrements4,
											const int iIncrements5,
											const int iIncrements6,
											const int iIncrements7,
											const int iIncrements8,
											const int iIncrements9,
											const int iIncrements10,
											const int iIncrements11,
											const int iIncrements12 );
typedef bool (__cdecl* pfCommandOther)( const int iCommandNo, const int iIncrements );
typedef bool (__cdecl* pfSetCatchMessages)( const bool bCatchMessages );
typedef char* (__cdecl* pfGetMessageKey)( const int iMessageNumber );
typedef int (__cdecl* pfGetMessagesArray)( const int iMessagesToGet, double** dArray2d, const int iMessageNumber );
typedef void (__cdecl* pfSetCatchReceivedLines)( const bool bCatchLines, const bool bCatchMessageLines );
typedef char* (__cdecl* pfGetReceivedLine)( void );
typedef void (__cdecl* pfCloseControl)( void );
typedef char* (__cdecl* pfGetClientIP)( void );
typedef bool (__cdecl* pfSetupImages)( const bool bLocal );
typedef IplImage* (__cdecl* pfGetIplImage)( const int width, const int height, const int x, const int y );
typedef void (__cdecl* pfCloseImages)( void );
typedef void (__cdecl* pfIncEMDParameters)( const bool bStartParamsAndLog );
typedef void (__cdecl* pfGetTimeMilliseconds)( void );


class CaURIButton
{
public:
	bool	m_bIsDrive;
	int		m_iIndex;
	int		m_iSign;
	CaURIButton() : m_bIsDrive( true ), m_iIndex( 0 ), m_iSign( 0 ) {};
	CaURIButton( const bool bIsDrive, const int iIndex, const int iSign ) :
	m_bIsDrive( bIsDrive ), m_iIndex( iIndex ), m_iSign( iSign ) {};
};

class CaltURI_plus 
{
public:
	CaltURI_plus(){ m_baltURI=true; m_bLocalImage=true; };
	~CaltURI_plus(){ ClosealtURI(); };

	bool OpenaltURI( const tString& sIniPath, const tString& sHookDllPath, const tString& sCmdDllPath, const bool bLocalImage, const bool bUseCmdDllImage, const bool bVisionOnly );
	//bool OpenaltRRI( const int device_no );							// real robot

	tString ReceiveLine();
	bool SendCommand( tString tCommand );
	bool CommandDriveAllStop( void );
	bool CommandDrive( const int iDriveNo, const int iSign ) { return m_pfCommandDrive( iDriveNo, iSign ); };
	bool CommandOther( const int iCommandNo, const int iSign ) { return m_pfCommandOther( iCommandNo, iSign ); };
	char* GetLocalAddress( void ){ return m_pfGetClientIP(); };

	void _SetCatchReceivedLines( const bool bCatchLines, const bool bCatchMessageLines ) { m_pfSetCatchReceivedLines( bCatchLines, bCatchMessageLines ); };
	void _SetCatchMessages( const bool bCatchMessages ) { m_pfSetCatchMessages( bCatchMessages ); };
	tString _GetMessageKey( const int iMessageNumber ){ tString sTemp( m_pfGetMessageKey( iMessageNumber ) ); return sTemp; };
	int _GetMessageArray( const int iNoOfMessages, double** dArray2d, const int iMessageNumber ){ return m_pfGetMessagesArray( iNoOfMessages, dArray2d, iMessageNumber ); };

	IplImage* GetFrame( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	void SetImageSource( const int iType );

	void IncEMDParams( const bool bStartParamsAndLog );

	CvSize GetSize( void );

	void ClosealtURI( void );

private:
	bool LinkCmdDll( const tString& sCmdDllPath );

private:
	IplImage*				m_IplImage;
	bool					m_baltURI;
	bool					m_bLocalImage;
	HINSTANCE				m_altHookDLL;
	HINSTANCE				m_altCmdDLL;
	tString					m_tsHookDllPath;
	tString					m_tsCmdDllPath;
	
	pfSetupControl			m_pfSetupControl;
	pfSendRobot				m_pfSendRobot;
	pfGetDriveMinValue		m_pfGetDriveMinValue;
	pfGetDriveMaxValue		m_pfGetDriveMaxValue;
	pfGetDriveIncValue		m_pfGetDriveIncValue;
	pfGetDriveCurValue		m_pfGetDriveCurValue;
	pfCommandDrive			m_pfCommandDrive;
	pfCommandDriveAllStop	m_pfCommandDriveAllStop;
	pfCommandDriveAll		m_pfCommandDriveAll;
	pfCommandOther			m_pfCommandOther;
	pfSetCatchMessages		m_pfSetCatchMessages;
	pfGetMessageKey			m_pfGetMessageKey;
	pfGetMessagesArray		m_pfGetMessagesArray;
	pfSetCatchReceivedLines	m_pfSetCatchReceivedLines;
	pfGetReceivedLine		m_pfGetReceivedLine;
	pfCloseControl			m_pfCloseControl;
	pfGetClientIP			m_pfGetClientIP;
	pfSetupImages			m_pfSetupImages;
	pfGetIplImage			m_pfGetIplImage;

	pfGetIplImage			m_pfGetIplImageEMD1Ch;
	pfGetIplImage			m_pfGetIplImageEMD2Ch;
	pfGetIplImage			m_pfGetIplImageEMDRegions;
	pfGetIplImage			m_pfGetIplImageEMD1ChStats;
	pfGetIplImage			m_pfGetIplImageEMD2ChStats;

	pfGetIplImage			m_pfGetIplImageActual;
	pfCloseImages			m_pfCloseImages;
	pfIncEMDParameters		m_pfIncEMDParameters;
	pfGetTimeMilliseconds	m_pfGetTimeMilliseconds;
};
	
