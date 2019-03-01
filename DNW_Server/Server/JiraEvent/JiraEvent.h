#pragma once
#include "../Utl/utility.h"
#include "../Singleton/Singleton.h"

//观察者基类
class CObCServer
{
public:
	CObCServer(){  }

	virtual ~CObCServer(){  }

	virtual void NotifyClient(char sendBuffer[MSGSIZE], int Size){}

	virtual void BroadcastNotifyClient(char sendBuffer[MSGSIZE], bool _isBroadcastSelf){}
};

//被观察者基类
class CJiraEvent :public Singleton<CJiraEvent>
{
public:
	CJiraEvent();
	virtual ~CJiraEvent();

public:
	void AddListenner(SOCKET socket,CObCServer* CObCServer);			//添加观察者
	void RemoverListenner(SOCKET socket);								//移除观察者
	void SendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size);		//通知观察者	
	void SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool isBroadcastSelf);//广播消息

private:
	map<SOCKET,CObCServer*> m_CObCServers;	//观察者键值对
};

