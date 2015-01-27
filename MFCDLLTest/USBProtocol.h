#pragma once
#define USB_BUFFER_LEN 1024
#define RECIEVE_PACKET_SIZE 1024
#define ECHO_PACKET_SIZE 64
#define COMMAND_PACKET_SIZE 64

#define CPR_FRAME_HEADER {'C','P','R',0}	//֡ͷ
#define CPR_FRAME_VER 0x01				//�汾��v1.0
#define CPR_FRAME_HEADER_SIZE 4
#define CPR_FRAME_RESERVED_SIZE 4	//���ݰ������ֽ�
#define CPR_DATAS_PER_PACKET 100	//һ��������������
//֡����
#define CPR_FRAME_TYPE_COMMAND 0x01	//����֡
#define CPR_FRAME_TYPE_DATA 0x02	//����֡
#define CPR_FRAME_TYPE_ECHO 0x03	//��Ӧ֡
//��������
#define CPR_COMMAND_START 0x01	//��ʼ��������
#define CPR_COMMAND_STOP 0x02	//ֹͣ��������
#define CPR_COMMAND_RESET 0x03	//��Ƭ����λ
//��Ӧ����
#define CPR_ECHO_NORMAL 0x01	//��Ƭ���Լ�����
#define CPR_ECHO_ABNORMAL 0x02	//�Լ췢�ֹ���

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

	unsigned char nPushLenth ; //��ѹ���
	unsigned char nBreathLenth; //�������
	unsigned char arryButton[5]; //��ѹ����
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