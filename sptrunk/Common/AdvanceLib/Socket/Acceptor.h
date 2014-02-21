#pragma once

#include "Session.h"
#include "NetMsg.h"

class Acceptor
{
	friend class Session;
public:
	Acceptor(UINT16 port);
	virtual ~Acceptor();
	//��ʼ�����˿�
	BOOL	StartListen();
	void	StopListen();

	Acceptor(Acceptor &);
	Acceptor & operator = (Acceptor &);

protected:
	virtual void OnAccept(int /*connindex*/){}										//���µ�������������
	virtual void OnSend(int /*connindex*/, int /*send_count*/){}						//�Ѿ����ͳ�ȥ������
	virtual void OnReadComplete(int /*connindex*/, CNetMsgPtr /*msgptr*/, int /*length*/){}	//�ɹ���ȡ��һ��������Ϣ
	virtual void OnClose(int /*connindex*/, const TCHAR* /*szErrDesc*/){}				//һ�����ӹر���

private:
	void handle_accept(Session* new_session,const boost::system::error_code& error);
	boost::asio::ip::tcp::acceptor acceptor_;

	UINT16	m_nListenPort;
};	
