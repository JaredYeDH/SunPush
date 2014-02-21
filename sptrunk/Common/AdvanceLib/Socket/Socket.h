#pragma once


#define SESSION_POOL_SIZE			10					//���ӻỰ�ڴ�ض����ʼ����
#define PACKET_POOL_SIZE			500					//��Ϣ�ڴ�ض����ʼ����
#define PACKET_BUFFER_SIZE			8192-48				//������Ϣ����󳤶�
#define SOCKET_IOBUFFER_SIZE		16384-48			//����/���ջ�������󻺳���������(Ϊɶ-48 ��Ϊsizeof IOPacket == 16384)

#define PACKET_HEADER_LENGTH		4

struct PACKET_HEADER
{
	unsigned int 	m_nDataLength;
	unsigned short	m_nVersionReserved;
	unsigned short	m_nMessageID;
};

struct DATA_HEADER
{
	DATA_HEADER():m_byMaxVersion(0),m_byMinVersion(1),m_byMsgType(0),m_byMsgID(0){}
	DATA_HEADER (unsigned char byMsgType, unsigned char byMsgID):m_byMaxVersion(0),m_byMinVersion(1),m_byMsgType(byMsgType),m_byMsgID(byMsgID){}
	unsigned char			m_byMaxVersion;
	unsigned char			m_byMinVersion;
	unsigned char			m_byMsgType;
	unsigned char			m_byMsgID;
};

//////////////////////////////////////////////////////////////////////////
//��ʼ������������, ��������ʱͨ�������̵߳���, ������
void	StartSocketService(int thread_num = 5);

//ֹͣ������������
void	StopSocketService();

//��Socket��������
bool	Send(long nConnectID, const char* data, unsigned short length);
bool	SendMulti( long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize );
bool	SendEx(long nConnectID, short nMessageCount, const char** ppMsg, short* pwSize );

//�ر�Socket����(ͬ���ر�)
void	CloseSocket(long nConnectID);
//�ر�Socket����(�첽�ر�)
void	AsyncClose(long nConnectID, long nWaitSeconds = 10);

//////////////////////////////////////////////////////////////////////////
//							�ṩ����ͳ�ƽӿ�
class Socket
{
public:
	static	volatile	long	s_nTotalThread;				//�������߳�����		
	static	volatile	long	s_nTotalSession;			//���лỰ����		
	static	volatile	long	s_nTotalListening;			//���ڼ����ĻỰ����
	static	volatile	long	s_nTotalConnecting;			//���������еĻỰ
	static	volatile	long	s_nTotalConneced;			//�Ѿ���������������ӵĿͻ�������
	static	volatile	long	s_nTotalClosed;				//�������ѹرյĻỰ����

	static	volatile	long	s_nTotalDataSend;			//�����ݷ�������
	static	volatile	long	s_nTotalDataRecv;			//�����ݽ�������
	static	volatile	long	s_nTotalDataSendWait;		//�ܵȴ����ݷ�������
	static	volatile	long	s_nTotalDataSendDiscard;	//�����ݷ��Ͷ�������
	static	volatile	long	s_nTotalDataRecvWait;		//�ܵȴ����ݽ�������

	static	volatile	long	s_nTotalDataSendAvg;		//��ƽ��ÿ�����ݷ�������
	static	volatile	long	s_nTotalDataRecvAvg;		//��ƽ��ÿ�����ݽ�������

	static	volatile	long	s_nTotalMsgRecvCount;		//�ܽ�����Ϣ��
	static	volatile	long	s_nTotalMsgSendCount;		//�ܷ�����Ϣ��

	static	volatile	long	s_nNetMsgObjCount;			//��Ϣ�������ڴ�����ѷ��������

};

