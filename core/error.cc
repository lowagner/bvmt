#include "error.h"

BVMT

error::error(std::string M, const char *A) throw() : Message(M), At(A) {}
error::error(const char *M, const char *A) throw() : Message(M), At(A) {}
error::error(std::string M, std::string A) throw() : Message(M), At(A) {}
error::error(const char *M, std::string A) throw() : Message(M), At(A) {}

const char* error::what() const throw()
{   return Message.c_str();
}

capturer::capturer(std::ostream &Os)
:   Ostream(Os), Out(), OldBuffer(Ostream.rdbuf(Out.rdbuf()))
{}

capturer::~capturer()
{   stopCapture();
}

std::string capturer::pull()
{   std::string Output = Out.str();
    Out.str("");
    return Output;
}

void capturer::stopCapture()
{   if (OldBuffer)
    {   Ostream.rdbuf(OldBuffer);
        OldBuffer = Null;
    }
}

SINGLETON_CC(debug, {})

#ifndef NDEBUG
template <>
bool checkEqual(const char *const &A, const char *const &B) {
    return strcmp(A, B) == 0;
}
template <>
bool checkEqual(const char *const &A, const std::string &B) {
    return B == A;
}
template <>
bool checkEqual(const std::string &A, const char *const &B) {
    return A == B;
}
#endif

TMVB
