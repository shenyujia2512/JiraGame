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
	DisconnectedFromCServer();   //�رշ���

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
	struct timeval tv = { 1, 0 };			//ָ���ȴ�ʱ��Ϊ1��
	char              Buffer[MSGSIZE];

	while (1)
	{
		if (CServer::g_bExit) break;

		FD_ZERO(&fdread);	//��fdread��ʼ���ɿռ���

		for (i = 0; i < CServer::g_iTotalConn; i++)
		{
			//�������׽��ֿͻ����׽������μ��뼯��fdread 
			FD_SET(CServer::g_CliSocketArr[i], &fdread);
		}

		if (fdread.fd_count >= FD_SETSIZE)
		{
			printf("���������Ѵ�����!\n");
			continue;
		}

		//�ж��׽������Ƿ�������ݿɶ� ��д����������ֻ����read event
		ret = select(0, &fdread, NULL, NULL, &tv);

		if (ret <= 0)
		{	
			//�ڵȴ�ʱ��û�н��յ�����
			continue;
		}

		for (i = 0; i < CServer::g_iTotalConn; i++)
		{
			//��g_CliSocketArr�пͻ����׽��ֽ��м�飬�ж��׽����Ƿ����ڼ�����
			//select���غ������޸�ÿ��fdset�ṹ��ɾ����Щ�����ڴ���I/O�������׽��־����
			if (FD_ISSET(CServer::g_CliSocketArr[i], &fdread))
			{
				
				//��g_CliSocketArr[i]��������
				ret = recv(CServer::g_CliSocketArr[i], Buffer, MSGSIZE, 0);

				//���յ����ݳ���Ϊ0�����߷���WSAECONNRESET�������ʾ�ͻ����׽��������ر�
				if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET))
				{	
					// �ͻ���socket�ر�
					setColor(4);
					printf_s("Client socket %d closed.\n", CServer::g_CliSocketArr[i]);
					setColor();

					//�������ж�Ӧ���׽������󶨵���Դ�ͷŵ���Ȼ������׽�������
					closesocket(CServer::g_CliSocketArr[i]);

					//�����пͻ��˷���������Ϣ
					CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(CServer::g_CliSocketArr[i]);
					if (clientEntity)
					{
						memcpy_s(clientEntity->sendBuffer, sizeof(Buffer), Buffer, MSGSIZE);
						//strcpy_s(clientEntity->sendBuffer, Buffer);
						clientEntity->Broadcast();
					}

					//SAFE_DELETE(clientEntity);

					//ֹͣ��ʱ���߳�
					//ientEntity->StopTimerMutex();

					//���û�ʵ���б����޳�
					//ache::GetInstance()->EraseEntity(CServer::g_CliSocketArr[i]);

					//���¼�����ϵͳ�޳�
					//iraEvent::GetInstance()->RemoverListenner(CServer::g_CliSocketArr[i]);

					//���������һ���׽���Ų����ǰ��λ����
					CServer::g_CliSocketArr[i] = CServer::g_CliSocketArr[--CServer::g_iTotalConn];
				}
				else
				{	//�������ݰ��쳣����
					if (ret >= MSGSIZE)
					{
						Buffer[MSGSIZE - 1] = '\0';
					}
					else
					{
						Buffer[ret] = '\0';
					}

					//���յ�����Ϣ���ݵ��û�ʵ��Ľ������ݻ������
					CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(CServer::g_CliSocketArr[i]);

					if (clientEntity != NULL)
					{
						strcpy_s(clientEntity->revBuffer, Buffer);

						clientEntity->revBufferSize = ret;

						//��Ϣ·��
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
	//����Socket����
	WSAStartup(0x0202, &wsaData);

	//���÷�������ַ��Ϣ
	memset(&CServerAddr, 0, sizeof(CServerAddr));

	CServerAddr.sin_family = AF_INET;
	CServerAddr.sin_addr.S_un.S_addr = inet_addr(CServerIP);
	CServerAddr.sin_port = htons(Port);

	//����Socket
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//socket����ʧ�� ���socket��������
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "socket error" << endl;
		WSACleanup();
		return;
	}


	//��socket ��IP�Ͷ˿�
	if (bind(listenSocket, (struct sockaddr*) &CServerAddr, sizeof(CServerAddr)) == INVALID_SOCKET)
	{
		cout << "listenSocket error" << endl;
		WSACleanup();
		return;
	}

	//��ʼ����
	listen(listenSocket, 5);

	//��ʼ����ſͻ�����Ϣ
	memset(&clientAddr, 0, sizeof(clientAddr));
	
	setColor(14);
	cout <<  "Debug: CServer is start run��waiting connet..." << endl;
	setColor();

	// ���������߳�
	CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);

	int len = sizeof(SOCKADDR);

	while (true)
	{
		if (CServer::g_bExit) break;

		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &len);
		
		if (CCache::GetInstance()->GetEntity(clientSocket) == NULL)
		{
			//ÿ��һ������ҳɹ�����ʱ  ����һ�����ʵ�����
			CClientEntity* ClientEntity = new CClientEntity(clientSocket, clientAddr);
			CCache::GetInstance()->SetEntity(clientSocket, ClientEntity);

			//����Ҽ����¼�����ϵͳ
			CJiraEvent::GetInstance()->AddListenner(clientSocket, ClientEntity);

			setColor(14);
			printf_s("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			setColor();
		}
		else
		{
			//ʵ����� �ͽ����߼�� �߳�ֹͣ
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

		//���յ��Ŀͻ����׽��� ��ŵ��׽�������g_CliSocketArr��
		g_CliSocketArr[g_iTotalConn++] = clientSocket;
	}
}

void CServer::DisconnectedFromCServer()
{
	if (clientSocket != NULL)
	{
		//�رշ�������
		shutdown(clientSocket, SD_SEND);	//�رտͻ������ӹ������׽���
		Sleep(1000);

		closesocket(clientSocket);			//�رշ��������
	}

	if (listenSocket != NULL)
	{
		closesocket(listenSocket);
	}

	WSACleanup();						//�رշ��������׽���
}