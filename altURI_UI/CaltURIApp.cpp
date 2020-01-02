
// OCVG_ui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CaltURIApp.h"
#include "MainDlg.h"


// CaltURIApp

BEGIN_MESSAGE_MAP(CaltURIApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CaltURIApp construction

CaltURIApp::CaltURIApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CaltURIApp object

CaltURIApp theApp;

// CaltURIApp initialization

BOOL CaltURIApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CMainDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


//=========================================================================
void set_DropDownSize(CComboBox& box, UINT LinesToDisplay)
/*--------------------------------------------------------------------------
 * Purpose:       Set the proper number of lines in a drop-down list or
 *                combo box.
 * Description:   Resizes the combo box window to fit the proper number
 *                of lines. The window must exist before calling this function.
 *  This function should be called when the combo box is created, and when
 *  the font of the combo box changes. (e.g. WM_SETTINGCHANGE)
 *  Testing needed:
 *    Are there cases where SM_CYBORDER should be used instead of SM_CYEDGE?
 *    owner-draw variable height combo box
 *    Subclassed combo box with horizontal scroll-bar
 * Returns:       nothing
 * Author:        KTM
 *--------------------------------------------------------------------------*/
{
    ASSERT(IsWindow(box));	// Window must exist or SetWindowPos won't work

    CRect cbSize;			// current size of combo box
    int Height;             // new height for drop-down portion of combo box

    box.GetClientRect(cbSize);
    Height = box.GetItemHeight(-1);      // start with size of the edit-box portion
    Height += box.GetItemHeight(0) * LinesToDisplay;	// add height of lines of text

    // Note: The use of SM_CYEDGE assumes that we're using Windows '95
    // Now add on the height of the border of the edit box
    Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

    // The height of the border of the drop-down box
    Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

    // now set the size of the window
    box.SetWindowPos(NULL,            // not relative to any other windows
        0, 0,                         // TopLeft corner doesn't change
        cbSize.right, Height,         // existing width, new height
        SWP_NOMOVE | SWP_NOZORDER     // don't move box or change z-ordering.
        );
}
