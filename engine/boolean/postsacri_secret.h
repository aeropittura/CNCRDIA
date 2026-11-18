/*
 * boolean/postsacri_secret.h
 *
 */

#pragma once

#include "boolean/malicious_rep_secret.h"

namespace GC
{

class PostSacriBin;

class PostSacriSecret : public MalRepSecretBase<PostSacriSecret>
{
    typedef PostSacriSecret This;
    typedef MalRepSecretBase<This> super;

public:
    typedef PostSacriBin Protocol;

    PostSacriSecret()
    {
    }

    template<class T>
    PostSacriSecret(const T& other) :
            super(other)
    {
    }
};

}
