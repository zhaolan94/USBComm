#include "stdafx.h"
#include "USB.h"
#include "tiva_guids.h"
#include "lmusbdll.h"
#include <winusb.h>
#include <list>
#include <iterator>
DWORD MyReadUSBPacket(LMUSB_HANDLE hUSB, unsigned char *pcBuffer, unsigned long ulSize,
	unsigned long *pulRead, unsigned long ulTimeoutmS, HANDLE _Signal,OVERLAPPED *_sOverlapped);
typedef struct
{
	HANDLE deviceHandle;
	WINUSB_INTERFACE_HANDLE winUSBHandle;
	UCHAR deviceSpeed;
	UCHAR bulkInPipe;
	UCHAR bulkOutPipe;
	HANDLE hReadEvent;
} tDeviceInfoWinUSB;
CUSB::CUSB()
{
	m_hReadEvent = NULL;
	m_hWriteEvent = NULL;
	m_hShutDown = NULL;
	m_bDriverInstalled = false;
	m_bThreadAlive = false;
	m_hUSB = NULL;
	m_szRecieveBuffer = new char[USB_BUFFER_LEN];
	m_szWriteBuffer = new char[USB_BUFFER_LEN];
	m_RecievePacket = new CPR_DATA;
	m_bisTest = false;
	m_RecievePacket = new CPR_DATA;
	m_bisReadFinish = true;
}


CUSB::~CUSB()
{
	if (m_hUSB != NULL)
	{
		TerminateDevice(m_hUSB);
		m_hUSB = NULL;
	}
}
BOOL CUSB::InitUSB()
{
	if (m_bThreadAlive)
	{
		if (m_bisTest)
		{
			m_bThreadAlive = false;
		}
		else
		{
			do{
				SetEvent(m_hShutDown);
			} while (m_bThreadAlive);
			TerminateDevice(m_hUSB);
		}

	}

	if (m_hReadEvent != NULL)
		ResetEvent(m_hReadEvent);
	else
		m_hReadEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	if (m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);
	else
		m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hShutDown != NULL)
		ResetEvent(m_hShutDown);
	else
		m_hShutDown = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hRecieveSignal != NULL)
		ResetEvent(m_hRecieveSignal);
	else
		m_hRecieveSignal = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hEventArray[0] = m_hShutDown;	// highest priority
	m_hEventArray[1] = m_hReadEvent;
	m_hEventArray[2] = m_hWriteEvent;
	InitializeCriticalSection(&CriticalReadSection);
	InitializeCriticalSection(&CriticalUSBSection);
	m_hUSB = InitializeDevice(BULK_VID, BULK_PID,(LPGUID)&(GUID_DEVINTERFACE_TIVA_BULK),&m_bDriverInstalled);
	if (m_hUSB == NULL)
	{
		m_bisTest = true;
		TRACE("USB iNIT ERROR!");
		return false;
		
	}
	TRACE("USB INited!");
	return true;
}
BOOL CUSB::CloseUSB()
{
	if (m_bisTest)
	{
		m_bThreadAlive = false;
	}
	else
	{
		do
		{
			SetEvent(m_hShutDown);
		} while (m_bThreadAlive);
	}


	if (m_hReadEvent != NULL)
	{
		CloseHandle(m_hReadEvent);
		m_hReadEvent = NULL;
	}

	if (m_hWriteEvent != NULL)
	{
		CloseHandle(m_hWriteEvent);
		m_hWriteEvent = NULL;
	}

	if (m_hShutDown != NULL)
	{
		CloseHandle(m_hShutDown);
		m_hShutDown = NULL;
	}
	if (m_hUSB != NULL)
	{
		TerminateDevice(m_hUSB);
		m_hUSB = NULL;
	}
	DeleteCriticalSection(&CriticalUSBSection);
	DeleteCriticalSection(&CriticalReadSection);
	return true;

}
BOOL CUSB::MonitoringStart()
{
	if (m_bisTest)
	{
		m_bThreadAlive = true;
		if (!(m_Thread = AfxBeginThread(TestThread, this)))
			return FALSE;
		TRACE("Monitor started\n");
		return TRUE;
	}
	else
	{
		if (!(m_Thread = AfxBeginThread(USBThread, this)))
			return FALSE;
		TRACE("Monitor started\n");
		return TRUE;
	}

}
BOOL CUSB::MonitoringResume()
{
	if (m_Thread)
	{
		TRACE("Monitor resumed\n");
		m_Thread->ResumeThread();
	}
	return TRUE;
}


