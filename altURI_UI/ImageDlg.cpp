/*
Copyright (c) 2012, Mark N R Smith, All rights reserved.

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
// CImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaltURIApp.h"
#include "ImageDlg.h"
#include <locale>

extern int iImageHeight;
extern int iImageWidth;
extern int iImageX;
extern int iImageY;
extern int iImageType;
extern bool bDontDisplayImage;

//==================================================================================================
// CImageDlg dialog

IMPLEMENT_DYNAMIC( CImageDlg, CDialog )

//=========================================================================
CImageDlg::CImageDlg( ) : CDialog( CImageDlg::IDD )
{

}

//=========================================================================
void CImageDlg :: DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_WIDTH, m_txtWidth );
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_txtHeight );
	DDX_Control(pDX, IDC_EDIT_X, m_txtX );
	DDX_Control(pDX, IDC_EDIT_Y, m_txtY );
	DDX_Control(pDX, IDC_COMBO_IMAGETYPE, m_cboImageType );
	DDX_Control(pDX, IDC_CHECK_NO_DISPLAY, m_chkDontDisplay );

}

//=========================================================================
BEGIN_MESSAGE_MAP(CImageDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDCANCEL, &CImageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CImageDlg::OnBnClickedOk)
END_MESSAGE_MAP()

//=========================================================================
void SetTextControlToInt( CEdit& cEdit, const int value )
{
	std::ostringstream strs; 

	strs << value;
	cEdit.SetWindowText( strs.str().c_str() );
}

//=========================================================================
BOOL CImageDlg :: OnInitDialog()
{
	CDialog::OnInitDialog();

	// set fields to current values
	SetTextControlToInt( m_txtWidth, iImageWidth );
	SetTextControlToInt( m_txtHeight, iImageHeight );
	SetTextControlToInt( m_txtX, iImageX );
	SetTextControlToInt( m_txtY, iImageY );

	// setup image type combo and set to current value
	set_DropDownSize(m_cboImageType, 6);
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Raw OpenCV (no image processing - just capture)") ), 0 );
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Raw Elementary Motion Detector - 1 channel") ), 1 );
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Raw Elementary Motion Detector - 2 channel") ), 2 );
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Regions Elementary Motion Detector - 2 channel") ), 3 );
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Stats Capture Elementary Motion Detector - 1 channel") ), 4 );
	m_cboImageType.SetItemData( m_cboImageType.AddString( TEXT("Stats Capture Elementary Motion Detector - 2 channel") ), 5 );

	m_cboImageType.SetCurSel(iImageType);

	// set max digits
	m_txtWidth.SetLimitText( MAX_NUMERIC_CHARS );
	m_txtHeight.SetLimitText( MAX_NUMERIC_CHARS );
	m_txtX.SetLimitText( MAX_NUMERIC_CHARS );
	m_txtY.SetLimitText( MAX_NUMERIC_CHARS );

	// set display setting
	m_chkDontDisplay.SetCheck( bDontDisplayImage ? BST_CHECKED : BST_UNCHECKED );

	return TRUE;
}

//=========================================================================
static inline bool IsValidNumber( const tString& s )
{	
	for ( tString::size_type i = 0; i < s.length(); i++ )
		if ( !isdigit( s[i] ) )
			return false;

	return true;
}

//=========================================================================
static inline tString &trimLeadingZeros( tString &s )
{   
	if ( s.empty() ) return s;

	while ( s[0] == TCHAR('0') && s.length() > 1 ) s.erase( 0, 1 );
 
	return s;
}

//=========================================================================
static inline tString &trim( tString &s )
{   
	if ( s.empty() ) return s;
 
	int val = 0;
	for ( tString::size_type cur = 0; cur < s.size(); cur++ )
		if ( s[cur] != TCHAR(' ') && std::isalnum( s[cur], std::locale("") ) )
		{
			s[val] = s[cur];
			val++;
		}
	
	s.resize(val);
	return s;
}

//=========================================================================
void CImageDlg :: OnBnClickedOk()
{
	// retrieve string values (hopefully numbers) from controls
	tString tWidth( TCHAR(' '), MAX_NUMERIC_CHARS ),  tHeight( TCHAR(' '), MAX_NUMERIC_CHARS ), tX( TCHAR(' '), MAX_NUMERIC_CHARS ), tY( TCHAR(' '), MAX_NUMERIC_CHARS );
	
	m_txtWidth.GetLine( 0, const_cast<LPSTR>( tWidth.c_str() ), MAX_NUMERIC_CHARS );
	m_txtHeight.GetLine( 0, const_cast<LPSTR>( tHeight.c_str() ), MAX_NUMERIC_CHARS );
	m_txtX.GetLine( 0, const_cast<LPSTR>( tX.c_str() ), MAX_NUMERIC_CHARS );
	m_txtY.GetLine( 0, const_cast<LPSTR>( tY.c_str() ), MAX_NUMERIC_CHARS );

	// do some validation

	// remove leading and trailing spaces
	tWidth = trim( tWidth );
	tHeight = trim( tHeight );
	tX = trim( tX );
	tY = trim( tY );

	// remove leading zeros
	tWidth = trimLeadingZeros( tWidth );
	tHeight = trimLeadingZeros( tHeight );
	tX = trimLeadingZeros( tX );
	tY = trimLeadingZeros( tY );

	// set to zero if empty
	tWidth = tWidth.empty() ? TEXT("0") : tWidth;
	tHeight = tHeight.empty() ? TEXT("0") : tHeight;
	tX = tX.empty() ? TEXT("0") : tX;
	tY = tY.empty() ? TEXT("0") : tY;

	if ( !( IsValidNumber( tWidth ) && IsValidNumber( tHeight ) && IsValidNumber( tX ) && IsValidNumber( tY ) ) )
	{
		AfxMessageBox( TEXT("Please enter ONLY digits 0 to 9 for image size and offsets"), MB_OK|MB_ICONSTOP );
		return;
	}

	if ( ( atoi( tWidth.c_str() ) == 0 && atoi( tHeight.c_str() ) != 0 ) || ( atoi( tWidth.c_str() ) != 0 && atoi( tHeight.c_str() ) == 0 ) )
	{
		AfxMessageBox( TEXT("Please enter both zeros or both non-zeros for height and width"), MB_OK|MB_ICONSTOP );
		return;
	}

	if ( ( atoi( tWidth.c_str() ) < 4 && atoi( tWidth.c_str() ) > 0) || ( atoi( tHeight.c_str() ) < 4 && atoi( tHeight.c_str() ) > 0 ) )
	{
		AfxMessageBox( TEXT("Image width and height must be at least 4"), MB_OK|MB_ICONSTOP );
		return;
	}
	
	// set image type
	int iTempImageType = m_cboImageType.GetCurSel();

	if ( iTempImageType > 0 )
	{
		if ( ( atoi( tWidth.c_str() ) % 2 != 0 && atoi( tWidth.c_str() ) > 0) || ( atoi( tHeight.c_str() ) % 2 != 0 && atoi( tHeight.c_str() ) > 0 ) )
		{
			AfxMessageBox( TEXT("Image width and height should be even numbers when using EMD"), MB_OK|MB_ICONSTOP );
			return;
		}
	}
	
	iImageType = iTempImageType;

	// set to enable or disable image display
	bDontDisplayImage = ( m_chkDontDisplay.GetCheck( ) == BST_CHECKED );


	// set image values
	iImageWidth = atoi( tWidth.c_str() ); 
	iImageHeight = atoi( tHeight.c_str() ); 
	iImageX = atoi( tX.c_str() ); 
	iImageY = atoi( tY.c_str() ); 
	
	OnOK();
}

//=========================================================================
void CImageDlg :: OnBnClickedCancel()
{
	OnCancel();
}

