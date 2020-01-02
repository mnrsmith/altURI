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
//	engine_dx11.cpp - DirectX11 graphics methods
//=========================================================================

#include "stdafx.h"
#include "engines.h"

#include <d3d11.h>
#include <DXGI.h>

#define CV_MAT_ELEM_CN( mat, elemtype, row, col ) \
    (*(elemtype*)((mat).data.ptr + (size_t)(mat).step*(row) + sizeof(elemtype)*(col)))

typedef HRESULT (STDMETHODCALLTYPE *D3D11CREATEDEVICEANDSWAPCHAIN)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

CDX11Engine g_DX11;

//=========================================================================
// ID3D11Device method-types function pointers 
typedef HRESULT (STDMETHODCALLTYPE *PFN_DX11_RESIZEBUFFERS)(IDXGISwapChain * This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
static PFN_DX11_RESIZEBUFFERS   s_D3D11_ResizeBuffers = NULL;

typedef HRESULT (STDMETHODCALLTYPE *PFN_DX11_PRESENT)(IDXGISwapChain *This, UINT SyncInterval, UINT Flags);
static PFN_DX11_PRESENT s_D3D11_Present = NULL;

//=========================================================================
static HRESULT GetFrameFullSize( D3D11_MAPPED_SUBRESOURCE& lockedSrcRect )
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
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			iCvType = CV_8UC3;
			iCvCode = CV_BGR5552BGR;
			break;

		case DXGI_FORMAT_B5G6R5_UNORM:
			iCvType = CV_8UC3;
			iCvCode = CV_BGR5652BGR;
			break;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
			iCvType = CV_8UC4;
			iCvCode = CV_RGBA2RGB;
			break;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
			iCvType = CV_8UC4;
			iCvCode = CV_RGBA2BGR;
			break;

		default:
			assert(false);
			return HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
	}
	
	if ( g_DllState.m_SourceImageStats.m_width % g_MyImage.m_IplImage->widthStep > 0 )
	{
		uchar* p = (uchar*)lockedSrcRect.pData;

		g_MyImage.Lock();

		for( int row = 0; row < g_MyImage.m_IplImage->height; row++ )
		{
			for ( int col = 0; col < g_MyImage.m_IplImage->width; col++ )
			{
				CV_IMAGE_ELEM( g_MyImage.m_IplImage, uchar, row, col * 3 )= p[row*lockedSrcRect.RowPitch + col*4 + 2];
				CV_IMAGE_ELEM( g_MyImage.m_IplImage, uchar, row, col * 3 + 1) = p[row*lockedSrcRect.RowPitch + col*4 + 1];
				CV_IMAGE_ELEM( g_MyImage.m_IplImage, uchar, row, col * 3 + 2) = p[row*lockedSrcRect.RowPitch + col*4 ];
			}
		}

		g_MyImage.Unlock();
	}
	else
	{
		CEngine::CopyAUImage( (void*)lockedSrcRect.pData, iCvType, iCvCode );
	}

	/*
	// 32-bit entries: discard alpha
	for (i=0, k=height-1; i<height, k>=0; i++, k--)
	{
		for (int j=0; j<width; j++)
		{
			// CV_IMAGE_ELEM( image_header, elemtype, y, x_Nc )
			g_MyImage.m_IplImage->imageData[i*g_MyImage.OCV_PITCH + j*3] = pSrcRow[k*iSrcPitch + j*4];
			g_MyImage.m_IplImage->imageData[i*g_MyImage.OCV_PITCH + j*3 + 1] = pSrcRow[k*iSrcPitch + j*4 + 1];
			g_MyImage.m_IplImage->imageData[i*g_MyImage.OCV_PITCH + j*3 + 2] = pSrcRow[k*iSrcPitch + j*4 + 2];
		}
	}
	*/

	// signal we have frame
	InterlockedExchange( &g_DllState.m_lFrameRequired, false );

	return S_OK;
}

