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

#include "stdafx.h"
#include "CaltURI_plus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
bool CaltURI_plus :: LinkCmdDll( const tString& sCmdDllPath )
{
	m_altCmdDLL = LoadLibrary( sCmdDllPath.c_str() );

	if ( m_altCmdDLL )
	{
		m_pfSetupControl			= (pfSetupControl)GetProcAddress( m_altCmdDLL, "SetupControl" );
		m_pfSendRobot				= (pfSendRobot)GetProcAddress( m_altCmdDLL, "SendRobot" );
		m_pfGetDriveMinValue		= (pfGetDriveMinValue)GetProcAddress( m_altCmdDLL, "GetDriveMinValue" );
		m_pfGetDriveMaxValue		= (pfGetDriveMaxValue)GetProcAddress( m_altCmdDLL, "GetDriveMaxValue" );
		m_pfGetDriveIncValue		= (pfGetDriveIncValue)GetProcAddress( m_altCmdDLL, "GetDriveIncValue" );
		m_pfGetDriveCurValue		= (pfGetDriveCurValue)GetProcAddress( m_altCmdDLL, "GetDriveCurValue" );
		m_pfCommandDrive			= (pfCommandDrive)GetProcAddress( m_altCmdDLL, "CommandDrive" );
		m_pfCommandDriveAllStop		= (pfCommandDriveAllStop)GetProcAddress( m_altCmdDLL, "CommandDriveAllStop" );
		m_pfCommandDriveAll			= (pfCommandDriveAll)GetProcAddress( m_altCmdDLL, "CommandDriveAll" );
		m_pfCommandOther			= (pfCommandOther)GetProcAddress( m_altCmdDLL, "CommandOther" );
		m_pfSetCatchMessages		= (pfSetCatchMessages)GetProcAddress( m_altCmdDLL, "SetCatchMessages" );
		m_pfGetMessageKey			= (pfGetMessageKey)GetProcAddress( m_altCmdDLL, "GetMessageKey" );
		m_pfGetMessagesArray		= (pfGetMessagesArray)GetProcAddress( m_altCmdDLL, "GetMessagesArray" );
		m_pfSetCatchReceivedLines	= (pfSetCatchReceivedLines)GetProcAddress( m_altCmdDLL, "SetCatchReceivedLines" );
		m_pfGetReceivedLine			= (pfGetReceivedLine)GetProcAddress( m_altCmdDLL, "GetReceivedLine" );
		m_pfCloseControl			= (pfCloseControl)GetProcAddress( m_altCmdDLL, "CloseControl" );
		m_pfGetClientIP				= (pfGetClientIP)GetProcAddress( m_altCmdDLL, "GetClientIP" );
		m_pfSetupImages				= (pfSetupImages)GetProcAddress( m_altCmdDLL, "SetupImages" );
		m_pfGetIplImage				= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImage" );

		m_pfGetIplImageEMD1Ch		= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImageEMD1Ch" );
		m_pfGetIplImageEMD2Ch		= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImageEMD2Ch" );
		m_pfGetIplImageEMDRegions	= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImageEMDRegions" );
		m_pfGetIplImageEMD1ChStats	= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImageEMD1ChStats" );
		m_pfGetIplImageEMD2ChStats	= (pfGetIplImage)GetProcAddress( m_altCmdDLL, "GetIplImageEMD2ChStats" );

		m_pfCloseImages				= (pfCloseImages)GetProcAddress( m_altCmdDLL, "CloseImages" );
		m_pfIncEMDParameters		= (pfIncEMDParameters)GetProcAddress( m_altCmdDLL, "IncEMDParameters" );
		m_pfGetTimeMilliseconds		= (pfGetTimeMilliseconds)GetProcAddress( m_altCmdDLL, "GetTimeMilliseconds" );

		return	m_pfSetupControl && 
				m_pfSendRobot &&
				m_pfGetDriveMinValue &&
				m_pfGetDriveMaxValue &&
				m_pfGetDriveIncValue &&
				m_pfGetDriveCurValue &&
				m_pfCommandDrive && 
				m_pfCommandDriveAllStop &&
				m_pfCommandDriveAll	&&
				m_pfCommandOther &&
				m_pfSetCatchMessages &&
				m_pfGetMessageKey &&
				m_pfGetMessagesArray &&
				m_pfSetCatchReceivedLines &&
				m_pfGetReceivedLine &&
				m_pfCloseControl &&
				m_pfGetClientIP &&
				m_pfSetupImages &&
				m_pfGetIplImage &&
				m_pfGetIplImageEMD1Ch &&
				m_pfGetIplImageEMD2Ch &&
				m_pfGetIplImageEMDRegions &&
				m_pfGetIplImageEMD1ChStats &&
				m_pfGetIplImageEMD2ChStats &&
				m_pfIncEMDParameters &&
				m_pfCloseImages &&
				m_pfGetTimeMilliseconds;
	}
	else
		return NULL;

}

