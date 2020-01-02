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

// Uses code from: http://www.codeproject.com/KB/graphics/PreviewCtrl.aspx

//=========================================================================
//	COpenCVImage.cpp - implementation of control to display an OCVG Image
//=========================================================================

#include "stdafx.h"
#include "CaltURIApp.h"
#include "COpenCVImage.h"


// ------------
// CPreviewRect
// ------------
/**
 * The standard-constructor.
 */
COpenCVImage::COpenCVImage()
{
	// initialize pointers
	m_bitmap = 0;
	m_visible = 0;

	m_bmi = (BITMAPINFO*)m_buffer;
	m_bmih = &(m_bmi->bmiHeader);

	ZeroMemory( m_bmih, sizeof(BITMAPINFOHEADER));

	m_bmih->biSize = sizeof(BITMAPINFOHEADER);
	m_bmih->biPlanes = 1;
	m_bmih->biCompression = BI_RGB;

	// initialize all other members
	Reset(FALSE);
}


// -------------
// ~COpenCVImage
// -------------
/**
 * The destructor frees the allocated memory.
 */
COpenCVImage::~COpenCVImage()
{
	// free memory
	Reset(FALSE);
}


// -----------------
// BEGIN_MESSAGE_MAP
// -----------------
/**
 * 
 */
BEGIN_MESSAGE_MAP(COpenCVImage, CStatic)
	//{{AFX_MSG_MAP(CPreviewRect)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// -------
// OnPaint
// -------
/**
 * WM_PAINT handler.
 */
void COpenCVImage::OnPaint() 
{
	CPaintDC dc(this);

	doDrawBitmap();

	doDrawBackground();
}


// ------
// OnSize
// ------
/**
 * WM_SIZE handler.
 */
void COpenCVImage::OnSize(UINT nType, int cx, int cy) 
{
	if (m_bitmap)
	{
		if (doCheckZoom() || (!m_visible))
		{
			delete m_visible;

			m_visible = doZoomBitmap(m_bitmap, CRect(0, 0, cx, cy));
		}
	}
	UNREFERENCED_PARAMETER(nType);
}


// ----------
// LoadBitmap
// ----------
/**
 * Loads a bmp file.
 */
