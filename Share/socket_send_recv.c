//  socket_send_recv.c  //
/**********************************************

Authors:
Dvir Katz
Matan Eckhaus Moyal

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the functions module for the sending & reciving messages in 7Boom project. 
/// Note: Based on recitation example that was Last updated by Amnon Drory, Winter 2011.

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "socket_send_recv.h"

// -------------------------------------------------------------------------------------------------------------------
// Functions for sending messages: 
// -------------------------------------------------------------------------------------------------------------------
/// === send_message ===
/// Description: This function sends a message.
///				 It gets a the data that needs to be delivered and send it.
/// parameters:
///		socket - socket handle for the current connection
///		message_type - message_type item, defines the type of message to be sent
///		parameters_arr - pointer to buffer that contains the paramters to the message if needed
///	Returns: transfer_result_t value - according to TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED.
transfer_result_t send_message(SOCKET socket, int message_type, char* parameters_arr[]) {
	// will check if the parameters_arr is NULL when needed. It can be NULL if there are no parameters.
	char message_to_send[LEN_MESSAGE];
	transfer_result_t send_res = TRNS_SUCCEEDED;

	switch (message_type) {
	case CLIENT_REQUEST: // client sends user name to server
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return ERROR_FUNC;
		}
		sprintf_s(message_to_send, LEN_MESSAGE, "CLIENT_REQUEST:%s\n", parameters_arr[0]);
		break;

	case CLIENT_VERSUS:
		sprintf_s(message_to_send, LEN_MESSAGE, "CLIENT_VERSUS\n");
		break;

	case CLIENT_PLAYER_MOVE: // response to SERVER_MOVE_REQUEST
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return ERROR_FUNC;
		}
		sprintf_s(message_to_send, LEN_MESSAGE, "CLIENT_PLAYER_MOVE:%s\n", parameters_arr[0]);
		break;

	case CLIENT_DISCONNECT:
		sprintf_s(message_to_send, LEN_MESSAGE, "CLIENT_DISCONNECT\n");
		break;

	case SERVER_APPROVED:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_APPROVED\n");
		break;

	case SERVER_DENIED:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_DENIED\n");
		break;

	case SERVER_MAIN_MENU:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_MAIN_MENU\n");
		break;

	case GAME_STARTED:
		sprintf_s(message_to_send, LEN_MESSAGE, "GAME_STARTED\n");
		break;

	case TURN_SWITCH: // switch turns between players
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return ERROR_FUNC;
		}
		sprintf_s(message_to_send, LEN_MESSAGE, "TURN_SWITCH:%s\n", parameters_arr[0]);
		break;

	case SERVER_MOVE_REQUEST:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_MOVE_REQUEST\n");
		break;

	case GAME_ENDED:
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return ERROR_FUNC;
		}
		sprintf_s(message_to_send, LEN_MESSAGE, "GAME_ENDED:%s\n", parameters_arr[0]);
		break;

	case SERVER_NO_OPPONENTS:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_NO_OPPONENTS\n");
		break;

	case GAME_VIEW:
		if (NULL == parameters_arr) {
			printf("Error: argument pointer is NULL\n");
			return ERROR_FUNC;
		}
		sprintf_s(message_to_send, LEN_MESSAGE, "GAME_VIEW:%s;%s;%s\n", parameters_arr[0], parameters_arr[1], parameters_arr[2]);
		break;

	case SERVER_OPPONENT_QUIT:
		sprintf_s(message_to_send, LEN_MESSAGE, "SERVER_OPPONENT_QUIT\n");
		break;

	default:
		if (send_res != TRNS_SUCCEEDED) {
			printf("Error:The message type is not valid!\n");
			return send_res;
		}
		break;
	}
	//send message 
	send_res = send_string(message_to_send, socket);
	if (send_res != TRNS_SUCCEEDED) {
		printf("Error: error while trying to write data to socket\n");
		return send_res;
	}
	//printf("-info- succeed sent messeage: %s", message_to_send);
	return TRNS_SUCCEEDED;
}


/// === recv_message ===
/// Description: This function sends a message.
///				 It gets a the data that needs to be delivered and send it.
/// parameters:
///		socket - socket handle for the current connection
///		message_type - message_type item, defines the type of message to be sent
///		parameters_arr - pointer to buffer that contains the paramters to the message if needed
///		timeout - int value. the max time to wait for response
///	Returns: transfer_result_t value - according to TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED.
transfer_result_t recv_message(SOCKET socket, int* message_type, char** parameters_arr, int timeout) {
	if (NULL == message_type || NULL == parameters_arr) {
		printf("Error: argument pointer is NULL\n");
		return ERROR_FUNC;
	}
	char* accepted_str = NULL;
	transfer_result_t recv_res;
	int ret_val, num_of_expected_params = 0, i = 0;
	
	// wait for response until timeout //
	fd_set set;
	struct timeval time_out;
	FD_ZERO(&set);			// clear the set
	FD_SET(socket, &set);	// add our file descriptor to the set
	time_out.tv_sec = timeout;
	time_out.tv_usec = 0;
	ret_val = select(socket + 1, &set, NULL, NULL, &time_out);
	if (ret_val == 0) { printf("TIME OUT Error: time limit expired while waiting for response\n"); return TRNS_FAILED; }
	if (ret_val == SOCKET_ERROR) {
		printf("Error: an error occurred while waiting for response. The error is:%d\n", WSAGetLastError());
		return TRNS_FAILED;
	}
	// recive string message: //
	recv_res = recv_str(&accepted_str, socket);
	if (recv_res != TRNS_SUCCEEDED) {
		printf("Error: failed to recive message from server!\n");
		return recv_res;
	}
	//printf("RecieveString from server succeed the message is: %s", accepted_str);

	// check message type: //
	if (strstr(accepted_str, "CLIENT_REQUEST") != NULL) {
		*message_type = CLIENT_REQUEST;
		num_of_expected_params = 1;
	}
	else if (strstr(accepted_str, "CLIENT_VERSUS") != NULL) {
		*message_type = CLIENT_VERSUS;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "CLIENT_PLAYER_MOVE") != NULL) {
		*message_type = CLIENT_PLAYER_MOVE;
		num_of_expected_params = 1;
	}
	else if (strstr(accepted_str, "CLIENT_DISCONNECT") != NULL) {
		*message_type = CLIENT_DISCONNECT;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "SERVER_APPROVED") != NULL) {
		*message_type = SERVER_APPROVED;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "SERVER_DENIED") != NULL) {
		*message_type = SERVER_DENIED;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "SERVER_MAIN_MENU") != NULL) {
		*message_type = SERVER_MAIN_MENU;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "GAME_STARTED") != NULL) {
		*message_type = GAME_STARTED;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "TURN_SWITCH") != NULL) {
		*message_type = TURN_SWITCH;
		num_of_expected_params = 1;
	}
	else if (strstr(accepted_str, "SERVER_MOVE_REQUEST") != NULL) {
		*message_type = SERVER_MOVE_REQUEST;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "GAME_ENDED") != NULL) {
		*message_type = GAME_ENDED;
		num_of_expected_params = 1;
	}
	else if (strstr(accepted_str, "SERVER_NO_OPPONENTS") != NULL) {
		*message_type = SERVER_NO_OPPONENTS;
		num_of_expected_params = 0;
	}
	else if (strstr(accepted_str, "GAME_VIEW") != NULL) {
		*message_type = GAME_VIEW;
		num_of_expected_params = 3;
	}
	else if (strstr(accepted_str, "SERVER_OPPONENT_QUIT") != NULL) {
		*message_type = SERVER_OPPONENT_QUIT;
		num_of_expected_params = 0;
	}
	else { // error
		if (accepted_str != NULL) free(accepted_str);
		printf("Error: invalid message type\n");
		return TRNS_FAILED;
	}

	// extracting the parameters if exist //
	if (num_of_expected_params > 0) {
		ret_val = extract_params_from_msg(accepted_str, num_of_expected_params, parameters_arr);
		free(accepted_str);
		if (ret_val == STATUS_FAILURE) { return ERROR_FUNC; }
	}
	else { free(accepted_str); }

	return TRNS_SUCCEEDED;
}


/// === extract_params_from_msg ===
/// Description: This function extracts the parametrs from string message and set it in parameters array.
///				 It gets a the data that needs to be written and write it into the file.
/// parameters:
///		curr_message - pointer to string that cotains the message
///		num_of_params - number of parametrs in the given message
///		parameters_arr - pointer to buffer, will we set contain the recieved paramters from the message
///	Returns: int value - 0 if succeeded, 1 if failed
int extract_params_from_msg(char curr_message[], int num_of_params, char** parameters_arr) {
	if (NULL == curr_message || NULL == parameters_arr) {
		printf("Error: argument pointer is NULL\n");
		return STATUS_FAILURE;
	}
	char* token = NULL, * curr_token = NULL;
	char* curr_message_type = strtok_s(curr_message, ":", &token);
	char* curr_message_inputs = strtok_s(NULL, ":", &token);
	
	int i = 0;
	char* temp = NULL;
	while (i < num_of_params) {
		parameters_arr[i] = NULL;
		temp = strtok_s(curr_message_inputs, ";", &token);
		temp = strtok_s(temp, "\n", &curr_token);
		parameters_arr[i] = (char*)malloc(sizeof(char) * (strlen(temp) + 1));
		if (parameters_arr[i] == NULL) {
			printf("Error: fialed to allocate memory!\n");
			return STATUS_FAILURE;
		}
		strcpy_s(parameters_arr[i], (strlen(temp) + 1), temp);
		curr_message_inputs = NULL;
		i++;
	}
	return STATUS_SUCCESS;
}


/// === free_parameters ===
/// Description: This function frees the parameters that were allocated, from the buffer.
/// parameters:
///		parameters_arr - pointer to buffer contain the recieved paramters from the message
///	Returns: VOID
void free_parameters(char** parameters_arr) {
	for (int i = 0; i < MAX_PARAMS; i++) {
		if (parameters_arr[i] != NULL) {
			//free(parameters_arr[i]);
			parameters_arr[i] = NULL;
		}
	}
}



// -------------------------------------------------------------------------------------------------------------------
// Basic function for communication:
// -------------------------------------------------------------------------------------------------------------------


/*
 * send_buffer() uses a socket to send a buffer.
 *
 * Accepts:
 * -------
 * Buffer - the buffer containing the data to be sent.
 * BytesToSend - the number of bytes from the Buffer to send.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if sending succeeded
 * TRNS_FAILED - otherwise
 */
