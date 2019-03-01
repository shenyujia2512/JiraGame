#pragma once
#include "../Utl/Utilitys.h"
#include "../Cache/Cache.h"
#include "../Server/Server.h"
#include "../DBServer/DBServer.h"
#include "../JiraEvent/JiraEvent.h"
#include "../GameLogic/SceneBase.h"
#include "../GameLogic/SceneFactory.h"

class CCache;
class CServer;
class CDBServer; 
class CJiraEvent;
class CSceneBase;
class CSceneFactory;


//实体
class CClientEntity :public CObCServer
{
	typedef void(*TimerProcess)(CSceneBase*);

public:
	CClientEntity(SOCKET clientSocket, sockaddr_in clientAddr);

	virtual ~CClientEntity();

	//消息发送
	friend void Send(CClientEntity* self)
	{

	};

	//消息通知
	void NotifyClient(char sendBuffer[MSGSIZE], int Size);

	//广播消息给客户端
	void BroadcastNotifyClient(char sendBuffer[MSGSIZE], bool _isBroadcastSelf);

	// 向客户端发送广播消息
	void Broadcast();

	//向客户端发送单播消息
	void GeneralSend();
	
	//消息路由
	void MsgDistribute();

	//定时器
	void SetTimerMutex(int deltaTime, TimerProcess pTimeFunc, CSceneBase* Arg);

	void StopTimerMutex();

private:
	static unsigned WINAPI SetTimerMutexThread(void* pvParam);

	
public:
	string SessionID;				//用户实体的sessionID
	int  revBufferSize;				//接收数据大小
	char revBuffer[MSGSIZE];		//接收数据缓冲
	char sendBuffer[MSGSIZE];		//发送数据缓冲
	SOCKET sockClient;				//客户端Socket

	bool			m_ThreadPower;		//线程开关
	bool            m_HasThread;		//是否已存在线程
	UINT			m_iDeltaTime;		//定时器时间秒
	TimerProcess	m_pTimeFunc;		//定时器处理函数指针
	CSceneBase*		m_pTimeFuncArg;		//定时器处理函数参数指针
	int				m_iSendMsgCount;	//房间计数器

protected:
	GS_Field(bool, isBroken, IsBroken);						//标记客户端是否断线
	GS_Field(string, ipAddress, IpAddress);					//IP地址
	GS_Field(int, port, Port);								//端口
	GS_Field(PBMessage::PlayerInfo*, roleInfo,RoleInfo);	//角色信息
};

