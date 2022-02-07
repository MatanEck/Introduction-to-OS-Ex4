//  comm_source.c  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the functions module for the communications and logs in 7Boom project.  
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

// Includes 
#include "comm_source.h"

/// === write_to_file ===
/// Description: This function writes relevant activity to the output file.
///				 It gets a the data that needs to be written and write it into the file.
/// parameters:
///		h_output_file - handle to the output file
///		str_to_write - char pointer for string to print to file
///	Returns: int value - 0 if succeeded, 1 if failed
int write_to_file(HANDLE h_output_file, char* str_to_write) {
	if (NULL == h_output_file || NULL == str_to_write) {
		printf("Error: argument pointer is NULL\n");
		return STATUS_FAILURE;
	}
	DWORD str_out_len = strlen(str_to_write), dw_bytes_written = 0;
	BOOL write_status = WriteFile(h_output_file, (void*)str_to_write, str_out_len, &dw_bytes_written, NULL);
	if (write_status == 0 && str_out_len != dw_bytes_written) {
		printf("Error: File write partially or failed. The error code is %d\n", GetLastError());
		return STATUS_FAILURE;
	}
	return STATUS_SUCCESS;
}

/// === allocate_str_buffer ===
/// Description: This function helps formatting a string - allocates the appropriate memory and cpoies the string
/// parameters:
///		str_in -  char pointer for string to be written
///	Returns: char pointer value - pointer to the str_to_erite if succeeded, NULL if failed
char* allocate_str_buffer(char* str_in) {
	if (NULL == str_in) {
		printf("Error: argument pointer is NULL\n");
		return NULL;
	}
	DWORD str_out_len = 0;
	char* str_to_write = NULL;
	str_out_len = snprintf(NULL, 0, "%s", str_in);
	str_to_write = (char*)malloc(sizeof(char) * str_out_len + 1);
	if (NULL == str_to_write) {
		printf("Error: pointer is NULL, malloc failed\n");
		return NULL; // error
	}
	snprintf(str_to_write, str_out_len + 1, "%s\n", str_in);

	//free(str_to_write); - remember to free str_to_write
	return str_to_write;
}

/// === log_activity ===
/// Description: This function logs activity to the log file.
///				 It gets a the data that needs to be written and write it into the file.
/// parameters:
///		h_output_file - handle to the log file
///		log_file_mutex	- handle to the log file mutex
///		log_fails_counter - int value - counter for the failures in logging to log file
///		activity_str - char pointer for string that contains the logged message
///		to_free - TRUE=1 - if need to free the activity_str, FALSE=0 if not
///	Returns: int value - 0 if succeeded, 1 if failed
int log_activity(HANDLE h_log_file, HANDLE log_file_mutex, int log_fails_counter, char* activity_str, int to_free) {
	if (NULL == h_log_file || NULL == log_file_mutex || NULL == activity_str) {
		printf("Error in print_and_log_error - argument pointer is NULL\n");
		if (log_activity(h_log_file, log_file_mutex, log_fails_counter, "Error in log_activity - argument pointer is NULL\n", TRUE)) log_fails_counter++;
		return STATUS_FAILURE;
	}
	int wait_code = 0, write_status = 0, release_res = 0;

	WaitForSingleObject(log_file_mutex, INFINITE);
	if (wait_code != WAIT_OBJECT_0) {
		print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "log_activity", "WaitForSingleObject failed. Failed while waiting for logfile mutex", FALSE);
		if (to_free) { free(activity_str); activity_str = NULL; }
		return STATUS_FAILURE;
	}

	// critical region //
	write_status = write_to_file(h_log_file, activity_str);	// updates log file 
	// end of critical region //

	release_res = ReleaseMutex(log_file_mutex);

	if (STATUS_FAILURE == write_status) {
		print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "log_activity", "write_to_file was falied", FALSE);
		if (to_free) { free(activity_str); activity_str = NULL; }
		return STATUS_FAILURE;
	}
	if (release_res == FALSE) {
		print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "log_activity", "ReleaseMutex failed. Failed while releasing logfile mutex", FALSE);
		if (to_free) { free(activity_str); activity_str = NULL; }
		return STATUS_FAILURE;
	}
	if (to_free) {
		free(activity_str);
		activity_str = NULL;
	}
	return STATUS_SUCCESS;
}

