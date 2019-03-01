#pragma once
#include "../SceneBase.h"
#include "../../DBServer/DBServer.h"
#include "../../Singleton/Singleton.h"
class CCache;
class CDBServer;

class CCreateScene :public CSceneBase, public Singleton<CCreateScene>
{
public: 
	CCreateScene();
	~CCreateScene();

	//Ö÷Âß¼­Èë¿Ú
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void CreateLogic();
};