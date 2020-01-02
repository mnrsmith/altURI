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
//	engine_dx9.cpp - DirectX9 graphics methods
//=========================================================================

#include "stdafx.h"
#include "engines.h"

#include <d3d9.h>

typedef IDirect3D9* (STDMETHODCALLTYPE *DIRECT3DCREATE9)(UINT);

CDX9Engine g_DX9;

//=========================================================================
// IDirect3DDevice9 method-types function pointers 
typedef ULONG   (STDMETHODCALLTYPE *PFN_DX9_ADDREF)(IDirect3DDevice9* pDevice);
static PFN_DX9_ADDREF s_D3D9_AddRef = NULL;
typedef ULONG   (STDMETHODCALLTYPE *PFN_DX9_RELEASE)(IDirect3DDevice9* pDevice);
static PFN_DX9_RELEASE s_D3D9_Release = NULL;
typedef HRESULT (STDMETHODCALLTYPE *PFN_DX9_RESET)(IDirect3DDevice9* pDevice, LPVOID);
static PFN_DX9_RESET   s_D3D9_Reset = NULL;
typedef HRESULT (STDMETHODCALLTYPE *PFN_DX9_PRESENT)(IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, LPVOID);
static PFN_DX9_PRESENT s_D3D9_Present = NULL;

//=========================================================================
static HRESULT GetFrameFullSize( D3DLOCKED_RECT& lockedSrcRect )
{
	if (	!CV_IS_IMAGE( g_MyImage.m_IplImage ) || 
			g_DllState.m_SourceImageStats.m_width != (UINT)g_MyImage.m_IplImage->width ||
			g_DllState.m_SourceImageStats.m_height != (UINT)g_MyImage.m_IplImage->height )
	{
		g_MyImage.Lock();
		g_DllState.m_UUID = g_MyImage.MakeAndSetMapName();
		g_MyImage.MakeSharedIplImage(	g_DllState.m_SourceImageStats.m_height,
										g_DllState.m_SourceImageStats.m_width );

		g_MyImage.Unlock();
	}

	int iCvType, iCvCode;

	switch (g_DllState.m_SourceImageStats.m_D3DFormat)
	{
		case D3DFMT_R8G8B8:
			iCvType = CV_8UC3;
			iCvCode = CV_RGBA2RGB;
			break;

		case D3DFMT_X1R5G5B5:
			iCvType = CV_8UC2;
			iCvCode = CV_BGR5552BGR;
			break;

		case D3DFMT_R5G6B5:
			iCvType = CV_8UC2;
			iCvCode = CV_BGR5652BGR;
			break;

		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			iCvType = CV_8UC4;
			iCvCode = CV_RGBA2RGB;
			break;

		//case D3DFMT_A2R10G10B10:
		//	iCvType = CV_16UC2;
		//	iCvCode = CV_BGR5552BGR;
		//	break;

		default:
			assert(false);
			return HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
	}

	CEngine::CopyAUImage( (void*)lockedSrcRect.pBits, iCvType, iCvCode );

	return S_OK;
}

//=========================================================================
HRESULT CDX9Engine::GetFrame()
{
	IRefPtr<IDirect3DSurface9>pBackBuffer;
	HRESULT hRes = m_pDevice->GetBackBuffer(	0,
												0,
												D3DBACKBUFFER_TYPE_MONO, 
												IREF_GETPPTR(pBackBuffer,IDirect3DSurface9));
	if ( FAILED(hRes) ) return hRes;

	D3DSURFACE_DESC desc;
	hRes = pBackBuffer->GetDesc(&desc);

	g_DllState.m_SourceImageStats.m_D3DFormat = desc.Format;

	// check dest.surface format
	switch (g_DllState.m_SourceImageStats.m_D3DFormat)
	{
		case D3DFMT_R8G8B8:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		//case D3DFMT_A2R10G10B10:
			break;
		default:
			return HRESULT_FROM_WIN32(ERROR_CTX_BAD_VIDEO_MODE);
	}

	g_DllState.m_SourceImageStats.m_width = desc.Width;
	g_DllState.m_SourceImageStats.m_height = desc.Height;

	IRefPtr<IDirect3DSurface9>pSurfTemp;
	hRes = m_pDevice->CreateOffscreenPlainSurface(	g_DllState.m_SourceImageStats.m_width,
													g_DllState.m_SourceImageStats.m_height, 
													(D3DFORMAT)g_DllState.m_SourceImageStats.m_D3DFormat,
													D3DPOOL_SYSTEMMEM, 
													IREF_GETPPTR(pSurfTemp,IDirect3DSurface9),
													NULL );
	if ( FAILED(hRes) ) return hRes;

	hRes = m_pDevice->GetRenderTargetData(pBackBuffer, pSurfTemp);

	if ( FAILED(hRes) ) return hRes;
	
	D3DLOCKED_RECT lockedSrcRect;
	RECT Rect = {0, 0, g_DllState.m_SourceImageStats.m_width, g_DllState.m_SourceImageStats.m_height};
	hRes = pSurfTemp->LockRect( &lockedSrcRect, &Rect, 0);
	
	if ( FAILED(hRes) ) return hRes;

	hRes = GetFrameFullSize( lockedSrcRect );

	if ( pSurfTemp )  pSurfTemp->UnlockRect();

	return hRes;
}

