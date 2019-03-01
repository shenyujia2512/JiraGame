#pragma once
#include "../Utl/Utilitys.h"
#include "../Cache/Cache.h"
#include "../JiraEvent/JiraEvent.h"

#include <windows.h> 
#include <stdio.h>
#include <process.h>


class CSceneBase
{
public:
	CSceneBase();
	virtual ~CSceneBase();

public:
	//发送单播消息给客户端
	virtual void sendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size);

	//发送广播消息给客户端
	virtual void SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool _isBroadcastSelf = true);

public:
	SOCKET clientSocket;			//客户端socket
	char sendBuffer[MSGSIZE];		//发送数据缓冲区
	PBMessage::UnPacket RecMessage;	//接收数据
};