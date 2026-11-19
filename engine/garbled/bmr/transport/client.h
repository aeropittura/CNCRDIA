/*
 * garbled/bmr/transport/client.h
 *
 */

#pragma once

#include "garbled/bmr/transport/common.h"
//#include <queue>
//#include <boost/thread.hpp>
//#include <boost/thread/mutex.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/thread.hpp>

#include "support/wait_queue.h"
#include "support/flex_buffer.h"

#define CONNECT_INTERVAL (1000000)

class ClientUpdatable {
public:
	virtual void connected_to_servers()=0;
};



namespace BIU
{

class Client {
public:
	Client(endpoint_t* servers, int numservers, ClientUpdatable* updatable, unsigned int max_message_size);
	virtual ~Client();

	void Connect();
	void Send(int id, SendBuffer& new_msg);
	void Broadcast(SendBuffer& new_msg);
	void broadcast2(SendBuffer& new_msg);
	void Stop();

private:

	WaitQueue< shared_ptr<SendBuffer> >* _message_queues;
//	std::queue<Msg>* _message_queues;
//	boost::mutex* _msg_mux;
//	boost::mutex* _thd_mux;
	unsigned int _max_message_size;
	void _send_thread(int i);
	void _send_blocking(SendBuffer& msg, int id);

	void _connect();
	void _connect_to_server(int i);

	int _numservers;
	struct sockaddr_in* _servers;
	int* _sockets;
	ClientUpdatable* _updatable;

	boost::thread_group threads;
};

}
