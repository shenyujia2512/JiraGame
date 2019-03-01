#include "RegisterScene.h"

CRegisterScene::CRegisterScene()
{

}

CRegisterScene::~CRegisterScene()
{

}

void CRegisterScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	if (clientSocket)
	{
		RegisterLogic();
	}
}

void CRegisterScene::RegisterLogic()
{
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::RegisterReturn registerReturn;
	packet.set_curscene(PBMessage::CS_Register);

	//����DNW���ݿ�
	CDBServer::GetInstance()->Open("DNW.db");

	//���ݿ��ֶ�
	string key[5] = {
		"userID",
		"PlayerInfoID",
		"session_id",
		"name",
		"password"
	};

	//�ֶ�����
	string value[5] = {
		"INTEGER PRIMARY KEY AUTOINCREMENT",
		"INTEGER NULL",
		"varchar(32) NULL",
		"varchar(32) UNIQUE",
		"varchar(16)"
	};

	//������ table if not exists
	CDBServer::GetInstance()->CreateTable("userInfo", key, value, 5);//�����û��� ����������Զ����Դ��������

	//�����û�
	string keys[2] = {
		"name",
		"password"
	};
	
	string values[2] = {
		"'" + RecMessage.registerrequest().username() + "'",
		"'" + RecMessage.registerrequest().password() + "'"
	};

	//д�����ݳɹ� -> ֪ͨ�ͻ���ע��ɹ� ������֪ͨ�ͻ���ʧ��
	sqlite3_stmt* stmt = CDBServer::GetInstance()->SqlExec(
		"select * from userInfo where name='" 
		+ RecMessage.registerrequest().username()
		+ "';");

	if (stmt && sqlite3_step(stmt) != SQLITE_ROW)
	{
		if (CDBServer::GetInstance()->Insert("userInfo", keys, values, 2))
		{
			registerReturn.set_returntype(PBMessage::RT_Success);
		}
		else
		{
			registerReturn.set_returntype(PBMessage::RT_Fail);
		}
	}
	else
	{
		registerReturn.set_returntype(PBMessage::RT_Fail);
	}
	
	//��ֹ������update����ʧ��
	CDBServer::GetInstance()->ReleasemStmt(stmt);

	//������PlayerInfo���id �� userInfo.PlayerInfoID����ID���� 
	sqlite3_stmt* _stmt = CDBServer::GetInstance()->SqlExec(
		"update userInfo set PlayerInfoID=userID where name='"
		+ RecMessage.registerrequest().username()
		+ "';");

	//ִ�����
	sqlite3_step(_stmt);

	//��ֹ����
	CDBServer::GetInstance()->ReleasemStmt(_stmt);

	CDBServer::GetInstance()->Close();	//�ر����ݿ�����


	packet.set_allocated_registerreturn(&registerReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);

	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	packet.release_registerreturn();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

