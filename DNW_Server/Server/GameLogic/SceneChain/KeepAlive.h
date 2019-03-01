#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"

struct CKeepAliveInfo :public CSceneBase
{
	void LeaveRoom();
};

class CKeepAlive :public CSceneBase, public Singleton<CKeepAlive>
{
public:
	CKeepAlive();
	~CKeepAlive();

	//Ö÷Âß¼­Èë¿Ú
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

protected:
	static void TimerFunc(CSceneBase* self);

	map<SOCKET, CKeepAliveInfo*> keepAliveInfoMap;
};