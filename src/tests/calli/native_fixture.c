#include <stdint.h>
#include <stdbool.h>
#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else
#define EXPORT
#define STDCALL
#endif
#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif
EXPORT NOINLINE int32_t calli_bench_add(int32_t x)
{
    return x + 1;
}

static double mixed(int32_t a, double b, float c, int64_t d)
{
    return a + b + c + d;
}

static double many(int32_t a, double b, int32_t c, double d, int32_t e, double f, int32_t g, double h, int32_t i, double j, int32_t k, double l)
{
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

static void byref(int32_t* x, double* y)
{
    *x = 42;
    *y = 84;
}

static int32_t boolean(int32_t x)
{
    return x ? 0 : 256;
}

static uint8_t character(uint8_t x)
{
    return x + 1;
}

static int8_t i1(int8_t x)
{
    return x + 1;
}

static uint8_t u1(uint8_t x)
{
    return x + 1;
}

static int16_t i2(int16_t x)
{
    return x + 1;
}

static uint16_t u2(uint16_t x)
{
    return x + 1;
}

static uint64_t u8(uint64_t x)
{
    return ~x;
}

static void* pointer(void* x)
{
    return x;
}

static double STDCALL stdcall_value(double x)
{
    return x + 1;
}

static float single(float x)
{
    return x * 2;
}

static intptr_t native_int(intptr_t x)
{
    return x + 1;
}

typedef struct
{
    int32_t a, b;
} Small;

typedef struct
{
    int64_t a, b, c;
} Triple;

typedef struct
{
    float a, b;
} FloatPair;

typedef struct
{
    double a, b, c;
} DoubleTriple;

#pragma pack(push, 1)

typedef struct
{
    uint8_t a;
    int32_t b;
    int16_t c;
} Packed;

#pragma pack(pop)

typedef struct
{
    int64_t a;
    int32_t b;
    uint8_t padding[4];
} Explicit;

typedef struct
{
    Small s;
    int64_t l;
} Nested;

static Small small_value(Small v)
{
    v.a++;
    v.b += 2;
    return v;
}

static Triple large_value(Triple v)
{
    v.a++;
    v.b += 2;
    v.c += 3;
    return v;
}

static FloatPair float_value(FloatPair v)
{
    v.a++;
    v.b += 2;
    return v;
}

static DoubleTriple double_value(DoubleTriple v)
{
    v.a++;
    v.b += 2;
    v.c += 3;
    return v;
}

static Packed packed_value(Packed v)
{
    v.a++;
    v.b += 2;
    v.c += 3;
    return v;
}

static Explicit explicit_value(Explicit v)
{
    v.a++;
    v.b += 2;
    return v;
}

static void nested_value(Nested* v)
{
    v->s.a++;
    v->s.b += 2;
    v->l += 3;
}

typedef union
{
    int32_t i;
    float f;
} Union;

static Union union_value(Union v)
{
    v.i += 2;
    return v;
}

static void (*collect_callback)(void);
static int32_t effects;

EXPORT void calli_fixture_set_callback(void (*callback)(void))
{
    collect_callback = callback;
}

EXPORT int32_t calli_fixture_effect_count(void)
{
    return effects;
}

static int32_t native_collect(int32_t* value)
{
    if (collect_callback)
        collect_callback();
    return *value;
}

static int32_t native_collect_stack(void)
{
    if (collect_callback)
        collect_callback();
    return 1;
}

static int32_t native_effect(int32_t x)
{
    ++effects;
    return x + 1;
}

static uint32_t native_generic(uint32_t value)
{
    return value ^ UINT32_C(0x80000000);
}

EXPORT void* calli_test_get_function(int32_t index)
{
    switch (index)
    {
    case 0:
        return (void*)calli_bench_add;
    case 1:
        return (void*)mixed;
    case 2:
        return (void*)many;
    case 3:
        return (void*)byref;
    case 4:
        return (void*)boolean;
    case 5:
        return (void*)character;
    case 6:
        return (void*)i1;
    case 7:
        return (void*)u1;
    case 8:
        return (void*)i2;
    case 9:
        return (void*)u2;
    case 10:
        return (void*)u8;
    case 11:
        return (void*)pointer;
    case 12:
        return (void*)stdcall_value;
    case 13:
        return (void*)single;
    case 14:
        return (void*)native_int;
    case 15:
        return (void*)small_value;
    case 16:
        return (void*)large_value;
    case 17:
        return (void*)float_value;
    case 18:
        return (void*)double_value;
    case 19:
        return (void*)packed_value;
    case 20:
        return (void*)explicit_value;
    case 21:
        return (void*)nested_value;
    case 22:
        return (void*)union_value;
    case 23:
        return (void*)native_collect;
    case 24:
        return (void*)native_collect_stack;
    case 25:
        return (void*)native_effect;
    case 26:
        return (void*)native_generic;
    default:
        return 0;
    }
}
