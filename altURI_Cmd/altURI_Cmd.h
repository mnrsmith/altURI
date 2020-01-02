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
//============================================================================
//     altURI_Cmd.h - internal and external declarations for command interface
//============================================================================
#pragma once

#ifndef ALTURI_CMD_INCLUDED
#define ALTURI_CMD_INCLUDED

#if defined(MATLAB_LINKED)
#include "CMxInterface.h"
#endif


#ifdef ALTURI_CMD_EXPORTS
#define LIBSPEC __declspec(dllexport)
#else
#define LIBSPEC __declspec(dllimport)
#pragma comment(lib, "altURI_Cmd")
#endif // ALTURI_CMD_EXPORTS

// some defines for matlab 64-bit
#ifndef BYTE
typedef unsigned char	BYTE;
#endif
#ifndef USHORT
typedef unsigned short	USHORT;
#endif
#ifndef UINT
typedef unsigned int	UINT;
#endif

// defines for getFrameData
#define FRAME_PENDING 0
#define FRAME_OK 1
#define FRAME_ERROR 2

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct FrameData_t {
		BYTE state;
		BYTE sequence;
		USHORT width;
		USHORT height;
		UINT size;
		BYTE data[640*480*3+1];
	} FrameData;

	typedef enum {
		EMD_PARAM_A1,	// 1 & 2 channel HPF
		EMD_PARAM_A2,	// 1 & 2 channel HPF
		EMD_PARAM_B1,	// 1 & 2 channel LPF
		EMD_PARAM_B2,	// 1 & 2 channel LPF
		EMD_PARAM_B3,	// 2 channel only LPF
		EMD_PARAM_B4,	// 2 channel only LPF
		EMD_PARAM_C1,	// 1 & 2 channel PREV EMD
		EMD_PARAM_D1,	// 2 channel only HW+ channel 1
		EMD_PARAM_D2,	// 2 channel only HW- channel 1 
		EMD_PARAM_D3,	// 2 channel only HW+ channel 2
		EMD_PARAM_D4	// 2 channel only HW- channel 2
	} EMD_Param;

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

	// visible when linked OR when using for MATLAB shared library
	// MATLAB doesn't seem to need CMxInterface.h include - which is good
#if defined(MATLAB_LINKED) || defined(__LCC__)
	// return image to matlab
	LIBSPEC mxArray* MatlabGetImage( int width, int height, int x, int y );

	// return sensor messages to matlab
	LIBSPEC mxArray* MatlabGetMessagesArray( int iMessagesToGet, int iMessageNumber  );

	LIBSPEC mxArray* MatlabGetImageEMD( int width, int height, int x, int y );

	// matlab function - not used
	LIBSPEC void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] );
#endif
	
	// get image via HTTP or local
	LIBSPEC IplImage* GetIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMD1Ch( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMD2Ch( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMDRegions( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMD1ChStats( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// get image EMD via HTTP or local
	LIBSPEC IplImage* GetIplImageEMD2ChStats( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	// usarsim compatible image retrieval (uses GetIplImage above)
	LIBSPEC FrameData* getFrameData( void );

	LIBSPEC void CloseImages( void );

	LIBSPEC void IncEMDParameters( const bool bStartParamsAndLog, const char* szRunNotation );

	LIBSPEC void SetEMDParameter( const EMD_Param epParam, const double dStart , const double dEnd, const int iSteps, const int iRepeats );

#ifdef __cplusplus
}
#endif

#endif

