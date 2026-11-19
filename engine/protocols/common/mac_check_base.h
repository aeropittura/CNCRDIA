/*
 * protocols/common/mac_check_base.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "net/player.h"
#include "support/pointer_vector.h"

template<class T> class Preprocessing;
template<class T> class MatrixOpener;

/**
 * Abstract base class for opening protocols
 */
template<class T>
class MacCheckBase
{
    friend class MatrixOpener<T>;

protected:
    /* MAC Share */
    typename T::MacKeyType::Scalar alphai;

    PointerVector<T> secrets;
    PointerVector<typename T::OpenType> values;

public:
    size_t values_opened;

    static void setup(Player&) {}
    static void teardown() {}

    MacCheckBase(const typename T::MacKeyType::Scalar& mac_key = { }) :
            alphai(mac_key), values_opened(0) {}
    virtual ~MacCheckBase() {}

    /// Run checking protocol
    virtual void Check(const Player& P) { (void)P; }

    int number() const { return values_opened; }

    /// Get MAC key
    const typename T::MacKeyType::Scalar& get_alphai() const { return alphai; }

    virtual void POpen_Begin(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    virtual void POpen_End(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    /// Open values in ``S`` and store results in ``values``
    virtual void POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    typename T::OpenType POpen(const T& secret, const Player& P);
    /// Open single value
    typename T::OpenType open(const T& secret, const Player& P) { return POpen(secret, P); }

    /// Initialize opening round
    virtual void initialize_open(const Player& P, int n = 0);
    /// Add value to be opened
    virtual void prepare_open(const T& secret, int n_bits = -1);
    /// Run opening protocol
    virtual void exchange(const Player& P) = 0;
    /// Get next opened value
    virtual typename T::clear finalize_open();
    virtual typename T::OpenType finalize_raw();
    virtual array<typename T::OpenType*, 2> finalize_several(size_t n);

    virtual void start_exchange(const Player& P);
    virtual void stop_exchange(const Player& P);

    /// Check whether all ``shares`` are ``value``
    virtual void check_for(const typename T::OpenType& value, const vector<T>& shares, const Player& P);

    virtual const Player& get_check_player(const Player& P) const { return P; }

    virtual void set_prep(Preprocessing<T>&) {}

    void set_random_element(const T&) {}
};
