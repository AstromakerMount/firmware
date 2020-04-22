#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
using std::string;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;

#include <vector>
using std::vector;


#include <queue>
using std::queue;

#include "Mount.h"

class Server {
public:
	Server( int passed_port , string passed_client_name);
	void start_server ();
	void stop_server();
	void send_message (string message) ;
	void receive_message ();
	string get_last_cmd();

	//getter functions
	int get_available_client() const;
	
private:
	Mount mount;
	int port;
	bool graceful_server_degradation {false};
	vector <int> socket_vec;
	queue <string> buffer;
	int client_available {0};
	int cmd_size {70};
	string client_name;
	mutex mux;
};



