// gtComPortTest.h : main header file for the GTCOMPORTTEST application
//

#if !defined(AFX_GTCOMPORTTEST_H__2942A12C_50C4_4054_B087_6318EB330F1A__INCLUDED_)
#define AFX_GTCOMPORTTEST_H__2942A12C_50C4_4054_B087_6318EB330F1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CgtComPortTestApp:
// See gtComPortTest.cpp for the implementation of this class
//

class CgtComPortTestApp : public CWinApp
{
public:
	CgtComPortTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CgtComPortTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CgtComPortTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GTCOMPORTTEST_H__2942A12C_50C4_4054_B087_6318EB330F1A__INCLUDED_)
