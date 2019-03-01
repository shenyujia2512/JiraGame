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
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::SelectRoleReturn selectReturn;
	packet.set_curscene(PBMessage::CS_SelectRole);

	//��ѯ�û�ʵ���sessionId �Ƿ�Ϳͻ��˴�����sessionIdһ��   ��У��ͻ�����ݣ�
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.selectrolerequest().sessionid()))
	{
		//�������ϼ�¼�û�ѡ��Ľ�ɫ����
		CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_roletype(RecMessage.selectrolerequest().roletype());

		//�������ݿ�
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

			//�������ϼ�¼�û�ID �ǳ� 
			CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_roleid(userID);
			CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->set_name(userName);
		}

		CDBServer::GetInstance()->ReleasemStmt(stmt);
		CDBServer::GetInstance()->Close();	//�ر����ݿ�����

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

