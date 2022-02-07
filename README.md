# Introduction-to-OS-Ex4
TCP Sockets

Project : Ex4 - 7 Boom
=======================
Notes:
*	In this program we implemented communication between Server side 
	(that have service threads for each client) and Client side - during 7Boom game.
*	For Gracefull shutdown, meaning to end communication - We used the message CLIENT_DISCONNECT.
	The client who wants to disconnect sends "CLIENT_DISCONNECT" message, 
	than the server gets is and send "CLIENT_DISCONNECT" message too, to signal that shutdown 
	communication has started.
	(We had to use the set of known messages, that's why we chose this manner).
  
  *Part of Introduction to System Programming in TAU. Assignment instructions were partially copied - All rights reserved to Tel Aviv University.
