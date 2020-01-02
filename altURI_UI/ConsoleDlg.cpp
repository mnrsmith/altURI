// CConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaltURIApp.h"
#include "ConsoleDlg.h"

extern CaltURI_plus altURI;

//==================================================================================================
// CProcessDlg dialog

IMPLEMENT_DYNAMIC(CConsoleDlg, CDialog)

//==================================================================================================
CConsoleDlg::CConsoleDlg(CWnd* pParent /*=NULL*/) : CDialog(CConsoleDlg::IDD, pParent)
{

}

//==================================================================================================
CConsoleDlg::~CConsoleDlg()
{
}

//==================================================================================================
void CConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtSend);
	DDX_Control(pDX, IDOK2, m_btnSend);
	DDX_Control(pDX, IDC_EDIT2, m_txtReceive);
	DDX_Control(pDX, IDC_CHECK1, m_chkOtherMessages);
	DDX_Control(pDX, IDC_CHECK2, m_chkSensorMessages);
}

//==================================================================================================
BEGIN_MESSAGE_MAP(CConsoleDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CConsoleDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CConsoleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CConsoleDlg::OnBnClickedOk2)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CConsoleDlg::OnBnClickedCheck)
	ON_BN_CLICKED(IDC_CHECK2, &CConsoleDlg::OnBnClickedCheck)
END_MESSAGE_MAP()


// CConsoleDlg message handlers

//=========================================================================
void CConsoleDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == IDC_TIMER_TCP )
	{
		CString csLatest( altURI.ReceiveLine().c_str() );
				
		if ( csLatest.GetLength() > 0 ) 
		{
			TRACE("Received: %s", csLatest);

			CString csReceivedCEditText;

			m_txtReceive.GetWindowText( csReceivedCEditText );

			csReceivedCEditText += csLatest;

			m_txtReceive.SetWindowText( csReceivedCEditText );
			m_txtReceive.LineScroll(65535);
		}
	}

	CDialog::OnTimer( nIDEvent );
}


//=========================================================================
BOOL CConsoleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer( IDC_TIMER_TCP, 500, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//=========================================================================
// CProcessDlg message handlers
void CConsoleDlg::OnBnClickedCancel()	// Send button
{
	CString csSendText, csReceivedCEditText;

	m_txtSend.GetWindowText( csSendText );

	if ( csSendText.GetLength() == 0 ) return;

	tString sSendText(csSendText);

	if ( altURI.SendCommand( sSendText ) )
	{
		m_txtSend.SetWindowText( TEXT("") );

		m_txtReceive.GetWindowText( csReceivedCEditText );

		if ( csReceivedCEditText.GetLength() != 0 )
			csReceivedCEditText += TEXT("\r\n");
		
		csReceivedCEditText += TEXT("SENT:") + csSendText;

		m_txtReceive.SetWindowText( csReceivedCEditText );
		m_txtReceive.LineScroll(65535);
	}
}

//=========================================================================
void CConsoleDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}


//=========================================================================
void CConsoleDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	Sleep(1000);

}

//=========================================================================
void CConsoleDlg::OnBnClickedCheck()
{
	// Switch both checkbox options appropriately

	altURI._SetCatchReceivedLines( m_chkOtherMessages.GetCheck() == BST_CHECKED, m_chkSensorMessages.GetCheck() == BST_CHECKED );
}


