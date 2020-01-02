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
// altURI_Hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

IplImage* Ipl_Resized;
IplImage* Ipl_Legacy;
FrameData fData = {FRAME_OK, 0, 640, 480, 921601, NULL };

//=======================================================================
bool SetupSharedImage()
{
	g_MyImage.SetMapName( g_DllState.m_UUID );
	g_MyImage.AccessSharedIplImage(	g_DllState.m_SourceImageStats.m_height,
									g_DllState.m_SourceImageStats.m_width );

	return true;
}

//=======================================================================
IplImage* GetNewIplImageWhenRequired( IplImage* iplOld, const int width, const int height )
{
	if ( !CV_IS_IMAGE( iplOld ) || height != iplOld->height || width != iplOld->width )
	{
		IplImage* iplImageTemp =  cvCreateImage( cvSize( width, height ), iplOld->depth, iplOld->nChannels);
		iplImageTemp->origin = iplOld->origin;
		return iplImageTemp;
	}

	return iplOld;
}

//=======================================================================
IplImage* GetResizedIplCopy( IplImage* iplIn, const int width, const int height, const int x, const int y )
{
	IplImage* iplImageFinal = NULL;
	
	// if using offsets to define a region e.g. multiview
	if ( x != 0 || y != 0 )
	{
		int iRegionWidth, iRegionHeight;

		// make size of region within image
		if ( ( x + width ) > iplIn->width  )
			iRegionWidth = iplIn->width - x;
		else
			iRegionWidth = ( width < 1 ? g_DllState.m_SourceImageStats.m_width : width );;

		if ( ( y + height ) > iplIn->height )
			iRegionHeight = iplIn->height - y;
		else
			iRegionHeight = ( height < 1 ? g_DllState.m_SourceImageStats.m_height : height );
		
		// make new image the size of ROI
		iplImageFinal = GetNewIplImageWhenRequired( iplIn, iRegionWidth, iRegionHeight );

		if ( Ipl_Resized != iplImageFinal ) cvReleaseImage( &Ipl_Resized );
		
		cvSetImageROI( iplIn, cvRect( x, y, iRegionWidth, iRegionHeight ) );
		cvCopy( iplIn, iplImageFinal );
		cvResetImageROI( iplIn );

		return iplImageFinal;
	}

	// resize if required
	iplImageFinal = GetNewIplImageWhenRequired( iplIn, width, height );
	if ( Ipl_Resized != iplImageFinal ) cvReleaseImage( &Ipl_Resized );
	
	cvResize( iplIn, iplImageFinal );

	return iplImageFinal;
}

//=======================================================================
// exported functions for supplying image data
//=======================================================================
LIBSPEC IplImage* GetIplImage( const int width, const int height, const int x, const int y )
{
	// resise this dlls copy of image if not set or different
	if (	!CV_IS_IMAGE( g_MyImage.m_IplImage ) ||
			g_DllState.m_SourceImageStats.m_width != (UINT)g_MyImage.m_IplImage->width || 
			g_DllState.m_SourceImageStats.m_height != (UINT)g_MyImage.m_IplImage->height )
		if ( !SetupSharedImage() ) return NULL;

	// set we require a frame
	InterlockedExchange( &g_DllState.m_lFrameRequired, true );

	int iTime = 0;

	// loop while a frame required for up to 200ms
	while ( InterlockedCompareExchange( &g_DllState.m_lFrameRequired, false, false ) )
	{
		Sleep(1);
		iTime++;
		if ( iTime > 200 ) return NULL;
	}

	// if requesting same size image or height and width zero then return shared buffer
	if (	width == 0 || height == 0 || (width == g_DllState.m_SourceImageStats.m_width && height == g_DllState.m_SourceImageStats.m_height ) )
		return g_MyImage.m_IplImage;

	// otherwise resize to buffer
	Ipl_Resized = GetResizedIplCopy( g_MyImage.m_IplImage, width, height, x, y );

	return Ipl_Resized;
}

//=======================================================================
LIBSPEC FrameData* getFrameData()
{
	fData.state = FRAME_PENDING;

	Ipl_Legacy = GetIplImage( 640, 480 );

	if ( CV_IS_IMAGE( Ipl_Legacy ) )
	{
		// set pointer in legacy array
		*( fData.data ) = *( Ipl_Legacy->imageData );
		fData.state = FRAME_OK;
	}
	else
	{
		*( fData.data ) = NULL;
		fData.state = FRAME_ERROR;
	}

	return &fData;
}

//=======================================================================
LIBSPEC void Lock( void )
{
	g_MyImage.Lock();
}

//=======================================================================
LIBSPEC void Unlock( void )
{
	g_MyImage.Unlock();
}