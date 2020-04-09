#include <sys/socket.h>
#include <iostream>
using std::cout;
using std::endl;
#include <limits>

#include <thread>
using std::thread;

#include <queue>
using std::queue;

#include <string>
using std::string;

#include "Angle.h"
#include "Server.h"

int main()
{
	Server ASCOM_server ( 11000 , "ASCOM driver" );
	ASCOM_server.start_server();
	
	std::cout << "Press ENTER to continue...";
  	std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );


  return 0;
}
