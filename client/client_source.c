//  client_source.c  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the functions module for the client side in 7Boom project. 

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "client_source.h"

// ------------------------------------------------------------------------------------------------------------------------------
// CLIENT FUNCTIONS:
// ------------------------------------------------------------------------------------------------------------------------------
/// === client_func ===
/// Description: This function simulates the behavior of the client during the gmae.
/// Parameters:
///		username - char pointer to the username string
///		ip - char pointer to the server ip
///		port -  int to the server port
///		h_log_file - Handle of the log file
///	Returns: connect_status that means - CONNECT_FAILED, CONNECT_DENIED, CONNECT_SUCCEEDED or ERROR_FUNC if error occured
connect_status client_func(char* username, char* ip, int port, HANDLE h_log_file) {
	if (NULL == h_log_file || NULL == username || NULL == ip) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "argument pointer is NULL", FALSE);
		return ERROR_FUNC;
	}
	int message_type = 0, ret_val = 0, str_len = 0, user_choice = 0;
	char user_move[LEN_MESSAGE] = { 0 };
	char* params_to_send[MAX_PARAMS] = { NULL };
	char* params_recieved[MAX_PARAMS] = { NULL };
	char* activity_str = NULL;

	// **************************************************************************************************
	// Initialize Winsock.
	SOCKET client_socket;
	SOCKADDR_IN client_service;
	WSADATA wsa_data; //Create a WSADATA object called wsa_data.
	int i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (i_result != NO_ERROR) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSAStartup returned an error code", FALSE);
		return ERROR_FUNC;
	}
	// Create a socket.
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Check for errors to ensure that the socket is a valid socket.
	if (client_socket == INVALID_SOCKET) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "Socket returned an error code", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSACleanup returned an error. Failed to close Winsocket", WSAGetLastError());
		}
		return CONNECT_FAILED;
	}
	// Create a sockaddr_in object client_service and set  values.
	client_service.sin_family = AF_INET;
	client_service.sin_addr.s_addr = inet_addr(ip); // Setting the IP address to connect to
	client_service.sin_port = htons(port);			// Setting the port to connect to.
	// try to connect to server:
	if (SOCKET_ERROR == connect(client_socket, (SOCKADDR*)&client_service, sizeof(client_service))) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "Connect was failed - There is no connection to server!", FALSE);
		closesocket(client_socket);
		if (WSACleanup() == SOCKET_ERROR) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSACleanup returned an error. Failed to close Winsocket", WSAGetLastError());
		}
		return CONNECT_FAILED;
	}
	// if connected - write to screen and log it
	str_len = snprintf(NULL, 0, "Connected to server on %s:%d\n", ip, port);
	activity_str = (char*)malloc(sizeof(char) * str_len + 1);
	if (NULL == activity_str) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "malloc was failed", FALSE);
		// gracefull
		closesocket(client_socket);
		if (WSACleanup() == SOCKET_ERROR) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSACleanup returned an error. Failed to close Winsocket", WSAGetLastError());
		}
		return ERROR_FUNC;
	}
	snprintf(activity_str, str_len + 1, "Connected to server on %s:%d\n", ip, port);
	// -----
	printf(activity_str);
	ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, activity_str, TRUE);
	if (ret_val) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "log_activity was failed", FALSE);
		// gracefull
		closesocket(client_socket);
		if (WSACleanup() == SOCKET_ERROR) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSACleanup returned an error. Failed to close Winsocket", WSAGetLastError());
		}
		if (NULL != activity_str) free(activity_str);
		return ERROR_FUNC;
	}
	// **************************************************************************************************

	int exit_status_client = STATUS_SUCCESS;
	int server_connection = TRUE;
	int timeout = DEFUALT_TIMEOUT;
	// first the user sends the username - and waits to SERVER_APPROVED (in the while loop)
	params_to_send[0] = username;
	ret_val = send_message(client_socket, CLIENT_REQUEST, params_to_send);
	if (ret_val != TRNS_SUCCEEDED) { server_connection = FALSE; }
	else { log_messages(h_log_file, client_logfile_mutex, log_fails_counter, "sent", "to", "server", CLIENT_REQUEST, params_to_send); }
	timeout = DEFUALT_TIMEOUT;
	free_parameters(params_to_send);

	while (server_connection)
	{
		if (server_connection) {   // recieve server response
			free_parameters(params_recieved);
			ret_val = recv_message(client_socket, &message_type, params_recieved, timeout);
			if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; exit_status_client = ret_val; break; }
			log_messages(h_log_file, client_logfile_mutex, log_fails_counter, "received", "from", "server", message_type, params_recieved);
			timeout = DEFUALT_TIMEOUT;

		}
		if (server_connection) {	// deal with server's response
			free_parameters(params_to_send);
			switch (message_type) {
			case SERVER_APPROVED:	// server approved user name
				break;

			case SERVER_DENIED:		// server denied user name / too many players
				server_connection = FALSE;
				exit_status_client = CONNECT_DENIED;
				break;

			case SERVER_MAIN_MENU:	// connected and approved - choose play/quit
				user_choice = main_menu(h_log_file);
				if (user_choice == WANT_PLAY) {		// client want to play
					ret_val = send_message(client_socket, CLIENT_VERSUS, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; exit_status_client = ret_val; break; }
					else { log_messages(h_log_file, client_logfile_mutex, log_fails_counter, "sent", "to", "server", CLIENT_VERSUS, NULL); }
					timeout = INVITE_TIMEOUT; // wait 30 sec for answer
				}
				if (user_choice == EXIT_GAME) {		// client wants to quit
					ret_val = send_message(client_socket, CLIENT_DISCONNECT, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; exit_status_client = ret_val; break; }
					else { log_messages(h_log_file, client_logfile_mutex, log_fails_counter,  "sent", "to", "server", CLIENT_DISCONNECT, NULL); }
				}
				break;

			case GAME_STARTED:				// the server found another player - game on!
				printf("Game is on!\n");
				break;

			case TURN_SWITCH:				// switch turns between players - just notice
				if (strcmp(params_recieved[0], username) == 0)
					printf("Your turn!\n");
				else
					printf("%s's turn!\n", params_recieved[0]);
				timeout = TEN_MIN;
				break;

			case SERVER_MOVE_REQUEST:		// it's my turn - choose num/boom
				do {
					printf("Enter the next number or boom:\n");
					scanf_s("%s", &user_move, (rsize_t)sizeof(user_move));
				} while (user_move_is_valid(user_move, h_log_file));	// checks if valid move (1 means not valid - try again)
				params_to_send[0] = user_move;
				ret_val = send_message(client_socket, CLIENT_PLAYER_MOVE, params_to_send);
				if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; exit_status_client = ret_val; break; }
				else { log_messages(h_log_file, client_logfile_mutex, log_fails_counter,  "sent", "to", "server", CLIENT_PLAYER_MOVE, params_to_send); }
				timeout = TEN_MIN;
				break;

			case GAME_ENDED:	// the game is over
				printf("%s won!\n", params_recieved[0]);
				break;

			case SERVER_NO_OPPONENTS:	// the server didn't find another player - choose what to do
				break;

			case GAME_VIEW:		// updates on the game
				if (!strcmp(params_recieved[2], "END")) { // Game is over
					printf("%s move was %s\nThe game ended\n", params_recieved[0], params_recieved[1]);
				}
				else {
					printf("%s move was %s\nThe game continues\n", params_recieved[0], params_recieved[1]);
				}
				break;

			case SERVER_OPPONENT_QUIT:	// the other client has quit
				printf("Opponent quit.\n");
				break;
			case CLIENT_DISCONNECT: // Part of gracefull disconnection
				server_connection = FALSE; exit_status_client = ret_val;
				break;
			}
		}
	}
	// gracefull
	if (!server_connection) {
		printf("Server disconnected. Exiting.\n");
		ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Server disconnected. Exiting.\n", FALSE);
		if (ret_val) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "log_activity was failed", FALSE);
		}
	}
	if (WSACleanup() == SOCKET_ERROR) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "client_func", "WSACleanup returned an error. Failed to close Winsocket", WSAGetLastError());
	}
	closesocket(client_socket);
	free_parameters(params_recieved);
	return exit_status_client;
}


