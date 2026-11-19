/*
 * vm/offline_machine.hpp
 *
 */

#pragma once

#include "vm/offline_machine.h"
#include "protocols/common/mac_key.hpp"
#include "support/buffer.h"

template<class W>
template<class V>
OfflineMachine<W>::OfflineMachine(int argc, const char** argv,
        ez::EzOptionParser& opt, OnlineOptions& online_options, V,
        int nplayers) :
        W(argc, argv, opt, online_options, V(), nplayers), player_names(
                W::player_names), P(*this->new_player("machine"))
{
    load_schedule(online_options.progname, false);
    Program program(player_names.num_players());
    program.parse(bc_filenames[0]);
    progs.push_back(program);

    if (program.usage_unknown())
    {
        cerr << "Preprocessing will be insufficient "
                << "due to unknown requirements" << endl;
        exit(1);
    }

    usage = program.get_offline_data_used();
}

template<class W>
OfflineMachine<W>::~OfflineMachine()
{
    delete &P;
}

template<class W>
template<class T, class U>
int OfflineMachine<W>::run()
{
    if (this->online_options.prime)
        T::clear::initialize_field(this->online_options.prime);
    else
        T::clear::initialize_default(this->online_options.prime_length());
    Machine<T, U>::initialize_binary_domains(this->online_options.security_parameter,
            this->get_lg2());
    auto binary_mac_key = T::BitType::LivePrep::get_mac_key(P);
    typename T::BitType::LivePrep bit_prep(usage);
    GC::ShareThread<typename T::BitType> thread(bit_prep, P, binary_mac_key);

    // setup before generation to fix prime
    T::LivePrep::basic_setup(P);

    T::MacCheck::setup(P);
    T::BitType::MacCheck::setup(P);
    U::MacCheck::setup(P);

    generate<T>();
    generate<typename T::BitType::part_type>();
    generate<U>();

    thread.MC->Check(P);

    T::MacCheck::teardown();
    T::BitType::MacCheck::teardown();
    U::MacCheck::teardown();

    return 0;
}

template<class W>
int OfflineMachine<W>::buffered_total(size_t required, size_t batch)
{
    return DIV_CEIL(required, batch) * batch + (nthreads - 1) * batch;
}

template<class W>
template<class T>
void OfflineMachine<W>::generate()
{
    T::clear::next::template init<typename T::clear>(false);
    T::clear::template write_setup<T>(P.num_players());
    auto mac_key = T::LivePrep::get_mac_key(P);
    DataPositions generated;
    generated.set_num_players(P.num_players());
    typename T::MacCheck output(mac_key);
    typename T::LivePrep preprocessing(0, generated);
    SubProcessor<T> processor(output, preprocessing, P);

    auto& domain_usage = usage.files[T::clear::field_type()];
    for (unsigned i = 0; i < domain_usage.size(); i++)
    {
        auto my_usage = domain_usage[i];
        Dtype dtype = Dtype(i);
        string filename = SubDataFiles<T>::get_filename(player_names, dtype,
                0);
        if (my_usage > 0)
        {
            ofstream out(filename, iostream::out | iostream::binary);
            file_signature<T>().output(out);
            if (i == DATA_DABIT)
            {
                for (long long j = 0;
                        j < buffered_total(my_usage, BUFFER_SIZE); j++)
                {
                    T a;
                    typename T::BitType b;
                    preprocessing.get_dabit(a, b);
                    Dabit<T>(a, b).output(out, false);
                }
            }
            else if (not (i == DATA_RANDOM or i == DATA_OPEN))
            {
                vector<T> tuple(DataPositions::tuple_size[i]);
                for (long long j = 0;
                        j < buffered_total(my_usage, BUFFER_SIZE); j++)
                {
                    preprocessing.get(dtype, tuple.data());
                    for (auto& x : tuple)
                        x.output(out, false);
                }
            }
        }
        else
            remove(filename.c_str());
    }

    long additional_inputs = SubDataFiles<T>::additional_inputs(usage);

    for (int i = 0; i < P.num_players(); i++)
    {
        auto n_inputs = usage.inputs[i][T::clear::field_type()]
                + additional_inputs;
        string filename = SubDataFiles<T>::get_input_filename(player_names, i, 0);
        if (n_inputs > 0)
        {
            ofstream out(filename, iostream::out | iostream::binary);
            file_signature<T>().output(out);
            InputTuple<T> tuple;
            for (long long j = 0;
                    j < buffered_total(n_inputs, BUFFER_SIZE); j++)
            {
                preprocessing.get_input(tuple.share, tuple.value, i);
                tuple.share.output(out, false);
                if (i == P.my_num())
                    tuple.value.output(out, false);
            }
        }
        else
            remove(filename.c_str());
    }

    if (T::clear::field_type() == DATA_INT)
    {
        int max_n_bits = 0;
        for (auto& x : usage.edabits)
            max_n_bits = max(max_n_bits, x.first.second);

        for (int n_bits = 1; n_bits <= max(100, max_n_bits); n_bits++)
        {
            int batch = Edabitvec<T>::MAX_SIZE;
            int total = usage.edabits[{false, n_bits}] +
                    usage.edabits[{true, n_bits}];
            string filename = SubDataFiles<T>::get_edabit_filename(player_names,
                                n_bits, 0);
            if (total > 0)
            {
                ofstream out(filename, ios::binary);
                file_signature<T>().output(out);
                auto& opts = OnlineOptions::singleton;
                opts.batch_size = DIV_CEIL(opts.batch_size, batch) * batch;
                for (int i = 0; i < buffered_total(total, batch) / batch; i++)
                    preprocessing.get_edabitvec(true, n_bits).output(n_bits,
                            out);
            }
            else
                remove(filename.c_str());
        }
    }

    output.Check(P);
}

template<class W>
const Names& OfflineMachine<W>::get_N()
{
    return player_names;
}
