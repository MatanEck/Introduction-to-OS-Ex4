//  server_source.h  //
/**********************************************

Authors:
Dvir Katz
Matan Eckhaus Moyal

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the declarations module for the server side in 7Boom project. 


#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <time.h>

#define SERVER_ADDRESS_STR "127.0.0.1"
#define DISCONNECTED -1
#define NUM_OF_WORKER_THREADS 3
#define ERROR_CODE -100
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define MAX_USERNAME_LENGTH 20
#define MAX_MESSAGE_SIZE 256
#define SEND_STR_SIZE 35
#define PLAYER_1 0
#define PLAYER_2 1
#define UP 1
#define DOWN -1
#define VERSUS_REACT_TIME 15

// structs:

// user - Struct that saves all player's relevant information- who's turn it is, his name and his socket number
typedef struct user {
	char user_name[MAX_USERNAME_LENGTH];
	bool my_turn;
	SOCKET user_socket;
}user;

// ready - Struct that saves data if players are ready = TRUE, FALSE otherwise
typedef struct ready {
	BOOL player_1;
	BOOL player_2;
}ready;

// Global variables
int turn;
int ind;
int log_fails_counter[NUM_OF_WORKER_THREADS];
user user_list[NUM_OF_WORKER_THREADS];
int num_of_players , in_game_players ;
SOCKET players_socket[NUM_OF_WORKER_THREADS];
ready ready_to_play;
BOOL connected;

// thread handles
HANDLE thread_handles[NUM_OF_WORKER_THREADS];

// Mutex handles
HANDLE log_file_mutex[NUM_OF_WORKER_THREADS];
HANDLE game_status;
HANDLE end_game;

// Log files handles
HANDLE h_log_file[NUM_OF_WORKER_THREADS];

// Function Declarations 
DWORD service_thread(int num);
DWORD disconnection_func(LPVOID lpParam);
int find_first_unused_thread_slot();
void clean_up_worker_threads();
int name_validation(char* user_name, SOCKET t_socket, int num_player);
void game_status_update(int player_num, int mode);
int check_boom(char* user_input);
void game_ended(int num);
void reset_game(int player_to_reset,BOOL denied);
void free_and_clean();

// Note: see descriptions for the functions in server_source.c
#endif // SERVER_H