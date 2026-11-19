/*
 * support/exceptions.cpp
 *
 */

#include "support/exceptions.h"
#include "numeric/bigint.h"
#include "vm/online_options.h"

void exit_error(const string& message)
{
    if (OnlineOptions::singleton.has_option("throw_exceptions"))
        throw runtime_error(message);

    cerr << message << endl;
    exit(1);
}

IoError::IoError(const string& m) :
        ans(m)
{
}

file_error::file_error(const string& m) :
        ans(m)
{
}

ProcessorError::ProcessorError(const string& m) :
        msg(m)
{
}

ProcessorError::ProcessorError(const char* m) :
        ProcessorError(string(m))
{
}

WrongGfpSize::WrongGfpSize(const char* name, const Bigint& p,
        const char* symbol, int n_limbs) :
        runtime_error(
                string() + name + " wrong size for modulus " + to_string(p)
                        + ". Maybe change " + symbol + " to "
                        + to_string(n_limbs) + ".")
{
}

overflow::overflow(const string& name, size_t i, size_t n) :
    runtime_error(
        name + " overflow: " + to_string(long(i)) + "/" + to_string(n)
            + ((long(i) < 0) ? ". A negative value indicates that "
                               "the computation modulus might be too small" :
                               ""))
{
}

UnknownInputType::UnknownInputType(int type) :
        runtime_error("unkown type: " + to_string(type))
{
}

InvalidOpcode::InvalidOpcode(int opcode) :
        runtime_error("invalid opcode: " + to_string(opcode))
{
}

input_error::input_error(const char* name, const string& filename,
        istream& input_file, size_t input_counter)
{
    input_file.clear();
    string token;
    input_file >> token;
    msg += string() + "cannot read " + name + " from " + filename
            + ", problem with '" + token + "' after "
            + to_string(input_counter);
}

SignatureMismatch::SignatureMismatch(const string& filename, bool has_mac) :
        runtime_error("Signature in " + filename + " doesn't match protocol. " +
                "Maybe re-run preprocessing"
                        + (has_mac ? " or check for MAC mismatch" : ""))
{
}

InsufficientMemory::InsufficientMemory(size_t size, const string& type) :
        runtime_error(
                "program requires too much " + type + " memory: "
                        + to_string(size))
{
}

NotEnoughToBuffer::NotEnoughToBuffer(const string& type, const string& filename)  :
        runtime_error(
                "Not enough data available for buffer"
                        + (filename.empty() ? "" : (" in " + filename)) + ". "
                                "Maybe insufficient preprocessing" + type
                        + ".\nFor benchmarking, you can activate reusing data by "
                                "adding -DINSECURE to the compiler options.")
{
}

Gf2nNotSupported::Gf2nNotSupported(int n, string options) :
        runtime_error(
                "GF(2^" + to_string(n) + ") not supported"
                        + (options.empty() ? "" : ", options are " + options))
{
}

SetupError::SetupError(const string& error) :
        runtime_error(error)
{
}

PrepSetupError::PrepSetupError(const string& error, int nplayers,
        const string& fake_options) :
        SetupError(
                "Something is wrong with the preprocessing data on disk: "
                        + error
                        + "\nHave you run the right program for generating it, "
                                "such as './Fake-Offline.x "
                        + to_string(nplayers) + " " + fake_options + "'?")
{
}

InsufficientShares::InsufficientShares(int expected, int actual, exception& e) :
        runtime_error(
                "expected " + to_string(expected) + " shares but only got "
                        + to_string(actual) + " (" + e.what() + ")")
{
}

PersistenceError::PersistenceError(const string& error) :
        runtime_error(
                "Error while reading from persistence file. "
                "You need to write to it first. "
                "See https://mp-spdz.readthedocs.io/en/latest/io.html#persistence. "
                "Details: " + error)
{
}

BytecodeError::BytecodeError(const string& error) :
        runtime_error(error)
{
}

NoDynamicMemory::NoDynamicMemory() :
        runtime_error("this functionality is only implemented "
                "for online-only BMR, see "
                "https://github.com/data61/MP-SPDZ?tab=readme-ov-file#bmr-1")
{
}

FieldTooSmall::FieldTooSmall(int length, int security) :
        runtime_error(
                "Field too small (" + to_string(length)
                        + " bits) for chosen security (" + to_string(security)
                        + "). Increase size with -lgp or "
                        "decrease security with --security")
{
}

InvalidCommitment::InvalidCommitment(int player, const char* message) :
        runtime_error(
                "invalid commitment"
                        + (player < 0 ?
                                string() : " from party " + to_string(player))
                        + (message ? string(": ") + message : string()))
{
}
