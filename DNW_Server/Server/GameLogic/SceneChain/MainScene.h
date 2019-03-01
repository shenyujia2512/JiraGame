#pragma once
#include "../../Singleton/Singleton.h"
#include "../SceneBase.h"


class CMainScene :public CSceneBase, public Singleton<CMainScene>
{
public:
	CMainScene();
	~CMainScene();

	//Ö÷Âß¼­Èë¿Ú
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

private:
	void MainLogic();

	void RsyncAttr();

	void RsyncStatus();

	void GameOver();
};