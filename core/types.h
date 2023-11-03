#pragma once

#include <cstdint> // int64_t, etc.
#include <functional> // std::function
#include <iostream> // std::ostream

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

#define ABSTRACT_SINGLETON_H(x) \
    protected: x(x *ChildInstance); \
            static x *Instance; \
    public: static x *get(); \
            virtual ~x(); \
    private:

#define ABSTRACT_SINGLETON_CC(x) \
    x::x(x *ChildInstance) { \
        ASSERT(ChildInstance != Null); \
        ASSERT(Instance == Null); \
        Instance = ChildInstance; \
    } \
    x::~x() { \
        /* This triggers within the child class to remove the instance
           if the child class was the currently active instance of this abstract base class. */ \
        if (Instance == this) Instance = Null; \
    } \
    x *x::get() { \
        if (Instance == Null) \
            throw error("Instantiate a child class of " #x " first.", AT); \
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
    inline otherType &unwrap(bvmtType &Bvmt) { \
        static_assert( \
            sizeof(Bvmt.Data) == sizeof(otherType), \
            "bvmt type " #bvmtType " Data does not match size of type " #otherType \
        ); \
        return (otherType &)(Bvmt.Data); \
    } \
    inline const otherType &unwrap(const bvmtType &Bvmt) { \
        static_assert( \
            sizeof(Bvmt.Data) == sizeof(otherType), \
            "bvmt type " #bvmtType " Data does not match size of type " #otherType \
        ); \
        return (const otherType &)(Bvmt.Data); \
    }

#define WRAPPER_DATA(x) public: u8 Data[x]; private:

#define PUSHER_POPPER_H() \
private: \
    int PushCount = 0; \
    void firstPush(); \
    void lastPop(); \
    template<typename t> \
    friend class pushPop;

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

template <class functionType>
using fn = std::function<functionType>;

// returns the hex character ('0' to 'f') for the value passed in (expected to be 0 to 15).
char nibble(u8 Value);

std::ostream &operator << (std::ostream &Out, const u8 &U8);

std::ostream &operator << (std::ostream &Out, const i8 &I8);

template <class t>
const t &constant(t &T) { return (const t &)T; }

template <class functionType>
using fn = std::function<functionType>;

template <class t>
class pointer;

template <class t, bool IsVoid>
struct typeDefault;

template <class t>
struct typeDefault<t, True> {};

template <class t>
struct typeDefault<t, False> {
    static const t DefaultInstance;
};

template <class t>
struct typeData {
    // Specializations for data-like constexpr values.  See
    // https://stackoverflow.com/a/35974120 for why we do this.
    static constexpr index WrapperMemoryBytes = 0;
};

template <class t>
struct typePointer {
    // Specializations for pointer-like constexpr values.  See
    // https://stackoverflow.com/a/35974120 for why we do this.
    // References and pointers are pointer-like:
    static constexpr bool IsPointerLike = std::is_reference<t>::value || std::is_pointer<t>::value;
    static constexpr bool IsPointer = False;

    typedef std::conditional_t<
        std::is_reference<t>::value,
        typename std::remove_reference<t>::type,
        std::conditional_t<
            std::is_pointer<t>::value,
            typename std::remove_pointer<t>::type,
            t
        >
    >   pointingAt;
};

template<class t>
struct typeFunction {
    template <class ...Args>
    using callableReturn = std::invoke_result_t<t, Args...>;
};

template<class t, class u>
struct typeFunction<t(u)> {
    typedef u functionArgument;
    typedef t functionReturn;
};

template<class t, class u>
struct typeFunction<fn<t(u)>> {
    typedef u functionArgument;
    typedef t functionReturn;
};

template <bool Value, class t, class u>
struct maybe {
    typedef std::conditional_t<Value, t, u> type;
};

template <class t>
struct type
:   public typeDefault<t, std::is_void_v<t>>,
    public typeData<t>,
    public typePointer<t>,
    public typeFunction<t>
{   static const char *Name;

    typedef std::remove_const<t> withoutConst;
    typedef std::conditional_t<
        std::is_const_v<t>,
        std::remove_const<t>,
        const t
    >       switchConst;

    static constexpr bool IsConstant = std::is_const_v<t>;
    static constexpr bool IsMutable = !std::is_const_v<t>;
    static constexpr bool IsCopyConstructible = std::is_copy_constructible_v<t>;

    static constexpr bool IsIntegral = std::is_integral<t>::value;
    static constexpr bool IsFloating = std::is_floating_point<t>::value;
    static constexpr bool IsUnsigned = std::is_unsigned<t>::value;

    static constexpr bool IsVoid = std::is_void_v<t>;

    static constexpr bool IsReference = std::is_reference<t>::value;
    static constexpr bool IsCPointer = std::is_pointer<t>::value;
    static constexpr bool IsCppPointer = std::is_pointer<t>::value || std::is_reference<t>::value;

    static constexpr t min() {
        return std::numeric_limits<t>::min();
    }
    static constexpr t max() {
        return std::numeric_limits<t>::max();
    }

    template <class u>
    static constexpr t clamp(u U) {
        if (U > max()) {
            return max();
        }
        if (U < min()) {
            return min();
        }
        return t(U);
    }

    template <class u>
    static constexpr bool equals() {
        return std::is_same<t, u>::value;
    }

    template <class u>
    static constexpr bool extends() {
        return std::is_base_of<u, t>::value;
    }
};


template <class t>
t sign(const t &T) {
    static_assert(!type<t>::IsUnsigned, "need a signed type here.");
    return T == t(0) ? t(0) : T > t(0) ? t(1) : t(-1);
}

#define typeForExpression(x) type<decltype(x)>

template <class t>
const char *type<t>::Name = typeid(t).name();

// TODO: more of these
template <>
const char *type<i64>::Name;

template <class t>
const t typeDefault<t, False>::DefaultInstance = t();

template <class t>
struct typePointer<pointer<t>> {
    static constexpr bool IsPointerLike = True;
    static constexpr bool IsPointer = True;
    typedef t pointingAt;
};

#define $ template 
#define $$ ::template 

#define ENABLE_TEMPLATE(t, when) \
    template< \
        class enablerType = t, \
        typename = std::enable_if_t<when> \
    >

#define ENABLE_FOR_VALUE_TYPES(t) \
    template \
    <   class enablerType = t, \
        typename = std::enable_if_t<!type<enablerType>::IsPointerLike> \
    >

#define ENABLE_FOR_POINTER_LIKE_TYPES(t) \
    template \
    <   class enablerType = t, \
        typename = std::enable_if_t<type<enablerType>::IsPointerLike> \
    >

#define IS_POINTER_LIKE(t) \
    constexpr (type<t>::IsPointerLike)

// TODO: any other true primitives that we care about.
#define ALL_PRIMITIVES(x) \
    x(i64) \
    x(i32) \
    x(i16) \
    x(i8) \
    x(u64) \
    x(u32) \
    x(u16) \
    x(u8) \
    x(dbl) \
    x(flt)

#define GETTER_SWAPPER_TEMPLATE(templatePreamble, t, accessorSwapper, Location) \
    templatePreamble \
    t accessorSwapper() const { return Location; } \
    templatePreamble \
    t accessorSwapper(t NewValue) { std::swap(Location, NewValue); return NewValue; }

#define UNMOVABLE_CLASS(x) \
    x(x &&IgnoreInstance) = delete; \
    x &operator =(x &&IgnoreInstance) = delete;

#define UNCOPYABLE_CLASS(type) \
    type (const type &ExistingInstance) = delete; \
    type &operator = (const type &ExistingInstance) = delete;

#define SET_EQUAL_GUARD(Other, do_this) \
    if ((void *)&Other != (void*)this) { \
        do_this; \
    } \
    return This

#define COPYABLE_TEMPLATE(x, X, startingLogicForAssignment, logic) \
    x(const x &X) { \
        logic; \
    } \
    x &operator = (const x &X) { \
        SET_EQUAL_GUARD(X, startingLogicForAssignment; logic); \
    }

#define COPYABLE_H(x, X) \
    x(const x &X); \
    x &operator = (const x &X);

#define COPYABLE_CC(x, X, startingLogicForAssignment, logic) \
    x::x(const x &X) { \
        logic; \
    } \
    x &x::operator = (const x &X) { \
        SET_EQUAL_GUARD(X, startingLogicForAssignment; logic); \
    }

#define MOVABLE_TEMPLATE(x, X, startingLogicForAssignment, logic) \
    x(x &&X) noexcept { \
        logic; \
    } \
    x &operator = (x &&X) noexcept { \
        SET_EQUAL_GUARD(X, startingLogicForAssignment; logic); \
    }

#define MOVABLE_H(x, X) \
    x(x &&X) noexcept; \
    x &operator = (x &&X) noexcept;

#define MOVABLE_CC(x, X, startingLogicForAssignment, logic) \
    x::x(x &&X) noexcept { \
        logic; \
    } \
    x &x::operator = (x &&X) noexcept { \
        SET_EQUAL_GUARD(X, startingLogicForAssignment; logic); \
    }

#define CAST_DEFINE(type, VariableName, FromVariable) \
    type VariableName = (type)FromVariable

#define NUMBER_OPERATOR_X_AND_X_EQUALS_H(className, op, classNameOther) \
    className operator op (classNameOther Other) const; \
    className &operator op##= (classNameOther Other); 

#define NUMBER_OPERATOR_X_CC(className, op, classNameOther) \
    className className::operator op (classNameOther Other) const { \
        className NewValue = This; \
        NewValue op##= Other; \
        return NewValue; \
    }

#define NUMBER_OPERATOR_X_EQUALS_CC(className, op, classNameOther) \
    className &className::operator op##= (classNameOther Other) { \
        this = This op Other; \
        return This; \
    }

#define EMPTY

#define NUMBER_OPERATOR_X_EQUALS_TEMPLATE_TEMPLATE(className, op, templateOther, classNameOther, logic) \
    templateOther \
    className &operator op##= (classNameOther Other) & { \
        logic; \
        return This; \
    } \
    /* TODO: & and && versions on *this */ \
    templateOther \
    className operator op (classNameOther Other) const { \
        className ReturnValue = This; \
        ReturnValue op##= Other; \
        return ReturnValue; \
    }

