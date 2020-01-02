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
//	dllmain.cpp - dll main
//=========================================================================

#include "stdafx.h"
#include "engines.h"
#include "CServerManager.h"
#include "CDLLStart.h"

//=========================================================================
#pragma data_seg(".SHARED")
CDllState g_DllState =	{	CDllState::NONE, {0, 0, 0}, NULL, NULL, true, { 0, 0, 0, 0 } };
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,RWS")
//=========================================================================

CSharedImage	g_MyImage;
CServerManager	g_Server;
CDLLStart		g_StartMe;
extern IplImage* Ipl_Resized;
extern IplImage* Ipl_Legacy;

//=========================================================================
bool OnDllProcessDetach()
{
	switch ( g_DllState.g_Graphics_Type )
	{
		case CDllState :: DX8:
			g_DX8.DetachGraphXMode();
			break;
		case CDllState :: DX9:
			g_DX9.DetachGraphXMode();
			break;
		case CDllState :: DX10:
			g_DX10.DetachGraphXMode();
			break;
		case CDllState :: DX11:
			g_DX11.DetachGraphXMode();
			break;
		case CDllState :: OGL:
			g_OGL.DetachGraphXMode();
		default:
			break;
	}

	if ( Ipl_Resized ) cvReleaseImage( &Ipl_Resized );
	if ( Ipl_Legacy ) cvReleaseImage( &Ipl_Legacy );

	return true;
}


//=========================================================================
//=========================================================================
//=========================================================================
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch ( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			g_StartMe.Start();
			return true;
		case DLL_PROCESS_DETACH:
			return OnDllProcessDetach();

	}
	return TRUE;
	
	UNREFERENCED_PARAMETER(lpReserved);
	UNREFERENCED_PARAMETER(hModule);
}

