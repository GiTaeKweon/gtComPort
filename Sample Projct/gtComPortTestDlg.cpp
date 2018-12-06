// gtComPortTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gtComPortTest.h"
#include "gtComPortTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CgtComPortTestDlg dialog

CgtComPortTestDlg::CgtComPortTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CgtComPortTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CgtComPortTestDlg)
	m_strEditCOM1 = _T("");
	m_strEditCOM2 = _T("");
	m_uiPortNumber = 1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgtComPortTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CgtComPortTestDlg)
	DDX_Text(pDX, IDC_EDIT_COM1, m_strEditCOM1);
	DDX_Text(pDX, IDC_EDIT_COM2, m_strEditCOM2);
	DDX_Text(pDX, IDC_EDIT_PORTNUMBER, m_uiPortNumber);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CgtComPortTestDlg, CDialog)
	//{{AFX_MSG_MAP(CgtComPortTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_BUTTON_SEND1, OnButtonSend1)
	ON_BN_CLICKED(IDC_BUTTON_SEND2, OnButtonSend2)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_PORTTEST, OnButtonPorttest)
	//}}AFX_MSG_MAP
//-------------------------------------------------------------
// ON_MESSAGE(WM_COMM_NOTIFY, OnCommNotify)
// : 데이터 수신 시 자동으로 호출되는 Message_Map 선언
//-------------------------------------------------------------
	ON_MESSAGE(WM_COMM_NOTIFY, OnCommNotify)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CgtComPortTestDlg message handlers

BOOL CgtComPortTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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



	
//-------------------------------------------------------------
// 다이얼 로그 오픈시 COM1, COM2 포트을 연다.
//-------------------------------------------------------------


//-------------------------------------------------------------
//	첫번째 openPort 함수 사용 예
//-------------------------------------------------------------
//	BOOL openPort(	int nPort,	
//					CWnd* pParentWnd = NULL, 
//					DWORD baudrate = CBR_9600, 
//					BYTE bytesize = 8, 
//					BYTE parity = NOPARITY, 
//					BYTE stopbits = ONESTOPBIT );
//-------------------------------------------------------------

	// TODO: Add extra initialization here
	if( TRUE != m_portCOM1.openPort(1,this) )
		MessageBox("COM 1을 열수 없습니다.");

	if( TRUE != m_portCOM2.openPort(2,this) )
		MessageBox("COM 2을 열수 없습니다.");


/*
//-------------------------------------------------------------
//	두번째 openPort 함수 사용 예
//-------------------------------------------------------------
//	BOOL openPort(	int nPort, 
//					LPDCB lpDCB, 
//					CWnd* pParentWnd = NULL);
//-------------------------------------------------------------
	DCB dcb;
	m_portCOM1.getPortState(1, &dcb);

	// 통신포트 설정
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
//	dcb.fOutxCtsFlow = 0;
//	dcb.fOutxDsrFlow = 0;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fRtsControl = RTS_CONTROL_ENABLE;
//	dcb.fInX = TRUE;
//	dcb.fOutX = TRUE;			// Xon, Xoff 사용
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	if( FALSE == m_portCOM1.openPort(1, &dcb, this) )
		this->MessageBox("쓰기 포트를 열수 없습니다.");


	m_portCOM2.getPortState(2, &dcb);

	// 통신포트 설정
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
//	dcb.fOutxCtsFlow = 0;
//	dcb.fOutxDsrFlow = 0;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fRtsControl = RTS_CONTROL_ENABLE;
//	dcb.fInX = TRUE;
//	dcb.fOutX = TRUE;			// Xon, Xoff 사용
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;


	if( FALSE == m_portCOM2.openPort(2, &dcb, this) )
		this->MessageBox("읽기 포트를 열수 없습니다.");

*/
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CgtComPortTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CgtComPortTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CgtComPortTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CgtComPortTestDlg::OnClear() 
{
	// TODO: Add your control notification handler code here
	m_strEditCOM1.Empty();
	m_strEditCOM2.Empty();
	
	UpdateData(FALSE);
}

void CgtComPortTestDlg::OnButtonSend1() 
{
	// TODO: Add your control notification handler code here
 	UpdateData(TRUE);
 
//-------------------------------------------------------------
// CString을 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
//	m_portCOM1.sendString( m_strEditCOM1 );

//-------------------------------------------------------------
// 한 BYTE를 송신한다.
//-------------------------------------------------------------
//	m_portCOM1.sendByte('k');


//-------------------------------------------------------------
// BYTE 포인터를 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
	int size = m_strEditCOM1.GetLength();
	BYTE* pByte = (BYTE*)LPCTSTR(m_strEditCOM1);

	m_portCOM1.changePortState(CBR_9600);
	m_portCOM2.changePortState(CBR_9600);

	m_portCOM1.sendBytes(pByte, size);

//-------------------------------------------------------------
// char 포인터를 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
//	char* sz = "abcdefghij";
//	m_portCOM1.sendBytes((BYTE*)sz, 10);

//-------------------------------------------------------------
// BYTE 배열을 사용해서 데이터를 송신한다.
//-------------------------------------------------------------
//	BYTE aByte[10] = {	0x65, 0x66, 0x67, 0x68, 0x69, 
//						0x6a, 0x6b, 0x6c, 0x6d, 0x6e	};
//	m_portCOM1.sendBytes(aByte, 10);	
}

