/*
 * net/all_but_last_player.h
 *
 */

#pragma once

#include "net/player.h"

class AllButLastPlayer : public Player
{
    const Player& P;
    Names* N;

public:
    AllButLastPlayer(const Player& P) :
            Player(*(N = new Names(P.my_num(), P.num_players() - 1))), P(P)
    {
    }

    ~AllButLastPlayer()
    {
        delete N;
    }

    void send_to(int player, const OctetStream& o) const
    {
        P.send_to(player, o);
    }

    void receive_player(int i, OctetStream& o) const
    {
        P.receive_player(i, o);
    }

    void send_to_no_statistics(int, const OctetStream&) const
    {
        throw NotImplemented();
    }

    void receive_player_no_statistics(int, OctetStream&) const
    {
        throw NotImplemented();
    }

    void send_receive_all_no_statistics(const vector<vector<bool>>& channels,
            const vector<OctetStream>& to_send,
            vector<OctetStream>& to_receive) const
    {
        auto my_channels = channels;
        if (channels.empty())
        {
            my_channels.resize(P.num_players()  - 1);
            for (auto& x : my_channels)
                x.resize(P.num_players() - 1, true);
        }
        my_channels.resize(P.num_players());
        for (auto& x : my_channels)
            x.resize(P.num_players());
        auto my_to_send = to_send;
        if (P.my_num() != P.num_players() - 1)
            P.send_receive_all(my_channels, my_to_send, to_receive);
        to_receive.resize(P.num_players() - 1);
    }

    void broadcast_receive_no_statistics(vector<OctetStream>& os) const
    {
        vector<bool> senders(P.num_players(), true), receivers(P.num_players(),
                true);
        senders.back() = false;
        receivers.back() = false;
        P.partial_broadcast(senders, receivers, os);
        os.resize(num_players());
    }
};
