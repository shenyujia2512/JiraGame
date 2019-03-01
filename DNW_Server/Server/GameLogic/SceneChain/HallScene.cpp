#include "HallScene.h"

CHallScene::CHallScene()
{

}

CHallScene::~CHallScene()
{

}

void CHallScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	hallLogic();
}

void CHallScene::hallLogic()
{
	//查询用户实体的sessionId 是否和客户端传来的sessionId一致   （校验客户端身份）
	if (CCache::GetInstance()->CheckSessionValid(clientSocket, RecMessage.hallrequest().sessionid()))
	{
		switch (RecMessage.hallrequest().msgtype())
		{
		case PBMessage::HST_AutoMatch:	 {					break; }
		case PBMessage::HST_CreateRoom:	 {	CreateRoom();   break; }
		case PBMessage::HST_OnlineFriend:{	OnlineFriend(); break; }
		case PBMessage::HST_RequestTeam: {	RequestTeam();  break; }
		case PBMessage::HST_JoinTeam:	 {	JoinTeam();		break; }
		case PBMessage::HST_JoinRoom:	 {	JoinRoom();		break; }
		case PBMessage::HST_GetRoomList: {	GetRoomList();	break; }
		case PBMessage::HST_LeaveRoom:	 {	LeaveRoom();	break; }
		case PBMessage::HST_GameWaite:	 {	GameWaite();	break; }
		case PBMessage::HST_GameStart:	 {	GameStart();	break; }
		default:break;
		}
	}
}

int CHallScene::GenrateOnlyRoomID()
{
	int _id = string2Int(GenrateRandomID());
	if (CCache::GetInstance()->CheckEntityRoomIsOnly(_id))
	{
		return _id;
	}

	GenrateOnlyRoomID();

	return 0;
}

void CHallScene::CreateRoom()
{
	if (RecMessage.hallrequest().roomname().length() <= 0)
	{
		return;
	}

	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();

	if (entityPlayerInfo->roomid() != 0)
	{
		return;
	}

	//生成唯一roomID
	int roomID = GenrateOnlyRoomID();

	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//创建room 服务器存储的(房间信息)
	PBMessage::RoomInfo* roomInfo = new PBMessage::RoomInfo();

	//设置房间id 名称 总人数和需要多少人 房间内玩家信息
	roomInfo->set_roomid(roomID);
	roomInfo->set_roomname(RecMessage.hallrequest().roomname());
	roomInfo->set_personcount(6);
	roomInfo->set_freepersoncount(5);

	//返回客户端玩家信息  (昵称 选择的角色 队伍 队伍ID)
	PBMessage::PlayerInfo* playerInfo = roomInfo->add_playerinfo();
	playerInfo->set_roleid(entityPlayerInfo->roleid());
	playerInfo->set_isroomctl(true);
	playerInfo->set_socket(clientSocket);
	playerInfo->set_name(entityPlayerInfo->name());
	playerInfo->set_roletype(entityPlayerInfo->roletype());
	playerInfo->set_team(PBMessage::Team_A);
	playerInfo->set_teamid(0);
	playerInfo->set_roomstate(PBMessage::RT_JoinRoom);

	//服务器ClientEntity上记录了玩家 (玩家ID 昵称 选择的角色 所在房间 队伍 队伍IDa)
	CCache::GetInstance()->CreateRoom(roomID, roomInfo);

	entityPlayerInfo->set_isroomctl(true);
	entityPlayerInfo->set_roomid(roomID);
	entityPlayerInfo->set_team(PBMessage::Team_A);
	entityPlayerInfo->set_teamid(0);
	entityPlayerInfo->set_roomstate(PBMessage::RT_JoinRoom);


	
	//传回客户端的数据
	hallReturn.set_roomname(RecMessage.hallrequest().roomname());
	hallReturn.set_roomid(roomID);
	hallReturn.set_msgtype(PBMessage::HST_CreateRoom);

	PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();
	_playerInfo->set_isroomctl(true);
	_playerInfo->set_roleid(playerInfo->roleid());
	_playerInfo->set_name(playerInfo->name());
	_playerInfo->set_roletype(playerInfo->roletype());
	_playerInfo->set_roomstate(playerInfo->roomstate());



	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallReturn.clear_friendlist();
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

void CHallScene::OnlineFriend()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//遍历所有在线玩家
	for (auto element : CCache::GetInstance()->GetEntitys())
	{
		//掉线玩家
		if (element.second->getIsBroken()) continue;

		PBMessage::PlayerInfo* roleInfo = element.second->getRoleInfo();

		if (roleInfo->roleid() == RecMessage.hallrequest().userid()) continue;

		PBMessage::PlayerInfo*  playerInfo = hallReturn.add_friendlist();
		playerInfo->set_roleid(roleInfo->roleid());
		playerInfo->set_name(roleInfo->name());
		playerInfo->set_roomstate(roleInfo->roomstate());
	}

	hallReturn.set_msgtype(PBMessage::HST_OnlineFriend);
	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);


	free(data); data = NULL;
	packet.release_hallreturn();
	hallReturn.clear_friendlist();
	hallReturn.Clear();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

