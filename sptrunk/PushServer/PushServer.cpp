#include "CommonHeader.h"
#include "PushServer.h"
#include "ServerConfig.h"
#include "NetThread.h"


int g_iWebServerConID = 0;

PushServer::PushServer()
{

}


PushServer::~PushServer()
{

}


bool PushServer::InitServer()
{
	//DISPMSG_NOTICE("Start", "Parsing Config File[LoginServer.ini]...");
	printf("��ȡ�����ļ�[LoginServer.ini]... \n");
	theConfig.LoadConfigFile();

	// --���Լ���ӵ���������������
	//ServerInfoManager::GetInstance()->AppendServer( theConfig.m_nChanleID, theConfig.m_nServerID, theConfig.m_szServerName, -1, LOGIN_SERVER, theConfig.m_nMaxPlayerCount);
	NetThread::StartNetThread( 4 );
	return TRUE;
}

void PushServer::RunServer()
{
	for (;;)
	{
		Sleep( 100 );
	}
}

void PushServer::Release()
{

}





