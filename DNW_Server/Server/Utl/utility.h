#pragma once
#include <iostream>
#include <cmath>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <WinSock2.h>
#include <windows.h>
#include <conio.h>
#include <codecvt>
#include "../../include/protoFiles/command.txt.pb.h"

using namespace std;
#pragma comment(lib,"ws2_32.lib")

//��Ϣ���ܴ�С
#define MSGSIZE 8192


//���������
//#define CServerInfo "CServer Version: CharRoom CServer v1.0"
#define CServerIP   "0.0.0.0"
#define Port 9000

