
#include "net/player.h"
#include "net/ssl_sockets.h"
#include "support/exceptions.h"
#include "support/int.h"
#include "support/network_options.h"
#include "net/server.h"
#include "net/server_socket.h"
#include "net/exchanger.h"
#include "vm/online_options.h"

#include <sys/select.h>
#include <utility>
#include <assert.h>

using namespace std;

void Names::init(int player, int pnb, int my_port, const char* servername,
    bool setup_socket)
{
  player_no=player;
  portnum_base=pnb;
  setup_names(servername, my_port);
  if (setup_socket)
    setup_server();
}

Names::Names(int player, int nplayers, const string& servername, int pnb,
    int my_port) :
    Names()
{
  Server::start_networking(*this, player, nplayers, servername, pnb, my_port);
}

void Names::init(int player,int pnb,vector<string> nms)
{
  player_no=player;
  portnum_base=pnb;
  nplayers=nms.size();
  names=nms;
  setup_ports();
  setup_server();
}

// initialize names from file, no Server.x coordination.
void Names::init(int player, int pnb, const string& filename, int nplayers_wanted)
{
  ifstream hostsfile(filename.c_str());
  if (hostsfile.fail())
  {
     stringstream ss;
     ss << "Error opening " << filename << ". See HOSTS.example for an example.";
     throw file_error(ss.str().c_str());
  }
  player_no = player;
  nplayers = 0;
  portnum_base = pnb;
  string line;
  ports.clear();
  while (getline(hostsfile, line))
  {
    if (line.length() > 0 && line.at(0) != '#') {
      auto pos = line.find(':');
      if (pos == string::npos)
      {
        names.push_back(line);
        ports.push_back(default_port(nplayers));
      }
      else
      {
        names.push_back(line.substr(0, pos));
        int port;
        stringstream(line.substr(pos + 1)) >> port;
        ports.push_back(port);
      }
      nplayers++;
      if (nplayers_wanted > 0 and nplayers_wanted == nplayers)
        break;
    }
  }
  if (nplayers_wanted > 0 and nplayers_wanted != nplayers)
    exit_error("not enough hosts in " + filename);

  if (OnlineOptions::singleton.has_option("debug_networking"))
    {
      cerr << "Got list of " << nplayers << " players from file: " << endl;
      for (unsigned int i = 0; i < names.size(); i++)
        cerr << "    " << names[i] << ":" << ports[i] << endl;
    }

  setup_server();
}

Names::Names(ez::EzOptionParser& opt, int argc, const char** argv,
    int default_nplayers) : Names()
{
  NetworkOptionsWithNumber network_options(opt, argc, argv, default_nplayers, true);
  opt.add(
          "", // Default.
          1, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "This player's number (required)", // Help description.
          "-p", // Flag token.
          "--player" // Flag token.
  );
  opt.parse(argc, argv);
  opt.get("-p")->getInt(player_no);
  vector<string> missing;
  if (not opt.got_required(missing))
  {
      string usage;
      opt.getUsage(usage);
      cerr << usage;
      exit(1);
  }
  network_options.start_networking(*this, player_no);
}

void Names::setup_ports()
{
  ports.resize(nplayers);
  for (int i = 0; i < nplayers; i++)
    ports[i] = default_port(i);
}

void Names::setup_names(const char *servername, int my_port)
{
  if (my_port == DEFAULT_PORT)
    my_port = default_port(player_no);

  int socket_num;
  int pn = portnum_base;

  try
  {
    set_up_client_socket(socket_num, servername, pn);
  }
  catch (exception& e)
  {
    exit_error(
        string("cannot reach coordination server: ") + e.what());
  }

  OctetStream("P" + to_string(player_no)).Send(socket_num);

  if (OnlineOptions::singleton.has_option("debug_networking"))
    cerr << "Sent " << player_no << " to " << servername << ":" << pn << endl;

  // Send my name
  sockaddr_in address;
  socklen_t size = sizeof address;
  getsockname(socket_num, (sockaddr*)&address, &size);
  char* my_name = inet_ntoa(address.sin_addr);
  OctetStream(my_name).Send(socket_num);
  send(socket_num,(Octet*)&my_port,4);

  if (OnlineOptions::singleton.has_option("debug_networking"))
    {
      fprintf(stderr, "My Name = %s\n",my_name);
      cerr << "My number = " << player_no << endl;
    }

  // Now get the set of names
  try
  {
    OctetStream os;
    os.Receive(socket_num);
    os.get(names);
    os.get(ports);
  }
  catch (exception& e)
  {
    exit_error(string("error in network setup: ") + e.what());
  }

  if (names.size() != ports.size())
    exit_error("invalid network setup");
  nplayers = names.size();


  if (OnlineOptions::singleton.has_option("debug_networking"))
    for (int i = 0; i < nplayers; i++)
      cerr << "Player " << i << " is running on machine " << names[i] << endl;

  close_client_socket(socket_num);
}


