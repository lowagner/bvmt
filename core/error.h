#pragma once

#include "types.h"

#include <iostream>
#include <exception>
#include <streambuf>
#include <string>
#include <string.h>
#include <sstream>

BVMT

class error : public std::exception
{   // error class for exceptions thrown in bvmt
public:
    const std::string Message;
    const std::string At;
    error(std::string M, const char *A) throw();
    error(const char *M, const char *A) throw();
    error(std::string M, std::string A) throw();
    error(const char *M, std::string A) throw();

    const char* what() const throw();
};

class capturer
{   // class for capturing output (e.g., stderr or stdout)
    // and returning it when desired.
public:
    capturer(std::ostream &Os);

    ~capturer();

    /**
     * Returns current string that has been captured,
     * and resets its internal captured string (so far) to the empty string.
     */
    [[nodiscard("make sure to check for presence or absence of stuff")]]
    std::string pull();

    void stopCapture();
private:
    std::ostream& Ostream;
    std::stringstream Out;
    std::streambuf *OldBuffer;
};

struct debug
{   SINGLETON_H(debug)
public:
    bool Debug = False;
};

#define RED_TEXT_OUTPUT "\033[31m"
#define NORMAL_TEXT_OUTPUT "\033[0m"
#define STRINGIFY(X) #X
#define TO_STRING(X) STRINGIFY(X)
#define AT __FILE__ ":" TO_STRING(__LINE__)
#define TRY(Context, x) \
{   try { x; } \
    catch (const bvmt::error &E) { \
        std::ostringstream StringStream; \
        StringStream.imbue(std::locale("C")); \
        StringStream << Context; \
        std::string StringContext = StringStream.str(); \
        if (StringContext.size()) { \
            StringContext += ": "; \
        } \
        throw bvmt::error(StringContext + E.Message, E.At + "\n    " AT); \
    } \
}

#ifdef NDEBUG
#define DEBUG_ONLY(x) ((void)0)
// Do not log when running NDEBUG:
#define LOG(x) ((void)0)
#define LOG_ERR(x) ((void)0)
#define LOG_DEBUG(x) ((void)0)
#define LOG_INFO(x) ((void)0)
// TODO: make ASSERT(x, y) or ASSERT(x) << "error reason" be a thing.
#define ASSERT(x) ((void)0)
#define ASSERT_THIS(X, y) ((void)0)
#define ASSERT_PROBABLY(x) ((void)0)
#define ASSERT_WORD_ALIGNED(X) ((void)0)
#define TEST(Context, x) ((void)0)
#define TEST_2_TYPES(Context, x, y, actualTest) ((void)0)
#define TEST_2_VALUES(Context, X, Y, actualTest) ((void)0)
#define TEST_4_VALUES(Context, W, X, Y, Z, actualTest) ((void)0)
#define TEST_LARGE_ALLOCATION(Context, x) ((void)0)
#define VISIBLE_FOR_TESTING(x) private: \
    x
#define MOCK(functionOutputType, function, functionInputType, functionArgs, Mock, DebugWrite) \
    static inline functionOutputType function functionInputType \
    {   return ::function functionArgs; \
    }
#define MOCKABLE(x, y) class x { public: y }

