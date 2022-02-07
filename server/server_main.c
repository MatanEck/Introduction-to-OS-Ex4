//  server_main.c  //
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


/// === main_server ===
/// Description: This program will simulate the server in the communication during 7Boom game.
///				 The function includes initialization of all events and threads, main function for the server.
/// Parameters:
///		port_num - char pointer to the number of server port
///	Returns: int value - 0 if succeeded, 1 if failed
int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Error: please insert server port to the command line\n");
		return STATUS_FAILURE;
	}
	DWORD wait_code;
	SOCKET main_socket = INVALID_SOCKET;
	unsigned long addr = 0;
	SOCKADDR_IN service;
	int bind_res, listen_res;
	int str_len = 0, ret_val = 0;
	connected = TRUE;

	int port = atoi(argv[1]);
	ready_to_play.player_1 = FALSE;
	ready_to_play.player_2 = FALSE;
	num_of_players = 0;
	in_game_players = 0;

	HANDLE disconnection_handle;

	// Initialize Winsock
	WSADATA wsaData;
	int startup_res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startup_res != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		return STATUS_FAILURE;
	}
	// Mutexes //
	// log_file_mutex - will be used when writing into files - can happen from different threads - need a mutex to settle it
	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++) {
		log_file_mutex[ind] = CreateMutex(NULL, NULL, NULL);
		if (NULL == log_file_mutex[ind]) {
			printf("Couldn't create thread\n");
			free_and_clean();
			return STATUS_FAILURE;
		}
	}
	// end_game- when sending to both players that the game has ended, the message suppose to be sent to both of them. Neccasary?
	end_game = CreateMutex(NULL, NULL, NULL);
	if (NULL == end_game) {
		printf("Couldn't create thread\n");
		free_and_clean();
		return STATUS_FAILURE;
	}
	game_status = CreateMutex(NULL, NULL, NULL);
	if (NULL == game_status) {
		printf("Couldn't create thread\n");
		free_and_clean();
		return STATUS_FAILURE;
	}

	disconnection_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)disconnection_func, ind, 0, NULL);
	if (NULL == disconnection_handle) {
		printf("Couldn't create thread\n");
		free_and_clean();
	}
	// Create a socket //
	main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (main_socket == INVALID_SOCKET) {
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		free_and_clean();
		return STATUS_FAILURE;
	}
	addr == inet_addr(SERVER_ADDRESS_STR);
	if (addr == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		if (closesocket(main_socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		free_and_clean();
		return STATUS_FAILURE;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = addr;
	service.sin_port = htons(port);
	bind_res = bind(main_socket, (SOCKADDR*)&service, sizeof(service));
	if (bind_res == SOCKET_ERROR) {
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		if (closesocket(main_socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		free_and_clean();
		return STATUS_FAILURE;
	}
	listen_res = listen(main_socket, SOMAXCONN); // Listen on the Socket.
	if (listen_res == SOCKET_ERROR) {
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		if (closesocket(main_socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		free_and_clean();
		return STATUS_FAILURE;
	}
	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (ind = 0; ind < NUM_OF_WORKER_THREADS; ind++) {
		thread_handles[ind] = NULL;
	}
	printf("Waiting for a client to connect...\n");

	// starting an infinite loop that will always waiting for new players, and accepts only two players conncted at a time
	while (connected) {
		if (num_of_players < 3) {
			// getting the socket for the player
			SOCKET accept_socket = accept(main_socket, NULL, NULL);
			if (accept_socket == INVALID_SOCKET) {
				printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
				free_and_clean();
			}
			printf("Client Connected.\n");
			//checking for available place.
			ind = find_first_unused_thread_slot();
			// saving player's socket
			user_list[ind].user_socket = accept_socket;
			players_socket[ind] = accept_socket;
			// creating thread for first player- he will wait there for the second player to connect
			thread_handles[ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)service_thread, ind, 0, NULL);
			if (NULL == thread_handles[ind]) {
				printf("Couldn't create thread\n");
				free_and_clean();
				return STATUS_FAILURE;
			}
			num_of_players += 1;
		}
		else break;
	} // while(connected) - end of the loop
	
	// Only after writing exit to Server terminal we disconnect all
	WaitForSingleObject(disconnection_handle, INFINITE);
	CloseHandle(disconnection_handle);
	return STATUS_SUCCESS;
}