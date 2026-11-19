/*
 * boolean/tiny_secret.h
 *
 */

#pragma once

#include "boolean/secret.h"
#include "boolean/tiny_share.h"
#include "boolean/share_party.h"
#include "ot/rectangle.h"
#include "ot/bit_diagonal.h"

template<class T> class NPartyTripleGenerator;
template<class T> class OtTripleGenerator;
template<class T> class TinyMultiplier;

namespace GC
{

template<class T> class TinyOnlyPrep;
template<class T> class TinyOpener;
template<class T> class VectorProtocol;
template<class T> class VectorInput;
template<class T> class CcdPrep;

template<class T>
class VectorSecret : public Secret<T>
{
    typedef VectorSecret This;

public:
    typedef T part_type;
    typedef Secret<part_type> Super;

    typedef typename part_type::MacKeyType MacKeyType;

    typedef BitVec OpenType;
    typedef BitVec clear;

    typedef typename part_type::SacriType SacriType;
    typedef typename part_type::MacType MacType;
    typedef typename part_type::MacShareType MacShareType;
    typedef BitDiagonal Rectangle;

    typedef typename T::Super CheckType;

    static const bool dishonest_majority = T::dishonest_majority;
    static const bool variable_players = T::variable_players;
    static const bool needs_ot = T::needs_ot;
    static const bool has_mac = T::has_mac;
    static const bool malicious = T::malicious;
    static const bool expensive_triples = T::expensive_triples;
    static const bool randoms_for_opens = false;
    static const bool symmetric = true;
    static const bool function_dependent = T::function_dependent;

    static const int default_length = 64;

    static bool real_shares(const Player&)
    {
        return true;
    }

    static int size()
    {
        return part_type::size() * default_length;
    }

    static void specification(OctetStream& os)
    {
        T::specification(os);
    }

    static void read_or_generate_mac_key(string directory, const Player& P,
            MacKeyType& key)
    {
        T::read_or_generate_mac_key(directory, P, key);
    }

    static typename T::MacType get_mac_key()
    {
        return T::get_mac_key();
    }

    static void set_mac_key(MacKeyType& mac_key)
    {
        part_type::set_mac_key(mac_key);
    }

    static void set_secrets_filename(string filename)
    {
        T::InputCheckType::set_secrets_filename(filename);
    }

    template<class U>
    static void reveal_inst(U& processor, const ArgVector& args)
    {
        T::reveal_inst(processor, args);
    }

    static This constant(BitVec other, int my_num, MacKeyType alphai,
            int n_bits = -1)
    {
        if (n_bits < 0)
            n_bits = other.length();
        This res;
        res.resize_registers(n_bits);
        for (int i = 0; i < n_bits; i++)
            res.get_reg(i) = part_type::constant(other.get_bit(i), my_num, alphai);
        return res;
    }

    VectorSecret()
    {
    }
    VectorSecret(const Super& other) :
            Super(other)
    {
    }
    VectorSecret(const Clear& other) :
            Super(other)
    {
    }
    VectorSecret(const part_type& other)
    {
        this->get_registers().push_back(other);
    }

    void assign(const char* buffer)
    {
        this->resize_registers(default_length);
        for (int i = 0; i < default_length; i++)
            this->get_reg(i).assign(buffer + i * part_type::size());
    }

    This operator-(const This& other) const
    {
        return *this + other;
    }

    This& operator^=(const This& other)
    {
        return *this = *this + other;
    }

    This operator*(const BitVec& other) const
    {
        This res = *this;
        for (int i = 0; i < Super::size(); i++)
            if (not other.get_bit(i))
                res.get_reg(i) = {};
        return res;
    }

    This operator&(const BitVec::Super& other) const
    {
        return *this * BitVec(other);
    }

    void extend_bit(This& res, int n_bits) const
    {
        auto& regs = res.get_registers();
        regs.assign(n_bits, this->get_reg(0));
    }

    T get_bit(int i) const
    {
        return this->get_reg(i);
    }

    void xor_bit(size_t i, const T& bit)
    {
        if (i < this->get_registers().size())
            XOR(this->get_reg(i), this->get_reg(i), bit);
        else
        {
            this->resize_registers(i + 1);
            this->get_reg(i) = bit;
        }
    }

    void output(ostream& s, bool human = true) const
    {
        assert(this->get_registers().size() == default_length);
        for (auto& reg : this->get_registers())
            reg.output(s, human);
    }

    void input(istream&, bool)
    {
        throw NotImplemented();
    }

    template <class U>
    void my_input(U& inputter, BitVec value, int n_bits)
    {
        inputter.add_mine(value, n_bits);
    }

    template <class U>
    void other_input(U& inputter, int from, int n_bits)
    {
        inputter.add_other(from, n_bits);
    }

    template <class U>
    void finalize_input(U& inputter, int from, int n_bits)
    {
        inputter.finalize(from, n_bits).mask(*this, n_bits);
    }

    void random_bit()
    {
        auto& thread = GC::ShareThread<typename T::WholeType>::s();
        *this = thread.DataF.get_part().get_bit();
    }
};

template<int S>
class TinySecret : public VectorSecret<TinyShare<S>>
{
    typedef VectorSecret<TinyShare<S>> Super;
    typedef TinySecret This;

public:
    typedef TinyOpener<This> MC;
    typedef MC MacCheck;
    typedef VectorProtocol<This> Protocol;
    typedef VectorInput<This> Input;
    typedef CcdPrep<This> LivePrep;
    typedef Memory<This> DynamicMemory;

    typedef OtTripleGenerator<This> TripleGenerator;
    typedef typename Super::part_type::TripleGenerator InputGenerator;

    typedef TinyMultiplier<This> Multiplier;

    static string type_short()
    {
        return "T";
    }

    static MC* new_opener(typename Super::MacKeyType mac_key)
    {
        return new MC(mac_key);
    }

    static void store_clear_in_dynamic(Memory<This>& mem,
            const vector<ClearWriteAccess>& accesses)
    {
        auto& party = ShareThread<This>::s();
        for (auto access : accesses)
            mem[access.address] = Super::constant(access.value,
                    party.P->my_num(), {});
    }

    static void generate_mac_key(typename This::MacKeyType& dest,
            const typename This::MacKeyType& source)
    {
        dest = source;
    }

    TinySecret()
    {
    }
    TinySecret(const typename Super::Super& other) :
            Super(other)
    {
    }
    TinySecret(const typename Super::part_type& other) :
            Super(other)
    {
    }

    void reveal(size_t n_bits, Clear& x)
    {
        auto& to_open = *this;
        to_open.resize_registers(n_bits);
        auto& party = ShareThread<This>::s();
        x = party.MC->POpen(to_open, *party.P);
    }
};

template<class U>
const int VectorSecret<U>::default_length;

template<class T>
inline VectorSecret<T> operator*(const BitVec& clear, const VectorSecret<T>& share)
{
    return share * clear;
}

} /* namespace GC */
