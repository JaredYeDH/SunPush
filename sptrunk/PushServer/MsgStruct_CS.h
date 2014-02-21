
#ifndef __PACKETSTRUCT_CL_H__
#define __PACKETSTRUCT_CL_H__

#include "GlobalDefine.h"
#include "MsgDefine.h"



#pragma pack(push,1)

// --PushServer --> Client
class MSG_S2C_SVR_READY_CMD : public MSG_BASE
{
public:
	MSG_S2C_SVR_READY_CMD()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = S2C_SVR_READY_CMD;
	}
	SHORT		sHighVer;					// --�߰汾��
	SHORT		sLowVer;					// --�Ͱ汾��
    int			iEncKey;					// --��Կ
	char		szAuthKey[MAX_AUTHSTR_LEN];	// --��֤��
};


// --Client --> PushServer����֤����
class MSG_C2S_USER_AUTH_SYN : public MSG_BASE
{
public:
	MSG_C2S_USER_AUTH_SYN()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = C2S_USER_AUTH_SYN;
	}
	char		szUserID[ID_MAX_LEN];		// --�ʺ�
	char		szPasswd[PASSWD_MAX_LEN];	// --����
};


// --PushServer --> Client����֤�ظ�
class MSG_S2C_USER_AUTH_ACK : public MSG_BASE
{
public:
	MSG_S2C_USER_AUTH_ACK()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = S2C_USER_AUTH_ACK;		
	}
	BYTE		byResult;
};


// --PushServer --> Client���߳�֪ͨ
class MSG_S2C_KICK_USER_CMD : public MSG_BASE
{
public:
	MSG_S2C_KICK_USER_CMD()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = S2C_KICK_USER_CMD;
	}
	UINT16				nErrorCode;
};


// --Client-->PushServer �������µ�
class MSG_C2S_TEST_ORDER_SYN : public MSG_BASE
{
public:
	MSG_C2S_TEST_ORDER_SYN()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = C2S_TEST_ORDER_SYN;
	}
	BYTE			byOrderType;
	int				iUserPosX;
	int				iUserPosY;
	int				iOrderNo;
	char			strPhone[MAX_PHONENO_LEN];
	char			strTest[168];
};


class MSG_C2S_TEST_ORDER_ACK : public MSG_BASE
{
public:
	MSG_C2S_TEST_ORDER_ACK()
	{
		m_byCategory = CS_LOGON;
		m_byProtocol = C2S_TEST_ORDER_ACK;
	}
	BYTE			byOrderType;
};


#pragma pack(pop)

#endif // __PACKETSTRUCT_CL_H__

