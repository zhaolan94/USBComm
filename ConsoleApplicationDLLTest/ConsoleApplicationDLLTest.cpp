// ConsoleApplicationDLLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "MFCDLLTEST.h"
#include <list>
#include <iterator>
using namespace std;
typedef struct _CPR_DATA
{

	unsigned char nPushLenth; //按压深度
	unsigned char nBreathLenth; //呼吸深度
	unsigned char arryButton[5]; //按压区域
}CPR_DATA;
int _tmain(int argc, _TCHAR* argv[])
{
	/*
	CExportUSB *objCUSB;
	HANDLE Signal;
	char Buffer[1024];
	objCUSB = new CExportUSB();
	objCUSB->InitUSB();
	Signal = objCUSB->GetRecieveSignal();
	objCUSB->MonitoringStart();
	while (1)
	{
		WaitForMultipleObjects(1, &Signal, FALSE, INFINITE);
		ResetEvent(Signal);
		objCUSB->GetRecieveBuffer(Buffer);
		printf("Buffer:%s", Buffer);
	}

	system("pause");
	objCUSB->CloseUSB();
	system("pause");

	delete objCUSB;
	*/

	HANDLE Signal;
	list<CPR_DATA> listBuffer;
	CPR_DATA Buffer;
	CreateUSBObject();
	InitUSB();
	Signal = GetRecieveSignal();
	MonitoringStart();
	int hit = 0;
	while (1)
	{
		
		WaitForMultipleObjects(1, &Signal, FALSE, INFINITE);
		ResetEvent(Signal);
		while (GetDataByList(&listBuffer))
		{
			do
			{
				memcpy(&Buffer, &listBuffer.front(), sizeof(CPR_DATA));
				listBuffer.pop_front();
				printf("PushLenth:%d\n", Buffer.nPushLenth);
				printf("PushLenth:%d\n", Buffer.nBreathLenth);
				for (int i = 0; i < 5; i++)
				{
					printf("[%d]", Buffer.arryButton[i]);
				}
				printf("\n");
				printf("HIT:%d\n", hit);
			} while (!listBuffer.empty());

		//	system("cls");
			
		}
		hit++;
		
	}

	system("pause");
	CloseUSB();
	system("pause");

	DestoryUSBObject();
	return 0;
}

