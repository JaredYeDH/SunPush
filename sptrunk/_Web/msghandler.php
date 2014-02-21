<?php

require_once( "msgbase.php" );


// --��Ϣ�ܴ���ӿ�
function msg_handler( $socketmsg )
{
	$msgLen  = readMsgInt($socketmsg);
	$msgbase = new MsgBase();
	$msgbase->_ReadPack( $socketmsg );
	
	switch( $msgbase->byMsgCategory )
	{
	   case WS_DATA:
	   {
	       switch( $msgbase->byMsgProtocol )
		   {
		       case S2W_PUSHSVRAUTH_SYN:
			   {
					Handler_S2W_PUSHSVRAUTH_SYN( $socketmsg );
			   }
			   break;
			   
			   case S2W_TEST_ORDER_SYN:
			   {
					Handler_S2W_TEST_ORDER_SYN( $socketmsg );
			   }
			   break;
		   }
	   }
	   break;
     
	}
}

// --ȷ�����ӳɹ�
function ConfirmConnect( $mysocket )
{
	$msgclass = new MSG_W2S_WSCONNECTED_CMD();
	$msg = $msgclass->_WritePack();
	socket_write( $mysocket, $msg, strlen($msg) );
}


// --S2W_PUSHSVRAUTH_SYN������
function Handler_S2W_PUSHSVRAUTH_SYN( $msgdata )
{
    $psmsg = new MSG_S2W_PUSHSVRAUTH_SYN( $msgdata ); 
	echo "[OUT] �յ���Ϣ"."  ��Ϣ����: ".$psmsg->byMsgCategory ."  ��ϢЭ���:".$psmsg->byMsgProtocol."S��֤��: ".$psmsg->szAuthKey.'<br />';
	$msgclass = new MSG_W2S_AUTH_RESULT_ACK();
	if( $psmsg->szAuthKey == "DHF100FSKJF324XDA23K34QPZ6XVO190" )
	{
		echo "[OUT] ��֤PushServer�ɹ�!".'<br />';
		$msgclass->byResult=1;
	}
	else
	{
		echo "[ERR] ��֤PushServerʧ��!".'<br />';
		$msgclass->byResult=0;
	}
	$msg = $msgclass->_WritePack();
	//$_SESSION["s_sock"]
	socket_write( $msgdata, $msg, strlen($msg) ) or die(" [ERR] socket_write() ����: " . socket_strerror(socket_last_error()).'<br />' );
	
	// --��֤ʧ�� �ر�Socket
	if( 0 == $msgclass->byResult )
	{
		socket_close( $msgdata );
	}
}


// ------------------���Զ���----------------
function Handler_S2W_TEST_ORDER_SYN( $msgdata )
{
	$recvmsg = new MSG_S2W_TEST_ORDER_SYN( $msgdata ); 
	$msgclass = new MSG_W2S_NEWORDERBRD_CMD();
	$msgclass->m_strPhone = $recvmsg->strPhone;
	$msgclass->iUsrConID  = $recvmsg->iUsrConID;
	$msgclass->iOrderNo   = $recvmsg->iOrderNo;
	$sendmsg = $msgclass->_WritePack();
	socket_write( $msgdata, $sendmsg, strlen($sendmsg) );
}

?>