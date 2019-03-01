/*
	Open("test.db");

	CreateTable(
		"student_info", 
		new string[3]{ "id", "name", "age" }, 
		new string[3]{ "int", "char", "short" },3);

	Delete(
		"student_info", 
		new string[1]{ "id"}, 
		new string[1]{ "1" }, 1);

	Insert(
		"student_info",
		new string[3]{ "id", "name", "age" }, 
		new string[3]{ "2", "'jiaw'", "19" }, 3);

	Update(
		"student_info",
		new string[1]{ "id"}, new string[1]{ "1" },
		new string[1]{ "age"}, new string[1]{ "12" }, 1, 1);

	Select("student_info");

	Close();
*/

#pragma once
#include "../Singleton/Singleton.h"
#include "../Utl/utility.h"
#include "../sqlite3.h"


class CDBServer :public Singleton<CDBServer>
{
public:
	CDBServer();

	//�����ݿ� �������ݿ�
	bool Open(char* db);

	//�ر�����
	void Close();

	/*
		���� -- ����������
		CreateTable(
		"student_info",
		new string[3]{ "id", "name", "age" }, 
		new string[3]{ "int", "char", "short" },
		3);
	*/
	void CreateTable(char* table, const string* key, const string* type, int count);

	//ɾ����
	void DropTable(char* table);

	/*
		����
		Insert(
		"student_info", 
		new string[3]{ "id", "name", "age" },
		new string[3]{ "1", "jira", "18" }, 
		3);
	*/
	bool Insert(char* table, const string* key, const string* value, int count);

	/*
		ɾ��
		Delete(
		"student_info", 
		new string[1]{ "id"},
		new string[1]{ "1" }, 
		1);
	*/
	void Delete(char* table, const string* key, const string* value, int count);

	/*
		����
		Update(
		"student_info", 
		new string[1]{ "id"}, new string[1]{ "1" },
		new string[1]{ "name"}, new string[1]{ "jia" },
		1,1);
	*/
	void Update(char* table, const string* key, const string* value, const string* setKey, const string* setValue, int count, int setCount);

	void ReleasemStmt(sqlite3_stmt* _pStmt);

	//��ѯ
	sqlite3_stmt* Select(char* table);

	sqlite3_stmt* SelectWhere(char* table, const string* getKey, const string* getValue, int count);

	sqlite3_stmt* SqlExec(string _SQL);

protected:
	sqlite3* m_pSql;	//���ݿ�ʵ��
};

