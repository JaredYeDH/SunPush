<?php
	require_once( "msghandler.php" );

	/*function asc2bin($temp) 
	{
        $len = strlen($temp);
        for($i=0; $i<$len; $i++) 
		{
            $data .= sprintf( "%08b", ord(substr($temp, $i, 1)) );
        }
        return $data;
	}
	*/
	error_reporting(E_ALL);
	
	// --��ʱʱ��
	set_time_limit(0);
	ob_implicit_flush();
	
	$address = '127.0.0.1';
	$port = 33066;

	// --����socket
	$socket = socket_create( AF_INET, SOCK_STREAM, SOL_TCP );
	if( $socket < 0 )
	{
	    echo "socket_create() failed: reason: " . socket_strerror($sock) . "\n";
	}
	echo "[OUT] Socket�����ɹ�!".'<BR />';

	// --��˿�
	$ret = socket_bind($socket, $address, $port );
	if( $ret < 0 )
	{
	    echo "socket_bind() failed: reason: " . socket_strerror($ret) . "\n";
	}

	// --����
	echo "[OUT] ��ʼ�����˿�: ".$port.'<BR />';
	$ret = socket_listen($socket,5);
	if( $ret < 0 )
	{
	    echo "socket_listen() failed: reason: " . socket_strerror($ret) . "\n";
	}
	
	// --ѭ��
	while( true )
	{
		// --�������ӣ���һ��Socket������ͨ��
		$spawn = socket_accept( $socket );
		if( $spawn )
		{
			echo "[OUT] �ɹ�����һ������".'<BR />';
			//$_SESSION["s_sock"] = $spawn;
			ConfirmConnect( $spawn );
		}
		else
		{
			echo "[ERR] �ɹ�����һ������".'<BR />';
		}
		
		// --��Ϣ����
		while( true )
		{
			msg_handler( $spawn );
		}
		// --�ر�socket
		//socket_close( $spawn ); 
	}
	
	socket_close( $socket ); 
?>