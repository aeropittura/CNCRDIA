/*
 * protocols/common/dabit.h
 *
 */

#pragma once

#include <array>
using namespace std;

template<class T>
class Dabit : public pair<T, typename T::BitType::part_type>
{
    typedef pair<T, typename T::BitType::part_type> Super;

public:
    typedef typename T::BitType::part_type BitType;

    static int size()
    {
        return T::size() + BitType::size();
    }

    static string type_string()
    {
        return T::type_string();
    }

    static void specification(OctetStream& os)
    {
        T::specification(os);
    }

    Dabit()
    {
    }

    Dabit(const T& a, const BitType& b) :
            Super(a, b)
    {
    }

    void assign(const char* buffer)
    {
        this->first.assign(buffer);
        this->second.assign(buffer + T::size());
    }

    void output(ostream& out, bool human)
    {
        this->first.output(out, human);
        this->second.output(out, human);
    }

    void pack(OctetStream& os) const
    {
        this->first.pack(os);
        this->second.pack(os);
    }

    void unpack(OctetStream& os)
    {
        this->first.unpack(os);
        this->second.unpack(os);
    }
};
