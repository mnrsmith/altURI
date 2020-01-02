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
//	COpenCVImage.h - control to display an OCVG Image
//=========================================================================


#pragma once

/*
 * A CStatic control that displays OpenCV images.
 */
class COpenCVImage : public CStatic
{

public:

	// ------------
	// Construction
	// ------------

	/// standard-constructor
	COpenCVImage();

	/// virtual destructor
	virtual ~COpenCVImage();


	// --------------
	// Initialization
	// --------------

	/// loads a bmp file
	void LoadBitmap(const CString& Path);

	/// loads a bitmap resource
	void LoadBitmap(UINT nIDResource);

	/// creates a new bitmap that is equal to the passed one
	void CopyBitmap(CBitmap* pBitmap);

	/// uses the passed bitmap without duplicating its data
	void BorrowBitmap(CBitmap* pBitmap);

	/// creates a new bitmap from raw RGB data
	void CreateBitmap(int Width, int Height, unsigned char* BGRA);

	/// creates a new bitmap from an OpenCV IplImage
	void CreateBitmapFromIplImage(const IplImage*	IplImage);

	// --------
	// Handling
	// --------

	/// resizes the control's bitmap to save up memory and to speed up drawing
	void CreateThumbnail(int MaxWidth, int MaxHeight);

	/// deletes the current bitmap
	void Reset(BOOL GraphicalUpdate = TRUE);


	// -----------
	// Information
	// -----------

	/// returns TRUE if the control contains a presentable bitmap
	BOOL IsInitialized() const;


	// ---------------------
	// Message map functions
	// ---------------------
	//{{AFX_MSG(CPreviewRect)
	/// WM_PAINT handler
	afx_msg void OnPaint();
	/// WM_SIZE  handler
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


protected:

	// ----------------
	// Internal methods
	// ----------------

	/// returns the rectangle of the bitmap
	CRect doGetBitmapRect(CBitmap* pBitmap);

	/// returns the rectangle of the proportionally zoomed bitmap
	CRect doGetZoomedBitmapRect(CBitmap* pBitmap, const CRect& FitInto);

	/// returns TRUE if the visible CBitmap needs to be resized
	BOOL doCheckZoom();

	/// draws the visible CBitmap
	void doDrawBitmap();

	/// fills the area that is not covered by the visible CBitmap with the background color
	void doDrawBackground();	

	/// invalidates and updates the client area
	void doRequestGraphicalUpdate();

	/// returns a new bitmap that is equal to the passed one
	CBitmap* doCopyBitmap(CBitmap* pBitmap);

	/// returns a new bitmap that fits proportionally into the specified rectangle
	CBitmap* doZoomBitmap(CBitmap* pBitmap, const CRect& FitInto);

	///
	HBITMAP IplImage2DIB(const IplImage *Image);


private:

	// ----------
	// Attributes
	// ----------

	/// the bitmap in its original (or thumbnail) size
	CBitmap* m_bitmap;

	/// the bitmap in its zoomed size
	CBitmap* m_visible;

	unsigned char m_buffer[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
	BITMAPINFO* m_bmi;
	BITMAPINFOHEADER* m_bmih;

	/// true if the bitmap is only borrowed
	bool m_borrowed;
};


