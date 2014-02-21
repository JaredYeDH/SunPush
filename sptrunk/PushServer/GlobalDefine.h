//-------------------------------------------------------------------
/*
�ļ�����Define.h
���ߣ�
������
*/
//--------------------------------------------------------------------

#ifndef __GLOBALDEFINE_H__
#define __GLOBALDEFINE_H__



#define	__AUTH_TEST__
#define	ONE_HUNDRED	100 
#define	TEN_THOUSAND 10000
#define TEMP_DBSERIAL_VALUE			(0)
#define MAX_CHARNAME_LENGTH			( 16 )		
#define MAX_CLASSNAME_LENGTH		( 16 )
#define MAX_CHAT_LENGTH				255
#define MAX_NOTICE_LEN				(1024)
#define MAX_SCROLLMSG_LEN			(128)
#define SERVER_UPDATE_TICK			(2)			// ���������� Tick//��Ϊ2  
#define MAX_IPADDRESS_SIZE			15
#define	MAX_LOGPATH_LEN				256
#define	MAX_CONNECTSTRING_LEN		512

#define MAX_AUTHSTR_LEN			(32)
#define MAX_PHONENO_LEN			(11)
#define TO_WS_AUTH_STR			("DHF100FSKJF324XDA23K34QPZ6XVO190")

const int INFO_MAX_LEN			= 128;
const int ID_MAX_LEN			= 20;
const int PASSWD_MAX_LEN		= 24;
const int IP_MAX_LEN			= 16;	// ���ϴ��븦 ����ؼ� Size�� �ø�(16->32)
const int AUTH_SERIAL_KEY_LEN	= 32;	// connection �ø��� Ű(��֤)
const int ENCODE_PASSWORD_LEN   = 32;   // md5(32)����
const int MAX_WORLD_NAME_LEN	= 32;	// �������� ����
const int MAX_CHANNEL_NAME_LEN  = 32;   // Channel���� ����
const DWORD HEARTBEAT_TICK      = 20000;





typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;





#endif // __DEFINE_H__
