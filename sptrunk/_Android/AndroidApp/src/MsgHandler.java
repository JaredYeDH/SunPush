import java.io.IOException;
import java.net.Socket;


public class MsgHandler 
{
	public static void HandleMsg( Socket mySocket ) throws IOException
	{
		// --��������
		int iMsgLen = ReadPackHead( mySocket );
		byte[] byMsgBytes = new byte[MsgBase.MAX_PACK_LEN];
		
		// --������Ϣͷ
		mySocket.getInputStream().read( byMsgBytes, 0, iMsgLen );
		MsgBase MsgHead = new MsgBase();
		MsgHead._ReadPack( byMsgBytes );
		
		// --����
		switch( MsgHead.byMsgCategory )
		{
		case MsgBase.CS_LOGON:
			{
				switch( MsgHead.byMsgProtocol )
				{
					case MsgBase.S2C_SVR_READY_CMD:
					{
						Handler_S2C_SVR_READY_CMD( mySocket, byMsgBytes );
					}
					break;
					
					case MsgBase.S2C_USER_AUTH_ACK:
					{
						Handler_S2C_USER_AUTH_ACK( byMsgBytes );
					}
					break;
				}
			}
			break;
		}
	}
	
	// --��ȡ��Ϣ���Ⱥ��� 
	private static int ReadPackHead( Socket mySocket )throws IOException
	{
		int str0 = mySocket.getInputStream().read();
		int str1 = mySocket.getInputStream().read();
		int str2 = mySocket.getInputStream().read();
		int str3 = mySocket.getInputStream().read();
		int intStr =  str0&0xff | (str1&0xff) << 8 | (str2&0xff) << 16 | (str3&0xff) << 24;
		return intStr;
	}
	
	// --pushsvr->client: �汾��֤��Ϣ������
	private static void Handler_S2C_SVR_READY_CMD( Socket mySocket, byte[] byMsgBytes )throws IOException
	{
		// --����
		MSG_S2C_SVR_READY_CMD rcvMsg = new MSG_S2C_SVR_READY_CMD();
		rcvMsg._ReadPack( byMsgBytes, 2 );
		System.out.println( "[OUT] ��֤�汾�ɹ�! ��ǰ���°汾: "+rcvMsg.sHighVer+"."+rcvMsg.sLowVer );
		System.out.println( "[OUT] ��֤��:"+rcvMsg.dwEncKey + "  У�鴮:" + rcvMsg.strAuth );
		
		// --����
		MSG_C2S_USER_AUTH_SYN sendMsg = new MSG_C2S_USER_AUTH_SYN();
		sendMsg.szUserID = "jakesun";
		sendMsg.szPasswd = "123456";
		sendMsg._WritePack( mySocket );
	}
	
	// --pushsvr->client: ��¼������Ϣ������
	private static void Handler_S2C_USER_AUTH_ACK( byte[] byMsgBytes  )throws IOException
	{
		MSG_S2C_USER_AUTH_ACK rcvMsg = new MSG_S2C_USER_AUTH_ACK();
		rcvMsg._ReadPack( byMsgBytes, 2 );
		if( 1 == rcvMsg.byRes )
		{
			System.out.println( "[OUT] �û���½�ɹ���" );
		}
		else
		{
			System.out.println( "[OUT] �û���½ʧ�ܣ�" );
		}
	}
}
