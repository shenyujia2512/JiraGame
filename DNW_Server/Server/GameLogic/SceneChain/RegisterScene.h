#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"
class CDBServer;

class CRegisterScene :public CSceneBase, public Singleton<CRegisterScene>
{
public:
	CRegisterScene();
	~CRegisterScene();

	//���߼����
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void RegisterLogic();
};