//  socket_send_recv.h  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex4 - 7 Boom

* *********************************************/
/// Description: This is the declarations module for the sending & reciving messages in 7Boom project. 
/// Note: Based on recitation example that was Last updated by Amnon Drory, Winter 2011.

#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS_STR "127.0.0.1"
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

// Constants:
#define STATUS_SUCCESS  0
#define STATUS_FAILURE  1
#define TRUE 1
#define FALSE 0
#define ERROR_FUNC -1
#define WANT_PLAY 1
#define EXIT_GAME 2
#define LEN_MESSAGE 256
#define MAX_PARAMS 3    
#define DEFUALT_TIMEOUT 15 // 15 sec
#define INVITE_TIMEOUT 30  // 30 sec
#define TEN_MIN 600        // 10 minutes

// Enums and Structs:
typedef enum {
    CLIENT_REQUEST, CLIENT_VERSUS, CLIENT_PLAYER_MOVE, CLIENT_DISCONNECT,
    SERVER_APPROVED, SERVER_DENIED, SERVER_MAIN_MENU, GAME_STARTED, TURN_SWITCH,
    SERVER_MOVE_REQUEST, GAME_ENDED, SERVER_NO_OPPONENTS, GAME_VIEW, SERVER_OPPONENT_QUIT
} message_type;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } transfer_result_t;

typedef enum { CONNECT_FAILED, CONNECT_DENIED, CONNECT_SUCCEEDED } connect_status;

typedef struct parameters {
    char* parameter;
    struct parameters* next;
}parameters;

typedef struct message {
    char* message_type;
    parameters* parameters;
}message;



// -------------------------------------------------------------------------------------------------------------------
// Functions for sending messages: 
// -------------------------------------------------------------------------------------------------------------------
transfer_result_t send_message(SOCKET socket, int message_type, char* parameters_arr[]);
transfer_result_t recv_message(SOCKET socket, int* message_type, char** parameters_arr, int timeout);
int extract_params_from_msg(char curr_message[], int num_of_params, char** parameters_arr);
void free_parameters(char** parameters_arr);


// -------------------------------------------------------------------------------------------------------------------
// Basic function for communication:
// -------------------------------------------------------------------------------------------------------------------
transfer_result_t send_buffer(const char* buffer, int bytes_to_send, SOCKET sd);
transfer_result_t send_string(const char* str, SOCKET sd);
transfer_result_t recv_buffer(char* output_buffer, int bytes_to_receive, SOCKET sd);
transfer_result_t recv_str(char** p_output_str, SOCKET sd);

// Note: see descriptions for the functions in socket_send_recv.c
#endif // SOCKET_SEND_RECV_TOOLS_H