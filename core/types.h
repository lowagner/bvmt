#pragma once

#define NAMESPACE namespace bvmt {
#define ECAPSEMAN }

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

#include <cstdint>

NAMESPACE

constexpr bool True = true;
constexpr bool False = false;
typedef decltype(nullptr) null;
constexpr null Null = nullptr;

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

ECAPSEMAN
