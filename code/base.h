/* date = March 23rd 2024 2:30 pm */

#ifndef CIEL_BASE_H
#define CIEL_BASE_H

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>


typedef float F32;
typedef double F64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t   I8;
typedef int32_t  I16;
typedef int32_t  I32;
typedef int64_t  I64;
typedef U32 B32;
typedef U64 B64;

typedef void Void;
typedef B32 Bool;
typedef char Char;
typedef I32 Int;
typedef U32 UInt;
typedef F32 Float;

#define kilobytes(x) (x * 1000)
#define megabytes(x) (x * 1000000)

struct _ansi_colors
{
#ifdef _WIN32
    const Char *red = "[31m";
    const Char *green = "[32m";
    const Char *yellow = "[33m";
    const Char *blue = "[34m";
    const Char *magenta = "[35m";
    const Char *cyan = "[36m";
    const Char *reset = "[0m";
#else
    const Char *red = "\x1b[31m";
    const Char *green = "\x1b[32m";
    const Char *yellow = "\x1b[33m";
    const Char *blue = "\x1b[34m";
    const Char *magenta = "\x1b[35m";
    const Char *cyan = "\x1b[36m";
    const Char *reset = "\x1b[0m";
#endif
};

_ansi_colors ansi_colors;
typedef const Char * ansi_color;


#ifndef TEMPEST_RELEASE
#define _assert(X) { if(!(X)) {\
Char *NullAddr = (Char *) 0; *NullAddr = 0;\
} }
#else
#define _assert(X) { if(!(X)) {\
} }
#endif


#define ASSERT(X) {\
if(!(X))\
{\
_assert((X))\
}\
}
Bool Logging_StopOnWarning = false;
Bool Logging_StopOnError = false;


Void
println(const Char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    vprintf(Format, Args);
    printf("\n");
}

Void
print(const Char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    vprintf(Format, Args);
}

Void
print_error(const Char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    printf(ansi_colors.red);\
    printf("ERROR: ");\
    vprintf(Format, Args);
    printf(ansi_colors.reset);\
    printf("\n");
}
Void
print_warning(const Char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    printf(ansi_colors.red);\
    printf("ERROR: ");\
    vprintf(Format, Args);
    printf(ansi_colors.reset);\
    printf("\n");
}



Void *
_alloc(U64 Size, const Char *CalledByFunction, int CalledByLine, const Char *CalledByFile)
{
    Void *Result = (Void *)malloc(Size);
    if(Result != NULL)
    {
        return Result;
    }
    else
    {
        print_error("Failed malloc. Called by: %s, %s, line %d", CalledByFile, CalledByFunction, CalledByLine);
        ASSERT(false);
        return NULL;
    }
}

Void *
_resize_alloc(Void *Data, U64 Size, const Char *CalledByFunction, int CalledByLine, const Char *CalledByFile)
{
    Void *Result = (Void *)realloc(Data, Size);
    if(Result != NULL)
    {
        return Result;
    }
    else
    {
        print_error("Failed realloc. Called by: %s, %s, line %d", CalledByFile, CalledByFunction, CalledByLine);
        return NULL;
    }
}

#define alloc(Size) _alloc(Size, __func__, __LINE__, __FILE__)
#define resize_alloc(Data, Size) _resize_alloc(Data, Size, __func__, __LINE__, __FILE__)

Void
zero_memory(Void *memory, U64 byte_count)
{
    U8 *memory_bytes = (U8 *)memory;
    for(Int i = 0; i < byte_count; i++)
    {
        memory_bytes[i] = 0;
    }
}

template <typename t>
U32 isolate_bit(t Data, int BitIndex)
{
    int Size = sizeof(Data) * 8;
    
    if(sizeof(Data) > 64)
    {
        print_warning("Cannot isolate bit in data > 64 bits");
        return 0;
    }
    
    if(BitIndex < 0 || BitIndex >= Size)
    {
        print_warning("Bit Index outside of data bounds");
        return 0;
    }
    
    U64 Mask = ((U64)1) << (Size - 1 - BitIndex);
    Char Val = Data & Mask;
    
    if(Val > 0)
        return 1;
    else
        return 0;
}



#endif //CIEL_BASE_H
