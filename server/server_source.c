//  server_source.c  //
/**********************************************

Authors:
Dvir Katz
Matan Eckhaus Moyal

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the functions module for the server side in 7Boom project. 

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "server_source.h"
#include "socket_send_recv.h"
#include "comm_source.h"

/// === service_thread ===
/// Description: This function runs in a thread (player specific) and serves as a manager for the 7Boom game
/// Parameters:
///		num -  int for the number of the player
///	Returns: int value - 0 if succeeded, 1 if failed 
DWORD service_thread(int num) {
	time_t start, stop;
	int player_num = num ;
	char* params_to_send[MAX_PARAMS] = { NULL };
	char* params_recieved[MAX_PARAMS] = { NULL };
	int ret_val = 0, str_len = 0, check_boom_val = 0, message_type = 0;
	int timeout = DEFUALT_TIMEOUT;
	char* activity_str = NULL;
	int server_connection = TRUE;
	BOOL denied = FALSE;

	while (server_connection)  // waiting for new message
	{
		if (server_connection) { // receive client response
			free_parameters(params_recieved);
			ret_val = recv_message(players_socket[player_num], &message_type, params_recieved, timeout);
			timeout = DEFUALT_TIMEOUT;
			if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
		}
		if (server_connection) { // deals with client response
			free_parameters(params_to_send);

			switch (message_type) {
			case CLIENT_REQUEST: 
				str_len = snprintf(NULL, 0, "Thread_log_%s.txt", params_recieved[0]);
				activity_str = (char*)malloc(sizeof(char) * str_len + 1);
				if (NULL == activity_str) {
					printf("Error: pointer is NULL, malloc failed\n");
					if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
					return STATUS_FAILURE;
				}
				snprintf(activity_str, str_len + 1, "Thread_log_%s.txt", params_recieved[0]);
				// -----
				h_log_file[player_num] = CreateFileA(activity_str, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (h_log_file[player_num] == INVALID_HANDLE_VALUE) {	// If failed to open file
					printf("Error: Failed to open file! The error code is %d\n", GetLastError());
					free(activity_str);
					if (ret_val == STATUS_FAILURE) { return STATUS_FAILURE; }
					return STATUS_FAILURE;
				}
				log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "received", "from", "client", message_type, params_recieved);
				if (name_validation(params_recieved[0], players_socket[player_num], player_num) == STATUS_FAILURE) { // player's name exists or not valid
					ret_val = send_message(players_socket[player_num], SERVER_DENIED, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_DENIED, NULL);
					server_connection=FALSE;
					denied = TRUE;
				}
				else { // name is OK - send approval:
					ret_val = send_message(players_socket[player_num], SERVER_APPROVED, NULL);
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_APPROVED, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					ret_val = send_message(players_socket[player_num], SERVER_MAIN_MENU, NULL);
					timeout = TEN_MIN; // Wait 10 minutes when in the menu 
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_MAIN_MENU, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				}
				break;
			
			case CLIENT_VERSUS:  // After game started need to change
				log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "received", "from", "client", message_type, params_recieved);
				game_status_update(player_num, UP);
				start = time(NULL);
				do {
					stop = time(NULL);
				} while (!(ready_to_play.player_1 && ready_to_play.player_2) && ((stop-start)<= VERSUS_REACT_TIME));

				if (ready_to_play.player_1 && ready_to_play.player_2)  // check if 2 players are ready to play
				{
					ret_val = send_message(players_socket[player_num], GAME_STARTED, NULL);
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", GAME_STARTED, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					free_parameters(params_to_send);
					timeout = TEN_MIN; // Wait 10 minutes to move
					if (user_list[player_num].my_turn == TRUE) { // my Player's turn - send messages:
						params_to_send[0] = allocate_str_buffer(user_list[player_num].user_name);
						ret_val = send_message(players_socket[PLAYER_1], TURN_SWITCH, params_to_send);
						log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", TURN_SWITCH, params_to_send);
						if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
						ret_val = send_message(players_socket[PLAYER_2], TURN_SWITCH, params_to_send);
						log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", TURN_SWITCH, params_to_send);
						if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
						ret_val = send_message(players_socket[player_num], SERVER_MOVE_REQUEST, NULL);
						log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_MOVE_REQUEST, NULL);
						if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					}
				}
				else { // there are no other players:
					game_status_update(player_num, DOWN);
					ret_val = send_message(players_socket[player_num], SERVER_NO_OPPONENTS, NULL);
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_NO_OPPONENTS, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					ret_val = send_message(players_socket[player_num], SERVER_MAIN_MENU, NULL);
					timeout = TEN_MIN;// Wait 10 minutes when in the menu
					log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", SERVER_MAIN_MENU, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				}
				break;
			
			case CLIENT_PLAYER_MOVE: 
				log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "received", "from", "client", message_type, params_recieved);
				params_to_send[1]= allocate_str_buffer(params_recieved[0]); // Saving the recieved player move
				if (user_list[PLAYER_1].my_turn == TRUE) // Check which player made the move
					params_to_send[0] = allocate_str_buffer(user_list[PLAYER_1].user_name);
				else
					params_to_send[0] = allocate_str_buffer(user_list[PLAYER_2].user_name);
				check_boom_val = check_boom(params_recieved[0]);
				if (check_boom_val ==TRUE) // Check if the move is valid
					params_to_send[2] = allocate_str_buffer("CONT");
				else if(check_boom_val == FALSE)
					params_to_send[2] = allocate_str_buffer("END");
				else {
					print_and_log_error(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "service_thread", "check_boom was failed",FALSE);
					return STATUS_FAILURE; // changed to gracefull
				}
				ret_val = send_message(players_socket[PLAYER_1], GAME_VIEW,params_to_send);
				log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", GAME_VIEW, params_to_send);
				if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				ret_val = send_message(players_socket[PLAYER_2], GAME_VIEW, params_to_send);
				log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", GAME_VIEW, params_to_send);
				if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }

				if (!strcmp(params_to_send[2], "END")) { // Game is over
					if (user_list[PLAYER_1].my_turn == TRUE) { // Check who lost the game
						free_parameters(params_to_send);
						params_to_send[0] = allocate_str_buffer(user_list[PLAYER_2].user_name); // The name of the winner
					}
					else {
						free_parameters(params_to_send);
						params_to_send[0] = allocate_str_buffer(user_list[PLAYER_1].user_name); // The name of the winner
					} // Sending the winner's name
					ret_val = send_message(players_socket[PLAYER_1], GAME_ENDED, params_to_send); 
					log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", GAME_ENDED, params_to_send);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					Sleep(2000);
					ret_val = send_message(players_socket[PLAYER_2], GAME_ENDED, params_to_send);
					log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", GAME_ENDED, params_to_send);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					game_ended(player_num);

					ret_val = send_message(players_socket[PLAYER_1], SERVER_MAIN_MENU, NULL);
					log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", SERVER_MAIN_MENU, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					ret_val = send_message(players_socket[PLAYER_2], SERVER_MAIN_MENU, NULL);
					timeout = TEN_MIN;
					log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", SERVER_MAIN_MENU, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				}
				else { // If the move is valid, we need to switch turn
					free_parameters(params_to_send);
					if (user_list[PLAYER_1].my_turn == TRUE) {
						user_list[PLAYER_1].my_turn = FALSE;
						user_list[PLAYER_2].my_turn = TRUE;
						params_to_send[0] = allocate_str_buffer(user_list[PLAYER_2].user_name);
					}
					else {
						user_list[PLAYER_1].my_turn = TRUE;
						user_list[PLAYER_2].my_turn = FALSE;
						params_to_send[0] = allocate_str_buffer(user_list[PLAYER_1].user_name);
					}
					ret_val = send_message(players_socket[PLAYER_1], TURN_SWITCH, params_to_send);
					log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", TURN_SWITCH, params_to_send);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					ret_val = send_message(players_socket[PLAYER_2], TURN_SWITCH, params_to_send);
					log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", TURN_SWITCH, params_to_send);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					free_parameters(params_to_send);
					if (user_list[PLAYER_1].my_turn == TRUE) {// Request from the appropriate player to send his move
						ret_val = send_message(players_socket[PLAYER_1], SERVER_MOVE_REQUEST, NULL);
						timeout = TEN_MIN;
						log_messages(h_log_file[PLAYER_1], log_file_mutex[PLAYER_1], log_fails_counter[PLAYER_1], "sent", "to", "client", SERVER_MOVE_REQUEST, NULL);
						if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					}
					else {
						ret_val = send_message(players_socket[PLAYER_2], SERVER_MOVE_REQUEST, NULL);
						timeout = TEN_MIN;
						log_messages(h_log_file[PLAYER_2], log_file_mutex[PLAYER_2], log_fails_counter[PLAYER_2], "sent", "to", "client", SERVER_MOVE_REQUEST, NULL);
						if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
					}
				}	
				break;
			
			case CLIENT_DISCONNECT: 
				log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "received", "from", "client", message_type, params_to_send);
				if (in_game_players == 2) {
					ret_val = send_message(players_socket[!player_num], SERVER_OPPONENT_QUIT, NULL); // Sending the other player that the client disconnect
					log_messages(h_log_file[!player_num], log_file_mutex[!player_num], log_fails_counter[!player_num], "sent", "to", "client", SERVER_OPPONENT_QUIT, NULL);
					if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				}
				ret_val = send_message(players_socket[player_num], CLIENT_DISCONNECT, NULL);  // Part of gracefull disconnection
				log_messages(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "sent", "to", "client", CLIENT_DISCONNECT, NULL);
				printf("Player disconnected. Exiting.\n");
				if (log_activity(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "Player disconnected. Exiting.\n", FALSE))
					print_and_log_error(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "server_thread", "log_activity was failed", FALSE);
				if (TRNS_SUCCEEDED != ret_val) { server_connection = FALSE; break; }
				reset_game(player_num,denied);
				server_connection = FALSE;
				break;
			
			}
		}
	}
	if (!connected) return STATUS_FAILURE;
	if (log_fails_counter[player_num] != 0) { print_and_log_error(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "service_thread", "log file: had errors while running", FALSE); }
	if (denied == TRUE) { // In case the server denied the player
		reset_game(player_num, denied);
		return STATUS_SUCCESS;
	}
	if (ret_val == TRNS_SUCCEEDED)
		return STATUS_SUCCESS;
	else { // In case of transmission failed or disconnected (even abrupt)
		if (num_of_players == 2) {
			ret_val = send_message(players_socket[!player_num], SERVER_OPPONENT_QUIT, NULL); // Sending the other player that the client disconnect
			log_messages(h_log_file[!player_num], log_file_mutex[!player_num], log_fails_counter[!player_num], "sent", "to", "client", SERVER_OPPONENT_QUIT, NULL);
			ret_val = send_message(players_socket[!player_num], SERVER_MAIN_MENU, NULL); // Because the opponent disconnect, the player need to decide what to do next
			timeout = TEN_MIN;
			log_messages(h_log_file[!player_num], log_file_mutex[!player_num], log_fails_counter[!player_num], "sent", "to", "client", SERVER_MAIN_MENU, NULL);
		}
		printf("Player disconnected. Exiting.\n");
		ret_val = log_activity(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "Player disconnected. Exiting.\n", FALSE);
		if (ret_val)
			print_and_log_error(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "server_thread", "log_activity was failed", FALSE);
		reset_game(player_num,denied);
		if (TRNS_SUCCEEDED != ret_val) {
			printf("Cannot send message to second Player.\n");
			ret_val = log_activity(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "Cannot send message to second Player.\n", FALSE);
			if (ret_val) {
				print_and_log_error(h_log_file[player_num], log_file_mutex[player_num], log_fails_counter[player_num], "service_thread", "log_activity was failed", FALSE);
			}
		}
		return STATUS_FAILURE;
	}
}

/// === disconnection_func ===
/// Description: This function runs until exit input from server occured and then close everything and escape server
/// Parameters:
///		lpParam - not in use
///	Returns: int value - 0 if succeeded
DWORD disconnection_func(LPVOID lpParam) {
	char exit_code[LEN_MESSAGE];
	do
	{
		while (!_kbhit());
		fgets(exit_code, LEN_MESSAGE, stdin);
		if (!_stricmp(exit_code, "exit\n")) {
			connected = FALSE;
			WSACleanup();
			Sleep(500);
			for (int i = 0; i < NUM_OF_WORKER_THREADS; i++) { // We cannot Close handle that in use so if it in use terminate it
				if (closesocket(players_socket[i]) == SOCKET_ERROR)
					printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());;
				if (thread_handles[i]!=NULL)
					TerminateThread(thread_handles[i], 0);
				if (log_file_mutex[i] != NULL)
						TerminateThread(log_file_mutex[i], 0);
				if (h_log_file[i] != NULL)
						TerminateThread(h_log_file[i], 0);
			}
			if (game_status != NULL)
					TerminateThread(game_status, 0);
			if (end_game != NULL)
					TerminateThread(end_game, 0);
		}
	} while (connected);
	return STATUS_SUCCESS;
}

/// === find_first_unused_thread_slot ===
/// Description: This program finds the first slot for unused thread in the threads' list
/// Parameters:
///		port_num - char pointer to the number of server port
///	Returns: int value - the index of the first slot for unused thread in the threads' list
int find_first_unused_thread_slot(){
 
	int ind;
	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++)	{
		if (thread_handles[ind] == NULL)
			break;
		else {
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(thread_handles[ind], 0);
			// Need to check if Res is not WAIT_OBJECT_0 (Error or something else) !!!
			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(thread_handles[ind]);
				thread_handles[ind] = NULL;
				break;
			}
			if (Res == WAIT_FAILED)
			{
				printf("%d", GetLastError());
			}
		}
	}
	return ind; // if index == NUM_OF_WORKER_THREADS (3) so no unused slot
}

/// === clean_up_worker_threads ===
/// Description: This program closes the sockets and handles for the working threads
/// Parameters:
///		None
///	Returns: Void
void clean_up_worker_threads(){
	int ind;

	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++) {
		if (thread_handles[ind] != NULL) 	{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(thread_handles[ind], INFINITE);

			if (Res == WAIT_OBJECT_0) {
				CloseHandle(thread_handles[ind]);
				thread_handles[ind] = NULL;
				break;
			}
			else	{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/// === name_validation ===
/// Description: This function validates the username that the client has inserted.
///				 It checks if the name is unique and sets the turns for the players according the order the names were sent
/// Parameters:
///		user_name - char pointer to the user name to be checked
/// 	t_socket - socket for the connection between the client and the server
///		num_player - int for the client number (index)
///	Returns: int value - 0 if succeeded, 1 if failed
int name_validation(char* user_name, SOCKET t_socket,int num_player) {
	
	int ret_val;
	BOOL first_to_play=TRUE;
	if (user_name == NULL) 	{
		print_and_log_error(h_log_file[num_player], log_file_mutex[num_player], log_fails_counter[num_player], "name_validation", "argument pointer is NULL", FALSE);
		return STATUS_FAILURE;
	}
	if (in_game_players == 2) {
		printf("There are already 2 players in game!\n");
		
		return STATUS_FAILURE;
	}
	else {
		ret_val=WaitForSingleObject(game_status, INFINITE);
		if(ret_val==WAIT_FAILED)
		{
			print_and_log_error(h_log_file[num_player], log_file_mutex[num_player], log_fails_counter[num_player], "name_validation", "Wait Failed", FALSE);
			return STATUS_FAILURE;
		}
		for (int i = 0; i < NUM_OF_WORKER_THREADS; i++) {
			if (strcmp(user_name, user_list[i].user_name) == 0) { //if this name is already in use
				if (!ReleaseMutex(game_status))
				{
					print_and_log_error(h_log_file[num_player], log_file_mutex[num_player], log_fails_counter[num_player], "name_validation", "Relasing mutex", FALSE);
				}
				printf("Not a unique username!\n");
				return STATUS_FAILURE;
			}
			if (user_list[i].my_turn == TRUE)
				first_to_play = FALSE;
		}
		if (strlen(user_list[num_player].user_name) == 0) { // finding place for the new player and save relevant information on him
			strcpy(user_list[num_player].user_name, user_name);
			user_list[num_player].user_socket = t_socket;
			if (first_to_play) {
				user_list[num_player].my_turn = TRUE;
			}
			else {
				user_list[num_player].my_turn = FALSE;
			}
			turn = 1;
			in_game_players += 1;
			if (!ReleaseMutex(game_status))
			{
				print_and_log_error(h_log_file[num_player], log_file_mutex[num_player], log_fails_counter[num_player], "name_validation", "Relasing mutex", FALSE);
			}
			return STATUS_SUCCESS;
		}
	}
	return STATUS_FAILURE;
}

/// === game_status_update ===
/// Description: This program updates (in a ready struct)that a player is ready/not ready to play
/// Parameters:
///		player_num - int value - the player number
///		mode - int value - UP for setting that the player is ready / DOWN for setting it NOT ready
///	Returns: Void
void game_status_update(int player_num,int mode) {
	WaitForSingleObject(game_status, INFINITE); // wait_object0
	if (player_num == PLAYER_1 && mode == UP)
		ready_to_play.player_1 = TRUE;
	else if (player_num == PLAYER_2 && mode == UP)
		ready_to_play.player_2 = TRUE;
	else if (player_num == PLAYER_1 && mode == DOWN)
		ready_to_play.player_1 = FALSE;
	else if (player_num == PLAYER_1 && mode == DOWN)
		ready_to_play.player_2 = FALSE;
	ReleaseMutex(game_status);
}

/// === check_boom ===
/// Description: This function checks if the user_input fits the current turn of the game.
///				 If the current turn contains 7 or multiplicand of 7 - it should be "BOOM"
///				 Otherwise the input should be the next number in order
/// Parameters:
///		user_input - char pointer to the user input for this turn
///	Returns: int value - 0 if is correct, non 0 otherwise
int check_boom(char* user_input) {
	if (NULL == user_input) {
		printf("Error: argument pointer is NULL\n");
		return ERROR_FUNC;
	}
	char *curr_number_str=NULL;
	int str_len = 0;
	str_len = snprintf(NULL, 0, "%d", turn);
	curr_number_str = (char*)malloc(sizeof(char) * str_len + 1);
	if (NULL == curr_number_str) {
		printf("Error: pointer is NULL, malloc failed\n");
		return ERROR_FUNC;
	}
	snprintf(curr_number_str, str_len + 1, "%d", turn);
	int is_correct = 0;
	if (NULL != strstr(curr_number_str, "7") || turn % 7 == 0) {
		// Boom
		is_correct = !_stricmp("Boom", user_input); 
	}
	else {
		//next number
		is_correct = !strcmp(curr_number_str, user_input);
	}
	turn++; // promote the next num
	free(curr_number_str);
	return is_correct;
}

/// === game_ended ===
/// Description: This program updates (when game is ended) that the players 
///				 are not ready and by default sets the turns for the next game
/// Parameters: num - int number for determetion of log_file in case of fail
///
///	Returns: Void
void game_ended(int num) {
	DWORD just_one_update; // We want just one thread to update shared sources
	just_one_update = WaitForSingleObject(end_game, INFINITE);
	switch (just_one_update)
	{
	case WAIT_OBJECT_0:
		turn = 1;
		user_list[PLAYER_1].my_turn = TRUE;
		user_list[PLAYER_2].my_turn = FALSE;
		ready_to_play.player_1 = FALSE;
		ready_to_play.player_2 = FALSE;
		if (!ReleaseMutex(end_game))
		{
			print_and_log_error(h_log_file[num], log_file_mutex[num], log_fails_counter[num], "game_ended", "Releasing mutex", FALSE);
		}
		break;
	
	case WAIT_FAILED:
		print_and_log_error(h_log_file[num], log_file_mutex[num], log_fails_counter[num], "game_ended", "mutex - Wait Failed", FALSE);
		break;
	
	}
}

/// === reset_game ===
/// Description: This program resets the game by setting that the players 
///				 are not ready, 'frees' the name of the current player and 
///  	         closes the relevant sockets and handles
/// Parameters:
///		player_to_reset - int value for the number of the player that has quited
///	Returns: Void
void reset_game(int player_to_reset,BOOL denied) {
	int ret_val = 0;
	DWORD just_one_update; // We want just one thread to update shared sources
	just_one_update = WaitForSingleObject(game_status, INFINITE);
	switch (just_one_update)
	{
	case WAIT_OBJECT_0:
		if (!denied)
		{
			turn = 1;
			ready_to_play.player_1 = FALSE;
			ready_to_play.player_2 = FALSE;
			in_game_players -= 1;
		}
		user_list[player_to_reset].my_turn = FALSE;
		strcpy(user_list[player_to_reset].user_name, "");
		if (closesocket(user_list[player_to_reset].user_socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());;
		ret_val = CloseHandle(thread_handles[player_to_reset]);
		if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
		thread_handles[player_to_reset] = NULL;
		num_of_players -= 1;
		if (!ReleaseMutex(game_status))
		{
			printf("Error when releasing mutex");
		}
		break;
	case WAIT_FAILED:
		print_and_log_error(h_log_file[player_to_reset], log_file_mutex[player_to_reset], log_fails_counter[player_to_reset], "game_ended", "Error by Mutex", FALSE);
		break;
	}

}

/// === free_and_clean ===
/// Description: This program close all handles and mutexes if they
///				 not been closed before or when an error occured before
///				 connecting to the server
/// Parameters:
///		None
///	Returns: Void
void free_and_clean() {
	int ret_val = 0;
	for (int i = 0; i < NUM_OF_WORKER_THREADS; i++) {
		if (thread_handles[i] != NULL) ret_val=CloseHandle(thread_handles[i]);
		if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
		if (log_file_mutex[i] != NULL) ret_val = CloseHandle(log_file_mutex[i]);
		if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
		if (h_log_file[i] != NULL) ret_val = CloseHandle(h_log_file[i]);
		if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
	}
	if(game_status!=NULL) ret_val = CloseHandle(game_status);
	if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
	if (end_game != NULL) ret_val = CloseHandle(end_game);
	if (!ret_val) printf("Error Closing Handle - Error code is: % d\n", GetLastError());
}

