//  comm_source.h  //
/**********************************************

Authors:
Dvir Katz
Matan Eckhaus Moyal

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the declarations module for the communications and logs in 7Boom project. 

#ifndef COMM_H
#define COMM_H
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>
#include "socket_send_recv.h"

int write_to_file(HANDLE h_output_file, char* str_to_write);
char* allocate_str_buffer(char* str_in);
int log_activity(HANDLE h_log_file, HANDLE log_file_mutex, int log_fails_counter, char* activity_str, int to_free);
void print_and_log_error(HANDLE* h_log_file, HANDLE log_file_mutex, int log_fails_counter,
						char* func_name, char* str_to_write, int error_code);
char* create_raw_msg_4_log(int message_type, char** parameters_arr);
void log_messages(HANDLE h_log_file, HANDLE log_file_mutex, int log_fails_counter,
	char* send_recv, char* to_from, char* owner_type, int message_type, char** parameters_arr);

// Note: see descriptions for the functions in comm_source.c
#endif // COMM_H