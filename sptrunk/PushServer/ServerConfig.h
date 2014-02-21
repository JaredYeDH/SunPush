#pragma once


enum eSVR_TYPE
{
	NULL_SERVER 			= 0,
	AGENT_SERVER			= 1,
	PUSH_SERVER				= 2,
	DATA_SERVER				= 3,
};


#define		DEFAULT_CHANNEL_UPDATE_TIMER		5					//Ĭ�϶�ʱˢ��Ƶ���б��ͻ��˵ļ��ʱ��(s) 
#define		DEFAULT_PCCU_UPDATE_TIMER			180					//Ĭ�϶�ʱ��¼PCCU�ļ��ʱ��(s) 

#define		DEFAULT_DBSVR_CONNECT_IP			"127.0.0.1"			//Ĭ������ DB���ݿ��������IP��ַ
#define		DEFAULT_DBSVR_CONNECT_PORT			19010				//Ĭ������ DB���ݿ�������Ķ˿�
#define		DEFAULT_WEBSVR_CONNECT_IP			"0.0.0.0"			//Ĭ�ϼ��� Web��������IP��ַ
#define		DEFAULT_WEBSVR_CONNECT_PORT			20120				//Ĭ�ϼ��� Web�������Ķ˿�
#define		DEFAULT_CLIENT_LISTEN_IP			"0.0.0.0"			//Ĭ�ϼ��� �ͻ��˵�IP��ַ
#define		DEFAULT_CLIENT_LISTEN_PORT			44405				//Ĭ�ϼ��� �ͻ��˵Ķ˿�


struct ServerConfig
{
public:
	ServerConfig()
	{
		memset(this, 0x0, sizeof(ServerConfig));
	}

	BOOL		LoadConfigFile( const char* lpszFileName = "PushServer.ini" );			// --���������ļ�

	char		m_szDBServerConnectIP[16];			// --���� DB����������IP��ַ
	char		m_szWebServerConnectIP[16];			// --���� Web��������IP��ַ
	char		m_szClientListenIP[16];				// --���� �ͻ��˵�IP��ַ
	UINT16		m_nDBServerConnectPort;				// --���� DB���ݿ�������Ķ˿�
	UINT16		m_nWebServerConnectPort;			// --���� Web�������Ķ˿�
	UINT16		m_nClientListenPort;				// --���� �ͻ��˵Ķ˿�
	int			m_nSocketThreadCnt;					// --	
	static		eSVR_TYPE	s_emServerType;			// --����������
};

extern ServerConfig	theConfig;