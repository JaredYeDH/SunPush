#pragma once

#include "Socket.h"
#include "IOPacket.h"
#include "MessageEncrypt.h"
#include "NetMsg.h"

enum SOCKET_STATUS
{
	SS_IDLE,
	SS_CONNECTING,
	SS_CONNECTED,
	SS_ACCEPTING,
	SS_CLOSED,
	SS_ERROR,
};

class Session;
class SessionManager
{
	friend void	StartSocketService(int thread_num);
public:
	static SessionManager* GetInstance();
	static void Release();

	Session*	AllocSession();										//���ڴ�ط���һ���Ự���� 
	void		DestroySession(Session* pSession);					//���ڴ���ͷ�һ���Ự����

	void		UpdateSession(long nConnectID, Session* pSession);
	void		RemoveSession(long nConnectID);
	void		RemoveAllSession();									//�ǳ����˳���Ҫ����
	Session*	GetSession(long nConnectID);						//���̰߳�ȫ, ֻ���ڵ�ǰ�Ự��ʹ��, �����ڴ�ض����ַ

	UINT32		GetSessionCount();
	UINT32		GetSessionColSize();
	UINT32		GetMessageColSize();

private:
	SessionManager();
	~SessionManager();

	static SessionManager* s_instance;

	boost::object_pool<Session>		m_poolSession;					//�Ự�ڴ��
	std::map<long, Session*>		m_mapSession;					//ͨ������ID�����Ự����

	boost::mutex m_mutex;
};

class Session
{
	friend class Acceptor;
	friend class Connector;
	friend class SessionManager;
//	friend class boost::object_pool;
public:
	Session();
	virtual ~Session();

	Session(Session &);
	Session& operator =(Session &);
	//////////////////////////////////////////////////////////////////////////
	//				�������ֻ������һЩ���ݶ��⿪�� 

	const char*		GetLocalIP();
	UINT16			GetLocalPort();
	const char*		GetRemoteIP();
	UINT16			GetRemotePort();
	SOCKET_STATUS	GetSocketStatus();
#ifdef _USE_PACKET_MEMORY_POOL_SEND
	void			Send(CNetMsgPtr spMsg, int length);
#else
	void			Send(const char* data, int length);
#endif
	void			Close(long nWaitSeconds);
protected:

	void handle_send(const boost::system::error_code& error);
	void handle_read_sp(CNetMsgPtr spMsg, const boost::system::error_code& error, size_t bytes_transferred);
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
#ifdef _USE_PACKET_MEMORY_POOL_SEND
		void handle_write(const boost::system::error_code& error, CNetMsgPtr spMsg, size_t bytes_transferred);
#else
		void handle_write(const boost::system::error_code& error, const char* spMsg, size_t bytes_transferred);
#endif

	void handle_timeout(const boost::system::error_code& error);
	void handle_destory(const boost::system::error_code& error);
	void handle_exit(const TCHAR* szDesc);

private:
	void ChangeSocketStatus(SOCKET_STATUS status);
	void start();

	boost::asio::ip::tcp::socket socket_;    
	boost::asio::deadline_timer timer_;
	boost::asio::strand strand_;

#ifdef _BIG_BUFFER_MODE
	char data_[PACKET_BUFFER_SIZE];					//���ν��ջ�����
	IOPacket	read_buffer_;			//���ݶ�ȡ������
	IOPacket	write_buffer_;			//���ݷ��ͻ�����
#else
	long			m_nSmallBufferModeReadLeave;
	long			m_nSmallBufferModeReadOffset;
	void	_receive_sp(CNetMsgPtr spMsg, long nRecvOffset, long nRecvLength);
#endif

	static volatile long s_global_current_connexindex;	//��ǰ������ߵ�ID

	mutable			int	m_connindex;					//��ǰsocket id
	TCHAR			m_strLocalIP[16];					//���ض����IP��ַ
	UINT16			m_nLocalPort;
	TCHAR			m_strRemoteIP[16];						//Զ�̿ɼ���IP��ַ
	UINT16			m_nRemotePort;
	SOCKET_STATUS	m_emSocketStatus;			//��ǰsocket��״̬

	Acceptor*		m_pAcceptor;
	Connector*		m_pConnector;

	MessageEncrypt	m_encryptSend;
	MessageEncrypt	m_encryptRecv;
};