//=========================================================================
EXTERN_C ULONG _declspec(dllexport) STDMETHODCALLTYPE DX9_AddRef(IDirect3DDevice9* pDevice)
{
	g_DX9.m_pDevice = pDevice;

	g_DX9.m_iRefCount = s_D3D9_AddRef(pDevice);
	
	return g_DX9.m_iRefCount;
}

//=========================================================================
EXTERN_C ULONG _declspec(dllexport) STDMETHODCALLTYPE DX9_Release(IDirect3DDevice9* pDevice)
{
	g_DX9.m_pDevice = pDevice;

	if ( ( g_DX9.m_iRefCount > 1 ) && s_D3D9_Release ) 
	{
		g_DX9.m_iRefCount = s_D3D9_Release(pDevice);
		return g_DX9.m_iRefCount;
	}

	g_DX9.UnhookFunctions();	// unhook device methods

	// reset the pointers
	g_DX9.m_Hook_AddRef = NULL;
	g_DX9.m_Hook_Release = NULL;

	// call the real Release()
	g_DX9.m_iRefCount = s_D3D9_Release(pDevice);

	return g_DX9.m_iRefCount;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX9_Reset(
	IDirect3DDevice9* pDevice, LPVOID params )
{
	g_DX9.m_pDevice = pDevice;

	// put back saved code fragment
	g_DX9.m_Hook_Reset.DisableHook();

	//call real Reset() 
	HRESULT hRes = s_D3D9_Reset(pDevice, params);

	// put JMP instruction again
	g_DX9.m_Hook_Reset.EnableHook();

	// force CSharedImage buffer resize
	g_DllState.m_SourceImageStats.m_width--;

	return hRes;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX9_Present(
	IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND hWnd, LPVOID unused)
{
	g_DX9.m_pDevice = pDevice;
	g_DX9.m_Hook_Present.DisableHook();

	g_DX9.PresentFrame();

	// call real Present() 
	HRESULT hRes = s_D3D9_Present(pDevice, src, dest, hWnd, unused);

	g_DX9.m_Hook_Present.EnableHook();
	return hRes;
}

//=========================================================================
bool CDX9Engine :: GetOffsets()
{
	// Get IDirect3D9
	DIRECT3DCREATE9 pDirect3DCreate9 = (DIRECT3DCREATE9)GetProcAddress(m_hModule, "Direct3DCreate9");
	if ( pDirect3DCreate9 == NULL ) return false;

	LPDIRECT3D9 pD3D = pDirect3DCreate9(D3D_SDK_VERSION);
	if ( !pD3D ) return false;

	// Get IDirect3DDevice9
    D3DDISPLAYMODE d3ddm;
	HRESULT hRes = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm );
    if ( FAILED(hRes) )  return false;

	HWND hwTemp = CreateDeviceWindow();

	if ( !hwTemp ) return NULL;

    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = d3ddm.Format;	// D3DFMT_UNKNOWN 
	d3dpp.hDeviceWindow = hwTemp;

	LPDIRECT3DDEVICE9 pD3DDevice;
	hRes = pD3D->CreateDevice(	D3DADAPTER_DEFAULT,
								D3DDEVTYPE_HAL,
								// g_DllState.m_hWnd,
								hwTemp,
								D3DCREATE_SOFTWARE_VERTEXPROCESSING,
								&d3dpp,
								&pD3DDevice );

    if ( FAILED(hRes) )  return false;

	// store method addresses in our vars
	UINT_PTR* pVTable = (UINT_PTR*)(*((UINT_PTR*)pD3DDevice));
	assert(pVTable);
	g_DllState.nDX_Present = pVTable[INTF_DX9_Present] - get_DllInt();
	g_DllState.nDX_Reset = pVTable[INTF_DX9_Reset] - get_DllInt();

    //SAFE_RELEASE( pD3DDevice ) 
	SAFE_RELEASE( pD3D )

	::SendMessage( hwTemp, WM_SYSCOMMAND, SC_CLOSE, 0);

	return ( g_DllState.nDX_Present && g_DllState.nDX_Reset ) ;
}

//=========================================================================
bool CDX9Engine::HookFunctions()
{
	// This function hooks two IDirect3DDevice9 methods, using code overwriting technique. 
	// hook IDirect3DDevice9::Present(), using code modifications at run-time.
	// ALGORITHM: we overwrite the beginning of real IDirect3DDevice9::Present
	// with a JMP instruction to our routine (DX9_Present).
	// When our routine gets called, first thing that it does - it restores 
	// the original bytes of IDirect3DDevice9::Present, then performs its pre-call tasks, 
	// then calls IDirect3DDevice9::Present, then does post-call tasks, then writes
	// the JMP instruction back into the beginning of IDirect3DDevice9::Present, and
	// returns.

	if (!GetOffsets()) return false;

	s_D3D9_Present = (PFN_DX9_PRESENT)(get_DllInt() + g_DllState.nDX_Present);
	s_D3D9_Reset = (PFN_DX9_RESET)(get_DllInt() + g_DllState.nDX_Reset);

	m_Hook_Present.InstallHook( s_D3D9_Present, DX9_Present );
	m_Hook_Reset.InstallHook( s_D3D9_Reset, DX9_Reset );

	return CEngine :: HookFunctions();
}

//=========================================================================
void CDX9Engine::UnhookFunctions()
{
	// Restore original Reset() and Present()
	if ( m_hModule == NULL ) return;

	// restore IDirect3D9Device methods
	m_Hook_Present.RemoveHook();
	m_Hook_Reset.RemoveHook();

	CEngine :: UnhookFunctions();
}




