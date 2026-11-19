
#include "support/commit.h"
#include "support/random.h"
#include "support/int.h"

void Commit(OctetStream& comm,OctetStream& open,const OctetStream& message, int send_player)
{
    open.store(send_player);
    open.append(message.get_data(), message.get_length());
    open.append_random(SEED_SIZE);
    comm = open.hash();
}

bool Open(OctetStream& message,const OctetStream& comm, OctetStream& open, int send_player)
{
    OctetStream h = open.hash();
    // first 4 bytes are player no.
    int open_player;
    try
    {
        open_player = open.get<int>();
    }
    catch (exception& e)
    {
        throw InvalidCommitment(send_player, e.what());
    }

    if (!(h.equals(comm) && open_player == send_player)
            or open.left() <= SEED_SIZE)
    {
        throw InvalidCommitment(send_player);
    }
    message.reset_write_head();
    message.append(open.consume(0), open.left() - SEED_SIZE);
    return true;
}

void Commitment::commit(const OctetStream& message)
{
    open.reset_write_head();
    open.append_random(SEED_SIZE);
    commit(message, open);
}

void Commitment::commit(const OctetStream& message, const OctetStream& open)
{
    Hash hash;
    hash.update(&send_player, sizeof(send_player));
    hash.update(message);
    hash.update(open);
    hash.final(comm);
}

void Commitment::check(const OctetStream& message, const OctetStream& comm,
        const OctetStream& open)
{
    if (open.empty())
        throw InvalidCommitment(send_player, "empty opening");
    commit(message, open);
    if (!(comm == this->comm))
        throw InvalidCommitment(send_player);
}

void AllCommitments::commit_and_open(const OctetStream& message)
{
    commit(message);
    open();
}

void AllCommitments::commit(const OctetStream& message)
{
    Commitment mine(P.my_num());
    mine.commit(message);
    comms[P.my_num()] = mine.comm;
    opens[P.my_num()] = mine.open;
    P.broadcast_receive(comms);
}

void AllCommitments::open()
{
    P.broadcast_receive(opens);
}

void AllCommitments::open(const OctetStream& message)
{
    messages.resize(P.num_players());
    messages[P.my_num()] = message;
    P.broadcast_receive(messages);
    open();
    for (int i = 0; i != P.my_num(); i++)
        check(i, messages[i]);
}

void AllCommitments::check(int player, const OctetStream& message)
{
    Commitment(player).check(message, comms[player], opens[player]);
}

void AllCommitments::check_relative(int diff, const OctetStream& message)
{
    check((P.my_num() + P.num_players() - diff) % P.num_players(), message);
}