void CHallScene::RequestTeam()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//查找需要通知的客户端socket
	SOCKET notityClientSocket = CCache::GetInstance()->GetEntitySocket(RecMessage.hallrequest().inviteinfo().revuserid());

	PBMessage::InviteInfo* inviteInfo = new PBMessage::InviteInfo(RecMessage.hallrequest().inviteinfo());
	inviteInfo->set_requsername(CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo()->name());
	hallReturn.set_allocated_inviteinfo(inviteInfo);

	hallReturn.set_msgtype(PBMessage::HST_RequestTeam);
	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);


	free(data); data = NULL;
	hallReturn.release_inviteinfo();
	packet.release_hallreturn();
	hallReturn.Clear();
	packet.Clear();

	if (notityClientSocket)
	{
		sendMsg(notityClientSocket, sendBuffer, size);
	}
}

void CHallScene::JoinTeam()
{
	//记录加入房间是否成功
	bool JoinRoomState = true;

	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_JoinTeam);

	//检查玩家即将要加入的房间位置当前是否还是空闲状态
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(RecMessage.hallrequest().inviteinfo().roomid());

	if (roomInfo != NULL)
	{
		for (int i = 0; i < roomInfo->playerinfo().size(); i++)
		{
			if (roomInfo->playerinfo(i).team() == RecMessage.hallrequest().inviteinfo().team()
				&& roomInfo->playerinfo(i).teamid() == RecMessage.hallrequest().inviteinfo().teamid())
			{
				//加入房间失败
				JoinRoomState = false;
				hallReturn.clear_friendlist();
				hallReturn.Clear();
				break;
			}

			if (JoinRoomState)
			{
				//传回客户端的数据
				PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();

				_playerInfo->set_isroomctl(roomInfo->playerinfo(i).isroomctl());
				_playerInfo->set_roleid(roomInfo->playerinfo(i).roleid());
				_playerInfo->set_name(roomInfo->playerinfo(i).name());
				_playerInfo->set_roletype(roomInfo->playerinfo(i).roletype());
				_playerInfo->set_roomstate(roomInfo->playerinfo(i).roomstate());
				_playerInfo->set_team(roomInfo->playerinfo(i).team());
				_playerInfo->set_teamid(roomInfo->playerinfo(i).teamid());
			}
		}
	}

	//可以加入房间 就将自己的信息也传回客户端 并在服务器上对应房间加入自己
	if (JoinRoomState && roomInfo != NULL)
	{
		hallReturn.set_roomid(roomInfo->roomid());

		//更新房间有多少空闲位置
		roomInfo->set_freepersoncount(roomInfo->freepersoncount() - 1);

		//传回客户端房间名
		hallReturn.set_roomname(roomInfo->roomname());

		PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();
		PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
		entityPlayerInfo->set_roomstate(PBMessage::RT_JoinRoom);
		entityPlayerInfo->set_roomid(roomInfo->roomid());

		_playerInfo->set_isroomctl(false);
		_playerInfo->set_socket(clientSocket);
		_playerInfo->set_roleid(entityPlayerInfo->roleid());
		_playerInfo->set_name(entityPlayerInfo->name());
		_playerInfo->set_roletype(entityPlayerInfo->roletype());
		_playerInfo->set_roomstate(entityPlayerInfo->roomstate());
		_playerInfo->set_team(RecMessage.hallrequest().inviteinfo().team());
		_playerInfo->set_teamid(RecMessage.hallrequest().inviteinfo().teamid());

		entityPlayerInfo->set_team(_playerInfo->team());
		entityPlayerInfo->set_teamid(_playerInfo->teamid());

		//记录到房间容器中
		CCache::GetInstance()->EntityRoomAddMember(RecMessage.hallrequest().inviteinfo().roomid(), _playerInfo);
	}

	//返回给自己的客户端
	packet.set_allocated_hallreturn(&hallReturn);

	//记录除自己以外房间内其他在线玩家的socket
	vector<SOCKET> socketLst;
	socketLst.clear();

	for (int i = 0; i < roomInfo->playerinfo().size(); i++)
	{
		if (roomInfo->playerinfo(i).socket() == clientSocket) continue;

		socketLst.push_back(roomInfo->playerinfo(i).socket());
	}

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallReturn.release_roomname();
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	//广播给房间内的玩家
	for (auto socket : socketLst)
	{
		sendMsg(socket, sendBuffer, size);
	}

	socketLst.clear();

	sendMsg(clientSocket, sendBuffer, size);
}

