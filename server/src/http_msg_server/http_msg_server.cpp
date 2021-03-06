//
//  http_msg_server.cpp
//  http_msg_server
//
//  Created by jianqing.du on 13-9-29.
//  Copyright (c) 2013年 ziteng. All rights reserved.
//

#include "netlib.h"
#include "ConfigFileReader.h"
#include "RouteServConn.h"
#include "DBServConn.h"
#include "version.h"
#include "ServInfo.h"
#include "HttpConn.h"
#include "HttpQuery.h"
#include "util.h"
#include "EncDec.h"
#include "QueryHttpConn.h"
#include "EventSocket.h"
CAes* pAes;

#define DEFAULT_CONCURRENT_DB_CONN_CNT  2


int main(int argc, char* argv[])
{
	PRINTSERVERVERSION()
    
	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));
    
	log("MsgServer max files can open: %d ", getdtablesize());
    
	CConfigFileReader config_file("httpmsgserver.conf");
    
	char* listen_ip = config_file.GetConfigName("ListenIP");
	char* str_listen_port = config_file.GetConfigName("ListenPort");
    
	uint32_t db_server_count = 0;
	serv_info_t* db_server_list = read_server_config(&config_file, "DBServerIP", "DBServerPort", db_server_count);
    
	uint32_t route_server_count = 0;
	serv_info_t* route_server_list = read_server_config(&config_file, "RouteServerIP", "RouteServerPort", route_server_count);

	// 到BusinessServer的开多个并发的连接
	uint32_t concurrent_db_conn_cnt = DEFAULT_CONCURRENT_DB_CONN_CNT;
	uint32_t db_server_count2 = db_server_count * DEFAULT_CONCURRENT_DB_CONN_CNT;
	char* concurrent_db_conn = config_file.GetConfigName("ConcurrentDBConnCnt");
	if (concurrent_db_conn) {
		concurrent_db_conn_cnt  = atoi(concurrent_db_conn);
		db_server_count2 = db_server_count * concurrent_db_conn_cnt;
	}

	serv_info_t* db_server_list2 = NULL;
	if (db_server_count2 > 0) {
		db_server_list2 = new serv_info_t [ db_server_count2];
		for (uint32_t i = 0; i < db_server_count2; i++) {
			db_server_list2[i].server_ip = db_server_list[i / concurrent_db_conn_cnt].server_ip.c_str();
			db_server_list2[i].server_port = db_server_list[i / concurrent_db_conn_cnt].server_port;
		}
	}


	char* str_aes_key = config_file.GetConfigName("aesKey");
	if (!str_aes_key || strlen(str_aes_key)!=32) {
		log("aes key is invalied");
		return -1;
	}
	pAes = new CAes(str_aes_key);

	if (!listen_ip || !str_listen_port) {
		log("config file miss, exit... ");
		return -1;
	}

	uint16_t listen_port = atoi(str_listen_port);
    
	int ret = netlib_init();
    
	if (ret == NETLIB_ERROR)
		return ret;
    
	CStrExplode listen_ip_list(listen_ip, ';');
	for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
		ret = tcp_server_listen(listen_ip_list.GetItem(i), listen_port, new IMConnEventDefaultFactory<QueryHttpConn>());
		if (ret == NETLIB_ERROR)
			return ret;
	}

	printf("server start listen on: %s:%d\n", listen_ip, listen_port);
    
	init_http_conn();
    
	if (db_server_count > 0) {
		HTTP::init_db_serv_conn(db_server_list2, db_server_count2, concurrent_db_conn_cnt);
	}

	if (route_server_count > 0) {
		HTTP::init_route_serv_conn(route_server_list, route_server_count);
	}

	printf("now enter the event loop...\n");
	writePid();
	netlib_eventloop();
	return 0;
}
