<?php

require_once( "netcommon.php" );


// --��Ϣ���ೣ��
define( "CS_LOGON", 10 );						// --�ƶ���<->PushSvr��Ϣ����
define( "WS_DATA" , 50 );						// --WebSvr<->PushSvr��Ϣ����


// --��ϢЭ���
define ( "W2S_WSCONNECTED_CMD", 0);				// --W2S:Web����������֪ͨ
define ( "S2W_PUSHSVRAUTH_SYN", 1);             // --S2W:���ͷ�������֤����
define ( "W2S_AUTH_RESULT_ACK", 2);             // --W2S:���ͷ�������֤����
define ( "W2S_NEWORDERBRD_CMD", 4);             // --W2S:���������㲥����
define ( "S2W_TEST_ORDER_SYN", 52);             // --W2S:���������㲥����


// --��Ϣ����
class MsgBase
{
   var  $byMsgCategory;							// --��Ϣ���
   var	$byMsgProtocol;							// --��ϢЭ���

   function _ReadPack( $msgdata )
   {
		$this->byMsgCategory = readMsgByte( $msgdata );
		$this->byMsgProtocol = readMsgByte( $msgdata ); 
   }
   
   function _WritePack( $msgpack )
   {
   }
   
    function __construct()
   {
		$this->byMsgCategory = 0;
		$this->byMsgProtocol = 0; 
   }
}


// --WebServer����PushServer���Ӻ��͵ĵ�һ����Ϣ
class MSG_W2S_WSCONNECTED_CMD extends MsgBase
{
	var $szUserID;	
    var $szPasswd;

	function __construct()
   {
		$this->byMsgCategory = WS_DATA;
		$this->byMsgProtocol = W2S_WSCONNECTED_CMD;
   }
   function _WritePack()
   {
		$msgpack = pack("ICC", 2, $this->byMsgCategory, $this->byMsgProtocol ); 
		return $msgpack;
   }
}


// --PushServer���͸�WebServer�ĵ�һ������Ϣ
class MSG_S2W_PUSHSVRAUTH_SYN extends MsgBase
{
    var $szAuthKey;
	function __construct( $msgdata )
   {
		$this->byMsgCategory = WS_DATA;
		$this->byMsgProtocol = S2W_PUSHSVRAUTH_SYN;
		$this->szAuthKey = readMsgStr( $msgdata, 32 );
   }
}


// --PushServer���͸�WebServer�ĵ�һ������Ϣ
class MSG_W2S_AUTH_RESULT_ACK extends MsgBase
{
    var $byResult;
	function __construct()
   {
		$this->byMsgCategory = WS_DATA;
		$this->byMsgProtocol = W2S_AUTH_RESULT_ACK;
   }
   
    function _WritePack()
   {
		$msgpack = pack("ICCC", 3, $this->byMsgCategory, $this->byMsgProtocol, $this->byResult ); 
		return $msgpack;
   }
}



// --WebServer���͸�PushServer�Ķ����㲥 
class MSG_W2S_NEWORDERBRD_CMD extends MsgBase
{
	var $iUsrConID;
	var $iOrderNo;
	var	$strPhone;
	var	$arrDrivers;
	function __construct()
   {
		$this->byMsgCategory = WS_DATA;
		$this->byMsgProtocol = W2S_NEWORDERBRD_CMD;
   }
   function _WritePack()
   {
		$msgpack = pack("ICCII", 10, $this->byMsgCategory, $this->byMsgProtocol, $this->iUsrConID, $this->iOrderNo ); 
		//$msgpack = $msgpack.$this->strPhone;
		return $msgpack;
   }
}


// --���Զ�����Ϣ
class MSG_S2W_TEST_ORDER_SYN  extends MsgBase
{
   	var	$byOrderType;
	var $iUsrConID;
	var $iOrderNo;
	var	$iUserPosX;
	var	$iUserPosY;
	var	$strPhone;
	var	$strTest;
	function __construct( $msgdata )
   {
		$this->byMsgCategory = WS_DATA;
		$this->byMsgProtocol = S2W_TEST_ORDER_SYN;
		$this->byOrderType = readMsgByte( $msgdata );
		$this->iUsrConID = readMsgInt( $msgdata );
		$this->iOrderNo = readMsgInt( $msgdata );
		$this->iUserPosX = readMsgInt( $msgdata );
		$this->iUserPosY = readMsgInt( $msgdata );
		$this->strPhone = readMsgStr( $msgdata, 11 );
		$this->strTest = readMsgStr( $msgdata, 168 );
   }
}

?>