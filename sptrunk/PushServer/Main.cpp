// PushServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "CommonHeader.h"
#include "PushServer.h"
#include "NetThread.h"



int main( int argc, char* argv )
{
	printf( "PushServer��������...\n");
	PushServer m_PushServer;
	if( !m_PushServer.InitServer() )
	{
		printf( "PushServer Initialization Failed! \n" );
		system( "pause" );
		return 0;
	}

	printf( "Start Server Start Success! \n");
	m_PushServer.RunServer();

	// --��ʱ�ر�
	printf( "Server Is Terminated... \n" );
	m_PushServer.Release();
	NetThread::StopNetThread();
	return 0;
}