//=========================================================================
HRESULT CDX11Engine::GetFrame()
{
	// Get back buffer
	IRefPtr<ID3D11Texture2D> pBackBuffer;
	HRESULT hRes = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)IREF_GETPPTR(pBackBuffer,ID3D11Texture2D) );

	// m_pSwapChain->GetDevice( IID_ID3D10Device, (void**)&m_pDevice );
	m_pSwapChain->GetDevice( __uuidof( ID3D11Device ), (void**)&m_pDevice );
	
	if ( FAILED(hRes) ) return hRes;
	
	// Change surface description to be staging buffer
	D3D11_TEXTURE2D_DESC surfaceDescription;
	pBackBuffer->GetDesc( &surfaceDescription );

	g_DllState.m_SourceImageStats.m_D3DFormat = surfaceDescription.Format;

	// check dest.surface format
	switch(g_DllState.m_SourceImageStats.m_D3DFormat)
	{
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		break;
	default:
		return HRESULT_FROM_WIN32(ERROR_CTX_BAD_VIDEO_MODE);	// unsupported format
	}

	surfaceDescription.MipLevels = 1;
    surfaceDescription.ArraySize = 1;
    surfaceDescription.SampleDesc.Count = 1;
    surfaceDescription.Usage = D3D11_USAGE_STAGING;
    surfaceDescription.BindFlags = 0;
	surfaceDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    surfaceDescription.MiscFlags = 0;
	
	g_DllState.m_SourceImageStats.m_width = surfaceDescription.Width;
	g_DllState.m_SourceImageStats.m_height = surfaceDescription.Height;

	// Create staging buffer and copy backbuffer into it
	IRefPtr<ID3D11Texture2D> pBackBufferCopy;
	hRes = m_pDevice->CreateTexture2D( &surfaceDescription, NULL, IREF_GETPPTR(pBackBufferCopy,ID3D11Texture2D)  );

	if ( FAILED(hRes) ) return hRes;

	IRefPtr<ID3D11DeviceContext> pDeviceContext;

	m_pDevice->GetImmediateContext( IREF_GETPPTR(pDeviceContext,ID3D11DeviceContext) );

	if ( FAILED(hRes) ) return hRes;

	pDeviceContext->CopyResource( (ID3D11Resource *)pBackBufferCopy, (ID3D11Resource *)pBackBuffer );

	// Lock the back buffer copy
	D3D11_MAPPED_SUBRESOURCE lockedSrcRect;
	hRes = pDeviceContext->Map( pBackBufferCopy, 0, D3D11_MAP_READ, NULL, &lockedSrcRect );

	if ( FAILED(hRes) ) return hRes;

	hRes = GetFrameFullSize( lockedSrcRect );

	// Unlock the back buffer
	pDeviceContext->Unmap( pBackBufferCopy, 0 );

	return hRes;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX11_ResizeBuffers(
	IDXGISwapChain * This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	g_DX11.m_pSwapChain  = This;

	// put back saved code fragment
	g_DX11.m_Hook_Reset.DisableHook();

	//call real ResizeBuffers() 
	HRESULT hRes = s_D3D11_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	// put JMP instruction again
	g_DX11.m_Hook_Reset.EnableHook();

	// force CSharedImage buffer resize
	g_DllState.m_SourceImageStats.m_width--;

	return hRes;
}

//=========================================================================
EXTERN_C HRESULT _declspec(dllexport) STDMETHODCALLTYPE DX11_Present(
	IDXGISwapChain *This, UINT SyncInterval, UINT Flags)
{
	g_DX11.m_pSwapChain  = This;

	g_DX11.m_Hook_Present.DisableHook();

	g_DX11.PresentFrame();

	// call real Present() 
	HRESULT hRes = s_D3D11_Present(This, SyncInterval, Flags);

	g_DX11.m_Hook_Present.EnableHook();

	return hRes;
}

//=========================================================================
bool CDX11Engine :: GetOffsets()
{
	ID3D11Device* pD3DDevice = NULL;
	IDXGISwapChain* pD3DSwapChain = NULL; 
	ID3D11DeviceContext *pD3DDeviceContext = NULL;

	D3D11CREATEDEVICEANDSWAPCHAIN pD3D11CreateDeviceAndSwapChain = (D3D11CREATEDEVICEANDSWAPCHAIN)GetProcAddress(m_hModule, "D3D11CreateDeviceAndSwapChain");
	if ( pD3D11CreateDeviceAndSwapChain == NULL ) return false;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	//sd.BufferDesc.Width = width;
	//sd.BufferDesc.Height = height;
	sd.BufferDesc.Format =  DXGI_FORMAT_R8G8B8A8_UNORM; 
	sd.BufferDesc.RefreshRate.Numerator = 0;		// leave at users rate
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage= DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// sd.OutputWindow = g_DllState.m_hWnd;
	sd.OutputWindow = CreateDeviceWindow();

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = NULL;

	HRESULT hRes = 	pD3D11CreateDeviceAndSwapChain(	NULL,
													D3D_DRIVER_TYPE_HARDWARE,
													NULL,
													0,
													&FeatureLevels,
													1,
													D3D11_SDK_VERSION,
													&sd,
													&pD3DSwapChain,
													&pD3DDevice,
													pFeatureLevel,
													&pD3DDeviceContext );

	if ( FAILED(hRes) )  return false;

	// store method addresses in out vars
	UINT_PTR* pVTable = (UINT_PTR*)(*((UINT_PTR*)pD3DSwapChain));
	assert(pVTable);
	g_DllState.nDX_Present = ( pVTable[INTF_DX11_Present] - get_DllInt());
	g_DllState.nDX_Reset = ( pVTable[INTF_DX11_ResizeBuffers] - get_DllInt());

	SAFE_RELEASE( pD3DSwapChain ) 
	SAFE_RELEASE( pD3DDevice )

	::SendMessage( sd.OutputWindow, WM_SYSCOMMAND, SC_CLOSE, 0);

	return ( g_DllState.nDX_Present && g_DllState.nDX_Reset ) ;
}
//=========================================================================
bool CDX11Engine::HookFunctions()
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

	s_D3D11_Present = (PFN_DX11_PRESENT)(((UINT_PTR) m_hModule) + g_DllState.nDX_Present);
	s_D3D11_ResizeBuffers = (PFN_DX11_RESIZEBUFFERS)(((UINT_PTR) m_hModule) + g_DllState.nDX_Reset);

	m_Hook_Present.InstallHook( s_D3D11_Present, DX11_Present );
	m_Hook_Reset.InstallHook( s_D3D11_ResizeBuffers, DX11_ResizeBuffers );

	return CEngine :: HookFunctions();
}

//=========================================================================
void CDX11Engine::UnhookFunctions()
{
	// Restore original Reset() and Present()
	if ( m_hModule == NULL ) return;

	// restore IDirect3D9Device methods
	m_Hook_Present.RemoveHook();
	m_Hook_Reset.RemoveHook();

	CEngine :: UnhookFunctions();
}



