
#include "numeric/z2k.h"
#include "protocols/common/share.h"
#include "numeric/setup.h"
#include "protocols/spdz2k/spdz2k_share.h"
#include "protocols/fake/fake_stuff.h"

#include "protocols/fake/fake_stuff.hpp"
#include "protocols/common/share.hpp"
#include "numeric/z2k.hpp"

#include <fstream>
#include <vector>
#include <numeric>

template <class W>
void check_triples_z2k(int n_players, string type_char = "")
{
    typedef typename W::OpenType T;
    typedef typename W::OpenType V;

    T keyp; keyp.assign_zero();
    read_global_mac_key(get_prep_sub_directory<W>(n_players), n_players, keyp);

    ifstream* input_files = new ifstream[n_players];
    for (int i = 0; i < n_players; i++)
    {
        stringstream ss;
        ss << get_prep_sub_directory<W>(n_players) << "Triples-";
        if (type_char.size())
            ss << type_char;
        else
            ss << W::type_short();
        ss << "-P" << i;
        input_files[i].open(ss.str().c_str());
        cout << "Opening file " << ss.str() << endl;
        OctetStream tmp;
        tmp.input(input_files[i]);
        assert(tmp == file_signature<W>());
    }

    int j = 0;
    while (input_files[0].peek() != EOF)
    {
        V a,b,c,prod;
        T mac;
        vector<Share<T>> as(n_players), bs(n_players), cs(n_players);

        for (int i = 0; i < n_players; i++)
        {
          as[i].input(input_files[i], false);
          bs[i].input(input_files[i], false);
          cs[i].input(input_files[i], false);
        }

        check_share<T, V>(as, a, mac, n_players, keyp);
        check_share<T, V>(bs, b, mac, n_players, keyp);
        check_share<T, V>(cs, c, mac, n_players, keyp);
        
        prod = (a * b);
        if (typename W::clear(prod) != typename W::clear(c))
        {
          cout << j << ": " << c << " != " << prod << " = " << a << " * " << b << endl;
          throw BadValue();
        }
        j++;
    }

    cout << dec << j << " correct triples of type " << T::type_string() << endl;
    delete[] input_files;
}

template<int K, int S>
void check(int n_players)
{
    check_triples_z2k<Spdz2kShare<K, S>>(n_players);
}

int main(int argc, char** argv)
{
    int n_players = 2;
    if (argc > 1)
        n_players = atoi(argv[1]);
    int k = 64;
    if (argc > 2)
    {
        k = atoi(argv[2]);
    }
    int s = k;
    if (argc > 3)
        s = atoi(argv[3]);

    if (k == 32)
        check<32, 32>(n_players);
    else if (k == 64 and s == 64)
        check<64, 64>(n_players);
    else if (k == 64 and s == 48)
        check<64, 48>(n_players);
    else if (k == 66 and s == 64)
        check<66, 64>(n_players);
    else if (k == 66 and s == 48)
        check<66, 48>(n_players);
    else
        throw runtime_error("not compiled for k=" + to_string(k) + ", s=" + to_string(s));
}