transfer_result_t send_buffer(const char* buffer, int bytes_to_send, SOCKET sd) {
	const char* p_curr_place = buffer;
	int bytes_transferred;
	int remaining_bytes_to_send = bytes_to_send;

	while (remaining_bytes_to_send > 0)
	{
		/* send does not guarantee that the entire message is sent */
		bytes_transferred = send(sd, p_curr_place, remaining_bytes_to_send, 0);
		if (bytes_transferred == SOCKET_ERROR)
		{
			printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}

		remaining_bytes_to_send -= bytes_transferred;
		p_curr_place += bytes_transferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/**
 * send_string() uses a socket to send a string.
 * Str - the string to send.
 * sd - the socket used for communication.
 */
transfer_result_t send_string(const char* str, SOCKET sd) {
	/* Send the the request to the server on socket sd */
	int total_str_size_in_bytes;
	transfer_result_t send_res;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	total_str_size_in_bytes = (int)(strlen(str) + 1); // terminating zero also sent	

	send_res = send_buffer(
		(const char*)(&total_str_size_in_bytes),
		(int)(sizeof(total_str_size_in_bytes)), // sizeof(int) 
		sd);

	if (send_res != TRNS_SUCCEEDED) return send_res;

	send_res = send_buffer(
		(const char*)(str),
		(int)(total_str_size_in_bytes),
		sd);

	return send_res;
}

/**
 * Accepts:
 * -------
 * recv_buffer() uses a socket to receive a buffer.
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */
transfer_result_t recv_buffer( char* output_buffer, int bytes_to_receive, SOCKET sd )
{
	char* p_curr_place = output_buffer;
	int bytes_just_transferred;
	int remaining_bytes_to_receive = bytes_to_receive;
	
	while ( remaining_bytes_to_receive > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		bytes_just_transferred = recv(sd, p_curr_place, remaining_bytes_to_receive, 0);
		if ( bytes_just_transferred == SOCKET_ERROR ) 
		{
			printf("recv() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}		
		else if ( bytes_just_transferred == 0 )
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		remaining_bytes_to_receive -= bytes_just_transferred;
		p_curr_place += bytes_just_transferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/**
 * recv_str() uses a socket to receive a string, and stores it in dynamic memory.
 *
 * Accepts:
 * -------
 * OutputStrPtr - a pointer to a char-pointer that is initialized to NULL, as in:
 *
 *		char *Buffer = NULL;
 *		recv_str( &Buffer, ___ );
 *
 * a dynamically allocated string will be created, and (*OutputStrPtr) will point to it.
 *
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */
transfer_result_t recv_str( char** p_output_str, SOCKET sd )
{
	/* Recv the the request to the server on socket sd */
	int total_str_size_in_bytes;
	transfer_result_t recv_res;
	char* StrBuffer = NULL;

	if ( ( p_output_str == NULL ) || ( *p_output_str != NULL ) )
	{
		printf("The first input to recv_str() must be " 
			   "a pointer to a char pointer that is initialized to NULL. For example:\n"
			   "\tchar* buffer = NULL;\n"
			   "\tReceiveString( &buffer, ___ )\n" );
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in 
	   an int variable ), then the string itself. */
		
	recv_res = recv_buffer( 
		(char *)( &total_str_size_in_bytes ),
		(int)( sizeof(total_str_size_in_bytes) ), // 4 bytes
		sd );

	if ( recv_res != TRNS_SUCCEEDED ) return recv_res;

	StrBuffer = (char*)malloc( total_str_size_in_bytes * sizeof(char) );

	if ( StrBuffer == NULL )
		return TRNS_FAILED;

	recv_res = recv_buffer( 
		(char *)( StrBuffer ),
		(int)( total_str_size_in_bytes), 
		sd );

	if ( recv_res == TRNS_SUCCEEDED ) 
		{ *p_output_str = StrBuffer; }
	else
	{
		free( StrBuffer );
	}
		
	return recv_res;
}
