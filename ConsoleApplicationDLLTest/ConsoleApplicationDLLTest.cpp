// ConsoleApplicationDLLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "MFCDLLTEST.h"
#include <list>
#include <iterator>
using namespace std;
typedef struct
{
	UINT16 len;
	char button[5][6];
}sRecievePacket;
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
	list<sRecievePacket> listBuffer;
	sRecievePacket Buffer;
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
				memcpy(&Buffer, &listBuffer.front(), sizeof(sRecievePacket));
				listBuffer.pop_front();
				printf("Len:%d\n", Buffer.len);
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 6; j++)
					{
						printf("[%d]", Buffer.button[i][j]);
					}
					printf("\n");
				}
				printf("HIT:%d", hit);
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

