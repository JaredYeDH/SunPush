#pragma once

#include "Session.h"
#include "NetMsg.h"

class Connector
{
	friend class Session;
public:
	Connector();
	virtual ~Connector();
	//��ʼ�����˿�
	void	AsyncConnect(const char* szIP, UINT16 port);
	void	DisConnect();
	int		Send(const char* data, short length){
		assert(m_pConnectSession != NULL);
		return ::Send(m_pConnectSession->m_connindex, data, length);
	}
	UINT32	GetConnectIndex()
	{
		assert(m_pConnectSession != NULL);
		if (NULL == m_pConnectSession)	
			return 0;
		return m_pConnectSession->m_connindex;
	}

protected:
	virtual void OnConnect(int /*connindex*/, int /*err_code*/){}						//���µ�������������
	virtual void OnSend(int /*connindex*/, int /*send_count*/){}						//�Ѿ����ͳ�ȥ������
	virtual void OnReadComplete(int /*connindex*/, CNetMsgPtr /*msgptr*/, int /*length*/){}	//�ɹ���ȡ��һ��������Ϣ
	virtual void OnClose(int /*connindex*/, const TCHAR* /*szErrDesc*/){}				//һ�����ӹر���

private:
	void handle_connect(Session* new_session, const boost::system::error_code& error);
	void handle_timeout(Session* new_session, const boost::system::error_code& e);
	
	Session*	m_pConnectSession;
	UINT16		m_nListenPort;
};	