/// === print_and_log_error ===
/// Description: This function prints to screen and logs the errors to the log file.
///				 It gets a the data that needs to be written and write it into the file.
/// parameters:
///		h_output_file - handle to the log file
///		log_file_mutex	- handle to the log file mutex
///		log_fails_counter - int value - counter for the failures in logging to log file
///		func_name - the name of the function that has failed
///		str_to_write - char pointer for string that contains the error message
///		error_code - int value for the error code if exists. FALSE if no error code to log.
///	Returns: VOID
void print_and_log_error(HANDLE* h_log_file, HANDLE log_file_mutex, int log_fails_counter, 
	char* func_name, char* str_to_write, int error_code) {
	if (NULL == h_log_file || NULL == log_file_mutex || NULL == str_to_write || NULL == func_name) {
		printf("Error in print_and_log_error - argument pointer is NULL\n");
		if (log_activity(h_log_file, log_file_mutex, log_fails_counter, "Error in print_and_log_error - argument pointer is NULL\n", TRUE)) log_fails_counter++;
		return;
	}
	int str_len = 0;
	char* log_str = NULL;
	// making the log string:
	if (FALSE == error_code) { // no specific error code to log
		str_len = snprintf(NULL, 0, "Error: Function %s - %s\n", func_name, str_to_write);
		log_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == log_str) {
			print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "print_and_log_error", "malloc was failed", FALSE);
		}
		snprintf(log_str, str_len + 1, "Error: Function %s - %s\n", func_name, str_to_write);
	}
	else {	// log with specific error code
		str_len = snprintf(NULL, 0, "Error: Function %s - %s - Error code is:%d\n", func_name, str_to_write, error_code);
		log_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == log_str) {
			print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "print_and_log_error", "malloc was failed", FALSE);
		}
		snprintf(log_str, str_len + 1, "Error: Function %s - %s - Error code is:%d\n", func_name, str_to_write, error_code);
	}

	printf("%s\n", log_str); // print the error log
	// print to log file:
	if (log_activity(h_log_file, log_file_mutex, log_fails_counter, log_str, TRUE)) log_fails_counter++;// note - this function suppose to free log_str
}

/// === create_raw_msg_4_log ===
/// Description: This function restores the message to log in order to write it to the log file.
/// parameters:
///		message_type - int value that represent the message type
///		parameters_arr - char pointer for array of strings for the message parameters
///	Returns: char pointer to a string (allocated in memory) of the raw message / NULL if failed
char* create_raw_msg_4_log(int message_type, char** parameters_arr)
{
	char* activity_str = NULL;
	int str_len = 0;
	switch (message_type) {
	case CLIENT_REQUEST: // client sends user name to server
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return NULL;
		}
		str_len = snprintf(NULL, 0, "CLIENT_REQUEST:%s\n", parameters_arr[0]);
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "CLIENT_REQUEST:%s\n", parameters_arr[0]);
		break;

	case CLIENT_VERSUS:
		str_len = snprintf(NULL, 0, "CLIENT_VERSUS\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "CLIENT_VERSUS\n");
		break;

	case CLIENT_PLAYER_MOVE: // response to SERVER_MOVE_REQUEST
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return NULL;
		}
		str_len = snprintf(NULL, 0, "CLIENT_PLAYER_MOVE:%s\n", parameters_arr[0]);
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "CLIENT_PLAYER_MOVE:%s\n", parameters_arr[0]);
		break;

	case CLIENT_DISCONNECT:
		str_len = snprintf(NULL, 0, "CLIENT_DISCONNECT\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "CLIENT_DISCONNECT\n");
		break;

	case SERVER_APPROVED:
		str_len = snprintf(NULL, 0, "SERVER_APPROVED\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_APPROVED\n");
		break;

	case SERVER_DENIED:
		str_len = snprintf(NULL, 0, "SERVER_DENIED\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_DENIED\n");
		break;

	case SERVER_MAIN_MENU:
		str_len = snprintf(NULL, 0, "SERVER_MAIN_MENU\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_MAIN_MENU\n");
		break;

	case GAME_STARTED:
		str_len = snprintf(NULL, 0, "GAME_STARTED\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "GAME_STARTED\n");
		break;

	case TURN_SWITCH: // switch turns between players
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return NULL;
		}
		str_len = snprintf(NULL, 0, "TURN_SWITCH:%s\n", parameters_arr[0]);
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "TURN_SWITCH:%s\n", parameters_arr[0]);
		break;

	case SERVER_MOVE_REQUEST:
		str_len = snprintf(NULL, 0, "SERVER_MOVE_REQUEST\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_MOVE_REQUEST\n");
		break;

	case GAME_ENDED:
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return NULL;
		}
		str_len = snprintf(NULL, 0, "GAME_ENDED:%s\n", parameters_arr[0]);
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "GAME_ENDED:%s\n", parameters_arr[0]);
		break;

	case SERVER_NO_OPPONENTS:
		str_len = snprintf(NULL, 0, "SERVER_NO_OPPONENTS\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_NO_OPPONENTS\n");
		break;

	case GAME_VIEW:
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return NULL;
		}
		str_len = snprintf(NULL, 0, "GAME_VIEW:%s;%s;%s\n", parameters_arr[0], parameters_arr[1], parameters_arr[2]);
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "GAME_VIEW:%s;%s;%s\n", parameters_arr[0], parameters_arr[1], parameters_arr[2]);
		break;

	case SERVER_OPPONENT_QUIT:
		str_len = snprintf(NULL, 0, "SERVER_OPPONENT_QUIT\n");
		activity_str = (char*)malloc(sizeof(char) * str_len + 1);
		if (NULL == activity_str) {
			printf("Error: pointer is NULL, malloc failed\n");
			return NULL;
		}
		snprintf(activity_str, str_len + 1, "SERVER_OPPONENT_QUIT\n");
		break;

	default:
		break;
	}
	return activity_str;
}

