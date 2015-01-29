#pragma once
#include <list>
#include "USBProtocol.h"
#include <windows.h>
#define ERROR_BUFFER_SIZE 64
using namespace std;
class CUSB
{
public:
	CUSB();
	void WriteToUSB(char *_string);
	void WriteToUSB(char *_string, UINT16 _nSize);
	void StopDataTransfer();
	BOOL InitUSB();
	BOOL CloseUSB();
	BOOL MonitoringStart();
	BOOL MonitoringResume();
	BOOL MonitoringSuspend();
	HANDLE GetRecieveSignal(){ return m_hRecieveSignal; };
	HANDLE GetAbnormalSignal(){ return m_hAbnormalSignal; };
	BOOL GetRecieveBuffer(char *_pBuffer);
	BOOL GetRecieveBuffer(list<CPR_DATA>*);
	UINT16 GetErrorCode();
	BOOL GetErrorLog(char*);
	~CUSB();
protected:
	static UINT	USBThread(LPVOID pParam);
	static BOOL RecieveData(CUSB* _objCUSB);
	static BOOL TransmitData(CUSB* _objCUSB);
	static UINT TestThread(LPVOID pParam);
	
private:
	void SendCommandFrame(const char _CommandType, const char *CommandPara = NULL);
	//Father thread
	CWinThread* m_Thread;
	//USB handle
	void* m_hUSB;
	//BUFFER
	char *m_szRecieveBuffer;
	char *m_szWriteBuffer;
	std::list<CPR_DATA> m_Packetlist;
	DWORD m_nWriteSize;
	DWORD m_nRecieveSize;
	//Packet
	CPR_DATA *m_RecievePacket;
	//Event handle
	HANDLE m_hReadEvent;
	HANDLE m_hWriteEvent;
	HANDLE m_hShutDown;
	HANDLE m_hNormalClose;
	HANDLE m_hEventArray[4];
	//Signal handle
	HANDLE m_hAbnormalSignal;
	HANDLE m_hRecieveSignal;
	//Some flag
	BOOL m_bThreadAlive;
	BOOL m_bDriverInstalled;
	BOOL m_bisTest;
	BOOL m_bisReadFinish;
	//Critical Section
	CRITICAL_SECTION CriticalUSBSection;
	CRITICAL_SECTION CriticalReadSection;
	//Error Handle
	UINT16 m_nErrorCode;
	char *m_szErrorInfo;
};

