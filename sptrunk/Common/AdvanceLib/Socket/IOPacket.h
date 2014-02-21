#pragma once
#include <boost/noncopyable.hpp>

#include "Socket.h"
////////////////////////////////////////////////////////////////////////
//�����ֻ���ڷ����������лỰ�շ����ݻ�������, �����ڶ�����Ϣ��
class IOPacket : public boost::noncopyable
{
	class Session;
public:
	IOPacket();
	~IOPacket(void);

	char		ReadChar();
	char* 		ReadCharArray(long length);
	short*		ReadShortArray(long length);
	long*		ReadLongArray(long length);
	__int64*	ReadLongLongArray(long length);
	short		ReadShort();
	long		ReadLong();
	__int64		ReadLongLong();
	const TCHAR*	ReadString();
	void*			ReadPointer();

	bool	ReverseReadLong(long& data);
	bool	ReverseReadLongLong(__int64& data);

	bool	WriteChar(char data);
	bool	WriteCharArray(const char* data, long length);
	bool	WriteShort(short data);
	bool	WriteShortArray(const short* data, long length);
	bool	WriteLong(long data);
	bool	WriteLongArray(const long* data, long length);
	bool	WriteLongLong(__int64 data);
	bool	WriteLongLongArray(const __int64* data, long length);
	bool	WriteString(const TCHAR* data);
	//bool	WriteString(const std::string& data);
	bool	WritePointer(void* ptr);

	bool	ReWriteChar(long offset, char data);
	bool	ReWriteShort(long offset, short data);
	bool	ReWriteLong(long offset, long data);
	bool	ReWriteLongLong(long offset, __int64 data);

	char*	GetBuffer(){return /*m_pWrite-m_nWriteLength;*/m_szBuffer;}
	void	ResetReadPos();
	void	ResetWritePos();
	bool	Resize(long nNewAllSize);			//�ضϻ�����
	void	Clear();

	long	GetWriteLength(){return m_nWriteLength;}
	long	GetWriteLeavings(){return m_nWriteLeavings;}
	long	GetReadLength(){return m_nReadLength;}
	long	GetReadLeavings(){return m_nReadLeavings;}

	void	ReadSkip(long length);
	void	WriteSkip(long length);

private: 
 
	char	m_szBuffer[SOCKET_IOBUFFER_SIZE];
	char*	m_pWrite;							//��ǰ������ָ��
	char*	m_pRead;							//��ǰ�����ָ��
	long	m_nWriteLength;						//��д���ݵĳ���
	long	m_nWriteLeavings;					//ʣ���д���ݳ���
	long	m_nReadLength;						//�Ѷ����ݵĳ���
	long	m_nReadLeavings;					//ʣ��ɶ����ݳ���
};

