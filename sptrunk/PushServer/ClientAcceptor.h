#ifndef _CLIENT_ACCEPTOR_H_
#define _CLIENT_ACCEPTOR_H_

#include <AdvanceLib/Socket/Acceptor.h>


class ClientAcceptor : public Acceptor
{
public:
	ClientAcceptor(  UINT16 port );
	~ClientAcceptor();


protected:
	virtual void	OnAccept(int connindex);										// --���µ�������������
	virtual void	OnSend(int connindex, int send_count);							// --�Ѿ����ͳ�ȥ������
	virtual void	OnReadComplete(int connindex, CNetMsgPtr msgptr, int length);	// --�ɹ���ȡ��һ��������Ϣ
	virtual void	OnClose(int connindex, const char* szErrDesc);					// --һ�����ӹر���

public:
	//static ClientAcceptor* GetInstance()
	//{
	//	static ClientAcceptor myClientAcceptor;
	//	return &myClientAcceptor;
	//

};


#endif