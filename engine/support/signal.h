/*
 * support/signal.h
 *
 */

#pragma once

#include <pthread.h>

class Signal
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;

public:
    Signal();
    virtual ~Signal();
    void lock();
    void unlock();
    void wait();
    int wait(int seconds);
    void broadcast();
};
