#pragma once
#include "../ClientEntity/ClientEntity.h"
#include "../Singleton/Singleton.h"
#include "../Utl/utility.h"
class CClientEntity;

class CCache : public Singleton < CCache > 
{
public:
	CCache();
	virtual ~CCache();

	/************************************************************************/
	/*                           m_mCClientEntity							*/
	/************************************************************************/
	//���Session�Ƿ���Ч
	bool CheckSessionValid(SOCKET socket, string sessionID);

	//��ȡ����ʵ���ַ
	map<SOCKET, CClientEntity*> GetEntitys();

	//��ȡʵ���ַ
	CClientEntity* GetEntity(SOCKET socket);

	SOCKET GetEntitySocket(int userID);

	//����ʵ���ַ
	void SetEntity(SOCKET socket,CClientEntity* clientEntity);

	//����ʵ���ַ
	void EraseEntity(SOCKET socket);

	//ʵ���Ƿ����
	bool EntityIsExist(SOCKET socket);

	/************************************************************************/
	/*                           m_mEntityRoom								*/
	/************************************************************************/
	//��⴫�ε�ID�Ƿ���m_mEntityRoom �����в�����
	bool CheckEntityRoomIsOnly(int _id);

	//��������
	void CreateRoom(int id, PBMessage::RoomInfo* roomInfo);

	//���ҷ���
	PBMessage::RoomInfo* FindEntityRoom(int roomID);

	//��ȡ��������
	map<int, PBMessage::RoomInfo*> GetEntityRooms();

	//���ӳ�Ա
	void EntityRoomAddMember(int roomID, PBMessage::PlayerInfo* pPlayerInfo);

	//�뿪����
	void EntityRoomRemoveMember(int roomID);

protected:
	map<int, PBMessage::RoomInfo*>	m_mEntityRoom;	  //���з�����Ϣ
	map<SOCKET, CClientEntity*>		m_mCClientEntity; //���пͻ���ʵ����Ϣ
};