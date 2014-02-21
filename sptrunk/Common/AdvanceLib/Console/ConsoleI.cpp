/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2008 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include "ConsoleO.h"
#include "ConsoleI.h"

//bool Rehash();
extern boost::program_options::variables_map	g_options_map;			//	global program-options map

void LogonConsole::TranslateRehash(char* str)
{
	//sLog.outString("rehashing config file...");
	//Rehash();
}

void LogonConsole::Kill()
{
	_thread->kill=true;
#ifdef WIN32
	/* write the return keydown/keyup event */
	DWORD dwTmp;
	INPUT_RECORD ir[2];
	ir[0].EventType = KEY_EVENT;
	ir[0].Event.KeyEvent.bKeyDown = TRUE;
	ir[0].Event.KeyEvent.dwControlKeyState = 288;
	ir[0].Event.KeyEvent.uChar.AsciiChar = 13;
	ir[0].Event.KeyEvent.wRepeatCount = 1;
	ir[0].Event.KeyEvent.wVirtualKeyCode = 13;
	ir[0].Event.KeyEvent.wVirtualScanCode = 28;
	ir[1].EventType = KEY_EVENT;
	ir[1].Event.KeyEvent.bKeyDown = FALSE;
	ir[1].Event.KeyEvent.dwControlKeyState = 288;
	ir[1].Event.KeyEvent.uChar.AsciiChar = 13;
	ir[1].Event.KeyEvent.wRepeatCount = 1;
	ir[1].Event.KeyEvent.wVirtualKeyCode = 13;
	ir[1].Event.KeyEvent.wVirtualScanCode = 28;
	WriteConsoleInput (GetStdHandle(STD_INPUT_HANDLE), ir, 2, & dwTmp);
#endif
	printf("Waiting for console thread to terminate....\n");
	while(_thread != NULL)
	{
		Sleep(100);
	}
	printf("Console shut down.\n");
}

bool LogonConsoleThread::run()
{
	new LogonConsole;

	//SetThreadName("Console Interpreter");
	//LogonConsole::getInstance()._thread = this;
	size_t i = 0, len = 0;
	char cmd[96];

#ifndef WIN32
	fd_set fds;
	struct timeval tv;
#endif

	while (!kill)
	{
#ifndef WIN32
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO( &fds );
		FD_SET( STDIN_FILENO, &fds );
		if ( select( 1, &fds, NULL, NULL, &tv ) <= 0 )
		{
			if(!kill) // timeout
				continue;
			else
				break;
		}
#endif
		// Make sure our buffer is clean to avoid Array bounds overflow
		memset( cmd,0,sizeof( cmd ) );
		// Read in single line from "stdin"
		fgets( cmd, 80, stdin );

		if ( kill )
			break;

		len = strlen( cmd );
		for ( i = 0; i < len; ++i )
		{
			if ( cmd[i] == '\n' || cmd[i] == '\r' )
				cmd[i] = '\0';
		}
		LogonConsole::getInstance().ProcessCmd( cmd );
	}

	LogonConsole::getInstance()._thread=NULL;
	return true;
}

//------------------------------------------------------------------------------
// Protected methods:
//------------------------------------------------------------------------------
// Process one command
void LogonConsole::ProcessCmd(char *cmd)
{
	typedef void (LogonConsole::*PTranslater)(char *str);
	struct SCmd
	{
		const char *name;
		PTranslater tr;
	};

	SCmd cmds[] =
	{

		{	"?", &LogonConsole::TranslateHelp}, {"help", &LogonConsole::TranslateHelp},
		{	"rehash", &LogonConsole::TranslateRehash},
		{	"reload", &LogonConsole::ReloadAccts},
		{	"netstatus", &LogonConsole::NetworkStatus},
		{	"shutdown", &LogonConsole::TranslateQuit}, 
		{	"exit", &LogonConsole::TranslateQuit}, 
		{	"OutputLevel", &LogonConsole::SetOutputLevel}, 
		{	"ol", &LogonConsole::SetOutputLevel}, 
		{	"ShowConfig", &LogonConsole::ShowConfig}, 
		{	"sc", &LogonConsole::ShowConfig}, 
		{	"ShowDetail", &LogonConsole::ShowDetail}, 
		{	"sd", &LogonConsole::ShowDetail}, 
	};

	char cmd2[80];
	strcpy(cmd2, cmd);
	for(size_t i = 0; i < strlen(cmd); ++i)
		cmd2[i] = tolower(cmd[i]);

	for (size_t i = 0; i < sizeof(cmds)/sizeof(SCmd); i++)
		if (strncmp(cmd2, cmds[i].name, strlen(cmds[i].name)) == 0)
		{
			(this->*(cmds[i].tr)) (cmd + strlen(cmds[i].name));
			return;
		}

	printf("�޷������Ŀ���̨��� (ʹ�� \"help\" �����ð�����).\n");
}

