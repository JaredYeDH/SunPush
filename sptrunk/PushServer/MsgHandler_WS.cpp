#include "CommonHeader.h"
#include "GlobalDefine.h"
#include "MsgHandler.h"
#include "MsgStruct_WS.h"
#include "MsgStruct_CS.h"
#include <AdvanceLib/Socket/Socket.h>


void PacketHandler::Handler_W2S_WSCONNECTED_CMD( int nConnIndex, MSG_BASE * pMsg, WORD wSize )
{
	// --Web���������ӳɹ�,����Web��������֤
	MSG_S2W_PUSHSVR_AUTH_SYN authPack;
	memcpy_s( authPack.szAuthKey, MAX_AUTHSTR_LEN, TO_WS_AUTH_STR, MAX_AUTHSTR_LEN );
	::Send( nConnIndex, (char*)&authPack, sizeof(MSG_S2W_PUSHSVR_AUTH_SYN) );
}


void PacketHandler::Handler_W2S_AUTH_RESULT_ACK( int nConnIndex, MSG_BASE * pMsg, WORD wSize )
{
	MSG_W2S_AUTH_RESULT_ACK* pRcvMsg = (MSG_W2S_AUTH_RESULT_ACK*)pMsg;
	if( 1 == pRcvMsg->byResult )
	{
		// --��֤ʧ��
		printf( "WebServer��֤�ɹ�! \n");
	}
	else
	{
		printf( "WebServer��֤ʧ��! \n");
	}
}


void PacketHandler::Handler_W2S_NEWORDERBRD_CMD( int nConnIndex, MSG_BASE * pMsg, WORD wSize )
{
	MSG_W2S_NEWORDERBRD_CMD* pRcvMsg = (MSG_W2S_NEWORDERBRD_CMD*)pMsg;
	printf( "�յ�WebServer���Ķ����㲥  �˿Ͷ���: %d \n", pRcvMsg->m_iOrder );

	MSG_C2S_TEST_ORDER_ACK ackMsg;
	ackMsg.byOrderType = 0;
	::Send( pRcvMsg->m_iUsrConID, (char*)&ackMsg, sizeof(MSG_C2S_TEST_ORDER_ACK) );
}