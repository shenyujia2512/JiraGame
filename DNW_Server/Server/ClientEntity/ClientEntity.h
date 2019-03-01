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


//ʵ��
class CClientEntity :public CObCServer
{
	typedef void(*TimerProcess)(CSceneBase*);

public:
	CClientEntity(SOCKET clientSocket, sockaddr_in clientAddr);

	virtual ~CClientEntity();

	//��Ϣ����
	friend void Send(CClientEntity* self)
	{

	};

	//��Ϣ֪ͨ
	void NotifyClient(char sendBuffer[MSGSIZE], int Size);

	//�㲥��Ϣ���ͻ���
	void BroadcastNotifyClient(char sendBuffer[MSGSIZE], bool _isBroadcastSelf);

	// ��ͻ��˷��͹㲥��Ϣ
	void Broadcast();

	//��ͻ��˷��͵�����Ϣ
	void GeneralSend();
	
	//��Ϣ·��
	void MsgDistribute();

	//��ʱ��
	void SetTimerMutex(int deltaTime, TimerProcess pTimeFunc, CSceneBase* Arg);

	void StopTimerMutex();

private:
	static unsigned WINAPI SetTimerMutexThread(void* pvParam);

	
public:
	string SessionID;				//�û�ʵ���sessionID
	int  revBufferSize;				//�������ݴ�С
	char revBuffer[MSGSIZE];		//�������ݻ���
	char sendBuffer[MSGSIZE];		//�������ݻ���
	SOCKET sockClient;				//�ͻ���Socket

	bool			m_ThreadPower;		//�߳̿���
	bool            m_HasThread;		//�Ƿ��Ѵ����߳�
	UINT			m_iDeltaTime;		//��ʱ��ʱ����
	TimerProcess	m_pTimeFunc;		//��ʱ��������ָ��
	CSceneBase*		m_pTimeFuncArg;		//��ʱ������������ָ��
	int				m_iSendMsgCount;	//���������

protected:
	GS_Field(bool, isBroken, IsBroken);						//��ǿͻ����Ƿ����
	GS_Field(string, ipAddress, IpAddress);					//IP��ַ
	GS_Field(int, port, Port);								//�˿�
	GS_Field(PBMessage::PlayerInfo*, roleInfo,RoleInfo);	//��ɫ��Ϣ
};

