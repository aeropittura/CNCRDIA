#pragma once

/* Define the hash based commitment scheme */

#include "support/octet_stream.h"
#include "net/player.h"

/*
 * Commit using comm = hash(send_player || message || r)
 * where r is SEED_SIZE random bytes
 */
void Commit(OctetStream& comm, OctetStream& open, const OctetStream& message, int send_player);

bool Open(OctetStream& message, const OctetStream& comm, OctetStream& open, int send_player);

// same as above using less memory
class Commitment
{
    int send_player;

public:
    // open only contains the randomness
    OctetStream comm, open;

    Commitment(int send_player) : send_player(send_player) {}
    void commit(const OctetStream& message);
    void commit(const OctetStream& message, const OctetStream& open);
    void check(const OctetStream& message, const OctetStream& comm,
            const OctetStream& open);
};

class AllCommitments
{
    const Player& P;
    vector<OctetStream> comms, opens;

public:
    vector<OctetStream> messages;

    AllCommitments(const Player& P) : P(P), comms(P.num_players()), opens(P.num_players()) {}
    // no checks yet
    void commit_and_open(const OctetStream& message);
    void commit(const OctetStream& message);
    // only open randomness
    void open();
    // open and check everything
    void open(const OctetStream& message);
    void check(int player, const OctetStream& message);
    void check_relative(int diff, const OctetStream& message);
};
