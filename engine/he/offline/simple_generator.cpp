/*
 * he/offline/simple_generator.cpp
 *
 */

#include "he/offline/simple_generator.h"
#include "he/offline/simple_machine.h"
#include "he/offline/sacrificing.h"
#include "protocols/common/mac_check.h"

#include "protocols/common/mac_check.hpp"

template <template <class> class T, class FD>
SimpleGenerator<T,FD>::SimpleGenerator(const Names& N, const PartSetup<FD>& setup,
        const MultiplicativeMachineParameters& machine,
        int thread_num, Dtype data_type, Player* player) :
        GeneratorBase(thread_num, N, player),
        setup(setup), machine(machine),
        volatile_memory(0), dd(P, setup),
        EC(P, setup.pk, setup.field_d, timers, machine, thread_num)
{
    string prep_directory = machine.get_prep_directory<FD>(P);
    if (machine.produce_inputs)
        producer = new InputProducer<FD>(P, thread_num, machine.output, prep_directory);
    else
        switch (data_type)
        {
        case DATA_TRIPLE:
            producer = new TripleProducer_<FD>(setup.field_d, P.my_num(),
                    thread_num, machine.output, prep_directory);
            break;
        case DATA_SQUARE:
            producer = new SquareProducer<FD>(setup.field_d, P.my_num(),
                    thread_num, machine.output, prep_directory);
            break;
        case DATA_BIT:
            producer = new_bit_producer(setup.field_d, P, setup.pk, machine.get_covert(), true,
                    thread_num, machine.output, prep_directory);
            break;
        case DATA_INVERSE:
            producer = new InverseProducer<FD>(setup.field_d, P.my_num(),
                    thread_num, machine.output, true, prep_directory);
            break;
        default:
            throw runtime_error("data type not implemented");
        }
}

template <template <class> class T, class FD>
SimpleGenerator<T,FD>::~SimpleGenerator()
{
    delete producer;
}

template <template <class> class T, class FD>
void SimpleGenerator<T,FD>::run(bool exhaust)
{
    Timer timer(CLOCK_THREAD_CPUTIME_ID);
    timer.start();
    timers["MC init"].start();
    MacCheck<typename FD::T> MC(setup.alphai);
    timers["MC init"].stop();
    producer->clear();
    total = 0;
    while (total < machine.n_triples_per_thread or (exhaust and EC.has_left()))
    {
        producer->run_with_catch(P, setup.pk, setup.calpha, EC, dd, setup.alphai);
        producer->sacrifice_with_catch(P, MC);
        total += producer->num_slots();
    }
    MC.Check(P);
    timer.stop();
    timers["Thread"] = timer;
    timers.insert(producer->timers.begin(), producer->timers.end());
    timers["Networking"] = P.timer;
}

template <template <class> class T, class FD>
size_t SimpleGenerator<T,FD>::report_size(ReportType type)
{
    return EC.report_size(type) + EC.get_volatile()
            + producer->report_size(type) + dd.report_size(type)
            + volatile_memory;
}

template <template <class> class T, class FD>
void SimpleGenerator<T,FD>::report_size(ReportType type, MemoryUsage& res)
{
    EC.report_size(type, res);
    res.add("producer", producer->report_size(type));
    res.add("generator volatile", volatile_memory);
    res.add("distributed decryption", dd.report_size(type));
}

template class SimpleGenerator<SimpleEncCommit_, FftData>;
template class SimpleGenerator<SummingEncCommit, FftData>;
template class SimpleGenerator<EncCommit_, FftData>;

template class SimpleGenerator<SimpleEncCommit_, P2Data>;
template class SimpleGenerator<SummingEncCommit, P2Data>;
template class SimpleGenerator<EncCommit_, P2Data>;
