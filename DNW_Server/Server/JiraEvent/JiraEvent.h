#pragma once
#include "../Utl/utility.h"
#include "../Singleton/Singleton.h"

//�۲��߻���
class CObCServer
{
public:
	CObCServer(){  }

	virtual ~CObCServer(){  }

	virtual void NotifyClient(char sendBuffer[MSGSIZE], int Size){}

	virtual void BroadcastNotifyClient(char sendBuffer[MSGSIZE], bool _isBroadcastSelf){}
};

//���۲��߻���
class CJiraEvent :public Singleton<CJiraEvent>
{
public:
	CJiraEvent();
	virtual ~CJiraEvent();

public:
	void AddListenner(SOCKET socket,CObCServer* CObCServer);			//��ӹ۲���
	void RemoverListenner(SOCKET socket);								//�Ƴ��۲���
	void SendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size);		//֪ͨ�۲���	
	void SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool isBroadcastSelf);//�㲥��Ϣ

private:
	map<SOCKET,CObCServer*> m_CObCServers;	//�۲��߼�ֵ��
};

