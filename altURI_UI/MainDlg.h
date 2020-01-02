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
//
// CMainDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "COpenCVImage.h"


// CMainDlg dialog
class CMainDlg : public CResizeDlg
{
// Construction
public:
	CMainDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MAIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	COpenCVImage m_OCVGImage;
	afx_msg void OnBnClickedButtonView();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonUp(UINT nHitTest, CPoint point);
	CButton m_button;
	CComboBox m_cboFrameRate;
	CButton m_chkVisionOnly;
	afx_msg void OnBnClickedVisionOnly();
	CButton m_buttonConnect;
	afx_msg void OnBnClickedOk2();
	CButton m_buttonRecordImage;
	CButton m_buttonImageType;
	afx_msg void OnBnClickedRadioSource();
	CComboBox m_cboRobotDLL;
	CButton m_RadioSim;
	CButton m_RadioReal;
	afx_msg void SetConnectControls( BOOL BNewValue );
	afx_msg void LoadButtons( void );
	afx_msg void OnBnClickedRadioSim();
	afx_msg void OnBnClickedRadioReal();
	afx_msg void OnBnClickedSensors();
	afx_msg void OnBnClickedRecordImage();
	CButton m_buttonMoveRight;
	CButton m_buttonMoveLeft;
	CButton m_buttonMoveForward;
	CButton m_buttonMoveBack;
	CButton m_buttonMoveUp;
	CButton m_buttonMoveDown;
	afx_msg void OnBnClickedOk4();
	CButton m_buttonInject;
	CButton m_buttonCommands;
	afx_msg void OnBnClickedOk5();
	CButton m_buttonHide;
	afx_msg void OnBnClickedOk6();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonForward();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	CButton m_buttonMoveCam1;
	CButton m_buttonMoveCam2;
	CButton m_buttonMoveCam3;
	CButton m_buttonMoveCam4;
	CButton m_buttonMoveCam5;
	CButton m_buttonMoveCam6;
	CButton m_buttonMoveCam7;
	CButton m_buttonE1;
	CButton m_buttonE2;
	CButton m_buttonE3;
	CButton m_buttonE4;
	CButton m_buttonE5;
	afx_msg void OnBnClickedButtonCam1();
	afx_msg void OnBnClickedButtonCam2();
	afx_msg void OnBnClickedButtonCam3();
	afx_msg void OnBnClickedButtonCam4();
	afx_msg void OnBnClickedButtonCam5();
	afx_msg void OnBnClickedButtonCam6();
	afx_msg void OnBnClickedButtonCam7();
	afx_msg void OnBnClickedButtonE1();
	afx_msg void OnBnClickedButtonE2();
	afx_msg void OnBnClickedButtonE3();
	afx_msg void OnBnClickedButtonE4();
	afx_msg void OnBnClickedButtonE5();
	CButton m_buttonViewport;
	afx_msg void OnBnClickedImageView();
	CButton m_buttonNewRobot;
	afx_msg void OnBnClickedButtonNewrobot();
	CButton m_buttonSensors;
	CButton m_buttonMoveStop;
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonForwardRight();
	afx_msg void OnBnClickedButtonForwardLeft();
	CButton m_buttonMoveForwardLeft;
	CButton m_buttonMoveForwardRight;
	CStatic m_labelFPS;
	afx_msg void OnBnClickedButtonImageType();

private:
	void SetImageFrameTitleToImageSpecs(	const int iRequestedWidth,
											const int iRequestedHeight,
											const int iRequestedX,
											const int iRequestedY,
											const int iImageType);
};
