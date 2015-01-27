#pragma once
#include <list>
#define USB_BUFFER_LEN 1024
#define RECIEVE_PACKET_SIZE 256
#define ECHO_PACKET_SIZE 64

#define CPR_FRAME_HEADER {'C','P','R',0}
#define CPR_FRAME_VER 1
#define CPR_FRAME_HEADER_SIZE 4
#define CPR_FRAME_RESERVED_SIZE 4
#define CPR_DATAS_PER_PACKET 100
//÷°¿‡–Õ
#define CPR_FRAME_TYPE_COMMAND 0x01
#define CPR_FRAME_TYPE_DATA 0x02
#define CPR_FRAME_TYPE_ECHO 0x03
using namespace std;
typedef struct _CPR_FRAME_BEGIN
{
	char szHeader[CPR_FRAME_HEADER_SIZE];
	char nVer;
	char nFrameType;
	UINT16 nFrameLenth;
}CPR_FRAME_BEGIN;
typedef struct _CPR_FRAME_END
{
	char szReserved[CPR_FRAME_RESERVED_SIZE];
}CPR_FRAME_END;
typedef struct _CPR_DATA
{
	
	UINT16 len;
	char button[5][6];
}CPR_DATA;
typedef struct _CPR_COMMAND_FRAME
{
	CPR_FRAME_BEGIN Frame_Begin;
	char nCommandType;
	char szCommandParamter[51];
	CPR_FRAME_END Frame_End;
}CPR_COMMAND_FRAME;
typedef struct _CPR_DATA_FRAME
{
	CPR_FRAME_BEGIN Frame_Begin;
	CPR_DATA szData[CPR_DATAS_PER_PACKET];
	CPR_FRAME_END Frame_End;
}CPR_DATA_FRAME;
typedef struct _CPR_ECHO_FRAME
{
	CPR_FRAME_BEGIN Frame_Begin;
	char nEchoType;
	char szEchoInfo[51];
	CPR_FRAME_END Frame_End;
}CPR_ECHO_FRAME;
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
	BOOL GetRecieveBuffer(list<CPR_DATA>*);
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
	std::list<CPR_DATA> m_Packetlist;
	DWORD m_nWriteSize;
	DWORD m_nRecieveSize;
	//Packet
	CPR_DATA *m_RecievePacket;
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

