// gtComPort.cpp: implementation of the CgtComPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gtComPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for Queue
//////////////////////////////////////////////////////////////////////

CgtQueue::CgtQueue()
{
	Clear();
}

CgtQueue::~CgtQueue()
{

}

void CgtQueue::Clear()
{
	m_iHead = m_iTail = 0;
	memset(m_aBuffer, 0, QUEUE_SIZE);
}

int CgtQueue::GetSize()
{
	// ȯ�� Queue�� �����ϱ� ���� '%' �� ����Ѵ�.
	int size = (m_iHead - m_iTail + QUEUE_SIZE) % QUEUE_SIZE;

	return size;
}

BOOL CgtQueue::PutByte(BYTE b)
{
	// Check up that buffer is full.
	if ( TRUE == IsFull() )		
	{
TRACE("Queue is full.");
		return FALSE;
	}

	// Queue �� FIFO ������ �տ� byte �� �߰��ϰ�
	// ������ ���� ���� �߰��� ���� ���� byte �� �����Ѵ�.
	m_aBuffer[m_iHead++] = b;

	// ȯ�� Queue�� �����ϱ� ���� '%' �� ����Ѵ�.
	m_iHead = m_iHead % QUEUE_SIZE;

	return TRUE;
}

BOOL CgtQueue::GetByte(BYTE& b)
{
	// Check up that buffer is empty.
	if ( TRUE == IsEmpty() )
	{
TRACE("Queue is empty.");
		return FALSE;
	}

	// Queue �� FIFO ������ �տ� byte �� �߰��ϰ�
	// ������ ���� ���� �߰��� ���� ���� byte �� �����Ѵ�.
	b = m_aBuffer[m_iTail++];

	// ȯ�� Queue�� �����ϱ� ���� '%' �� ����Ѵ�.
	m_iTail = m_iTail % QUEUE_SIZE;

	return TRUE;
}

BOOL CgtQueue::IsFull()
{
	if(GetSize() == (QUEUE_SIZE-1))
		return TRUE;
	else
		return FALSE;
}

