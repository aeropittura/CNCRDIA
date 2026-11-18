/*
 * YaoGarbleMaster.h
 *
 */

#ifndef YAO_YAOGARBLEMASTER_H_
#define YAO_YAOGARBLEMASTER_H_

#include "boolean/thread_master.h"
#include "boolean/secret.h"
#include "garbled/yao/yao_garble_wire.h"
#include "vm/online_options.h"

class YaoGarbleMaster : public GC::ThreadMaster<GC::Secret<YaoGarbleWire>>
{
    typedef GC::ThreadMaster<GC::Secret<YaoGarbleWire>> super;

    Key delta;

public:
    bool continuous;
    int threshold;

    YaoGarbleMaster(bool continuous, OnlineOptions& opts, int threshold = 1024);

    GC::Thread<GC::Secret<YaoGarbleWire>>* new_thread(int i);

    Key get_delta()
    {
        return delta;
    }
};

#endif /* YAO_YAOGARBLEMASTER_H_ */
