#include "JiraEvent.h"


CJiraEvent::CJiraEvent()
{

}

CJiraEvent::~CJiraEvent()
{

}

void CJiraEvent::AddListenner(SOCKET socket, CObCServer* CObCServer)
{
	m_CObCServers[socket] = CObCServer;
}

void CJiraEvent::RemoverListenner(SOCKET socket)
{
	m_CObCServers.erase(socket);
}

void CJiraEvent::SendMsg(SOCKET socket, char sendBuffer[MSGSIZE],int Size)
{
	if (m_CObCServers[socket])
	{
		m_CObCServers[socket]->NotifyClient(sendBuffer, Size);
	}
}

void CJiraEvent::SendBroadCast(SOCKET socket, char sendBuffer[MSGSIZE], bool isBroadcastSelf)
{
	if (m_CObCServers[socket])
	{
		m_CObCServers[socket]->BroadcastNotifyClient(sendBuffer, isBroadcastSelf);
	}
}