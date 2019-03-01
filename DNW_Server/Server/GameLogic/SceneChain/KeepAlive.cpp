#include "KeepAlive.h"

CKeepAlive::CKeepAlive()
{
	keepAliveInfoMap.clear();
}

CKeepAlive::~CKeepAlive()
{
	keepAliveInfoMap.clear();
}

void CKeepAlive::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(clientSocket);
	if (NULL != clientEntity)
	{
		if (RecMessage.keepalive().curscene() == PBMessage::CS_WaitRoom)
		{
			if (keepAliveInfoMap.find(clientSocket) != keepAliveInfoMap.end())
			{
				keepAliveInfoMap[clientSocket]->RecMessage = RecMessage;
			}
		}
		else if(RecMessage.keepalive().curscene() == PBMessage::CS_MainScene)
		{
			if (keepAliveInfoMap.find(clientSocket) != keepAliveInfoMap.end())
			{
				keepAliveInfoMap[clientSocket]->RecMessage = RecMessage;
			}
		}

		clientEntity->m_iSendMsgCount = 0;	//清理计数
		clientEntity->setIsBroken(false);		//标记为在线状态

		//线程已存在就直接返回 
		if (clientEntity->m_HasThread) return;

		CKeepAliveInfo* keepAliveInfo = new CKeepAliveInfo();
		keepAliveInfo->clientSocket = clientSocket;
		keepAliveInfo->RecMessage = RecMessage;

		keepAliveInfoMap[clientSocket] = keepAliveInfo;

		//设置定时器时间每1秒执行
		clientEntity->SetTimerMutex(1, TimerFunc, keepAliveInfo);
	}
}


void CKeepAlive::TimerFunc(CSceneBase* self)
{
	if (self == nullptr) return;

	//向客户端发送心跳检测消息还没有响应时标记实体已断线
	CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(self->clientSocket);

	if (NULL == clientEntity) return;

	//如果在房间中有用户 断线3秒 就从房间中剔除
	if (clientEntity->m_iSendMsgCount >= 3 && clientEntity->m_iSendMsgCount < 10)
	{
		//此逻辑执行一次
		if (!clientEntity->getIsBroken() &&
			self->RecMessage.keepalive().curscene() == PBMessage::CS_WaitRoom)
		{
			setColor(2);
			cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " 离开房间 " << endl;
			setColor();

			((CKeepAliveInfo*)self)->LeaveRoom();
		}

		clientEntity->setIsBroken(true);
	}
	//如果在游戏主场景中有用户 断线10秒 就发送给其他客户端 此玩家掉线了
	else if (clientEntity->m_iSendMsgCount >= 10 && clientEntity->m_iSendMsgCount < 30)
	{
		//此逻辑执行一次
		if (clientEntity->getIsBroken() &&
			self->RecMessage.keepalive().curscene() == PBMessage::CS_MainScene)
		{
			setColor(2);
			cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " MainScene 已掉线 " << endl;
			setColor();

			//更新玩家属性信息
			PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(self->clientSocket)->getRoleInfo();
			PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());	//获取房间信息

			if (roomInfo != nullptr)
			{
				//打包 创建数据的结果给客户端
				PBMessage::Packet packet;
				PBMessage::GameMainReturn gameMainReturn;
				packet.set_curscene(PBMessage::CS_MainScene);
				gameMainReturn.set_msgtype(PBMessage::GMT_BreakLine);
				PBMessage::PlayerInfo* playerInfo = gameMainReturn.add_friendlist();
				playerInfo->set_name(entityPlayerInfo->name());

				packet.set_allocated_gamemainreturn(&gameMainReturn);

				int size = packet.ByteSize();
				void* data = malloc(size);
				packet.SerializePartialToArray(data, size);
				memcpy(self->sendBuffer, data, size);

				free(data); data = NULL;
				gameMainReturn.Clear();
				packet.release_gamemainreturn();
				packet.Clear();

				for (auto elment : roomInfo->playerinfo())
				{
					if (elment.socket() == self->clientSocket) continue;

					self->sendMsg(elment.socket(), self->sendBuffer, size);
				}
			}
		}

		clientEntity->setIsBroken(false);
	}
	//如果断线3分钟 就删除实体对象
	else if (clientEntity->m_iSendMsgCount >= 30)
	{
		clientEntity->setIsBroken(true);

		setColor(2);
		cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " 删除此实体对象 " << endl;
		setColor();

		//从用户实体列表里剔除
		CCache::GetInstance()->EraseEntity(self->clientSocket);

		//从事件监听系统剔除
		CJiraEvent::GetInstance()->RemoverListenner(self->clientSocket);


		CKeepAliveInfo* pkeepAlive = ((CKeepAliveInfo*)self);
		delete(pkeepAlive);
		pkeepAlive = nullptr;

		//停止定时器线程
		clientEntity->StopTimerMutex();

		return;
	}

	//返回给客户端心跳检测信息
	if (self->RecMessage.keepalive().curscene() == PBMessage::CS_WaitRoom)
	{
		//打包 创建数据的结果给客户端
		PBMessage::Packet packet;
		PBMessage::HallReturn hallReturn;
		packet.set_curscene(PBMessage::CS_WaitRoom);
		hallReturn.set_msgtype(PBMessage::HST_KeepAlive);
		packet.set_allocated_hallreturn(&hallReturn);

		//返回给自己的客户端
		int size = packet.ByteSize();
		void* data = malloc(size);
		packet.SerializePartialToArray(data, size);
		memcpy(self->sendBuffer, data, size);

		free(data); data = NULL;
		hallReturn.Clear();
		packet.release_hallreturn();
		packet.Clear();

		self->sendMsg(self->clientSocket, self->sendBuffer, size);
	}
	else if (self->RecMessage.keepalive().curscene() == PBMessage::CS_MainScene)
	{
		//打包 创建数据的结果给客户端
		PBMessage::Packet packet;
		PBMessage::GameMainReturn gameMainReturn;
		packet.set_curscene(PBMessage::CS_MainScene);
		gameMainReturn.set_msgtype(PBMessage::GMT_KeepAlive);
		packet.set_allocated_gamemainreturn(&gameMainReturn);

		//返回给自己的客户端
		int size = packet.ByteSize();
		void* data = malloc(size);
		packet.SerializePartialToArray(data, size);
		memcpy(self->sendBuffer, data, size);

		free(data); data = NULL;
		gameMainReturn.Clear();
		packet.release_gamemainreturn();
		packet.Clear();

		self->sendMsg(self->clientSocket, self->sendBuffer, size);
	}

	clientEntity->m_iSendMsgCount++;
}

