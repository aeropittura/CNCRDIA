/*
 * vm/input_tuple.h
 *
 */

#pragma once


template <class T>
struct InputTuple
{
  T share;
  typename T::OpenType value;

  static int size()
    { return T::OpenType::size() + T::size(); }

  static string type_string()
    { return T::type_string(); }

  static void specification(OctetStream& os)
    {
      T::specification(os);
    }

  InputTuple() {}

  InputTuple(const T& share, const typename T::OpenType& value) : share(share), value(value) {}

  template<class U>
  InputTuple(const InputTuple<U>& other) : share(other.share), value(other.value) {}

  void assign(const char* buffer)
    {
      share.assign(buffer);
      value.assign(buffer + T::size());
    }
};


template <class T>
struct RefInputTuple
{
  T& share;
  typename T::OpenType& value;
  RefInputTuple(T& share, typename T::OpenType& value) : share(share), value(value) {}
  void operator=(InputTuple<T>& other) { share = other.share; value = other.value; }
};