//=========================================================================
bool CaltURI_plus :: OpenaltURI( const tString& sIniPath, const tString& sHookDllPath, const tString& sCmdDllPath, const bool bLocalImage, const bool bUseCmdDllImage, const bool bVisionOnly )
{
	m_bLocalImage = bLocalImage;
	m_IplImage = NULL;
	m_altHookDLL = NULL;
	m_altCmdDLL = NULL;
	m_pfGetIplImage = NULL;

	// must use command dll if not local images (ie. html images only via command dll
	if ( !bUseCmdDllImage && !m_bLocalImage ) return false;

	if ( !LinkCmdDll( sCmdDllPath ) ) return false;
	
	if ( !bVisionOnly )
		if ( !m_pfSetupControl( sIniPath.c_str() ) ) return false;

	if ( bUseCmdDllImage )
		m_pfSetupImages( m_bLocalImage );
	else
	{
		if ( m_bLocalImage )	// get image directly from hook dll
		{
			m_altHookDLL = LoadLibrary( sHookDllPath.c_str() );

			if ( m_altHookDLL ) m_pfGetIplImage = (pfGetIplImage)GetProcAddress( m_altHookDLL, "GetIplImage" );

			// redirect all EMD images to normal images if we use direct from hook dll (EMD only implemented in cmd dlls)
			m_pfGetIplImageEMD1Ch = (pfGetIplImage)m_pfGetIplImage;
			m_pfGetIplImageEMD2Ch = (pfGetIplImage)m_pfGetIplImage;
			m_pfGetIplImageEMDRegions = (pfGetIplImage)m_pfGetIplImage;
			m_pfGetIplImageEMD1ChStats = (pfGetIplImage)m_pfGetIplImage;
			m_pfGetIplImageEMD2ChStats = (pfGetIplImage)m_pfGetIplImage;
		}
	}

	m_baltURI = true;

	SetImageSource( 0 );

	return ( m_pfGetIplImageActual != NULL );
}

//=========================================================================
void CaltURI_plus :: SetImageSource( const int iType )
{
	// set function pointer to correct image function
	switch ( iType )
	{
		case 0:	// raw opencv
			m_pfGetIplImageActual = m_pfGetIplImage;
			break;
		case 1:	// raw EMD 1 channel
			m_pfGetIplImageActual = (pfGetIplImage)m_pfGetIplImageEMD1Ch;
			break;
		case 2:	// raw EMD 2 channel
			m_pfGetIplImageActual = (pfGetIplImage)m_pfGetIplImageEMD2Ch;
			break;
		case 3:	// regions EMD 2 channel
			m_pfGetIplImageActual = (pfGetIplImage)m_pfGetIplImageEMDRegions;
			break;
		case 4:	// EMD stats 1 channel
			m_pfGetIplImageActual = (pfGetIplImage)m_pfGetIplImageEMD1ChStats;
			break;
		case 5:	// EMD stats 2 channel
			m_pfGetIplImageActual = (pfGetIplImage)m_pfGetIplImageEMD2ChStats;
			break;
		default: // set to raw if not implemented
			m_pfGetIplImageActual = m_pfGetIplImage;
	}

}

//=========================================================================
void CaltURI_plus :: IncEMDParams( const bool bStartParamsAndLog )
{
	m_pfIncEMDParameters( bStartParamsAndLog );
	return;
}

//=========================================================================
IplImage* CaltURI_plus :: GetFrame( const int width, const int height, const int x, const int y )
{
	//return m_pfGetIplImage( width, height, x, y );
	return m_pfGetIplImageActual( width, height, x, y );
}

//=========================================================================
CvSize CaltURI_plus :: GetSize( void )
{
	if (m_IplImage)
		return cvGetSize( m_IplImage );
	else
		return cvSize( 0, 0 );
}

//=========================================================================
tString CaltURI_plus :: ReceiveLine( void )
{
	char* sTemp = m_pfGetReceivedLine();

	if ( !sTemp ) sTemp = "";
	
	tString tS( sTemp );
	
	return tS;
}

//=========================================================================
bool CaltURI_plus :: SendCommand( tString tCommand )
{
	return m_pfSendRobot( tCommand.c_str() );
}

//=========================================================================
bool CaltURI_plus :: CommandDriveAllStop( void)
{
	return m_pfCommandDriveAllStop();
}

//=========================================================================
void CaltURI_plus :: ClosealtURI( void )
{
	if ( m_altCmdDLL )
	{
		m_pfCloseControl();
		if ( !m_bLocalImage ) m_pfCloseImages();
		FreeLibrary( m_altCmdDLL );
		m_altCmdDLL = NULL;
	}
	
	if ( m_altHookDLL ) 
	{
		FreeLibrary( m_altHookDLL );
		m_altHookDLL = NULL;
	}
}