#define NUMBER_OPERATOR_X_EQUALS_TEMPLATE(className, op, classNameOther, logic) \
    NUMBER_OPERATOR_X_EQUALS_TEMPLATE_TEMPLATE(className, op, EMPTY, classNameOther, logic)

#define OPERATOR_X_FROM_X_EQUALS_TEMPLATE(className, op) \
    template <class other> \
    className operator op (other Other) const & { \
        className Result = This; /* copy */ \
        Result op##= Other; \
        return Result; \
    } \
    template <class other> \
    className operator op (other Other) && { \
        className Result = std::move(This); \
        Result op##= Other; \
        return Result; \
    }

// For safely wrapping a type that has multiple template types, e.g. <t, u>, into a macro:
#define SAFE(t) typename typeFunction<void(t)>::functionArgument
#define AND ,

template <class t, class u>
bool checkEqualFloat(const t &A, const u &B) {
    static_assert(std::is_floating_point<t>::value);
    t AbsDelta = std::abs(A - B);
    t AbsMin = std::min(std::abs(A), std::abs(t(B)));
    if (AbsMin < 1e-2) {
        return AbsDelta < 1e-7;
    }
    return AbsDelta / AbsMin < 1e-5;
}

template <class t, class u>
bool checkEqual(const t &A, const u &B) {
    if constexpr (std::is_floating_point<t>::value || std::is_floating_point<u>::value) {
        if constexpr (std::is_floating_point<t>::value) {
            return checkEqualFloat(A, B);
        }
        else {
            return checkEqualFloat(B, A);
        }
    }
    return A == B;
}

