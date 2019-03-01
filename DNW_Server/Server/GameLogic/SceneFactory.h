#pragma once
#include "../Singleton/Singleton.h"
#include "../Utl/Utilitys.h"

class CSceneFactory : public Singleton<CSceneFactory>
{
public:
	CSceneFactory();
	~CSceneFactory();

	void ProcessMsgLogic(PBMessage::CurScene curScene, SOCKET clientSocket, PBMessage::UnPacket unPacket);
	
	void Destory();
};