void CHallScene::GetRoomList()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GetRoomList);
	PBMessage::HallInfo* hallInfo = new PBMessage::HallInfo();

	//遍历所有房间
	for (auto room : CCache::GetInstance()->GetEntityRooms())
	{
		int roomId = room.first;
		PBMessage::RoomInfo* roomInfo = room.second;

		PBMessage::RoomInfo* _roomInfo = hallInfo->add_roominfo();
		_roomInfo->set_roomid(roomId);
		_roomInfo->set_roomname(roomInfo->roomname());
		_roomInfo->set_personcount(roomInfo->personcount());
		_roomInfo->set_freepersoncount(roomInfo->freepersoncount());

		//将房间内队长的状态传回客户端 用来判断房间是否可以加入
		PBMessage::PlayerInfo* playerInfo = _roomInfo->add_playerinfo();
		playerInfo->set_roomstate(roomInfo->playerinfo(0).roomstate());
	}

	hallReturn.set_allocated_hallinfo(hallInfo);
	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallInfo->clear_roominfo();
	hallReturn.release_hallinfo();
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

void CHallScene::JoinRoom()
{
	//记录加入房间是否成功
	bool JoinRoomState = true;

	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_JoinTeam);

	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(RecMessage.hallrequest().inviteinfo().roomid());	//查找客户端请求的房间信息

	if (roomInfo != NULL)
	{
		//记录可以加入的队伍和位置id
		map<int, vector<int>> IdleTeam_ID;

		for (int i = 0; i < 2; i++)	//2个队伍
		{
			vector<int> pos;

			for (int j = 0; j < 3; j++) //每个队伍3个位置
			{
				pos.push_back(j);
			}

			IdleTeam_ID[i] = pos;
		}

		//查询房间内玩家所在队伍和ID位置 如果有符合条件的就从IdleTeam_ID 容器中移出 剩下的就是可以加入得队伍和位置信息
		int len = roomInfo->playerinfo().size();
		for (int i = 0; i < len; i++)
		{
			for (map<int, vector<int>>::iterator idleTeam = IdleTeam_ID.begin();
				idleTeam != IdleTeam_ID.end();
				idleTeam++)
			{
				if ((roomInfo->playerinfo(i).team() - 1) == idleTeam->first)
				{
					for (vector<int>::iterator idleTeamID = idleTeam->second.begin();
						idleTeamID != idleTeam->second.end();
						idleTeamID++)
					{
						if (roomInfo->playerinfo(i).teamid() == *idleTeamID)
						{

							idleTeam->second.erase(idleTeamID);

							idleTeamID = idleTeam->second.begin();

							if (idleTeamID == idleTeam->second.end())
							{
								break;
							}
						}
					}

					if (idleTeam->second.size() == 0)
					{
						IdleTeam_ID.erase(idleTeam);

						idleTeam = IdleTeam_ID.begin();


						if (idleTeam == IdleTeam_ID.end())
						{
							break;
						}
					}
				}
			}
		}

		for (int i = 0; i < len; i++)
		{
			auto entityInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
			if (roomInfo->playerinfo(i).roleid() == entityInfo->roleid()) //房间内已存在用户
			{
				//加入房间失败
				JoinRoomState = false;

				//当前玩家所在房间信息
				if (NULL != roomInfo)
				{
					PBMessage::RoomInfo* newRoomInfo = new PBMessage::RoomInfo();

					//设置房间id 名称 总人数和需要多少人 房间内玩家信息
					newRoomInfo->set_roomid(roomInfo->roomid());
					newRoomInfo->set_roomname(roomInfo->roomname());
					newRoomInfo->set_personcount(roomInfo->personcount());
					newRoomInfo->set_freepersoncount(roomInfo->freepersoncount() + 1);

					//最后一个玩家离开时 直接删除房间
					if (newRoomInfo->personcount() == newRoomInfo->freepersoncount())
					{
						CCache::GetInstance()->EntityRoomRemoveMember(roomInfo->roomid());
						entityInfo->set_roomid(0);
						entityInfo->set_roomstate(PBMessage::RT_Idle);
					}
					else
					{
						//标本选择一个队长
						bool leader = false;

						//打包 创建数据的结果给客户端
						PBMessage::Packet _packet;
						PBMessage::HallReturn _hallReturn;
						_packet.set_curscene(PBMessage::CS_WaitRoom);
						_hallReturn.set_msgtype(PBMessage::HST_LeaveRoom);

						//0 离开的对象
						PBMessage::PlayerInfo* pPlayerInfo = _hallReturn.add_friendlist();
						pPlayerInfo->CopyFrom(roomInfo->playerinfo(i));

						//忽略自己 添加房间内其他玩家信息到新的房间中
						bool isCaptain = false; //记录当前玩家是否是队长
						for (int i = 0; i < roomInfo->playerinfo().size(); i++)
						{
							if (roomInfo->playerinfo(i).roleid() == entityInfo->roleid())
							{
								if (roomInfo->playerinfo(i).isroomctl())
								{
									isCaptain = true;
								}
								continue;
							}

							PBMessage::PlayerInfo* _playerInfo = newRoomInfo->add_playerinfo();
							_playerInfo->CopyFrom(roomInfo->playerinfo(i));

							//如果离开的玩家是队长就重新 选一个队长
							if (isCaptain && leader == false)
							{
								leader = true;
								_playerInfo->set_isroomctl(true);	//更新房间信息此用户为新房主

								auto playerInfoEntity = CCache::GetInstance()->GetEntity(_playerInfo->socket());
								if (playerInfoEntity != nullptr)
								{
									PBMessage::PlayerInfo* newPlayerInfo = playerInfoEntity->getRoleInfo();
									if (newPlayerInfo != nullptr)
									{
										newPlayerInfo->set_isroomctl(true);	//更新次玩家实体对象信息为新房主
									}
								}

								//返回给客户端 
								//1 新房主
								PBMessage::PlayerInfo* pPlayerInfo = _hallReturn.add_friendlist();
								pPlayerInfo->CopyFrom(*_playerInfo);
							}
						}

						int roomID = roomInfo->roomid();
						CCache::GetInstance()->EntityRoomRemoveMember(roomID);	//先删除房间	
						CCache::GetInstance()->CreateRoom(roomID, newRoomInfo);	//新的房间信息记录到房间容器中
						_packet.set_allocated_hallreturn(&_hallReturn);

						int size = _packet.ByteSize();
						void* data = malloc(size);
						_packet.SerializePartialToArray(data, size);
						memcpy(sendBuffer, data, size);

						//告诉其他客户端离开的成员
						for (auto elment : newRoomInfo->playerinfo())
						{
							sendMsg(elment.socket(), sendBuffer, size);
						}

						
						free(data); data = NULL;
						_hallReturn.Clear();
						_packet.release_hallreturn();
						_packet.Clear();

						entityInfo->set_roomid(0);
						entityInfo->set_roomstate(PBMessage::RT_Idle);
					}
				}

				hallReturn.clear_friendlist();
				hallReturn.Clear();
				break;
			}

			//传回客户端的数据
			PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();
			_playerInfo->CopyFrom(roomInfo->playerinfo(i));
		}

		//房间已满
		if (IdleTeam_ID.size() == 0)
		{
			//加入房间失败
			JoinRoomState = false;
			hallReturn.clear_friendlist();
			hallReturn.Clear();
		}


		//可以加入房间 就将自己的信息也传回客户端 并在服务器上对应房间加入自己
		if (JoinRoomState)
		{
			hallReturn.set_roomid(roomInfo->roomid());
			roomInfo->set_freepersoncount(roomInfo->freepersoncount() - 1);	//更新房间有多少空闲位置
			hallReturn.set_roomname(roomInfo->roomname());	//传回客户端房间名

			PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();
			PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
			entityPlayerInfo->set_roomstate(PBMessage::RT_JoinRoom);
			entityPlayerInfo->set_roomid(roomInfo->roomid());

			_playerInfo->set_roomid(roomInfo->roomid());
			_playerInfo->set_isroomctl(false);
			_playerInfo->set_socket(clientSocket);
			_playerInfo->set_roleid(entityPlayerInfo->roleid());
			_playerInfo->set_name(entityPlayerInfo->name());
			_playerInfo->set_roletype(entityPlayerInfo->roletype());
			_playerInfo->set_roomstate(entityPlayerInfo->roomstate());

			//设置加入后玩家的队伍和位置
			for (map<int, vector<int>>::iterator idleTeam = IdleTeam_ID.begin();
				idleTeam != IdleTeam_ID.end();
				idleTeam++)
			{
				_playerInfo->set_team((PBMessage::TeamType)(idleTeam->first + 1));
				_playerInfo->set_teamid(idleTeam->second[0]);

				entityPlayerInfo->set_team((PBMessage::TeamType)(idleTeam->first + 1));
				entityPlayerInfo->set_teamid(idleTeam->second[0]);
				break;
			}


			//记录到房间容器中
			CCache::GetInstance()->EntityRoomAddMember(RecMessage.hallrequest().inviteinfo().roomid(), _playerInfo);

			//返回给自己的客户端
			packet.set_allocated_hallreturn(&hallReturn);

			int size = packet.ByteSize();
			void* data = malloc(size);
			packet.SerializePartialToArray(data, size);
			memcpy(sendBuffer, data, size);

			free(data); data = NULL;
			hallReturn.release_roomname();
			hallReturn.clear_friendlist();
			hallReturn.Clear();
			packet.release_hallreturn();
			packet.Clear();
			IdleTeam_ID.clear();

			//广播给房间内的玩家
			for (auto clientEntity : roomInfo->playerinfo())
			{
				sendMsg(clientEntity.socket(), sendBuffer, size);
			}
		}
	}
}

