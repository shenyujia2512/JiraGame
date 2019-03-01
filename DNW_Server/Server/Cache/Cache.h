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
	//检测Session是否有效
	bool CheckSessionValid(SOCKET socket, string sessionID);

	//获取所有实体地址
	map<SOCKET, CClientEntity*> GetEntitys();

	//获取实体地址
	CClientEntity* GetEntity(SOCKET socket);

	SOCKET GetEntitySocket(int userID);

	//设置实体地址
	void SetEntity(SOCKET socket,CClientEntity* clientEntity);

	//销毁实体地址
	void EraseEntity(SOCKET socket);

	//实体是否存在
	bool EntityIsExist(SOCKET socket);

	/************************************************************************/
	/*                           m_mEntityRoom								*/
	/************************************************************************/
	//检测传参的ID是否在m_mEntityRoom 容器中不存在
	bool CheckEntityRoomIsOnly(int _id);

	//创建房间
	void CreateRoom(int id, PBMessage::RoomInfo* roomInfo);

	//查找房间
	PBMessage::RoomInfo* FindEntityRoom(int roomID);

	//获取房间容器
	map<int, PBMessage::RoomInfo*> GetEntityRooms();

	//增加成员
	void EntityRoomAddMember(int roomID, PBMessage::PlayerInfo* pPlayerInfo);

	//离开房间
	void EntityRoomRemoveMember(int roomID);

protected:
	map<int, PBMessage::RoomInfo*>	m_mEntityRoom;	  //所有房间信息
	map<SOCKET, CClientEntity*>		m_mCClientEntity; //所有客户端实体信息
};