void CKeepAliveInfo::LeaveRoom()
{
	PBMessage::PlayerInfo* playerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo(); //获取当前玩家房间信息

	if (playerInfo != nullptr)
	{
		PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(playerInfo->roomid());	   //查找当前玩家所在房间信息

		if (NULL != roomInfo)
		{
			PBMessage::Packet packet;
			PBMessage::HallReturn hallReturn;
			PBMessage::RoomInfo* newRoomInfo = new PBMessage::RoomInfo();

			//设置房间id 名称 总人数和需要多少人 房间内玩家信息
			newRoomInfo->set_roomid(roomInfo->roomid());
			newRoomInfo->set_roomname(roomInfo->roomname());
			newRoomInfo->set_personcount(roomInfo->personcount());
			newRoomInfo->set_freepersoncount(roomInfo->freepersoncount() + 1);

			//最后一个玩家离开时 直接删除房间
			if (newRoomInfo->personcount() == newRoomInfo->freepersoncount())
			{
				CCache::GetInstance()->EntityRoomRemoveMember(playerInfo->roomid());
				playerInfo->set_roomid(0);
				playerInfo->set_roomstate(PBMessage::RT_Idle);
			}
			else
			{
				packet.set_curscene(PBMessage::CS_WaitRoom);
				hallReturn.set_msgtype(PBMessage::HST_LeaveRoom);

				//返回给客户端2个数组元素  -->0 表示离开的对象 1表示新房主
				//0 离开的对象
				PBMessage::PlayerInfo* pPlayerInfo = hallReturn.add_friendlist();
				pPlayerInfo->set_roleid(playerInfo->roleid());
				pPlayerInfo->set_isroomctl(playerInfo->isroomctl());
				pPlayerInfo->set_team(playerInfo->team());
				pPlayerInfo->set_teamid(playerInfo->teamid());

				//标本选择一个队长
				bool leader = false;

				//忽略自己 添加房间内其他玩家信息到新的房间中
				for (int i = 0; i < roomInfo->playerinfo().size(); i++)
				{
					if (roomInfo->playerinfo(i).roleid() == playerInfo->roleid()) continue;

					PBMessage::PlayerInfo* _playerInfo = newRoomInfo->add_playerinfo();
					_playerInfo->CopyFrom(roomInfo->playerinfo(i));

					//如果离开的玩家是队长就重新 选一个队长
					if (playerInfo->isroomctl() && leader == false)
					{
						leader = true;
						_playerInfo->set_isroomctl(true);	//更新房间信息此用户为新房主

						PBMessage::PlayerInfo* newPlayerInfo = CCache::GetInstance()->GetEntity(_playerInfo->socket())->getRoleInfo();
						newPlayerInfo->set_isroomctl(true);	//更新次玩家实体对象信息为新房主

						//返回给客户端 
						//1 新房主
						PBMessage::PlayerInfo* pPlayerInfo = hallReturn.add_friendlist();
						pPlayerInfo->CopyFrom(*_playerInfo);
					}
				}

				int rootID = playerInfo->roomid();
				playerInfo->set_roomid(0);
				playerInfo->set_roomstate(PBMessage::RT_Idle);
				CCache::GetInstance()->EntityRoomRemoveMember(rootID);	//先删除房间
				CCache::GetInstance()->CreateRoom(rootID, newRoomInfo);	//新的房间信息记录到房间容器中
				packet.set_allocated_hallreturn(&hallReturn);

				int size = packet.ByteSize();
				void* data = malloc(size);
				packet.SerializePartialToArray(data, size);
				memcpy(sendBuffer, data, size);

				//告诉其他客户端离开的成员
				for (auto elment : newRoomInfo->playerinfo())
				{
					sendMsg(elment.socket(), sendBuffer, size);
				}

				free(data); data = NULL;
				hallReturn.Clear();
				packet.release_hallreturn();
				packet.Clear();
			}
		}
	}
}
