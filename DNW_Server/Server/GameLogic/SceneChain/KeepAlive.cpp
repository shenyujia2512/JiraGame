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

		clientEntity->m_iSendMsgCount = 0;	//�������
		clientEntity->setIsBroken(false);		//���Ϊ����״̬

		//�߳��Ѵ��ھ�ֱ�ӷ��� 
		if (clientEntity->m_HasThread) return;

		CKeepAliveInfo* keepAliveInfo = new CKeepAliveInfo();
		keepAliveInfo->clientSocket = clientSocket;
		keepAliveInfo->RecMessage = RecMessage;

		keepAliveInfoMap[clientSocket] = keepAliveInfo;

		//���ö�ʱ��ʱ��ÿ1��ִ��
		clientEntity->SetTimerMutex(1, TimerFunc, keepAliveInfo);
	}
}


void CKeepAlive::TimerFunc(CSceneBase* self)
{
	if (self == nullptr) return;

	//��ͻ��˷������������Ϣ��û����Ӧʱ���ʵ���Ѷ���
	CClientEntity* clientEntity = CCache::GetInstance()->GetEntity(self->clientSocket);

	if (NULL == clientEntity) return;

	//����ڷ��������û� ����3�� �ʹӷ������޳�
	if (clientEntity->m_iSendMsgCount >= 3 && clientEntity->m_iSendMsgCount < 10)
	{
		//���߼�ִ��һ��
		if (!clientEntity->getIsBroken() &&
			self->RecMessage.keepalive().curscene() == PBMessage::CS_WaitRoom)
		{
			setColor(2);
			cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " �뿪���� " << endl;
			setColor();

			((CKeepAliveInfo*)self)->LeaveRoom();
		}

		clientEntity->setIsBroken(true);
	}
	//�������Ϸ�����������û� ����10�� �ͷ��͸������ͻ��� ����ҵ�����
	else if (clientEntity->m_iSendMsgCount >= 10 && clientEntity->m_iSendMsgCount < 30)
	{
		//���߼�ִ��һ��
		if (clientEntity->getIsBroken() &&
			self->RecMessage.keepalive().curscene() == PBMessage::CS_MainScene)
		{
			setColor(2);
			cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " MainScene �ѵ��� " << endl;
			setColor();

			//�������������Ϣ
			PBMessage::PlayerInfo* entityPlayerInfo = CCache::GetInstance()->GetEntity(self->clientSocket)->getRoleInfo();
			PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(entityPlayerInfo->roomid());	//��ȡ������Ϣ

			if (roomInfo != nullptr)
			{
				//��� �������ݵĽ�����ͻ���
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
	//�������3���� ��ɾ��ʵ�����
	else if (clientEntity->m_iSendMsgCount >= 30)
	{
		clientEntity->setIsBroken(true);

		setColor(2);
		cout << clientEntity->getIpAddress() << ": " << clientEntity->getPort() << " ɾ����ʵ����� " << endl;
		setColor();

		//���û�ʵ���б����޳�
		CCache::GetInstance()->EraseEntity(self->clientSocket);

		//���¼�����ϵͳ�޳�
		CJiraEvent::GetInstance()->RemoverListenner(self->clientSocket);


		CKeepAliveInfo* pkeepAlive = ((CKeepAliveInfo*)self);
		delete(pkeepAlive);
		pkeepAlive = nullptr;

		//ֹͣ��ʱ���߳�
		clientEntity->StopTimerMutex();

		return;
	}

	//���ظ��ͻ������������Ϣ
	if (self->RecMessage.keepalive().curscene() == PBMessage::CS_WaitRoom)
	{
		//��� �������ݵĽ�����ͻ���
		PBMessage::Packet packet;
		PBMessage::HallReturn hallReturn;
		packet.set_curscene(PBMessage::CS_WaitRoom);
		hallReturn.set_msgtype(PBMessage::HST_KeepAlive);
		packet.set_allocated_hallreturn(&hallReturn);

		//���ظ��Լ��Ŀͻ���
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
		//��� �������ݵĽ�����ͻ���
		PBMessage::Packet packet;
		PBMessage::GameMainReturn gameMainReturn;
		packet.set_curscene(PBMessage::CS_MainScene);
		gameMainReturn.set_msgtype(PBMessage::GMT_KeepAlive);
		packet.set_allocated_gamemainreturn(&gameMainReturn);

		//���ظ��Լ��Ŀͻ���
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
	PBMessage::PlayerInfo* playerInfo = CCache::GetInstance()->GetEntity(clientSocket)->getRoleInfo(); //��ȡ��ǰ��ҷ�����Ϣ

	if (playerInfo != nullptr)
	{
		PBMessage::RoomInfo* roomInfo = CCache::GetInstance()->FindEntityRoom(playerInfo->roomid());	   //���ҵ�ǰ������ڷ�����Ϣ

		if (NULL != roomInfo)
		{
			PBMessage::Packet packet;
			PBMessage::HallReturn hallReturn;
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
}
