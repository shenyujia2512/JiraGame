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
	//��ѯ�û�ʵ���sessionId �Ƿ�Ϳͻ��˴�����sessionIdһ��   ��У��ͻ�����ݣ�
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

	//����ΨһroomID
	int roomID = GenrateOnlyRoomID();

	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//����room �������洢��(������Ϣ)
	PBMessage::RoomInfo* roomInfo = new PBMessage::RoomInfo();

	//���÷���id ���� ����������Ҫ������ �����������Ϣ
	roomInfo->set_roomid(roomID);
	roomInfo->set_roomname(RecMessage.hallrequest().roomname());
	roomInfo->set_personcount(6);
	roomInfo->set_freepersoncount(5);

	//���ؿͻ��������Ϣ  (�ǳ� ѡ��Ľ�ɫ ���� ����ID)
	PBMessage::PlayerInfo* playerInfo = roomInfo->add_playerinfo();
	playerInfo->set_roleid(entityPlayerInfo->roleid());
	playerInfo->set_isroomctl(true);
	playerInfo->set_socket(clientSocket);
	playerInfo->set_name(entityPlayerInfo->name());
	playerInfo->set_roletype(entityPlayerInfo->roletype());
	playerInfo->set_team(PBMessage::Team_A);
	playerInfo->set_teamid(0);
	playerInfo->set_roomstate(PBMessage::RT_JoinRoom);

	//������ClientEntity�ϼ�¼����� (���ID �ǳ� ѡ��Ľ�ɫ ���ڷ��� ���� ����IDa)
	CCache::GetInstance()->CreateRoom(roomID, roomInfo);

	entityPlayerInfo->set_isroomctl(true);
	entityPlayerInfo->set_roomid(roomID);
	entityPlayerInfo->set_team(PBMessage::Team_A);
	entityPlayerInfo->set_teamid(0);
	entityPlayerInfo->set_roomstate(PBMessage::RT_JoinRoom);


	
	//���ؿͻ��˵�����
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
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//���������������
	for (auto element : CCache::GetInstance()->GetEntitys())
	{
		//�������
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
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);

	//������Ҫ֪ͨ�Ŀͻ���socket
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
	//��¼���뷿���Ƿ�ɹ�
	bool JoinRoomState = true;

	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_JoinTeam);

	//�����Ҽ���Ҫ����ķ���λ�õ�ǰ�Ƿ��ǿ���״̬
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(RecMessage.hallrequest().inviteinfo().roomid());

	if (roomInfo != NULL)
	{
		for (int i = 0; i < roomInfo->playerinfo().size(); i++)
		{
			if (roomInfo->playerinfo(i).team() == RecMessage.hallrequest().inviteinfo().team()
				&& roomInfo->playerinfo(i).teamid() == RecMessage.hallrequest().inviteinfo().teamid())
			{
				//���뷿��ʧ��
				JoinRoomState = false;
				hallReturn.clear_friendlist();
				hallReturn.Clear();
				break;
			}

			if (JoinRoomState)
			{
				//���ؿͻ��˵�����
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

	//���Լ��뷿�� �ͽ��Լ�����ϢҲ���ؿͻ��� ���ڷ������϶�Ӧ��������Լ�
	if (JoinRoomState && roomInfo != NULL)
	{
		hallReturn.set_roomid(roomInfo->roomid());

		//���·����ж��ٿ���λ��
		roomInfo->set_freepersoncount(roomInfo->freepersoncount() - 1);

		//���ؿͻ��˷�����
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

		//��¼������������
		CCache::GetInstance()->EntityRoomAddMember(RecMessage.hallrequest().inviteinfo().roomid(), _playerInfo);
	}

	//���ظ��Լ��Ŀͻ���
	packet.set_allocated_hallreturn(&hallReturn);

	//��¼���Լ����ⷿ��������������ҵ�socket
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

	//�㲥�������ڵ����
	for (auto socket : socketLst)
	{
		sendMsg(socket, sendBuffer, size);
	}

	socketLst.clear();

	sendMsg(clientSocket, sendBuffer, size);
}

void CHallScene::GetRoomList()
{
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GetRoomList);
	PBMessage::HallInfo* hallInfo = new PBMessage::HallInfo();

	//�������з���
	for (auto room : CCache::GetInstance()->GetEntityRooms())
	{
		int roomId = room.first;
		PBMessage::RoomInfo* roomInfo = room.second;

		PBMessage::RoomInfo* _roomInfo = hallInfo->add_roominfo();
		_roomInfo->set_roomid(roomId);
		_roomInfo->set_roomname(roomInfo->roomname());
		_roomInfo->set_personcount(roomInfo->personcount());
		_roomInfo->set_freepersoncount(roomInfo->freepersoncount());

		//�������ڶӳ���״̬���ؿͻ��� �����жϷ����Ƿ���Լ���
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
	//��¼���뷿���Ƿ�ɹ�
	bool JoinRoomState = true;

	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_JoinTeam);

	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(RecMessage.hallrequest().inviteinfo().roomid());	//���ҿͻ�������ķ�����Ϣ

	if (roomInfo != NULL)
	{
		//��¼���Լ���Ķ����λ��id
		map<int, vector<int>> IdleTeam_ID;

		for (int i = 0; i < 2; i++)	//2������
		{
			vector<int> pos;

			for (int j = 0; j < 3; j++) //ÿ������3��λ��
			{
				pos.push_back(j);
			}

			IdleTeam_ID[i] = pos;
		}

		//��ѯ������������ڶ����IDλ�� ����з��������ľʹ�IdleTeam_ID �������Ƴ� ʣ�µľ��ǿ��Լ���ö����λ����Ϣ
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
			if (roomInfo->playerinfo(i).roleid() == entityInfo->roleid()) //�������Ѵ����û�
			{
				//���뷿��ʧ��
				JoinRoomState = false;

				//��ǰ������ڷ�����Ϣ
				if (NULL != roomInfo)
				{
					PBMessage::RoomInfo* newRoomInfo = new PBMessage::RoomInfo();

					//���÷���id ���� ����������Ҫ������ �����������Ϣ
					newRoomInfo->set_roomid(roomInfo->roomid());
					newRoomInfo->set_roomname(roomInfo->roomname());
					newRoomInfo->set_personcount(roomInfo->personcount());
					newRoomInfo->set_freepersoncount(roomInfo->freepersoncount() + 1);

					//���һ������뿪ʱ ֱ��ɾ������
					if (newRoomInfo->personcount() == newRoomInfo->freepersoncount())
					{
						CCache::GetInstance()->EntityRoomRemoveMember(roomInfo->roomid());
						entityInfo->set_roomid(0);
						entityInfo->set_roomstate(PBMessage::RT_Idle);
					}
					else
					{
						//�걾ѡ��һ���ӳ�
						bool leader = false;

						//��� �������ݵĽ�����ͻ���
						PBMessage::Packet _packet;
						PBMessage::HallReturn _hallReturn;
						_packet.set_curscene(PBMessage::CS_WaitRoom);
						_hallReturn.set_msgtype(PBMessage::HST_LeaveRoom);

						//0 �뿪�Ķ���
						PBMessage::PlayerInfo* pPlayerInfo = _hallReturn.add_friendlist();
						pPlayerInfo->CopyFrom(roomInfo->playerinfo(i));

						//�����Լ� ��ӷ��������������Ϣ���µķ�����
						bool isCaptain = false; //��¼��ǰ����Ƿ��Ƕӳ�
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

							//����뿪������Ƕӳ������� ѡһ���ӳ�
							if (isCaptain && leader == false)
							{
								leader = true;
								_playerInfo->set_isroomctl(true);	//���·�����Ϣ���û�Ϊ�·���

								auto playerInfoEntity = CCache::GetInstance()->GetEntity(_playerInfo->socket());
								if (playerInfoEntity != nullptr)
								{
									PBMessage::PlayerInfo* newPlayerInfo = playerInfoEntity->getRoleInfo();
									if (newPlayerInfo != nullptr)
									{
										newPlayerInfo->set_isroomctl(true);	//���´����ʵ�������ϢΪ�·���
									}
								}

								//���ظ��ͻ��� 
								//1 �·���
								PBMessage::PlayerInfo* pPlayerInfo = _hallReturn.add_friendlist();
								pPlayerInfo->CopyFrom(*_playerInfo);
							}
						}

						int roomID = roomInfo->roomid();
						CCache::GetInstance()->EntityRoomRemoveMember(roomID);	//��ɾ������	
						CCache::GetInstance()->CreateRoom(roomID, newRoomInfo);	//�µķ�����Ϣ��¼������������
						_packet.set_allocated_hallreturn(&_hallReturn);

						int size = _packet.ByteSize();
						void* data = malloc(size);
						_packet.SerializePartialToArray(data, size);
						memcpy(sendBuffer, data, size);

						//���������ͻ����뿪�ĳ�Ա
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

			//���ؿͻ��˵�����
			PBMessage::PlayerInfo* _playerInfo = hallReturn.add_friendlist();
			_playerInfo->CopyFrom(roomInfo->playerinfo(i));
		}

		//��������
		if (IdleTeam_ID.size() == 0)
		{
			//���뷿��ʧ��
			JoinRoomState = false;
			hallReturn.clear_friendlist();
			hallReturn.Clear();
		}


		//���Լ��뷿�� �ͽ��Լ�����ϢҲ���ؿͻ��� ���ڷ������϶�Ӧ��������Լ�
		if (JoinRoomState)
		{
			hallReturn.set_roomid(roomInfo->roomid());
			roomInfo->set_freepersoncount(roomInfo->freepersoncount() - 1);	//���·����ж��ٿ���λ��
			hallReturn.set_roomname(roomInfo->roomname());	//���ؿͻ��˷�����

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

			//���ü������ҵĶ����λ��
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


			//��¼������������
			CCache::GetInstance()->EntityRoomAddMember(RecMessage.hallrequest().inviteinfo().roomid(), _playerInfo);

			//���ظ��Լ��Ŀͻ���
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

			//�㲥�������ڵ����
			for (auto clientEntity : roomInfo->playerinfo())
			{
				sendMsg(clientEntity.socket(), sendBuffer, size);
			}
		}
	}
}

void CHallScene::LeaveRoom()
{
	//��� �������ݵĽ�����ͻ���
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

	//֪ͨ�Լ��Ŀͻ��� �ͻ��˱���
	sendMsg(clientSocket, sendBuffer, size);

	PBMessage::PlayerInfo* playerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo(); //��ȡ��ǰ��ҷ�����Ϣ
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(playerInfo->roomid());	   //���ҵ�ǰ������ڷ�����Ϣ

	if (NULL != roomInfo)
	{
		PBMessage::RoomInfo* newRoomInfo = new PBMessage::RoomInfo();

		//���÷���id ���� ����������Ҫ������ �����������Ϣ
		newRoomInfo->set_roomid(roomInfo->roomid());
		newRoomInfo->set_roomname(roomInfo->roomname());
		newRoomInfo->set_personcount(roomInfo->personcount());
		newRoomInfo->set_freepersoncount(roomInfo->freepersoncount() + 1);

		//���һ������뿪ʱ ֱ��ɾ������
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

			//���ظ��ͻ���2������Ԫ��  -->0 ��ʾ�뿪�Ķ��� 1��ʾ�·���
			//0 �뿪�Ķ���
			PBMessage::PlayerInfo* pPlayerInfo = hallReturn.add_friendlist();
			pPlayerInfo->set_roleid(playerInfo->roleid());
			pPlayerInfo->set_isroomctl(playerInfo->isroomctl());
			pPlayerInfo->set_team(playerInfo->team());
			pPlayerInfo->set_teamid(playerInfo->teamid());

			//�걾ѡ��һ���ӳ�
			bool leader = false;

			//�����Լ� ��ӷ��������������Ϣ���µķ�����
			for (int i = 0; i < roomInfo->playerinfo().size(); i++)
			{
				if (roomInfo->playerinfo(i).roleid() == playerInfo->roleid()) continue; 

				PBMessage::PlayerInfo* _playerInfo = newRoomInfo->add_playerinfo();
				_playerInfo->CopyFrom(roomInfo->playerinfo(i));

				//����뿪������Ƕӳ������� ѡһ���ӳ�
				if (playerInfo->isroomctl() && leader == false)
				{
					leader = true;
					_playerInfo->set_isroomctl(true);	//���·�����Ϣ���û�Ϊ�·���

					PBMessage::PlayerInfo* newPlayerInfo = CCache::GetInstance()->GetEntity(_playerInfo->socket())->getRoleInfo(); 
					newPlayerInfo->set_isroomctl(true);	//���´����ʵ�������ϢΪ�·���

					//���ظ��ͻ��� 
					//1 �·���
					PBMessage::PlayerInfo* pPlayerInfo = hallReturn.add_friendlist();
					pPlayerInfo->CopyFrom(*_playerInfo);
				}
			}

			int rootID = playerInfo->roomid();
			playerInfo->set_roomid(0);
			playerInfo->set_roomstate(PBMessage::RT_Idle);
			CCache::GetInstance()->EntityRoomRemoveMember(rootID);	//��ɾ������
			CCache::GetInstance()->CreateRoom(rootID, newRoomInfo);	//�µķ�����Ϣ��¼������������
			packet.set_allocated_hallreturn(&hallReturn);

			int size = packet.ByteSize();
			void* data = malloc(size);
			packet.SerializePartialToArray(data, size);
			memcpy(sendBuffer, data, size);

			//���������ͻ����뿪�ĳ�Ա
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
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GameWaite);

	//���ظ��ͻ��˱��� ��׼��״̬�����ǿ���״̬
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

	//֪ͨ�����ڵ����пͻ���
	for (auto elment : roomInfo->playerinfo())
	{
		sendMsg(elment.socket(), sendBuffer, size);
	}
}

void CHallScene::GameStart()
{
	//��� �������ݵĽ�����ͻ���
	PBMessage::Packet packet;
	PBMessage::HallReturn hallReturn;
	packet.set_curscene(PBMessage::CS_WaitRoom);
	hallReturn.set_msgtype(PBMessage::HST_GameStart);

	PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo();
	PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());

	//��¼��������������Ƿ���׼��״̬
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

	//��׼����  �ͽ����������Ϣ���͸��ͻ��ˣ��ڽ��븱����ͼ֮ǰ��
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
