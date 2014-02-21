#include "../CommonHeader.h"
#include "Socket.h"
#include "IOPacket.h"
#include "Session.h"
#include "MessageEncrypt.h"
#include "NetMsg.h"

boost::asio::io_service			g_io_service;
boost::object_pool<CNetMsg>		g_poolMessage;

extern boost::asio::io_service	g_io_service;
bool		g_bStopSocketFlag = false;

volatile	long	Socket::s_nTotalThread;				//�������߳�����		
volatile	long	Socket::s_nTotalSession;			//���лỰ����		
volatile	long	Socket::s_nTotalListening;			//���ڼ����ĻỰ����
volatile	long	Socket::s_nTotalConnecting;			//���������еĻỰ
volatile	long	Socket::s_nTotalConneced;			//�Ѿ���������������ӵĿͻ�������
volatile	long	Socket::s_nTotalClosed;				//�������ѹرյĻỰ����;

volatile	long	Socket::s_nTotalDataSend;			//�����ݷ�������
volatile	long	Socket::s_nTotalDataRecv;			//�����ݽ�������
volatile	long	Socket::s_nTotalDataSendWait;		//�ܵȴ����ݷ�������
volatile	long	Socket::s_nTotalDataSendDiscard;	//�����ݷ��Ͷ�������
volatile	long	Socket::s_nTotalDataRecvWait;		//�ܵȴ����ݽ�������

volatile	long	Socket::s_nTotalDataSendAvg;		//��ƽ��ÿ�����ݷ�������
volatile	long	Socket::s_nTotalDataRecvAvg;		//��ƽ��ÿ�����ݽ�������

volatile	long	Socket::s_nTotalMsgRecvCount;		//�ܽ�����Ϣ��
volatile	long	Socket::s_nTotalMsgSendCount;		//�ܷ�����Ϣ��
volatile	long	Socket::s_nNetMsgObjCount;			//��Ϣ�������ڴ�����ѷ��������

//��ʼ������������, ��������ʱͨ�������̵߳���, ������
void	StartSocketService(int thread_num)
{
	Socket::s_nTotalThread = thread_num;
	SessionManager::GetInstance()->m_poolSession.set_next_size(SESSION_POOL_SIZE);
	g_poolMessage.set_next_size(PACKET_POOL_SIZE);

	//UINT32	__stdcall	SocketCountThread(void* pPara);
	//UINT nThreadID;
	//(HANDLE)_beginthreadex(NULL, NULL, &SocketCountThread, NULL, NULL, &nThreadID);
	//SetThreadName(nThreadID, _T("[Socket Flux Count]"));

	theConsole.Notice(_T("Socket"), _T("spawning [%d] work threads !"), thread_num);
	while(g_bStopSocketFlag == false)
	{
		try
		{
			Sleep(100);

 			std::map<int, boost::thread*> mapThreads;
 			for(int i=1; i<thread_num; i++)
 			{
 			 	boost::thread t(boost::bind(&boost::asio::io_service::run, &g_io_service));
 			 	boost::thread::id tid = t.get_id();

 			 	mapThreads[i] = &t;
 			}

			boost::system::error_code error;
			g_io_service.run(error);
			theConsole.Warning(_T("Socket"), _T("ASIO Service Run Exit![%s]"), boost::system::system_error(error).what());

 			std::map<int, boost::thread*>::iterator ite = mapThreads.begin();
 			for(ite; ite!=mapThreads.end(); ++ite)
 			{
 				ite->second->join();
 			}

		} catch (std::exception& e) 
		{
			theConsole.Error(_T("Exception"), _T("%s:%d - %s"), __FILE__, __LINE__, e.what());
		}
	}
}


//ֹͣ������������
void	StopSocketService()
{
	g_bStopSocketFlag = true;

	
	theConsole.Notice(_T("Terminate"), _T("Please Wait, Destory Sessions[%d] Now..."), Socket::s_nTotalSession);
	SessionManager::GetInstance()->RemoveAllSession();

// 	int nTimeOut = 3;
// 	while(/*Socket::s_nTotalSession && */nTimeOut--)
// 	{
// 		theConsole.Debug("Terminate", "Session Releasing ... Leave:[%d] ", Socket::s_nTotalSession);
// 		Sleep(1000);
// 	}

	g_io_service.stop();
	SessionManager::Release();
}

//static boost::mutex s_mutexSend;

