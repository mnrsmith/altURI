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
//	CSharedImage - shared File Mapped image
//=========================================================================
#pragma once

#pragma comment(lib, "Rpcrt4.lib")
#include "rpc.h"

#define MY_MUTEX_NAME TEXT("HookMutex")

struct CSharedImage
{
public:
	CSharedImage() :  m_IplImage( cvCreateImageHeader( cvSize(100,100), IPL_DEPTH_8U, 3) ), 
	  m_hMapObject( NULL ), m_lpvMem( NULL ), m_pitch( 0 ), m_dwSize( 0 ), m_sGuid( TEXT( "" ) )
	{
		m_hMutex = CreateMutex( NULL, FALSE, MY_MUTEX_NAME );
		assert( m_hMutex );
	}

	~CSharedImage() { cvReleaseImageHeader( &m_IplImage ); Free(); CloseHandle(m_hMutex); }

	CSharedImage& operator= (const CSharedImage& f) { assert(false); f; }	// not allowed

	UUID MakeAndSetMapName( )
	{
		UUID my_uuid;

		UuidCreate( &my_uuid );

		return SetMapName( my_uuid );
	}

	UUID SetMapName( UUID new_uuid )
	{
		TCHAR* tTemp = NULL;

		UuidToString( &new_uuid, (RPC_WSTR*)&tTemp );

		m_sGuid.assign( tTemp );

		RpcStringFree( (RPC_WSTR*)&tTemp );

		return new_uuid;
	}

	void MakeSharedIplImage( const int height, const int width )
	{
		m_pitch = GetPitch( width );
		MakeImage( height, width, (char*)Alloc( m_pitch * height ) );
	}

	void AccessSharedIplImage( const int height, const int width )
	{
		m_pitch = GetPitch( width );
		MakeImage( height, width, (char*)GetRef( m_pitch * height ) );
	}

	inline bool Lock(void)
	{
		return ( WaitForSingleObject( m_hMutex, INFINITE ) == WAIT_OBJECT_0 );
	}

	inline void Unlock(void)
	{
		ReleaseMutex(m_hMutex);
	}


private:
	void MakeImage( const int height, int const width, const char* ref )
	{
		cvInitImageHeader( m_IplImage, cvSize( width, height ), IPL_DEPTH_8U, 3 );

		m_IplImage->imageData = (char*)ref;
		m_IplImage->imageDataOrigin = m_IplImage->imageData;
	}
	
	// get size of line for an OpenCV image - should use widthstep?
	int GetPitch( const int width, const int BitsPerPixel = 3, const int PaddingPerPixel = sizeof(DWORD) )
	{
		int iPitchUn = width * BitsPerPixel;
		int iRem = iPitchUn % PaddingPerPixel;
		return (iRem == 0) ? iPitchUn : (iPitchUn + PaddingPerPixel - iRem);
	}

	LPVOID Alloc( const DWORD size )
	{
		assert( m_sGuid.size() >0 );		// check map name defined
		
		Free();

		m_dwSize = size;

		m_hMapObject = CreateFileMapping(	INVALID_HANDLE_VALUE,   // use paging file
											NULL,                   // default security attributes
											PAGE_READWRITE,         // read/write access
											0,                      // size: high 32-bits
											m_dwSize,				// size: low 32-bits
											m_sGuid.c_str() );		// name of map object
           
		if (m_hMapObject == NULL) return NULL;
		
		//SetNamedSecurityInfo(	MY_MAP_NAME,
		//						SE_KERNEL_OBJECT,
		//						DACL_SECURITY_INFORMATION,
		//						0,
		//						0,
		//						(PACL) NULL,
		//						NULL);

		m_lpvMem = MapViewOfFile(	m_hMapObject,   // object to map view of
									FILE_MAP_WRITE, // read/write access
									0,              // high offset:  map from
									0,              // low offset:   beginning
									0);             // default: map entire file

        if (m_lpvMem == NULL)
		{
			CloseHandle(m_hMapObject);
			m_hMapObject = NULL;
			return NULL; 
		}
 
		ZeroMemory( m_lpvMem, m_dwSize );

		return m_lpvMem;
	}

	LPVOID GetRef( const DWORD size )
	{
		assert( m_sGuid.size() >0 );	// check mapname assigned
		
		m_dwSize = size;

		m_hMapObject = OpenFileMapping(	FILE_MAP_READ,			// read/write access
										FALSE,					// do not inherit the name
										m_sGuid.c_str() );			// name of mapping object 

		if (m_hMapObject == NULL) return NULL;

		m_lpvMem = MapViewOfFile(	m_hMapObject,	// handle to map object
									FILE_MAP_READ,  // read/write permission
									0,                    
									0,                    
									m_dwSize);                   

        if (m_lpvMem == NULL)
		{
			CloseHandle(m_hMapObject); 
			m_hMapObject = NULL;
			return NULL; 
		}

		return m_lpvMem;
   }
	
	void Free()
	{
		if (m_lpvMem)
		{
			UnmapViewOfFile(m_lpvMem);
			m_lpvMem = NULL;
		}

		if (m_hMapObject)
		{
			CloseHandle(m_hMapObject);
			m_hMapObject = NULL;
		}
		m_dwSize = 0;
	}

public:
	int			m_pitch;
	DWORD		m_dwSize;
	IplImage*	m_IplImage;
	CvMat		m_cvmBuf;	
	CvMat		m_cvmBuf1;	

private:
	HANDLE		m_hMapObject;
	tString		m_sGuid;
	HANDLE		m_hMutex;
	LPVOID		m_lpvMem;
};
