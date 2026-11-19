#pragma once

#include "net/sockets.h"
#include "net/ssl_sockets.h"
#include "support/exceptions.h"
#include "support/lock.h"
#include "clients/client.h"
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sodium.h>
#include <assert.h>

class AnonymousServerSocket;

/*
 * Manage the reading and writing of data from/to external clients via Sockets.
 * Generate the session keys for encryption/decryption of secret communication with external clients.
 */

class ExternalClients
{
  map<int,AnonymousServerSocket*> client_connection_servers;
  
  int party_num;

  // Maps holding per client values (indexed by unique 32-bit id)
  std::map<int,ClientSocket*> external_client_sockets;
  std::map<int, int> client_ports;

  SslService io_service;
  ClientContext* ctx;
  map<int, ClientContext*> peer_ctxs;

  Lock lock;

  public:

  ExternalClients(int party_num);
  ~ExternalClients();

  void start_listening(int portnum_base);

  int get_client_connection(int portnum_base);
  int initialize_client_connection(const string& host, int portnum, int my_client_id);

  void close_connection(int client_id);

  // return the socket for a given client or server identifier
  ClientSocket* get_socket(int socket_id);

  int get_party_num();
};
