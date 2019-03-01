#include "LoginScene.h"

CLoginScene::CLoginScene()
{

}

CLoginScene::~CLoginScene()
{

}

void CLoginScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	LoginLogic();	
}


void CLoginScene::LoginLogic()
{
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::LoginReturn loginReturn;
	packet.set_curscene(PBMessage::CS_Login);


	//�������ݿ�
	CDBServer::GetInstance()->Open("DNW.db");

	string user[2] = {
		"name",
		"password"
	};

	string passwd[2] = {
		"'" + RecMessage.loginrequest().username() + "'",
		"'" + RecMessage.loginrequest().password() + "'"
	};

	//��ѯ���ݿ����Ƿ������������� �û��������� �ɹ���֪ͨ�ͻ��˵�¼�ɹ� ����֪ͨ�ͻ��˵�¼ʧ��
	sqlite3_stmt* stmt = CDBServer::GetInstance()->SelectWhere("userInfo", user, passwd, 2);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		//��¼�ɹ���д��session_id
		loginReturn.set_sessionid(RecMessage.loginrequest().username() + "_" + GenrateRandomID());

		if (CCache::GetInstance()->EntityIsExist(clientSocket))
		{
			CCache::GetInstance()->GetEntity(clientSocket)->SessionID = loginReturn.sessionid();
		}

		CDBServer::GetInstance()->Update(
			"userInfo",
			new string[1]{ "name"},
			new string[1]{ "'" + RecMessage.loginrequest().username() + "'", },
			new string[1]{ "session_id"}, 
			new string[1]{ "'" + loginReturn.sessionid() + "'" },
			1, 1);


		//��ѯ�Ƿ��Ѵ��ڽ�ɫ
		sqlite3_stmt* stmt = CDBServer::GetInstance()->SqlExec(
			"select roleType,playerInfo.name,userID from userInfo left join playerInfo on userInfo.PlayerInfoID=playerInfo.roleID where userInfo.name='"
			+ RecMessage.loginrequest().username() + "' and roleID=PlayerInfoID;");

		if (stmt != NULL)
		{
			bool b_flag = false;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				PBMessage::PlayerInfo* playerinfo = loginReturn.add_playerinfo();

				const unsigned char* _value = sqlite3_column_text(stmt, 0);

				if (_value != NULL)
				{
					playerinfo->set_roletype(atoi((char*)_value));
				}
	
				_value = sqlite3_column_text(stmt, 1);

				if (_value != NULL)
				{
					playerinfo->set_name((char*)_value);
				}

				_value = sqlite3_column_text(stmt, 2);

				if (_value != NULL)
				{
					playerinfo->set_roleid(atoi((char*)_value));
				}

				if (b_flag == false)
				{
					b_flag = true;
					loginReturn.set_returntype(PBMessage::ToSelectRole);
				}
			}

			if (b_flag == false)
			{
				loginReturn.set_returntype(PBMessage::ToCreateRole);
			}
		}
		else
		{
			loginReturn.set_returntype(PBMessage::ToCreateRole);
		}
	}
	else
	{
		loginReturn.set_returntype(PBMessage::Fail);
	}

	CDBServer::GetInstance()->ReleasemStmt(stmt);

	CDBServer::GetInstance()->Close();	//�ر����ݿ�����


	packet.set_allocated_loginreturn(&loginReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);

	memcpy(sendBuffer, data, size);

	
	free(data); data = NULL;
	packet.release_loginreturn();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

