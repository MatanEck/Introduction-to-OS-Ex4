//  client_source.h  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the declarations module for the client side in 7Boom project. 

#ifndef CLIENT_SOURCE_H
#define CLIENT_SOURCE_H

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include "socket_send_recv.h"
#include "comm_source.h"
#pragma once

// Global parameters:
SOCKET m_socket;				// socket for communication 
HANDLE client_logfile_mutex;	// log file mutex
int log_fails_counter;			// counter for the failed logging attempts

// Functions' declarations:
connect_status client_func(char* username, char* ip, int port, HANDLE h_log_file);
int user_move_is_valid(char* move, HANDLE h_log_file);
int main_menu(HANDLE h_log_file);
int reconnect_menu(char  ip[], int port, HANDLE h_log_file);
int server_denied_menu(char  ip[], int port, HANDLE h_log_file);
int free_mem_close_handles(char* ip, char* username, char* log_file_name, HANDLE* h_log_file, HANDLE client_logfile_mutex);

// Note: see descriptions for the functions in client_source.c
#endif // CLIENT_SOURCE_H