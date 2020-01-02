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
//	engine_dx10.cpp - DirectX10 graphics methods
//=========================================================================

#include "stdafx.h"
#include "engines.h"

#include <d3d10.h>
#include <DXGI.h>

typedef HRESULT (STDMETHODCALLTYPE *D3D10CREATEDEVICEANDSWAPCHAIN)(IDXGIAdapter *pAdapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion, DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain, ID3D10Device **ppDevice);

CDX10Engine g_DX10;

//=========================================================================
// ID3D10Device method-types function pointers 

typedef HRESULT (STDMETHODCALLTYPE *PFN_DX10_RESIZEBUFFERS)(IDXGISwapChain *This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
static PFN_DX10_RESIZEBUFFERS   s_D3D10_ResizeBuffers = NULL;

typedef HRESULT (STDMETHODCALLTYPE *PFN_DX10_PRESENT)(IDXGISwapChain *This, UINT SyncInterval, UINT Flags);
static PFN_DX10_PRESENT s_D3D10_Present = NULL;

//=========================================================================
static HRESULT GetFrameFullSize( D3D10_MAPPED_TEXTURE2D& lockedSrcRect )
{
	if (	g_DllState.m_SourceImageStats.m_width != (UINT)g_MyImage.m_IplImage->width ||
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
		//case D3DFMT_R8G8B8:
		//	iCvType = CV_8UC3;
		//	iCvCode = CV_RGBA2RGB;
		//	break;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			iCvType = CV_8UC2;
			iCvCode = CV_BGR5552BGR;
			break;

		case DXGI_FORMAT_B5G6R5_UNORM:
			iCvType = CV_8UC2;
			iCvCode = CV_BGR5652BGR;
			break;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			iCvType = CV_8UC4;
			iCvCode = CV_RGBA2RGB;
			break;

		default:
			assert(false);
			return HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
	}
	
	CEngine::CopyAUImage( (void*)lockedSrcRect.pData, iCvType, iCvCode );

	return S_OK;
}

//=========================================================================
HRESULT CDX10Engine::GetFrame()
{
	// Get back buffer
	IRefPtr<ID3D10Texture2D> pBackBuffer;
	HRESULT hRes = m_pSwapChain->GetBuffer(	0,
											__uuidof( ID3D10Texture2D ),
											(void**)IREF_GETPPTR(pBackBuffer,ID3D10Texture2D) );

	m_pSwapChain->GetDevice( __uuidof( ID3D10Device ), (void**)&m_pDevice );
	
	if ( FAILED(hRes) ) return hRes;
	
	// Change surface description to be staging buffer
	D3D10_TEXTURE2D_DESC surfaceDescription;
	pBackBuffer->GetDesc( &surfaceDescription );

	g_DllState.m_SourceImageStats.m_D3DFormat = surfaceDescription.Format;

	// check dest.surface format
	switch(g_DllState.m_SourceImageStats.m_D3DFormat)
	{
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		break;
	default:
		return HRESULT_FROM_WIN32(ERROR_CTX_BAD_VIDEO_MODE);	// unsupported format
	}

	surfaceDescription.MipLevels = 1;
    surfaceDescription.ArraySize = 1;
    surfaceDescription.SampleDesc.Count = 1;
    surfaceDescription.Usage = D3D10_USAGE_STAGING;
    surfaceDescription.BindFlags = 0;
	surfaceDescription.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    surfaceDescription.MiscFlags = 0;
	
	g_DllState.m_SourceImageStats.m_width = surfaceDescription.Width;
	g_DllState.m_SourceImageStats.m_height = surfaceDescription.Height;

	// Create staging buffer and copy backbuffer into it
	IRefPtr<ID3D10Texture2D> pBackBufferCopy;
	hRes = m_pDevice->CreateTexture2D( &surfaceDescription, NULL, IREF_GETPPTR(pBackBufferCopy,ID3D10Texture2D)  );

	if ( FAILED(hRes) ) return hRes;

	m_pDevice->CopyResource( (ID3D10Resource *)pBackBufferCopy, (ID3D10Resource *)pBackBuffer );

	// Lock the back buffer copy
	D3D10_MAPPED_TEXTURE2D lockedSrcRect;
	hRes = pBackBufferCopy->Map( 0, D3D10_MAP_READ, NULL, &lockedSrcRect );

	if ( FAILED(hRes) ) return hRes;

	hRes = GetFrameFullSize( lockedSrcRect );

	// Unlock the back buffer
	pBackBufferCopy->Unmap( 0 );

	return hRes;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX10_ResizeBuffers(
	IDXGISwapChain * This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	g_DX10.m_pSwapChain  = This;

	// put back saved code fragment
	g_DX10.m_Hook_Reset.DisableHook();

	//call real Present() 
	HRESULT hRes = s_D3D10_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	// put JMP instruction again
	g_DX10.m_Hook_Reset.EnableHook();

	// force CSharedImage buffer resize
	g_DllState.m_SourceImageStats.m_width--;

	return hRes;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX10_Present(
	IDXGISwapChain *This, UINT SyncInterval, UINT Flags)
{
	g_DX10.m_pSwapChain  = This;

	g_DX10.m_Hook_Present.DisableHook();

	g_DX10.PresentFrame();

	// call real Present() 
	HRESULT hRes = s_D3D10_Present(This, SyncInterval, Flags);

	g_DX10.m_Hook_Present.EnableHook();

	return hRes;
}



//=========================================================================
bool CDX10Engine :: GetOffsets()
{
	ID3D10Device* pD3DDevice;
	IDXGISwapChain* pD3DSwapChain; 

	D3D10CREATEDEVICEANDSWAPCHAIN pD3D10CreateDeviceAndSwapChain = (D3D10CREATEDEVICEANDSWAPCHAIN)GetProcAddress(m_hModule, "D3D10CreateDeviceAndSwapChain");
	if ( pD3D10CreateDeviceAndSwapChain == NULL ) return false;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	//sd.BufferDesc.Width = width;
	//sd.BufferDesc.Height = height;
	sd.BufferDesc.Format =  DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	sd.BufferDesc.RefreshRate.Numerator = 0;	// leave at users rate
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage= DXGI_USAGE_BACK_BUFFER;

	sd.OutputWindow = CreateDeviceWindow();

	// sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT hRes = pD3D10CreateDeviceAndSwapChain(	NULL,
													D3D10_DRIVER_TYPE_HARDWARE,
													NULL,
													0,
													D3D10_SDK_VERSION,
													&sd,
													&pD3DSwapChain,
													&pD3DDevice);
	if ( FAILED(hRes) )
	{
		hRes = pD3D10CreateDeviceAndSwapChain(	NULL,
														D3D10_DRIVER_TYPE_REFERENCE,
														NULL,
														0,
														D3D10_SDK_VERSION,
														&sd,
														&pD3DSwapChain,
														&pD3DDevice);
	}

	if ( FAILED(hRes) )  return false;

	// store method addresses in out vars
	UINT_PTR* pVTable = (UINT_PTR*)(*((UINT_PTR*)pD3DSwapChain));
	assert(pVTable);
	g_DllState.nDX_Present = ( pVTable[INTF_DX10_Present] - get_DllInt());
	g_DllState.nDX_Reset = ( pVTable[INTF_DX10_ResizeBuffers] - get_DllInt());

	SAFE_RELEASE( pD3DSwapChain ) 
	SAFE_RELEASE( pD3DDevice )

	::SendMessage( sd.OutputWindow, WM_SYSCOMMAND, SC_CLOSE, 0);

	return ( g_DllState.nDX_Present && g_DllState.nDX_Reset ) ;
}
//=========================================================================
bool CDX10Engine::HookFunctions()
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
	
	// if (!FindProcessWindow()) return false;

	if (!GetOffsets()) return false;
	
	s_D3D10_Present = (PFN_DX10_PRESENT)(((UINT_PTR) m_hModule) + g_DllState.nDX_Present);
	s_D3D10_ResizeBuffers = (PFN_DX10_RESIZEBUFFERS)(((UINT_PTR) m_hModule) + g_DllState.nDX_Reset);

	m_Hook_Present.InstallHook( s_D3D10_Present, DX10_Present );
	m_Hook_Reset.InstallHook( s_D3D10_ResizeBuffers, DX10_ResizeBuffers );

	return CEngine :: HookFunctions();
}

//=========================================================================
void CDX10Engine::UnhookFunctions()
{
	// Restore original Reset() and Present()
	if ( m_hModule == NULL ) return;

	// restore IDirect3D9Device methods
	m_Hook_Present.RemoveHook();
	m_Hook_Reset.RemoveHook();

	CEngine :: UnhookFunctions();
}



