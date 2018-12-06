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
	// 환형 Queue를 구성하기 위해 '%' 를 사용한다.
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

	// Queue 는 FIFO 구조로 앞에 byte 를 추가하고
	// 삭제는 가장 먼저 추가된 가장 뒤의 byte 를 제거한다.
	m_aBuffer[m_iHead++] = b;

	// 환형 Queue를 구성하기 위해 '%' 를 사용한다.
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

	// Queue 는 FIFO 구조로 앞에 byte 를 추가하고
	// 삭제는 가장 먼저 추가된 가장 뒤의 byte 를 제거한다.
	b = m_aBuffer[m_iTail++];

	// 환형 Queue를 구성하기 위해 '%' 를 사용한다.
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
	m_nPort = 0;		// close에서 0을 확인해서 Open 시도 여부를 검사한다.

	m_bOpened = FALSE;

	m_nDelayTime = 0;	// byte 전송사이의 지연을 주지 않음

	m_nSendingMode = COMM_WITH_PC;

#ifdef _FILE_OUTPUT
	m_fileMessage.open( "comm.err" );
#endif

}

// 객체 소멸시 자동으로 Port 를 닫는다.
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
		// 10 이상의 Port 번호는 다르게 처리한다.
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
	// 포트 상태 설정
	//-----------------------------------------
	
	// timeout 설정
	timeouts.ReadIntervalTimeout = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 1000;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;
	timeouts.WriteTotalTimeoutConstant = 1000;
	bResult = SetCommTimeouts( m_hComPort, &timeouts );

	if( bResult == FALSE )
		writeMessage("SetCommTimeouts() error in openPort()");


	// EV_RXCHAR Event만을 사용한다.
	// ( EV_BREAK, EV_CTS, EV_ERR, EV_RING, EV_RLSD, EV_RXFLAG, EV_TXEMPTY )
	bResult = SetCommMask( m_hComPort, EV_RXCHAR );

	if( bResult == FALSE )
		writeMessage("SetCommMask() error in openPort()");

	// InQueue, OutQueue 크기설정
	bResult = SetupComm( m_hComPort, 4096, 4096 );

	if( bResult == FALSE )
		writeMessage("SetupComm() error in openPort()");

	// 포트 설정을 읽어낸다.
	dcb.DCBlength = sizeof(DCB);
	bResult = GetCommState(m_hComPort, &dcb);

	if( bResult == FALSE )
		writeMessage("GetCommState() error in openPort()");

	// 포트를 새로이 설정한다.
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
//	dcb.fOutX = TRUE;			// Xon, Xoff 사용
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	bResult = SetCommState(m_hComPort, &dcb);
	
	if( bResult == FALSE )
		writeMessage("SetCommState() error in openPort()");

	// comport buffer 를 비운다.
	PurgeComm(	m_hComPort, 
				PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);


	// Stop the Critecal Section;
	LeaveCriticalSection( &m_csCommSync );


	// Parent 가 초기화 되었을때만 쓰레드가 동작한다.
	if( m_pParent != NULL )
	{
		// COM Port의 Open과 동시에 Thread 를 생성하고 동작시킨다.
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
	
	m_bOpened = TRUE;		// 무사히 포트가 열림을 뜻한다.

	char szTemp[30];
	sprintf(szTemp, "Opened the COM %d", nPort);
	writeMessage(szTemp);

	return TRUE;
}


// closePort() 함수 의미는 
// 열려있는 포트를 닫고 객체 생성시의 상태로 돌리는것을 뜻한다.

