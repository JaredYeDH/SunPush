#include "../CommonHeader.h"
#include "Socket.h"
#include "IOPacket.h"
#include "Session.h"
#include "Acceptor.h"
#include "Connector.h"
#include "Socket.h"
#include "MessageEncrypt.h"
#include <boost/enable_shared_from_this.hpp>
#include "NetMsg.h"

extern boost::asio::io_service			g_io_service;
extern boost::object_pool<CNetMsg>		g_poolMessage;
extern bool		g_bStopSocketFlag;


//////////////////////////////////////////////////////////////////////////

SessionManager::SessionManager()
{
}
SessionManager::~SessionManager()
{
}

SessionManager* SessionManager::s_instance = NULL;
SessionManager* SessionManager::GetInstance()
{
	if(s_instance == NULL)
	{
		s_instance = new SessionManager();
	}
	return s_instance;
}

//���ڴ�ط���һ���Ự���� 
Session* SessionManager::AllocSession()
{
	boost::mutex::scoped_lock lock(m_mutex);
	Session* pSession = m_poolSession.construct();
	if(pSession==NULL)
	{
		theConsole.Error(_T("Acceptor"), _T("m_poolSession.Construct Failure!"));
		return NULL;
	}
	return pSession;
}

//���ڴ���ͷ�һ���Ự����
void SessionManager::DestroySession(Session* pSession)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_poolSession.destroy(pSession);
}

void SessionManager::RemoveAllSession()
{
	boost::mutex::scoped_lock lock(m_mutex);

	std::map<long, Session*>::iterator ite = m_mapSession.begin();
	for(ite; ite!=m_mapSession.end(); ++ite)
	{
		Session* pSession = ite->second;
		boost::system::error_code ec;
		pSession->socket_.cancel(ec);

		m_poolSession.destroy(pSession);

	}

	m_mapSession.clear();
}

void SessionManager::Release()
{
	delete s_instance;
	s_instance = NULL;
}

void SessionManager::UpdateSession(long nConnectID, Session* pSession)
{
	boost::mutex::scoped_lock lock(m_mutex);

	m_mapSession[nConnectID] = pSession;
}

void SessionManager::RemoveSession(long nConnectID)
{
	boost::mutex::scoped_lock lock(m_mutex);

	m_mapSession.erase(nConnectID);
}

Session* SessionManager::GetSession(long nConnectID)
{
	boost::mutex::scoped_lock lock(m_mutex);

	std::map<long, Session*>::iterator ite = m_mapSession.find(nConnectID);
	if(ite == m_mapSession.end())return NULL;

	return ite->second;
}

UINT32 SessionManager::GetSessionCount()
{
	return (UINT32)m_mapSession.size();
}

UINT32 SessionManager::GetSessionColSize()
{
	return (UINT32)m_poolSession.get_next_size();
}

UINT32 SessionManager::GetMessageColSize()
{
	return (UINT32)g_poolMessage.get_next_size();
}

//////////////////////////////////////////////////////////////////////////


//��ǰ������ߵ�ID
volatile long Session::s_global_current_connexindex = 1;

Session::Session():socket_(g_io_service), timer_(g_io_service),	strand_(g_io_service)
{
	m_emSocketStatus = SS_IDLE;
	m_connindex = InterlockedIncrement(&s_global_current_connexindex);
	m_nLocalPort = 0;
	m_nRemotePort = 0;

	m_pAcceptor = NULL;
	m_pConnector = NULL;

	_tcscpy_s(m_strLocalIP, _T("0.0.0.0"));
	_tcscpy_s(m_strRemoteIP, _T("0.0.0.0"));

	InterlockedIncrement((long*)&Socket::s_nTotalSession);

}

Session::~Session()
{
	m_pAcceptor = NULL;
	m_pConnector = NULL;

	InterlockedDecrement((long*)&Socket::s_nTotalSession);
}

