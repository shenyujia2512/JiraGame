#pragma once
#include "Utl/Utilitys.h"
#include "Server/Server.h"
#include "Cache/Cache.h"

CServer* server = nullptr;

//����̨���ڹر�ʱ���� �ͷ��ڴ�
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (CTRL_CLOSE_EVENT == dwCtrlType)
	{
		if (server != nullptr)
		{
			server->g_bExit = true;
			SAFE_DELETE(server);
		}
	}

	return FALSE;
}

void main()
{
	//ʱ����Ϊ���������
	srand((unsigned)time(NULL));

	//���ü�����̨�ر��¼�
	SetConsoleCtrlHandler(HandlerRoutine, true);

	//���÷�����
	server = new CServer();
	server->StartCServer();
}