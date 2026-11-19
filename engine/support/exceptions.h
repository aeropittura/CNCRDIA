#pragma once

#include <exception>
#include <string>
#include <sstream>
#include <stdexcept>
using namespace std;

void exit_error(const string& message);

class NotImplemented: public exception
    { virtual const char* what() const throw()
        { return "Case not implemented"; }
    };
class DivisionByZero: public exception
    { virtual const char* what() const throw()
        { return "Division by zero"; }
    };
class InvalidPlaintext: public exception
    { virtual const char* what() const throw()
        { return "Inconsistent plaintext space"; }
    };
class RepMismatch: public exception
    { virtual const char* what() const throw()
        { return "Representation mismatch"; }
    };
class PrMismatch: public exception
    { virtual const char* what() const throw()
        { return "Prime mismatch"; }
    };
class ParametersMismatch: public exception
    { virtual const char* what() const throw()
        { return "FHE params mismatch"; }
    };
class FieldMismatch: public exception
    { virtual const char* what() const throw()
        { return "Plaintext Field mismatch"; }
    };
class LevelMismatch: public exception
    { virtual const char* what() const throw()
        { return "Level mismatch"; }
    };
class InvalidLength: public runtime_error
    {
    public:
      InvalidLength(string msg = "") : runtime_error("Invalid length: " + msg) {}
    };
class InvalidCommitment: public runtime_error
    {
public:
    InvalidCommitment(int player = -1, const char* message = 0);
    };
class IoError: public exception
    {
      string ans;
      public:
      IoError(const string& m);
      ~IoError()throw() { }
      virtual const char* what() const throw()
        {
          return ans.c_str(); 
        }
    };
class BroadcastInvalid: public exception
    { virtual const char* what() const throw()
        { return "Inconsistent broadcast at some point"; }
    };
class BadKeygen: public runtime_error
    {
      public:
      BadKeygen(string m) : runtime_error("KeyGen has gone wrong: "+m) {}
    };
class BadEnccommit:  public exception
    { virtual const char* what() const throw()
        { return "Error in EncCommit"; }
    };
class InvalidParameters:  public exception
    { virtual const char* what() const throw()
        { return "Invalid Params"; }
    };
class BadValue: public runtime_error
    {
      public:
      BadValue(string msg = "Some value is wrong somewhere") : runtime_error(msg) {}
    };
class OfflineCheckError: public runtime_error
    {
      public:
      OfflineCheckError(string m) :
          runtime_error("Offline-Check-Error : " + m) {}
    };
class MacFail:  public BadValue
    {
      public:
      MacFail(string msg = "MacCheck Failure") : BadValue(msg) {}
    };
class ConsistencyCheckFail:  public exception
    { virtual const char* what() const throw()
        { return "OT consistency check failed"; }
    };
class InvalidProgram:  public exception
    { virtual const char* what() const throw()
        { return "Invalid Program"; }
    };
class file_error:  public exception
    {
      string ans;
      public:
      file_error(const string& m);
      ~file_error()throw() { }
      virtual const char* what() const throw()
        {
          return ans.c_str();
        }
    };
class EndOfFile: public exception
    { string filename, context, ans;
      public:
      EndOfFile(string pfilename="no filename", string pcontext="") : 
        filename(pfilename), context(pcontext)
        {
          ans="End of file when reading ";
          ans+=filename;
          ans+=" ";
          ans+=context;
        }
      ~EndOfFile()throw() { }
      virtual const char* what() const throw()
        {
          return ans.c_str();
        }
    };
class FileMissing:  public exception
    { string filename, context, ans;
      public:
      FileMissing(string pfilename="no filename", string pcontext="") : 
        filename(pfilename), context(pcontext)
        {
          ans="File missing : ";
          ans+=filename;
          ans+=" ";
          ans+=context;
        }
      ~FileMissing()throw() { }
      virtual const char* what() const throw()
        {
          return ans.c_str();
        }
    };    
class ProcessorError: public exception
    { string msg;
      public:
      ProcessorError(const string& m);
      ProcessorError(const char* m);
      ~ProcessorError()throw() { }
      virtual const char* what() const throw()
        {
          return msg.c_str();
        }
    };

class MaxModSizeTooSmall : public exception
    {
      string msg;
      public:
      MaxModSizeTooSmall(int len)
        { stringstream out;
          out << "MAX_MOD_SZ too small for desired bit length of p, "
                  << "must be at least ceil(len(p)/len(word)), "
                  << "in this case: " << len;
          msg = out.str();
        }
      ~MaxModSizeTooSmall() throw() {}
      virtual const char* what() const throw()
        {
          return msg.c_str();
        }
    };
class CrashRequested: public exception
    { virtual const char* what() const throw()
        { return "Crash requested by program"; }
    };
class MemoryException : public exception {};
class HowWouldThatWork : public exception {};
class NotEnoughToBuffer : public runtime_error
{
public:
    NotEnoughToBuffer(const string& type, const string& filename);
};
class NeedsCleaning : public exception {};

class ClosedConnection : public exception
{
    const char* what() const throw()
    {
        return "Connection closed down. "
            "This is most likely due to a problem on the other side. "
            "If there's no error message on the other side, it might be "
            "due to a lack of memory. See "
            "https://mp-spdz.readthedocs.io/en/latest/troubleshooting.html#crash-without-error-message-killed-or-bad-alloc "
            "for possible remedies.";
    }
};

class NoSingleton : public runtime_error
{
public:
    NoSingleton(string msg = "no singleton") :
            runtime_error(msg)
    {
    }
};

class RanOut : public exception
{
    const char* what() const throw()
    {
        return "insufficient preprocessing";
    }
};

class Bigint;

class WrongGfpSize : public runtime_error
{
public:
    WrongGfpSize(const char* name, const Bigint& p, const char* symbol,
            int n_limbs);
};

class MacKeyError: public runtime_error
{
public:
    MacKeyError(string filename) :
            runtime_error("error loading MAC key from " + filename)
    {
    }
};

class overflow : public runtime_error
{
public:
    overflow(const string& name, size_t i, size_t n);
};

class UnknownInputType : public runtime_error
{
public:
    UnknownInputType(int type);
};

class InvalidOpcode : public runtime_error
{
public:
    InvalidOpcode(int opcode);
};

class input_error : public exception
{
    string msg;

public:
    input_error(const char* name, const string& filename,
            istream& input_file, size_t input_counter);

    const char* what() const throw()
    {
        return msg.c_str();
    }
};

class SignatureMismatch : public runtime_error
{
public:
    SignatureMismatch(const string& filename, bool has_mac = false);
};

class InsufficientMemory : public runtime_error
{
public:
    InsufficientMemory(size_t size, const string& type);
};

class Gf2nNotSupported : public runtime_error
{
public:
    Gf2nNotSupported(int n, string options = "");
};

class SetupError : public runtime_error
{
public:
    SetupError(const string& error);
};

class PrepSetupError : public SetupError
{
public:
    PrepSetupError(const string& error, int nplayers, const string& fake_options);
};

class InsufficientShares : public runtime_error
{
public:
    InsufficientShares(int expected, int actual, exception& e);
};

class PersistenceError : public runtime_error
{
public:
    PersistenceError(const string& error);
};

class BytecodeError : public runtime_error
{
public:
    BytecodeError(const string& error);
};

typedef BytecodeError InvalidInstruction;

class NoDynamicMemory : public runtime_error
{
public:
    NoDynamicMemory();
};

class FieldTooSmall : public runtime_error
{
public:
    FieldTooSmall(int length, int security);
};
