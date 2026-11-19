/*
 * net/sender.h
 *
 */

#pragma once

#include <pthread.h>

#include "net/receiver.h"

#include "support/octet_stream.h"
#include "support/wait_queue.h"
#include "support/time_func.h"

template<class T>
class Sender : CommunicationThread
{
    T socket;
    WaitQueue<const OctetStream*> in;
    WaitQueue<const OctetStream*> out;
    pthread_t thread;

    static void* run_thread(void* sender);

    // prevent copying
    Sender(const Sender& other);

    void start();
    void stop();
    void run_with_error();
    void fail();

public:
    Timer timer;

    Sender(T socket, int other);
    ~Sender();

    T get_socket()
    {
        return socket;
    }

    void request(const OctetStream& os);
    void wait(const OctetStream& os);
};
