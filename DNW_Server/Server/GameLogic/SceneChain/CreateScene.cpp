#include "CreateScene.h"
#include <codecvt>

CCreateScene::CCreateScene()
{

}

CCreateScene::~CCreateScene()
{

}

void CCreateScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;


	CreateLogic();
}

void CCreateScene::CreateLogic()
{
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::CreateRoleReturn createRoleReturn;
	packet.set_curscene(PBMessage::CS_CreateRole);


	//��ѯ�û�ʵ���sessionId �Ƿ�Ϳͻ��˴�����sessionIdһ��   ��У��ͻ�����ݣ�
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.createrolerequest().sessionid()))
	{
		//�������ݿ�
		CDBServer::GetInstance()->Open("DNW.db");

		//���ݿ��ֶ�
		string key[9] = {
			"roleID",
			"roleType",
			"team",
			"name",
			"level",
			"hp",
			"mp",
			"attack",
			"moveSpeed"
		};

		//�ֶ�����
		string value[9] = {
			"INTEGER",
			"INTEGER",
			"INTEGER NULL",
			"varchar(32) UNIQUE",
			"INTEGER NULL",
			"FLOAT NULL",
			"FLOAT NULL",
			"FLOAT NULL",
			"FLOAT NULL",
		};

		//������ table if not exists
		CDBServer::GetInstance()->CreateTable("playerInfo", key, value, 9);//�����û��� ����������Զ����Դ��������

		//�����û�session ��ѯ�û�userInfo�� �� playerInfo ������PlayerInfoID
		sqlite3_stmt* stmt = CDBServer::GetInstance()->SqlExec(
			"select PlayerInfoID,name from userInfo where session_id='" 
			+ RecMessage.createrolerequest().sessionid() 
			+ "';"
		);

		if (sqlite3_step(stmt) == SQLITE_ROW
			&& RecMessage.createrolerequest().playerinfo().name() != "")
		{
			int PlayerInfoID = sqlite3_column_int(stmt, 0);
			string PlayerName = (char*)sqlite3_column_text(stmt, 1);

			CDBServer::GetInstance()->ReleasemStmt(stmt);

			//������ɫ
			string keys[3] = {
				"roleID",
				"roleType",
				"name"
			};

			//�ͻ��˴������ĺ�����char ��ҪתΪwchar   ���ݿ�Ҳ��char ���� �������ݿ�Ͳ���Ҫת����
			//wstring wPlayeName = utf8string2wstring(RecMessage.createrolerequest().playerinfo().name());
			//string PlayerName = wstring2string(wPlayeName,"");
			
			//��������֮ǰ�ж������Ƿ��Ѿ�����
			sqlite3_stmt* stmt2 = CDBServer::GetInstance()->SqlExec(
				"select PlayerInfoID from userInfo left join playerInfo on userInfo.PlayerInfoID=playerInfo.roleID where  playerInfo.roleType="
				+ int2String(RecMessage.createrolerequest().playerinfo().roletype())
				+ " and userInfo.name='"
				+ PlayerName
				+ "';"
			);

			//δ��ѯ������
			if (sqlite3_step(stmt2) != SQLITE_ROW)
			{
				string values[3] = {
					int2String(PlayerInfoID),
					int2String(RecMessage.createrolerequest().playerinfo().roletype()),
					"'" + RecMessage.createrolerequest().playerinfo().name() + "'"
				};

				CDBServer::GetInstance()->ReleasemStmt(stmt2);

				if (CDBServer::GetInstance()->Insert("playerInfo", keys, values, 3))
				{
					createRoleReturn.set_returntype(PBMessage::CRT_Success);

					//���ؽ�ɫ�б� �������name��ѯ  name���н����ݿ�����
					//CREATE INDEX userInfoIndex on userInfo(name);
					sqlite3_stmt* _stmt = CDBServer::GetInstance()->SqlExec(
						"select roleType,playerInfo.name from userInfo left join  playerInfo "
						"on playerInfo.roleID=userInfo.PlayerInfoID where userInfo.name='" + PlayerName + "';");

					while (sqlite3_step(_stmt) == SQLITE_ROW)
					{
						PBMessage::PlayerInfo* playerinfo = createRoleReturn.add_playerinfo();

						const unsigned char* _value = sqlite3_column_text(_stmt, 0);

						if (_value != NULL)
						{
							playerinfo->set_roletype(atoi((char*)_value));
						}

						_value = sqlite3_column_text(_stmt, 1);

						if (_value != NULL)
						{
							playerinfo->set_name((char*)_value);
						}
					}

					CDBServer::GetInstance()->ReleasemStmt(_stmt);
				}
				else
				{
					createRoleReturn.set_returntype(PBMessage::CRT_Fail);
				}
			}
			else
			{
				CDBServer::GetInstance()->ReleasemStmt(stmt2);
				createRoleReturn.set_returntype(PBMessage::CRT_Fail);
			}
		}
		else
		{
			CDBServer::GetInstance()->ReleasemStmt(stmt);
			createRoleReturn.set_returntype(PBMessage::CRT_Fail);
		}

		CDBServer::GetInstance()->Close();	//�ر����ݿ�����
	}
	else
	{
		createRoleReturn.set_returntype(PBMessage::CRT_Fail);
	}

	packet.set_allocated_createrolereturn(&createRoleReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);

	memcpy(sendBuffer, data, size);


	free(data); data = NULL;
	packet.release_createrolereturn();	
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}