void CgtComPortTestDlg::OnButtonSend2() 
{
	// TODO: Add your control notification handler code here
 	UpdateData(TRUE);

//-------------------------------------------------------------
// CString을 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
//	m_portCOM2.sendString( m_strEditCOM2 );

//-------------------------------------------------------------
// 한 BYTE를 송신한다.
//-------------------------------------------------------------
//	m_portCOM2.sendByte('k');


//-------------------------------------------------------------
// BYTE 포인터를 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
	int size = m_strEditCOM2.GetLength();
	BYTE* pByte = (BYTE*)LPCTSTR(m_strEditCOM2);
	
	m_portCOM1.changePortState(CBR_2400);
	m_portCOM2.changePortState(CBR_2400);

	m_portCOM2.sendBytes(pByte, size);

//-------------------------------------------------------------
// char 포인터를 사용해서 문자열을 송신한다.
//-------------------------------------------------------------
//	char* sz = "abcdefghij";
//	m_portCOM2.sendBytes((BYTE*)sz, 10);

//-------------------------------------------------------------
// BYTE 배열을 사용해서 데이터를 송신한다.
//-------------------------------------------------------------
//	BYTE aByte[10] = {	0x65, 0x66, 0x67, 0x68, 0x69, 
//						0x6a, 0x6b, 0x6c, 0x6d, 0x6e	};
//	m_portCOM2.sendBytes(aByte, 10);
}


void CgtComPortTestDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	m_portCOM1.closePort();
	m_portCOM2.closePort();	
		
}


//-------------------------------------------------------------
// 데이터 수신시 자동으로 호출되는 함수이다.
//-------------------------------------------------------------
LRESULT CgtComPortTestDlg::OnCommNotify(WPARAM wParam, LPARAM lParam)
{

	int nPort = lParam;
	CString strReceive;

//-------------------------------------------------------------
// 어느 Port에서 수신되었는지 검사해서 데이터를 문자열로 읽어온다.
//-------------------------------------------------------------
//	unsigned char* szReceive;
//	int nByteSize; 
//	if( nPort == 1 )
//	{
//		nByteSize = m_portCOM1.getReceivedByteSize();
		// +1 은 문자열의 마지막을 알리는 NULL를 넣은 자리이다.
//		szReceive = new unsigned char[nByteSize+1];
//		m_portCOM1.receiveBytes(szReceive, nByteSize);
//		szReceive[nByteSize] = NULL;
//		strReceive = szReceive;
//		delete [] szReceive;
//	}
//	else if( nPort == 2 )
//	{
//		nByteSize = m_portCOM2.getReceivedByteSize();
		// +1 은 문자열의 마지막을 알리는 NULL를 넣은 자리이다.
//		szReceive = new unsigned char[nByteSize+1];
//		m_portCOM2.receiveBytes(szReceive, nByteSize);
//		szReceive[nByteSize] = NULL;
//		strReceive = szReceive;
//		delete [] szReceive;
//	}
//	else
//		TRACE("COM port number error.");



//-------------------------------------------------------------
// 어느 Port에서 수신되었는지 검사해서 데이터를 CString으로 읽어온다.
//-------------------------------------------------------------
	if( nPort == 1 )
		m_portCOM1.receiveString( strReceive );
	else if( nPort == 2 )
		m_portCOM2.receiveString( strReceive );
	else
		TRACE("COM port number error.");

	
//-------------------------------------------------------------
// 수신 데이터를 에디터박스에 디스플레이한다.
//-------------------------------------------------------------
	if( nPort == 1 )
	{
		if( m_strEditCOM1.GetLength() < 1024 )
			m_strEditCOM1 += strReceive;
	}
	else if( nPort == 2 )
	{
		if( m_strEditCOM2.GetLength() < 1024 )
			m_strEditCOM2 += strReceive;
	}
	else
		TRACE("COM port number error.");

 	
	UpdateData(FALSE);


	return 1;
} 

void CgtComPortTestDlg::OnButtonPorttest() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
 	UpdateData(TRUE);

	CString temp;
	
	if(m_uiPortNumber < 1) return;

	if(TRUE == CgtComPort::checkPort( m_uiPortNumber ))
	{
		temp.Format("COM%d 은 사용이 가능합니다.",m_uiPortNumber);
		MessageBox(temp);
	}
	else
	{
		temp.Format("COM%d 은 사용이 불가능하거나, 이미 사용중입니다.",m_uiPortNumber);
		MessageBox(temp);
	}	
}