#ifdef _USE_PACKET_MEMORY_POOL_SEND
bool	Send(long nConnectID, const char* data, WORD length)
{
	if(length < 0 || length > _MAX_MSGSIZE)
	{
		theConsole.Error("Send", "Session ID[%d] Send Data Too big [%d/%d] MSGCP:[%d/%d]!", nConnectID, length, _MAX_MSGSIZE, ((char*)data)[0], ((char*)data)[1]);
		return FALSE;
	}
	else if(length >= 2048)
	{
		theConsole.Warning("Send", "Session ID[%d] Send Data Length>=2048 [%d/%d] MSGCP:[%d/%d]!", nConnectID, length, _MAX_MSGSIZE, ((char*)data)[0], ((char*)data)[1]);
	}
	if(g_bStopSocketFlag)return FALSE;

	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning("Send", "Session ID[%d] Not Found, Send Data Failure!", nConnectID);
		return FALSE;
	}

	CNetMsgPtr msgptr = GetPoolMessage();
	if(msgptr == NULL) return FALSE;
	memcpy(msgptr->GetBuf(),	(char*)&length, PACKET_HEADER_LENGTH);
	memcpy(msgptr->GetBuf()+PACKET_HEADER_LENGTH,	data, length);

	pSession->Send(msgptr, msgptr->GetSize());
	return TRUE;
}
#else
bool	Send(long nConnectID, const char* data, WORD length)
{
	if(length < 0 || length > _MAX_MSGSIZE)
	{
		theConsole.Error(_T("Send"), _T("Session ID[%d] Send Data Too big [%d/%d] MSGCP:[%d/%d]!"), nConnectID, length, _MAX_MSGSIZE, ((char*)data)[0], ((char*)data)[1]);
		return FALSE;
	}
	else if(length >= 2048)
	{
		theConsole.Warning(_T("Send"), _T("Session ID[%d] Send Data Length>=2048 [%d/%d] MSGCP:[%d/%d]!"), nConnectID, length, _MAX_MSGSIZE, ((char*)data)[0], ((char*)data)[1]);
	}

	if(g_bStopSocketFlag)return FALSE;

	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning(_T("Send"), _T("Session ID[%d] Not Found, Send Data Failure!"), nConnectID);
		return FALSE;
	}
	//boost::mutex::scoped_lock lock(s_mutexSend);
	char SendBuf[_MAX_MSGSIZE];
	memcpy(SendBuf, &length, sizeof(UINT32));
	memcpy(SendBuf+sizeof(UINT32), data, length);
	pSession->Send(SendBuf, length+sizeof(UINT32));
	return TRUE;
}
#endif

//һ�η��Ͷ����Ϣ
#ifdef _USE_PACKET_MEMORY_POOL_SEND
bool SendMulti( long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize )
{
	if(g_bStopSocketFlag)return FALSE;
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning("Send", "Session ID[%d] Not Found, Send Data Failure!", nConnectID);
		return FALSE;
	}

	assert( !IsBadReadPtr( ppMsg, sizeof(ppMsg) * nMessageCount ) );
	assert( !IsBadReadPtr( pwSize, sizeof(pwSize) * nMessageCount ) );

	CNetMsgPtr msgptr = GetPoolMessage();
	if(msgptr.get()==NULL)return FALSE;

	int offset = 0;
	for( int i = 0; i < nMessageCount; ++i )
	{
		memcpy(msgptr->GetBuf()+offset, (char*)&(pwSize[i]), PACKET_HEADER_LENGTH);
		offset += PACKET_HEADER_LENGTH;
		memcpy(msgptr->GetBuf()+offset, ppMsg[i], pwSize[i]);
		offset += pwSize[i];
	}
	pSession->Send(msgptr, offset );
	return TRUE;
}
#else
bool SendMulti( long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize )
{
	if(g_bStopSocketFlag)return FALSE;
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning(_T("Send"), _T("Session ID[%d] Not Found, Send Data Failure!"), nConnectID);
		return FALSE;
	}

	assert( !IsBadReadPtr( ppMsg, sizeof(ppMsg) * nMessageCount ) );
	assert( !IsBadReadPtr( pwSize, sizeof(pwSize) * nMessageCount ) );


	//boost::mutex::scoped_lock lock(s_mutexSend);	
	char SendBuf[_MAX_MSGSIZE];
	int offset = 0;
	for( int i = 0; i < nMessageCount; ++i )
	{
		memcpy(SendBuf+offset, (char*)&(pwSize[i]), PACKET_HEADER_LENGTH);
		offset += PACKET_HEADER_LENGTH;
		memcpy(SendBuf+offset, ppMsg[i], pwSize[i]);
		offset += pwSize[i];
	}
	pSession->Send(SendBuf, offset );
	return TRUE;
}
#endif 


