/*
 * garbled/bmr/transport/server.h
 *
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

#include "garbled/bmr/transport/common.h"
#include "support/flex_buffer.h"

class ServerUpdatable {
public:
	virtual void clients_connected()=0;
	virtual void NewMsg(ReceivedMessage& msg, struct sockaddr_in* from)=0;
	virtual void node_aborted(struct sockaddr_in* from) =0;
};

namespace BIU
{

class Server {
public:
	Server(int port, int expected_clients, ServerUpdatable* updatable, unsigned int max_message_size);
	~Server();

	sockaddr_in* get_client_address(int id) { return &_clients_address[id]; }

	boost::thread* starter;
	std::vector<boost::thread*> listeners;

	vector<Timer> timers;

private:
	int _expected_clients;
	int *_clients;
	struct sockaddr_in* _clients_address;
	int _port;
	int _servfd;
	struct sockaddr_in _servaddr;

	ServerUpdatable* _updatable;
	unsigned int _max_message_size;

	void _start_server();
	void _listen_to_client(int id);
	bool _handle_receive_length(int id, size_t actual_length, size_t expected_length);
};

}