#ifndef NDEBUG
namespace test {
    template <size_t Size>
    class noisyU8s {
        // type that prints when constructed/moved/destroyed
        static_assert(Size > 0);
        static_assert(Size <= 8);
        u8 Values[Size] = {0};
    public:
        void value(i64 Value) {
            u64 UV = Value;
            u8 *U8s = Values;
            for (size_t I = 0; I < Size; ++I) {
                *(U8s++) = UV & 255;
                UV >>= 8;
            }
        }

        explicit noisyU8s(i64 Value = -1) {
            value(Value);
            std::cout << This;
        }

        i64 value() const {
            u64 UV = 0;
            const u8 *U8s = Values;
            bool Negative = False;
            for (size_t I = 0; I < Size; ++I) {
                u8 U8 = *(U8s++);
                if (I == Size - 1 && (U8 & 128))
                {   U8 &= 127;
                    Negative = True;
                }
                UV |= U8 << (8 * I);
            }
            return Negative
                // do twos complement for negative values:
                ?   UV - (1L << (8*Size - 1)) 
                :   UV
            ;
        }

        noisyU8s(const noisyU8s& N) {
            copy(N);
            std::cout << "{CC}";
        }

        noisyU8s &operator = (const noisyU8s &N) {
            if (this == &N) {
                std::cout << "[oops, copying to self]";
                return This;
            }
            copy(N);
            std::cout << "{CA}";
            return This;
        }

