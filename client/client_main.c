//  client_source.c  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the main module for the client side in 7Boom project. 

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "client_source.h"
#include "socket_send_recv.h"

// ------------------------------------------------------------------------------------------------------------------------------
// MAIN FUNCTION:
// ------------------------------------------------------------------------------------------------------------------------------
/**************************************** Main Function Summary: ****************************************
/// Description: This program will simulate the client in the communication during 7Boom game.
///
/// Parameters:
///		argc - int. the number of input arguments - should accept 4 arguments.
///		argv - char pointer to a list of input arguments - executable, ip, port, username
///	Returns: int value - 0 if succeeded, 1 if failed
*********************************************************************************************************/
int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("Error: please insert server ip, server port and username to the command line\n");
		return STATUS_FAILURE;
	}
	int ret_val = 0;
	log_fails_counter = 0;
	char* ip = allocate_str_buffer(argv[1]);
	if (NULL == ip) { return STATUS_FAILURE; }
	int port = atoi(argv[2]);
	char* username = allocate_str_buffer(argv[3]);
	if (NULL == username) {
		ret_val = free_mem_close_handles(ip, NULL, NULL, NULL, NULL);
		if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
		return STATUS_FAILURE;
	}
	int user_choice = 0, str_len = 0, loop_flag = TRUE;
	char* activity_str = NULL;
	char* log_file_name = NULL;
	// LOG FILE: creating handle for the log file for this client //
	str_len = snprintf(NULL, 0, "Client_log_%s.txt", username);
	log_file_name = (char*)malloc(sizeof(char) * str_len + 1);
	if (NULL == log_file_name) {
		printf("Error: pointer is NULL, malloc failed\n");
		ret_val = free_mem_close_handles(ip, username, NULL, NULL, NULL);
		if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
		return STATUS_FAILURE;
	}
	snprintf(log_file_name, str_len + 1, "Client_log_%s.txt", username);
	// -----
	HANDLE h_log_file = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_log_file == INVALID_HANDLE_VALUE) {	// If failed to open file
		printf("Error: Failed to open file! The error code is %d\n", GetLastError());
		ret_val = free_mem_close_handles(ip, username, log_file_name, NULL, NULL);
		if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
		return STATUS_FAILURE;
	}
	// create logfile mutex
	client_logfile_mutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == client_logfile_mutex) {
		printf("Error: failed to create mutex. Error is: %d\n", GetLastError());
		ret_val = free_mem_close_handles(ip, username, log_file_name, h_log_file, NULL);
		if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
		return STATUS_FAILURE;
	}
	connect_status connection_status = 0;
	while (loop_flag) {
		connection_status = client_func(username, ip, port, h_log_file);
		if (connection_status == ERROR_FUNC) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "client_func was failed", FALSE);
			ret_val = free_mem_close_handles(ip, username, log_file_name, h_log_file, client_logfile_mutex);
			if (ret_val == STATUS_FAILURE) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "free_mem_close_handles was failed", FALSE);
				return STATUS_FAILURE; 
			}
			return STATUS_FAILURE;
		}
		switch (connection_status) {
		case CONNECT_SUCCEEDED: 
			loop_flag = FALSE; break; 
		case CONNECT_FAILED: 
			user_choice = reconnect_menu(ip, port, h_log_file);
			str_len = snprintf(NULL, 0, "Failed connecting to server on %s:%d\n", ip, port);
			activity_str = (char*)malloc(sizeof(char) * str_len + 1);
			if (NULL == activity_str) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "malloc was failed", FALSE);
				ret_val = free_mem_close_handles(ip, username, log_file_name, h_log_file, client_logfile_mutex);
				if (ret_val == STATUS_FAILURE) {
					print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "free_mem_close_handles was failed", FALSE);
					return STATUS_FAILURE;
				}
				return STATUS_FAILURE;
			}
			snprintf(activity_str, str_len + 1, "Failed connecting to server on %s:%d\n", ip, port);
			// -----
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, activity_str, TRUE);	// updates log file
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "log_activity was failed", FALSE);
				if (NULL != activity_str) free(activity_str);
				return STATUS_FAILURE;
			}
			break;
		
		case CONNECT_DENIED: 
			user_choice = server_denied_menu(ip, port, h_log_file);
			break;
		
		default: 
			user_choice = reconnect_menu(ip, port, h_log_file);
			break;
		
		}		
		if (user_choice == EXIT_GAME)	// user wants to exit game 
			loop_flag = FALSE;			// breaks out from while - close program
	}
	// Ending program:
	if (log_fails_counter != 0) { print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "log file: had errors while running", FALSE); }
	ret_val = free_mem_close_handles(ip, username, log_file_name, h_log_file, client_logfile_mutex);
	if (ret_val == STATUS_FAILURE) { 
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main", "free_mem_close_handles was failed", FALSE);
		return STATUS_FAILURE; 
	}
	return STATUS_SUCCESS;
}