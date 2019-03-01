#pragma once
#include "Utl/Utilitys.h"
#include "Server/Server.h"
#include "Cache/Cache.h"

CServer* server = nullptr;

//控制台窗口关闭时调用 释放内存
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
	//时间作为随机数种子
	srand((unsigned)time(NULL));

	//设置检测控制台关闭事件
	SetConsoleCtrlHandler(HandlerRoutine, true);

	//启用服务器
	server = new CServer();
	server->StartCServer();
}