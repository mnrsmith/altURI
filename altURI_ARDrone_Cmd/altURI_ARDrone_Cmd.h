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
//============================================================================
//     altURI_ARDrone_Cmd.h - internal/external command interface for ARDrone
//============================================================================
#pragma once

#ifndef ALTURI_ARDRONE_CMD_INCLUDED
#define ALTURI_ARDRONE_CMD_INCLUDED


#ifdef ALTURI_ARDRONE_CMD_EXPORTS
#define LIBSPEC __declspec(dllexport)
#else
#define LIBSPEC __declspec(dllimport)
#pragma comment(lib, "altURI_ARDrone_Cmd")
#endif // ALTURI_ARDRONE_CMD_EXPORTS



#ifdef __cplusplus
extern "C"
{
#endif

	LIBSPEC bool SetupControl( const char* szIniFile );

	LIBSPEC bool SendRobot( const char* szAnyCommand );

	LIBSPEC float GetDriveMinValue( const int iDriveNo );
	LIBSPEC float GetDriveMaxValue( const int iDriveNo );
	LIBSPEC float GetDriveIncValue( const int iDriveNo );
	LIBSPEC float GetDriveCurValue( const int iDriveNo );

	LIBSPEC bool CommandDrive( const int iDriveNo, const int iIncrements );

	LIBSPEC bool CommandDriveAllStop( void );

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
									const int iIncrements12 );

	LIBSPEC bool CommandOther( const int iCommandNo, const int iIncrements );

	LIBSPEC bool SetCatchMessages( const bool bCatchMessages );

	LIBSPEC char* GetMessageKey( const int iMessageNumber );

	LIBSPEC int GetMessagesArray( const int iMessagesToGet, double** dArray2d, const int iMessageNumber );

	LIBSPEC void SetCatchReceivedLines( const bool bCatchLines, const bool bCatchMessageLines );

	LIBSPEC char* GetReceivedLine( void );
	
	LIBSPEC void CloseControl( void );

	LIBSPEC char* GetClientIP( void );

	LIBSPEC bool SetupImages( const bool bLocal );

	// get image via HTTP or local
	LIBSPEC IplImage* GetIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMD( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	LIBSPEC void CloseImages( void );

	LIBSPEC long GetTimeMilliseconds( void );

#ifdef __cplusplus
}
#endif

#endif

