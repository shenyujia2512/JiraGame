#include "ClientEntity.h"
#include "../GameLogic/SceneFactory.h"
#include <thread>

CClientEntity::CClientEntity(SOCKET clientSocket, sockaddr_in clientAddr)
{
	//初始化
	sockClient = clientSocket;
	port = ntohs(clientAddr.sin_port);
	ipAddress = inet_ntoa(clientAddr.sin_addr);

	SessionID = "";
	isBroken = false;
	m_ThreadPower = false;
	m_HasThread = false;
	m_iDeltaTime = 0;
	m_pTimeFunc = nullptr;
	m_pTimeFuncArg = nullptr;
	roleInfo = new PBMessage::PlayerInfo();
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	memset(&revBuffer, 0, sizeof(revBuffer));
}

CClientEntity::~CClientEntity()
{
	SAFE_DELETE(roleInfo);
}

void CClientEntity::NotifyClient(char sendBuffer[MSGSIZE], int Size)
{
	int byte = send(sockClient, sendBuffer, Size, 0);

	setColor(10);
	cout << "Debug Log: NotifyClient -> " << ipAddress << ":" << port << "  PackSize:" << Size << "byte!" << endl;
	setColor();
}

void CClientEntity::BroadcastNotifyClient(char sendBuffer[MSGSIZE], bool _isBroadcastSelf)
{
	//发送消息
	HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Send, (LPVOID)this, 0, 0);

	if (hThread1 != NULL)
	{
		CloseHandle(hThread1);
	}
}

void CClientEntity::MsgDistribute()
{
	//解析包
	PBMessage::UnPacket unPacket;
	unPacket.ParseFromArray(revBuffer, revBufferSize);

	//收到的消息内容
	setColor(11);
	cout << "Debug Log: RecClientMsg ->  PackSize:" << revBufferSize << "byte!" << endl;
	setColor();

	//场景消息
	PBMessage::CurScene curScene = unPacket.curscene();
	CSceneFactory::GetInstance()->ProcessMsgLogic(curScene, sockClient, unPacket);
	
	//释放内存
	unPacket.release_loginrequest();
	unPacket.Clear();
}

void CClientEntity::GeneralSend()
{
	int byte = send(sockClient, sendBuffer, strlen(sendBuffer), 0);
	cout << "Debug : GeneralSend -> " << ipAddress << ":" << port << " [ " << sendBuffer << " ] " << endl;
}

void CClientEntity::Broadcast()
{
	//发送消息
	HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Send, (LPVOID)this, 0, 0);

	if (hThread1 != NULL)
	{
		CloseHandle(hThread1);
	}
}

void CClientEntity::SetTimerMutex(int deltaTime, TimerProcess pTimeFunc, CSceneBase* Arg)
{
	m_pTimeFunc = pTimeFunc;
	m_pTimeFuncArg = Arg;
	m_iDeltaTime = deltaTime;
	auto aa = this;

	//创建子线程
	m_HasThread = true;
	m_ThreadPower = false;
	thread _thread(SetTimerMutexThread, this);

	_thread.detach();
}

unsigned WINAPI CClientEntity::SetTimerMutexThread(void* pvParam)
{
	if (pvParam == NULL) return 0;

	CClientEntity* self = (CClientEntity*)pvParam;

	while (true)
	{
		if (CServer::g_bExit || self->m_ThreadPower) break;

		static DWORD preTime = time(0);

		if (time(0) > (preTime + self->m_iDeltaTime))
		{
			self->m_pTimeFunc(self->m_pTimeFuncArg);
			preTime = time(0);
		}
	}

	return 0;
}

void CClientEntity::StopTimerMutex()
{
	m_ThreadPower = true;
	m_iSendMsgCount = 0;
}
