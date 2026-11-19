/*
 * parties/offline/stream-fake-mascot-triples.cpp
 *
 */

#include "protocols/common/share.h"
#include "numeric/gfpvar.h"
#include "support/benchmarking.h"

#include "numeric/setup.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/common/share.hpp"

class Info
{
public:
    int thread_num;
    int nplayers;
    KeySetup<Share<Gfpvar>> key;
    pthread_t thread;
};

void* run(void* arg)
{
    auto& info = *(Info*) arg;
    SeededPrng G;
    Files<Share<Gfpvar>> files(info.nplayers, info.key, PREP_DIR, DATA_TRIPLE, G, info.thread_num);
    int count = 0;
    while (true)
    {
        for (int i = 0; i < 100000; i++)
        {
            Gfpvar triple[3];
            for (int i = 0; i < 2; i++)
                triple[i].randomize(G);
            triple[2] = triple[0] * triple[1];
            for (int i = 0; i < 3; i++)
                files.output_shares(triple[i]);
            count++;
        }
        // take a break to make them wait
        sleep(1);
    }
    cerr << "failed after " << count << endl;
    return 0;
}

int main()
{
    insecure_fake();
    typedef Share<Gfpvar> T;
    int nplayers = 2;
    int lgp = 128;
    string prep_data_prefix = PREP_DIR;
    Gfpvar::generate_setup<T>(prep_data_prefix, nplayers, lgp);
    KeySetup<T> keyp;
    SeededPrng G;
    generate_mac_keys<T>(keyp, nplayers, prep_data_prefix, G);

    int nthreads = 3;
    OnlineOptions::singleton.file_prep_per_thread = true;
    vector<Info> infos(3);
    for (int i = 0; i < nthreads; i++)
    {
        auto& info = infos[i];
        info.thread_num = i;
        info.nplayers = nplayers;
        info.key = keyp;
        pthread_create(&info.thread, 0, run, &info);
    }
    pthread_join(infos[0].thread, 0);
}
