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
// CProcessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaltURIApp.h"
#include "ProcessDlg.h"
#include "CDllInject.h"

extern tString tsHookDllPath;
extern CIniFile IniFile;
extern tString tsHookDll;

CDllInject myInjection;

int iMapsFound;

//==================================================================================================
bool fileExists(const std::string& sFileName)
{
	return ( GetFileAttributes(sFileName.c_str()) != 0xFFFFFFFF );
}

//==================================================================================================
int GetMapsList( const std::string& sPath, const std::string& sFileSpec, CListBox& m_lbo_maps )
{
	m_lbo_maps.ResetContent();	// clear listbox

	CFileFind finder;
	tString sPathWithFileSpec( sPath + TEXT("\\") + sFileSpec );
	
	BOOL bWorking = finder.FindFile( sPathWithFileSpec.c_str() );

	while (bWorking)
	{
	   bWorking = finder.FindNextFile();

	   if ( finder.IsDirectory() ) continue;

	   m_lbo_maps.AddString( finder.GetFileTitle() );
	}

	int r = m_lbo_maps.GetCount();
			
	if ( r == 0 )
		m_lbo_maps.AddString( TEXT("***No Maps found***") );

	return r;
}

//==================================================================================================
// CProcessDlg dialog

IMPLEMENT_DYNAMIC(CProcessDlg, CDialog)

CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessDlg::IDD, pParent)
{

}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_listProcess);
	DDX_Control(pDX, IDOK, m_buttonOK);
	DDX_Control(pDX, IDOK3, m_buttonStart);
	DDX_Control(pDX, IDC_LIST1, m_listMaps);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CProcessDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CProcessDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK3, &CProcessDlg::OnBnClickedOk3)
END_MESSAGE_MAP()

//=========================================================================
int FillList( CListBox& clist )
{
	clist.ResetContent();	// clear listbox

	std::list <CInjectableProcess>::iterator Iter = myInjection.m_lProcesses.begin();

	while( Iter != myInjection.m_lProcesses.end() )
	{
		clist.SetItemData( clist.AddString( Iter->m_sProcessName.c_str() ), Iter->m_dwProcessID );
		++Iter;
	}

	int r = clist.GetCount();
			
	if ( r == 0 )
		clist.AddString( TEXT("***No simulator process found***") );

	return r;
}

//=========================================================================
// CProcessDlg message handlers

BOOL CProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_buttonStart.EnableWindow(FALSE);

	tString sMapPath( IniFile.GetValue( "USARSim settings", "MapPath" ) );
	tString sMapFilter( IniFile.GetValue( "USARSim settings", "MapFilter" ) );

	iMapsFound = GetMapsList( sMapPath, sMapFilter, m_listMaps );

	tString sLastMap( IniFile.GetValue( "altURI_UI settings", "LastMap" ) );

	m_listMaps.SelectString( -1, sLastMap.c_str() );

#ifndef DEBUG
	myInjection.SetHookDllNameToExclude( tsHookDll );
#endif

	if ( !myInjection.FillProcesses() ) return TRUE;

	m_buttonOK.EnableWindow( FillList( m_listProcess ) );	// only try if we have at least one process

	m_buttonStart.EnableWindow(TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//=========================================================================
// CProcessDlg message handlers
void CProcessDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

//=========================================================================
void CProcessDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();

	if ( !fileExists( tsHookDllPath ) )
	{
		AfxMessageBox( TEXT("Sorry, can't find dll to inject"), MB_ICONERROR );
		return;
	}

	int iSelect = m_listProcess.GetCurSel();
	
	if ( iSelect != CB_ERR )
	{
		DWORD_PTR ProcessID = m_listProcess.GetItemData(iSelect);

		BeginWaitCursor();

		if ( myInjection.DoInject( ProcessID, tsHookDllPath ) )
		{
			EndWaitCursor();
			AfxMessageBox( TEXT("Injected OK!"), MB_OK );
		}
		else
			AfxMessageBox( TEXT("ERROR: Could not inject hook DLL!\n\nCheck OpenCV DLLs are available\n\n\tand\n\nYou have debug programs privilege\n\te.g. Admin"), MB_ICONERROR );


		EndWaitCursor();
	}

}

//=========================================================================
void CProcessDlg::OnBnClickedOk3()
{
	// start simulator

	int iSelect = m_listMaps.GetCurSel();
		
	CString csMap = TEXT("DM-USAR_red");	// default
		
	if ( iSelect != CB_ERR )
	{	
		BeginWaitCursor();

		m_listMaps.GetText( iSelect, csMap );

		tString sMap( (LPCTSTR)csMap );
		tString sParams( TEXT(" ") + sMap + IniFile.GetValue( "USARSim settings", "SimParams" ) );

		tString sAppPath( IniFile.GetValue( "USARSim settings", "SimPath" ) );
		tString sExeName( sAppPath + TEXT("\\") + IniFile.GetValue( "USARSim settings", "SimExe" ) + TEXT(" ") );
		
		bool bRes = myInjection.LaunchSimulator(	sAppPath,
													sExeName,
													sParams );

		if ( bRes )
		{
			std::string sWindowTitle( IniFile.GetValue( "USARSim settings", "WindowTitle" ) );
			int i = 0;
			while ( !FindWindow( NULL, sWindowTitle.c_str()) && i < 60 )
			{
				Sleep(1000); // wait for sim app (for upto 1 minute)
				i++;
			}
			SetForegroundWindow();
			EndWaitCursor();
			m_buttonStart.EnableWindow(FALSE);
			// save new value
			if ( IniFile.SetValue( "altURI_UI settings", "LastMap", sMap ) ) IniFile.WriteFile();
		}
		else
		{
			CString		strMsg;
			strMsg.Format("Unable to start simulator (%d).\n", GetLastError()); 
			AfxMessageBox( strMsg, MB_OK|MB_ICONSTOP );
		}

		if ( myInjection.FillProcesses() ) 
			m_buttonOK.EnableWindow( FillList( m_listProcess ) );	// only try if we have at least one process
		else
			m_buttonOK.EnableWindow( FALSE );

		EndWaitCursor();
	}
}

//=========================================================================