/// === log_messages ===
/// Description: This function logs messages activity to the log file.
///				 It gets a the data that needs to be written and write it into the file.
///				 Log format: <send_recv> from <owner_type>-<raw_msg>
/// parameters:
///		h_output_file - handle to the log file
///		log_file_mutex	- handle to the log file mutex
///		log_fails_counter - int value - counter for the failures in logging to log file
///		owner_type - char pointer for the owner of the message
///		send_recv - char pointer for string - send / recive message
///		to_from - char pointer for string - "to"/"from"
///		message_type - int value that represent the message type
///		parameters_arr - char pointer for array of strings for the message parameters
///	Returns: int value - 0 if succeeded, 1 if failed
void log_messages(HANDLE h_log_file, HANDLE log_file_mutex, int log_fails_counter,
	char* send_recv, char* to_from, char* owner_type, int message_type, char** parameters_arr) {
	if (NULL == h_log_file || NULL == owner_type || NULL == send_recv || NULL == to_from) {
		print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "log_messages", "argument pointer is NULL", FALSE);
		log_fails_counter++;
		return;
	}// note - parameters_arr will be checked accordingly in create_raw_msg_4_log
	DWORD wait_code;
	BOOL release_res;
	int write_status;
	char* raw_msg = NULL;
	int str_len = 0;
	char* str_to_write = NULL;
	raw_msg = create_raw_msg_4_log(message_type, parameters_arr); // restoring the message to log
	if (raw_msg == NULL) { 
		free(raw_msg);
		print_and_log_error(h_log_file, log_file_mutex, log_fails_counter, "log_messages", "create_raw_msg_4_log was failed", FALSE);
		log_fails_counter++;
		return;
	}
	str_len = snprintf(NULL, 0, "%s %s %s-%s", send_recv, to_from ,owner_type, raw_msg);
	str_to_write = (char*)malloc(sizeof(char) * str_len + 1);
	if (NULL == str_to_write) {
		printf("Error: pointer is NULL, malloc failed\n");
		log_fails_counter++;
		return;
	}
	snprintf(str_to_write, str_len + 1, "%s %s %s-%s", send_recv, to_from, owner_type, raw_msg);
	wait_code = WaitForSingleObject(log_file_mutex, INFINITE);
	if (wait_code != WAIT_OBJECT_0) { printf("Error: Failed while waiting for logfile mutex\n"); free(raw_msg);	log_fails_counter++; return; }

	//critical region
	write_status = write_to_file(h_log_file, str_to_write); // log the relevant message
	//end of critical region

	release_res = ReleaseMutex(log_file_mutex);
	if (STATUS_FAILURE == write_status) { free(raw_msg); free(str_to_write); log_fails_counter++; return; }
	if (release_res == FALSE) { printf("Error: Failed when releasing logfile mutex\n"); free(raw_msg); free(str_to_write); log_fails_counter++; return; }
	free(raw_msg);
	free(str_to_write);
	return;
}