void LogonConsole::SetOutputLevel(char *str)
{
	int level = atoi(str);
	theConsole.SetLevel(level);
	theConsole.Notice("��������", "�Ѹ��Ŀ���̨��Ϣ������𣬼���=[%d]", level);
}

void LogonConsole::ShowConfig(char *str)
{
	theConsole.Print("��ǰ��������������:\n");
	theConsole.Print("----------------------------------------------------------------------\n");
	theConsole.Print("    �ʺ����ݿ��ַ    ��%s\n", g_options_map["db_user_address"].as<std::string>().c_str());
	theConsole.Print("    �ʺ����ݿ�˿�    ��%s\n", g_options_map["db_user_port"].as<std::string>().c_str());
	theConsole.Print("    �ʺ����ݿ��ʺ�    ��%s\n", g_options_map["db_user_username"].as<std::string>().c_str());
	theConsole.Print("    �ʺ����ݿ�����    ��%s\n", g_options_map["db_user_password"].as<std::string>().c_str());
	theConsole.Print("    ��Ϸ���ݿ��ַ    ��%s\n", g_options_map["db_game_address"].as<std::string>().c_str());
	theConsole.Print("    ��Ϸ���ݿ�˿�    ��%s\n", g_options_map["db_game_port"].as<std::string>().c_str());
	theConsole.Print("    ��Ϸ���ݿ��ʺ�    ��%s\n", g_options_map["db_game_username"].as<std::string>().c_str());
	theConsole.Print("    ��Ϸ���ݿ�����    ��%s\n", g_options_map["db_game_password"].as<std::string>().c_str());
	theConsole.Print("    ���ݿ��߳�����    ��%d\n", g_options_map["db_thread_num"].as<unsigned int>());
	theConsole.Print("\n");
	theConsole.Print("    ����WS�������ĵ�ַ��%s\n", g_options_map["address"].as<std::string>().c_str());
	theConsole.Print("    ����WS�������Ķ˿ڣ�%d\n", g_options_map["port"].as<unsigned int>());
	theConsole.Print("\n");
	theConsole.Print("    ����̨�������    ��%d\n", theConsole.GetLevel());
	theConsole.Print("    �ļ���־�������  ��%d\n", 0);
	theConsole.Print("    ������Ʒ��������С��%d\n", omp::app::DP_ITEM_DATA_SIZE);
	theConsole.Print("    MYSQL��¼�������� ��%d\n", MAX_ROWS_LIMIT);
	theConsole.Print("    MYSQL��¼�������� ��%d\n", MAX_ROW_SIZE_LIMIT);
	theConsole.Print("\n");
}

