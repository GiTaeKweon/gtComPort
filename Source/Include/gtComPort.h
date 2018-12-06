// gtComPort.h: interface for the CgtComPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GTSERIALPORT_H__D7E5B080_6BD6_44E6_BB89_4FEE2CC5C5C7__INCLUDED_)
#define AFX_GTSERIALPORT_H__D7E5B080_6BD6_44E6_BB89_4FEE2CC5C5C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//-----------------------------------------------
// Constants for Serial Port
//-----------------------------------------------
#define	WM_COMM_NOTIFY	(WM_USER + 1000)


const int QUEUE_SIZE			= 16384;

const unsigned char ASCII_LF	= 0x0a;
const unsigned char ASCII_CR	= 0x0d;
const unsigned char ASCII_XON	= 0x11;
const unsigned char ASCII_XOFF	= 0x13;

const int COMM_NOTIFY_BREAK		= 0x0000;
const int COMM_NOTIFY_CTS		= 0x0001;
const int COMM_NOTIFY_DSR		= 0x0002;
const int COMM_NOTIFY_LINEERR	= 0x0004;
const int COMM_NOTIFY_RING		= 0x0008;
const int COMM_NOTIFY_RLSD		= 0x0010;
const int COMM_NOTIFY_RXCHAR	= 0x0020;
const int COMM_NOTIFY_RXFLAG	= 0x0040;
const int COMM_NOTIFY_TXEMPTY	= 0x0080;
const int COMM_NOTIFY_ERROR		= 0xffff;

const int COMM_WITH_PC			= 1;
const int COMM_WITH_MICOM		= 2;

const DWORD NOT_OPENED_ERROR	= -2;
//-----------------------------------------------


class CgtQueue  
{

public:
	BOOL IsEmpty(void);
	BOOL IsFull(void);
	
	BOOL GetByte(BYTE& b);
	BOOL PutByte(BYTE b);
	int GetSize();
	void Clear();
	
	CgtQueue();
	virtual ~CgtQueue();

protected:

	int m_iHead;
	int m_iTail;
	BYTE m_aBuffer[QUEUE_SIZE];

};

//----------------------------------------------------------
// 용어 정리
//----------------------------------------------------------
//
// 송수신 Buffer : Com Port에 내장되어 통신에 사용되는 Buffer
//
// 수신 Queue : 수신 Buffer 에서 읽어낸 데이터를 담아두는 CgtQueue
//
//----------------------------------------------------------

class AFX_EXT_CLASS CgtComPort  
{

public:
	int getWaitingByteSize(void);
	BOOL changePortState(	DWORD baudrate = CBR_9600, 
							BYTE bytesize = 8, 
							BYTE parity = NOPARITY, 
							BYTE stopbits = ONESTOPBIT );
	BOOL setSendingMode(int nMode);
	BOOL getPortState(int nPort, LPDCB lpDCB);
	int getReceivedByteSize(void);
	static BOOL checkPort(int number);
	BOOL isOpened(void) { return m_bOpened; }

	CgtComPort();
	virtual ~CgtComPort();

	BOOL sendByte(BYTE byte);
	BOOL receiveByte(BYTE &byte);

	BOOL sendBytes(BYTE* pBuff, int nByteSize, int nDelayTime = 0);
	DWORD receiveBytes(BYTE* pBuff, int nByteSize);

	BOOL sendString(CString& strSend, int nDelayTime = 0);
	DWORD receiveString(CString& strReceive);

	BOOL closePort(void);
	BOOL openPort(	int nPort,	
					CWnd* pParentWnd = NULL, 
					DWORD baudrate = CBR_9600, 
					BYTE bytesize = 8, 
					BYTE parity = NOPARITY, 
					BYTE stopbits = ONESTOPBIT );

	BOOL openPort(	int nPort, 
					LPDCB lpDCB, 
					CWnd* pParentWnd = NULL);

	BOOL clearPort(void);

protected:
	static void readBytes(CgtComPort* port, COMSTAT comstat);
	static DWORD writeBytes(CgtComPort* port);
	static DWORD CommThread(CgtComPort* port);
	void writeMessage(char* errorTextm);

	HANDLE		m_hComPort;
	HANDLE		m_hShutdownEvent;
	HANDLE		m_hWriteEvent;
	CRITICAL_SECTION m_csCommSync;

	HANDLE		m_hEventArray[4];
	BOOL		m_bThreadAlive;
	HANDLE		m_hThreadWatchComm;

	int			m_nDelayTime;	// msec

	// OVERLAPPED structures
//	OVERLAPPED	m_ovWrite;
//	OVERLAPPED	m_ovRead;
	OVERLAPPED	m_ov;

	int			m_nPort;
	BOOL		m_bOpened;
	int			m_nSendingMode;

	// owner window
	CWnd*		m_pParent;

	CgtQueue		m_queueReceive;
	CgtQueue		m_queueSend;

};

#endif // !defined(AFX_GTSERIALPORT_H__D7E5B080_6BD6_44E6_BB89_4FEE2CC5C5C7__INCLUDED_)