//
// Suspend the comm thread
//
BOOL CUSB::MonitoringSuspend()
{
	if (m_Thread)
	{
		TRACE("Monitor suspended\n");
		m_Thread->SuspendThread();
	}
	return TRUE;
}
UINT CUSB::USBThread(LPVOID pParam)
{
	CUSB *objCUSB = (CUSB*)pParam;
	DWORD Event;
	DWORD bResult = ERROR_SUCCESS;
	OVERLAPPED sOverlapped;
	for (;;)
	{
	
		if (bResult != ERROR_IO_PENDING)
		{
			sOverlapped.hEvent = objCUSB->m_hEventArray[1];
			sOverlapped.Offset = 0;
			sOverlapped.OffsetHigh = 0;
			bResult = MyReadUSBPacket(objCUSB->m_hUSB, (unsigned char*)objCUSB->m_szRecieveBuffer, USB_BUFFER_LEN, &objCUSB->m_nRecieveSize,
				INFINITE, objCUSB->m_hEventArray[1], &sOverlapped);
		}
		
		Event = WaitForMultipleObjects(3, objCUSB->m_hEventArray, FALSE, INFINITE);
		switch (Event)
		{
		case 0:
			{
				//ShutDown
				objCUSB->m_bThreadAlive = false;
				TRACE("Thread Terminated!\n");
				AfxEndThread(100);
				break;
			}
		case 1:
			{
				//Read
				if (bResult = ERROR_IO_PENDING)
				{
					GetOverlappedResult(objCUSB->m_hUSB, &sOverlapped, &objCUSB->m_nRecieveSize, FALSE);
				}
				TRACE("Recieve Data!\n");
				if (objCUSB->m_bisReadFinish)
				{
					RecieveData(objCUSB);
					SetEvent(objCUSB->m_hRecieveSignal);
				}
				bResult = ERROR_SUCCESS;
				break;
			}
		case 2:
		{
			//Read
			ResetEvent(objCUSB->m_hWriteEvent);
			TransmitData(objCUSB);
			break;
		}

		}


	}
	return NULL;
}
#include<time.h>
UINT CUSB::TestThread(LPVOID pParam)
{
	CPR_DATA TestPacket;
	CUSB *objCUSB = (CUSB*)pParam;
	static unsigned char Flag = 0;
	static char nDeltaX = 4;
	while(objCUSB->m_bThreadAlive)
	{
		EnterCriticalSection(&objCUSB->CriticalUSBSection);
		TestPacket.len = Flag;
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 6;j++)
				TestPacket.button[i][j] = rand()%2;
		objCUSB->m_Packetlist.push_back(TestPacket);
		SetEvent(objCUSB->m_hRecieveSignal);
		LeaveCriticalSection(&objCUSB->CriticalUSBSection);
		Sleep(1000/100);
		Flag = Flag + nDeltaX + (rand() % 3);
		if (Flag >= 100)
		{
			nDeltaX = -4;
		}

		if (Flag <= 10) nDeltaX = 4;
	}
	TRACE("Thread Terminated!\n");
	AfxEndThread(100);
	return NULL;
}

BOOL CUSB::TransmitData(CUSB* _objCUSB)
{
	
	unsigned long pulwritten;
	WriteUSBPacket(_objCUSB->m_hUSB, (unsigned char*)_objCUSB->m_szWriteBuffer, _objCUSB->m_nWriteSize, &pulwritten);
	TRACE("Transmit %ld Data\n", pulwritten);
	return true;
}

