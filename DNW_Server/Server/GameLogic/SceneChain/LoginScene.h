#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"


class CLoginScene :public CSceneBase, public Singleton<CLoginScene>
{
public:
	CLoginScene();
	~CLoginScene();

	
	//Ö÷Âß¼­Èë¿Ú
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void LoginLogic();
};