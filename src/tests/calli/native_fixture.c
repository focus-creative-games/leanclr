#include <stdint.h>
#include <stdbool.h>
#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else
#define EXPORT
#define STDCALL
#endif
static int32_t add(int32_t x)
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
    return !x;
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

EXPORT void* calli_test_get_function(int32_t index)
{
    switch (index)
    {
    case 0:
        return (void*)add;
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
    default:
        return 0;
    }
}
