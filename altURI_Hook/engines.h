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
//	engines.h - defines graphics engines handling classes
//=========================================================================

#pragma once

#include "CHookJump.h"

#define COUNTOF(a) 		(sizeof(a)/sizeof((a)[0]))

//=========================================================================
//	the generic graphics engine base class.
//=========================================================================
struct CEngine
{
public:
	CEngine() : m_bHookedFunctions(false) { };

	~CEngine() { FreeDll(); }

	HRESULT AttachGraphXMode();
	void DetachGraphXMode();

	void PresentFrame();

	bool CheckDll();
	void FreeDll();

	static void CopyAUImage( void* pBuffer, int iType, int iCode );

	UINT_PTR get_DllInt() const { 	return (UINT_PTR) m_hModule; };
	//HWND FindProcessWindow();

protected:
	virtual bool HookFunctions()
	{
		assert( m_hModule != NULL );
		return true;
	}
	virtual void UnhookFunctions()
	{
		m_bHookedFunctions = false;
	}

	virtual HRESULT GetFrame() = 0;

protected:
	HMODULE			m_hModule;			// dll hModule
	LPCWSTR			m_pszModuleName;

public:
	bool			m_bHookedFunctions;	// HookFunctions() called and returned true
};

//=========================================================================
//	a directx capture class
//=========================================================================
struct CDXEngine : public CEngine
{
public:
	CDXEngine() : m_iRefCount( 0 ), m_Hook_AddRef(NULL), m_Hook_Release(NULL){} ;

	virtual bool GetOffsets() = 0;
	virtual bool HookFunctions() = 0;
	virtual void UnhookFunctions() = 0;
	virtual HRESULT GetFrame() = 0;

public:
	ULONG		m_iRefCount;	// Saved RefCounts to check for last release and unhook etc.

	CHookJump	m_Hook_Present;
	CHookJump	m_Hook_Reset;
	UINT_PTR*	m_Hook_AddRef;
	UINT_PTR*	m_Hook_Release;

protected:
	HWND CreateDeviceWindow();

private:
	enum { INTF_AddRef = 1, INTF_Release = 2 } ;
};

//=========================================================================
//	a directx 9 capture class
//=========================================================================

interface IDirect3DDevice8;
struct CDX8Engine : public CDXEngine
{
public:
	CDX8Engine() : m_pDevice( NULL ) { m_pszModuleName = TEXT("d3d8.dll"); };

	virtual bool GetOffsets();
	virtual bool HookFunctions();
	virtual void UnhookFunctions();

	virtual HRESULT GetFrame();
	
public:
	IDirect3DDevice8* m_pDevice;

};
extern CDX8Engine g_DX8;

//=========================================================================
//	a directx 9 capture class
//=========================================================================
interface IDirect3DDevice9;
struct CDX9Engine : public CDXEngine
{
public:
	CDX9Engine() : m_pDevice( NULL ) { m_pszModuleName = TEXT("d3d9.dll"); }

	virtual bool GetOffsets();
	virtual bool HookFunctions();
	virtual void UnhookFunctions();

	virtual HRESULT GetFrame();
	
public:
	IDirect3DDevice9* m_pDevice;
};

extern CDX9Engine g_DX9;

//=========================================================================
//	a directx 10 capture class
//=========================================================================
interface ID3D10Device;
interface IDXGISwapChain;
struct CDX10Engine : public CDXEngine
{
public:
	CDX10Engine() : m_pDevice( NULL ) {  m_pszModuleName = TEXT("d3d10.dll"); }

	virtual bool GetOffsets();
	virtual bool HookFunctions();
	virtual void UnhookFunctions();

	virtual HRESULT GetFrame();
	
public:
	ID3D10Device*		m_pDevice;
	IDXGISwapChain*		m_pSwapChain;
};

extern CDX10Engine g_DX10;

//=========================================================================
//	a directx 11 capture class
//=========================================================================
interface ID3D11Device;
interface IDXGISwapChain;
struct CDX11Engine : public CDXEngine
{
public:
	CDX11Engine() : m_pDevice( NULL ) {  m_pszModuleName = TEXT("d3d11.dll"); }

	virtual bool GetOffsets();
	virtual bool HookFunctions();
	virtual void UnhookFunctions();

	virtual HRESULT GetFrame();
	
public:
	ID3D11Device*		m_pDevice;
	IDXGISwapChain*		m_pSwapChain;
};

extern CDX11Engine g_DX11;

//=========================================================================
//	a OpenGL capture class
//=========================================================================

struct COGLEngine : public CEngine
{
public:
	COGLEngine(){  m_pszModuleName = TEXT("opengl32.dll"); };

	virtual bool HookFunctions();
	virtual void UnhookFunctions();

	virtual HRESULT GetFrame();

public:
	CHookJump	m_Hook_SwapBuffers;
	CHookJump	m_Hook_ViewPort;
};
extern COGLEngine g_OGL;

//=========================================================================


