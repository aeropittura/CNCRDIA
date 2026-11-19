/*
 * net/crypto_player.h
 *
 */

#pragma once

#include "net/ssl_sockets.h"
#include "net/player.h"

#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

/**
 * Encrypted multi-party communication.
 * Uses OpenSSL and certificates issued to "P<player_no>".
 * Sending and receiving is done in separate threads to allow
 * for bidirectional communication.
 */
class CryptoPlayer : public MultiPlayer<SslSocket*>
{
    SslContext ctx;
    SslService io_service;

    vector<SslSocket*> other_sockets;

    vector<Sender<SslSocket*>*> senders;
    vector<Receiver<SslSocket*>*> receivers;

    void connect(int other, vector<int>* plaintext_sockets);

public:
    /**
     * Start a new set of encrypted connections.
     * @param nms network setup
     * @param id unique identifier
     */
    CryptoPlayer(const Names& nms, const string& id);
    // legacy interface
    CryptoPlayer(const Names& nms, int id_base = 0);
    ~CryptoPlayer();

    bool is_encrypted() { return true; }

    void send_to_no_statistics(int other, const OctetStream& o) const;
    void receive_player_no_statistics(int other, OctetStream& o) const;

    size_t send_no_statistics(int player, const PlayerBuffer& buffer,
            bool block) const;
    size_t receive_no_statistics(int player, const PlayerBuffer& buffer,
            bool block) const;

    void exchange_no_statistics(int other, const OctetStream& to_send,
        OctetStream& to_receive) const;

    void pass_around_no_statistics(const OctetStream& to_send, OctetStream& to_receive,
        int offset) const;

    void send_receive_all_no_statistics(const vector<vector<bool>>& channels,
            const vector<OctetStream>& to_send,
            vector<OctetStream>& to_receive) const;

    void partial_broadcast_no_statistics(const vector<bool>& my_senders,
            const vector<bool>& my_receivers, vector<OctetStream>& os) const;

    void broadcast_receive_no_statistics(vector<OctetStream>& os) const;

    Player* fresh_fork(string suffix);
};