void CHallScene::LeaveRoom()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_LeaveRoom);
	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	//通知自己的客户端 客户端表现
	sendMsg(clientSocket, sendBuffer, size);

	PBMessage::PlayerInfo* playerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo(); //获取当前玩家房间信息
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(playerInfo->roomid());	   //查找当前玩家所在房间信息

	if (NULL != roomInfo)
	{
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

void CHallScene::GameWaite()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GameWaite);

	//返回给客户端表现 是准备状态，还是空闲状态
	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::PlayerInfo* playerInfo = hallReturn.add_friendlist();

	playerInfo->set_gamewaite(!entityPlayerInfo->gamewaite());
	entityPlayerInfo->set_gamewaite(playerInfo->gamewaite());

	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());
	for (auto elment : roomInfo->playerinfo())
	{
		if (elment.roleid() == entityPlayerInfo->roleid())
		{
			if (playerInfo->gamewaite())
			{
				playerInfo->set_roomstate(PBMessage::RT_Wait);
				playerInfo->set_team(elment.team());
				playerInfo->set_teamid(elment.teamid());
			}
			else
			{
				playerInfo->set_roomstate(PBMessage::RT_JoinRoom);
				playerInfo->set_team(elment.team());
				playerInfo->set_teamid(elment.teamid());
			}

			break;
		}
	}

	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	//通知房间内的所有客户端
	for (auto elment : roomInfo->playerinfo())
	{
		sendMsg(elment.socket(), sendBuffer, size);
	}
}

