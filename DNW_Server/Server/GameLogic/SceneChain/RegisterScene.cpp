#include "RegisterScene.h"

CRegisterScene::CRegisterScene()
{

}

CRegisterScene::~CRegisterScene()
{

}

void CRegisterScene::Start(SOCKET socket, PBMessage::UnPacket unPacket)
{
	clientSocket = socket;
	RecMessage = unPacket;

	if (clientSocket)
	{
		RegisterLogic();
	}
}

void CRegisterScene::RegisterLogic()
{
	//打包 创建数据的结果给客户端
	PBMessage::Packet packet;
	PBMessage::RegisterReturn registerReturn;
	packet.set_curscene(PBMessage::CS_Register);

	//创建DNW数据库
	CDBServer::GetInstance()->Open("DNW.db");

	//数据库字段
	string key[5] = {
		"userID",
		"PlayerInfoID",
		"session_id",
		"name",
		"password"
	};

	//字段类型
	string value[5] = {
		"INTEGER PRIMARY KEY AUTOINCREMENT",
		"INTEGER NULL",
		"varchar(32) NULL",
		"varchar(32) UNIQUE",
		"varchar(16)"
	};

	//创建表 table if not exists
	CDBServer::GetInstance()->CreateTable("userInfo", key, value, 5);//创建用户表 如果存在则自动忽略创建表操作

	//创建用户
	string keys[2] = {
		"name",
		"password"
	};
	
	string values[2] = {
		"'" + RecMessage.registerrequest().username() + "'",
		"'" + RecMessage.registerrequest().password() + "'"
	};

	//写入数据成功 -> 通知客户端注册成功 ，否则通知客户端失败
	sqlite3_stmt* stmt = CDBServer::GetInstance()->SqlExec(
		"select * from userInfo where name='" 
		+ RecMessage.registerrequest().username()
		+ "';");

	if (stmt && sqlite3_step(stmt) != SQLITE_ROW)
	{
		if (CDBServer::GetInstance()->Insert("userInfo", keys, values, 2))
		{
			registerReturn.set_returntype(PBMessage::RT_Success);
		}
		else
		{
			registerReturn.set_returntype(PBMessage::RT_Fail);
		}
	}
	else
	{
		registerReturn.set_returntype(PBMessage::RT_Fail);
	}
	
	//防止锁表导致update数据失败
	CDBServer::GetInstance()->ReleasemStmt(stmt);

	//将关联PlayerInfo表的id 和 userInfo.PlayerInfoID自增ID关联 
	sqlite3_stmt* _stmt = CDBServer::GetInstance()->SqlExec(
		"update userInfo set PlayerInfoID=userID where name='"
		+ RecMessage.registerrequest().username()
		+ "';");

	//执行语句
	sqlite3_step(_stmt);

	//防止锁表
	CDBServer::GetInstance()->ReleasemStmt(_stmt);

	CDBServer::GetInstance()->Close();	//关闭数据库连接


	packet.set_allocated_registerreturn(&registerReturn);

	int size = packet.ByteSize();
	void* data = malloc(size);
	packet.SerializePartialToArray(data, size);

	memcpy(sendBuffer, data, size);

	free(data); data = NULL;
	packet.release_registerreturn();
	packet.Clear();

	sendMsg(clientSocket, sendBuffer, size);
}

