#include "DBServer.h"

CDBServer::CDBServer()
{
	m_pSql = NULL;
}

bool CDBServer::Open(char* db)
{
	string dbPath = "DB/";
	dbPath += db;

	if (m_pSql != NULL)
	{
		return true;
	}

	if (sqlite3_open_v2(dbPath.c_str(),
		&m_pSql,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
		| SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,NULL) 
		== SQLITE_OK)
	{
		return true;
	}

	return false;
}

void CDBServer::Close()
{
	if (m_pSql)
	{
		sqlite3_close_v2(m_pSql);
		m_pSql = nullptr;
	}
}

void CDBServer::CreateTable(char* table, const string* key, const string* type, int count)
{
	sqlite3_stmt* _pStmt = NULL;

	string strSql = "";
	strSql = strSql + "create table if not exists " + table + "(";

	strSql = strSql + key[0] + " " + type[0];

	for (int i = 1; i < count; i++)
	{
		strSql = strSql + "," + key[i] + " " + type[i];
	}

	strSql += ");";


	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);
	
	if (res == SQLITE_OK)
	{
		//Ö´ÐÐÓï¾ä
		sqlite3_step(_pStmt);
	}

	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;
}

void CDBServer::DropTable(char* table)
{
	sqlite3_stmt* _pStmt = NULL;

	string strSql = "drop table ";
	strSql += table;

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		//Ö´ÐÐÓï¾ä
		sqlite3_step(_pStmt);
	}

	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;
}

bool CDBServer::Insert(char* table, const string* key, const string* value, int count)
{
	sqlite3_stmt* _pStmt = NULL;

	string strSql = "";
	strSql = strSql + "insert into " + table + "(";

	strSql = strSql + key[0];

	//key
	for (int i = 1; i < count; i++)
	{
		strSql = strSql + "," + key[i];
	}

	strSql += ") values(";

	//value
	strSql = strSql + value[0];

	for (int j = 1; j < count; j++)
	{
		strSql = strSql + "," + value[j];
	}
	strSql += ");";

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (_pStmt)
	{
		//Ö´ÐÐÓï¾ä
		int iResult = sqlite3_step(_pStmt);

		if (iResult == 101)
		{
			sqlite3_finalize(_pStmt);
			_pStmt = nullptr;

			return true;
		}
	}

	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;

	return false;
}

void CDBServer::Delete(char* table, const string* key, const string* value, int count)
{
	sqlite3_stmt* _pStmt;

	string strSql = "";
	strSql = strSql + "delete from " + table + " where ";

	strSql = strSql + key[0] + "=" + value[0];

	//key
	for (int i = 1; i < count; i++)
	{
		strSql = strSql + " and " + key[i] + "=" + value[i];
	}

	strSql += ";";

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		//Ö´ÐÐÓï¾ä
		sqlite3_step(_pStmt);
	}

	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;
}

void CDBServer::Update(char* table, const string* key, const string* value, const string* setKey, const string* setValue, int count, int setCount)
{
	sqlite3_stmt* _pStmt;

	string strSql = "";
	strSql = strSql + "update " + table + " set ";

	strSql = strSql + setKey[0] + "=" + setValue[0];

	for (int i = 1; i < count; i++)
	{
		strSql = strSql + "," + setKey[i] + "=" + setValue[i];
	}

	strSql += " where ";

	strSql = strSql + key[0] + "=" + value[0];

	for (int j = 1; j < count; j++)
	{
		strSql = strSql + "and " + key[j] + "=" + value[j];
	}

	strSql += ";";

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		//Ö´ÐÐÓï¾ä
		sqlite3_step(_pStmt);
	}

	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;
}

sqlite3_stmt* CDBServer::Select(char* table)
{
	sqlite3_stmt* _pStmt;

	string strSql = "select * from ";
	strSql += table;

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		return _pStmt;
	}

	return NULL;
}

sqlite3_stmt* CDBServer::SelectWhere(char* table, const string* getKey, const string* getValue, int count)
{
	sqlite3_stmt* _pStmt;

	string strSql = "";
	strSql = strSql + "select * from " + table + " where ";

	strSql = strSql + getKey[0] + "=" + getValue[0];

	for (int i = 1; i < count; i++)
	{
		strSql = strSql + " and " + getKey[i] + "=" + getValue[i];
	}

	strSql += ";";

	int res = sqlite3_prepare_v2(m_pSql, strSql.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		return _pStmt;
	}

	return NULL;
}

sqlite3_stmt* CDBServer::SqlExec(string _SQL)
{
	sqlite3_stmt* _pStmt;

	int res = sqlite3_prepare_v2(m_pSql, _SQL.c_str(), -1, &_pStmt, NULL);

	if (res == SQLITE_OK)
	{
		return _pStmt;
	}

	return NULL;
}

void CDBServer::ReleasemStmt(sqlite3_stmt* _pStmt)
{
	sqlite3_finalize(_pStmt);
	_pStmt = nullptr;
}
