#pragma once
#include "../../Singleton/Singleton.h"
#include "../../DBServer/DBServer.h"
#include "../SceneBase.h"


class CHallScene :public CSceneBase, public Singleton<CHallScene>
{
public:
	CHallScene();
	~CHallScene();

	
	//���߼����
	void Start(SOCKET socket, PBMessage::UnPacket unPacket);

	int GenrateOnlyRoomID();

private:
	//��Ϣ�ַ�
	void hallLogic();

	//����������Ϣ����
	void CreateRoom();

	//��ȡ�������
	void OnlineFriend();

	//�������
	void RequestTeam();

	//�������
	void JoinTeam();

	//��ȡ�����б���Ϣ
	void GetRoomList();

	//���뷿��
	void JoinRoom();

	//�뿪����
	void LeaveRoom();

	//��Ϸ׼��
	void GameWaite();

	//��Ϸ��ʼ
	void GameStart();

protected:
	static void TimerFunc(CSceneBase* self);
};