        noisyU8s(noisyU8s&& N) {
        {   copy(N);
            std::cout << "{MC}";
            N.value(-N.value());
        }

        noisyU8s &operator = (noisyU8s &&N) {
            if (this == &N) {
                std::cout << "[oops, moving to self]";
                return This;
            }
            copy(N);
            std::cout << "{MA}";
            N.value(-N.value());
            return This;
        }

        ~noisyU8s() {
            std::cout << "~" << This;
        }

        bool operator == (const noisyU8s &Other) const {
            for (size_t I = 0; I < Size; ++I) {
                if (Values[I] != Other.Values[I]) {
                    return False;
                }
            }
            return True;
        }

    private:
        void copy(const noisyU8s &N) {
            for (size_t I = 0; I < Size; ++I) {
                Values[I] = N.Values[I];
            }
            std::cout << This;
        }
    };

    template <size_t Size>
    std::ostream &operator << (std::ostream & Out, const noisyU8s<Size> &Noisy) {
        return Out << "noisy(" << Noisy.value() << ")";
    }

    class noisy {
        // type that prints when constructed/moved/destroyed
        const char *TypeName;
    public:
        i64 Value;
        
        explicit noisy(i64 V = -1, const char *T = "noisy");

        virtual void copyOver(const noisy& From);

        virtual void print(std::ostream &Out = std::cout) const;

        noisy(const noisy& N);

        noisy &operator = (const noisy &N);

        noisy(noisy&& N);

        noisy &operator = (noisy &&N);

        virtual ~noisy();

        friend bool operator == (const noisy &N1, const noisy &N2);

    protected:
        virtual bool equals(const noisy &Other) const;
    };

    bool operator == (const noisy &N1, const noisy &N2);
    std::ostream &operator <<(std::ostream &Out, const noisy &N);

    class child;

    class parent : public noisy {
        public:
        parent(i64 V = -1);
        parent(const parent &Other); // CC
        parent(parent &&Other); // MC

        parent &operator = (parent &&Other); // MA
        parent &operator = (const parent &Other); // CA

        virtual void print(std::ostream &Out = std::cout) const;
        virtual ~parent();

    protected:
        virtual bool equals(const noisy &Other) const override;
    };

    class child : public parent {
        public:
        std::string Name;

        child(std::string N, i64 V = -10);

        child(const child &Other);
        child(child &&Other);

        virtual void copyOver(const child& From);

        virtual void print(std::ostream &Out = std::cout) const;
        virtual ~child();

    protected:
        virtual bool equals(const noisy &Other) const override;
    };

    class aunt : public noisy {
        // NO INHERITANCE from parent/child
    public:
        std::string Name;

        aunt(std::string N = "nil");

        aunt(const child &Child);

        virtual void copyOver(const aunt& From);

        virtual void print(std::ostream &Out = std::cout) const;

        ~aunt();
    };
}

template <>
struct typeData<test::noisy> {
    static constexpr index WrapperMemoryBytes =
            8   // VTable
        +   8   // const char *TypeName
        +   8   // i64 Value
        +   sizeof(std::string); // for child (and aunt) class
};

template <>
bool checkEqual(const test::child &A, const test::aunt &B);

template <>
bool checkEqual(const test::aunt &B, const test::child &A);

template <>
bool checkEqual(const test::aunt &A, const test::aunt &B);

template <>
bool checkEqual(const test::parent &A, const test::aunt &B);

template <>
bool checkEqual(const test::aunt &B, const test::parent &A);

#endif

TMVB
