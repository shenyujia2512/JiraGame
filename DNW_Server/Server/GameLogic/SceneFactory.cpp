#include "SceneFactory.h"
#include "SceneChain/KeepAlive.h"
#include "SceneChain/LoginScene.h"
#include "SceneChain/MainScene.h"
#include "SceneChain/RegisterScene.h"
#include "SceneChain/CreateScene.h"
#include "SceneChain/SelectScene.h"
#include "SceneChain/HallScene.h"


CSceneFactory::CSceneFactory()
{

}

CSceneFactory::~CSceneFactory()
{
	Destory();
}

void CSceneFactory::ProcessMsgLogic(PBMessage::CurScene curScene, SOCKET clientSocket, PBMessage::UnPacket unPacket)
{
	switch (curScene)
	{
	case PBMessage::CS_None:break;
	case PBMessage::CS_KeepAlive: { CKeepAlive::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_Login:     { CLoginScene::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_CreateRole:{ CCreateScene::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_Register:  { CRegisterScene::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_SelectRole:{ CSelectScene::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_WaitRoom:  {	CHallScene::GetInstance()->Start(clientSocket, unPacket); break; }
	case PBMessage::CS_MainScene: { CMainScene::GetInstance()->Start(clientSocket, unPacket); break; }
	default:break;
	}
}

void CSceneFactory::Destory()
{
	CKeepAlive::DesInstance();
	CLoginScene::DesInstance();
	CCreateScene::DesInstance();
	CRegisterScene::DesInstance();
	CSelectScene::DesInstance();
	CHallScene::DesInstance();
	CMainScene::DesInstance();
}