BOOL CgtComPort::closePort()
{
	// openPort 함수조차도 실행시키지 않은 경우
	if( 0 == m_nPort ) 	return FALSE;

	// 정상적으로 동작하는 포트 경우에만 버퍼를 초기화 한다.
	if( TRUE == m_bOpened )	 clearPort();

	// m_bOpened (포트 완전열림) 에 상관없이 
	// 각각 마다 그때의 상태를 보고 초기화 하도록 하였다.
	if( m_hThreadWatchComm != NULL )
	{
		do
		{
			SetEvent( m_hShutdownEvent );

			// 루프안에서 연속적으로 이벤트가 작동시키는 경우에
			// 이벤트가 동작하지 않는것을 해결한다.
			//
			// 주의 : SetEvent() 함수 호출후에 꼭 delay를 주어야한다.
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

	m_nDelayTime = 0;	// byte 전송사이의 지연을 주지 않음
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


	// 여기에서 앞의 전송에서 사용한 데이터의 필요하다.
//	m_queueSend.Clear();

	m_queueSend.PutByte( byte );

//	LeaveCriticalSection( &m_csCommSync );


	// byte 전송에는 지연이 필요없다.
	// 또한 연속함수 호출문제는 아래의 Sleep가 해결하고 있다.
	m_nDelayTime = 0;

	SetEvent(m_hWriteEvent);

	// sendByte() 함수를 루프안에서 연속적으로 호출하는 경우
	// 이벤트가 연속적으로 발생해서 동작하지 않는것을 해결한다.
	//
	// 주의 : SetEvent() 함수 호출후에 꼭 delay를 주어야한다.
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
	// 여기에서 앞의 전송에서 사용한 데이터의 필요하다.
//	m_queueSend.Clear();

	for(int i=0; i<nByteSize; i++)
		m_queueSend.PutByte( pBuff[i] );
	
//	LeaveCriticalSection( &m_csCommSync );


	// byte 전송간의 시간지연을 지정하고 있다.
	// 연속함수 호출문제는 아래의 Sleep가 해결하고 있다.
	m_nDelayTime = nDelayTime;


	SetEvent(m_hWriteEvent);

	// sendBytes() 함수를 루프안에서 연속적으로 호출하는 경우
	// 이벤트가 연속적으로 발생해서 동작하지 않는것을 해결한다.
	//
	// 주의 : SetEvent() 함수 호출후에 꼭 delay를 주어야한다.
	//		  최소 1ms까지 가능하나 안전을 위해 2ms를 주었다.
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

	// 여기에서 앞의 전송에서 사용한 데이터의 필요하다.
//	m_queueSend.Clear();

	int nByteSize = strSend.GetLength();

	for(int i=0; i<nByteSize; i++)
		m_queueSend.PutByte( (BYTE)strSend.GetAt(i) );

//	LeaveCriticalSection( &m_csCommSync );

	// byte 전송간의 시간지연을 지정하고 있다.
	// 연속함수 호출문제는 아래의 Sleep가 해결하고 있다.
	m_nDelayTime = nDelayTime;


	SetEvent(m_hWriteEvent);

	// sendString() 함수를 루프안에서 연속적으로 호출하는 경우
	// 이벤트가 연속적으로 발생해서 동작하지 않는것을 해결한다.
	//
	// 주의 : SetEvent() 함수 호출후에 꼭 delay를 주어야한다.
	//		  최소 1ms까지 가능하나 안전을 위해 2ms를 주었다.
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


	// 수신된 버퍼의 바이트 수가 요구 바이트수보다 작으면 
	// 바이트수를 수정한다.
	if( nByteSize < queueSize ) 	nByteSize = queueSize;

	// Queue 에서 하나의 byte를 읽어낸다.
	for( int j=0; j<nByteSize; j++)
	{
		m_queueReceive.GetByte(bt);
		pBuff[j] = bt;
	}

//	LeaveCriticalSection( &m_csCommSync );


	// 실제 buffer에서 읽어낸 byte 갯수를 리턴한다.
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


	// Queue 에서 하나의 패킷을 읽어낸다.
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
// 아주 중요 
//--------------------------------------------------------------
//
//	1. CommThread 에서 사용되는 변수들을 CgtComPort 의 protected 변수로
//	  선언하기 위해 CommThread 를 CgtComPort 의 멤버함수로 선언했다. 
//
//	2. CommThread 함수를 CallBack 함수로 사용하기 위해 static 으로 선언했다.
//
//  3. static 함수이기 때문에 static 변수만 접근가능하다.
//	  이를 해결하기 위해 pParam 을 받아 CgtComPort 객체의 포인터로
//	  형변환을 해서 포인터를 이용해 변수에 접근해야한다.
//	  ( ex. pPort->m_hComPort )

//	DWORD		dwRead;			// 읽은 바이트수
	DWORD		dwError;
	DWORD		dwEvent = 0;
	BOOL		bResult;
	COMSTAT		comstat;


	// 동작을 허용한다.
	port->m_bThreadAlive = TRUE;

	// Port 를 모두 비운다.
	if( NULL != port->m_hComPort)
	{
		PurgeComm(	port->m_hComPort, 
					PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	}

	
//	while( port->m_bThreadAlive )
	while( TRUE )
	{

		// 포트에 데이터가 들어올때까지 기다린다.
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

		// 수신버퍼가 비어있으면 바로 루프를 빠져나간다.
		if( nInBufferSize == 0 )	break;

		
		// 수신 buffer 에서 한 바이트씩 읽어낸다.
		bResult = ReadFile(	port->m_hComPort, 
							&btBuff, 
							1, 
							&dwRead, 
							&port->m_ov );


		if( FALSE == bResult )
		{
			//
			//	< 네트워크 프로그래밍 p121 참고 >
			//			
			switch( dwError = GetLastError() )
			{
			case ERROR_IO_PENDING:
			
				port->writeMessage( "Warning io pending in readBytes()" );

				// 읽을 문자가 남아있거나 전송할 문자가 남아 있을 경우
				// Overapped IO의 특성에 따라 ERROR_IO_PENDING ERROR 메세지가
				// 전달된다.
				//
				// 증첩된 작업을 수행한 후 종료여부를 알 수 있다.
				while( FALSE == GetOverlappedResult(port->m_hComPort, 
													&port->m_ov, 
													&dwRead, 
													TRUE) )	// TRUE -> 작업종료시까지 blocking 한다.
				{
					dwError = GetLastError();
					// 종료를 검사한다.
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


	// 아주 중요 : 
	//		메세지를 날리기 전에 critical section을 풀어주어야 한다.
	LeaveCriticalSection( &port->m_csCommSync );


	// 데이터가 수신되었다는 메세지를 발생한다.
	if( bRead == TRUE )
	{
//TRACE("\nRead Byte at COM %d.", port->m_nPort);

		// 아주 중요 :
		//    SendMessage()를 호출하면 메세지를 받는 루틴에서
		//    리턴이 돌아올 때까지 기다리게 된다.
		//
		//    종료신호를 받았을때 종료 Event를 발생시키는 Looping
		//    코드에서 CommThread()가 동작해서 m_bThreadAlive가
		//    바뀌길 바라는데 이 함수에서 리턴이 되지 못하기 때문에
		//    CommThread()가 동작을 못해 무한 루프가 걸려버린다.
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

	// comport buffer 를 비운다.
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
			//	< 네트워크 프로그래밍 p121 참고 >
			//			
			switch( dwError = GetLastError() )
			{
			case ERROR_IO_PENDING:

//				port->writeMessage( "Warning io pending in writeBytes()" );

				// 읽을 문자가 남아있거나 전송할 문자가 남아 있을 경우
				// Overapped IO의 특성에 따라 ERROR_IO_PENDING ERROR 메세지가
				// 전달된다.
				//
				// 증첩된 작업을 수행한 후 종료여부를 알 수 있다.
				while( FALSE == GetOverlappedResult(port->m_hComPort, 
													&port->m_ov, 
													&dwWritten, 
													TRUE) )	// TRUE -> 작업종료시까지 blocking 한다.
				{
					dwError = GetLastError();
					// 종료를 검사한다.
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

			// 한 바이트를 전송한다.
			bResult = WriteFile(	port->m_hComPort,
									(LPSTR) &btBuff,
									1,
									&dwWritten,
									&port->m_ov );

			if( FALSE == bResult )
			{
				//
				//	< 네트워크 프로그래밍 p121 참고 >
				//			
				switch( dwError = GetLastError() )
				{
				case ERROR_IO_PENDING:

	//				port->writeMessage( "Warning io pending in writeBytes()" );

					// 읽을 문자가 남아있거나 전송할 문자가 남아 있을 경우
					// Overapped IO의 특성에 따라 ERROR_IO_PENDING ERROR 메세지가
					// 전달된다.
					//
					// 증첩된 작업을 수행한 후 종료여부를 알 수 있다.
					while( FALSE == GetOverlappedResult(port->m_hComPort, 
														&port->m_ov, 
														&dwWritten, 
														TRUE) )	// TRUE -> 작업종료시까지 blocking 한다.
					{
						dwError = GetLastError();
						// 종료를 검사한다.
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

			// 각 바이트 사이의 지연시간을 부여한다.
			Sleep(port->m_nDelayTime);
		}
	}
	else
	{
		// 모드 선택이 문제가 발생한 경우이다.
		LeaveCriticalSection( &port->m_csCommSync );
		return -1;
	}

	LeaveCriticalSection( &port->m_csCommSync );
	
	// 실제 전송된 바이트 수를 리턴한다.
	return dwWritten;
}



BOOL CgtComPort::clearPort()
{

	if( TRUE == m_bOpened )
	{
		// queue 를 비운다.
		m_queueReceive.Clear();
		m_queueSend.Clear();
		
		EnterCriticalSection( &m_csCommSync );

		// comport buffer 를 비운다.
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
	HANDLE		hComPort;	// 통신 포트 파일 핸들
	

	if ( nPort < 10 )
		// 1 -> COM1, 2 -> COM2, ...
		wsprintf( szPort, "COM%d", nPort);
	else
		// 10 이상의 Port 번호는 다르게 처리한다.
		wsprintf( szPort, "\\\\.\\COM%d", nPort);


	// 포트 열기
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
		// 포트 OPEN 불가능
		return FALSE;   
	}
	else    
	{	
		// 포트 OPEN 가능
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
		// 10 이상의 Port 번호는 다르게 처리한다.
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
	// 포트 상태 설정
	//-----------------------------------------
	
	// timeout 설정
	timeouts.ReadIntervalTimeout = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 1000;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;
	timeouts.WriteTotalTimeoutConstant = 1000;
	bResult = SetCommTimeouts( m_hComPort, &timeouts );

	if( bResult == FALSE )
		writeMessage("SetCommTimeouts() error in openPort()");


	// EV_RXCHAR Event만을 사용한다.
	// ( EV_BREAK, EV_CTS, EV_ERR, EV_RING, EV_RLSD, EV_RXFLAG, EV_TXEMPTY )
	bResult = SetCommMask( m_hComPort, EV_RXCHAR );

	if( bResult == FALSE )
		writeMessage("SetCommMask() error in openPort()");

	// InQueue, OutQueue 크기설정
	bResult = SetupComm( m_hComPort, 4096, 4096 );

	if( bResult == FALSE )
		writeMessage("SetupComm() error in openPort()");

	// 포트 설정을 읽어낸다.
//	dcb.DCBlength = sizeof(DCB);
//	bResult = GetCommState(m_hComPort, &dcb);

//	if( bResult == FALSE )
//		writeMessage("GetCommState() error in openPort()");

	// 포트를 새로이 설정한다.
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
//	dcb.fOutX = TRUE;			// Xon, Xoff 사용
//	dcb.XonChar = ASCII_XON;
//	dcb.XoffChar = ASCII_XOFF;
//	dcb.XonLim = 100;
//	dcb.XoffLim = 100;

	bResult = SetCommState(m_hComPort, lpDCB);
	
	if( bResult == FALSE )
		writeMessage("SetCommState() error in openPort()");

	// comport buffer 를 비운다.
	PurgeComm(	m_hComPort, 
				PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);


	// Stop the Critecal Section;
	LeaveCriticalSection( &m_csCommSync );


	// Parent 가 초기화 되었을때만 쓰레드가 동작한다.
	if( m_pParent != NULL )
	{
		// COM Port의 Open과 동시에 Thread 를 생성하고 동작시킨다.
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
	
	m_bOpened = TRUE;		// 무사히 포트가 열림을 뜻한다.

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


	// 포트 설정을 읽어낸다.
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


	// 포트 설정을 읽어낸다.
	dcb.DCBlength = sizeof(DCB);
	bResult = GetCommState(m_hComPort, &dcb);

	if( bResult == FALSE )
		writeMessage("GetCommState() error in changePortState()");

	// 포트를 새로이 설정한다.
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
//	dcb.fOutX = TRUE;			// Xon, Xoff 사용
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
