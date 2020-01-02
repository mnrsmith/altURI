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
//	graphx_ogl.cpp - OpenGL graphics methods
//=========================================================================

#include "stdafx.h"
#include "engines.h"

#include <GL/gl.h>

COGLEngine g_OGL;

//=========================================================================
// declare function pointer types for the OGL exported function.
// function pointers to allow explicit run-time linking.

#define GRAPHXOGLFUNC(a,b,c) \
	typedef WINGDIAPI b (APIENTRY * PFN##a) c;\
	static PFN##a s_##a = NULL;
#include "engine_ogl.tbl"
#undef GRAPHXOGLFUNC

//=========================================================================
HRESULT COGLEngine::GetFrame()
{
	// select back buffer
	s_glReadBuffer(GL_BACK);

	HDC hDC = s_wglGetCurrentDC();
	HWND hWnd = ::WindowFromDC( hDC );

	if ( hWnd == NULL ) return NULL;

	RECT rect;
	if ( ! ::GetClientRect( hWnd, &rect ) ) return NULL;

	g_MyImage.Lock();

	if ( g_DllState.m_SourceImageStats.m_width != (UINT)rect.right || g_DllState.m_SourceImageStats.m_height != (UINT)rect.bottom )
	{
		g_DllState.m_SourceImageStats.m_width = rect.right;
		g_DllState.m_SourceImageStats.m_height = rect.bottom;
		g_DllState.m_UUID = g_MyImage.MakeAndSetMapName();
		g_MyImage.MakeSharedIplImage(	g_DllState.m_SourceImageStats.m_height,
										g_DllState.m_SourceImageStats.m_width );
	}

	// read the pixels data
	s_glReadPixels(0, 0, g_DllState.m_SourceImageStats.m_width, g_DllState.m_SourceImageStats.m_height, GL_RGB, GL_UNSIGNED_BYTE, g_MyImage.m_IplImage->imageData);

	g_MyImage.Unlock();		// must be better way to do this... (don't lock twice)

	CopyAUImage( (void*)g_MyImage.m_IplImage->imageData, CV_8UC3, CV_BGR2RGB );

	return S_OK;
}

//=========================================================================
EXTERN_C __declspec(dllexport) void APIENTRY OGL_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	// put back saved code fragment
	g_OGL.m_Hook_ViewPort.DisableHook();

	// call original function
	s_glViewport( x, y, width, height);

	// put JMP instruction again
	g_OGL.m_Hook_ViewPort.EnableHook();
}

//=========================================================================
EXTERN_C __declspec(dllexport) BOOL APIENTRY OGL_WglSwapBuffers(HDC hdc)
{
	// put back saved code fragment
	g_OGL.m_Hook_SwapBuffers.DisableHook();

	g_OGL.PresentFrame();

	// call original function
	BOOL bRes = s_wglSwapBuffers(hdc);

	// put JMP instruction again
	g_OGL.m_Hook_SwapBuffers.EnableHook();
	return bRes;
}

//=========================================================================
bool COGLEngine::HookFunctions()
{
	// Hooks multiple functions 
	// hook wglSwapBuffers, using code modifications at run-time.
	// ALGORITHM: we overwrite the beginning of real wglSwapBuffers
	// with a JMP instruction to our routine (OGL_WglSwapBuffers).
	// When our routine gets called, first thing that it does - it restores 
	// the original bytes of wglSwapBuffers, then performs its pre-call tasks, 
	// then calls wglSwapBuffers, then does post-call tasks, then writes
	// the JMP instruction back into the beginning of wglSwapBuffers, and
	// returns.
	
  	
	// initialize function pointers
#define GRAPHXOGLFUNC(a,b,c) s_##a = (PFN##a) GetProcAddress(m_hModule, #a);\
	if ( s_##a == NULL ) \
	{\
		return false;\
	}
#include "engine_ogl.tbl"
#undef GRAPHXOGLFUNC

	// DEBUG_MSG(( "CTaksiOGL::HookFunctions: checking JMP-implant..." LOG_CR));
	if ( !m_Hook_SwapBuffers.InstallHook(s_wglSwapBuffers,OGL_WglSwapBuffers) ) return false;

	if ( !m_Hook_ViewPort.InstallHook(s_glViewport,OGL_glViewport) ) return false;

	return CEngine :: HookFunctions();
}

//=========================================================================
void COGLEngine::UnhookFunctions()
{
	m_Hook_SwapBuffers.RemoveHook();
	m_Hook_ViewPort.RemoveHook();

	CEngine :: UnhookFunctions();
}


