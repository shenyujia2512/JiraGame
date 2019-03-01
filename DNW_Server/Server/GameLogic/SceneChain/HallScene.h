#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"


class CHallScene :public CSceneBase, public Singleton<CHallScene>
{
public:
	CHallScene();
	~CHallScene();

	
	//主逻辑入口
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

	int GenrateOnlyRoomID();

private:
	//消息分发
	void hallLogic();

	//创建房间消息处理
	void CreateRoom();

	//获取在线玩家
	void OnlineFriend();

	//请求组队
	void RequestTeam();

	//加入队伍
	void JoinTeam();

	//获取房间列表信息
	void GetRoomList();

	//加入房间
	void JoinRoom();

	//离开房间
	void LeaveRoom();

	//游戏准备
	void GameWaite();

	//游戏开始
	void GameStart();

protected:
	static void TimerFunc(CSceneBase* self);
};