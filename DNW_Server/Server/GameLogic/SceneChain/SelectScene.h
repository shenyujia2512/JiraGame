#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"


class CSelectScene :public CSceneBase, public Singleton<CSelectScene>
{
public:
	CSelectScene();
	~CSelectScene();

	
	//���߼����
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void hallLogic();
};