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
//	altURI_Hook.h - internal and external declarations for image access
//=========================================================================
#pragma once


#ifdef ALTURI_HOOK_EXPORTS
#define LIBSPEC __declspec(dllexport)
#else
#define LIBSPEC __declspec(dllimport)
#endif // ALTURI_HOOK_EXPORTS

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

	typedef IplImage* (CALLBACK* pfGetIplImage)( void );

	LIBSPEC IplImage* GetIplImage( const int width = 0, const int height = 0, const int x = 0, const int y = 0 );

	LIBSPEC void Lock( void );

	LIBSPEC void Unlock( void );

	LIBSPEC FrameData* getFrameData( void );

#ifdef __cplusplus
}
#endif