BOOL CgtQueue::IsEmpty()
{
	if(GetSize() == 0)
		return TRUE;
	else
		return FALSE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction for Serial Port
//////////////////////////////////////////////////////////////////////

CgtComPort::CgtComPort()
{
	m_hComPort = NULL;
	m_pParent = NULL;	
	m_hThreadWatchComm = NULL;
	m_hShutdownEvent = NULL;
	m_hWriteEvent = NULL;

	ZeroMemory(&m_ov, sizeof(OVERLAPPED));
	
	m_bThreadAlive = FALSE;
	m_nPort = 0;		// close���� 0�� Ȯ���ؼ� Open �õ� ���θ� �˻��Ѵ�.

	m_bOpened = FALSE;

	m_nDelayTime = 0;	// byte ���ۻ����� ������ ���� ����

	m_nSendingMode = COMM_WITH_PC;

#ifdef _FILE_OUTPUT
	m_fileMessage.open( "comm.err" );
#endif

}

// ��ü �Ҹ�� �ڵ����� Port �� �ݴ´�.
CgtComPort::~CgtComPort()
{
	closePort();

#ifdef _FILE_OUTPUT
	m_fileMessage.close();
#endif
}


BOOL CgtComPort::openPort(	int nPort,
							CWnd* pParentWnd /* = NULL */,
							DWORD baudrate /* =CBR_9600 */, 
							BYTE bytesize /* =8 */, 
							BYTE parity /* =NOPARITY */, 
							BYTE stopbits /* =ONESTOPBIT */ )
{
	COMMTIMEOUTS	timeouts;
	DCB				dcb;
	DWORD			dwThreadID;
	char			szPort[20];

	BOOL			bResult;

	// If the port open already, 
	// the port is opened again before closing.
	if( TRUE == m_bOpened )
	{
		writeMessage("Reopen a COM Port in openPort()");
		closePort();
	}

	if( nPort < 1)
	{
		writeMessage("COM port number is not correct in openPort()");
		return FALSE;
	}

	if( pParentWnd != NULL )
	{
		if( TRUE != pParentWnd->IsKindOf( RUNTIME_CLASS( CWnd ) ) )
		{
			writeMessage("Parent class must be a CWnd class in openPort()");
			return FALSE;
		}
	}


	InitializeCriticalSection( &m_csCommSync );

	// The OVERLAPPED structure contaions informatiln 
	// used in asynchronous input and output.
	ZeroMemory(&m_ov, sizeof(OVERLAPPED));

	if(m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (	NULL == m_ov.hEvent || NULL == m_hShutdownEvent ||
			NULL == m_hWriteEvent )
	{
		closePort();
		
		writeMessage("Can't create a event handle in openPort()");
		return FALSE;
	}

	// initializethe event objects.
	m_hEventArray[0] = m_hShutdownEvent;
	m_hEventArray[1] = m_ov.hEvent; 
	m_hEventArray[2] = m_hWriteEvent;

	m_pParent = pParentWnd;
	m_nPort = nPort;

	if ( m_nPort < 10 )
		// 1 -> COM1, 2 -> COM2, ...
		wsprintf( szPort, "COM%d", m_nPort);
	else
		// 10 �̻��� Port ��ȣ�� �ٸ��� ó���Ѵ�.
		wsprintf( szPort, "\\\\.\\COM%d", m_nPort);


	// Start a Critical Section
	EnterCriticalSection( &m_csCommSync );

	m_hComPort = CreateFile(	szPort, 
								GENERIC_READ | GENERIC_WRITE, 
								0, 
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
								NULL );

	if ( m_hComPort == INVALID_HANDLE_VALUE )
	{
		LeaveCriticalSection( &m_csCommSync );

		closePort();

		writeMessage("Can't create a com port handle in openPort()");
		return FALSE;
	}


	//-----------------------------------------
	// ��Ʈ ���� ����
	//-----------------------------------------
	
	// timeout ����
	timeouts.ReadIntervalTimeout = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 1000;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;
	timeouts.WriteTotalTimeoutConstant = 1000;
	bResult = SetCommTimeouts( m_hComPort, &timeouts );

	if( bResult == FALSE )
		writeMessage("SetCommTimeouts() error in openPort()");


	// EV_RXCHAR Event���� ����Ѵ�.
	// ( EV_BREAK, EV_CTS, EV_ERR, EV_RING, EV_RLSD, EV_RXFLAG, EV_TXEMPTY )
	bResult = SetCommMask( m_hComPort, EV_RXCHAR );

	if( bResult == FALSE )
		writeMessage("SetCommMask() error in openPort()");

	// InQueue, OutQueue ũ�⼳��
	bResult = SetupComm( m_hComPort, 4096, 4096 );

	if( bResult == FALSE )
		writeMessage("SetupComm() error in openPort()");

	// ��Ʈ ������ �о��.
	dcb.DCBlength = sizeof(DCB);
	bResult = GetCommState(m_hComPort, &dcb);

	if( bResult == FALSE )
		writeMessage("GetCommState() error in openPort()");

	// ��Ʈ�� ������ �����Ѵ�.
	dcb.BaudRate = baudrate;
//	dcb.fBinary = 1;
	dcb.ByteSize = bytesize;
	dcb.Parity = parity;
	dcb.StopBits = stopbits;
//	dcb.fOutxCtsFlow = 0;
//	dcb.fOutxDsrFlow = 0;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fRtsControl = RTS_CONTROL_ENABLE;
//	dcb.fInX = TRUE;
//	dcb.fOutX = TRUE;			// Xon, Xoff ���
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	bResult = SetCommState(m_hComPort, &dcb);
	
	if( bResult == FALSE )
		writeMessage("SetCommState() error in openPort()");

	// comport buffer �� ����.
	PurgeComm(	m_hComPort, 
				PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);


	// Stop the Critecal Section;
	LeaveCriticalSection( &m_csCommSync );


	// Parent �� �ʱ�ȭ �Ǿ������� �����尡 �����Ѵ�.
	if( m_pParent != NULL )
	{
		// COM Port�� Open�� ���ÿ� Thread �� �����ϰ� ���۽�Ų��.
		m_hThreadWatchComm = CreateThread(	
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)CommThread,
							this,
							// If this value is zero, 
							// the thread runs immediately after creation
							0,		
							&dwThreadID );

		if ( NULL == m_hThreadWatchComm )
		{
			closePort();

			writeMessage("Can't create a Comm thread in openPort()");
			return FALSE;
		}
	}
	
	m_bOpened = TRUE;		// ������ ��Ʈ�� ������ ���Ѵ�.

	char szTemp[30];
	sprintf(szTemp, "Opened the COM %d", nPort);
	writeMessage(szTemp);

	return TRUE;
}


// closePort() �Լ� �ǹ̴� 
// �����ִ� ��Ʈ�� �ݰ� ��ü �������� ���·� �����°��� ���Ѵ�.

BOOL CgtComPort::closePort()
{
	// openPort �Լ������� �����Ű�� ���� ���
	if( 0 == m_nPort ) 	return FALSE;

	// ���������� �����ϴ� ��Ʈ ��쿡�� ���۸� �ʱ�ȭ �Ѵ�.
	if( TRUE == m_bOpened )	 clearPort();

	// m_bOpened (��Ʈ ��������) �� ������� 
	// ���� ���� �׶��� ���¸� ���� �ʱ�ȭ �ϵ��� �Ͽ���.
	if( m_hThreadWatchComm != NULL )
	{
		do
		{
			SetEvent( m_hShutdownEvent );

			// �����ȿ��� ���������� �̺�Ʈ�� �۵���Ű�� ��쿡
			// �̺�Ʈ�� �������� �ʴ°��� �ذ��Ѵ�.
			//
			// ���� : SetEvent() �Լ� ȣ���Ŀ� �� delay�� �־���Ѵ�.
			Sleep(10);

		} while ( m_bThreadAlive );

		m_hThreadWatchComm = NULL;
	}

	if ( m_ov.hEvent != NULL )	
	{
		CloseHandle( m_ov.hEvent );
		ZeroMemory(&m_ov, sizeof(OVERLAPPED));
	}

	if ( m_hShutdownEvent != NULL )
	{
		CloseHandle( m_hShutdownEvent );
		m_hShutdownEvent = NULL;
	}

	if ( m_hWriteEvent != NULL )	
	{
		CloseHandle( m_hWriteEvent );
		m_hWriteEvent = NULL;
	}

	if ( m_hComPort != NULL )
	{
		CloseHandle( m_hComPort );
		m_hComPort = NULL;
	}

	m_pParent = NULL;	
	m_bThreadAlive = FALSE;

	m_nPort = 0;
	m_bOpened = FALSE;

	m_nDelayTime = 0;	// byte ���ۻ����� ������ ���� ����
	m_nSendingMode = COMM_WITH_PC;


	char szTemp[30];
	sprintf(szTemp, "Closed the COM %d", m_nPort);
	writeMessage(szTemp);

	DeleteCriticalSection( &m_csCommSync );

	return TRUE;
}

BOOL CgtComPort::sendByte(BYTE byte)
{
	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in sendByte()");
		return FALSE;
	}

//	EnterCriticalSection( &m_csCommSync );

	if(m_queueSend.IsFull() == TRUE)
	{
		writeMessage("Be full the data queue in sendByte()");
		return 0;
	}


	// ���⿡�� ���� ���ۿ��� ����� �������� �ʿ��ϴ�.
//	m_queueSend.Clear();

	m_queueSend.PutByte( byte );

//	LeaveCriticalSection( &m_csCommSync );


	// byte ���ۿ��� ������ �ʿ����.
	// ���� �����Լ� ȣ�⹮���� �Ʒ��� Sleep�� �ذ��ϰ� �ִ�.
	m_nDelayTime = 0;

	SetEvent(m_hWriteEvent);

	// sendByte() �Լ��� �����ȿ��� ���������� ȣ���ϴ� ���
	// �̺�Ʈ�� ���������� �߻��ؼ� �������� �ʴ°��� �ذ��Ѵ�.
	//
	// ���� : SetEvent() �Լ� ȣ���Ŀ� �� delay�� �־���Ѵ�.
	Sleep(1);

	
	return TRUE;
}


