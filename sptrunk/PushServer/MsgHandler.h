#ifndef _PAKET_HANDLER_H_
#define _PAKET_HANDLER_H_

#include <Utility/Singleton.h>
#include <Utility/FunctionMap.h>
#include "UserManager.h"

using namespace util;

struct MSG_BASE;
class PacketHandler : public util::Singleton<PacketHandler>
{
	typedef void (*fnHandler_CS)( USER_INFOMATION* pUserInfo, MSG_BASE *pMsg, WORD wSize );
	typedef void (*fnHandler_WS)( int nConnIndex, MSG_BASE *pMsg, WORD wSize );

public:
	PacketHandler();
	~PacketHandler();

	BOOL			ParsePacket_CS( USER_INFOMATION * pSession, MSG_BASE * pMsg, WORD wSize );
	BOOL			ParsePacket_WS( int nConnIndex, MSG_BASE *pMsg, WORD wSize );

private:
	BOOL			RegisterHandler_CS();
	BOOL			RegisterHandler_WS();

	BOOL			AddHandler_CS( BYTE category, BYTE protocol, fnHandler_CS fnHandler );
	BOOL			AddHandler_WS (BYTE category, BYTE protocol, fnHandler_WS fnHandler );

	struct FUNC_CS : public BASE_FUNC
	{
		fnHandler_CS	m_fnHandler;
	};

	struct FUNC_WS : public util::BASE_FUNC
	{
		fnHandler_WS	m_fnHandler;
	};
	util::FunctionMap		*m_pFuncMap_CS;
	util::FunctionMap		*m_pFuncMap_WS;

	/* ����: static void OnHandler_CL_AUTH_C2S_ASK_AUTH( PLAYER_INFOMATION * pPlayerInfo, MSG_BASE * pMsg, WORD wSize );
	 * ˵��: ��ҵ�½��Ϸ�������ĵ�һ����Ϣ����֤�û��������룬����Ϣ�лὫ����ת����DBPROXY�н���������֤
	 */
	static void Handler_C2S_USER_AUTH_SYN ( USER_INFOMATION * pUserInfo, MSG_BASE * pMsg, WORD wSize );
	static void Handler_C2S_TEST_ORDER_SYN( USER_INFOMATION * pUserInfo, MSG_BASE * pMsg, WORD wSize );

	static void Handler_W2S_WSCONNECTED_CMD( int nConnIndex, MSG_BASE * pMsg, WORD wSize );
	static void Handler_W2S_AUTH_RESULT_ACK( int nConnIndex, MSG_BASE * pMsg, WORD wSize );
	static void Handler_W2S_NEWORDERBRD_CMD( int nConnIndex, MSG_BASE * pMsg, WORD wSize );
};

extern PacketHandler g_CSPackHander;
#endif