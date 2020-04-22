#define DEBUG

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>
#include <iterator>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;
#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <string>

#include "Server.h"

Server::Server( int passed_port, string passed_client_name ) {
	port = passed_port ;
	client_name = passed_client_name;
}

string Server::get_last_cmd() {
	if (buffer.size() > 0 ){
		string last_cmd = buffer.front();
		std::unique_lock<std::mutex> lock(mux);
		buffer.pop();
		return last_cmd;
	}
	else
		return "buffer empty";
}

string get_instruction (string *message ){
	size_t pos = message->std::string::find('#');
	string instruction = message->std::string::substr( 0, pos ); 
	message->erase( 0, pos + 1 ); //erase read portion of message
	return instruction;
}

void Server::receive_message( ){
	int ID = client_available;
	queue <string> local_buffer;
	bool connection_active = true;
	
	#ifdef DEBUG
	cout << "receiving thread started" <<endl;
	#endif

	while ( connection_active ){ //while client is running
		char temporary_buffer[cmd_size];
		ssize_t bytes_read = recv( socket_vec[ ID-1 ] , temporary_buffer , cmd_size , 0);
		
		if (bytes_read != 0){ //if receives something
			string message (temporary_buffer); 
			
			#ifdef DEBUG
			cout << "\n\n\n\n NEW COMMAND\nchat ID: " << ID << " just received message: " << message << " from " << client_name << endl; 
			#endif
				
			string command = get_instruction ( &message ); 
			
			if ( command.std::string::compare ( "closeConnection" ) == 0 ) //if client disconnects, closes related thread
				connection_active = false; //signals thread must be closed
				
			else{ 
				if ( command.std::string::compare ( "AbortSlew" ) == 0 ){ //if client requested to start tracking
					mount.abort_slew();
				}
				
				else if ( command.std::string::compare ( "Track" ) == 0 ){ //if client requested to start tracking
					mount.start_track();
				}
				
				else if ( command.std::string::compare ( "StopTrack" ) == 0 ){ //if client requested to start tracking
					mount.stop_track();
				}
				
				else if ( command.std::string::compare ( "SlewTo" ) == 0 ){ //if client requested to start tracking
					double target_AR = std::stod ( get_instruction ( &message ) ); 
					double target_DEC = std::stod ( get_instruction ( &message ) );
					double sideral_time = std::stod ( get_instruction ( &message ) ); 
				
					mount.slew_axis ( target_AR , target_DEC, sideral_time );
				}
			}
    		}
	}
	#ifdef DEBUG
	cout << client_name << "disconnected, closing receiving thread" << endl;
	#endif
}

int Server::get_available_client() const {
	return client_available;
}

void Server::send_message (string message ) {
	for ( std::vector<int>::iterator it = socket_vec.begin(); it != socket_vec.end(); it++ ){  
		if (send( *it , message.c_str() , static_cast<int> (message.length() ) , 0) == -1)
			cerr << "server error, could not send message  \n";
		#ifdef DEBUG
		else
			cout << "just sent message:     " << message << " to " << client_name << endl;	
		#endif
	}
}


void Server::start_server () {
	
	int socket_desc , c;
	struct sockaddr_in server , client;
     
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		cerr << "Could not create socket" << endl;
		exit(1);
	}
     
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
     
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		cerr << "bind failed" <<endl;
		exit(1);
	}
	#ifdef DEBUG
	cout << "bind done" <<endl;
	#endif
	
	listen(socket_desc , 3);

	//Accept and incoming connection
	#ifdef DEBUG
	puts("Waiting for incoming connections...");
	#endif
	
	c = sizeof(struct sockaddr_in);

	while( true) {
		
		int new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		
		if ( new_socket < 0 ){
			cerr << "accept failed" << endl;
		}
		else{
			socket_vec.push_back( new_socket );
			client_available++;
			#ifdef DEBUG	
			cout << "Connection accepted. Now " << client_available << " client(s) currently connected." << endl ;
			#endif
		}
	
		thread receive_thread ( &Server::receive_message, this );
		receive_thread.detach();
	}
}

void Server::stop_server () {
	send_message("closeConnection#");
	graceful_server_degradation = true;
	
}