void CHallScene::GameStart()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GameStart);

	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());

	//记录房间内所有玩家是否都是准备状态
	bool allPlayerWaite = true;

	for (auto elment : roomInfo->playerinfo())
	{
		if (elment.roleid() == entityPlayerInfo->roleid()) continue;

		PBMessage::PlayerInfo* _entityPlayerInfo = CCache::GetInstance()->GetEntity(elment.socket())->getRoleInfo();

		if (_entityPlayerInfo)
		{
			if (!_entityPlayerInfo->gamewaite())
			{
				allPlayerWaite = false;
				break;
			}
		}
		else
		{
			allPlayerWaite = false;
			break;
		}
	}

	//都准备后  就将所有玩家信息发送给客户端（在进入副本地图之前）
	if (allPlayerWaite)
	{
		for (auto elment : roomInfo->playerinfo())
		{
			PBMessage::PlayerInfo* playerInfo = hallReturn.add_friendlist();
			playerInfo->CopyFrom(elment);
		}
	}

	packet.set_allocated_hallreturn(&hallReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);
	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	hallReturn.Clear();
	packet.release_hallreturn();
	packet.Clear();

	for (auto elment : roomInfo->playerinfo())
	{
		sendMsg(elment.socket(), sendBuffer, size);
	}
}