void Session::start()
{

	//boost::asio::socket_base::send_buffer_size option_send(65535);
	//socket_.set_option(option_send);
	//boost::asio::socket_base::receive_buffer_size option_recv(65535);
	//socket_.set_option(option_recv);

#ifndef _BIG_BUFFER_MODE
	m_nSmallBufferModeReadLeave = -1;
	m_nSmallBufferModeReadOffset = 0;	
	CNetMsgPtr netmsg_ptr = GetPoolMessage();
	_receive_sp(netmsg_ptr, 0, PACKET_HEADER_LENGTH);
#else
	socket_.async_receive(boost::asio::buffer(data_, read_buffer_.GetWriteLeavings()),
		strand_.wrap(boost::bind(&Session::handle_read, this, 	
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred)));
#endif

	//size_t cancelcnt = timer_.expires_from_now(boost::posix_time::seconds(10));
	////@return The number of asynchronous operations that were cancelled;��һ��������ʱ��
	//std::cout << "timer canceled count is = " << cancelcnt << std::endl;

	//timer_.async_wait(strand_.wrap(boost::bind(&Session::handle_timeout,this,
	//	boost::asio::placeholders::error)));        
}

#ifndef _BIG_BUFFER_MODE

void Session::_receive_sp(CNetMsgPtr spMsg, long nRecvOffset, long nRecvLength )
{
	 boost::arg<2> iTransBytes = boost::asio::placeholders::bytes_transferred;
	socket_.async_receive( boost::asio::buffer(spMsg->GetBuf()+nRecvOffset, nRecvLength),
		strand_.wrap(boost::bind(&Session::handle_read_sp, this, spMsg,		
		boost::asio::placeholders::error, nRecvLength ) ) );
}

