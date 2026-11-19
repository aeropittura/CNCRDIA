/*
 * clients/client.h
 *
 */

#pragma once

#include "net/ssl_sockets.h"

#ifdef NO_CLIENT_TLS
class ClientContext
{
public:
    ClientContext(string)
    {
    }
};

class ClientSocket
{
public:
    int socket;

    ClientSocket(SslService&,
            ClientContext&, int plaintext_socket, string,
            string, bool) : socket(plaintext_socket)
    {
    }

    ~ClientSocket()
    {
        close(socket);
    }
};

inline void send(ClientSocket* socket, Octet* data, size_t len)
{
    send(socket->socket, data, len);
}

inline void receive(ClientSocket* socket, Octet* data, size_t len)
{
    receive(socket->socket, data, len);
}

#else

typedef SslContext ClientContext;
typedef SslSocket ClientSocket;

#endif

/**
 * Client-side interface
 */
class Client
{
    vector<int> plain_sockets;
    ClientContext ctx;
    SslService io_service;

public:
    /**
     * Sockets for cleartext communication
     */
    vector<ClientSocket*> sockets;

    /**
     * Specification of computation domain
     */
    OctetStream specification;

    /**
     * Start a new set of connections to computing parties.
     * @param hostnames location of computing parties
     * @param port_base port base
     * @param my_client_id client identifier
     */
    Client(const vector<string>& hostnames, int port_base, int my_client_id);
    ~Client();

    /**
     * Securely input private values.
     * @param values vector of integer-like values
     */
    template<class T>
    void send_private_inputs(const vector<T>& values);

    /**
     * Securely receive output values.
     * @param n number of values
     * @returns vector of integer-like values
     */
    template<class T, class U = T>
    vector<U> receive_outputs(int n);
};
