// gtComPortTestDlg.h : header file
//

#if !defined(AFX_GTCOMPORTTESTDLG_H__474433AB_E45B_4D3A_AD3A_AE30FE5C4027__INCLUDED_)
#define AFX_GTCOMPORTTESTDLG_H__474433AB_E45B_4D3A_AD3A_AE30FE5C4027__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CgtComPortTestDlg dialog

//-------------------------------------------------------------
// gtComPort를 사용하기 위해 해드파일을 연다.
//-------------------------------------------------------------
#include "gtComPort.h"


class CgtComPortTestDlg : public CDialog
{
// Construction
public:
	CgtComPortTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CgtComPortTestDlg)
	enum { IDD = IDD_GTCOMPORTTEST_DIALOG };
	CString	m_strEditCOM1;
	CString	m_strEditCOM2;
	UINT	m_uiPortNumber;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CgtComPortTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

//-------------------------------------------------------------
// 송수신에 필요한 COM1, COM2 객체를 선언한다.
//-------------------------------------------------------------
	CgtComPort m_portCOM1;
	CgtComPort m_portCOM2;


	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CgtComPortTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClear();
	afx_msg void OnButtonSend1();
	afx_msg void OnButtonSend2();
	afx_msg void OnDestroy();
	afx_msg void OnButtonPorttest();
	//}}AFX_MSG
//-------------------------------------------------------------
// afx_msg LRESULT OnCommNotify(WPARAM, LPARAM)
// : 데이터 수신 시 자동으로 호출되는 함수 선언
//-------------------------------------------------------------
	afx_msg LRESULT OnCommNotify(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GTCOMPORTTESTDLG_H__474433AB_E45B_4D3A_AD3A_AE30FE5C4027__INCLUDED_)
