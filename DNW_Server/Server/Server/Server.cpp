#include "Server.h"
#include "../Cache/Cache.h"
#include "../ClientEntity/ClientEntity.h"

int CServer::g_iTotalConn = 0;
UINT CServer::g_icheckTime = 1;
bool CServer::g_bExit = false;
SOCKET CServer::g_CliSocketArr[FD_SETSIZE]; //64

CServer::CServer()
{

}

CServer::~CServer()
{
	DisconnectedFromCServer();   //关闭服务

	destroyPtr();
}

void CServer::destroyPtr()
{
	CSceneFactory::DesInstance();

	CCache::DesInstance();
}

DWORD WINAPI WorkerThread(LPVOID lpParam)
{
	int               i;
	fd_set            fdread;
	int               ret;
	struct timeval tv = { 1, 0 };			//指定等待时间为1秒
	char              Buffer[MSGSIZE];

	while (1)
	{
		if (CServer::g_bExit) break;

		FD_ZERO(&fdread);	//将fdread初始化成空集合

		for (i = 0; i < CServer::g_iTotalConn; i++)
		{
			//将所有套接字客户端套接字依次加入集合fdread 
			FD_SET(CServer::g_CliSocketArr[i], &fdread);
		}

		if (fdread.fd_count >= FD_SETSIZE)
		{
			printf("连接数量已达上限!\n");
			continue;
		}

		//判断套接字上是否存在数据可读 可写，这里我们只关心read event
		ret = select(0, &fdread, NULL, NULL, &tv);

		if (ret <= 0)
		{	
			//在等待时间没有接收到数据
			continue;
		}

		for (i = 0; i < CServer::g_iTotalConn; i++)
		{
			//对g_CliSocketArr中客户端套接字进行检查，判断套接字是否仍在集合中
			//select返回后，它会修改每个fdset结构，删除那些不存在待决I/O操作的套接字句柄。
			if (FD_ISSET(CServer::g_CliSocketArr[i], &fdread))
			{
				
				//从g_CliSocketArr[i]接收数据
				ret = recv(CServer::g_CliSocketArr[i], Buffer, MSGSIZE, 0);

				//接收的数据长度为0，或者发生WSAECONNRESET错误，则表示客户端套接字主动关闭
				if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET))
				{	
					// 客户端socket关闭
					setColor(4);
					printf_s("Client socket %d closed.\n", CServer::g_CliSocketArr[i]);
					setColor();

					//服务器中对应的套接字所绑定的资源释放掉，然后调整套接字数组
					closesocket(CServer::g_CliSocketArr[i]);

					//给所有客户端发送销毁消息
					CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(CServer::g_CliSocketArr[i]);
					if (clientEntity)
					{
						memcpy_s(clientEntity->sendBuffer, sizeof(Buffer), Buffer, MSGSIZE);
						//strcpy_s(clientEntity->sendBuffer, Buffer);
						clientEntity->Broadcast();
					}

					//SAFE_DELETE(clientEntity);

					//停止定时器线程
					//ientEntity->StopTimerMutex();

					//从用户实体列表里剔除
					//ache::GetInstance()->EraseEntity(CServer::g_CliSocketArr[i]);

					//从事件监听系统剔除
					//iraEvent::GetInstance()->RemoverListenner(CServer::g_CliSocketArr[i]);

					//数组中最后一个套接字挪到当前的位置上
					CServer::g_CliSocketArr[i] = CServer::g_CliSocketArr[--CServer::g_iTotalConn];
				}
				else
				{	//接收数据包异常后处理
					if (ret >= MSGSIZE)
					{
						Buffer[MSGSIZE - 1] = '\0';
					}
					else
					{
						Buffer[ret] = '\0';
					}

					//将收到的消息传递到用户实体的接收数据缓冲变量
					CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(CServer::g_CliSocketArr[i]);

					if (clientEntity != NULL)
					{
						strcpy_s(clientEntity->revBuffer, Buffer);

						clientEntity->revBufferSize = ret;

						//消息路由
						clientEntity->MsgDistribute();
					}
				}
			}
		}
	}

	return 0;
}

void CServer::StartCServer()
{
	//启动Socket环境
	WSAStartup(0x0202, &wsaData);

	//设置服务器地址信息
	memset(&CServerAddr, 0, sizeof(CServerAddr));

	CServerAddr.sin_family = AF_INET;
	CServerAddr.sin_addr.S_un.S_addr = inet_addr(CServerIP);
	CServerAddr.sin_port = htons(Port);

	//创建Socket
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//socket创建失败 清空socket环境数据
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "socket error" << endl;
		WSACleanup();
		return;
	}


	//绑定socket 到IP和端口
	if (bind(listenSocket, (struct sockaddr*) &CServerAddr, sizeof(CServerAddr)) == INVALID_SOCKET)
	{
		cout << "listenSocket error" << endl;
		WSACleanup();
		return;
	}

	//开始监听
	listen(listenSocket, 5);

	//初始化存放客户端信息
	memset(&clientAddr, 0, sizeof(clientAddr));
	
	setColor(14);
	cout <<  "Debug: CServer is start run，waiting connet..." << endl;
	setColor();

	// 创建工作线程
	CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);

	int len = sizeof(SOCKADDR);

	while (true)
	{
		if (CServer::g_bExit) break;

		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &len);
		
		if (CCache::GetInstance()->GetEntity(clientSocket) == NULL)
		{
			//每当一个新玩家成功连接时  创建一个玩家实体对象
			CClientEntity* ClientEntity = new CClientEntity(clientSocket, clientAddr);
			CCache::GetInstance()->SetEntity(clientSocket, ClientEntity);

			//将玩家加入事件监听系统
			CJiraEvent::GetInstance()->AddListenner(clientSocket, ClientEntity);

			setColor(14);
			printf_s("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			setColor();
		}
		else
		{
			//实体存在 就将掉线检测 线程停止
			CCache::GetInstance()->GetEntity(clientSocket)->setIsBroken(false);
			CCache::GetInstance()->GetEntity(clientSocket)->m_HasThread = false;
			CCache::GetInstance()->GetEntity(clientSocket)->StopTimerMutex();
		}

		if (clientSocket == INVALID_SOCKET)
		{
			setColor(4);
			cout << "Debug: Accept Failed!" << endl;
			setColor();
			continue;
		}

		//将收到的客户端套接字 存放到套接字数组g_CliSocketArr中
		g_CliSocketArr[g_iTotalConn++] = clientSocket;
	}
}

void CServer::DisconnectedFromCServer()
{
	if (clientSocket != NULL)
	{
		//关闭服务器端
		shutdown(clientSocket, SD_SEND);	//关闭客户端连接过来的套接字
		Sleep(1000);

		closesocket(clientSocket);			//关闭服务端连接
	}

	if (listenSocket != NULL)
	{
		closesocket(listenSocket);
	}

	WSACleanup();						//关闭服务器的套接字
}