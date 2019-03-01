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
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::LoginReturn loginReturn;
	packet.set_curscene(PBMessage::CS_Login);


	//连接数据库
	CDBServer::GetInstance()->Open("DNW.db");

	string user[2] = {
		"name",
		"password"
	};

	string passwd[2] = {
		"'" + RecMessage.loginrequest().username() + "'",
		"'" + RecMessage.loginrequest().password() + "'"
	};

	//查询数据库中是否有满足条件的 用户名和密码 成功、通知客户端登录成功 否则、通知客户端登录失败
	sqlite3_stmt* stmt = CDBServer::GetInstance()->SelectWhere("userInfo", user, passwd, 2);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		//登录成功后写入session_id
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


		//查询是否已存在角色
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

	CDBServer::GetInstance()->Close();	//关闭数据库连接


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

