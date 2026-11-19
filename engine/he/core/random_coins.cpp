
#include "he/core/random_coins.h"
#include "he/core/fhe_keys.h"

IntRandomCoins::IntRandomCoins(const FHE_PK& pk) :
    IntRandomCoins(pk.get_parameters())
{
}

RandomCoins::RandomCoins(const FHE_PK& pk) :
    RandomCoins(pk.get_parameters())
{
}

void add(RandomCoins& ans,const RandomCoins& x,const RandomCoins& y)
{ 
  if (x.params!=y.params)   { throw ParametersMismatch(); }
  if (ans.params!=y.params) { throw ParametersMismatch(); }

  add(ans.uu,x.uu,y.uu); 
  add(ans.vv,x.vv,y.vv); 
  add(ans.ww,x.ww,y.ww); 
}

