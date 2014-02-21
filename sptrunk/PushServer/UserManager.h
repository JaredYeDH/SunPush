#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_

#include <map>
#include <utility>

using namespace std;

#include "GlobalDefine.h"
#pragma pack(4)

enum USER_STATUS
{
	eus_ERROR		= -1,
	eus_UNKNOW		= 0,					// --�ͻ��˸յ�½�� ʱ���С
	eus_CONNECTED	= 1,					// --�Ѿ�����
	eus_LOGINED		= 2,					// --�Ѿ���½
	eus_READY		= 3,					// --�Ѿ��ɹ�������������Ϣ
};

/* ����: PLAYER_INFOMATION
 * ˵��: ͨ�÷�������½�û��Ļ�����Ϣ
 *       ע��һ��ʹ�øýṹ�ķ�������Ҫ���Լ��Ķ�������, ����Ҫ�����������ʵ��������
 */
struct MSG_BASE;
struct MSG_BASE_FORWARD2;
struct USER_INFOMATION
{
public:
	USER_INFOMATION();
	~USER_INFOMATION();

	USER_STATUS		ChangeStatus(USER_STATUS newStatus);
	void			KickUser(const char* lpszErrorDesc, short nErrorCode = -1);

	/************************************************************************/
	/*                              ͨ�ò���                                */
	/************************************************************************/
	long			m_nConnectID;						// --����ID
	UINT32			m_nAccountID;						// --�ʺ�ID
	USER_STATUS		m_eUserStatus;						// --��ǰ״̬
	char			m_szIPAddr[16];						// --�û�IP��ַ
	char			m_szAccountName[32];				// --�ʺ�����
	char			m_szLoginTime[24];					// --�û���½ʱ��
	char			m_szPassWord[32];                   // --�û�����



	BOOL			SendToClient( MSG_BASE* pMsg, short length );

	BOOL			SendToDBProxy(MSG_BASE_FORWARD2* data, short length);
	void			SendSysMessage(const char* lpszFormat , ...);
};

/* ����: UserInfoManager
 * ˵��: ͨ�õ��û���Ϣ������, ��������һЩͨ�õķ���
 */
template<class T> class UserInfoManager
{
public:
	~UserInfoManager();
	 static UserInfoManager* GetInstance();

	// --������û��� ע�����������Ҫ���ö��Σ� ��һ�ε���ֻ����һ������ID KEY�� �ڶ����ٵ��õ�ʱ�����û��ʺ�KEY��GUID KEY�����ȥ
	T*			AddUser(long nConnectID, UINT32 nAccountID=0, UINT64 nGUID=0);	

	// --�Ƴ�һ���û�����Ϣ
	void		RemoveUserByAccountID(UINT32 nAccountID);
	void		RemoveUserByConnectID(long nConnectID);

	// --����һ���û�����Ϣ
	BOOL		UpdateUserByConnectID(long nConnectID, const T* pPlayerInfo);

	// --�����û�����Ϣ
	T*			FindUserByConnectID(long nConnectID);
	T*			FindUserByAccountID(UINT32 nAccountID);

	UINT32		GetAccountIDByUserName(const TCHAR* lpszUserName);
	UINT32		GetCurUserCount(){return g_nCurPlayerCount;}
	UINT32		GetMaxUserCount(){return g_nMaxPlayerCount;}
	UINT32		GetUserColSize(){return g_nPlayerColSize;}

	void		NoticeMessage(const char* pMsg, short nSize);

private:
	UserInfoManager(){}
	static UserInfoManager*		s_instance;
	std::map<UINT32, T*>		m_mapAIDToINFO;		// --�ʺ�ID���û���Ϣ
	std::map<long  , T*>		m_mapCIDToINFO;		// --����ID���û���Ϣ
	boost::object_pool<T>		m_poolPlayers;		// --�ڴ��
	UINT32						m_nCurPlayerCount;	// --��ǰ�����û�����
	UINT32						m_nMaxPlayerCount;	// --��ֵ�����û�����
	boost::mutex				m_mutex;
};

extern UINT32  g_nPlayerColSize;					// --ȡ���ڴ�ص�ǰ��С
extern UINT32  g_nCurPlayerCount;					// --��ǰ�����û�����
extern UINT32  g_nMaxPlayerCount;					// --��ֵ�����û�����


//-----------------------------------------------------------------------/
/*                              ģ��ʵ��                                */
//-----------------------------------------------------------------------/
template<class T> UserInfoManager<T>* UserInfoManager<T>::s_instance = NULL;
template<class T> UserInfoManager<T>* UserInfoManager<T>::GetInstance()
{
	if(s_instance == NULL)
	{
		s_instance = new UserInfoManager;
	}
	return s_instance;
}


