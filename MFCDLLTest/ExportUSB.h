#pragma once
class CUSB;
class __declspec(dllexport) CExportUSB
{
public:
	CExportUSB();
	~CExportUSB();
	BOOL InitUSB();
	BOOL CloseUSB();
	void WriteToPort(char *_string);
	BOOL MonitoringStart();
	BOOL MonitoringResume();
	BOOL MonitoringSuspend();
	HANDLE GetRecieveSignal();
	BOOL GetRecieveBuffer(char *_pBuffer);
private:
	CUSB *m_pCUSB;
};

//Export By C---------------------------------
#ifdef __cplusplus

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

//
// Functions exported by this DLL.
//
extern "C" {
#endif

	    void  __stdcall CreateUSBObject();
		void __stdcall DestoryUSBObject();
		BOOL __stdcall InitUSB();
		BOOL __stdcall CloseUSB();
		BOOL __stdcall MonitoringStart();
		BOOL __stdcall MonitoringResume();
		BOOL __stdcall MonitoringSuspend();
		void __stdcall WriteToPort(char*);
		HANDLE __stdcall GetRecieveSignal();
		BOOL __stdcall GetRecieveBuffer(char*);
		BOOL __stdcall GetDataByList(void*);




#ifdef __cplusplus
}
#endif