//��ʾ��ϸ��Ϣ
void LogonConsole::ShowDetail(char *str)
{
	
	theConsole.Print("��ǰ����������״̬:\n");
	theConsole.Print("----------------------------------------------------------------------\n");
	SYSTEMTIME time = theCountStat.m_server_start_time;
	theConsole.Print("    ����������ʱ��    ��%04d-%02d-%02d %02d:%02d:%02d\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	GetLocalTime(&time);
	theConsole.Print("    ��ǰ������ʱ��    ��%04d-%02d-%02d %02d:%02d:%02d\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	theConsole.Print("    �����������      ��%d/%d\n", memcache_mgr::getInstance().m_char_count, memcache_mgr::getInstance().m_char_count);	//����ҪLOCK
	theConsole.Print("    ������Ҷ�������  ��%d/%d\n", memcache_mgr::getInstance().m_object_count, memcache_mgr::getInstance().m_object_count);
	const SYSTEMTIME& time1 = memcache_mgr::getInstance().m_prev_commit_time;
	theConsole.Print("    �ϴα������ʱ��  ��%04d-%02d-%02d %02d:%02d:%02d\n", time1.wYear, time1.wMonth, time1.wDay, time1.wHour, time1.wMinute, time1.wSecond);
	const SYSTEMTIME& time2 = memcache_mgr::getInstance().m_prev_commited_time;
	theConsole.Print("    �ϴγɹ��ύʱ��  ��%04d-%02d-%02d %02d:%02d:%02d\n", time2.wYear, time2.wMonth, time2.wDay, time2.wHour, time2.wMinute, time2.wSecond);
	theConsole.Print("    �ϴγɹ��ύ����  ��%d\n", memcache_mgr::getInstance().m_commit_save_count);
	theConsole.Print("    �´α�����Ҽ��  ��%d ms\n", g_options_map["interval_save_to_db"].as<unsigned int>());

	if(strstr(str, "-a"))
	{
		memcache_mgr::MemcacheMap cache_map;
		memcache_mgr::getInstance().get_memcache_map(cache_map);
		memcache_mgr::MemcacheMapIt itea = cache_map.begin();
		theConsole.Line();
		for(itea; itea!=cache_map.end(); ++itea)
		{
			std::map<const char*, memcache_obj* >& object_map = itea->second;
			std::map<const char*, memcache_obj* >::iterator iteo = object_map.begin();
			for(iteo; iteo!=object_map.end(); ++iteo)
			{
				theConsole.Print("  ���:[%d],", iteo->second->get_char_dbid());
				SYSTEMTIME& time = iteo->second->m_last_commited;
				theConsole.Print("����[%s],��־:[%04X],����:[%04d-%02d-%02d %02d:%02d:%02d]\n", iteo->first,iteo->second->get_update_flag(),
					time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);
			}
		}

		if(cache_map.empty())
		{
			theConsole.Print("\n    ��ǰ��Ҷ������δ�����κζ���!\n");
		}
	}

	theConsole.Print("\n");
}

void LogonConsole::ReloadAccts(char *str)
{
	theConsole.Error("��������", "�ù�����δʵ�֣�");
	//AccountMgr::getSingleton().ReloadAccounts(false);
	//IPBanner::getSingleton().Reload();
}

void LogonConsole::NetworkStatus(char *str)
{
	//sSocketMgr.ShowStatus();
	theConsole.Error("����ͳ��", "�ù�����δʵ�֣�");
}

// quit | exit
void LogonConsole::TranslateQuit(char *str)
{
	int delay = str != NULL ? atoi(str) : 5000;
	if(!delay)
		delay = 5000;
	else
		delay *= 1000;

	ProcessQuit(delay);
}
void LogonConsole::ProcessQuit(int delay)
{
	//mrunning = false;
}
//------------------------------------------------------------------------------
// help | ?
void LogonConsole::TranslateHelp(char *str)
{
	ProcessHelp(NULL);
}
void LogonConsole::ProcessHelp(char *command)
{
	if (command == NULL)
	{
		printf("Console:--------help--------\n");
		printf("	Help, ?       : ��ʾ���п�ʹ�õĿ���̨����.\n");
		//printf("	Reload        : ���¼��ز�Ӧ��DP��������������Ŀ(��Щ��Ҫ����).\n");
		printf("	OutputLevel,ol: ���ÿ���̨�������־���𣡲���1=�������\n");
		printf("	LogLevel,   ll: ����������ļ�����־���𣡲���1=��¼����\n");
		printf("	ShowConfig, sc: ��ʾ��ǰDP���������л���������������Ϣ\n");
		printf("	ShowDetail, sd: ��ʾ��ǰDP��������������ʱ״̬��Ϣ\n");
		printf("\n");


		//printf("	Netstatus: Shows network status.\n");
		//printf("	Exit:      Closes the logonserver.\n\n");
	}
}
//------------------------------------------------------------------------------
bool LogonConsoleThread::kill = false;
CThreadState LogonConsoleThread::ThreadState;
time_t LogonConsoleThread::start_time;
int LogonConsoleThread::ThreadId;

LogonConsoleThread::LogonConsoleThread()
{
	kill=false;
}

LogonConsoleThread::~LogonConsoleThread()
{
}
