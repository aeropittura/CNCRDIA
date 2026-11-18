/*
 * ReplicatedParty.h
 *
 */

#ifndef GC_SHAREPARTY_H_
#define GC_SHAREPARTY_H_

#include "boolean/share_secret.h"
#include "boolean/processor.h"
#include "boolean/program.h"
#include "boolean/memory.h"
#include "boolean/thread_master.h"

namespace GC
{

template<class T>
class ShareParty : public ThreadMaster<T>
{
    static ShareParty<T>* singleton;

    ez::ezOptionParser& opt;
    OnlineOptions online_opts;

public:
    static ShareParty& s();

    typename T::mac_key_type mac_key;

    ShareParty(int argc, const char** argv, ez::ezOptionParser& opt,
            int default_batch_size = 0);

    Thread<T>* new_thread(int i);

    void post_run();
};

template<class T>
inline ShareParty<T>& ShareParty<T>::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no ShareParty singleton");
}

}

#endif /* GC_SHAREPARTY_H_ */