void Names::setup_server()
{
  server = new ServerSocket(ports.at(player_no));
  server->init();
}

void Names::set_server(ServerSocket* socket)
{
  assert(not server);
  server = socket;
}


Names::Names(const Names& other)
{
  if (other.server != 0)
      throw runtime_error("Can copy Names only when uninitialized");
  player_no = other.player_no;
  nplayers = other.nplayers;
  portnum_base = other.portnum_base;
  names = other.names;
  ports = other.ports;
  server = 0;
}

Names::Names(int my_num, int num_players) :
    nplayers(num_players), portnum_base(-1), player_no(my_num), server(0)
{
}

Names::~Names()
{
  if (server != 0)
    delete server;
}


Player::Player(const Names& nms) :
        PlayerBase(nms.my_num()), N(nms)
{
  nplayers=nms.nplayers;
  player_no=nms.player_no;
  thread_statistics.resize(nplayers);
}


template<class T>
MultiPlayer<T>::MultiPlayer(const Names& nms, const string& id) :
        Player(nms), id(id), send_to_self_socket(0)
{
  sockets.resize(nms.num_players());
}


PlainPlayer::PlainPlayer(const Names& nms, const string& id) :
        MultiPlayer<int>(nms, id)
{
  if (nms.num_players() > 1)
    setup_sockets(nms.names, nms.ports, id, *nms.server);
}


PlainPlayer::PlainPlayer(const Names& nms, int id_base) :
        PlainPlayer(nms, to_string(id_base))
{
}

PlainPlayer::~PlainPlayer()
{
  if (num_players() > 1)
    {
      /* Close down the sockets */
      for (auto socket : sockets)
        close_client_socket(socket);
      close_client_socket(send_to_self_socket);
    }
}

template<class T>
MultiPlayer<T>::~MultiPlayer()
{
}

Player::~Player()
{
#ifdef VERBOSE
  for (auto& x : thread_statistics)
    x.print();
#endif
  for (auto child : children)
    delete child.second;
}

PlayerBase::~PlayerBase()
{
#ifdef VERBOSE
  communication_statistics.print();
  if (timer.elapsed() > 0)
    cerr << "Receiving took " << timer.elapsed() << " seconds" << endl;
#endif
}



