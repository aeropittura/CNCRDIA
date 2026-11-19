#include "ot/ot_triple_setup.h"
#include "support/benchmarking.h"

void* run_ot(void* job)
{
    ((OtTripleSetup::SetupJob*)job)->run();
    return 0;
}

void OtTripleSetup::setup()
{
    timeval baseOTstart, baseOTend;
    gettimeofday(&baseOTstart, NULL);

    G.re_seed();
    for (int i = 0; i < get_nbase(); i++)
    {
        base_receiver_inputs[i] = G.get_uchar() & 1;
    }
    //base_receiver_input.randomize(G);

    vector<SetupJob> threads;
    for (int i = 0; i < nparties - 1; i++)
        threads.push_back({*this, i});
    for (int i = 0; i < nparties - 1; i++)
        pthread_create(&threads.at(i).thread, 0, run_ot, &threads.at(i));
    for (int i = 0; i < nparties - 1; i++)
        pthread_join(threads.at(i).thread, 0);
    gettimeofday(&baseOTend, NULL);
#ifdef VERBOSE_BASEOT
    double basetime = timeval_diff(&baseOTstart, &baseOTend);
    cout << "\t\tBaseTime: " << basetime/1000000 << endl << flush;
#endif

    for (size_t i = 0; i < base_ots.size(); i++)
    {
        delete base_ots.at(i);
    }

    // Receiver send something to force synchronization
    // (since Sender finishes base_ots before Receiver)
}

void OtTripleSetup::run(int i)
{
    base_ots.at(i)->set_receiver_inputs(base_receiver_inputs);
    base_ots.at(i)->exec_base(false);
    base_sender_inputs.at(i) = base_ots.at(i)->sender_inputs;
    base_receiver_outputs.at(i) = base_ots.at(i)->receiver_outputs;
}

void OtTripleSetup::close_connections()
{
    for (size_t i = 0; i < players.size(); i++)
    {
        delete players.at(i);
    }
}

OtTripleSetup OtTripleSetup::get_fresh()
{
    OtTripleSetup res = *this;
    for (int i = 0; i < nparties - 1; i++)
    {
        auto bot = get_base(i);
        res.base_sender_inputs.at(i) = bot.sender_inputs;
        res.base_receiver_outputs.at(i) = bot.receiver_outputs;
    }
    return res;
}

BaseOT OtTripleSetup::get_base(size_t index)
{
    BaseOT bot(get_nbase(), 0);
    bot.receiver_inputs = base_receiver_inputs;
    bot.sender_inputs = base_sender_inputs.at(index);
    bot.receiver_outputs = base_receiver_outputs.at(index);
    bot.set_seeds();
    bot.extend_length();
    base_sender_inputs.at(index) = bot.sender_inputs;
    base_receiver_outputs.at(index) = bot.receiver_outputs;
    bot.extend_length();
    return bot;
}

void OtTripleSetup::pack(OctetStream& os) const
{
    os.store(my_num);
    os.store(base_sender_inputs);
    os.store(base_receiver_outputs);
    base_receiver_inputs.pack(os);
}

void OtTripleSetup::unpack(OctetStream& os)
{
    os.get(my_num);
    os.get(base_sender_inputs);
    os.get(base_receiver_outputs);
    base_receiver_inputs.unpack(os);

    nparties = base_sender_inputs.size() + 1;
    size_t nbase = get_nbase();

    assert(my_num < nparties);
    assert(base_receiver_outputs.size() == base_sender_inputs.size());
    for (auto& x : base_receiver_outputs)
        assert(x.size() == size_t(nbase));
    for (auto& x : base_sender_inputs)
        assert(x.size() == size_t(nbase));
}

void OtTripleSetup::check(TwoPartyPlayer& P)
{
    insecure("reveal for check");
    int index = index_for(P.other_player_num());
    vector<OctetStream> os(2);
    os[0].store(base_sender_inputs.at(index));
    P.send_receive_player(os);
    SenderType x;
    os[1].get(x);
    for (size_t i = 0; i < base_receiver_inputs.size(); i++)
    {
        auto bit = base_receiver_inputs.get_bit(i);
        assert(base_receiver_outputs.at(index).at(i) == x.at(i)[bit]);
    }
}
