// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
//#include <assert.h>

// TODO: reference additional headers your program requires here

#if defined(_DEBUG)
#pragma comment(lib, "opencv_core242d")
#pragma comment(lib, "opencv_imgproc242d")
#pragma comment(lib, "opencv_highgui242d")
#else
#pragma comment(lib, "opencv_core242")
#pragma comment(lib, "opencv_imgproc242")
#pragma comment(lib, "opencv_highgui242")
#endif

// remove warning for 'deprecated' functions
#pragma warning(disable : 4996)

#include <string>
typedef std::basic_string <TCHAR> tString;

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "altURI_Hook.h"
#include "IRefPtr.h"
#include "CLock.h"
#include "CSharedImage.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

enum INTF_TYPE
{
	// DX interface entry offsets.
	INTF_QueryInterface = 0,	// always 0
	INTF_AddRef = 1,
	INTF_Release = 2,

	INTF_DX8_Reset = 14,
	INTF_DX8_Present = 15,

	INTF_DX9_Reset = 16,
	INTF_DX9_Present = 17,
	
	INTF_DX10_ResizeBuffers = 13,
	INTF_DX10_Present = 8,

	INTF_DX11_ResizeBuffers = 13,
	INTF_DX11_Present = 8
};


struct ISTATS
{
	UINT	m_width;
	UINT	m_height;
	int		m_D3DFormat;
};

struct LIBSPEC CDllState
{
public:
	static enum GRAPHICS_TYPE { NONE, DX8, DX9, DX10, DX11, OGL };

	GRAPHICS_TYPE g_Graphics_Type;		// current graphics API

	ISTATS	m_SourceImageStats;

	// keep Direct3D-related pointers, tested just once for all.
	UINT_PTR nDX_Present; // offset from start of DLL to the interface element we want.
	UINT_PTR nDX_Reset;

	long			m_lFrameRequired;	// A frame has been requested 

	UUID	m_UUID;
};

extern LIBSPEC CDllState g_DllState;
extern CSharedImage g_MyImage;





