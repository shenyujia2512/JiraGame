#include "Cache.h"

CCache::CCache()
{
	m_mEntityRoom.clear();
	m_mCClientEntity.clear();
}

CCache::~CCache()
{
	for (auto clientEntity : m_mCClientEntity)
	{
		SAFE_DELETE(clientEntity.second);
	}

	for (auto entityRoom : m_mEntityRoom)
	{
		entityRoom.second->Clear();
		SAFE_DELETE(entityRoom.second);
	}

	m_mEntityRoom.clear();
	m_mCClientEntity.clear();
}

bool CCache::CheckEntityRoomIsOnly(int _id)
{
	map<int, PBMessage::RoomInfo*>::iterator _result = m_mEntityRoom.find(_id);
	if (_result == m_mEntityRoom.end())
	{
		//²»´æÔÚ
		return true;
	}

	return false;
}

bool CCache::CheckSessionValid(SOCKET socket, string sessionID)
{
	if (m_mCClientEntity.find(socket) != m_mCClientEntity.end())
	{
		if (m_mCClientEntity[socket]->SessionID == sessionID)
		{
			return true;
		}
	}

	return false;
}

CClientEntity* CCache::GetEntity(SOCKET socket)
{
	if (m_mCClientEntity.find(socket) != m_mCClientEntity.end())
	{
		return m_mCClientEntity[socket];
	}

	return NULL;
}

void CCache::EraseEntity(SOCKET socket)
{
	if (m_mCClientEntity.find(socket) != m_mCClientEntity.end())
	{
		m_mCClientEntity.erase(socket);
	}
}

void CCache::SetEntity(SOCKET socket, CClientEntity* clientEntity)
{
	if (m_mCClientEntity.find(socket) == m_mCClientEntity.end())
	{
		m_mCClientEntity[socket] = clientEntity;
	}
}

bool CCache::EntityIsExist(SOCKET socket)
{
	if (m_mCClientEntity.find(socket) != m_mCClientEntity.end())
	{
		return true;
	}

	return false;
}

map<SOCKET, CClientEntity*> CCache::GetEntitys()
{
	return m_mCClientEntity;
}

void CCache::CreateRoom(int id, PBMessage::RoomInfo* roomInfo)
{
	if (m_mEntityRoom.find(id) == m_mEntityRoom.end())
	{
		m_mEntityRoom[id] = roomInfo;
	}
}

PBMessage::RoomInfo* CCache::FindEntityRoom(int roomID)
{
	if (m_mEntityRoom.find(roomID) != m_mEntityRoom.end())
	{
		return m_mEntityRoom[roomID];
	}

	return NULL;
}

SOCKET CCache::GetEntitySocket(int userID)
{
	for (auto element : m_mCClientEntity)
	{
		auto a = element.second->getRoleInfo()->roleid();
		if (element.second->getRoleInfo()->roleid() == userID)
		{
			return element.second->sockClient;
		}
	}

	return NULL;
}

void CCache::EntityRoomAddMember(int roomID, PBMessage::PlayerInfo* pPlayerInfo)
{
	if (m_mEntityRoom.find(roomID) != m_mEntityRoom.end())
	{
		auto playerInfo = m_mEntityRoom[roomID]->add_playerinfo();
		playerInfo->CopyFrom(*pPlayerInfo);
	}
}

void CCache::EntityRoomRemoveMember(int roomID)
{
	if (m_mEntityRoom.find(roomID) != m_mEntityRoom.end())
	{
		SAFE_DELETE(m_mEntityRoom[roomID]);
		m_mEntityRoom.erase(roomID);
	}
}

map<int, PBMessage::RoomInfo*> CCache::GetEntityRooms()
{
	return m_mEntityRoom;
}