BOOL CgtComPort::sendBytes(	BYTE* pBuff, 
							int nByteSize,
							int nDelayTime /* =0 */)
{
	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in sendBytes()");
		return FALSE;
	}

//	EnterCriticalSection( &m_csCommSync );

	if(m_queueSend.IsFull() == TRUE)
	{
		writeMessage("Be full the data queue in sendBytes()");
		return 0;
	}
	// ���⿡�� ���� ���ۿ��� ����� �������� �ʿ��ϴ�.
//	m_queueSend.Clear();

	for(int i=0; i<nByteSize; i++)
		m_queueSend.PutByte( pBuff[i] );
	
//	LeaveCriticalSection( &m_csCommSync );


	// byte ���۰��� �ð������� �����ϰ� �ִ�.
	// �����Լ� ȣ�⹮���� �Ʒ��� Sleep�� �ذ��ϰ� �ִ�.
	m_nDelayTime = nDelayTime;


	SetEvent(m_hWriteEvent);

	// sendBytes() �Լ��� �����ȿ��� ���������� ȣ���ϴ� ���
	// �̺�Ʈ�� ���������� �߻��ؼ� �������� �ʴ°��� �ذ��Ѵ�.
	//
	// ���� : SetEvent() �Լ� ȣ���Ŀ� �� delay�� �־���Ѵ�.
	//		  �ּ� 1ms���� �����ϳ� ������ ���� 2ms�� �־���.
	Sleep(2);
	
	return TRUE;
}

BOOL CgtComPort::sendString(CString& strSend,
							int nDelayTime /* =0 */)
{
	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in sendString()");
		return FALSE;
	}

//	EnterCriticalSection( &m_csCommSync );
	if(m_queueSend.IsFull() == TRUE)
	{
		writeMessage("Be full the data queue in sendString()");
		return 0;
	}

	// ���⿡�� ���� ���ۿ��� ����� �������� �ʿ��ϴ�.
//	m_queueSend.Clear();

	int nByteSize = strSend.GetLength();

	for(int i=0; i<nByteSize; i++)
		m_queueSend.PutByte( (BYTE)strSend.GetAt(i) );

//	LeaveCriticalSection( &m_csCommSync );

	// byte ���۰��� �ð������� �����ϰ� �ִ�.
	// �����Լ� ȣ�⹮���� �Ʒ��� Sleep�� �ذ��ϰ� �ִ�.
	m_nDelayTime = nDelayTime;


	SetEvent(m_hWriteEvent);

	// sendString() �Լ��� �����ȿ��� ���������� ȣ���ϴ� ���
	// �̺�Ʈ�� ���������� �߻��ؼ� �������� �ʴ°��� �ذ��Ѵ�.
	//
	// ���� : SetEvent() �Լ� ȣ���Ŀ� �� delay�� �־���Ѵ�.
	//		  �ּ� 1ms���� �����ϳ� ������ ���� 2ms�� �־���.
	Sleep(2);

	return TRUE;
}

