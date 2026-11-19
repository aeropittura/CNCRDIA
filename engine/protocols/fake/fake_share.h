/*
 * protocols/fake/fake_share.h
 *
 */

#pragma once

#include "boolean/fake_secret.h"
#include "protocols/common/share_interface.h"
#include "protocols/fake/fake_mc.h"
#include "protocols/fake/fake_protocol.h"
#include "protocols/fake/fake_prep.h"
#include "protocols/fake/fake_input.h"

template<class T>
class FakeShare : public T, public ShareInterface
{
    typedef FakeShare This;

public:
    typedef T OpenType;
    typedef T clear;
    typedef This ShareType;

    typedef FakePrep<This> LivePrep;
    typedef FakeOpener<This> MacCheck;
    typedef MacCheck DirectOpener;
    typedef FakeInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef FakeProtocol<This> Protocol;

    typedef GC::FakeSecret BitType;

    static const bool has_trunc_pr = true;
    static const bool dishonest_majority = false;
    static const bool malicious = false;
    static const bool is_real = false;
    static const bool variable_players = false;

    static false_type is_clear;

    static string type_short()
    {
        return "emul";
    }

    static int threshold(int)
    {
        return 0;
    }

    static T constant(T value, int = 0, MacKeyType = {})
    {
        return value;
    }

    FakeShare()
    {
    }

    template<class U>
    FakeShare(U other) :
            T(other)
    {
    }

    static void split(StackedVector<BitType>& dest, const ArgVector& regs,
            int n_bits, const This* source, int n_inputs,
            GC::FakeSecret::Protocol& protocol);
};