// Set up nmachines client and server sockets to send data back and fro
//   A machine is a server between it and player i if i>=my_number
//   Can also communicate with myself, but only with send_to and receive_from
void PlainPlayer::setup_sockets(const vector<string>& names,
        const vector<int>& ports, const string& id_base, ServerSocket& server)
{
    if (OnlineOptions::singleton.has_option("debug_connection_id"))
        cerr << "Setting up connection " << id_base << endl;

    sockets.resize(nplayers);
    // Set up the client side
    for (int i=0; i<=player_no; i++) {
        auto pn=id_base+"P"+to_string(player_no);
        if (i==player_no) {
          const char* localhost = "127.0.0.1";
#ifdef DEBUG_NETWORKING
          fprintf(stderr,
              "Setting up send to self socket to %s:%d with id %s\n",
              localhost, ports[i], pn.c_str());
#endif
          try
          {
            set_up_client_socket(sockets[i],localhost,ports[i]);
          }
          catch (exception& e)
          {
            exit_error("cannot connect to myself, "
                "maybe check your firewall configuration");
          }
        } else {
#ifdef DEBUG_NETWORKING
            fprintf(stderr, "Setting up client to %s:%d with id %s\n",
                names[i].c_str(), ports[i], pn.c_str());
#endif
          set_up_client_socket(sockets[i],names[i].c_str(),ports[i]);
        }
        OctetStream(pn).Send(sockets[i]);
    }
    send_to_self_socket = sockets[player_no];
    // Setting up the server side
    for (int i=player_no; i<nplayers; i++) {
        auto id=id_base+"P"+to_string(i);
#ifdef DEBUG_NETWORKING
        fprintf(stderr,
            "As a server, waiting for client with id %s to connect.\n",
            id.c_str());
#endif
        sockets[i] = server.get_connection_socket(id);
    }

    for (int i = 0; i < nplayers; i++) {
        // timeout of 5 minutes
        struct timeval tv;
        tv.tv_sec = 300;
        tv.tv_usec = 0;
        int fl = setsockopt(sockets[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
        if (fl<0) { error("set_up_socket:setsockopt");  }
    }
}


vector<int> Player::balanced_others() const
{
  vector<int> res;
  for (int offset = 1; offset <= num_players() / 2; offset++)
    {
      int others[] = { get_player(offset), get_player(-offset) };
      if (my_num() % (2 * offset) < offset)
        swap(others[0], others[1]);

      if (num_players() % 2 == 0 and offset == num_players() / 2)
        res.push_back(others[0]);
      else
        for (int i = 0; i < 2; i++)
          res.push_back(others[i]);
    }
  return res;
}


template<class T>
void MultiPlayer<T>::send_long(int i, long a) const
{
  TimeScope ts(communication_statistics["Sending by number"].add(sizeof(long)));
  send(sockets[i], (Octet*)&a, sizeof(long));
  sent += sizeof(long);
}

template<class T>
long MultiPlayer<T>::receive_long(int i) const
{
  long res;
  receive(sockets[i], (Octet*)&res, sizeof(long));
  return res;
}



void Player::send_to(int player,const OctetStream& o) const
{
  TimeScope ts(communication_statistics["Sending directly"].add(o, player));
  send_to_no_statistics(player, o);
  sent += o.get_length();
}

template<class T>
void MultiPlayer<T>::send_to_no_statistics(int player,const OctetStream& o) const
{
  T socket = socket_to_send(player);
  o.Send(socket);
}


void Player::send_all(const OctetStream& o) const
{
  TimeScope ts(communication_statistics["Sending to all"].add(o));
  for (int i=0; i<nplayers; i++)
     { if (i!=player_no)
         send_to_no_statistics(i, o);
     }
  sent += o.get_length() * (num_players() - 1);
}


void Player::send_all(const vector<OctetStream>& os) const
{
  for (int i=0; i<nplayers; i++)
     { if (i!=player_no)
         send_to(i, os[i]);
     }
}


void Player::receive_all(vector<OctetStream>& os) const
{
  os.resize(num_players());
  for (int j = 0; j < num_players(); j++)
    if (j != my_num())
      receive_player(j, os[j]);
}


void Player::receive_player(int i,OctetStream& o) const
{
  TimeScope ts(timer);
  receive_player_no_statistics(i, o);
  communication_statistics["Receiving directly"].add(o, ts, i);
}

template<class T>
void MultiPlayer<T>::receive_player_no_statistics(int i,OctetStream& o) const
{
  o.reset_write_head();
  o.Receive(sockets[i]);
}

void Player::receive_player(int i, FlexBuffer& buffer) const
{
  OctetStream os;
  receive_player(i, os);
  buffer = os;
}

size_t PlainPlayer::send_no_statistics(int player,
        const PlayerBuffer& buffer, bool block) const
{
  if (block)
    {
      send(socket(player), buffer.data, buffer.size);
      return buffer.size;
    }
  else
    return send_non_blocking(socket(player), buffer.data, buffer.size);
}

size_t PlainPlayer::receive_no_statistics(int player,
        const PlayerBuffer& buffer, bool block) const
{
    if (block)
      {
        receive(socket(player), buffer.data, buffer.size);
        return buffer.size;
      }
    else
      return receive_non_blocking(socket(player), buffer.data, buffer.size);
}


void Player::send_relative(const vector<OctetStream>& os) const
{
  assert((int)os.size() == num_players() - 1);
  for (int offset = 1; offset < num_players(); offset++)
    send_relative(offset, os[offset - 1]);
}

void Player::send_relative(int offset, const OctetStream& o) const
{
  send_to(positive_modulo(my_num() + offset, num_players()), o);
}

void Player::receive_relative(vector<OctetStream>& os) const
{
  assert((int)os.size() == num_players() - 1);
  for (int offset = 1; offset < num_players(); offset++)
    receive_relative(offset, os[offset - 1]);
}

void Player::receive_relative(int offset, OctetStream& o) const
{
  receive_player(positive_modulo(my_num() + offset, num_players()), o);
}

template<class T>
void MultiPlayer<T>::exchange_no_statistics(int other, const OctetStream& o, OctetStream& to_receive) const
{
  o.exchange(sockets[other], sockets[other], to_receive);
}

void Player::exchange(int other, const OctetStream& o, OctetStream& to_receive) const
{
  TimeScope ts(communication_statistics["Exchanging"].add(o, other));
  exchange_no_statistics(other, o, to_receive);
  sent += o.get_length();
}


void Player::exchange(int player, OctetStream& o) const
{
  exchange(player, o, o);
}

void Player::exchange_relative(int offset, OctetStream& o) const
{
  exchange(get_player(offset), o);
}


template<class T>
void MultiPlayer<T>::pass_around_no_statistics(const OctetStream& o, OctetStream& to_receive, int offset) const
{
  o.exchange(sockets.at(get_player(offset)), sockets.at(get_player(-offset)), to_receive);
}

void Player::pass_around(OctetStream& o, OctetStream& to_receive, int offset) const
{
  TimeScope ts(communication_statistics["Passing around"].add(o));
  pass_around_no_statistics(o, to_receive, offset);
  sent += o.get_length();
}


/* This is deliberately weird to avoid problems with OS max buffer
 * size getting in the way
 */
template<class T>
void MultiPlayer<T>::broadcast_receive_no_statistics(vector<OctetStream>& o) const
{
  vector<bool> parties(num_players(), true);
  partial_broadcast_no_statistics(parties, parties, o);
}


template<class T>
void MultiPlayer<T>::partial_broadcast_no_statistics(
    const vector<bool>& senders, const vector<bool>& receivers,
    vector<OctetStream>& o) const
{
  if (o.size() != sockets.size())
    throw runtime_error("player numbers don't match");

  vector<Exchanger<T>> exchangers;
  OctetStream empty;
  for (int i=1; i<nplayers; i++)
    {
      int send_to = (my_num() + i) % num_players();
      int receive_from = (my_num() + num_players() - i) % num_players();

      if (senders.at(send_to) and receivers.at(receive_from))
        exchangers.push_back({sockets[send_to], o[my_num()],
          sockets[receive_from], o[receive_from]});
      else if (receivers.at(receive_from))
        exchangers.push_back({0, empty,
          sockets[receive_from], o[receive_from]});
      else if (senders.at(send_to))
        exchangers.push_back({sockets[send_to], o[my_num()],
          0, o[receive_from]});

      if (OnlineOptions::singleton.has_option("detailed_verbose_comm"))
        {
          if (senders.at(send_to))
            cerr << "Send " << o[my_num()].get_length() << " bytes to "
                << send_to << endl;
          if (receivers.at(receive_from))
            cerr << "Receiving from " << receive_from << endl;
        }
    }

  int left = 1;
  while (left > 0)
    {
      left = 0;
      for (auto& exchanger : exchangers)
        left += exchanger.round(false);
    }

  for (int i = 0; i < num_players(); i++)
    if (i != my_num() and not o.at(i).empty())
      throw runtime_error("unexpected information from " + to_string(i));
}

void Player::unchecked_broadcast(vector<OctetStream>& o) const
{
  TimeScope ts(communication_statistics["Broadcasting"].add(o[player_no]));
  broadcast_receive_no_statistics(o);
  sent += o[player_no].get_length() * (num_players() - 1);
}

void Player::broadcast_receive(vector<OctetStream>& o) const
{
  unchecked_broadcast(o);
    { for (int i=0; i<nplayers; i++)
        { hash_update(&ctx,o[i].get_data(),o[i].get_length()); }
    }
}


void Player::check_broadcast() const
{
  if (ctx.size == 0)
    return;
  vector<OctetStream> h(nplayers);
  h[player_no].concat(ctx.final());

  unchecked_broadcast(h);
  for (int i=0; i<nplayers; i++)
    { if (i!=player_no)
        { if (!h[i].equals(h[player_no]))
	    { throw BroadcastInvalid(); }
        }
    }
  ctx.reset();
}

void Player::send_receive_all(const vector<OctetStream>& to_send,
    vector<OctetStream>& to_receive) const
{
  send_receive_all(vector<vector<bool>>(), to_send, to_receive);
}

void Player::send_receive_all(const vector<bool>& senders,
    const vector<OctetStream>& to_send, vector<OctetStream>& to_receive) const
{
  vector<vector<bool>> channels;
  for (int i = 0; i < num_players(); i++)
    channels.push_back(vector<bool>(num_players(), senders.at(i)));
  send_receive_all(channels, to_send, to_receive);
}

void Player::send_receive_all(const vector<vector<bool>>& channels,
    const vector<OctetStream>& to_send,
    vector<OctetStream>& to_receive) const
{
  size_t data = 0;
  for (int i = 0; i < num_players(); i++)
    if (i != my_num() and (channels.empty() or channels.at(my_num()).at(i)))
      {
        data += to_send.at(i).get_length();
        if (OnlineOptions::singleton.has_option("detailed_verbose_comm"))
          cerr << "Send " << to_send.at(i).get_length() << " bytes to " << i << endl;
      }
  TimeScope ts(communication_statistics["Sending/receiving"].add(data));
  sent += data;
  send_receive_all_no_statistics(channels, to_send, to_receive);
}

void Player::partial_broadcast(const vector<bool>& senders,
    const vector<bool>& receivers, vector<OctetStream>& os) const
{
  TimeScope ts(communication_statistics["Partial broadcasting"].add(os[my_num()]));
  partial_broadcast_no_statistics(senders, receivers, os);
}

template<class T>
void MultiPlayer<T>::send_receive_all_no_statistics(
    const vector<vector<bool>>& channels, const vector<OctetStream>& to_send,
    vector<OctetStream>& to_receive) const
{
  to_receive.resize(num_players());
  for (int offset = 1; offset < num_players(); offset++)
    {
      int receive_from = get_player(-offset);
      int send_to = get_player(offset);
      bool receive = channels.empty() or channels[receive_from][my_num()];
      if (channels.empty() or channels[my_num()][send_to])
        {
          if (receive)
            pass_around_no_statistics(to_send[send_to], to_receive[receive_from], offset);
          else
            send_to_no_statistics(send_to, to_send[send_to]);
        }
      else if (receive)
        receive_player_no_statistics(receive_from, to_receive[receive_from]);
    }
}


ThreadPlayer::ThreadPlayer(const Names& nms, const string& id_base) :
    PlainPlayer(nms, id_base)
{
  for (int i = 0; i < nms.num_players(); i++)
    {
      receivers.push_back(new Receiver<int>(sockets[i], i));
      senders.push_back(new Sender<int>(socket_to_send(i), i));
    }
}

ThreadPlayer::~ThreadPlayer()
{
  for (unsigned int i = 0; i < receivers.size(); i++)
    {
      if (receivers[i]->timer.elapsed() > 0)
        cerr << "Waiting for receiving from " << i << ": " << receivers[i]->timer.elapsed() << endl;
      delete receivers[i];
    }

  for (unsigned int i = 0; i < senders.size(); i++)
    {
      if (senders[i]->timer.elapsed() > 0)
        cerr << "Waiting for sending to " << i << ": " << senders[i]->timer.elapsed() << endl;
      delete senders[i];
    }
}

void ThreadPlayer::request_receive(int i, OctetStream& o) const
{
  receivers[i]->request(o);
}

void ThreadPlayer::wait_receive(int i, OctetStream& o) const
{
  receivers[i]->wait(o);
}

void ThreadPlayer::receive_player_no_statistics(int i, OctetStream& o) const
{
  request_receive(i, o);
  wait_receive(i, o);
}

void ThreadPlayer::send_all(const OctetStream& o) const
{
  for (int i=0; i<nplayers; i++)
     { if (i!=player_no)
         senders[i]->request(o);
     }

  for (int i = 0; i < nplayers; i++)
    if (i != player_no)
      senders[i]->wait(o);
}


RealTwoPartyPlayer::RealTwoPartyPlayer(const Names& nms, int other_player, const string& id) :
        VirtualTwoPartyPlayer(*(P = new PlainPlayer(nms, id + "2")), other_player)
{
}

RealTwoPartyPlayer::RealTwoPartyPlayer(const Names& nms, int other_player,
        int id_base) : RealTwoPartyPlayer(nms, other_player, to_string(id_base))
{
}

RealTwoPartyPlayer::~RealTwoPartyPlayer()
{
  delete P;
}

void VirtualTwoPartyPlayer::send(OctetStream& o) const
{
  TimeScope ts(communication_statistics["Sending one-to-one"].add(o));
  P.send_to_no_statistics(other_player, o);
  communication_statistics.sent += o.get_length();
}

void VirtualTwoPartyPlayer::receive(OctetStream& o) const
{
  TimeScope ts(timer);
  P.receive_player_no_statistics(other_player, o);
  communication_statistics["Receiving one-to-one"].add(o, ts);
}

void VirtualTwoPartyPlayer::send_receive_player(vector<OctetStream>& o) const
{
  TimeScope ts(communication_statistics["Exchanging one-to-one"].add(o[0]));
  communication_statistics.sent += o[0].get_length();
  P.exchange_no_statistics(other_player, o[0], o[1]);
}

VirtualTwoPartyPlayer::VirtualTwoPartyPlayer(Player& P, int other_player) :
    TwoPartyPlayer(P.my_num()), P(P), other_player(other_player), communication_statistics(
        P.thread_statistics.at(other_player))
{
}

VirtualTwoPartyPlayer::~VirtualTwoPartyPlayer()
{
  for (auto x : children)
      delete x;
}

size_t VirtualTwoPartyPlayer::send(const PlayerBuffer& buffer, bool block) const
{
  auto sent = P.send_no_statistics(other_player, buffer, block);
  lock.lock();
  communication_statistics.add_to_last_round("Sending one-to-one", sent);
  communication_statistics.sent += sent;
  lock.unlock();
  return sent;
}

size_t VirtualTwoPartyPlayer::recv(const PlayerBuffer& buffer, bool block) const
{
  auto received = P.receive_no_statistics(other_player, buffer, block);
  lock.lock();
  communication_statistics.add_to_last_round("Receiving one-to-one", received);
  lock.unlock();
  return received;
}

void VirtualTwoPartyPlayer::exchange(OctetStream& o)
{
  TimeScope ts(communication_statistics["Exchanging one-to-one"].add(o));
  communication_statistics.sent += o.get_length();
  P.exchange_no_statistics(other_player, o, o);
}

void OffsetPlayer::send_receive_player(vector<OctetStream>& o) const
{
  P.exchange(P.get_player(offset), o[0], o[1]);
}

void TwoPartyPlayer::broadcast_receive(vector<OctetStream>& o) const
{
  vector<OctetStream> os(2);
  os[0] = o[my_num()];
  send_receive_player(os);
  o[1 - my_num()] = os[1];
}

NamedCommunicationStatistics::NamedCommunicationStatistics() : sent(0)
{
}

CommunicationStatistics& CommunicationStatistics::operator +=(const CommunicationStatistics& other)
{
  data += other.data;
  rounds += other.rounds;
  timer += other.timer;
  return *this;
}

NamedCommunicationStatistics& NamedCommunicationStatistics::operator +=(const NamedCommunicationStatistics& other)
{
  sent += other.sent;
  for (auto it = other.begin(); it != other.end(); it++)
    map<string, CommunicationStatistics>::operator[](it->first) += it->second;
  return *this;
}

NamedCommunicationStatistics NamedCommunicationStatistics::operator +(const NamedCommunicationStatistics& other) const
{
  auto res = *this;
  res += other;
  return res;
}

CommunicationStatistics& CommunicationStatistics::operator -=(const CommunicationStatistics& other)
{
  data -= other.data;
  rounds -= other.rounds;
  timer -= other.timer;
  return *this;
}

NamedCommunicationStatistics NamedCommunicationStatistics::operator -(const NamedCommunicationStatistics& other) const
{
  NamedCommunicationStatistics res = *this;
  res.sent = sent - other.sent;
  for (auto it = other.begin(); it != other.end(); it++)
    res.map<string, CommunicationStatistics>::operator[](it->first) -= it->second;
  return res;
}

CommunicationStatistics& CommunicationStatistics::imax(const CommunicationStatistics& other)
{
  data = max(data, other.data);
  rounds = max(rounds, other.rounds);
  timer = max(timer, other.timer);
  return *this;
}

NamedCommunicationStatistics& NamedCommunicationStatistics::imax(const NamedCommunicationStatistics& other)
{
  sent = max(sent, other.sent);
  for (auto it = other.begin(); it != other.end(); it++)
    map<string, CommunicationStatistics>::operator[](it->first).imax(it->second);
  return *this;
}

void NamedCommunicationStatistics::print(bool newline, const NamedCommunicationStatistics& max)
{
  for (auto it = begin(); it != end(); it++)
    if (it->second.data)
      {
        auto time = it->second.timer.elapsed();
        cerr << it->first << " " << 1e-6 * it->second.data << " MB in "
            << it->second.rounds << " rounds, taking " << time << " seconds";
        try
        {
          auto max_time = max.at(it->first).timer.elapsed();
          if (max_time and max_time != time)
            cerr << " (at most " << max_time << ")";
        }
        catch(out_of_range&)
        {
        }
        cerr << endl;
      }
  if (size() and newline)
    cerr << endl;
}

void NamedCommunicationStatistics::reset()
{
  clear();
  sent = 0;
}

Timer& NamedCommunicationStatistics::add_to_last_round(const string& name, size_t length)
{
  if (name == last)
    return (*this)[name].add_length_only(length);
  else
    {
      last = name;
      return (*this)[name].add(length);
    }
}

double NamedCommunicationStatistics::total_time() const
{
  double res = 0;
  for (auto& x : *this)
    res += x.second.timer.elapsed();
  return res;
}

Timer& CommunicationStatisticsWithName::add_length_only(size_t length)
{
  if (OnlineOptions::singleton.has_option("verbose_comm"))
    fprintf(stderr, "%s %zu bytes in same round\n", name.c_str(), length);
  return statistics.add_length_only(length);
}

Timer& CommunicationStatisticsWithName::add(const OctetStream& os, int player)
{
  return add(os.get_length(), player);
}

Timer& CommunicationStatisticsWithName::add(size_t length, int player)
{
  if (OnlineOptions::singleton.has_option("verbose_comm"))
    {
      if (player < 0)
        fprintf(stderr, "%s %zu bytes\n", name.c_str(), length);
      else
        fprintf(stderr, "%s %zu bytes with party %d\n", name.c_str(), length,
            player);
    }

  return statistics.add(length);
}

void Player::reset_statistics()
{
  communication_statistics.reset();
  for (auto& x : thread_statistics)
      x.reset();
  for (auto child : children)
      child.second->reset_statistics();
}

NamedCommunicationStatistics Player::total_communication() const
{
  auto res = communication_statistics;
  for (auto& x : thread_statistics)
    res += x;
  for (auto child : children)
    res += child.second->total_communication();
  return res;
}

Player* Player::fork(string suffix)
{
  lock.lock();
  if (children.find(suffix) == children.end())
      children[suffix] = fresh_fork(suffix);
  auto res = children[suffix];
  lock.unlock();
  return res;
}

Player* PlainPlayer::fresh_fork(string suffix)
{
  return new PlainPlayer(N, get_id() + "-" + suffix);
}

TwoPartyPlayer* VirtualTwoPartyPlayer::fork(string suffix)
{
  lock.lock();
  auto res = new VirtualTwoPartyPlayer(*P.fork(suffix), other_player);
  children.push_back(res);
  lock.unlock();
  return res;
}

template class MultiPlayer<int>;
template class MultiPlayer<SslSocket*> ;
