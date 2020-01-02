/*
Copyright (c) 2010-2012, Mark N R Smith, All rights reserved.

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
//
// CMainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mmsystem.h"		// for timeGetTime()
#include "CaltURIApp.h"
#include "MainDlg.h"
#include "ImageDlg.h"
#include "ProcessDlg.h"
#include "ConsoleDlg.h"
#include "SensorDlg.h"

#pragma comment(lib, "winmm")

CaltURI_plus altURI;
CvVideoWriter *writer = 0;
std::map<int, CaURIButton> mCommandButtons;

bool bUnrealVisible = true;
bool bConnected = false;
bool bConnectVisionOnly = false;
bool bDontDisplayImage = false;
bool bVaryingEMDParameters = false;
int iTimerDelay;
clock_t ctLastFrameClock = 0;
DWORD dwLastFrameClock;
bool bPlaying = false;
bool bRecording = false;
int iNextRobotNumber;
int iNextCameraNumber;
std::string sRobotName;


//=========================================================================
const tString GetAppPath()
{
	tString tsBuffer;
	tsBuffer.resize(_MAX_PATH);

	GetModuleFileName(0, (TCHAR*)tsBuffer.c_str(), _MAX_PATH);

	return tsBuffer;
}

//=========================================================================
const tString ExtractAppPath()
{ 
	tString tsPath( GetAppPath() );
	tString tsDrive;
	tsDrive.resize(_MAX_DRIVE);
	tString tsFolder;
	tsFolder.resize(_MAX_DIR);
	_splitpath( (TCHAR*)tsPath.c_str(), (TCHAR*)tsDrive.c_str(), (TCHAR*)tsFolder.c_str(), NULL, NULL );

	tString tsAppPath;

	// cope with unc shares
	if ( tsDrive.at(0) != 0 )
		tsAppPath = tsDrive.substr(0, 2) + tsFolder;
	else
		tsAppPath = tsFolder;

	tsAppPath.resize( tsAppPath.find_last_of( TEXT("\\") ) + 1 );

	return tsAppPath;
}

//==================================================================================================
tString ToLower(const tString& sString)
{
	tString sReturned( sString );
	std::transform( sReturned.begin(), sReturned.end(), sReturned.begin(), tolower );
	return sReturned;
}

//=========================================================================
tString GetIniRoot( const tString& sRobotIniFile )
{
	tString sLowercaseTemp( ToLower( sRobotIniFile ) );
	int iStart = sLowercaseTemp.rfind( TEXT("\\") ) == tString::npos ? 0 : sLowercaseTemp.rfind( TEXT("\\") ) + 1;
	tString sReturned = sLowercaseTemp.substr( iStart, sLowercaseTemp.size() );
	sReturned = sReturned.substr( 0, sReturned.size() - 4 );
	return sReturned;
}

//=========================================================================
// set correct paths for ini files 
#if defined(_DEBUG)
#	if defined _M_X64
	CIniFile IniFile( ExtractAppPath() + TEXT("..\\..\\altURI_UI\\altURI_UI.ini") );		// read ini file
	tString tsRobotIniFile (  ExtractAppPath() + TEXT("..\\..\\altURI_Cmd\\") + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
	tString tsRobotIniFileReal (  ExtractAppPath() + TEXT("..\\..\\altURI_ARDrone_Cmd\\") + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
#	else
	CIniFile IniFile( ExtractAppPath() + TEXT("..\\altURI_UI\\altURI_UI.ini") );		// read ini file
	tString tsRobotIniFile (  ExtractAppPath() + TEXT("..\\altURI_Cmd\\") + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
	tString tsRobotIniFileReal (  ExtractAppPath() + TEXT("..\\altURI_ARDrone_Cmd\\") + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
#	endif
#else
CIniFile IniFile( ExtractAppPath() + TEXT("altURI_UI.ini") );		// read ini file
tString tsRobotIniFile (  ExtractAppPath() + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
tString tsRobotIniFileReal (  ExtractAppPath() + IniFile.GetValue( "altURI_UI settings", "RobotIniFile" ) );
#endif

// get some values from the ini file
tString tsHookDll (  IniFile.GetValue( "altURI_UI settings", "HookDll" ) );
tString tsCmdDll (  IniFile.GetValue( "altURI_UI settings", "CmdDll" ) );
tString tsHookDllPath ( ExtractAppPath() + tsHookDll  );
tString tsCmdDllPath ( ExtractAppPath() + tsCmdDll  );
tString tsIniRoot( GetIniRoot( tsRobotIniFile ) );
bool bLocalImage =  IniFile.GetValueB( "altURI_UI settings", "LocalImage" );
bool bUseCmdDllImage =  IniFile.GetValueB( "altURI_UI settings", "UseCmdDllImage" );
tString sWindowTitle( IniFile.GetValue( "USARSim settings", "WindowTitle" ) );
int iImageHeight = IniFile.GetValueI( "altURI_UI settings", "ImageHeight" );
int iImageWidth = IniFile.GetValueI( "altURI_UI settings", "ImageWidth" );
int iImageX = IniFile.GetValueI( "altURI_UI settings", "ImageX" );
int iImageY = IniFile.GetValueI( "altURI_UI settings", "ImageY" );
int iImageType = IniFile.GetValueI( "altURI_UI settings", "ImageType" );
int iRawImageHeight;
int iRawImageWidth;

//=========================================================================
//=========================================================================
//=========================================================================
// CAboutDlg dialog used for App About
//=========================================================================
class CAboutDlg : public CDialog
{
	DECLARE_DYNAMIC(CAboutDlg)
public:
	CAboutDlg();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor (CDC * pDCM , CWnd * pWnd , UINT nCtlColor);

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	CBrush m_brush;

public:
	CEdit m_editCopyright;
};

//=========================================================================
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_COPYRIGHT1, m_editCopyright);
}


//=========================================================================
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_CTLCOLOR() 
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

//=========================================================================
CAboutDlg::CAboutDlg() : CDialog( CAboutDlg::IDD )
{
}

//=========================================================================
BOOL CAboutDlg::OnInitDialog()
{
	m_brush.CreateSolidBrush(RGB(255, 255, 255)); // color white brush

	CDialog::OnInitDialog();

	std::string sCopyright = TEXT( "\tThe altURI software is licensed under the BSD License:\r\n\r\n");
	sCopyright += TEXT( "Copyright © Mark N R Smith and contributors, All rights reserved.\r\n\r\n" );
	sCopyright += TEXT( "Redistribution and use in source and binary forms, with or without modification,\r\n" );
	sCopyright += TEXT( "are permitted provided that the following conditions are met:\r\n\r\n" );
	sCopyright += TEXT( "Redistributions of source code must retain the above copyright notice, this list\r\n" );
	sCopyright += TEXT( "of conditions and the following disclaimer. Redistributions in binary form must\r\n" );
	sCopyright += TEXT( "reproduce the above copyright notice, this list of conditions and the following\r\n" );
	sCopyright += TEXT( "disclaimer in the documentation and/or other materials provided with the\r\n" );
	sCopyright += TEXT( "distribution. Neither the name of the author nor the names of its contributors\r\n" );
	sCopyright += TEXT( "may be used to endorse or promote products derived from this software without\r\n" );
	sCopyright += TEXT( "specific prior written permission.\r\n\r\n" );
	sCopyright += TEXT( "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND\r\n" );
	sCopyright += TEXT( "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\r\n" );
	sCopyright += TEXT( "OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL\r\n" );
	sCopyright += TEXT( "THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,\r\n" );
	sCopyright += TEXT( "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT\r\n");
	sCopyright += TEXT( "OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\n");
	sCopyright += TEXT( "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR\r\n");
	sCopyright += TEXT( "TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,\r\n");
	sCopyright += TEXT( "EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.");
	m_editCopyright.SetWindowText( sCopyright.c_str() );

	return TRUE;
}

//=========================================================================
HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Return a different brush if the default is not desired
	return m_brush;

	UNREFERENCED_PARAMETER(pDC);
	UNREFERENCED_PARAMETER(pWnd);
	UNREFERENCED_PARAMETER(nCtlColor);
}

//=========================================================================
//=========================================================================
//=========================================================================
CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CResizeDlg(CMainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}


//=========================================================================
void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OCVG_IMAGE, m_OCVGImage);
	DDX_Control(pDX, IDC_BUTTON_VIEW, m_button);
	DDX_Control(pDX, IDC_COMBO_FRAME_RATE, m_cboFrameRate);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
	DDX_Control(pDX, IDC_CHECK_VISION_ONLY, m_chkVisionOnly);
	DDX_Control(pDX, IDC_BUTTON_RECORD, m_buttonRecordImage);
	DDX_Control(pDX, IDC_BUTTON_IMAGETYPE, m_buttonImageType);
	DDX_Control(pDX, IDC_COMBO_ROBOTDLL, m_cboRobotDLL);
	DDX_Control(pDX, IDC_RADIO_SIM, m_RadioSim);
	DDX_Control(pDX, IDC_RADIO_REAL, m_RadioReal);
	DDX_Control(pDX, IDC_BUTTON_RIGHT, m_buttonMoveRight);
	DDX_Control(pDX, IDC_BUTTON_LEFT, m_buttonMoveLeft);
	DDX_Control(pDX, IDC_BUTTON_FORWARD, m_buttonMoveForward);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_buttonMoveBack);
	DDX_Control(pDX, IDC_BUTTON_UP, m_buttonMoveUp);
	DDX_Control(pDX, IDC_BUTTON_DOWN, m_buttonMoveDown);
	DDX_Control(pDX, IDC_BUTTON_INJECT, m_buttonInject);
	DDX_Control(pDX, IDC_BUTTON_COMMANDS, m_buttonCommands);
	DDX_Control(pDX, IDC_BUTTON_HIDESIM, m_buttonHide);
	DDX_Control(pDX, IDC_BUTTON_CAM1, m_buttonMoveCam1);
	DDX_Control(pDX, IDC_BUTTON_CAM2, m_buttonMoveCam2);
	DDX_Control(pDX, IDC_BUTTON_CAM3, m_buttonMoveCam3);
	DDX_Control(pDX, IDC_BUTTON_CAM4, m_buttonMoveCam4);
	DDX_Control(pDX, IDC_BUTTON_CAM5, m_buttonMoveCam5);
	DDX_Control(pDX, IDC_BUTTON_CAM6, m_buttonMoveCam6);
	DDX_Control(pDX, IDC_BUTTON_CAM7, m_buttonMoveCam7);
	DDX_Control(pDX, IDC_BUTTON_E1, m_buttonE1);
	DDX_Control(pDX, IDC_BUTTON_E2, m_buttonE2);
	DDX_Control(pDX, IDC_BUTTON_E3, m_buttonE3);
	DDX_Control(pDX, IDC_BUTTON_E4, m_buttonE4);
	DDX_Control(pDX, IDC_BUTTON_E5, m_buttonE5);
	DDX_Control(pDX, IDC_BUTTON_VIEWPORT, m_buttonViewport);
	DDX_Control(pDX, IDC_BUTTON_NEWROBOT, m_buttonNewRobot);
	DDX_Control(pDX, IDC_BUTTON_SENSORS, m_buttonSensors);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_buttonMoveStop);
	DDX_Control(pDX, IDC_BUTTON_FORWARD_LEFT, m_buttonMoveForwardLeft);
	DDX_Control(pDX, IDC_BUTTON_FORWARD_RIGHT, m_buttonMoveForwardRight);
	DDX_Control(pDX, IDC_TEXT_FPS, m_labelFPS);
}


//=========================================================================
BEGIN_MESSAGE_MAP(CMainDlg, CResizeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_VIEW, &CMainDlg::OnBnClickedButtonView)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CMainDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDC_CHECK_VISION_ONLY, &CMainDlg::OnBnClickedVisionOnly)
	ON_BN_CLICKED(IDC_RADIO_SIM, &CMainDlg::OnBnClickedRadioSim)
	ON_BN_CLICKED(IDC_RADIO_REAL, &CMainDlg::OnBnClickedRadioReal)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, &CMainDlg::OnBnClickedRecordImage)
	ON_BN_CLICKED(IDC_BUTTON_INJECT, &CMainDlg::OnBnClickedOk4)
	ON_BN_CLICKED(IDC_BUTTON_COMMANDS, &CMainDlg::OnBnClickedOk5)
	ON_BN_CLICKED(IDC_BUTTON_HIDESIM, &CMainDlg::OnBnClickedOk6)
	ON_BN_CLICKED(IDC_BUTTON_SENSORS, &CMainDlg::OnBnClickedSensors)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CMainDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CMainDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, &CMainDlg::OnBnClickedButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CMainDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CMainDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CMainDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD_RIGHT, &CMainDlg::OnBnClickedButtonForwardRight)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD_LEFT, &CMainDlg::OnBnClickedButtonForwardLeft)
	ON_BN_CLICKED(IDC_BUTTON_CAM1, &CMainDlg::OnBnClickedButtonCam1)
	ON_BN_CLICKED(IDC_BUTTON_CAM2, &CMainDlg::OnBnClickedButtonCam2)
	ON_BN_CLICKED(IDC_BUTTON_CAM3, &CMainDlg::OnBnClickedButtonCam3)
	ON_BN_CLICKED(IDC_BUTTON_CAM4, &CMainDlg::OnBnClickedButtonCam4)
	ON_BN_CLICKED(IDC_BUTTON_CAM5, &CMainDlg::OnBnClickedButtonCam5)
	ON_BN_CLICKED(IDC_BUTTON_CAM6, &CMainDlg::OnBnClickedButtonCam6)
	ON_BN_CLICKED(IDC_BUTTON_CAM7, &CMainDlg::OnBnClickedButtonCam7)
	ON_BN_CLICKED(IDC_BUTTON_E1, &CMainDlg::OnBnClickedButtonE1)
	ON_BN_CLICKED(IDC_BUTTON_E2, &CMainDlg::OnBnClickedButtonE2)
	ON_BN_CLICKED(IDC_BUTTON_E3, &CMainDlg::OnBnClickedButtonE3)
	ON_BN_CLICKED(IDC_BUTTON_E4, &CMainDlg::OnBnClickedButtonE4)
	ON_BN_CLICKED(IDC_BUTTON_E5, &CMainDlg::OnBnClickedButtonE5)
	ON_BN_CLICKED(IDC_BUTTON_VIEWPORT, &CMainDlg::OnBnClickedImageView)
	ON_BN_CLICKED(IDC_BUTTON_NEWROBOT, &CMainDlg::OnBnClickedButtonNewrobot)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMainDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_IMAGETYPE, &CMainDlg::OnBnClickedButtonImageType)
END_MESSAGE_MAP()


//=========================================================================
inline double fround( double dNumber, int iDecimals )
{
	return floor( dNumber * pow( 10., iDecimals ) + .5 ) / pow( 10., iDecimals );
}

//=========================================================================
void CMainDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == IDC_TIMER_IMG )
	{
		// stop the timer
		KillTimer( IDC_TIMER_IMG );

		IplImage* image = altURI.GetFrame( iImageWidth, iImageHeight, iImageX, iImageY );

		if ( CV_IS_IMAGE( image ) )
		{
			if ( !bDontDisplayImage) 
			{	
				m_OCVGImage.CreateBitmapFromIplImage(image);
				if ( bRecording ) cvWriteFrame( writer, image );
			}
		}

		// if testing EMD increment the current parameter
		if (bVaryingEMDParameters) altURI.IncEMDParams( false );

		double dwFrameMilliseconds = fround( 1000 / ( timeGetTime() - dwLastFrameClock ), 0 );
		dwLastFrameClock = timeGetTime();

		std::ostringstream strs; 

		strs << TEXT("Display: ") << dwFrameMilliseconds << TEXT(" fps");

		m_labelFPS.SetWindowText( strs.str().c_str() );

		// then restart
		SetTimer(IDC_TIMER_IMG, iTimerDelay, NULL);;
	}
	CDialog::OnTimer(nIDEvent);
}


//=========================================================================
bool ConnectReal( tString sRobotDLL )
{
	if ( !altURI.OpenaltURI( tsRobotIniFileReal, TEXT(""), sRobotDLL, false, true, bConnectVisionOnly ) )
	{
		altURI.ClosealtURI();
		AfxMessageBox( TEXT("Sorry - Unable to connect, check .ini settings"), MB_OK|MB_ICONSTOP );
		return false;
	}

	return true;
}

//=========================================================================
bool ConnectSimulator( void )
{
	if ( !altURI.OpenaltURI( tsRobotIniFile, tsHookDllPath, tsCmdDllPath, bLocalImage, bUseCmdDllImage, bConnectVisionOnly ) )
	{
		altURI.ClosealtURI();
		AfxMessageBox( TEXT("Sorry - Unable to connect, check .ini settings"), MB_OK|MB_ICONSTOP );
		return false;
	}

	return true;
}

//=========================================================================
void LoadIconToButton( int iResourceID, CButton& button)
{
	HICON hIcon = reinterpret_cast<HICON>( LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE(iResourceID), IMAGE_ICON, 0, 0, LR_SHARED ) );
	button.SetIcon( hIcon );
}

//=========================================================================
// CMainDlg message handlers

BOOL CMainDlg::OnInitDialog()
{
	CResizeDlg::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// set controls resize params			(left,		top,		right,		bottom)
	
	// image stuff
	AddControl(IDC_FRAME_IMAGE,				CST_RESIZE,	CST_RESIZE,	CST_RESIZE,	CST_RESIZE, 0);
	AddControl(IDC_OCVG_IMAGE,				CST_RESIZE,	CST_RESIZE,	CST_RESIZE,	CST_RESIZE, 1);

	AddControl(IDC_FRAME_IMAGE_CONTROLS,	CST_RESIZE,	CST_NONE,	CST_RESIZE,	CST_REPOS, 0);

	// frames
	AddControl(IDOK,						CST_NONE,	CST_REPOS,	CST_REPOS,	CST_REPOS, 1);

	AddControl(IDC_FRAME_ROBOT_SOURCE,		CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_FRAME_ROBOT_CONTROLS,	CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_FRAME_CONNECTIONS,		CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);

	// image control frame controls
	AddControl(IDC_BUTTON_RECORD,			CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);
	AddControl(IDC_BUTTON_IMAGETYPE,		CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);
	AddControl(IDC_BUTTON_VIEW,				CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);
	AddControl(IDC_TEXT_FRAME_RATE,			CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);
	AddControl(IDC_COMBO_FRAME_RATE,		CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);
	AddControl(IDC_TEXT_FPS,				CST_REPOS,	CST_NONE,	CST_NONE,	CST_REPOS, 1);

	// robot source frame controls
	AddControl(IDC_RADIO_SIM,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_RADIO_REAL,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_BUTTON_INJECT,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_BUTTON_HIDESIM,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_BUTTON_VIEWPORT,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);

	// connection frame controls
	AddControl(IDC_BUTTON_CONNECT,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_CHECK_VISION_ONLY,		CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);

	AddControl(IDC_BUTTON_NEWROBOT,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_BUTTON_COMMANDS,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);
	AddControl(IDC_BUTTON_SENSORS,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 1);

	AddControl(IDC_TEXT_ROBOTDLL,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_COMBO_ROBOTDLL,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);

	// robot control frame controls
	AddControl(IDC_BUTTON_LEFT,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_FORWARD,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_RIGHT,			CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_BACK,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_UP,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_DOWN,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_STOP,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_FORWARD_LEFT,		CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_FORWARD_RIGHT,	CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);

	AddControl(IDC_BUTTON_CAM1,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM2,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM3,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM4,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM5,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM6,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_CAM7,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);

	AddControl(IDC_BUTTON_E1,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_E2,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_E3,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_E4,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);
	AddControl(IDC_BUTTON_E5,				CST_NONE,	CST_REPOS,	CST_REPOS,	CST_NONE, 0);


	// do some other stuff here
	set_DropDownSize(m_cboRobotDLL, 4);
	set_DropDownSize(m_cboFrameRate, 4);

	// load real robots dlls from ini file
	int iRobotDLLs = IniFile.GetValueI( "altURI_UI settings", "NoRobotDLLs" );

	for ( int iDLL = 1; iDLL < iRobotDLLs + 1; iDLL++ )
	{
		std::stringstream oss;
		oss << "RobotDLL" << iDLL;
		tString sKey( oss.str() );
		tString sValue( IniFile.GetValue( "altURI_UI settings", sKey ) );
		m_cboRobotDLL.AddString( sValue.c_str() );
	}
	
	m_cboRobotDLL.SetCurSel(0);
	m_cboRobotDLL.EnableWindow(FALSE);

	// set frames per second with milliseconds delay value in itemdata
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("200") ), 5 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("100") ), 10 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("50") ), 20 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("30") ), 33 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("20") ), 50 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("10") ), 100 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("5") ), 200 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("1") ), 1000 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("0.5") ), 2000 );
	m_cboFrameRate.SetItemData( m_cboFrameRate.AddString( TEXT("0.25") ), 4000 );
	m_cboFrameRate.SetCurSel(3);
	m_cboFrameRate.EnableWindow(0);

	// disable everything ready for clicking a robot source radio button
	SetConnectControls( FALSE );
	// m_buttonConnect.EnableWindow( bLocalImage ? FALSE : TRUE );

	LoadIconToButton( IDI_ICON_ARROW_LEFT, m_buttonMoveLeft );
	LoadIconToButton( IDI_ICON_ARROW_RIGHT, m_buttonMoveRight );
	LoadIconToButton( IDI_ICON_ARROW_UP, m_buttonMoveUp );
	LoadIconToButton( IDI_ICON_ARROW_DOWN, m_buttonMoveDown );
	LoadIconToButton( IDI_ICON_ARROW_FORWARD, m_buttonMoveForward );
	LoadIconToButton( IDI_ICON_ARROW_BACK, m_buttonMoveBack );
	LoadIconToButton( IDI_ICON_ARROW_FORWARD_LEFT, m_buttonMoveForwardLeft );
	LoadIconToButton( IDI_ICON_ARROW_FORWARD_RIGHT, m_buttonMoveForwardRight );

	// make random number for robot name to prevent client using same name
	srand ( (unsigned)time(NULL) );
	iNextRobotNumber = rand() % 10000;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//=========================================================================
void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CResizeDlg::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
//=========================================================================
void CMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CResizeDlg::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
//=========================================================================
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//=========================================================================
void CMainDlg :: SetImageFrameTitleToImageSpecs(	const int iRequestedWidth,
													const int iRequestedHeight,
													const int iRequestedX,
													const int iRequestedY,
													const int iImageType)
{
	int iRawWidth, iRawHeight;

	IplImage* image = altURI.GetFrame( 0, 0, 0, 0 );

	if ( CV_IS_IMAGE( image ) )
	{
		iRawHeight = image->height;
		iRawWidth = image->width;
	}
	else
	{
		SetDlgItemText(IDC_FRAME_IMAGE,_T("Image: (error: none found)"));
		return;
	}
	
	std::stringstream out;

	CString cImageType = iImageType == 0 ? TEXT("Raw OpenCV") : TEXT("EMD");

	out << TEXT("Image:") << cImageType << TEXT(",src:(w,h):(") <<
		iRawWidth << TEXT(",") << iRawHeight << TEXT("),req:(w,h):(")
		<< iRequestedWidth << TEXT(",") << iRequestedHeight << TEXT("),offset:(x,y):(")
		<< iRequestedX  << TEXT(",") << iRequestedY << TEXT(")");

	SetDlgItemText( IDC_FRAME_IMAGE, out.str().c_str() );
}

//=========================================================================
void CMainDlg :: OnBnClickedButtonView()
{
	if ( bPlaying )
	{
		m_labelFPS.SetWindowText( TEXT("Display: 0 fps") );
		m_button.SetWindowText( TEXT("Play") );
		KillTimer( IDC_TIMER_IMG );
		m_buttonRecordImage.EnableWindow(FALSE);
		m_cboFrameRate.EnableWindow(TRUE);
		SetDlgItemText(IDC_FRAME_IMAGE,_T("Image: (none)"));
	}
	else
	{
		altURI.SetImageSource( iImageType );

		m_button.SetWindowText( TEXT("Stop") );
		m_buttonRecordImage.EnableWindow(TRUE);
		m_cboFrameRate.EnableWindow(FALSE);

		SetImageFrameTitleToImageSpecs( iImageWidth, iImageHeight, iImageX, iImageY, iImageType );

		int iSelect = m_cboFrameRate.GetCurSel();

		iTimerDelay = m_cboFrameRate.GetItemData(iSelect);
		SetTimer( IDC_TIMER_IMG, iTimerDelay, NULL );
	}
	bPlaying = !bPlaying;
}

//=========================================================================
bool IsButtonCommand( const int iID )
{
	return (mCommandButtons[iID].m_iIndex > 0);
}

//=========================================================================
void DoCommand( const int iID )
{
	if ( mCommandButtons[iID].m_bIsDrive )
		altURI.CommandDrive( mCommandButtons[iID].m_iIndex, mCommandButtons[iID].m_iSign );		
	else
		altURI.CommandOther( mCommandButtons[iID].m_iIndex, mCommandButtons[iID].m_iSign );		
}

//=========================================================================
void CMainDlg::SetConnectControls( BOOL BNewValue )
{
	// m_buttonConnect.EnableWindow( BNewValue );

	m_buttonInject.EnableWindow( !BNewValue );

	// only allow window hiding if local image i.e UT is running on this pc (and not real robot)
	m_buttonHide.EnableWindow( sWindowTitle.length() > 1 && bLocalImage && BNewValue && m_RadioSim.GetCheck( ) == BST_CHECKED );

	m_button.EnableWindow( BNewValue );
	m_buttonImageType.EnableWindow( BNewValue );
	m_cboFrameRate.EnableWindow( BNewValue );

	m_chkVisionOnly.EnableWindow( !BNewValue );
	
	if ( bConnectVisionOnly ) return;

	m_buttonViewport.EnableWindow( BNewValue );
	m_buttonCommands.EnableWindow( BNewValue );
	m_buttonNewRobot.EnableWindow( BNewValue );
	m_buttonSensors.EnableWindow( BNewValue );
	m_buttonMoveStop.EnableWindow( BNewValue );

	if ( BNewValue == TRUE )
	{
		if ( IsButtonCommand( IDC_BUTTON_RIGHT ) ) m_buttonMoveRight.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_LEFT ) ) m_buttonMoveLeft.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_FORWARD ) ) m_buttonMoveForward.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_BACK ) ) m_buttonMoveBack.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_UP ) ) m_buttonMoveUp.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_DOWN ) ) m_buttonMoveDown.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_FORWARD_LEFT ) ) m_buttonMoveForwardLeft.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_FORWARD_RIGHT ) ) m_buttonMoveForwardRight.EnableWindow( BNewValue );

		if ( IsButtonCommand( IDC_BUTTON_CAM1 ) ) m_buttonMoveCam1.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM2 ) ) m_buttonMoveCam2.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM3 ) ) m_buttonMoveCam3.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM4 ) ) m_buttonMoveCam4.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM5 ) ) m_buttonMoveCam5.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM6 ) ) m_buttonMoveCam6.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_CAM7 ) ) m_buttonMoveCam7.EnableWindow( BNewValue );

		if ( IsButtonCommand( IDC_BUTTON_E1 ) ) m_buttonE1.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_E2 ) ) m_buttonE2.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_E3 ) ) m_buttonE3.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_E4 ) ) m_buttonE4.EnableWindow( BNewValue );
		if ( IsButtonCommand( IDC_BUTTON_E5 ) ) m_buttonE5.EnableWindow( BNewValue );
	}
	else
	{
		m_buttonMoveRight.EnableWindow( BNewValue );
		m_buttonMoveLeft.EnableWindow( BNewValue );
		m_buttonMoveForward.EnableWindow( BNewValue );
		m_buttonMoveBack.EnableWindow( BNewValue );
		m_buttonMoveUp.EnableWindow( BNewValue );
		m_buttonMoveDown.EnableWindow( BNewValue );
		m_buttonMoveForwardLeft.EnableWindow( BNewValue );
		m_buttonMoveForwardRight.EnableWindow( BNewValue );

		m_buttonMoveCam1.EnableWindow( BNewValue );
		m_buttonMoveCam2.EnableWindow( BNewValue );
		m_buttonMoveCam3.EnableWindow( BNewValue );
		m_buttonMoveCam4.EnableWindow( BNewValue );
		m_buttonMoveCam5.EnableWindow( BNewValue );
		m_buttonMoveCam6.EnableWindow( BNewValue );
		m_buttonMoveCam7.EnableWindow( BNewValue );

		m_buttonE1.EnableWindow( BNewValue );
		m_buttonE2.EnableWindow( BNewValue );
		m_buttonE3.EnableWindow( BNewValue );
		m_buttonE4.EnableWindow( BNewValue );
		m_buttonE5.EnableWindow( BNewValue );
	}

}

//=========================================================================
void CMainDlg::OnBnClickedOk2()		// connect ALL
{
	// switch to state allowing show images	
	bPlaying = true;
	OnBnClickedButtonView();

	if ( m_RadioReal.GetCheck( ) != BST_CHECKED && m_RadioSim.GetCheck( ) != BST_CHECKED ) return;

	if ( bConnected )
	{
		m_buttonConnect.SetWindowText("Connect");
		SetConnectControls( FALSE );
		
		altURI.ClosealtURI();
	}
	else
	{
		if ( m_RadioSim.GetCheck() == BST_CHECKED )
		{
			if ( !ConnectSimulator() ) return;
		}
		else if ( m_RadioReal.GetCheck() == BST_CHECKED )
		{
			int iSelect = m_cboRobotDLL.GetCurSel();
		
			CString csRobotDLL = _TEXT("???");
		
			if ( iSelect != CB_ERR ) m_cboRobotDLL.GetLBText( iSelect, csRobotDLL );

			tString sRobotDLL( csRobotDLL );

			if ( !ConnectReal( ExtractAppPath() + sRobotDLL ) ) return;
		}
		
		LoadButtons();									// get values from ini file
		SetConnectControls( TRUE );
		m_buttonConnect.SetWindowText("Disconnect");
	}
	bConnected = !bConnected;

}

//=========================================================================
void CMainDlg::OnBnClickedVisionOnly()
{
	// TODO: Add your control notification handler code here
	
	bConnectVisionOnly = ( m_chkVisionOnly.GetCheck( ) == BST_CHECKED );

}

//=========================================================================
void CMainDlg::OnBnClickedRadioSource()
{
	bConnected = true;	// i.e. turn off any images
	OnBnClickedOk2();	// but left true if no radio checked so set below

	if ( m_RadioSim.GetCheck( ) == BST_CHECKED ) 
	{
		m_buttonConnect.EnableWindow( TRUE );
		m_chkVisionOnly.EnableWindow( TRUE );
		m_cboRobotDLL.EnableWindow(FALSE);
		m_buttonInject.EnableWindow(TRUE);
		m_RadioReal.SetCheck( BST_UNCHECKED );
	}
	else if ( m_RadioReal.GetCheck( ) == BST_CHECKED ) 
	{
		m_buttonConnect.EnableWindow( TRUE );
		m_chkVisionOnly.EnableWindow( TRUE );
		m_cboRobotDLL.EnableWindow(TRUE);
		m_buttonInject.EnableWindow(FALSE);
		m_RadioSim.SetCheck( BST_UNCHECKED );
	}
	else
	{
		m_buttonConnect.EnableWindow( FALSE );
		m_chkVisionOnly.EnableWindow( FALSE );
		m_button.EnableWindow(FALSE);
		m_buttonInject.EnableWindow(FALSE);
		m_cboFrameRate.EnableWindow(FALSE);
		bConnected = false;
	}


}

//=========================================================================
void CMainDlg::OnBnClickedRadioSim()
{
	// TODO: Add your control notification handler code here
	OnBnClickedRadioSource();
}

//=========================================================================
void CMainDlg::OnBnClickedRadioReal()
{
	// TODO: Add your control notification handler code here
	OnBnClickedRadioSource();
}

//=========================================================================
void CMainDlg::OnBnClickedRecordImage()
{
	// TODO: Add your control notification handler code here
	if ( bRecording )
	{
		m_buttonRecordImage.SetWindowText("Save AVI");
		m_buttonImageType.EnableWindow( TRUE );
		cvReleaseVideoWriter(&writer);
	}
	else
	{
		if ( altURI.GetSize().height == 0 ) return;

		CString cFile = TEXT("out.avi");

		CFileDialog fdlg(	FALSE,
							TEXT("*.avi"),
							cFile,
							OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
							TEXT("movie file (*.avi)"),
							NULL);

		fdlg.m_ofn.lpstrTitle= TEXT("Save Movie");

		if ( fdlg.DoModal() != IDOK ) return;
		
		cFile = fdlg.GetPathName();  // contain the selected filename

		int iSelect = m_cboFrameRate.GetCurSel();
		
		CString fps = TEXT("25");	// default
		
		if ( iSelect != CB_ERR ) m_cboFrameRate.GetLBText(iSelect, fps);

		//writer = cvCreateVideoWriter( cFile, CV_FOURCC('P','I','M','1'), atof((LPCSTR)fps), altURI.GetSize() );
		writer = cvCreateVideoWriter( cFile, CV_FOURCC('D', 'I', 'V', 'X'), atof((LPCSTR)fps), altURI.GetSize() );
		// writer = cvCreateVideoWriter( cFile, CV_FOURCC_PROMPT , atof((LPCSTR)fps), altURI.GetSize() );

		if (!writer) return;

		m_buttonRecordImage.SetWindowText("Stop AVI");
		m_buttonImageType.EnableWindow( FALSE );	// don't allow image changing during movie
	}
	bRecording = !bRecording;
}


//=========================================================================
void CMainDlg::OnBnClickedOk4()
{
	// TODO: Add your control notification handler code here

	CProcessDlg proc_dlg;

	INT_PTR nResponse = proc_dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
		m_buttonConnect.EnableWindow( TRUE );
	}
	//else
	//	m_buttonConnect.EnableWindow( FALSE );
}

//=========================================================================
void CMainDlg::OnBnClickedOk5()
{
	// TODO: Add your control notification handler code here

	CConsoleDlg proc_dlg;

	INT_PTR nResponse = proc_dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

}

//=========================================================================
void CMainDlg::OnBnClickedOk6()
{
	// Toggle sim window visible/hide
	::ShowWindow( ::FindWindow( NULL, sWindowTitle.c_str()), bUnrealVisible ? SW_HIDE : SW_SHOW );
	m_buttonHide.SetWindowText( !bUnrealVisible ? "Hide" : "Show" );
	bUnrealVisible = !bUnrealVisible;
}

//=========================================================================
void CMainDlg::OnBnClickedSensors()
{
	CSensorDlg proc_dlg;

	INT_PTR nResponse = proc_dlg.DoModal();

	altURI._SetCatchMessages( false );

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
}

//=========================================================================
void CMainDlg::OnBnClickedOk()	// exit
{
	// TODO: Add your control notification handler code here
	if ( bConnected ) OnBnClickedOk2();
	OnCancel();
}

//=========================================================================
void CMainDlg::OnBnClickedButtonBack()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_BACK );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonLeft()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_LEFT );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonForward()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_FORWARD );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonRight()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_RIGHT );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonUp()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_UP );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonDown()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_DOWN );
}

//=========================================================================
void CMainDlg:: OnLButtonUp(UINT nHitTest, CPoint point)
{
	CAboutDlg proc_dlg;		// show about box

	INT_PTR nResponse = proc_dlg.DoModal();

	UNREFERENCED_PARAMETER(nResponse);
	UNREFERENCED_PARAMETER(nHitTest);
	UNREFERENCED_PARAMETER(point);
}

//=========================================================================
void SetCommand( const int iID,  const std::string& sSection, const std::string& sPrefix )
{
	mCommandButtons[iID] = CaURIButton(	IniFile.GetValueB( sSection, sPrefix + "CommandDrive", true ),
										IniFile.GetValueI( sSection, sPrefix + "Index", 0 ),
										IniFile.GetValueI( sSection, sPrefix + "Sign", 0 ) );
}

//=========================================================================
void CMainDlg::LoadButtons()
{
	// buttons are robot specific based on (lowercase) name of ini file
	tString sSection =  tsIniRoot + TEXT( "_arrowbuttons" );

	mCommandButtons.clear();

	SetCommand( IDC_BUTTON_LEFT, sSection, TEXT( "Left" ) );
	SetCommand( IDC_BUTTON_RIGHT, sSection, TEXT( "Right" ) );
	SetCommand( IDC_BUTTON_FORWARD, sSection, TEXT( "Forward" ) );
	SetCommand( IDC_BUTTON_BACK, sSection, TEXT( "Back" ) );
	SetCommand( IDC_BUTTON_UP, sSection, TEXT( "Up" ) );
	SetCommand( IDC_BUTTON_DOWN, sSection, TEXT( "Down" ) );
	SetCommand( IDC_BUTTON_FORWARD_LEFT, sSection, TEXT( "ForwardLeft" ) );
	SetCommand( IDC_BUTTON_FORWARD_RIGHT, sSection, TEXT( "ForwardRight" ) );

	sSection = tsIniRoot + TEXT( "_camerabuttons" );
	SetCommand( IDC_BUTTON_CAM1, sSection, TEXT( "CAM1" ) );
	SetCommand( IDC_BUTTON_CAM2, sSection, TEXT( "CAM2" ) );
	SetCommand( IDC_BUTTON_CAM3, sSection, TEXT( "CAM3" ) );
	SetCommand( IDC_BUTTON_CAM4, sSection, TEXT( "CAM4" ) );
	SetCommand( IDC_BUTTON_CAM5, sSection, TEXT( "CAM5" ) );
	SetCommand( IDC_BUTTON_CAM6, sSection, TEXT( "CAM6" ) );
	SetCommand( IDC_BUTTON_CAM7, sSection, TEXT( "CAM7" ) );

	sSection = tsIniRoot + TEXT( "_extrabuttons" );
	SetCommand( IDC_BUTTON_E1, sSection, TEXT( "E1" ) );
	SetCommand( IDC_BUTTON_E2, sSection, TEXT( "E2" ) );
	SetCommand( IDC_BUTTON_E3, sSection, TEXT( "E3" ) );
	SetCommand( IDC_BUTTON_E4, sSection, TEXT( "E4" ) );
	SetCommand( IDC_BUTTON_E5, sSection, TEXT( "E5" ) );

}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam1()
{
	DoCommand( IDC_BUTTON_CAM1 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam2()
{
	DoCommand( IDC_BUTTON_CAM2 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam3()
{
	DoCommand( IDC_BUTTON_CAM3 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam4()
{
	DoCommand( IDC_BUTTON_CAM4 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam5()
{
	DoCommand( IDC_BUTTON_CAM5 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam6()
{
	DoCommand( IDC_BUTTON_CAM6 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonCam7()
{
	DoCommand( IDC_BUTTON_CAM7 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonE1()
{
	DoCommand( IDC_BUTTON_E1 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonE2()
{
	DoCommand( IDC_BUTTON_E2 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonE3()
{
	DoCommand( IDC_BUTTON_E3 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonE4()
{
	DoCommand( IDC_BUTTON_E4 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonE5()
{
	DoCommand( IDC_BUTTON_E5 );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonForwardRight()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_FORWARD_RIGHT );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonForwardLeft()
{
	// TODO: Add your control notification handler code here
	DoCommand( IDC_BUTTON_FORWARD_LEFT );
}

//=========================================================================
void CMainDlg::OnBnClickedImageView()
{
	std::string sClientIP( altURI.GetLocalAddress() );
	std::string sCommand = TEXT("SET {Type Camera}");
	std::string sClientBit = TEXT(" {Client ") + sClientIP + TEXT("}");
	
	// make ini key for next camera name and increment camera number
	std::stringstream out;
	out << TEXT("CameraName") << iNextCameraNumber++;

	std::string sCameraNameBit = TEXT( "{Name " ) + IniFile.GetValue( tsIniRoot + TEXT( "_cameras" ), out.str() ) + TEXT( "}" );

	// if no more cameras
	if ( iNextCameraNumber > IniFile.GetValueI( tsIniRoot + TEXT( "_cameras" ), TEXT("NoOfCameras") ) + 1 )
	{
		iNextCameraNumber = 1;		// set back to first camera for next time
		sCommand += sClientBit;		// set to client view (not camera)
	}
	else							// set to next camera
		sCommand += TEXT(" {Robot ") + sRobotName + TEXT("}") + sCameraNameBit + sClientBit;

	altURI.SendCommand( sCommand );
}

//=========================================================================
void CMainDlg::OnBnClickedButtonNewrobot()
{
	// TODO: Add your control notification handler code here
	std::string sInit = IniFile.GetValue( "inits", "RobotInitCommandLast" );
	std::string sStart = IniFile.GetValue( "playerstarts", "MapPlayerStartLast" );
	std::stringstream out;
	out << iNextRobotNumber++;
	sRobotName = IniFile.GetValue( "altURI_UI settings", "RobotNamePrefix" ) + out.str();

	altURI.SendCommand( sInit + TEXT(" ") + sStart + TEXT(" {Name ") + sRobotName + TEXT("}") );
	iNextCameraNumber = 1;
}

//=========================================================================
void CMainDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	altURI.CommandDriveAllStop();
}


//=========================================================================
void CMainDlg::OnBnClickedButtonImageType()
{
	// TODO: Add your control notification handler code here
	CImageDlg proc_dlg;

	int iOldImageType = iImageType;

	INT_PTR nResponse = proc_dlg.DoModal();
	if (nResponse == IDOK)
	{
		if ( iOldImageType != iImageType )
			altURI.SetImageSource( iImageType );

		IniFile.SetValueI( "altURI_UI settings", "ImageHeight" , iImageHeight);
		IniFile.SetValueI( "altURI_UI settings", "ImageWidth", iImageWidth );
		IniFile.SetValueI( "altURI_UI settings", "ImageX", iImageX );
		IniFile.SetValueI( "altURI_UI settings", "ImageY", iImageY );
		IniFile.SetValueI( "altURI_UI settings", "ImageType", iImageType );

		// show current image values
		SetImageFrameTitleToImageSpecs( iImageWidth, iImageHeight, iImageX, iImageY, iImageType );
	}

}