BOOL CgtComPort::receiveByte(BYTE &byte)
{

	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in receiveBytes()");
		return FALSE;
	}

//	EnterCriticalSection( &m_csCommSync );

	int queueSize = m_queueReceive.GetSize();

	if( m_queueReceive.IsEmpty() == TRUE )
	{
		writeMessage("Be empty the data queue in receiveByte()");
		return FALSE;
	}

	
	m_queueReceive.GetByte(byte);

//	LeaveCriticalSection( &m_csCommSync );

	return TRUE;
}

DWORD CgtComPort::receiveBytes(BYTE* pBuff, int nByteSize)
{
	BYTE bt;

	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in receiveBytes()");
		return NOT_OPENED_ERROR;
	}

//	EnterCriticalSection( &m_csCommSync );

	int queueSize = m_queueReceive.GetSize();

	if( m_queueReceive.IsEmpty() == TRUE )
	{
		writeMessage("Be empty the data queue in receiveBytes()");
		return FALSE;
	}


	// ���ŵ� ������ ����Ʈ ���� �䱸 ����Ʈ������ ������ 
	// ����Ʈ���� �����Ѵ�.
	if( nByteSize < queueSize ) 	nByteSize = queueSize;

	// Queue ���� �ϳ��� byte�� �о��.
	for( int j=0; j<nByteSize; j++)
	{
		m_queueReceive.GetByte(bt);
		pBuff[j] = bt;
	}

//	LeaveCriticalSection( &m_csCommSync );


	// ���� buffer���� �о byte ������ �����Ѵ�.
	return nByteSize;
}

DWORD CgtComPort::receiveString(CString& strReceive)
{
	BYTE bt;
	
	if( FALSE == m_bOpened  )
	{
		writeMessage("A Com Port didn't open in receiveString()");
		return NOT_OPENED_ERROR;
	}

//	EnterCriticalSection( &m_csCommSync );

	int queueSize = m_queueReceive.GetSize();
	
	if( m_queueReceive.IsEmpty() == TRUE )
	{
		writeMessage("Be empty the data queue in receiveString()");
		return FALSE;
	}


	// Queue ���� �ϳ��� ��Ŷ�� �о��.
	for( int j=0; j<queueSize; j++)
	{
		m_queueReceive.GetByte(bt);
		strReceive += (TCHAR)bt;
	}
	
//	LeaveCriticalSection( &m_csCommSync );

	return queueSize;
}

