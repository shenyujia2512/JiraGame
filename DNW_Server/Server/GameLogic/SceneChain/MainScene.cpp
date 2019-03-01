#include "MainScene.h"

CMainScene::CMainScene(){}

CMainScene::~CMainScene(){}

void CMainScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	MainLogic();
}

void CMainScene::MainLogic()
{
	//查询用户实体的sessionId 是否和客户端传来的sessionId一致   （校验客户端身份）
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.gamemainrequest().sessionid()))
	{
		switch (RecMessage.gamemainrequest().msgtype())
		{
		case PBMessage::GMT_RsyncAttr:	{ RsyncAttr();		break; }
		case PBMessage::GMT_RsyncStatus:{ RsyncStatus();	break; }
		case PBMessage::GMT_GameOver:	{ GameOver();		break; }
		default:break;
		}
	}
}

void CMainScene::RsyncAttr()
{
	//更新玩家属性信息
	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());	//获取房间信息

	if (roomInfo != nullptr)
	{
		//打包 创建数据的结果给客户端
		PBMessage::Packet packet;
		PBMessage::GameMainReturn gameMainReturn;
		packet.set_curscene(PBMessage::CS_MainScene);
		gameMainReturn.set_msgtype(PBMessage::GMT_RsyncAttr);

		int roomID = entityPlayerInfo->roomid();
		int socket = entityPlayerInfo->socket();
		int userID = entityPlayerInfo->roleid();
		entityPlayerInfo->CopyFrom(RecMessage.gamemainrequest().playerinfo());
		entityPlayerInfo->set_roomid(roomID);
		entityPlayerInfo->set_socket(socket);
		entityPlayerInfo->set_roleid(userID);

		for (auto elment : roomInfo->playerinfo())
		{
			auto playerEntity = CCache::GetInstance()->GetEntity(elment.socket());
			if (playerEntity != nullptr)
			{
				PBMessage::PlayerInfo* _playerInfoEntity = playerEntity->getRoleInfo();
				if (_playerInfoEntity)
				{
					PBMessage::PlayerInfo* playerInfo = gameMainReturn.add_friendlist();
					playerInfo->CopyFrom(*_playerInfoEntity);
				}
			}
		}

		packet.set_allocated_gamemainreturn(&gameMainReturn);

		int size = packet.ByteSize();
		void* data = malloc(size);
		packet.SerializePartialToArray(data, size);
		memcpy(sendBuffer, data, size);

		free(data); data = NULL;
		gameMainReturn.Clear();
		packet.release_gamemainreturn();
		packet.Clear();

		for (auto elment : roomInfo->playerinfo())
		{
			sendMsg(elment.socket(), sendBuffer, size);
		}
	}
}

void CMainScene::RsyncStatus()
{
	//更新玩家属性信息
	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());	//获取房间信息

	if (roomInfo != nullptr)
	{
		//打包 创建数据的结果给客户端
		PBMessage::Packet packet;
		PBMessage::GameMainReturn gameMainReturn;
		packet.set_curscene(PBMessage::CS_MainScene);
		gameMainReturn.set_msgtype(PBMessage::GMT_RsyncStatus);

		int roomID = entityPlayerInfo->roomid();
		int socket = entityPlayerInfo->socket();
		int userID = entityPlayerInfo->roleid();
		entityPlayerInfo->CopyFrom(RecMessage.gamemainrequest().playerinfo());
		entityPlayerInfo->set_roomid(roomID);
		entityPlayerInfo->set_socket(socket);
		entityPlayerInfo->set_roleid(userID);

		for (auto elment : roomInfo->playerinfo())
		{
			auto playerEntity = CCache::GetInstance()->GetEntity(elment.socket());
			if (playerEntity != nullptr)
			{
				PBMessage::PlayerInfo* _playerInfoEntity = playerEntity->getRoleInfo();
				if (_playerInfoEntity)
				{
					PBMessage::PlayerInfo* playerInfo = gameMainReturn.add_friendlist();
					playerInfo->CopyFrom(*_playerInfoEntity);
				}
			}
		}

		packet.set_allocated_gamemainreturn(&gameMainReturn);

		int size = packet.ByteSize();
		void* data = malloc(size);
		packet.SerializePartialToArray(data, size);
		memcpy(sendBuffer, data, size);

		free(data); data = NULL;
		gameMainReturn.Clear();
		packet.release_gamemainreturn();
		packet.Clear();

		for (auto elment : roomInfo->playerinfo())
		{
			sendMsg(elment.socket(), sendBuffer, size);
		}
	}
}

void CMainScene::GameOver()
{
	//更新玩家属性信息
	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());	//获取房间信息
	int roomID = entityPlayerInfo->roomid();

	if (roomInfo != nullptr)
	{
		for (auto elment : roomInfo->playerinfo())
		{
			auto playerEntity = CCache::GetInstance()->GetEntity(elment.socket())->getRoleInfo();
			if (playerEntity != nullptr)
			{
				playerEntity->set_roomid(0);
			}
		}
	}

	CCache::GetInstance()->EntityRoomRemoveMember(roomID);	//删除房间
}
