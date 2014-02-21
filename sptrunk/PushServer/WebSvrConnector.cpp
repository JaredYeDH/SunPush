#include "CommonHeader.h"
#include "WebSvrConnector.h"

#include "MsgHandler.h"
#include "MsgStruct_CS.h"



extern int g_iWebServerConID;

WebSvrConnector::WebSvrConnector()
{

}

WebSvrConnector::~WebSvrConnector()
{

}

void WebSvrConnector::OnConnect( int connindex, int nErrCode )
{
	g_iWebServerConID = connindex;
	//MSG_S2C_SVR_READY_CMD readyPacket;
	//memcpy_s( readyPacket.szServerInfo, INFO_MAX_LEN, "TIOBE�ڽ������ڹ�����2013��7��TIOBE �������ָ�����а񡣴˴�ǰ������Ȼ��C ���ԡ�Java��Objective-C������Objective-C��Ȼ�����������ơ�", INFO_MAX_LEN );
	//readyPacket.dwEncKey = 6543;
	//printf("Sending Client Message:[CL_AUTH][CL_AUTH_S2C_READY] ...");
	//::Send( connindex, (char*)&readyPacket, sizeof(MSG_S2C_SVR_READY_CMD) );
}

void WebSvrConnector::OnSend( int connindex, int send_count )
{

}

void WebSvrConnector::OnClose( int connindex, const char* szErrDesc )
{
	volatile int i = 6;
}

void WebSvrConnector::OnReadComplete( int connindex, CNetMsgPtr msgptr, int length )
{
	MSG_BASE* pMsgBase = (MSG_BASE*)msgptr->GetBuf();
	if( pMsgBase->m_byCategory != WS_DATA )
	{
		return;
	}
	g_CSPackHander.ParsePacket_WS( connindex, (MSG_BASE *)msgptr->GetBuf(), length );
}
