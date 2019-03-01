#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"


class CLoginScene :public CSceneBase, public Singleton<CLoginScene>
{
public:
	CLoginScene();
	~CLoginScene();

	
	//���߼����
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void LoginLogic();
};