#ifndef _WEBSVR_CONNECTOR_H_
#define _WEBSVR_CONNECTOR_H_

#include <AdvanceLib/Socket/connector.h>

class WebSvrConnector : public Connector
{
public:
	WebSvrConnector();
	~WebSvrConnector();


protected:
	virtual void	OnConnect(int connindex, int nErrCode);							//���µ�������������
	virtual void	OnSend(int connindex, int send_count);							//�Ѿ����ͳ�ȥ������
	virtual void	OnReadComplete(int connindex, CNetMsgPtr msgptr, int length);	//�ɹ���ȡ��һ��������Ϣ
	virtual void	OnClose(int connindex, const char* szErrDesc);					//һ�����ӹر���

};


#endif