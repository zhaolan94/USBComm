#include "stdafx.h"
#include "ExportUSB.h"
#include "USB.h"

CExportUSB::CExportUSB()
{
	m_pCUSB = new CUSB();
}


CExportUSB::~CExportUSB()
{
	if (m_pCUSB)
	{
		delete m_pCUSB;
		m_pCUSB = NULL;
	}
}

BOOL CExportUSB::InitUSB()
{
	return m_pCUSB->InitUSB();
}

BOOL CExportUSB::CloseUSB()
{
	return m_pCUSB->CloseUSB();
}

BOOL CExportUSB::MonitoringStart()
{
	return m_pCUSB->MonitoringStart();
}

BOOL CExportUSB::MonitoringResume()
{
	return m_pCUSB->MonitoringResume();
}

BOOL CExportUSB::MonitoringSuspend()
{
	return m_pCUSB -> MonitoringSuspend();
}
void CExportUSB::WriteToPort(char* _string)
{
	m_pCUSB->WriteToUSB(_string);
}
HANDLE CExportUSB::GetRecieveSignal()
{
	return m_pCUSB->GetRecieveSignal();
}
BOOL CExportUSB::GetRecieveBuffer(char *_pBuffer)
{
	return  m_pCUSB->GetRecieveBuffer(_pBuffer);
}

//Export By C
CUSB *pCUSB;
extern "C" void PASCAL EXPORT
CreateUSBObject()
{
	pCUSB = new CUSB();
}

extern "C" void PASCAL EXPORT
DestoryUSBObject()
{
	if (pCUSB)
	{
		delete pCUSB;
		pCUSB = NULL;
	}
}

extern "C" BOOL PASCAL EXPORT
InitUSB()
{
	return pCUSB->InitUSB();
}

extern "C" BOOL PASCAL EXPORT
CloseUSB()
{
	return pCUSB->CloseUSB();
}

extern "C" BOOL PASCAL EXPORT
MonitoringStart()
{
	return pCUSB->MonitoringStart();
}

extern "C" BOOL PASCAL EXPORT
MonitoringResume()
{
	return pCUSB->MonitoringResume();
}

extern "C" BOOL PASCAL EXPORT
MonitoringSuspend()
{
	return pCUSB->MonitoringSuspend();
}

extern "C" void PASCAL EXPORT
WriteToPort(char* _string)
{
	pCUSB->WriteToUSB(_string);
}

extern "C" HANDLE PASCAL EXPORT
GetRecieveSignal()
{
	return pCUSB->GetRecieveSignal();
}

extern "C" BOOL PASCAL EXPORT
GetRecieveBuffer(char *_pBuffer)
{
	return  pCUSB->GetRecieveBuffer(_pBuffer);
}

extern "C" BOOL PASCAL EXPORT
GetDataByList(void* _pTemp)
{
	return pCUSB->GetRecieveBuffer((list<CPR_DATA>*) _pTemp);

}