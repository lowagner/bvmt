#pragma once

#define BVMT namespace bvmt {
#define TMVB }

#define DELETE(x) {delete (x); (x) = Null;}

#define SINGLETON_H(x) \
    private: x(); \
            static x *Instance; \
    public: static x *get(); \

#define SINGLETON_CC(x, initLogic) \
    x::x() initLogic \
    x *x::get() { \
        if (Instance == Null) \
            Instance = new x(); \
        return Instance; \
    }  \
    x *x::Instance = Null;

#define UNMOVABLE_CLASS(x) \
    x(x &&IgnoreInstance) = delete; \
    x &operator =(x &&IgnoreInstance) = delete;

#define UNCOPYABLE_CLASS(type) \
    type (const type &ExistingInstance) = delete; \
    type &operator = (const type &ExistingInstance) = delete;

// We should never use WRAPPER with classes that can have children.
// TODO: maybe fix the sizeof expressions to accomodate the vtable.
#define WRAPPER(bvmtType, otherType) \
    inline otherType &unwrap(bvmtType &Data) { \
        static_assert( \
            sizeof(bvmtType) == sizeof(otherType), \
            "bvmt type " #bvmtType " does not match size of type " #otherType \
        ); \
        return (otherType &)Data; \
    } \
    inline const otherType &unwrap(const bvmtType &Data) { \
        static_assert( \
            sizeof(bvmtType) == sizeof(otherType), \
            "bvmt type " #bvmtType " does not match size of type " #otherType \
        ); \
        return (const otherType &)Data; \
    }

#include <cstdint>

BVMT

constexpr bool True = true;
constexpr bool False = false;
typedef decltype(nullptr) null;
constexpr null Null = nullptr;
#define This (*this)

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

TMVB
