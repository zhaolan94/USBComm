#pragma once
#define USB_BUFFER_LEN 1024
#define RECIEVE_PACKET_SIZE 1024
#define ECHO_PACKET_SIZE 64
#define COMMAND_PACKET_SIZE 64

#define CPR_FRAME_HEADER {'C','P','R',0}	//帧头
#define CPR_FRAME_VER 0x01				//版本号v1.0
#define CPR_FRAME_HEADER_SIZE 4
#define CPR_FRAME_RESERVED_SIZE 4	//数据包保留字节
#define CPR_DATAS_PER_PACKET 100	//一个包多少组数据
//帧类型
#define CPR_FRAME_TYPE_COMMAND 0x01	//命令帧
#define CPR_FRAME_TYPE_DATA 0x02	//数据帧
#define CPR_FRAME_TYPE_ECHO 0x03	//响应帧
//命令类型
#define CPR_COMMAND_START 0x01	//开始发送数据
#define CPR_COMMAND_STOP 0x02	//停止发送数据
#define CPR_COMMAND_RESET 0x03	//单片机复位
//响应类型
#define CPR_ECHO_NORMAL 0x01	//单片机自检正常
#define CPR_ECHO_ABNORMAL 0x02	//自检发现故障

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

	unsigned char nPushLenth ; //按压深度
	unsigned char nBreathLenth; //呼吸深度
	unsigned char arryButton[5]; //按压区域
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