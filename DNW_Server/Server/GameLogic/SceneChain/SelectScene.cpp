#include "SelectScene.h"

CSelectScene::CSelectScene()
{

}

CSelectScene::~CSelectScene()
{

}

void CSelectScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	hallLogic();
}


void CSelectScene::hallLogic()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::SelectRoleReturn selectReturn;
	packet.set_curscene(PBMessage::CS_SelectRole);

	//查询用户实体的sessionId 是否和客户端传来的sessionId一致   （校验客户端身份）
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.selectrolerequest().sessionid()))
	{
		//服务器上记录用户选择的角色类型
		CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_roletype(RecMessage.selectrolerequest().roletype());

		//连接数据库
		CDBServer::GetInstance()->Open("DNW.db");

		sqlite3_stmt* stmt = CDBServer::GetInstance()->SqlExec(
			"select userID,playerInfo.name from userInfo left Join playerInfo on userInfo.PlayerInfoID=playerInfo.roleID where session_id='"
			+ RecMessage.selectrolerequest().sessionid() + "' and playerInfo.roleType='" + int2String(RecMessage.selectrolerequest().roletype())
			+ "';"
			);

		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			int userID = sqlite3_column_int(stmt, 0);
			selectReturn.set_userid(userID);

			string userName = (char*)sqlite3_column_text(stmt, 1);

			//服务器上记录用户ID 昵称 
			CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_roleid(userID);
			CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_name(userName);
		}

		CDBServer::GetInstance()->ReleasemStmt(stmt);
		CDBServer::GetInstance()->Close();	//关闭数据库连接

		packet.set_allocated_selectrolereturn(&selectReturn);

		int size = packet.ByteSize();
		void* data = malloc(size);
		packet.SerializePartialToArray(data, size);

		memcpy(sendBuffer, data, size);


		free(data); data = NULL;
		packet.release_selectrolereturn();
		packet.Clear();

		sendMsg(clientSocket, sendBuffer, size);
	}
}

