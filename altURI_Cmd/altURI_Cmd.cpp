/*
Copyright (c) 2013, Mark N R Smith, All rights reserved.

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
// usar_cmd.cpp : Defines the exported (and helper) functions for the DLL.
//

#include "stdafx.h"

#include "altURI_Cmd.h"
#include "CControlInterface.h"
#include "mmsystem.h"		// for timeGetTime()

CControlInterface Robot;
CIniFile IniFile;
FrameData fData = { FRAME_OK, 0, 640, 480, 921601, NULL };

TCHAR* lpReturnLines = NULL;

#if defined(MATLAB_LINKED)
//mxArray* CMxInterface::mxarr = NULL;
CMxInterface MxInterface;
#endif

//=========================================================================
bool AddParamNEW( vparams& Params, const std::string& sSection, const int iIndex )
{
	std::stringstream oss;
	oss << "Param" << iIndex;
	tString sParam( oss.str() );

	Control_param_NEW param;

	param.m_sSubkey = IniFile.GetValue( sSection, "Subkey" );
	param.m_fMin = static_cast<float> ( IniFile.GetValueF( sSection, sParam + "Min" ) );
	param.m_fMax = static_cast<float> ( IniFile.GetValueF( sSection, sParam + "Max" ) );
	param.m_fIncrement = static_cast<float> ( IniFile.GetValueF( sSection, sParam + "Increment" ) );

	// some simple validation
	if (	param.m_sSubkey.size()<1 || param.m_fMax < param.m_fMin || param.m_fIncrement < 0 )
			return false;
	else
	{
		param.m_fCurrent = 0;			// assume begin with command zeroed
		Params.push_back( param );
		return true;
	}
}

//=========================================================================
bool AddCommandNEW( vcommands& Commands, const std::string& sSectionKey, const int iIndex )
{
	std::stringstream oss;
	oss << sSectionKey << iIndex;
	tString sSection( oss.str() );

	Control_command_NEW command;

	command.m_sKey = IniFile.GetValue( sSection, "Key" );
	int iNoOfParams = IniFile.GetValueI( sSection, "NoOfParams" );

	for ( int i = 0; i < iNoOfParams; i++ )
	{
		if ( !AddParamNEW( command.m_vParams, sSection, i + 1 ) ) break;
	}

	// if we have a command string and correct number of parameters
	if (	command.m_sKey.size()<1 || command.m_vParams.size() != iNoOfParams )
			return false;
	else
	{
		Commands.push_back( command );
		return true;
	}
}


//=========================================================================
bool AddCommand( std::vector<Control_command>& Commands, const tString& sSectionKey, const int iIndex )
{
	std::stringstream oss;
	oss << sSectionKey << iIndex;
	tString sSection( oss.str() );

	Control_command command;

	command.m_sKey = IniFile.GetValue( sSection, "Key" );
	command.m_fMin = static_cast<float> ( IniFile.GetValueF( sSection, "Min" ) );
	command.m_fMax = static_cast<float> ( IniFile.GetValueF( sSection, "Max" ) );
	command.m_fIncrement = static_cast<float> ( IniFile.GetValueF( sSection, "Increment" ) );

	// some simple validation
	if (	command.m_sKey.size()<1 ||
			command.m_fMax < command.m_fMin ||
			command.m_fIncrement < 0 )
			return false;
	else
	{
		command.m_fCurrent = 0;			// assume begin with command zeroed
		Commands.push_back( command );
		return true;
	}
}

//=========================================================================
bool AddMessage( std::vector<Message_spec>& MessageSpecs, const tString& sSectionKey, const int iIndex )
{
	std::stringstream oss;
	oss << sSectionKey << iIndex;
	tString sSection( oss.str() );
	
	Message_spec message;

	message.m_sMessageName = IniFile.GetValue( sSection, "MessageName" );
	message.m_sKeyName = IniFile.GetValue( sSection, "KeyName" );
	message.m_iMessageNumber = iIndex;
	message.m_iIndex = IniFile.GetValueI( sSection, "Index" );

	// some simple validation
	if (	message.m_sMessageName.size()<1 ||
			message.m_sKeyName.size()<1 ||
			message.m_iMessageNumber < 1 ||
			message.m_iMessageNumber > 20 )
			return false;
	else
	{
		MessageSpecs.push_back( message );
		return true;
	}
}

//=========================================================================
bool ReadMessagesFromIniFile( void )
{
	// get number of messages to load from ini file
	int iNoOfMessages = IniFile.GetValueI( "messages", "MessageTotal"  );

	Robot.GetMessageSpecs().clear();	// remove any previous message

	// try to load correct number of messages
	for ( int i = 0; i < iNoOfMessages; i++ )
		if ( !AddMessage( Robot.GetMessageSpecs(), TEXT("message"), i + 1 ) ) break;

	// check no. of messages specfied equals messages read
	return ( iNoOfMessages == Robot.GetMessageSpecs().size() );
}

//=========================================================================
LIBSPEC bool SetupControl( const char* szIniFile )
{
	IniFile.Path( szIniFile );

	// load the ini file contents
	if ( !IniFile.ReadFile() ) return false;

	// get ip address and port from ini file
	tString sIP = IniFile.GetValue( "connection", "SimIP4" );
	tString sPort =  IniFile.GetValue( "connection", "SimCommandPort" );

	// get number of drive commands to load from ini file
	int iDriveCommands = IniFile.GetValueI( "drives", "DriveTotal"  );
	Robot.m_sDrivePrefix = IniFile.GetValue( "drives", "DrivePrefix" );

	Robot.m_vDrives.clear();	// remove any previous drive commands

	// try to load correct number of drive commands
	for ( int i = 0; i < iDriveCommands; i++ )
	{
		if ( !AddCommand( Robot.m_vDrives, TEXT("drive"), i + 1 ) ) break;
	}

	// check no. of drive commands specfied equals commands read
	if ( iDriveCommands != Robot.m_vDrives.size() ) return false;

	Robot.m_bNormalized = IniFile.GetValueB( "drive", "Normalize"  );

	// get number of non-drive commands to load from ini file
	int iOtherCommands = IniFile.GetValueI( "commands", "CommandTotal" );
	Robot.m_sCommandPrefix = IniFile.GetValue( "commands", "CommandPrefix" );

	Robot.m_vCommands.clear();	// remove any previous non-drive commands

	// try to load correct number of non-drive commands
	for ( int i = 0; i < iOtherCommands; i++ )
	{
		if ( !AddCommand( Robot.m_vCommands, TEXT("command"), i + 1 ) ) break;
	}

	// check no. of commands specfied equals commands read
	if ( iOtherCommands != Robot.m_vCommands.size() ) return false;

	// read sensor messages from ini file
	( void )ReadMessagesFromIniFile();

	// connect to the simulator via tcpip
	return Robot.OpenControl( sIP, sPort );
}

//=========================================================================
//	Send a string to the robot - eg init
//=========================================================================
LIBSPEC bool SendRobot( const char* szAnyCommand )
{
	if ( Robot.IsConnected() )
	{
		tString sData( szAnyCommand );

		return Robot.SendCommand( sData );
	}
	else
		return false;
}

//=========================================================================
LIBSPEC float GetDriveMinValue( const int iDriveNo )
{
	if ( !Robot.IsVectorControlIndex( Robot.m_vDrives, iDriveNo ) )
		return 0;

	return Robot.m_vDrives[iDriveNo].m_fMin;
}

//=========================================================================
LIBSPEC float GetDriveMaxValue( const int iDriveNo )
{
	if ( !Robot.IsVectorControlIndex( Robot.m_vDrives, iDriveNo ) )
		return 0;

	return Robot.m_vDrives[iDriveNo].m_fMax;
}

//=========================================================================
LIBSPEC float GetDriveIncValue( const int iDriveNo )
{
	if ( !Robot.IsVectorControlIndex( Robot.m_vDrives, iDriveNo ) )
		return 0;

	return Robot.m_vDrives[iDriveNo].m_fIncrement;
}

//=========================================================================
LIBSPEC float GetDriveCurValue( const int iDriveNo )
{
	if ( !Robot.IsVectorControlIndex( Robot.m_vDrives, iDriveNo ) )
		return 0;

	return Robot.m_vDrives[iDriveNo].m_fCurrent;
}

//=========================================================================
tString FormatCommand( const tString& sKey, const float fCurrent )
{
	TCHAR tcTemp[1024];

	sprintf( tcTemp, sKey.c_str(), fCurrent );

	return tString( tcTemp );
}

//=========================================================================
tString MakeDriveCommand( const int iIndex, const int iIncrements )
{
	// check within vector
	if ( !Robot.IsVectorControlIndex( Robot.m_vDrives, iIndex ) )
			return TEXT("");

	// calculate new value
	float fNew = Robot.m_vDrives[iIndex].m_fCurrent + ( iIncrements ? iIncrements * Robot.m_vDrives[iIndex].m_fIncrement : -(iIncrements * Robot.m_vDrives[iIndex].m_fIncrement) );

	// set limit if outside range
	if ( fNew > Robot.m_vDrives[iIndex].m_fMax )
		 fNew = Robot.m_vDrives[iIndex].m_fMax;

	if ( fNew < Robot.m_vDrives[iIndex].m_fMin )
		 fNew = Robot.m_vDrives[iIndex].m_fMin;

	Robot.m_vDrives[iIndex].m_fCurrent = fNew;

	return FormatCommand( Robot.m_vDrives[iIndex].m_sKey, Robot.m_vDrives[iIndex].m_fCurrent );
}

//=========================================================================
LIBSPEC bool CommandDrive( const int iDriveNo, const int iIncrements )
{
	if ( Robot.IsConnected() )
	{
		tString sData = MakeDriveCommand( iDriveNo - 1, iIncrements );

		if ( sData.size() == 0 ) return false;

		if ( Robot.m_sDrivePrefix.size() > 0 )
			sData = Robot.m_sDrivePrefix + TEXT(" ") + sData;

		return Robot.SendCommand( sData );
	}
	else
		return false;
}

//=========================================================================
LIBSPEC bool CommandDriveAll(	const int iIncrements1,
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
								const int iIncrements12 )
{
	if ( Robot.IsConnected() )
	{
		tString sBigData;

		if ( iIncrements1 != 0 ) sBigData += MakeDriveCommand( 1, iIncrements1 );
		if ( iIncrements2 != 0 ) sBigData += MakeDriveCommand( 2, iIncrements2 );
		if ( iIncrements3 != 0 ) sBigData += MakeDriveCommand( 3, iIncrements3 );
		if ( iIncrements4 != 0 ) sBigData += MakeDriveCommand( 4, iIncrements4 );
		if ( iIncrements5 != 0 ) sBigData += MakeDriveCommand( 5, iIncrements5 );
		if ( iIncrements6 != 0 ) sBigData += MakeDriveCommand( 6, iIncrements6 );
		if ( iIncrements7 != 0 ) sBigData += MakeDriveCommand( 7, iIncrements7 );
		if ( iIncrements8 != 0 ) sBigData += MakeDriveCommand( 8, iIncrements8 );
		if ( iIncrements9 != 0 ) sBigData += MakeDriveCommand( 9, iIncrements9 );
		if ( iIncrements10 != 0 ) sBigData += MakeDriveCommand( 10, iIncrements10 );
		if ( iIncrements11 != 0 ) sBigData += MakeDriveCommand( 11, iIncrements11 );
		if ( iIncrements12 != 0 ) sBigData += MakeDriveCommand( 12, iIncrements12 );

		if ( sBigData.size() != 0 )
		{
			if ( Robot.m_sDrivePrefix.size() > 0 )
				sBigData = Robot.m_sDrivePrefix + TEXT(" ") + sBigData;

			return Robot.SendCommand( sBigData );
		}
		else
			return false;
	}
	else
		return false;
}

//=========================================================================
LIBSPEC bool CommandDriveAllStop( void )
{
	if ( Robot.IsConnected() )
	{
		tString sBigData;

		for ( int i = 0; i < (int)Robot.m_vDrives.size(); i++ )
		{
			if ( Robot.m_vDrives[i].m_fCurrent != 0 )
			{
				Robot.m_vDrives[i].m_fCurrent = 0;
				sBigData += FormatCommand( Robot.m_vDrives[i].m_sKey, Robot.m_vDrives[i].m_fCurrent );
			}
		}

		if ( Robot.m_sDrivePrefix.size() > 0 )
			sBigData = Robot.m_sDrivePrefix + TEXT(" ") + sBigData;

		return Robot.SendCommand( sBigData );
	}
	else
		return false;
}

//=========================================================================
LIBSPEC bool CommandOther( const int iCommandNo, const int iIncrements  )
{
	if ( Robot.IsConnected() )
	{
		int iIndex = iCommandNo - 1;

		// check within vector
		if ( !Robot.IsVectorControlIndex( Robot.m_vCommands, iIndex ) )
			return false;

		// calculate new value
		float fNew = Robot.m_vCommands[iIndex].m_fCurrent + ( iIncrements ? iIncrements * Robot.m_vCommands[iIndex].m_fIncrement : -(iIncrements * Robot.m_vCommands[iIndex].m_fIncrement ) );

		// set limit if outside range
		if ( fNew > Robot.m_vCommands[iIndex].m_fMax )
			 fNew = Robot.m_vCommands[iIndex].m_fMax;

		if ( fNew < Robot.m_vCommands[iIndex].m_fMin )
			 fNew = Robot.m_vCommands[iIndex].m_fMin;

		Robot.m_vCommands[iIndex].m_fCurrent = fNew;

		tString sData( FormatCommand( Robot.m_vCommands[iIndex].m_sKey, Robot.m_vCommands[iIndex].m_fCurrent ) );

		return Robot.SendCommand( sData );
	}
	else
		return false;
}

//=========================================================================
LIBSPEC bool SetCatchMessages( const bool bCatchMessages )
{
	// load messages if catching and not loaded before
	if ( bCatchMessages && Robot.GetNoMessageSpecs() == 0 )
	{
		if ( !ReadMessagesFromIniFile() ) return false;
	}

	Robot.SetCatchMessages( bCatchMessages );

	return true;
}

//=========================================================================
//	return string from Ini file corresponding to message number
//=========================================================================
LIBSPEC char* GetMessageKey( const int iMessageNumber )
{
	static char key[255];

	memset( key, 0, sizeof( key ) );

	for ( std::vector<Message_spec>::iterator iter = Robot.GetMessageSpecs().begin(); iter < Robot.GetMessageSpecs().end(); iter++ )
		if ( iter->m_iMessageNumber == iMessageNumber )
		{
			std::strncpy( key, iter->m_sKeyName.c_str(), sizeof( key ) );
			break;
		}

	return key;
}

//=========================================================================
//	Copy to array for MatLab or whatever iMessageNumber=0 means all messages
//=========================================================================
LIBSPEC int GetMessagesArray( const int iMessagesToGet, double** dArray2d, const int iMessageNumber )
{
	int iValuesGot = 0;
	
	// protect vector unloading process
	EnterCriticalSection( &Robot.m_Sensorcs );
	{
		std::vector<Message_value>::iterator iter = Robot.GetMessageValues().begin();

		// iterate thru vector loading array and deleting value
		while ( iter != Robot.GetMessageValues().end() && iValuesGot < iMessagesToGet ) 
		{
			// if getting all messages or message is one that has been requested
			if ( iMessageNumber == 0 || iMessageNumber == iter->m_iMessageNumber )
			{
				dArray2d[iValuesGot][0] = iter->m_fTime;
				dArray2d[iValuesGot][1] = iter->m_iMessageNumber;
				dArray2d[iValuesGot][2] = iter->m_iIndex;
				dArray2d[iValuesGot][3] = iter->m_fValue;
				iter = Robot.GetMessageValues().erase( iter ); 
				iValuesGot++;
			}
			else
				iter++;
		}
	}
	LeaveCriticalSection( &Robot.m_Sensorcs );

	return iValuesGot;
}


//=========================================================================
LIBSPEC void SetCatchReceivedLines( const bool bCatchLines, const bool bCatchMessageLines )
{
	Robot.SetCatchLines( bCatchLines );
	Robot.SetCatchMessagesLines( bCatchMessageLines );
}

//=========================================================================
LIBSPEC char* GetReceivedLine( void )
{
	if ( Robot.GetNoLinesReceived() > 0 )
	{
		tString sLines( Robot.GetLines() );

		// delete any old
		if ( lpReturnLines ) delete[] lpReturnLines;

		lpReturnLines = new TCHAR[ sLines.size() + 1 ];

		// copy the string
		std::strcpy( lpReturnLines, sLines.c_str() );

		return lpReturnLines;
	}
	else
		return NULL;
}

//=========================================================================
LIBSPEC void CloseControl( void )
{
	// delete any lines received
	if ( lpReturnLines ) delete[] lpReturnLines;

	Robot.CloseControl();
}

//=========================================================================
LIBSPEC char* GetClientIP( void )
{
	static char addr[16];
	char buffer[255];
	gethostname( buffer, sizeof( buffer ) );
	hostent*  lpHostEnt = gethostbyname( buffer );

	if ( !lpHostEnt ) 
		strcpy( addr, "127.0.0.1" );
	else
		strcpy( addr, inet_ntoa(*(LPIN_ADDR)*(lpHostEnt->h_addr_list)) );

	return addr;
}

//=========================================================================
LIBSPEC bool SetupImages( const bool bLocal )
{
	if ( bLocal )
	{
		if ( Robot.OpenDLLImages( ) )
		{
			Robot.GetIplImage( );
			return true;
		}
		else
			return false;
	}
	else
	{
		// get ip address and port from ini file
		tString sIP = IniFile.GetValue( "connection", "SimIP4" );
		tString sPort =  IniFile.GetValue( "connection", "SimImagePort" );

		if ( Robot.OpenHTMLImages( sIP, sPort ) )
		{
			Robot.GetIplImage( );
			return true;
		}
		else
			return false;
	}
}

#if defined(MATLAB_LINKED)
//=========================================================================
LIBSPEC mxArray* MatlabGetImage( int width, int height, int x, int y )
{
	if ( !MxInterface.IsMxLoaded() ) 
			if ( !MxInterface.OpenMxInterface() ) return NULL;

	IplImage* tempImage = Robot.GetIplImage( width, height, x, y );

	if ( CV_IS_IMAGE( tempImage ) )
	{
		// convert to RGB for matlab
		IplImage* matlabImage = cvCreateImage( cvSize( tempImage->width, tempImage->height ), tempImage->depth, tempImage->nChannels );
		cvCvtColor( tempImage, matlabImage, CV_RGB2BGR );

		mxArray* mxA = MxInterface.mxArrayFromIplImage( matlabImage );

		cvReleaseImage( &matlabImage );

		return mxA;
	}
	else
		return NULL;
}

//=========================================================================
LIBSPEC mxArray* MatlabGetImageEMD( int width, int height, int x, int y )
{
	if ( !MxInterface.IsMxLoaded() ) 
		if ( !MxInterface.OpenMxInterface() ) return NULL;

	IplImage* tempImage = Robot.GetIplImageEMD2( width, height, x, y );

	if ( CV_IS_IMAGE( tempImage ) )
	{
		mxArray* mxA = MxInterface.mxArrayFromIplImage( tempImage );

		//cvReleaseImage( &tempImage );

		return mxA;
	}
	else
		return NULL;
}

//=========================================================================
LIBSPEC mxArray* MatlabGetMessagesArray( int iMessagesToGet, int iMessageNumber  )
{
	if ( !MxInterface.IsMxLoaded() ) 
		if ( !MxInterface.OpenMxInterface() ) return NULL;

	const int iColumnsToGet = 4;

	// make array for some sensor data
	double** dArray2D = new double*[iMessagesToGet];

	for (int i = 0; i < iMessagesToGet; ++i)
		dArray2D[i] = new double[iColumnsToGet];

	// get the sensor data (if any)
	int iRows = GetMessagesArray( iMessagesToGet, dArray2D, iMessageNumber );

	// if no sensor data rows
	if ( iRows == 0 ) return NULL;

	mxArray* mxarr = MxInterface.mxArrayFromDouble2DArray( dArray2D, iRows, iColumnsToGet );

	// deallocate memory used
	for (int i = 0; i < iMessagesToGet; ++i)
		delete [] dArray2D[i];
	delete [] dArray2D;

	return mxarr;
}

//=========================================================================
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
	UNREFERENCED_PARAMETER(nlhs);
	UNREFERENCED_PARAMETER(plhs);
	UNREFERENCED_PARAMETER(nrhs);
	UNREFERENCED_PARAMETER(prhs);
}

#endif

//=========================================================================
LIBSPEC IplImage* GetIplImage( const int width, const int height, const int x, const int y )
{
	return Robot.GetIplImage( width, height, x, y );
}

//=========================================================================
LIBSPEC IplImage* GetIplImageEMD1Ch( const int width, const int height, const int x, const int y )
{
	return Robot.m_EMD.MakeEMD9( Robot.GetIplImage( width, height, x, y ) );
}

//=========================================================================
LIBSPEC IplImage* GetIplImageEMD2Ch( const int width, const int height, const int x, const int y )
{
	return Robot.m_EMD.Make2ChannelEMD( Robot.GetIplImage( width, height, x, y ) );
}

//=========================================================================
LIBSPEC IplImage* GetIplImageEMDRegions( const int width, const int height, const int x, const int y )
{
	return Robot.m_EMD.MakeEMDRegions( Robot.m_EMD.Make2ChannelEMD( Robot.GetIplImage( width, height, x, y ) ), 0.25, 0.25 );
}

//=========================================================================
LIBSPEC IplImage* GetIplImageEMD1ChStats( const int width, const int height, const int x, const int y )
{
	return Robot.m_EMD.MakeEMDLog( Robot.m_EMD.MakeEMD9( Robot.GetIplImage( width, height, x, y ) ), 0.25, 0.25 );
}

//=========================================================================
LIBSPEC IplImage* GetIplImageEMD2ChStats( const int width, const int height, const int x, const int y )
{
	return Robot.m_EMD.MakeEMDLog( Robot.m_EMD.Make2ChannelEMD( Robot.GetIplImage( width, height, x, y ) ), 0.25, 0.25 );
}

//=========================================================================
LIBSPEC  FrameData* getFrameData()
{
	fData.state = FRAME_PENDING;

	IplImage* iplTemp = Robot.GetIplImage( 640, 480 );

	if ( CV_IS_IMAGE( iplTemp ) )
	{
		// set pointer in legacy array
		*( fData.data ) = *( iplTemp->imageData );
		fData.state = FRAME_OK;
	}
	else
	{
		*( fData.data ) = NULL;
		fData.state = FRAME_ERROR;
	}

	return &fData;
}

//=========================================================================
LIBSPEC void CloseImages( void )
{
	Robot.CloseImages();
}

//=========================================================================
LIBSPEC void IncEMDParameters( const bool bStartParamsAndLog, const char* szRunNotation ) 
{
	std::string sFileName;

	if ( bStartParamsAndLog )
	{
		std::string sNotation( szRunNotation );
		sFileName = TEXT("EMD_RESULTS_") + sNotation + TEXT("_") + Robot.m_EMD.GetFileDateTimeString();
	}

	// inc param here



	Robot.m_EMD.WriteLogLines( sFileName );

	return;
}

//=========================================================================
LIBSPEC void SetEMDParameter( const EMD_Param epParam, const double dStart , const double dEnd, const int iSteps, const int iRepeats )
{
	return;
	//return timeGetTime();
}