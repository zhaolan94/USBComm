#pragma once
#include <list>
#define USB_BUFFER_LEN 512
#define RECIEVE_PACKET_SIZE 256
#define ECHO_PACKET_SIZE 64
using namespace std;
typedef struct
{
	UINT16 len;
	char button[5][6];
}sRecievePacket;
class CUSB
{
public:
	CUSB();
	void WriteToUSB(char *_string);
	BOOL InitUSB();
	BOOL CloseUSB();
	BOOL MonitoringStart();
	BOOL MonitoringResume();
	BOOL MonitoringSuspend();
	HANDLE GetRecieveSignal(){ return m_hRecieveSignal; };
	BOOL GetRecieveBuffer(char *_pBuffer);
	BOOL GetRecieveBuffer(list<sRecievePacket>*);
	~CUSB();
protected:
	static UINT	USBThread(LPVOID pParam);
	static BOOL RecieveData(CUSB* _objCUSB);
	static BOOL TransmitData(CUSB* _objCUSB);
	static UINT TestThread(LPVOID pParam);
	
private:
	//Father thread
	CWinThread* m_Thread;
	//USB handle
	void* m_hUSB;
	//BUFFER
	char *m_szRecieveBuffer;
	char *m_szWriteBuffer;
	std::list<sRecievePacket> m_Packetlist;
	DWORD m_nWriteSize;
	DWORD m_nRecieveSize;
	//Packet
	sRecievePacket *m_RecievePacket;
	//Event handle
	HANDLE m_hReadEvent;
	HANDLE m_hWriteEvent;
	HANDLE m_hShutDown;
	HANDLE m_hEventArray[3];
	//Signal handle
	HANDLE m_hRecieveSignal;
	//Some flag
	BOOL m_bThreadAlive;
	BOOL m_bDriverInstalled;
	BOOL m_bisTest;
	BOOL m_bisReadFinish;
	//Critical Section
	CRITICAL_SECTION CriticalUSBSection;
	CRITICAL_SECTION CriticalReadSection;
};

