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
	//���͵�����Ϣ���ͻ���
	virtual void sendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size);

	//���͹㲥��Ϣ���ͻ���
	virtual void SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool _isBroadcastSelf = true);

public:
	SOCKET clientSocket;			//�ͻ���socket
	char sendBuffer[MSGSIZE];		//�������ݻ�����
	PBMessage::UnPacket RecMessage;	//��������
};