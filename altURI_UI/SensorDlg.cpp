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
// CSensorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaltURIApp.h"
#include "SensorDlg.h"

extern CaltURI_plus altURI;
extern void set_DropDownSize(CComboBox& box, UINT LinesToDisplay);

// CSensorDlg dialog

IMPLEMENT_DYNAMIC(CSensorDlg, CDialog)

CSensorDlg::CSensorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSensorDlg::IDD, pParent)
{

}

CSensorDlg::~CSensorDlg()
{
}

void CSensorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATCH_SENSOR_CHECK, m_chkCatchSensorData);
	DDX_Control(pDX, IDC_SENSOR_LIST, m_lstSensorMessages);
	DDX_Control(pDX, IDC_MESSAGE_COMBO, m_cboSensorMessage);
}


BEGIN_MESSAGE_MAP(CSensorDlg, CDialog)
	ON_BN_CLICKED(IDC_CATCH_SENSOR_CHECK, &CSensorDlg::OnBnClickedCatchSensorCheck)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSensorDlg message handlers

//=========================================================================
void CSensorDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == IDC_TIMER_SENSOR )
	{
		if ( m_chkCatchSensorData.GetCheck() == BST_CHECKED )
		{
			const int iMessagesToGet = 10;
			const int iColumnsToGet = 4;

			// make array for some sensor data
			double** dArray2D = new double*[iMessagesToGet];

			for (int i = 0; i < iMessagesToGet; ++i)
				dArray2D[i] = new double[iColumnsToGet];

			const tString sSeparator = TEXT( "\t" );

			DWORD_PTR iMessageNumber;
			DWORD_PTR iSelect = m_cboSensorMessage.GetCurSel();
	
			if ( iSelect != CB_ERR ) 
				iMessageNumber = m_cboSensorMessage.GetItemData( iSelect );
			else
				iMessageNumber = 0;

			// get iMessagesToGet rows of sensor data - may not be available but returns rows actually got
			int iMessages = altURI._GetMessageArray( iMessagesToGet, dArray2D, iMessageNumber );

			// load it into the listbox
			for ( int i = 0; i < iMessages; i++ )
			{
				std::ostringstream strs; 

				for ( int j = 0; j < iColumnsToGet; j++ )
					strs << dArray2D[i][j] << sSeparator;

				tString sTemp( strs.str() );
				m_lstSensorMessages.AddString( sTemp.c_str() );
			}

			// deallocate memory used
			for (int i = 0; i < iMessagesToGet; ++i)
				delete [] dArray2D[i];
			delete [] dArray2D;

			// scroll to end - how ???
			//m_lstSensorMessages.SetTopIndex( m_lstSensorMessages.GetCount() );
		}
	}

	CDialog::OnTimer( nIDEvent );
}

//=========================================================================
BOOL CSensorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//m_lstSensorMessages.SetTabStops( (300) / LOWORD(::GetDialogBaseUnits()));

	m_cboSensorMessage.SetItemData( m_cboSensorMessage.AddString(  TEXT(" 0 - ALL") ), 0 );

	for ( int i = 1; i < 31; i++ )
	{
		std::ostringstream strs;
		tString sTemp( altURI._GetMessageKey( i ) );
		strs << i << " - " << ( sTemp.size() == 0 ? TEXT("None set") : sTemp );
		m_cboSensorMessage.SetItemData( m_cboSensorMessage.AddString( strs.str().c_str() ), i );
	}

	m_cboSensorMessage.SetCurSel(0);

	set_DropDownSize( m_cboSensorMessage, 6 );

	SetTimer( IDC_TIMER_SENSOR, 1000, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}


//==================================================================================================
void CSensorDlg::OnBnClickedCatchSensorCheck()
{
	// TODO: Add your control notification handler code here
	altURI._SetCatchMessages( m_chkCatchSensorData.GetCheck() == BST_CHECKED );
}
