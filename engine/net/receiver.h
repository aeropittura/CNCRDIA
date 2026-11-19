/*
 * net/receiver.h
 *
 */

#pragma once

#include <pthread.h>

#include "support/octet_stream.h"
#include "support/wait_queue.h"
#include "support/time_func.h"

class CommunicationThread
{
    int other;

protected:
    CommunicationThread(int other);
    virtual ~CommunicationThread() {}

    void run();
    virtual void run_with_error() = 0;
    virtual void fail() = 0;
};

template<class T>
class Receiver : CommunicationThread
{
    T socket;
    WaitQueue<OctetStream*> in;
    WaitQueue<OctetStream*> out;
    pthread_t thread;

    static void* run_thread(void* receiver);

    // prevent copying
    Receiver(const Receiver& other);

    void start();
    void stop();
    void run_with_error();
    void fail();

public:
    Timer timer;

    Receiver(T socket, int other);
    ~Receiver();

    T get_socket()
    {
        return socket;
    }

    void request(OctetStream& os);
    void wait(OctetStream& os);
};