/// *********************** PRINT,LOG AND MENUS FUNCTIONS: ************************************

/// === main_menu ===
/// Description: This function prints to the user the main_menu and return its choice
/// Parameters:
///		h_log_file - Handle of the log file
///	Returns: int value - user's choice - 1= play / 2=exit game
int main_menu(HANDLE h_log_file) {
	int user_choice = 0, ret_val = 0;
	char line[LEN_MESSAGE];
	int isint;
	do {
		printf("Choose what to do next:\n");
		printf("1. Play against another client\n");
		printf("2. Quit\n");
		fgets(line, sizeof line, stdin);
		line[strcspn(line, "\n")] = 0; //check for only \n
		if (!strcmp(line,""))
			fgets(line, sizeof line, stdin);
		isint = sscanf(line, "%d", &user_choice);
		if (!isint) {	// if the user inserted a letter/word
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
		else if (user_choice != 1 && user_choice != 2) { // if the user inserted unvalid number
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "main_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
	} while (user_choice != 1 && user_choice != 2);
	return user_choice;
}

/// === reconnect_menu ===
/// Description: This function prints to the user the reconnect_menu to and return its choice
/// Parameters:
///		h_log_file - Handle of the log file
///	Returns: int value - user's choice - 1= reconnect / 2=exit game 
int reconnect_menu(char  ip[], int port, HANDLE h_log_file)
{
	int user_choice = 0, ret_val = 0;
	char line[LEN_MESSAGE];
	int isint;
	do {
		printf("Failed connecting to server on %s:%d.\n", ip, port);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		fgets(line, sizeof line, stdin);
		line[strcspn(line, "\n")] = 0;
		if (!strcmp(line, ""))
			fgets(line, sizeof line, stdin);
		isint = sscanf(line, "%d", &user_choice);
		if (!isint){	// if the user inserted a letter/word
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "reconnect_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
		else if (user_choice != 1 && user_choice != 2) { // if the user inserted unvalid number
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "reconnect_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
	} while (user_choice != 1 && user_choice != 2);

	return user_choice;
}

/// === server_denied_menu ===
/// Description: This function prints to the user the server_denied_menu to and return its choice
/// Parameters:
///		h_log_file - Handle of the log file
///	Returns: int value - user's choice - 1= reconnect / 2=exit game
int server_denied_menu(char  ip[], int port ,HANDLE h_log_file)
{
	int user_choice = 0, ret_val = 0;
	char line[LEN_MESSAGE];
	int isint;
	do {
		printf("server on %s:%d denied the connection request.\n", ip, port);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		fgets(line, sizeof line, stdin);
		line[strcspn(line, "\n")] = 0;
		if (!strcmp(line, ""))
			fgets(line, sizeof line, stdin);
		isint = sscanf(line, "%d", &user_choice);
		if (!isint) {	// if the user inserted a letter/word
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "server_denied_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
		else if (user_choice != 1 && user_choice != 2) { // if the user inserted unvalid number
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "server_denied_menu", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
	} while (user_choice != 1 && user_choice != 2);
	return user_choice;
}

/// === free_mem_close_handles ===
/// Description: This function closes all the relevant handles and frees the memory allocations. 
///				 It gets NULL when a parameter is not relevant so it won't be affected
/// Parameters:
///		ip -  char pointer to a string of the server ip	
///		username - char pointer to a string of the user name
///		log_file_name - char pointer to a string to the name of the log file
///		h_log_file - Handle of the log file
///		client_logfile_mutex - Handle of the log file mutex
///	Returns: int value - 0 if succeeded, 1 if failed
int free_mem_close_handles(char* ip, char* username, char* log_file_name, HANDLE* h_log_file, HANDLE client_logfile_mutex) {
	int ret_val = 0;
	int status = 0; // if 0 the function succeeded, if status<0 - fails

	if (NULL != h_log_file) {
		ret_val = CloseHandle(h_log_file);
		if (FALSE == ret_val) {
			print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "free_mem_close_handles", "Error when closing file handle", GetLastError());
			status--;
		}
	}

	if (NULL != client_logfile_mutex) {
		ret_val = CloseHandle(client_logfile_mutex);
		if (FALSE == ret_val) {
			printf("Error: Function free_mem_close_handles - Error when closing mutex handle - Error code is:%d\n", GetLastError());
			status--;
		}
	}
	if (NULL != ip) { free(ip); }
	if (NULL != username) { free(username); }
	if (NULL != log_file_name) { free(log_file_name); }

	if (status < 0)
		return STATUS_FAILURE;
	return STATUS_SUCCESS;
}


/// === user_move_is_valid ===
/// Description: This function checks if the user's move is valid.
/// Parameters:
///		move - char pointer to the player's move
///		h_log_file - Handle of the log file
///	Returns: int value - 0 if succeeded, 1 if failed
int user_move_is_valid(char* move, HANDLE h_log_file) {
	if (NULL == move) {
		print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "user_move_is_valid", "argument pointer is NULL", FALSE);
		return STATUS_FAILURE;
	}
	int user_choice = 0, ret_val = 0;
	int isint;

	isint = sscanf(move, "%d", &user_choice);
	if (isint) { // if the user inserted a number
		return STATUS_SUCCESS;
	}
	else {	// if the user inserted a letter/word
		if (!_stricmp("Boom", move)) { // if the string is boom - valid
			return STATUS_SUCCESS;
		}
		else {
			printf("Error: Illegal command\n");
			ret_val = log_activity(h_log_file, client_logfile_mutex, log_fails_counter, "Error: Illegal command\n", FALSE); // updates log file 
			if (ret_val) {
				print_and_log_error(h_log_file, client_logfile_mutex, log_fails_counter, "user_move_is_valid", "log_activity was failed", FALSE);
				log_fails_counter++;
			}
		}
	}
	return STATUS_FAILURE;
}
