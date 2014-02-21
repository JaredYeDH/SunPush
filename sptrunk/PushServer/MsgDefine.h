#ifndef __PROTOCOL_CS_H__
#define __PROTOCOL_CS_H__


/*   ���ṹ�������� Jake.Sun
1) ����(Request)						_SYN
2) ����Ӧ��(Answer)					_ACK
3) ֪ͨ(Command)						-CMD
4) �㲥(Broadcasting)				_BRD
*/


// --��ϢЭ��
#define		CS_CATEGORY_LENGTH		40
#define		WS_CATEGORY_LENGTH		50
#define		DS_CATEGORY_LENGTH		100

enum
{
	CS_CATEGORY_BASE	=	10,
	WS_CATEGORY_BASE	=	CS_CATEGORY_BASE + CS_CATEGORY_LENGTH,
	DS_CATEGORY_BASE	=	WS_CATEGORY_BASE + WS_CATEGORY_LENGTH,
};


enum eCS_CATEGORY
{
	CS_LOGON	= CS_CATEGORY_BASE,			// --��½��Ϣ����
	CS_MAX,
};


// --��¼��֤
enum eCS_LOGON
{
	S2C_SVR_READY_CMD		= 0,			// --S2C:������׼��
	C2S_USER_AUTH_SYN		= 1,			// --C2S:��֤����
	S2C_USER_AUTH_ACK		= 2,			// --S2C:��֤�ظ�
	S2C_KICK_USER_CMD		= 4,			// --S2C:�߳��˺�

	C2S_TEST_ORDER_SYN		= 51,			// --C2S:�����µ�
	C2S_TEST_ORDER_ACK		= 54,			// --S2C:�����µ��ظ�
};


// --
enum eWS_CATEGORY
{
	WS_DATA	= WS_CATEGORY_BASE,				// --
	WS_MAX,
};


// --��¼��֤
enum eWS_AUTH
{
	W2S_WSCONNECTED_CMD	= 0,				// --W2S:Web����������֪ͨ
	S2W_PUSHSVRAUTH_SYN	= 1,				// --S2W:���ͷ�������֤����
	W2S_AUTH_RESULT_ACK	= 2,				// --W2S:���ͷ�������֤����
	W2S_NEWORDERBRD_CMD	= 4,				// --W2S:���������㲥����

	S2W_TEST_ORDER_SYN	= 52,				// --S2W:�����µ�
};


#pragma pack(push,1)

//-------------------------------------------------------------------------------------------------------
// --������
// --������� Packet,���а��ĸ���
struct MSG_BASE
{
	BYTE						m_byCategory;	// Э�����
	BYTE						m_byProtocol;
};


/* ����: MSG_SERVER_TYPE
 * ˵��: ���������½��Ϣ��ָʾ�˷��������ͣ��ɣģ����ƣ��������������Ϣ
 */
struct MSG_SERVER_TYPE : public MSG_BASE
{
	BYTE						m_byServerType;					// ����������
 	char						m_szServerName[32];				// ����������
 	UINT16						m_nChanleID;					// ������ID
	UINT16						m_nServerID;					// ������ID	С���ڱ���Ψһ
	UINT16						m_nMaxPlayerCount;				// ����������û�����
};


struct MSG_DBPROXY_RESULT : public MSG_BASE
{
	void *						m_pData;
};


#pragma pack(pop)




#endif // __PROTOCOL_CL_H__


