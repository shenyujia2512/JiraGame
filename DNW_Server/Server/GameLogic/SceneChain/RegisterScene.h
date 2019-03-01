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

	//Ö÷Âß¼­Èë¿Ú
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void RegisterLogic();
};