void COpenCVImage::LoadBitmap(const CString& Path)
{
	// reset first
	Reset(FALSE);

	CBitmap* attempt = new CBitmap;

	HBITMAP hbmp = (HBITMAP)LoadImage(0, Path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (hbmp && attempt->Attach(hbmp))
	{
		m_bitmap = attempt;

		// take ownership
		m_borrowed = false;

		CRect client;
		GetClientRect(&client);

		// zoom to client rect
		m_visible = doZoomBitmap(m_bitmap, client);
	}

	else
	{
		delete attempt;
	}

	// show image
	doRequestGraphicalUpdate();
}


// ----------
// LoadBitmap
// ----------
/**
 * Loads a bitmap resource.
 */
void COpenCVImage::LoadBitmap(UINT nIDResource)
{
	// reset first
	Reset(FALSE);

	// create new bitmap
	m_bitmap = new CBitmap;

	// take ownership
	m_borrowed = false;

	// initialize bitmap
	if (m_bitmap->LoadBitmap(nIDResource))
	{
		CRect client;
		GetClientRect(&client);

		// zoom to client rect
		m_visible = doZoomBitmap(m_bitmap, client);
	}

	// failure
	else
	{
		Reset(FALSE);
	}

	// show image
	doRequestGraphicalUpdate();
}


// ----------
// CopyBitmap
// ----------
/**
 * Creates a new bitmap that is equal to the passed one.
 */
void COpenCVImage::CopyBitmap(CBitmap* pBitmap)
{
	// reset first
	Reset(FALSE);

	// copy bitmap
	m_bitmap = doCopyBitmap(pBitmap);

	// take ownership
	m_borrowed = false;

	// check pointer
	if (m_bitmap)
	{
		CRect client;
		GetClientRect(&client);

		// zoom to client rect
		m_visible = doZoomBitmap(m_bitmap, client);
	}

	// failure
	else
	{
		Reset(FALSE);
	}

	// show image
	doRequestGraphicalUpdate();
}


// ------------
// BorrowBitmap
// ------------
/**
 * Uses the passed bitmap without duplicating its data.
 *
 * @par Usage
 * Make sure that the lifetime of the passed bitmap is at least equal to the lifetime of the borrowing control.
 *
 * @par Limitations
 * A borrowed bitmap cannot be scaled down to thumbnail size.
 */
void COpenCVImage::BorrowBitmap(CBitmap* pBitmap)
{
	// reset first
	Reset(FALSE);

	// refuse ownership
	m_borrowed = true;

	// use passed bitmap
	m_bitmap = pBitmap;

	// the zoomed bitmap is always copied
	m_visible = doCopyBitmap(pBitmap);

	// show image
	doRequestGraphicalUpdate();
}


// ------------
// CreateBitmap
// ------------
/**
 * Creates a new bitmap from raw RGB data.
 *
 * @par Data
 * - The BGRA vector must have (Width * Height * 4) elements.
 * - Each pixel is defined by 4 consecutive elements: BLUE-GREEN-RED-ALPHA
 * - The first 4 elements defines the pixel in the image's top-left corner.
 * - The last 4 elements defines the pixel in the image's bottom-right corner.
 */
void COpenCVImage::CreateBitmap(int Width, int Height, unsigned char* BGRA)
{
	// reset first
	Reset(FALSE);

	// RGB + Alpha
	int size = Width * Height * 4;

	// create new bitmap
	m_bitmap = new CBitmap;

	// take ownership
	m_borrowed = false;

	// get window's device context
	CDC* pDC = GetDC();

	// initialize bitmap
	if (m_bitmap->CreateCompatibleBitmap(pDC, Width, Height))
	{
		// set pixels
		if (m_bitmap->SetBitmapBits(size, BGRA))
		{
			CRect client;
			GetClientRect(&client);

			// zoom to client rect
			m_visible = doZoomBitmap(m_bitmap, client);
		}

		// failure
		else
		{
			Reset(FALSE);
		}
	}

	// failure
	else
	{
		Reset(FALSE);
	}

	// release window's device context
	ReleaseDC(pDC);

	// show image
	doRequestGraphicalUpdate();
}

// ------------------------
// IplImage2DIB
// ------------------------
/**
 * Creates a new DIB from raw RGB data.
 *
 * @par Data
 */
HBITMAP COpenCVImage::IplImage2DIB(const IplImage* Image)
{
	m_bmih->biWidth = Image->width;
	m_bmih->biHeight = (Image->origin == IPL_ORIGIN_BL ) ? abs(Image->height) : -abs(Image->height);
	m_bmih->biBitCount = (WORD)Image->nChannels * 8;

	if ( m_bmih->biBitCount == 8 )
	{
		RGBQUAD* palette = m_bmi->bmiColors;
		for (int i = 0; i < 256; i++)
		{
			palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
	}

	HBITMAP hbmp = CreateDIBSection(NULL, m_bmi, DIB_RGB_COLORS, NULL, 0, 0);

	SetDIBits( NULL, hbmp, 0, Image->height, Image->imageData, m_bmi, DIB_RGB_COLORS);

	return hbmp;
}

// ------------------------
// CreateBitmapFromIplImage
// ------------------------
/**
 * Creates a new bitmap.
 *
 * @par Data
 */
void COpenCVImage::CreateBitmapFromIplImage(const IplImage* iplIplImage)
{
	// reset first
	Reset(FALSE);

	// take ownership
	m_borrowed = false;

	// create new bitmap
	m_bitmap = new CBitmap;

	m_bitmap->Attach( IplImage2DIB( iplIplImage ) );

	// check pointer
	if (m_bitmap)
	{
		CRect client;
		GetClientRect(&client);

		// zoom to client rect
		m_visible = doZoomBitmap(m_bitmap, client);
	}
	// failure
	else
	{
		Reset(FALSE);
	}

	// show image
	doRequestGraphicalUpdate();
}


// ---------------
// CreateThumbnail
// ---------------
/**
 * Resizes the control's bitmap to save up memory and to speed up drawing.
 *
 * @par The image isn't shown in thumbnail size but its quality degraded. What's wrong?
 * The shown bitmap always fits proportionally into the control's client area.
 * If you call this method, the source of CDC::StretchBlt will be downsized but not its (visible) output.
 *
 * @par Canging the shown bitmap
 * Each time the shown bitmap changes it is stored primarily in its full size.
 * If you want to hold it in thumbnail size, call this method subsequently.
 *
 * @par Limitations
 * If the shown bitmap is borrowed, this method will have no effect.
 */
void COpenCVImage::CreateThumbnail(int MaxWidth, int MaxHeight)
{
	// check pointer
	if (m_bitmap == 0) return;

	// check ownership
	if (m_borrowed) return;

	// check max size
	if (MaxWidth  <= 0) return;
	if (MaxHeight <= 0) return;

	// get current size
	CRect current = doGetBitmapRect(m_bitmap);

	// don't scale up small images
	if ((current.Width() < MaxWidth) && (current.Height() < MaxHeight)) return;

	// create thumbnail
	CBitmap* thumb = doZoomBitmap(m_bitmap, CRect(0, 0, MaxWidth, MaxHeight));

	// free memory
	delete m_bitmap;

	// reset pointer
	m_bitmap = thumb;
}


// -----
// Reset
// -----
/**
 * Deletes the current bitmap.
 */
void COpenCVImage::Reset(BOOL GraphicalUpdate)
{
	// check pointer
	if (m_bitmap)
	{
		// check ownership
		if (!m_borrowed)
		{
			// free memory
			delete m_bitmap;
		}
	}

	// always free this memory
	if (m_visible) delete m_visible;

	// reset pointers
	m_bitmap = 0;
	m_visible = 0;

	// reset ownership flag
	m_borrowed = false;

	// check update flag
	if (GraphicalUpdate) doRequestGraphicalUpdate();
}


// -------------
// IsInitialized
// -------------
/**
 * Returns TRUE if the control contains a presentable bitmap.
 */
BOOL COpenCVImage::IsInitialized() const
{
	return ((m_bitmap == 0) ? FALSE : TRUE);
}


// ---------------
// doGetBitmapRect
// ---------------
/**
 * Returns the rectangle of the bitmap.
 */
CRect COpenCVImage::doGetBitmapRect(CBitmap* pBitmap)
{
	// start with empty rect
	CRect rect(0, 0, 0, 0);

	// check pointer
	if (!pBitmap) return rect;

	BITMAP bmpInfo;

	// get bitmap information
	if (pBitmap->GetObject(sizeof(bmpInfo), &bmpInfo) == 0)
	{
		// return empty rect
		return rect;
	}

	// set borders
	rect.right = bmpInfo.bmWidth;
	rect.bottom = bmpInfo.bmHeight;

	return rect;
}


// ---------------------
// doGetZoomedBitmapRect
// ---------------------
/**
 * Returns the rectangle of the proportionally zoomed bitmap.
 */
CRect COpenCVImage::doGetZoomedBitmapRect(CBitmap* pBitmap, const CRect& FitInto)
{
	// start with empty rect
	CRect zoomed(0, 0, 0, 0);

	// return empty rect
	if (FitInto.IsRectEmpty()) return zoomed;

	BITMAP bmpInfo;

	// get bitmap information
	if (pBitmap->GetObject(sizeof(bmpInfo), &bmpInfo) == 0)
	{
		// return empty rect
		return zoomed;
	}

	// return empty rect
	if (bmpInfo.bmWidth <= 0) return zoomed;
	if (bmpInfo.bmHeight <= 0) return zoomed;

	// calculate zoom factors
	double xfac = FitInto.Width() / static_cast<double>(bmpInfo.bmWidth);
	double yfac = FitInto.Height() / static_cast<double>(bmpInfo.bmHeight);

	// use smaller factor
	double zfac = (xfac < yfac) ? (xfac) : (yfac);

	// get zoomed size
	zoomed.right = static_cast<int>(zfac * bmpInfo.bmWidth + 0.5);		// round = add 0.5 first and cut off
	zoomed.bottom = static_cast<int>(zfac * bmpInfo.bmHeight + 0.5);	// round = add 0.5 first and cut off

	return zoomed;
}


// -----------
// doCheckZoom
// -----------
/**
 * Returns TRUE if the visible CBitmap needs to be resized.
 */
BOOL COpenCVImage::doCheckZoom()
{
	if (m_visible)
	{
		CRect bmp = doGetBitmapRect(m_visible);

		CRect client;
		GetClientRect(&client);

		// scale down bitmap
		if (client.Width() < bmp.Width()) return TRUE;

		// scale down bitmap
		if (client.Height() < bmp.Height()) return TRUE;

		// scale up bitmap
		if ((client.Width() > bmp.Width()) && (client.Height() > bmp.Height())) return TRUE;
	}

	// it fits
	return FALSE;
}


// ------------
// doDrawBitmap
// ------------
/**
 * Draws the visible CBitmap.
 */
void COpenCVImage::doDrawBitmap()
{
	// get current bitmap rect
	CRect bmpRect = doGetBitmapRect(m_visible);

	// check size
	if (bmpRect.IsRectEmpty()) return;

	// get window's client device context
	CDC* pDC = GetDC();

	// create memory device context
	CDC* memDC = new CDC; memDC->CreateCompatibleDC(pDC);

	// buffer bitmap
	CBitmap* old = memDC->SelectObject(m_visible);

	// copy bitmap bits
	pDC->BitBlt
	(
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the destination rectangle
		0,					// specifies the y-coordinate (in logical units) of the upper-left corner of the destination rectangle
		bmpRect.Width(),	// specifies the width (in logical units) of the destination rectangle and source bitmap
		bmpRect.Height(),	// specifies the height (in logical units) of the destination rectangle and source bitmap
		memDC,				// specifies the source device context
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		SRCCOPY				// specifies the raster operation to be performed
	);

	// reselect first bitmap
	memDC->SelectObject(old);

	// delete device context and reset pointer
	delete memDC; memDC = 0;

	// release dc
	ReleaseDC(pDC);
}


// ----------------
// doDrawBackground
// ----------------
/**
 * Fills the area that is not covered by the visible CBitmap with the background color.
 */
void COpenCVImage::doDrawBackground()
{
	// get client rect
	CRect client;
	GetClientRect(&client);

	// get visible bitmap rect
	CRect bmpRect = doGetBitmapRect(m_visible);

	// right gap
	CRect besides;
	besides.top = 0;
	besides.left = bmpRect.right;
	besides.right = client.right;
	besides.bottom = client.bottom;

	// bottom gap
	CRect below;
	below.top = bmpRect.bottom;
	below.left = 0;
	below.right = client.right;
	below.bottom = client.bottom;

	// get background color
	COLORREF wndColor = GetSysColor(COLOR_3DFACE);

	CDC* pDC = GetDC();

	pDC->FillSolidRect(besides, wndColor);

	if (below != besides)
	{
		pDC->FillSolidRect(below, wndColor);
	}

	ReleaseDC(pDC);
}


// ------------------------
// doRequestGraphicalUpdate
// ------------------------
/**
 * Invalidates and updates the client area.
 */
void COpenCVImage::doRequestGraphicalUpdate()
{
	if (GetSafeHwnd())
	{
		Invalidate();
		UpdateWindow();
	}
}


// ------------
// doCopyBitmap
// ------------
/**
 * Returns a new bitmap that is equal to the passed one.
 */
CBitmap* COpenCVImage::doCopyBitmap(CBitmap* pBitmap)
{
	// get current bitmap rect
	CRect bmpRect = doGetBitmapRect(pBitmap);

	// check size
	if (bmpRect.IsRectEmpty()) return 0;

	// get device context
	CDC* pDC = GetDC();

	// check device context
	if (pDC == 0) return 0;

	// create memory device context (source bitmap)
	CDC* memDC1 = new CDC;
	if (memDC1->CreateCompatibleDC(pDC) == 0)
	{
		delete memDC1;

		ReleaseDC(pDC);

		return 0;
	}

	// create memory device context (new bitmap)
	CDC* memDC2 = new CDC;
	if (memDC2->CreateCompatibleDC(pDC) == 0)
	{
		delete memDC1;
		delete memDC2;

		ReleaseDC(pDC);

		return 0;
	}

	// create new bitmap
	CBitmap* copy = new CBitmap;

	// initialize bitmap
	if (copy->CreateCompatibleBitmap(pDC, bmpRect.Width(), bmpRect.Height()) == 0)
	{
		delete memDC1;
		delete memDC2;
		delete copy;

		ReleaseDC(pDC);

		return 0;
	}

	// not needed anymore
	ReleaseDC(pDC);

	// select bitmaps
	CBitmap* old1 = memDC1->SelectObject(pBitmap);
	CBitmap* old2 = memDC2->SelectObject(copy);

	// copy bitmap bity
	BOOL result = memDC2->BitBlt
	(
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the destination rectangle
		0,					// specifies the y-coordinate (in logical units) of the upper-left corner of the destination rectangle
		bmpRect.Width(),	// specifies the width (in logical units) of the destination rectangle and source bitmap
		bmpRect.Height(),	// specifies the height (in logical units) of the destination rectangle and source bitmap
		memDC1,				// specifies the source device context
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		0,					// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		SRCCOPY				// specifies the raster operation to be performed
	);

	// check operation
	if (result == FALSE)
	{
		// free memory
		delete copy;

		// reset pointer
		copy = 0;
	}

	// reselect old bitmaps
	memDC2->SelectObject(old2);
	memDC1->SelectObject(old1);

	// delete temporary device contexts
	delete memDC2;
	delete memDC1;

	// the copied bitmap
	return copy;
}


// ------------
// doZoomBitmap
// ------------
/**
 * Returns a new bitmap that fits proportionally into the specified rectangle.
 *
 * @return The returned CBitmap is created by using the new operator. You have to delete it yourself.
 */
CBitmap* COpenCVImage::doZoomBitmap(CBitmap* pBitmap, const CRect& FitInto)
{
	// check size
	if (FitInto.IsRectEmpty()) return 0;

	// get current bitmap rect
	CRect bmpRect = doGetBitmapRect(pBitmap);

	// check size
	if (bmpRect.IsRectEmpty()) return 0;

	// get zoomed bitmap rect
	CRect zoomedRect = doGetZoomedBitmapRect(pBitmap, FitInto);

	// check size
	if (zoomedRect.IsRectEmpty()) return 0;

	// get device context
	CDC* pDC = GetDC();

	// check device context
	if (pDC == 0) return 0;

	// create memory device context (source bitmap)
	CDC* memDC1 = new CDC;
	if (memDC1->CreateCompatibleDC(pDC) == 0)
	{
		delete memDC1;

		ReleaseDC(pDC);

		return 0;
	}

	// create memory device context (zoomed bitmap)
	CDC* memDC2 = new CDC;
	if (memDC2->CreateCompatibleDC(pDC) == 0)
	{
		delete memDC1;
		delete memDC2;

		ReleaseDC(pDC);

		return 0;
	}

	// create new bitmap
	CBitmap* zoomed = new CBitmap;

	// initialize bitmap
	if (zoomed->CreateCompatibleBitmap(pDC, zoomedRect.Width(), zoomedRect.Height()) == 0)
	{
		delete memDC1;
		delete memDC2;
		delete zoomed;

		ReleaseDC(pDC);

		return 0;
	}

	// not needed anymore
	ReleaseDC(pDC);

	// select bitmaps
	CBitmap* old1 = memDC1->SelectObject(pBitmap);
	CBitmap* old2 = memDC2->SelectObject(zoomed);

	// stretch bitmap
	memDC2->SetStretchBltMode(HALFTONE);
	BOOL result = memDC2->StretchBlt
	(
		0,						// specifies the x-coordinate (in logical units) of the upper-left corner of the destination rectangle
		0,						// specifies the y-coordinate (in logical units) of the upper-left corner of the destination rectangle
		zoomedRect.Width(),		// specifies the width (in logical units) of the destination rectangle
		zoomedRect.Height(),	// specifies the height (in logical units) of the destination rectangle
		memDC1,					// specifies the source device context
		0,						// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		0,						// specifies the x-coordinate (in logical units) of the upper-left corner of the source rectangle
		bmpRect.Width(),		// specifies the width (in logical units) of the source rectangle
		bmpRect.Height(),		// specifies the height (in logical units) of the source rectangle
		SRCCOPY					// specifies the raster operation to be performed
	);

	// check operation
	if (result == FALSE)
	{
		// free memory
		delete zoomed;

		// reset pointer
		zoomed = 0;
	}

	// reselect old bitmaps
	memDC2->SelectObject(old2);
	memDC1->SelectObject(old1);

	// delete temporary device contexts
	delete memDC2;
	delete memDC1;

	// the zoomed bitmap
	return zoomed;
}