// --�����û�
template<class T> 
T* UserInfoManager<T>::AddUser(long nConnectID, UINT32 nAccountID/*=0*/, UINT64 nGUID/*=0*/)
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<long, T*>::iterator ite = m_mapCIDToINFO.find(nConnectID);
	T* pPlayerInfo = NULL;
	if(ite == m_mapCIDToINFO.end())
	{
		pPlayerInfo = static_cast<T*>(m_poolPlayers.construct());
		if(pPlayerInfo==NULL)return NULL;

		pPlayerInfo->m_nConnectID = nConnectID;
		m_mapCIDToINFO.insert(make_pair(nConnectID, pPlayerInfo));
	}
	else
	{
		pPlayerInfo = (T*)ite->second;
	}

	if(nAccountID != 0)
	{
		pPlayerInfo->m_nAccountID = nAccountID;
		std::pair<std::map<UINT32, T*>::iterator, bool> ret = m_mapAIDToINFO.insert(make_pair(nAccountID, pPlayerInfo));
		ASSERT(ret.second);
		if(!ret.second)return NULL;
	}

	g_nPlayerColSize  = (UINT32)m_poolPlayers.get_next_size();
	g_nCurPlayerCount = (UINT32)m_mapCIDToINFO.size();
	if(g_nCurPlayerCount > g_nMaxPlayerCount)
	{
		g_nMaxPlayerCount = g_nCurPlayerCount;
	}
	return pPlayerInfo;
}


// --ͨ������ID�Ƴ��û�
template<class T> 
void UserInfoManager<T>::RemoveUserByConnectID(long nConnectID)
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<long, T*>::iterator ite = m_mapCIDToINFO.find(nConnectID);
	if(ite == m_mapCIDToINFO.end())return;

	T* pPlayerInfo = (T*)ite->second;
	UINT32 nAccountID = pPlayerInfo->m_nAccountID;

	m_mapCIDToINFO.erase(nConnectID);
	if(nAccountID!=0)m_mapAIDToINFO.erase(nAccountID);
}


// --ͨ���ʺ�ID�Ƴ��û�
template<class T> 
void UserInfoManager<T>::RemoveUserByAccountID( UINT32 nAccountID )
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<UINT32, T*>::iterator ite = m_mapAIDToINFO.find(nAccountID);
	if(ite == m_mapAIDToINFO.end())return;
	T* pPlayerInfo = (T*)ite->second;
	UINT32 nConnectID = pPlayerInfo->m_nConnectID;
	lock.unlock();
	RemovePlayerByConnectID(nConnectID);
}


// --ͨ������ID�����û�
template<class T> 
BOOL UserInfoManager<T>::UpdateUserByConnectID( long nConnectID, const T* pPlayerInfo )
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<long, T*>::iterator ite = m_mapCIDToINFO.find(nConnectID);
	if(ite == m_mapCIDToINFO.end())return FALSE;
	T* pSrcPlayerInfo = ite->second;
	*pSrcPlayerInfo = *pPlayerInfo;
	return TRUE;
}


// --ͨ������ID�����û�����Ϣ
template<class T> 
T* UserInfoManager<T>::FindUserByConnectID(long nConnectID)
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<long, T*>::iterator ite = m_mapCIDToINFO.find(nConnectID);
	if(ite == m_mapCIDToINFO.end())return NULL;
	return ite->second;
}


// --ͨ���ʺ�ID�����û�����Ϣ
template<class T> 
T* UserInfoManager<T>::FindUserByAccountID(UINT32 nAccountID)
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::map<UINT32, T*>::iterator ite = m_mapAIDToINFO.find(nAccountID);
	if(ite == m_mapAIDToINFO.end())return NULL;
	return ite->second;
}


// --������ͨ���û���ط�������
template<class T> 
void UserInfoManager<T>::NoticeMessage(const char* pMsg, short nSize)
{
	boost::mutex::scoped_lock lock(m_mutex);
	MSG_BASE_FORWARD2* pSendMsg = (MSG_BASE_FORWARD2*)pMsg;
	std::map<long  , T*>::iterator ite = m_mapCIDToINFO.begin();
	for(ite; ite!=m_mapCIDToINFO.end(); ++ite)
	{
		ite->second->SendToClient(pMsg, nSize);
		//int nSendConnectID = -1;
		//switch(theConfig.s_emServerType)
		//{
		//case LOGIN_SERVER:	
		//case AGENT_SERVER:		nSendConnectID = ite->first;																break;
		//case WORLD_SERVER:
		//case GAME_SERVER:		pSendMsg->m_dwKey = ite->first; nSendConnectID = ite->second->m_nAgentServerConnectID;		break;
		//case GAME_DBPROXY:		pSendMsg->m_dwKey = ite->first; nSendConnectID = ite->second->m_nGameServerConnectID;		break;
		//default:	ASSERT(!"����������δ���壡");
		//}
		//::Send(nSendConnectID, pMsg, nSize);
	}
}





#endif 