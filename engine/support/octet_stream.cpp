
#include <fcntl.h>
#include <sodium.h>

#include "support/octet_stream.h"
#include <string.h>
#include "net/sockets.h"
#include "net/ssl_sockets.h"
#include "support/exceptions.h"
#include "net/data.h"
#include "net/player.h"
#include "net/exchanger.h"
#include "numeric/bigint.h"
#include "support/time_func.h"
#include "support/flex_buffer.h"


void OctetStream::reset()
{
    data = 0;
    data_end = 0;
    ptr = 0;
    end = 0;
}

void OctetStream::reset_reserve(size_t size)
{
	reset_write_head();
	reserve(size);
}

void OctetStream::clear()
{
    if (data)
        delete[] data;
    reset();
}

void OctetStream::assign(const OctetStream& os)
{
  auto read = os.get_ptr();
  if (os.get_length() >= get_max_length())
    {
      if (data)
        delete[] data;
      data=new Octet[os.get_max_length()];
      set_max_length(os.get_max_length());
    }
  set_length(os.get_length());
  memcpy(data,os.data,get_length()*sizeof(Octet));
  ptr = data + read;
  bits = os.bits;
}


OctetStream::OctetStream(size_t maxlen)
{
  data=new Octet[maxlen];
  ptr=data;
  set_length(0);
  set_max_length(maxlen);
}

OctetStream::OctetStream(size_t len, const Octet* source) :
    OctetStream(len)
{
  append(source, len);
}

OctetStream::OctetStream(const string& other) :
    OctetStream(other.size(), (const Octet*)other.data())
{
}

OctetStream::OctetStream(const OctetStream& os)
{
  data=new Octet[os.get_max_length()];
  set_length(os.get_length());
  set_max_length(os.get_max_length());
  memcpy(data,os.data,get_length()*sizeof(Octet));
  ptr = data + os.get_ptr();
  bits = os.bits;
}

OctetStream::OctetStream(FlexBuffer& buffer)
{
  data = (Octet*)buffer.data();
  set_length(buffer.size());
  set_max_length(buffer.capacity());
  ptr = (Octet*)buffer.ptr;
  buffer.reset();
}


string OctetStream::str() const
{
  return string((char*) get_data(), get_length());
}


void OctetStream::hash(OctetStream& output) const
{
  output.resize(crypto_generichash_BYTES_MIN);
  crypto_generichash(output.data, crypto_generichash_BYTES_MIN, data,
      get_length(), NULL, 0);
  output.set_length(crypto_generichash_BYTES_MIN);
}


OctetStream OctetStream::hash() const
{
  OctetStream h(crypto_generichash_BYTES_MIN);
  hash(h);
  return h;
}


Bigint OctetStream::check_sum(int req_bytes) const
{
  auto hash = new unsigned char[req_bytes];
  crypto_generichash(hash, req_bytes, data, get_length(), NULL, 0);

  Bigint ans;
  bigint_from_bytes(ans,hash,req_bytes);
  // cout << ans << "\n";
  delete[] hash;
  return ans;
}


bool OctetStream::equals(const OctetStream& a) const
{
  if (get_length()!=a.get_length()) { return false; }
  return memcmp(data, a.data, get_length()) == 0;
}


void OctetStream::flush_bits()
{
  bits[0].n = 0;
  store_int<1>(bits[0].buffer);
  bits[0].buffer = 0;
}


void OctetStream::append_random(size_t num)
{
  randombytes_buf(append(num), num);
}


void OctetStream::concat(const OctetStream& os)
{
  memcpy(append(os.get_length()), os.data, os.get_length()*sizeof(Octet));
}


void OctetStream::store_bytes(Octet* x, const size_t l)
{
  encode_length(append(4), l, 4);
  memcpy(append(l), x, l*sizeof(Octet));
}

void OctetStream::get_bytes(Octet* ans, size_t& length)
{
  auto rec_length = get_int(4);
  if (rec_length != length)
    throw runtime_error("unexpected length");
  memcpy(ans, consume(length), length * sizeof(Octet));
}

void OctetStream::store(int l)
{
  encode_length(append(4), l, 4);
}


void OctetStream::get(int& l)
{
  l = get_int(4);
}


void OctetStream::store(const Bigint& x, long num)
{
  *append(1) = x < 0;

  if (num <= 0)
    {
      num = numBytes(x);
      encode_length(append(4), num, 4);
    }

  bytes_from_bigint(append(num), x, num);
}


void OctetStream::get(Bigint& ans, long length)
{
  int sign = *consume(1);
  if (sign!=0 && sign!=1) { throw BadValue(); }

  if (length <= 0)
    length = get_int(4);

  if (length!=0)
    {
      bigint_from_bytes(ans, consume(length), length);
      if (sign)
        mpz_neg(ans.get_mpz_t(), ans.get_mpz_t());
    }
  else
    ans=0;
}


void OctetStream::store(const string& str)
{
  store(str.length());
  append((const Octet*) str.data(), str.length());
}


void OctetStream::get(string& str)
{
  size_t size;
  get(size);
  str.assign((const char*) consume(size), size);
}


template<class T>
void OctetStream::exchange(T send_socket, T receive_socket, OctetStream& receive_stream) const
{
  Exchanger<T> exchanger(send_socket, *this, receive_socket, receive_stream);
  while (exchanger.round())
    ;
}



void OctetStream::input(const string& filename)
{
  ifstream s(filename);
  if (not s.good())
    throw file_error("cannot read from " + filename);
  input(s);
}

void OctetStream::input(istream& s)
{
  size_t size;
  s.read((char*)&size, sizeof(size));
  if (not s.good())
    throw IoError("not enough data");
  resize_min(size);
  s.read((char*)data, size);
  set_length(size);
  if (not s.good())
    throw IoError("not enough data");
  reset_read_head();
}

void OctetStream::output(ostream& s) const
{
  auto len = get_length();
  s.write((char*)&len, sizeof(len));
  s.write((char*)data, len);
}

void OctetStream::output(const string& filename)
{
  ofstream s(filename);
  output(s);
  if (not s.good())
   throw file_error("cannot write to " + filename);
  s.close();
}

ostream& operator<<(ostream& s,const OctetStream& o)
{
  for (size_t i=0; i<o.get_length(); i++)
    { int t0=o.data[i]&15;
      int t1=o.data[i]>>4;
      s << hex << t1 << t0 << dec;
    }
  return s;
}

OctetStreams::OctetStreams(const Player& P)
{
  reset(P);
}

void OctetStreams::reset(const Player& P)
{
  resize(P.num_players());
  for (auto& o : *this)
    o.reset_write_head();
}

template void OctetStream::exchange(int, int, OctetStream&) const;
template void OctetStream::exchange(SslSocket*, SslSocket*, OctetStream&) const;
