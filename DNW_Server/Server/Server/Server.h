#pragma once
#include "../Utl/utility.h"

class CClientEntity;
class CCache;
class CServer
{

public:
	CServer();

	virtual ~CServer();

	void destroyPtr();

public:
	//创建服务器
	void StartCServer();

	//线程处理函数
	friend DWORD WINAPI WorkerThread(LPVOID lpParam);

private:
	//断开与客户端的所有连接
	void DisconnectedFromCServer();

public: 


private :
	WSADATA wsaData;			//初始化环境数据
	sockaddr_in CServerAddr;	//服务器地址信息
	sockaddr_in clientAddr;		//客户端地址信息
	SOCKET listenSocket;		//监听套接字
	SOCKET clientSocket;		//客户端连接过来的套接字

	HANDLE hMutex;
	char Buffer[MSGSIZE];		//数据缓冲

public:
	static bool g_bExit;					  //标示程序退出
	static UINT g_icheckTime;
	static int  g_iTotalConn;
	static SOCKET g_CliSocketArr[FD_SETSIZE]; //用来存放当前所有活动的客户端套接字的套接字数组
};