void Session::handle_read_sp(CNetMsgPtr spMsgBuffer, const boost::system::error_code& error, size_t bytes_transferred)
{
	Socket::s_nTotalDataRecv += (long)bytes_transferred;
	if(g_bStopSocketFlag || m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;

	if(bytes_transferred == 0 || (bytes_transferred == -1 && WSAGetLastError() != WSAEWOULDBLOCK))
	{
#ifdef _UNICODE
		TCHAR buf[512];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, boost::system::system_error(error).what(), -1, buf, 512);
		return handle_exit(buf);
#else
		return handle_exit(boost::system::system_error(error).what());
#endif
	}
	if(m_nSmallBufferModeReadLeave == -1 && bytes_transferred == PACKET_HEADER_LENGTH)
	{
		PACKET_HEADER* pHeader = (PACKET_HEADER*)spMsgBuffer->GetBuf();
		UINT16 nMessageLength = (UINT16)pHeader->m_nDataLength /*- 4*/;
		m_nSmallBufferModeReadLeave = nMessageLength;
		m_nSmallBufferModeReadOffset = 0;
		if(spMsgBuffer->GetMaxSize() < nMessageLength)
		{
			//�����Ϣ���ڽ��ջ������� ����ղ�����
			TCHAR szBuffer[512];
			_stprintf_s(szBuffer, _countof(szBuffer), _T("MsgLen:[%d] NetMsg Limit:[%d], Please Reset NetworkMessage Limit!Macro:[_MAX_MSGSIZE]"), nMessageLength, spMsgBuffer->GetMaxSize());
			//::MessageBox(NULL, szBuffer, _T("Error"), MB_OK|MB_ICONERROR);
			assert(szBuffer);
			theConsole.Error(_T("Discard1"), szBuffer);
			_receive_sp(spMsgBuffer, 0, spMsgBuffer->GetMaxSize());
		}
		else if(nMessageLength > 0)
		{
			_receive_sp(spMsgBuffer, 0, nMessageLength);
		}
		else if(nMessageLength == 0)
		{
			//�յ�0���������ݵ���Ϣ�� ����������һ����Ϣ
			m_nSmallBufferModeReadLeave = -1;
			m_nSmallBufferModeReadOffset = 0;
			CNetMsgPtr netmsg_ptr = GetPoolMessage();
			if(netmsg_ptr.get()==NULL)
			{
				TCHAR buff[512];
				_stprintf_s(buff, _countof(buff), _T("������Ϣ�����ڴ�ʧ�ܣ��޿����ڴ档����Ϣ��:[%d], �����ڴ�:[%dKB]"), Socket::s_nNetMsgObjCount, Socket::s_nNetMsgObjCount*_MAX_MSGSIZE/1000);
				theConsole.Error(_T("Socket"), buff);
				return handle_exit(buff);
			}
			_receive_sp(netmsg_ptr, 0, PACKET_HEADER_LENGTH); 
		}
	}
	else
	{
		m_nSmallBufferModeReadLeave -= bytes_transferred;
		m_nSmallBufferModeReadOffset += bytes_transferred;
		if(m_nSmallBufferModeReadLeave == 0)
		{
			//�ص��ϲ�
			if(spMsgBuffer->GetMaxSize() < m_nSmallBufferModeReadOffset+m_nSmallBufferModeReadLeave)
			{
				theConsole.Error(_T("Discard3"), _T("Discard TooBig Message! MsgLen:[%d]"), m_nSmallBufferModeReadOffset);
			}
			else
			{
				InterlockedIncrement((long*)&Socket::s_nTotalMsgRecvCount);
				//��������Magic Code
				PACKET_HEADER* pHeader = (PACKET_HEADER*)spMsgBuffer->GetBuf();
				spMsgBuffer->m_dwMsgSize = m_nSmallBufferModeReadOffset/*-PACKET_HEADER_LENGTH*/;

  				char connbuf[PACKET_BUFFER_SIZE];
  				char tempbuf[512];
  				sprintf(connbuf, "HEADER(%d)=", PACKET_HEADER_LENGTH);
  				for(int i=0; i<PACKET_HEADER_LENGTH; i++)
  				{
  					sprintf(tempbuf, "%02X ", (char)*(char*)(spMsgBuffer->GetBuf()+i));
  					if (strlen(tempbuf)<=3)strcat(connbuf, tempbuf);
  					else strcat(connbuf, tempbuf+6);
  				}
  				theConsole.Trace("Receive","%s", connbuf);
  
  				sprintf(connbuf, "BODY(%d)=", spMsgBuffer->GetSize());
  				for(int i=0; i<spMsgBuffer->GetSize(); i++)
  				{
  					if(i>100){strcat(connbuf,"...");break;}
  					sprintf(tempbuf, "%02X ", (char)*(char*)(spMsgBuffer->GetBuf()+PACKET_HEADER_LENGTH+i));
  					//while(strlen(tempbuf)>3)strcpy(tempbuf, tempbuf+1);
  					if (strlen(tempbuf)<=3)strcat(connbuf, tempbuf);
  					else strcat(connbuf, tempbuf+6);
  				}
  				theConsole.Trace("Receive","%s", connbuf);
  
 
// 				if(pHeader->m_nMagicCode != 0x8888)									//��� Magic Code
// 				{
// 					theConsole.Error("Receive", "Magic Code Error!, Code:[0x%X], MSGID=[%d]", pHeader->m_nMagicCode, pHeader->m_nMessageID);
// 					return;
// 				}

				if(m_pAcceptor)m_pAcceptor->OnReadComplete(m_connindex, spMsgBuffer, m_nSmallBufferModeReadOffset);
 				else if(m_pConnector)m_pConnector->OnReadComplete(m_connindex, spMsgBuffer, m_nSmallBufferModeReadOffset);
			}

			m_nSmallBufferModeReadLeave = -1;
			m_nSmallBufferModeReadOffset = 0;
			CNetMsgPtr netmsg_ptr = GetPoolMessage();
			if(netmsg_ptr.get()==NULL)
			{
				TCHAR buff[512];
				_stprintf_s(buff, _countof(buff), _T("������Ϣ�����ڴ�ʧ�ܣ��޿����ڴ档����Ϣ��:[%d], �����ڴ�:[%dKB]"), Socket::s_nNetMsgObjCount, Socket::s_nNetMsgObjCount*_MAX_MSGSIZE/1000);
				theConsole.Error(_T("Socket"), buff);
				return handle_exit(buff);
			}
			_receive_sp(netmsg_ptr, 0, PACKET_HEADER_LENGTH);
		}
		else if(spMsgBuffer->GetMaxSize() < m_nSmallBufferModeReadOffset+m_nSmallBufferModeReadLeave)
		{
			//�����Ϣ���ڽ��ջ������� ����ղ�����
			if(bytes_transferred-m_nSmallBufferModeReadOffset==0)
			{
				PACKET_HEADER* pHeader = (PACKET_HEADER*)spMsgBuffer->GetBuf();
				theConsole.Error(_T("Discard2"), _T("Discard TooBig Message!RecvLen:[%d], MsgCP:[%d/%d]"), bytes_transferred, HIBYTE(pHeader->m_nMessageID), LOBYTE(pHeader));
			}
			_receive_sp(spMsgBuffer, 0, min(m_nSmallBufferModeReadLeave, spMsgBuffer->GetMaxSize()));
		}
		else if(m_nSmallBufferModeReadLeave > 0)
		{
			_receive_sp(spMsgBuffer, m_nSmallBufferModeReadOffset, m_nSmallBufferModeReadLeave);
		} 
	}
}
#else
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
	Socket::s_nTotalDataRecv += (long)bytes_transferred;
	if(g_bStopSocketFlag || m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;

	if(error)
	{
		//if(error.value() == 995)return;		//�����߳��˳���Ӧ�ó��������ѷ��� I/O ������
#ifdef DEBUGINFO_RECEIVE
		theConsole.Warning("Receive", "handle_read-[%s]", boost::system::system_error(error).what());
#endif
		return handle_exit(boost::system::system_error(error).what());
	}

#ifdef DEBUGINFO_RECEIVE
	theConsole.Debug("Receive","CID[%d] From[%s:%d]��Length Current/Addon=[%d/+%d]", m_connindex, GetRemoteIP(), GetRemotePort(),
		read_buffer_.GetWriteLength(), bytes_transferred);
#endif

	if (bytes_transferred <= 0)
	{
#ifdef DEBUGINFO_CONNECT
		return handle_exit("Զ�����������ر���һ����������!");
#endif
	}

	//1. �Ƚ����ӵ�������ȫ���ŵ�[��ȡ��������]
	if(!read_buffer_.WriteCharArray(data_, (long)bytes_transferred))
	{

		char buff[512];
#ifdef DEBUGINFO_RECEIVE
		theConsole.Warning("Receive", "Message TooBig�� No Buffer��L=[%d/%d]", read_buffer_.GetWriteLength()+bytes_transferred, SOCKET_IOBUFFER_SIZE);
#endif
		sprintf(buff, "���ݳ������, �ѳ������ջ���������! L=[%u/%u]", read_buffer_.GetWriteLength()+bytes_transferred, SOCKET_IOBUFFER_SIZE);
		return handle_exit(buff);
	}

	//2. ѭ����ʼ������ݻ������е����ݣ���������Ϣ�ص��¼�
	while(true)
	{
		if(read_buffer_.GetReadLeavings() < PACKET_HEADER_LENGTH)			//������ʣ�����ݲ���һ����Ϣͷ
		{
			socket_.async_receive(boost::asio::buffer(data_, read_buffer_.GetWriteLeavings()),
				strand_.wrap(boost::bind(&Session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));
#ifdef DEBUGINFO_RECEIVE
			theConsole.Warning("Receive", "Header Not Integrity, Continue Receive! [%d/%d]", read_buffer_.GetReadLeavings(), PACKET_HEADER_LENGTH);
#endif
			break;
		}

		//��Ϣͷ�����Ѿ�ȫ������
		PACKET_HEADER* pHeader = (PACKET_HEADER*)read_buffer_.GetBuffer();	//ȡ��Ϣͷ
		UINT16 nMessageSize = pHeader->m_nDataLength;
		assert((short)nMessageSize > 0); //  ע�͵����У��������close������������ж� Will [2010-12-10 11:24:37 ]
		if (nMessageSize == 0)	Close();
#ifdef USE_ENCRYPT_DATA		//���ܲ���
		m_encryptRecv.Decrypt((unsigned char *)&nMessageSize, 2, false);	// false: ���ƶ�����ָ��
#endif
		if(nMessageSize > PACKET_BUFFER_SIZE)
		{
			char buff[512];
			sprintf(buff, "��Ϣͷ��¼�ĳ������, �ѳ������ջ���������! (�Ƿ�ʹ���˲�ͬ�ļ����㷨?)L=[%u/%u]", read_buffer_.GetWriteLength()+bytes_transferred, SOCKET_IOBUFFER_SIZE);
			theConsole.Warning("Receive", "Data TooBig, No Buffer! MSGCP:[%d/%d]? L=[%d/%d]", ((char*)pHeader)[0], ((char*)pHeader)[1], pHeader->m_nDataLength, SOCKET_IOBUFFER_SIZE);
			return handle_exit(buff);
		}
		//if(pHeader->m_nMagic != 0x0000)									//��� Magic Code
		//{
		//	theConsole.Error("Receive", "Magic Code Error!, MSGID=[%d]", pHeader->m_nMessageID);
		//	return;
		//}

		//char connbuf[PACKET_BUFFER_SIZE];
		//char tempbuf[512];
		//sprintf(connbuf, "HEADER(6)=", pHeader->m_nDataLength);
		//for(int i=0; i<PACKET_HEADER_LENGTH; i++)
		//{
		//	sprintf(tempbuf, "%02X ", (char)*(char*)(read_buffer_.GetBuffer()+i));
		//	strcat(connbuf, tempbuf);
		//}
		//sprintf(tempbuf, " BODY(%d)=", pHeader->m_nDataLength);
		//strcat(connbuf, tempbuf);
		//for(int i=0; i<pHeader->m_nDataLength; i++)
		//{
		//	sprintf(tempbuf, "%02X ", (char)*(char*)(read_buffer_.GetBuffer()+PACKET_HEADER_LENGTH+i));
		//	while(strlen(tempbuf)>3)strcpy(tempbuf, tempbuf+1);
		//	strcat(connbuf, tempbuf);
		//}
		//theConsole.Debug("Receive","%s", connbuf);

		if(nMessageSize+PACKET_HEADER_LENGTH > read_buffer_.GetWriteLength())
		{
			//���յ������ݲ���һ����Ϣʵ�������
			//memset(data_, 0xCC, sizeof(data_));

			socket_.async_receive(boost::asio::buffer(data_, read_buffer_.GetWriteLeavings()),
				strand_.wrap(boost::bind(&Session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));

#ifdef DEBUGINFO_RECEIVE
			theConsole.Debug("Receive", "Message Not Integrity, Continue Receive! [%d/%d]", read_buffer_.GetReadLeavings(), pHeader->m_nDataLength);
#endif

			break;
		}

#ifdef DEBUGINFO_RECEIVE
		theConsole.Notice("Receive", "Receive Complete, TotalLength[%d], BufferLength=[%d]",nMessageSize, read_buffer_.GetWriteLength());
#endif

		//////////////////////////////////////////////////////////////////////////
		//3. ����������һ����Ϣ�����ص��ϲ��¼�
		read_buffer_.ResetReadPos();
		CNetMsgPtr netmsg_ptr = GetPoolMessage();
		//memcpy(data_, read_buffer_.ReadCharArray(nMessageSize), nMessageSize);
		read_buffer_.ReadCharArray(sizeof(PACKET_HEADER));
		//assert(netmsg_ptr->GetMaxSize() >= nMessageSize);	// ע�͵����У���������ж� Will [2010-12-10 11:26:28 ]
		if(netmsg_ptr->GetMaxSize() < nMessageSize) Close();//  Will [2010-12-10 11:27:25 ]
		memcpy(netmsg_ptr->GetBuf(), read_buffer_.ReadCharArray(nMessageSize), nMessageSize);

#ifdef USE_ENCRYPT_DATA		//���ܲ���
		m_encryptRecv.Decrypt((UINT8*)netmsg_ptr->m_bufMsg, nMessageSize);
#endif
		if(m_pAcceptor)m_pAcceptor->OnReadComplete(m_connindex, netmsg_ptr, nMessageSize);
		else if(m_pConnector)m_pConnector->OnReadComplete(m_connindex, netmsg_ptr, nMessageSize);

		//4. ���Ѵ������Ϣ���ݴӻ��������Ƴ�
		int nLeavingLength = read_buffer_.GetReadLeavings();
		if(nLeavingLength>0)memcpy(data_, read_buffer_.GetBuffer() + nMessageSize+sizeof(PACKET_HEADER), nLeavingLength);
		read_buffer_.Clear();
		if(nLeavingLength>0)read_buffer_.WriteCharArray(data_, nLeavingLength);

		//5. ���������ʣ�����ݣ���������գ� ����Ļ��� �ʹ��½�����Ϣͷ����
		if(nLeavingLength >= PACKET_HEADER_LENGTH)
		{
			PACKET_HEADER* pHeader = (PACKET_HEADER*)read_buffer_.GetBuffer();
			nMessageSize = pHeader->m_nDataLength;
#ifdef USE_ENCRYPT_DATA		//���ܲ���
			m_encryptRecv.Decrypt((unsigned char *)&nMessageSize, 2, false);	// false: ���ƶ�����ָ��
#endif
			if(nMessageSize <= nLeavingLength)continue;		//����һ����Ϣ��
		}

#ifdef DEBUGINFO_RECEIVE
		theConsole.Debug("Receive", "No Integrity Message, Leave[%d]", nLeavingLength);
#endif

		memset(data_, 0xCC, sizeof(data_));
		socket_.async_receive(boost::asio::buffer(data_, read_buffer_.GetWriteLeavings()),
			strand_.wrap(boost::bind(&Session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));

		break;
	}
}
#endif

#ifdef _USE_PACKET_MEMORY_POOL_SEND
void Session::Send(CNetMsgPtr msgptr, int length)
//void Session::Send(const char* data, int length)
{
	if(m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;
	InterlockedIncrement((long*)&Socket::s_nTotalMsgSendCount);

#ifdef DEBUGINFO_SEND
	PACKET_HEADER* pHeader = (PACKET_HEADER*)data;
	//	theConsole.Debug("Send", "Send Message CID:[%d], MID:[%d], Len:[%d]", m_connindex, pHeader->m_nMessageID, length);
	//	assert(pHeader->m_nDataLength == length);
#endif

	const char* pFinalSendData = msgptr->GetBuf();
#ifdef USE_ENCRYPT_DATA							//���ܲ���, ���������ݽ��и���
	char buff[PACKET_BUFFER_SIZE];
	memcpy(buff, msgptr->GetBuf(), length);
	m_encryptSend.Encrypt((unsigned char *)buff, length);
	pFinalSendData = buff;
#endif

	boost::asio::const_buffers_1 buffer(pFinalSendData, length);
	boost::system::error_code ec;
	boost::asio::async_write(socket_,buffer,strand_.wrap(boost::bind(&Session::handle_write,
		this,boost::asio::placeholders::error, msgptr,length)));


	InterlockedExchange(&Socket::s_nTotalDataSendWait, Socket::s_nTotalDataSendWait+length);
	InterlockedIncrement(&Socket::s_nNetMsgSendWaitCount);
}
#else
void Session::Send(const char* data, int length)
{
	if(m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;
	InterlockedIncrement((long*)&Socket::s_nTotalMsgSendCount);

#ifdef DEBUGINFO_SEND
	PACKET_HEADER* pHeader = (PACKET_HEADER*)data;
	//	theConsole.Debug("Send", "Send Message CID:[%d], MID:[%d], Len:[%d]", m_connindex, pHeader->m_nMessageID, length);
	//	assert(pHeader->m_nDataLength == length);
#endif

	const char* pFinalSendData = data;
#ifdef USE_ENCRYPT_DATA							//���ܲ���, ���������ݽ��и���
	char buff[PACKET_BUFFER_SIZE];
	memcpy(buff, data,length);
	m_encryptSend.Encrypt((unsigned char *)buff, length);
	pFinalSendData = buff;
#endif

	boost::asio::const_buffers_1 buffer(pFinalSendData, length);
	boost::system::error_code ec;
	boost::asio::async_write(socket_,buffer,strand_.wrap(boost::bind(&Session::handle_write,
		this,boost::asio::placeholders::error, data,length)));


	InterlockedExchange(&Socket::s_nTotalDataSendWait, Socket::s_nTotalDataSendWait+length);
}
#endif


void Session::Close(long nWaitSeconds)
{
	if(nWaitSeconds != 0)
	{
		timer_.expires_from_now(boost::posix_time::seconds(nWaitSeconds));
		timer_.async_wait(boost::bind(&Session::handle_exit, this, _T("���ػ��������ر���һ����������!")));
	}
	else
	{
		handle_exit(_T("���ػ��������ر���һ����������!"));
	}
}

#ifdef _USE_PACKET_MEMORY_POOL_SEND
void Session::handle_write(const boost::system::error_code& error, CNetMsgPtr spMsg, size_t bytes_transferred)
#else
void Session::handle_write(const boost::system::error_code& error, const char* spMsg, size_t bytes_transferred)
#endif
{
	if(m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;

	if(m_pAcceptor)m_pAcceptor->OnSend(m_connindex, (long)bytes_transferred);
	else if(m_pConnector)m_pConnector->OnSend(m_connindex, (long)bytes_transferred);

	if (!error)
	{
		InterlockedExchange(&Socket::s_nTotalDataSendWait, (long)Socket::s_nTotalDataSendWait-bytes_transferred);
		Socket::s_nTotalDataSend += (long)bytes_transferred;
	}
	else
	{
		//__asm int 3;  //  Will [2011-1-26 13:56:55 ] ������
		Socket::s_nTotalDataSendDiscard += (long)bytes_transferred;
		theConsole.Error(_T("Socket"), _T("Send Fail! CID:[%d], [%d]Bytes, [%s]"), m_connindex, (UINT32)bytes_transferred, boost::system::system_error(error).what());
	}

}

void Session::handle_timeout(const boost::system::error_code& e)
{
	if(m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;

	size_t ret = this->socket_.send(boost::asio::const_buffers_1("", 1));

	if (e != boost::asio::error::operation_aborted)
	{
		std::cout << "timeout now.\n";    
		//	this->handle_exit();
	}
	else
	{//��ȡ��
		std::cout << "cancelled now.\n";
	}            
	return;
}

void Session::handle_exit(const TCHAR* szDesc)
{
	if(m_emSocketStatus == SS_CLOSED || m_emSocketStatus == SS_ERROR)return;

	this->m_emSocketStatus = SS_CLOSED;
	InterlockedIncrement((long*)&Socket::s_nTotalClosed);

#ifdef DEBUGINFO_CONNECT
		theConsole.Debug(_T("Close"), _T("CID[%d] Remote Connect Closed, IP=[%s], Port=[%d]"), m_connindex, GetRemoteIP(), GetRemotePort());
#endif
	if(m_pAcceptor)
	{
		m_pAcceptor->OnClose(this->m_connindex, szDesc);
		InterlockedDecrement((long*)&Socket::s_nTotalConneced);
		m_pAcceptor = NULL;
	}
	else if(m_pConnector/* && m_pConnector->m_pConnectSession*/)
	{
		m_pConnector->OnClose(this->m_connindex, szDesc);
		InterlockedDecrement((long*)&Socket::s_nTotalConnecting);
		m_pConnector->m_pConnectSession = NULL;
		m_pConnector = NULL;
	}
	else
	{
		theConsole.Error(_T("Close"), _T("handle_exit Occur Error! Very Serious!"));
		//__asm int 3; //  Will [2011-2-27 6:46:03 ] ��Σ��
	}

	if (!SessionManager::GetInstance()->GetSession(m_connindex))
	{
		//ASSERT("�ظ�����Session::handle_exit");
		theConsole.Error(_T("RemoveSession"), _T("�ظ�����Session::handle_exit connindex: %d"), m_connindex);
		socket_.close();
		return;
	}
	SessionManager::GetInstance()->RemoveSession(m_connindex);
	socket_.close();


	/**//*
		Any asynchronous send, receive
		or connect operations will be cancelled immediately, and will complete
		with the boost::asio::error::operation_aborted error
		*/


	timer_.expires_from_now(boost::posix_time::seconds(10)); //  Will [2011-2-27 5:25:13 ] ������Ϊʲô��Ҫ�ȴ�,�����񲻵Ȼ������⣬�����ģ�������5�ĳ�10���õ����
	timer_.async_wait(boost::bind(&Session::handle_destory, this, boost::asio::placeholders::error));

}

void Session::handle_destory(const boost::system::error_code& error)
{
#ifdef DEBUGINFO_CONNECT
	theConsole.Debug(_T("Close"), _T("CID[%d] Remote Connect Session Destoryed, IP=[%s], Port=[%d]"), m_connindex, GetRemoteIP(), GetRemotePort());
#endif

	SessionManager::GetInstance()->DestroySession(this);
	InterlockedDecrement((long*)&Socket::s_nTotalClosed);
}

//////////////////////////////////////////////////////////////////////////

const TCHAR*	Session::GetLocalIP()
{
	if(_tcslen(m_strLocalIP)>7)return m_strLocalIP;
#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, socket_.local_endpoint().address().to_string().c_str(), -1, m_strLocalIP, 16);
#else
	_tcscpy(m_strLocalIP, socket_.local_endpoint().address().to_string().c_str());
#endif
	return m_strLocalIP;
}

UINT16	Session::GetLocalPort()
{
	if(m_nLocalPort==0)m_nLocalPort = socket_.local_endpoint().port();
	return m_nLocalPort;
}

const char*	Session::GetRemoteIP()
{
	if(_tcslen(m_strRemoteIP)>7)return m_strRemoteIP;
	if(m_emSocketStatus == SS_CLOSED)return _T("0.0.0.0");
#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, socket_.remote_endpoint().address().to_string().c_str(), -1, m_strRemoteIP, 16);
#else
	_tcscpy(m_strRemoteIP, socket_.remote_endpoint().address().to_string().c_str());
#endif
	return m_strRemoteIP;
}

UINT16	Session::GetRemotePort()
{
	if(!socket_.is_open())return 0;
	if(m_nRemotePort==0)m_nRemotePort = socket_.remote_endpoint().port();
	return m_nRemotePort;
}

SOCKET_STATUS Session::GetSocketStatus()
{
	return m_emSocketStatus;
}