#else // DEBUG
static bool TestOnly = False;
#define DEBUG_ONLY(X) X
#define LOG(X) { std::cout << "[" AT "]: " << X << "\n"; }
#define LOG_ERR(X) { std::cerr << "[" AT "]: " << X << "\n"; }
#define LOG_DEBUG(x) { if (debug::get()->Debug) LOG(x); }
#define LOG_INFO(X) LOG(X)
#define ASSERT(X) { if (!(X)) throw bvmt::error("expected " #X, AT); }
#define ASSERT_THIS(X, y) \
{   if (!X . y) \
    {   LOG_ERR("expected " << X << " " #y); \
        throw bvmt::error("expected " #X " " #y, AT); \
    } \
}
#define ASSERT_PROBABLY(X) { if (!(X)) LOG_ERR("expected " #X); }
#define ASSERT_WORD_ALIGNED(X) \
    ASSERT(((i64)(u8 *)&(X)) % sizeof(u8 *) == 0);
#define TEST(Context, x) \
{   TestOnly = True; \
    bvmt::capturer TestPrintOutput(std::cout); \
    TRY(Context, x); \
    TestOnly = False; \
}
#define TEST_TYPE(Context, x, actualTest) \
{   using testType = x; \
    TEST_WITH_CONTEXT(#Context " with " #x " testType", actualTest); \
}
#define TEST_2_TYPES(Context, x, y, actualTest) \
{   TEST_TYPE(Context, x, actualTest); \
    TEST_TYPE(Context, y, actualTest); \
}
#define TEST_3_TYPES(Context, x, y, z, actualTest) \
{   TEST_2_TYPES(Context, x, y, actualTest); \
    TEST_TYPE(Context, z, actualTest); \
}
#define TEST_4_TYPES(Context, w, x, y, z, actualTest) \
{   TEST_2_TYPES(Context, w, x, actualTest); \
    TEST_2_TYPES(Context, y, z, actualTest); \
}
#define TEST_VALUE(Context, X, actualTest) \
{   auto TestValue = X; \
    TEST(#Context " with TestValue=" #X, actualTest); \
} 
#define TEST_2_VALUES(Context, X, Y, actualTest) \
{   TEST_VALUE(Context, X, actualTest); \
    TEST_VALUE(Context, Y, actualTest); \
}
#define TEST_3_VALUES(Context, X, Y, Z, actualTest) \
{   TEST_2_VALUES(Context, X, Y, actualTest); \
    TEST_VALUE(Context, Z, actualTest); \
}
#define TEST_4_VALUES(Context, W, X, Y, Z, actualTest) \
{   TEST_2_VALUES(Context, W, X, actualTest); \
    TEST_2_VALUES(Context, Y, Z, actualTest); \
}
#ifdef LARGE_ALLOCATION
#define TEST_LARGE_ALLOCATION(Context, x) {LOG("large memory test..."); TEST(#Context " large allocation", x);}
#else
#define TEST_LARGE_ALLOCATION(Context, x) ((void)0)
#endif
#define VISIBLE_FOR_TESTING(x) \
    public: \
    x \
    private:
#define MOCK(functionOutputType, function, functionInputType, functionArgs, DebugWrite, Mock) \
    static functionOutputType function functionInputType \
    {   if (MockOnly || Verbose) \
            std::cout << TO_STRING(function) << "(" << DebugWrite << ")"; \
        if (MockOnly) \
            return Mock; \
        return ::function functionArgs; \
    }
#define MOCKABLE(x, y) \
    class x { public: static bool MockOnly; static bool Verbose; y }; \
    bool x::MockOnly = False; \
    bool x::Verbose = False
// Try-catch-finally type idea:
#define TEST_WITH_MOCK(mockClass, x) \
{   mockClass::MockOnly = true; \
    try { TEST(x); } \
    catch (const bvmt::error &E) \
    {   mockClass::MockOnly = false; \
        throw; \
    } \
    mockClass::resetMocks(); \
    mockClass::MockOnly = false; \
}

template <>
bool checkEqual(const char *const &A, const char *const &B);

template <>
bool checkEqual(const char *const &A, const std::string &B);

template <>
bool checkEqual(const std::string &A, const char *const &B);

#define EXPECT_EQUAL(A, B) \
{   const auto AValue = (A); const auto BValue = (B); \
    if (!bvmt::checkEqual(AValue, BValue)) \
    {   LOG_ERR(#A " = " << AValue << " != " #B " = " << BValue); \
        throw bvmt::error("expected " #A " to equal " #B, AT); \
    } \
}
#define EXPECT_NOT_EQUAL(A, B) \
{   const auto AValue = (A); const auto BValue = (B); \
    if (bvmt::checkEqual(AValue, BValue)) \
    {   LOG_ERR(#A " = " << AValue << " == " #B " = " << BValue); \
        throw bvmt::error("expected " #A " NOT to equal " #B, AT); \
    } \
}
#define EXPECT_POINTER_EQUAL(A, B) \
{   auto APointer = (A); \
    ASSERT(APointer != nullptr); \
    const auto &AValue = *APointer; const auto &BValue = (B); \
    if (!bvmt::checkEqual(AValue, BValue)) \
    {   LOG_ERR(#A " = " << AValue << " != " #B " = " << BValue); \
        throw bvmt::error("expected " #A " to equal " #B, AT); \
    } \
}
#define EXPECT_THROW(doStuff, ExpectedError) \
{   bool EXPECT_THROW_DoStuffSucceeded = False; \
    try \
    {   doStuff; \
        EXPECT_THROW_DoStuffSucceeded = True; \
    } \
    catch (const bvmt::error &ActualError) \
    {   if (strcmp(ActualError.what(), ExpectedError)) \
        {   std::string CompoundErrorMsg = std::string( \
                "expected `" #doStuff "` to throw " #ExpectedError " but it threw \"" \
            )   +   ActualError.what()  +   "\""; \
            throw bvmt::error(CompoundErrorMsg, ActualError.At + "\n    " AT); \
        } \
    } \
    catch (const std::exception &ActualError) \
    {   if (strcmp(ActualError.what(), ExpectedError)) \
        {   std::string CompoundErrorMsg = std::string( \
                "expected `" #doStuff "` to throw " #ExpectedError " but it threw \"" \
            )   +   ActualError.what()  +   "\""; \
            throw bvmt::error(CompoundErrorMsg, AT); \
        }\
    } \
    if (EXPECT_THROW_DoStuffSucceeded) \
    {   throw bvmt::error("expected `" #doStuff "` to throw " #ExpectedError, AT); \
    } \
}
#endif
#define MOCK_VOID(function) MOCK(void, function, (), (), "", void())
#define MOCK_VOID_WITH_ARG(function, argType, ArgOnly) \
    MOCK(void, function, (argType ArgOnly), (ArgOnly), ArgOnly, void())
#define MOCK_RETURN(returnType, function, ReturnValue) \
    DEBUG_ONLY(static returnType ReturnValue); \
    MOCK(returnType, function, (), (), "", ReturnValue)
#define MOCK_RETURN_WITH_ARG(returnType, function, argType, ArgOnly, ReturnValue) \
    DEBUG_ONLY(static returnType ReturnValue); \
    MOCK(returnType, function, (argType ArgOnly), (ArgOnly), ArgOnly, ReturnValue)

TMVB
