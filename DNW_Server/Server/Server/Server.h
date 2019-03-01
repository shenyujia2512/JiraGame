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
	//����������
	void StartCServer();

	//�̴߳�����
	friend DWORD WINAPI WorkerThread(LPVOID lpParam);

private:
	//�Ͽ���ͻ��˵���������
	void DisconnectedFromCServer();

public: 


private :
	WSADATA wsaData;			//��ʼ����������
	sockaddr_in CServerAddr;	//��������ַ��Ϣ
	sockaddr_in clientAddr;		//�ͻ��˵�ַ��Ϣ
	SOCKET listenSocket;		//�����׽���
	SOCKET clientSocket;		//�ͻ������ӹ������׽���

	HANDLE hMutex;
	char Buffer[MSGSIZE];		//���ݻ���

public:
	static bool g_bExit;					  //��ʾ�����˳�
	static UINT g_icheckTime;
	static int  g_iTotalConn;
	static SOCKET g_CliSocketArr[FD_SETSIZE]; //������ŵ�ǰ���л�Ŀͻ����׽��ֵ��׽�������
};