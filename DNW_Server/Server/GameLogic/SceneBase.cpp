#include "SceneBase.h"

CSceneBase::CSceneBase()
{
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	clientSocket = NULL;
}

CSceneBase::~CSceneBase(){}

void CSceneBase::sendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size)
{
	CJiraEvent::GetInstance()->SendMsg(socket, sendBuffer, Size);
}

void CSceneBase::SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool _isBroadcastSelf /*= true*/)
{
	CJiraEvent::GetInstance()->SendBroadCast(socket, sendBuffer, _isBroadcastSelf);
}