void CUSB::WriteToUSB(char *_string)
{
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	strcpy_s( m_szWriteBuffer, sizeof(m_szWriteBuffer), _string);
	m_nWriteSize = strlen(_string);

	// set event for write
	SetEvent(m_hWriteEvent);
}
BOOL CUSB::RecieveData(CUSB* _objCUSB)
{
	
	EnterCriticalSection(&_objCUSB->CriticalUSBSection);
	_objCUSB->m_szRecieveBuffer[_objCUSB->m_nRecieveSize] = '\0';
	CPR_DATA pPackettemp;
	memcpy(&pPackettemp, _objCUSB->m_szRecieveBuffer, sizeof(CPR_DATA));
	_objCUSB->m_Packetlist.push_back(pPackettemp);
	LeaveCriticalSection(&_objCUSB->CriticalUSBSection);
	TRACE("Recieve %ld Data:%s\n", _objCUSB->m_nRecieveSize, _objCUSB->m_szRecieveBuffer);
	Sleep(1000/500);
	return true;
}
BOOL CUSB::GetRecieveBuffer(char *_pBuffer)
{
	EnterCriticalSection(&CriticalUSBSection);
	if (m_Packetlist.empty())

	{
		m_bisReadFinish = true;
		LeaveCriticalSection(&CriticalUSBSection);
		return false;
	}
	else
	{
		m_bisReadFinish = false;
		memcpy(_pBuffer, &m_Packetlist.front(), sizeof(CPR_DATA));
		m_Packetlist.pop_front();
		LeaveCriticalSection(&CriticalUSBSection);
		return true;
	}
		
	
}

BOOL CUSB::GetRecieveBuffer(list<CPR_DATA>* _list)
{
	EnterCriticalSection(&CriticalUSBSection);
	if (m_Packetlist.empty())
	{

		LeaveCriticalSection(&CriticalUSBSection);
		return false;
	}
	else
	{
		std::copy(m_Packetlist.begin(), m_Packetlist.end(), std::back_inserter(*_list));
		m_Packetlist.clear();
		LeaveCriticalSection(&CriticalUSBSection);
		return true;
	}

}


#pragma comment(lib, "winusb.lib ") 
DWORD MyReadUSBPacket(LMUSB_HANDLE hUSB, unsigned char *pcBuffer, unsigned long ulSize,
						unsigned long *pulRead, unsigned long ulTimeoutmS, HANDLE _Signal, OVERLAPPED *_sOverlapped = NULL)
{
	BOOL bResult;
	DWORD dwError;
	OVERLAPPED *sOverlapped;
	tDeviceInfoWinUSB *psDevInfo = (tDeviceInfoWinUSB *)hUSB;

	//
	// Check for bad parameters.
	//
	if (!hUSB || !pcBuffer || !pulRead || !ulSize)
	{
		return(ERROR_INVALID_PARAMETER);
	}

	//
	// Tell WinUSB how to signal us when reads are completed (if blocking)
	//
	if (_sOverlapped != NULL)
	{
		sOverlapped = _sOverlapped;
	}
	else
	{
		sOverlapped = new OVERLAPPED;
		sOverlapped->hEvent = _Signal;
		sOverlapped->Offset = 0;
		sOverlapped->OffsetHigh = 0;
	}


	//
	// Perform the read.
	//
	bResult = WinUsb_ReadPipe(psDevInfo->winUSBHandle,
		psDevInfo->bulkInPipe,
		pcBuffer,
		ulSize,
		pulRead,
		sOverlapped);

	//
	// A good return code indicates success regardless of whether we performed
	// a blocking or non-blocking read.
	//
	if (bResult)
	{
		dwError = ERROR_SUCCESS;
	}
	else
	{
		//
		// An error occurred or the read will complete asynchronously.
		// Which is it?
		//
		dwError = GetLastError();
	}
	return(dwError);
}