//ʹ��ͬһ����Ϣͷ�� ���Ͷ����Ϣ������
#ifdef  _USE_PACKET_MEMORY_POOL_SEND
bool SendEx( long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize )
{
	if(g_bStopSocketFlag)return FALSE;
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning("Send", "Session ID[%d] Not Found, Send Data Failure!", nConnectID);
		return FALSE;
	}

	assert( !IsBadReadPtr( ppMsg, sizeof(ppMsg) * nMessageCount ) );
	assert( !IsBadReadPtr( pwSize, sizeof(pwSize) * nMessageCount ) );

	typedef struct tagPACKET_HEADER
	{
		WORD	size;		/// ���� ��Ŷ�� ũ��
	} PACKET_HEADER;

	PACKET_HEADER header;

	// ����ͷ��С
	header.size = 0;
	short i = 0;
	for( i = 0; i < nMessageCount; ++i )
	{
		assert( USHRT_MAX-header.size > pwSize[i] );
		header.size += pwSize[i];
	}

	CNetMsgPtr msgptr = GetPoolMessage();
	memcpy(msgptr->GetBuf(),	(char*)&header, PACKET_HEADER_LENGTH);
	if(header.size > _MAX_MSGSIZE)
	{
		theConsole.Error("Send", "Session ID[%d] Send Data Too big [%d/%d] MSGCP:[%d/%d]!", nConnectID, header.size, _MAX_MSGSIZE, ((char*)SendBuf)[0], ((char*)SendBuf)[1]);
		return FALSE;
	}


	int offset = PACKET_HEADER_LENGTH;
	for( i = 0; i < nMessageCount; ++i )
	{
		memcpy(msgptr->GetBuf()+offset, ppMsg[i], pwSize[i]);
		offset += pwSize[i];
	}
	assert(header.size+PACKET_HEADER_LENGTH == offset);
	pSession->Send(msgptr, offset );

	return TRUE;
}

#else
bool SendEx( long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize )
{
	if(g_bStopSocketFlag)return FALSE;
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)
	{
		theConsole.Warning(_T("Send"), _T("Session ID[%d] Not Found, Send Data Failure!"), nConnectID);
		return FALSE;
	}

	assert( !IsBadReadPtr( ppMsg, sizeof(ppMsg) * nMessageCount ) );
	assert( !IsBadReadPtr( pwSize, sizeof(pwSize) * nMessageCount ) );

	typedef struct tagPACKET_HEADER
	{
		WORD	size;		/// ���� ��Ŷ�� ũ��
	} PACKET_HEADER;

	PACKET_HEADER header;

	//boost::mutex::scoped_lock lock(s_mutexSend);
	char SendBuf[_MAX_MSGSIZE];
	// ����ͷ��С
	header.size = 0;
	short i = 0;
	for( i = 0; i < nMessageCount; ++i )
	{
		assert( USHRT_MAX-header.size > pwSize[i] );
		header.size += pwSize[i];
	}

	memcpy(SendBuf,	(char*)&header, PACKET_HEADER_LENGTH);
	if(header.size > _MAX_MSGSIZE)
	{
		theConsole.Error(_T("Send"), _T("Session ID[%d] Send Data Too big [%d/%d] MSGCP:[%d/%d]!"), nConnectID, header.size, _MAX_MSGSIZE, ((char*)SendBuf)[0], ((char*)SendBuf)[1]);
		return FALSE;
	}

	int offset = PACKET_HEADER_LENGTH;
	for( i = 0; i < nMessageCount; ++i )
	{
		memcpy(SendBuf+offset, ppMsg[i], pwSize[i]);
		offset += pwSize[i];
	}
	assert(header.size+PACKET_HEADER_LENGTH == offset);
	pSession->Send(SendBuf, offset );

	return TRUE;
}

#endif


//�ر�Socket����(ͬ���ر�)
void	CloseSocket(long nConnectID)
{
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)return;
	pSession->Close(0);
}

//�ر�Socket����(�첽�ر�)
void	AsyncClose(long nConnectID, long nWaitSeconds/*=10*/)
{
	Session* pSession = SessionManager::GetInstance()->GetSession(nConnectID);
	if(pSession == NULL)return;
	pSession->Close(nWaitSeconds);
}


UINT32	__stdcall	SocketCountThread(void* pPara)
{
	static UINT32 s_nLastTotalDataRecv = 0;
	static UINT32 s_nLastTotalDataSend = 0;

	while(g_bStopSocketFlag == false)
	{
		//ͳ��ÿ�뷢��/�����ֽ���
		Sleep(1000);	

		Socket::s_nTotalDataRecvAvg = Socket::s_nTotalDataRecv - s_nLastTotalDataRecv;
		Socket::s_nTotalDataSendAvg = Socket::s_nTotalDataSend - s_nLastTotalDataSend;
		
		s_nLastTotalDataRecv = Socket::s_nTotalDataRecv;
		s_nLastTotalDataSend = Socket::s_nTotalDataSend;
	}
	return 0;
}
