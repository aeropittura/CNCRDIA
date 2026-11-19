#include "support/hash.h"
#include "support/octet_stream.h"

void hash_update(Hash *ctx, const void *data, unsigned long len)
{
    ctx->update(data, len);
}

Hash::Hash()
{
    // deal with alignment issues
    int error = posix_memalign((void**) &state, 64,
            sizeof(crypto_generichash_state));

    if (error)
        throw runtime_error(
                string("failed to allocate hash state: ") + strerror(error));

    reset();
}

Hash::~Hash()
{
    free(state);
}

void Hash::reset()
{
    crypto_generichash_init(state, 0, 0, crypto_generichash_BYTES);
    size = 0;
}

void Hash::update(const OctetStream& os)
{
    update(os.get_data(), os.get_length());
}

void Hash::update(const string& str)
{
    update(str.data(), str.size());
}

void Hash::final(OctetStream& os)
{
    os.resize_precise(hash_length);
    os.reset_write_head();
    append_final(os);

}

void Hash::append_final(OctetStream& os)
{
    final(os.append(hash_length));
}

OctetStream Hash::final()
{
    OctetStream res;
    final(res);
    return res;
}

bool Hash::matches(OctetStream& os)
{
    array<unsigned char, hash_length> a, b;
    final(a.data());
    os.consume(b.data(), hash_length);
    return a == b;
}
