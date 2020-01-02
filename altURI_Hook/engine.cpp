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
//	engine.cpp - common graphics methods
//=========================================================================

#include "stdafx.h"
#include "engines.h"

//=========================================================================
void CEngine :: FreeDll()
{
	if ( m_hModule == NULL ) return;

	UnhookFunctions();

	::FreeLibrary(m_hModule);

	m_hModule = NULL;
}

//=========================================================================
bool CEngine :: CheckDll()
{
	if ( m_hModule ) return true;	// already loaded?

	m_hModule = ::GetModuleHandle( m_pszModuleName );

	return  ( m_hModule != NULL );
}

//=========================================================================
void CEngine :: PresentFrame()
{
	// if we require a frame get one but don't change the flag yet
	if ( InterlockedCompareExchange( &g_DllState.m_lFrameRequired, true, true ) )
		GetFrame();
}

//=========================================================================
HRESULT CEngine::AttachGraphXMode()
{
	// Check for DLL for this graphics mode - don't load it
	if ( !CheckDll() ) return HRESULT_FROM_WIN32( ERROR_DLL_NOT_FOUND );

	m_bHookedFunctions = HookFunctions();	// virtual

	if ( !m_bHookedFunctions ) return HRESULT_FROM_WIN32( ERROR_INVALID_HOOK_HANDLE );

	return S_OK;
}

//=========================================================================
void CEngine::DetachGraphXMode()
{
	FreeDll();
}

//=========================================================================
void CEngine::CopyAUImage( void* pBuffer, int iType, int iCode )
{
	g_MyImage.m_cvmBuf = cvMat(	g_DllState.m_SourceImageStats.m_height,
								g_DllState.m_SourceImageStats.m_width,
								iType,
								pBuffer );

	cvCvtColor( &g_MyImage.m_cvmBuf, g_MyImage.m_IplImage, iCode );

	// signal we have frame
	InterlockedExchange( &g_DllState.m_lFrameRequired, false );
}