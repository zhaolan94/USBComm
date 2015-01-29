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
	m_hNormalClose = NULL;
	m_hRecieveSignal = NULL;
	m_hAbnormalSignal = NULL;
	m_bDriverInstalled = false;
	m_bThreadAlive = false;
	m_hUSB = NULL;
	m_szRecieveBuffer = new char[USB_BUFFER_LEN];
	m_szWriteBuffer = new char[USB_BUFFER_LEN];
	m_RecievePacket = new CPR_DATA;
	m_szErrorInfo = new char[ERROR_BUFFER_SIZE];
	m_bisTest = false;
	m_bisReadFinish = true;
	m_nErrorCode = ERROR_NORMAL;
}


CUSB::~CUSB()
{
	if (m_hUSB != NULL)
	{
		TerminateDevice(m_hUSB);
		m_hUSB = NULL;
	}
	delete m_szRecieveBuffer;
	delete m_szWriteBuffer;
	delete m_RecievePacket;
	delete m_szErrorInfo;

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
				SetEvent(m_hNormalClose);
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
	if (m_hNormalClose != NULL)
		ResetEvent(m_hNormalClose);
	else
		m_hNormalClose = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hRecieveSignal != NULL)
		ResetEvent(m_hRecieveSignal);
	else
		m_hRecieveSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hAbnormalSignal != NULL)
		ResetEvent(m_hAbnormalSignal);
	else
		m_hAbnormalSignal = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hEventArray[0] = m_hShutDown;	// highest priority
	m_hEventArray[1] = m_hReadEvent;
	m_hEventArray[2] = m_hWriteEvent;
	m_hEventArray[3] = m_hNormalClose;
	InitializeCriticalSection(&CriticalReadSection);
	InitializeCriticalSection(&CriticalUSBSection);
	m_hUSB = InitializeDevice(BULK_VID, BULK_PID,(LPGUID)&(GUID_DEVINTERFACE_TIVA_BULK),&m_bDriverInstalled);
	if (m_hUSB == NULL)
	{
		m_bisTest = true;
		TRACE("USB iNIT ERROR!");
		m_nErrorCode = ERROR_UNCONNECTED;
		return false;
		
	}
	else
	{
		m_bisTest = false;
		TRACE("USB INited!");
		return true;
	}

}
BOOL CUSB::CloseUSB()
{
	if (m_bisTest)
	{
		m_bThreadAlive = false;
	}
	else
	{
		if (m_bThreadAlive)
		{
			do
			{
				SetEvent(m_hNormalClose);
			} while (m_bThreadAlive);
		}

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
	if (m_hNormalClose != NULL)
	{
		CloseHandle(m_hNormalClose);
		m_hNormalClose = NULL;
	}
	if (m_hRecieveSignal != NULL)
	{
		CloseHandle(m_hRecieveSignal);
		m_hRecieveSignal = NULL;
	}
	if (m_hAbnormalSignal != NULL)
	{
		CloseHandle(m_hAbnormalSignal);
		m_hAbnormalSignal = NULL;
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
	BOOL bIsCheckDevice = false;
	for (;;)
	{
		if (!bIsCheckDevice)
		{
			//未检查设备状态
			objCUSB->SendCommandFrame(CPR_COMMAND_STATUS);
			bIsCheckDevice = true;
		}
		if (bResult != ERROR_IO_PENDING)
		{
			sOverlapped.hEvent = objCUSB->m_hEventArray[1];
			sOverlapped.Offset = 0;
			sOverlapped.OffsetHigh = 0;
			bResult = MyReadUSBPacket(objCUSB->m_hUSB, (unsigned char*)objCUSB->m_szRecieveBuffer, USB_BUFFER_LEN, &objCUSB->m_nRecieveSize,
				INFINITE, objCUSB->m_hEventArray[1], &sOverlapped);
		}
		if (bResult != ERROR_SUCCESS && bResult != ERROR_IO_PENDING)
		{
			//USB连接意外丢失
			objCUSB->m_nErrorCode = ERROR_UNCONNECTED;
			SetEvent(objCUSB->m_hShutDown);
			SetEvent(objCUSB->m_hAbnormalSignal);
		}
		
		Event = WaitForMultipleObjects(4, objCUSB->m_hEventArray, FALSE, INFINITE);
		switch (Event)
		{
		case 0:
			{
				//ShutDown
				objCUSB->m_bThreadAlive = false;
				TRACE("Thread Terminated!\n");
				ResetEvent(objCUSB->m_hEventArray[0]);
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
				ResetEvent(objCUSB->m_hEventArray[1]);
				TRACE("Recieve Data!\n");
				RecieveData(objCUSB);
				bResult = ERROR_SUCCESS;
				break;
			}
		case 2:
		{
			//Write
			ResetEvent(objCUSB->m_hEventArray[2]);
			TransmitData(objCUSB);
			break;
		}
		case 3:
		{
			//Normal Close
			ResetEvent(objCUSB->m_hEventArray[3]);
			const char conHeader[] = CPR_FRAME_HEADER;
			CPR_COMMAND_FRAME *pCommandFrame = new CPR_COMMAND_FRAME;
			memset(pCommandFrame, 0, sizeof(CPR_COMMAND_FRAME));
			memcpy(&pCommandFrame->Frame_Begin.szHeader, conHeader, CPR_FRAME_HEADER_SIZE);
			pCommandFrame->Frame_Begin.nFrameType = CPR_FRAME_TYPE_COMMAND;
			pCommandFrame->Frame_Begin.nFrameLenth = sizeof(CPR_COMMAND_FRAME);
			pCommandFrame->nCommandType = CPR_COMMAND_STOP;
			memcpy(objCUSB->m_szWriteBuffer, pCommandFrame, sizeof(CPR_COMMAND_FRAME));
			objCUSB->m_nWriteSize = sizeof(CPR_COMMAND_FRAME);
			TransmitData(objCUSB);
			SetEvent(objCUSB->m_hShutDown);
			delete pCommandFrame;
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
	srand(unsigned int(time(0)));
	static unsigned char Flag = 0;
	static char nDeltaX = 4;
	while(objCUSB->m_bThreadAlive)
	{
		EnterCriticalSection(&objCUSB->CriticalUSBSection);
		TestPacket.nPushLenth = Flag;
		for (int i = 0; i < 5; i++)
				TestPacket.arryButton[i] = rand()%2;
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
void CUSB::WriteToUSB(char *_string,UINT16 _nSize)
{
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, _string, _nSize);
	m_nWriteSize = _nSize;

	// set event for write
	SetEvent(m_hWriteEvent);
}
BOOL CUSB::RecieveData(CUSB* _objCUSB)
{
	_objCUSB->m_szRecieveBuffer[_objCUSB->m_nRecieveSize] = '\0';
	char szHeader[] = CPR_FRAME_HEADER;
	char *pBuffer = _objCUSB->m_szRecieveBuffer;
	char *pTemp = new char[CPR_FRAME_HEADER_SIZE];
	memcpy(pTemp, pBuffer, sizeof(pTemp));
	if ( 0 == strcmp(pTemp, szHeader) )
	{
		delete pTemp;
		//Get Frame Type
		pTemp = pBuffer + CPR_FRAME_HEADER_SIZE + CPR_FRAME_VER_SIZE;
		if (CPR_FRAME_TYPE_ECHO == *pTemp)
		{
			//GET ECHO FRAME
			pTemp = pBuffer + sizeof(CPR_FRAME_BEGIN);
			if (CPR_ECHO_NORMAL == *pTemp)
			{
				//Device feels good!
				_objCUSB->SendCommandFrame(CPR_COMMAND_START);
				return true;
			}
			else if (CPR_ECHO_ABNORMAL == *pTemp)
			{
				//Device feels BAD!
				_objCUSB->m_nErrorCode = ERROR_CHECKFAULT;
				memcpy(_objCUSB->m_szErrorInfo, pTemp + 1, 51);
				SetEvent(_objCUSB->m_hAbnormalSignal);
				return true;
			}
			else
			{
				//Frame Fault
				_objCUSB->m_nErrorCode = ERROR_FRAMEFAULT;
				SetEvent(_objCUSB->m_hAbnormalSignal);
				return false;
			}
		}
		else if (CPR_FRAME_TYPE_DATA == *pTemp)
		{
			//GET DATA FRAME
			pTemp = pBuffer + sizeof(CPR_FRAME_BEGIN); //Go to Data Section's Base

			EnterCriticalSection(&_objCUSB->CriticalUSBSection);
			for (int i = 0; i < CPR_DATAS_PER_PACKET; i++)
			{
				CPR_DATA pDataTemp;
				memcpy(&pDataTemp, pTemp, sizeof(CPR_DATA));
				_objCUSB->m_Packetlist.push_back(pDataTemp);
				pTemp = pTemp + sizeof(CPR_DATA);

			}
			LeaveCriticalSection(&_objCUSB->CriticalUSBSection);
			TRACE("Recieve %ld Data:%s\n", _objCUSB->m_nRecieveSize, _objCUSB->m_szRecieveBuffer);
			SetEvent(_objCUSB->m_hRecieveSignal);
			
			return true;
		}
		else
		{
			
			//Frame Fault
			_objCUSB->m_nErrorCode = ERROR_FRAMEFAULT;
			SetEvent(_objCUSB->m_hAbnormalSignal);
			return false;
		}

	}
	else
	{
		
		delete pTemp;
		//Frame Fault
		_objCUSB->m_nErrorCode = ERROR_FRAMEFAULT;
		SetEvent(_objCUSB->m_hAbnormalSignal);
		return false;
	}

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

void CUSB::StopDataTransfer()
{
	;
}
void CUSB::SendCommandFrame(const char _CommandType, const char *CommandPara)
{
	const char conHeader[] = CPR_FRAME_HEADER;
	CPR_COMMAND_FRAME *pCommandFrame = new CPR_COMMAND_FRAME;
	memset(pCommandFrame, 0, sizeof(CPR_COMMAND_FRAME));
	memcpy(&pCommandFrame->Frame_Begin.szHeader, conHeader, CPR_FRAME_HEADER_SIZE);
	pCommandFrame->Frame_Begin.nFrameType = CPR_FRAME_TYPE_COMMAND;
	pCommandFrame->Frame_Begin.nFrameLenth = sizeof(CPR_COMMAND_FRAME);
	pCommandFrame->nCommandType = _CommandType;
	if (CommandPara != NULL)
	{
		strcpy_s(pCommandFrame->szCommandParamter, CommandPara);
	}
	WriteToUSB((char*)pCommandFrame, (UINT16)sizeof(CPR_COMMAND_FRAME));
	delete pCommandFrame;

}
UINT16 CUSB::GetErrorCode()
{
	return m_nErrorCode;
}
BOOL CUSB::GetErrorLog(char*_pTemp)
{
	memcpy(_pTemp, m_szErrorInfo, ECHO_PACKET_SIZE);
	return true;
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