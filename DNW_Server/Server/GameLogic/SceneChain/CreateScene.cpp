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
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::CreateRoleReturn createRoleReturn;
	packet.set_curscene(PBMessage::CS_CreateRole);


	//查询用户实体的sessionId 是否和客户端传来的sessionId一致   （校验客户端身份）
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.createrolerequest().sessionid()))
	{
		//连接数据库
		CDBServer::GetInstance()->Open("DNW.db");

		//数据库字段
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

		//字段类型
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

		//创建表 table if not exists
		CDBServer::GetInstance()->CreateTable("playerInfo", key, value, 9);//创建用户表 如果存在则自动忽略创建表操作

		//根据用户session 查询用户userInfo表 与 playerInfo 关联的PlayerInfoID
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

			//创建角色
			string keys[3] = {
				"roleID",
				"roleType",
				"name"
			};

			//客户端传过来的汉字是char 需要转为wchar   数据库也是char 类型 存入数据库就不需要转化了
			//wstring wPlayeName = utf8string2wstring(RecMessage.createrolerequest().playerinfo().name());
			//string PlayerName = wstring2string(wPlayeName,"");
			
			//插入数据之前判断数据是否已经存在
			sqlite3_stmt* stmt2 = CDBServer::GetInstance()->SqlExec(
				"select PlayerInfoID from userInfo left join playerInfo on userInfo.PlayerInfoID=playerInfo.roleID where  playerInfo.roleType="
				+ int2String(RecMessage.createrolerequest().playerinfo().roletype())
				+ " and userInfo.name='"
				+ PlayerName
				+ "';"
			);

			//未查询到数据
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

					//返回角色列表 根据玩家name查询  name是有建数据库索引
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

		CDBServer::GetInstance()->Close();	//关闭数据库连接
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