DWORD CgtComPort::CommThread(CgtComPort* port)
{

//--------------------------------------------------------------
// ���� �߿� 
//--------------------------------------------------------------
//
//	1. CommThread ���� ���Ǵ� �������� CgtComPort �� protected ������
//	  �����ϱ� ���� CommThread �� CgtComPort �� ����Լ��� �����ߴ�. 
//
//	2. CommThread �Լ��� CallBack �Լ��� ����ϱ� ���� static ���� �����ߴ�.
//
//  3. static �Լ��̱� ������ static ������ ���ٰ����ϴ�.
//	  �̸� �ذ��ϱ� ���� pParam �� �޾� CgtComPort ��ü�� �����ͷ�
//	  ����ȯ�� �ؼ� �����͸� �̿��� ������ �����ؾ��Ѵ�.
//	  ( ex. pPort->m_hComPort )

//	DWORD		dwRead;			// ���� ����Ʈ��
	DWORD		dwError;
	DWORD		dwEvent = 0;
	BOOL		bResult;
	COMSTAT		comstat;


	// ������ ����Ѵ�.
	port->m_bThreadAlive = TRUE;

	// Port �� ��� ����.
	if( NULL != port->m_hComPort)
	{
		PurgeComm(	port->m_hComPort, 
					PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	}

	
//	while( port->m_bThreadAlive )
	while( TRUE )
	{

		// ��Ʈ�� �����Ͱ� ���ö����� ��ٸ���.
		bResult = WaitCommEvent( port->m_hComPort, &dwEvent, &port->m_ov);

		if( TRUE == bResult )
		{
			bResult = ClearCommError(port->m_hComPort, &dwError, &comstat);

			// if there are no bytes to read at the port
			// Do nothing and continue.
			if (comstat.cbInQue == 0)
				continue;
		}
		else
		{
			dwError = GetLastError();

			switch (dwError) 
			{ 
			case ERROR_IO_PENDING:	
 					// This is a normal return value if there are no bytes
					// to read at the port.
					// Do nothing and continue
					break;

			case 87:
					break;

			default:
					// All other error codes indicate a serious error has
					// occured.  Process this error.
					port->writeMessage("WaitCommEvent() error in CommThread()");
					break;
			}
		}

		// Main wait function.	This function will normally block the thread
		// until one of nine events occur that require action.
		dwEvent = WaitForMultipleObjects(3, port->m_hEventArray, FALSE, INFINITE);


		switch (dwEvent)
		{

		case 0:

			// Shutdown event.  This is event zero so it will be
			// the higest priority and be serviced first.

			port->m_bThreadAlive = FALSE;

			// Kill this thread.  break is not needed, but makes me feel better.
			AfxEndThread(100);

			break;

		case 1: // wait comm event

			DWORD dwCommEvent;

			GetCommMask(port->m_hComPort, &dwCommEvent);

			if( dwCommEvent & EV_RXCHAR )
				port->readBytes(port, comstat);

			break;

		case 2:

			writeBytes(port);

			break;
		}
	}

	port->m_hThreadWatchComm = NULL;

	return TRUE;
}

void CgtComPort::writeMessage(char *errorText)
{

#ifdef _FILE_OUTPUT

	char szPort[20];
	wsprintf(szPort, " at COM%d.", m_nPort);
	m_fileMessage << errorText << szPort << endl;

#else

	TRACE("\n%s\n",errorText);

#endif 


}

void CgtComPort::readBytes(CgtComPort* port, COMSTAT comstat)
{
	DWORD	dwError, dwErrorFlags;
	BYTE	btBuff;
	DWORD	dwRead;
	DWORD	nInBufferSize;
	BOOL	bResult;
	BOOL	bRead = FALSE;
	
	if( FALSE == port->m_bOpened  )
	{
		port->writeMessage("A Com Port didn't open in readBytes()");
		return;
	}


	EnterCriticalSection( &port->m_csCommSync );

	do
	{
		bResult = ClearCommError(port->m_hComPort, &dwError, &comstat);

		nInBufferSize = comstat.cbInQue;

		// ���Ź��۰� ��������� �ٷ� ������ ����������.
		if( nInBufferSize == 0 )	break;

		
		// ���� buffer ���� �� ����Ʈ�� �о��.
		bResult = ReadFile(	port->m_hComPort, 
							&btBuff, 
							1, 
							&dwRead, 
							&port->m_ov );


		if( FALSE == bResult )
		{
			//
			//	< ��Ʈ��ũ ���α׷��� p121 ���� >
			//			
			switch( dwError = GetLastError() )
			{
			case ERROR_IO_PENDING:
			
				port->writeMessage( "Warning io pending in readBytes()" );

				// ���� ���ڰ� �����ְų� ������ ���ڰ� ���� ���� ���
				// Overapped IO�� Ư���� ���� ERROR_IO_PENDING ERROR �޼�����
				// ���޵ȴ�.
				//
				// ��ø�� �۾��� ������ �� ���Ῡ�θ� �� �� �ִ�.
				while( FALSE == GetOverlappedResult(port->m_hComPort, 
													&port->m_ov, 
													&dwRead, 
													TRUE) )	// TRUE -> �۾�����ñ��� blocking �Ѵ�.
				{
					dwError = GetLastError();
					// ���Ḧ �˻��Ѵ�.
					if( dwError != ERROR_IO_INCOMPLETE )
					{
						ClearCommError(port->m_hComPort, &dwErrorFlags, &comstat);
						break;
					}
				}

				break;

			default:
				port->writeMessage("Reading error in readBytes()");
				break;
			}
		}
		else
		{
			if( QUEUE_SIZE - port->m_queueReceive.GetSize() > 1 )
			{
				port->m_queueReceive.PutByte( btBuff );
				bRead = TRUE;
			}
			else
			{
				LeaveCriticalSection( &port->m_csCommSync );
				port->writeMessage("The Queue is full in readBytes()");
				return;
			}
		}

	} while( TRUE );


	// ���� �߿� : 
	//		�޼����� ������ ���� critical section�� Ǯ���־�� �Ѵ�.
	LeaveCriticalSection( &port->m_csCommSync );


	// �����Ͱ� ���ŵǾ��ٴ� �޼����� �߻��Ѵ�.
	if( bRead == TRUE )
	{
//TRACE("\nRead Byte at COM %d.", port->m_nPort);

		// ���� �߿� :
		//    SendMessage()�� ȣ���ϸ� �޼����� �޴� ��ƾ����
		//    ������ ���ƿ� ������ ��ٸ��� �ȴ�.
		//
		//    �����ȣ�� �޾����� ���� Event�� �߻���Ű�� Looping
		//    �ڵ忡�� CommThread()�� �����ؼ� m_bThreadAlive��
		//    �ٲ�� �ٶ�µ� �� �Լ����� ������ ���� ���ϱ� ������
		//    CommThread()�� ������ ���� ���� ������ �ɷ�������.
		if( port->m_pParent != NULL )
		{
//			port->m_pParent->SendMessage(	WM_COMM_NOTIFY,
			port->m_pParent->PostMessage(	WM_COMM_NOTIFY,
									(WPARAM) COMM_NOTIFY_RXCHAR,
									(LPARAM) port->m_nPort );	

		}
	}		
}


DWORD CgtComPort::writeBytes(CgtComPort* port)
{
	BYTE*	pbtBuff = NULL;
	BYTE	btBuff;
	DWORD	dwWritten, dwError, dwErrorFlags;
	COMSTAT comstat;
	BOOL	bResult;

	ResetEvent( port->m_hWriteEvent );

	if( FALSE == port->m_bOpened  )
	{
		port->writeMessage("A Com Port didn't open in writeBytes()");
		return NOT_OPENED_ERROR;
	}

	EnterCriticalSection( &port->m_csCommSync );

	int nByteSize = port->m_queueSend.GetSize();
	

	if( nByteSize < 1 )
	{
		LeaveCriticalSection( &port->m_csCommSync );

		port->writeMessage("The Write Bytes don't exist in writeBytes()");
		return 0;
	}

	// comport buffer �� ����.
	PurgeComm(	port->m_hComPort, 
				PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);


//TRACE("\nWrite %d Byte at COM %d",nByteSize, port->m_nPort);


	if( port->m_nSendingMode == COMM_WITH_PC )
	{
		pbtBuff = new BYTE[nByteSize];


		for(int i=0; i<nByteSize; i++)
			port->m_queueSend.GetByte(pbtBuff[i]);


		bResult = WriteFile(	port->m_hComPort,
								(LPSTR) pbtBuff,
								nByteSize,
								&dwWritten,
								&port->m_ov );

		if( FALSE == bResult )
		{
			//
			//	< ��Ʈ��ũ ���α׷��� p121 ���� >
			//			
			switch( dwError = GetLastError() )
			{
			case ERROR_IO_PENDING:

//				port->writeMessage( "Warning io pending in writeBytes()" );

				// ���� ���ڰ� �����ְų� ������ ���ڰ� ���� ���� ���
				// Overapped IO�� Ư���� ���� ERROR_IO_PENDING ERROR �޼�����
				// ���޵ȴ�.
				//
				// ��ø�� �۾��� ������ �� ���Ῡ�θ� �� �� �ִ�.
				while( FALSE == GetOverlappedResult(port->m_hComPort, 
													&port->m_ov, 
													&dwWritten, 
													TRUE) )	// TRUE -> �۾�����ñ��� blocking �Ѵ�.
				{
					dwError = GetLastError();
					// ���Ḧ �˻��Ѵ�.
					if( dwError != ERROR_IO_INCOMPLETE )
					{
						ClearCommError(port->m_hComPort, &dwErrorFlags, &comstat);
						break;
					}
				}

				break;

			default:
				port->writeMessage("Writing error in writeBytes()");
				break;
			}

		}

		delete [] pbtBuff;

	}
	else if( port->m_nSendingMode == COMM_WITH_MICOM )
	{

		for(int i=0; i<nByteSize; i++)
		{
			port->m_queueSend.GetByte(btBuff);

			// �� ����Ʈ�� �����Ѵ�.
			bResult = WriteFile(	port->m_hComPort,
									(LPSTR) &btBuff,
									1,
									&dwWritten,
									&port->m_ov );

			if( FALSE == bResult )
			{
				//
				//	< ��Ʈ��ũ ���α׷��� p121 ���� >
				//			
				switch( dwError = GetLastError() )
				{
				case ERROR_IO_PENDING:

	//				port->writeMessage( "Warning io pending in writeBytes()" );

					// ���� ���ڰ� �����ְų� ������ ���ڰ� ���� ���� ���
					// Overapped IO�� Ư���� ���� ERROR_IO_PENDING ERROR �޼�����
					// ���޵ȴ�.
					//
					// ��ø�� �۾��� ������ �� ���Ῡ�θ� �� �� �ִ�.
					while( FALSE == GetOverlappedResult(port->m_hComPort, 
														&port->m_ov, 
														&dwWritten, 
														TRUE) )	// TRUE -> �۾�����ñ��� blocking �Ѵ�.
					{
						dwError = GetLastError();
						// ���Ḧ �˻��Ѵ�.
						if( dwError != ERROR_IO_INCOMPLETE )
						{
							ClearCommError(port->m_hComPort, &dwErrorFlags, &comstat);
							break;
						}
					}

					break;

				default:
					port->writeMessage("Writing error in writeBytes()");
					break;
				}
			}

			// �� ����Ʈ ������ �����ð��� �ο��Ѵ�.
			Sleep(port->m_nDelayTime);
		}
	}
	else
	{
		// ��� ������ ������ �߻��� ����̴�.
		LeaveCriticalSection( &port->m_csCommSync );
		return -1;
	}

	LeaveCriticalSection( &port->m_csCommSync );
	
	// ���� ���۵� ����Ʈ ���� �����Ѵ�.
	return dwWritten;
}



BOOL CgtComPort::clearPort()
{

	if( TRUE == m_bOpened )
	{
		// queue �� ����.
		m_queueReceive.Clear();
		m_queueSend.Clear();
		
		EnterCriticalSection( &m_csCommSync );

		// comport buffer �� ����.
		PurgeComm(	m_hComPort, 
					PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		LeaveCriticalSection( &m_csCommSync );

		return TRUE;
	}
	else
		return FALSE;
}


BOOL CgtComPort::checkPort(int nPort)
{
	if( nPort < 1 )		return FALSE;

	char		szPort[20];
	HANDLE		hComPort;	// ��� ��Ʈ ���� �ڵ�
	

	if ( nPort < 10 )
		// 1 -> COM1, 2 -> COM2, ...
		wsprintf( szPort, "COM%d", nPort);
	else
		// 10 �̻��� Port ��ȣ�� �ٸ��� ó���Ѵ�.
		wsprintf( szPort, "\\\\.\\COM%d", nPort);


	// ��Ʈ ����
	hComPort = CreateFile(	szPort, 
							GENERIC_READ | GENERIC_WRITE, 
							0, 
							NULL,
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 
							NULL );

	Sleep(1);

	if ( hComPort == INVALID_HANDLE_VALUE )
	{
		// ��Ʈ OPEN �Ұ���
		return FALSE;   
	}
	else    
	{	
		// ��Ʈ OPEN ����
		CloseHandle( hComPort );
		hComPort = NULL;
		return TRUE;    
	}
}

int CgtComPort::getReceivedByteSize()
{
	int queueSize;

	EnterCriticalSection( &m_csCommSync );

	queueSize = m_queueReceive.GetSize();

	LeaveCriticalSection( &m_csCommSync );

	return queueSize;
}

BOOL CgtComPort::openPort(int nPort, LPDCB lpDCB, CWnd *pParentWnd)
{
	COMMTIMEOUTS	timeouts;
//	DCB				dcb;
	DWORD			dwThreadID;
	char			szPort[20];

	BOOL			bResult;

	// If the port open already, 
	// the port is opened again before closing.
	if( TRUE == m_bOpened )
	{
		writeMessage("Reopen a COM Port in openPort()");
		closePort();
	}

	if( nPort < 1)
	{
		writeMessage("COM port number is not correct in openPort()");
		return FALSE;
	}

	if( pParentWnd != NULL )
	{
		if( TRUE != pParentWnd->IsKindOf( RUNTIME_CLASS( CWnd ) ) )
		{
			writeMessage("The parent class must be a CWnd class in openPort()");
			return FALSE;
		}
	}

	InitializeCriticalSection( &m_csCommSync );

	// The OVERLAPPED structure contaions informatiln 
	// used in asynchronous input and output.
	ZeroMemory(&m_ov, sizeof(OVERLAPPED));

	if(m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (	NULL == m_ov.hEvent || NULL == m_hShutdownEvent ||
			NULL == m_hWriteEvent )
	{
		closePort();		

		writeMessage("Can't create a event handle in openPort()");
		return FALSE;
	}

	// initializethe event objects.
	m_hEventArray[0] = m_hShutdownEvent;
	m_hEventArray[1] = m_ov.hEvent; 
	m_hEventArray[2] = m_hWriteEvent;

	m_pParent = pParentWnd;
	m_nPort = nPort;

	if ( m_nPort < 10 )
		// 1 -> COM1, 2 -> COM2, ...
		wsprintf( szPort, "COM%d", m_nPort);
	else
		// 10 �̻��� Port ��ȣ�� �ٸ��� ó���Ѵ�.
		wsprintf( szPort, "\\\\.\\COM%d", m_nPort);

	// Start a Critical Section
	EnterCriticalSection( &m_csCommSync );

	m_hComPort = CreateFile(	szPort, 
								GENERIC_READ | GENERIC_WRITE, 
								0, 
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
								NULL );

	if ( m_hComPort == INVALID_HANDLE_VALUE )
	{
		LeaveCriticalSection( &m_csCommSync );

		closePort();

		writeMessage("Can't create a com port handle in openPort()");
		return FALSE;
	}


	//-----------------------------------------
	// ��Ʈ ���� ����
	//-----------------------------------------
	
	// timeout ����
	timeouts.ReadIntervalTimeout = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 1000;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;
	timeouts.WriteTotalTimeoutConstant = 1000;
	bResult = SetCommTimeouts( m_hComPort, &timeouts );

	if( bResult == FALSE )
		writeMessage("SetCommTimeouts() error in openPort()");


	// EV_RXCHAR Event���� ����Ѵ�.
	// ( EV_BREAK, EV_CTS, EV_ERR, EV_RING, EV_RLSD, EV_RXFLAG, EV_TXEMPTY )
	bResult = SetCommMask( m_hComPort, EV_RXCHAR );

	if( bResult == FALSE )
		writeMessage("SetCommMask() error in openPort()");

	// InQueue, OutQueue ũ�⼳��
	bResult = SetupComm( m_hComPort, 4096, 4096 );

	if( bResult == FALSE )
		writeMessage("SetupComm() error in openPort()");

	// ��Ʈ ������ �о��.
//	dcb.DCBlength = sizeof(DCB);
//	bResult = GetCommState(m_hComPort, &dcb);

//	if( bResult == FALSE )
//		writeMessage("GetCommState() error in openPort()");

	// ��Ʈ�� ������ �����Ѵ�.
//	dcb.BaudRate = baudrate;
//	dcb.fBinary = 1;
//	dcb.ByteSize = bytesize;
//	dcb.Parity = parity;
//	dcb.StopBits = stopbits;
//	dcb.fOutxCtsFlow = 0;
//	dcb.fOutxDsrFlow = 0;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fRtsControl = RTS_CONTROL_ENABLE;
//	dcb.fInX = TRUE;
//	dcb.fOutX = TRUE;			// Xon, Xoff ���
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	bResult = SetCommState(m_hComPort, lpDCB);
	
	if( bResult == FALSE )
		writeMessage("SetCommState() error in openPort()");

	// comport buffer �� ����.
	PurgeComm(	m_hComPort, 
				PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);


	// Stop the Critecal Section;
	LeaveCriticalSection( &m_csCommSync );


	// Parent �� �ʱ�ȭ �Ǿ������� �����尡 �����Ѵ�.
	if( m_pParent != NULL )
	{
		// COM Port�� Open�� ���ÿ� Thread �� �����ϰ� ���۽�Ų��.
		m_hThreadWatchComm = CreateThread(	
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)CommThread,
							this,
							// If this value is zero, 
							// the thread runs immediately after creation
							0,		
							&dwThreadID );

		if ( NULL == m_hThreadWatchComm )
		{
			closePort();

			writeMessage("Can't create a Comm thread in openPort()");
			return FALSE;
		}
	}
	
	m_bOpened = TRUE;		// ������ ��Ʈ�� ������ ���Ѵ�.

	char szTemp[30];
	sprintf(szTemp, "Opened the COM %d", nPort);
	writeMessage(szTemp);


	return TRUE;
}

BOOL CgtComPort::getPortState(int nPort, LPDCB lpDCB)
{
	char	szPort[20];
	HANDLE	hComPort;
	BOOL	bResult;

	if( nPort < 1)
	{
		writeMessage("COM port number is not correct in getPortState()");
		return FALSE;
	}

	// 1 -> COM1, 2 -> COM2, ...
	wsprintf( szPort, "COM%d", nPort);

	hComPort = CreateFile(	szPort, 
							GENERIC_READ | GENERIC_WRITE, 
							0, 
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							NULL );

	if ( hComPort == INVALID_HANDLE_VALUE )
	{
		writeMessage("Can't create a com port handle in getPortState()");
		CloseHandle( hComPort );
		return FALSE;
	}


	// ��Ʈ ������ �о��.
	lpDCB->DCBlength = sizeof(DCB);
	bResult = GetCommState(hComPort, lpDCB);

	if( bResult == FALSE )
	{
		writeMessage("GetCommState() error in getPortState()");
		CloseHandle( hComPort );
		return FALSE;
	}

	CloseHandle( hComPort );
	return TRUE;
}

BOOL CgtComPort::setSendingMode(int nMode)
{
	m_nSendingMode = nMode;

	return TRUE;
}

BOOL CgtComPort::changePortState(	DWORD baudrate /* =CBR_9600 */, 
									BYTE bytesize /* =8 */, 
									BYTE parity /* =NOPARITY */, 
									BYTE stopbits /* =ONESTOPBIT */ )
{
	DCB		dcb;
	BOOL	bResult;

	if( TRUE != m_bOpened )
	{
		writeMessage("The Port didn't open in changePortState()");
		return FALSE;
	}

	// Start a Critical Section
	EnterCriticalSection( &m_csCommSync );


	// ��Ʈ ������ �о��.
	dcb.DCBlength = sizeof(DCB);
	bResult = GetCommState(m_hComPort, &dcb);

	if( bResult == FALSE )
		writeMessage("GetCommState() error in changePortState()");

	// ��Ʈ�� ������ �����Ѵ�.
	dcb.BaudRate = baudrate;
//	dcb.fBinary = 1;
	dcb.ByteSize = bytesize;
	dcb.Parity = parity;
	dcb.StopBits = stopbits;
//	dcb.fOutxCtsFlow = 0;
//	dcb.fOutxDsrFlow = 0;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fRtsControl = RTS_CONTROL_ENABLE;
//	dcb.fInX = TRUE;
//	dcb.fOutX = TRUE;			// Xon, Xoff ���
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	bResult = SetCommState(m_hComPort, &dcb);
	
	if( bResult == FALSE )
		writeMessage("SetCommState() error in changePortState()");


	// Stop the Critecal Section;
	LeaveCriticalSection( &m_csCommSync );

	clearPort();


	char szTemp[30];
	sprintf(szTemp, "Changed the COM %d", m_nPort);
	writeMessage(szTemp);

	return TRUE;
}

int CgtComPort::getWaitingByteSize()
{
	int queueSize;

	EnterCriticalSection( &m_csCommSync );

	queueSize = m_queueSend.GetSize();

	LeaveCriticalSection( &m_csCommSync );

	return queueSize;
}
