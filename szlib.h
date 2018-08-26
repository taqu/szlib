#ifndef INC_SZLIB_H_
#define INC_SZLIB_H_
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
/**
@author t-sakai
@date 2018/01/07 create
@date 2018/08/04 add createInflate
@date 2018/08/17 add deflate (no compression and static haffuman)

USAGE:
Put '#define SZLIB_IMPLEMENTATION' before including this file to create the implementation.

--- CPP sample
using namespace szlib;
sz_s32 srcSize; //source data size
sz_u8 src[srcSize]; //source data

sz_s32 ret;
szContext context;
ret = initInflate(&context, srcSize, src);
if(SZ_OK != ret){
    return -1;
}
const sz_s32 Chunk = 16384;
sz_u8 out[Chunk];
sz_s32 outCount = 0;
size_t total = 0;
std::vector<sz_u8> dst;
for(;;){
    context.availOut_ = Chunk;
    context.nextOut_ = out;
    ret = inflate(&context);
    switch(ret)
    {
    case SZ_ERROR_MEMORY:
    case SZ_ERROR_FORMAT:
        break;
    default:
        total = outCount+context.thisTimeOut_;
        if(dst.capacity()<total){
            size_t capacity = dst.capacity();
            while(capacity<total){
                capacity += 1024;
            }
            dst.reserve(capacity);
        }
        dst.resize(total);
        memcpy(&dst[0]+outCount, out, context.thisTimeOut_);
        outCount += context.thisTimeOut_;
        if(SZ_END!=ret){
            continue;
        }
        break;
    };
    break;
}
termInflate(&context);
return ret == SZ_END? outCount : -1;
---

*/

/*
--- note
-------------------------------
zlib: https://tools.ietf.org/html/rfc1950
deflate: https://tools.ietf.org/html/rfc1951

Fixed Huffman Codes
Literal   Bits Codes
-------   ---- -----
0 - 143    8   00110000 - 10111111
144 - 255  9   110010000 - 111111111
256 - 279  7   0000000 - 0010111
280 - 287  8   11000000 - 11000111

Distance Bits Codes
-------- ---- -----
0-31      5   00000 - 11111
*/
#ifdef __cplusplus
#include <cstdint>
#include <cassert>
#include <cstddef>
#else
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
namespace szlib
{
#endif

//C++98 199711L
//C++11 201103L
#ifdef __cplusplus
#   if 201103L<=__cplusplus || 1900<=_MSC_VER
#       define SZ_CPP11 1
#   endif
#endif

#ifdef __cplusplus
#   ifdef SZ_CPP11
#       define SZ_NULL nullptr
#   else
#       define SZ_NULL 0
#   endif
#else
#   define SZ_NULL ((void*)0)
#endif

typedef int8_t sz_s8;
typedef int16_t sz_s16;
typedef int32_t sz_s32;
typedef int64_t sz_s64;

typedef uint8_t sz_u8;
typedef uint16_t sz_u16;
typedef uint32_t sz_u32;
typedef uint64_t sz_u64;

typedef float sz_f32;
typedef double sz_f64;

typedef size_t sz_size_t;

typedef bool sz_bool;

//#define SZ_TRACE (1)

#ifdef _NDEBUG
#define SZ_ASSERT(exp)
#else
#define SZ_ASSERT(exp) assert(exp)
#endif

#ifdef __cplusplus
#define SZ_STATIC

static const sz_bool SZ_TRUE = true;
static const sz_bool SZ_FALSE = false;

static const sz_s32 SZ_CONTEXT_INFLATE = 0;
static const sz_s32 SZ_CONTEXT_DEFLATE = 1;

static const sz_u8 SZ_Z_COMPRESSION_TYPE = 8;
static const sz_u8 SZ_LZ77_WINDOWSIZE_MINUS_8 = 7;
static const sz_u8 SZ_Z_COMPRESSION_LEVEL_FARSTEST = 0;
static const sz_u8 SZ_Z_COMPRESSION_LEVEL_FARST = 1;
static const sz_u8 SZ_Z_COMPRESSION_LEVEL_DEFUALT = 2;
static const sz_u8 SZ_Z_COMPRESSION_LEVEL_SLOWEST = 3;

static const sz_s32 SZ_BLOCK_HEADER_SIZE = 3;
static const sz_u32 SZ_FLAG_LASTBLOCK = 0x01U;
static const sz_u32 SZ_FLAG_BLOCK_TYPE_MASK = 3;
static const sz_s32 SZ_BLOCK_TYPE_NOCOMPRESSION = 0;
static const sz_s32 SZ_BLOCK_TYPE_FIXED_HUFFMAN = 1;
static const sz_s32 SZ_BLOCK_TYPE_DYNAMIC_HUFFMAN = 2;

static const sz_s16 SZ_HUFFMAN_ENDCODE = 256;
static const sz_s32 SZ_LENGTH_CODES = 29;
static const sz_s32 SZ_DISTANCE_CODES = 30;

static const sz_s32 SZ_MAX_LENGTH = 258;
static const sz_s32 SZ_MAX_DISTANCE = 32767;
static const sz_s32 SZ_MAX_WINDOW_SIZE = SZ_MAX_DISTANCE + SZ_MAX_LENGTH;

static const sz_s32 SZ_MIN_INFLATE_OUTBUFF_SIZE = 258;

static const sz_s32 SZ_HCLENS = 15;
static const sz_s32 SZ_HCLEN_CODES = SZ_HCLENS+4;

static const sz_s32 SZ_HLENS = 286;
static const sz_s32 SZ_HDISTS = 30;
static const sz_s32 SZ_SYMBOL_LENGTH_SIZE = 19;

static const sz_s32 SZ_MAX_BITS_LITERAL_CODE = 15;
static const sz_s32 SZ_MAX_BITS_DISTANCE_CODE = 15;

static const sz_s16 SZ_TREE_LEFT = 0x01U;
static const sz_s16 SZ_TREE_RIGHT = 0x02U;

static const sz_s32 SZ_TREESIZE_LITERAL = 1<<(SZ_MAX_BITS_LITERAL_CODE+2);
static const sz_s32 SZ_TREESIZE_DISTANCE = 1<<(SZ_MAX_BITS_DISTANCE_CODE+2);

static const sz_s32 SZ_MINIMUM_OUT_BUFFER_SIZE = 16;
static const sz_s32 SZ_MAX_BLOCK_SIZE = 0xFFFF;

static const sz_s32 SZ_MAX_MATCH_LENGTH = 258;
static const sz_s32 SZ_MAX_CHAIN_SIZE = 16384;
static const sz_u32 SZ_CHAIN_MASK = SZ_MAX_CHAIN_SIZE-1;
static const sz_u16 SZ_CHAIN_EMPTY16 = 0xFFFFU;
static const sz_s32 SZ_MAX_LITERAL_BUFFER_SIZE = 4096-1;
static const sz_s32 SZ_HASH_LENGTH = 3;
static const sz_s32 SZ_LENGTH_CODE_BITS = 9;
static const sz_s32 SZ_LENGTH_MAX_EXTRA_BITS = 5;
static const sz_s32 SZ_DISTANCE_BITS = 5;
static const sz_s32 SZ_DISTANCE_MAX_EXTRA_BITS = 13;

static const sz_s32 SZ_MIN_DEFLATE_OUTBUFF_SIZE = 16;
static const sz_s32 SZ_REVERSE_PACKAGE_MERGE_BUFFER_SIZE = 3874;
static const sz_s32 SZ_MAX_SYMBOL_REPEAT = 138;

#define STATIC_CAST(TYPE, VALUE) static_cast<TYPE>(VALUE)
#define REINTERPRET_CAST(TYPE, VALUE) reinterpret_cast<TYPE>(VALUE)

#ifndef SZ_MALLOC
#define SZ_MALLOC(size) std::malloc(size)
#endif

#ifndef SZ_FREE
#define SZ_FREE(ptr) std::free(ptr)
#endif

#define SZ_EXTERN
#define SZ_PREFIX(name) name
#define SZ_STRUCT_BEGIN(name) struct name
#define SZ_STRUCT_END(name) ;
#define SZ_ENUM_BEGIN(name) enum name
#define SZ_ENUM_END(name) ;

template<class T>
inline void swap(T& x0, T& x1)
{
    T tmp = x0;
    x0 = x1;
    x1 = tmp;
}

#else
#define SZ_STATIC static

#define SZ_TRUE true
#define SZ_FALSE false

#define SZ_CONTEXT_INFLATE (0)
#define SZ_CONTEXT_DEFLATE (1)

#define SZ_Z_COMPRESSION_TYPE (8)
#define SZ_LZ77_WINDOWSIZE_MINUS_8 (7)
#define SZ_Z_COMPRESSION_LEVEL_FARSTEST (0)
#define SZ_Z_COMPRESSION_LEVEL_FARST (1)
#define SZ_Z_COMPRESSION_LEVEL_DEFUALT (2)
#define SZ_Z_COMPRESSION_LEVEL_SLOWEST (3)

#define SZ_BLOCK_HEADER_SIZE (3)
#define SZ_FLAG_LASTBLOCK (0x01U)
#define SZ_FLAG_BLOCK_TYPE_MASK (3)
#define SZ_BLOCK_TYPE_NOCOMPRESSION (0)
#define SZ_BLOCK_TYPE_FIXED_HUFFMAN (1)
#define SZ_BLOCK_TYPE_DYNAMIC_HUFFMAN (2)

#define SZ_HUFFMAN_ENDCODE (256)
#define SZ_LENGTH_CODES (29)
#define SZ_DISTANCE_CODES (30)

#define SZ_MAX_LENGTH (258)
#define SZ_MAX_DISTANCE 32767
#define SZ_MAX_WINDOW_SIZE (SZ_MAX_DISTANCE+SZ_MAX_LENGTH)

#define SZ_MIN_INFLATE_OUTBUFF_SIZE (258)

#define SZ_HCLENS (15)
#define SZ_HCLEN_CODES (19)

#define SZ_HLENS (286)
#define SZ_HDISTS (30)
#define SZ_SYMBOL_LENGTH_SIZE(19)

#define SZ_MAX_BITS_LITERAL_CODE (15)
#define SZ_MAX_BITS_DISTANCE_CODE (15)

#define SZ_TREE_LEFT (0x01U)
#define SZ_TREE_RIGHT (0x02U)

#define SZ_TREESIZE_LITERAL (1<<(SZ_MAX_BITS_LITERAL_CODE+2))
#define SZ_TREESIZE_DISTANCE (1<<(SZ_MAX_BITS_DISTANCE_CODE+2))

#define SZ_MINIMUM_OUT_BUFFER_SIZE (16)
#define SZ_MAX_BLOCK_SIZE (0xFFFF)

#define SZ_MAX_MATCH_LENGTH (258)
#define SZ_MAX_CHAIN_SIZE (16384)
#define SZ_CHAIN_MASK (SZ_MAX_CHAIN_SIZE-1)
#define SZ_CHAIN_EMPTY16 (0xFFFFU)
#define SZ_MAX_LITERAL_BUFFER_SIZE (4096-1)
#define SZ_HASH_LENGTH (3)

#define SZ_MIN_DEFLATE_OUTBUFF_SIZE (16)

#define STATIC_CAST(TYPE, VALUE) (TYPE)(VALUE)
#define REINTERPRET_CAST(TYPE, VALUE) (TYPE)(VALUE)

#ifndef SZ_MALLOC
#define SZ_MALLOC(size) malloc(size)
#endif

#ifndef SZ_FREE
#define SZ_FREE(ptr) free(ptr)
#endif

#define SZ_EXTERN extern
#define SZ_PREFIX(name) sz_ ## name
#define SZ_STRUCT_BEGIN(name) typedef struct name ## _t
#define SZ_STRUCT_END(name) name;
#define SZ_ENUM_BEGIN(name) typedef enum name ## _t
#define SZ_ENUM_END(name) name;
#endif


typedef void*(*FUNC_MALLOC)(sz_size_t size, void* user);
typedef void(*FUNC_FREE)(void* ptr, void* user);

/**
Result status of processing
*/
SZ_ENUM_BEGIN(SZ_Status)
{
    SZ_OK = 0,
    SZ_END = 1,
    SZ_PENDING = 2,
    SZ_ERROR_MEMORY = -1,
    SZ_ERROR_FORMAT = -2,
}
SZ_ENUM_END(SZ_Status)

/**
Internal states
*/
SZ_ENUM_BEGIN(SZ_State)
{
    SZ_State_Init =0,
    SZ_State_Block,
    SZ_State_NoComp,
    SZ_State_LZSS,
    SZ_State_Fixed,
    SZ_State_Dynamic,
    SZ_State_Dynamic_Size,
    SZ_State_Dynamic_Lengths,
    SZ_State_End,
}
SZ_ENUM_END(SZ_State)

/**
Internal states
*/
SZ_ENUM_BEGIN(SZ_Level)
{
    SZ_Level_NoCompression =0,
    SZ_Level_Fixed,
    SZ_Level_Dynamic,
}
SZ_ENUM_END(SZ_Level)

SZ_STRUCT_BEGIN(szZHeader)
{
    sz_u8 compressionMethodInfo_; ///< allowed with only 8
    sz_u8 flags_;
    sz_u32 presetDictionary_; ///< id for determining preset dictionary
    sz_u32 adler_; ///< adler32 check sum
}
SZ_STRUCT_END(szZHeader)


SZ_STRUCT_BEGIN(szBitStream)
{
    sz_s32 bit_;
    sz_s32 current_;
    sz_s32 size_;
    const sz_u8* src_;
}
SZ_STRUCT_END(szBitStream)

SZ_STRUCT_BEGIN(szWriteStream)
{
    sz_s16 bit_;
    sz_s16 pendingBitsLE_;
    sz_u16 pendingLE_;
}
SZ_STRUCT_END(szWriteStream)

SZ_STRUCT_BEGIN(szCode)
{
    sz_s16 literal_;
    sz_s16 length_;
    sz_s32 distance_;
}
SZ_STRUCT_END(szCode)

SZ_STRUCT_BEGIN(szLengthCode)
{
    sz_s16 length_;
    sz_s16 code_;
}
SZ_STRUCT_END(szLengthCode)

SZ_STRUCT_BEGIN(szCodeTree)
{
    sz_s16 children_;
    sz_s16 literal_;
}
SZ_STRUCT_END(szCodeTree)

struct szContextInflate;
struct szContextDeflate;

/**
A context for inflating
*/
SZ_STRUCT_BEGIN(szContext)
{
    sz_s32 status_;
    sz_s32 totalOut_;
    sz_s32 thisTimeOut_;
    sz_s32 availOut_;
    sz_u8* nextOut_;

    void* internal_;
}
SZ_STRUCT_END(szContext)

union Hash
{
    sz_u32 value_;
    sz_u8 bytes_[4];
};

SZ_STRUCT_BEGIN(szLZSSHEntry)
{
    sz_u16 start_;
    sz_u16 history_;
    sz_u16 prev_;
    sz_u16 next_;
    Hash hash_;
    sz_s32 position_;
}
SZ_STRUCT_END(szLZSSHEntry)

SZ_STRUCT_BEGIN(szLZSSHistory)
{
    sz_u16 empty_;
    sz_u16 count_;
    szLZSSHEntry entries_[SZ_MAX_CHAIN_SIZE];
}
SZ_STRUCT_END(szLZSSHistory)

SZ_STRUCT_BEGIN(szLZSSLiteral)
{
    sz_u32 literal_;
}
SZ_STRUCT_END(szLZSSLiteral)

SZ_STRUCT_BEGIN(szFreqCode)
{
    sz_u32 frequency_;
    sz_u16 code_;
    sz_u16 huffCode_;
}
SZ_STRUCT_END(szFreqCode)

inline sz_u16 getLengthCode(szLZSSLiteral literal)
{
    return literal.literal_ & ((0x01U<<SZ_LENGTH_CODE_BITS)-1);
}

inline szLZSSLiteral setLengthCode(szLZSSLiteral literal, sz_u16 code)
{
    literal.literal_ |= code;
    return literal;
}

inline sz_u16 getLengthExtra(szLZSSLiteral literal)
{
    return (literal.literal_>>SZ_LENGTH_CODE_BITS) & ((0x01U<<SZ_LENGTH_MAX_EXTRA_BITS)-1);
}

inline szLZSSLiteral setLengthExtra(szLZSSLiteral literal, sz_u16 extra)
{
    literal.literal_ |= extra << SZ_LENGTH_CODE_BITS;
    return literal;
}

inline sz_u16 getDistanceCode(szLZSSLiteral literal)
{
    return (literal.literal_>>(SZ_LENGTH_CODE_BITS+SZ_LENGTH_MAX_EXTRA_BITS)) & ((0x01U<<SZ_DISTANCE_BITS)-1);
}

inline szLZSSLiteral setDistanceCode(szLZSSLiteral literal, sz_u16 code)
{
    literal.literal_ |= code << (SZ_LENGTH_CODE_BITS+SZ_LENGTH_MAX_EXTRA_BITS);
    return literal;
}

inline sz_u16 getDistanceExtra(szLZSSLiteral literal)
{
    return literal.literal_>>(SZ_LENGTH_CODE_BITS+SZ_LENGTH_MAX_EXTRA_BITS+SZ_DISTANCE_BITS) & ((0x01U<<SZ_DISTANCE_MAX_EXTRA_BITS)-1);
}

inline szLZSSLiteral setDistanceExtra(szLZSSLiteral literal,  sz_u16 extra)
{
    literal.literal_ |= extra << (SZ_LENGTH_CODE_BITS+SZ_LENGTH_MAX_EXTRA_BITS+SZ_DISTANCE_BITS);
    return literal;
}

void heapsort(sz_s32 n, szFreqCode* v);
void takePackage(sz_s32 i, sz_s32 size, sz_u16* length, sz_u32* type[], sz_s32* currentPosition);
void getLengths(sz_s32 size, sz_u16* lengths, szFreqCode* frequencies, sz_s32 limit, sz_u32* valueBuffer, sz_u32* typeBuffer);
void calcHuffCodes(sz_s32 size, szFreqCode* codes, const sz_u16* lengths);

//--- Inflate
//--------------------------------------------------------------------------------------------------------------
/**
@brief Initialize context.
@param context ... 
@param size ... size of input data "src"
@param src ... source
@param pMalloc ... user's malloc
@param pFree ... user's free
@param user ... user data for malloc/free functions
@warn Both pMalloc and pFree should be provided together.
*/
#ifdef __cplusplus
SZ_Status SZ_PREFIX(initInflate) (szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc=SZ_NULL, FUNC_FREE pFree=SZ_NULL, void* user=SZ_NULL);
#else
SZ_EXTERN SZ_Status SZ_PREFIX(initInflate) (szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user);
#endif

/**
@brief Create context only without initializing. The function `resetInflate' should be called.
@param context ... 
@param pMalloc ... user's malloc
@param pFree ... user's free
@param user ... user data for malloc/free functions
@warn Both pMalloc and pFree should be provided together.
*/
#ifdef __cplusplus
SZ_Status SZ_PREFIX(createInflate) (szContext* context, FUNC_MALLOC pMalloc=SZ_NULL, FUNC_FREE pFree=SZ_NULL, void* user=SZ_NULL);
#else
SZ_EXTERN SZ_Status SZ_PREFIX(createInflate) (szContext* context, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user);
#endif

/**
@brief Terminate context with deallocating used resources.
@param context ...
*/
SZ_EXTERN void SZ_PREFIX(termInflate) (szContext* context);
/**
@brief Reset internal states of context.
*/
SZ_EXTERN void SZ_PREFIX(resetInflate) (szContext* context, sz_s32 size, const sz_u8* src);

/**
@brief Process inflating.
@param context ... 
@warning Size of output buffer "nextOut_", that is a number "availOut", needs above SZ_MIN_INFLATE_OUTBUFF_SIZE(258) in bytes.
*/
SZ_EXTERN SZ_Status SZ_PREFIX(inflate) (szContext* context);

//--- Deflate
//--------------------------------------------------------------------------------------------------------------
/**
@brief Initialize context.
@param context ... 
@param size ... size of input data "src"
@param src ... source
@param pMalloc ... user's malloc
@param pFree ... user's free
@param user ... user data for malloc/free functions
@warn Both pMalloc and pFree should be provided together.
*/
#ifdef __cplusplus
SZ_Status SZ_PREFIX(initDeflate) (szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc=SZ_NULL, FUNC_FREE pFree=SZ_NULL, void* user=SZ_NULL, SZ_Level level = SZ_Level_Fixed);
#else
SZ_EXTERN SZ_Status SZ_PREFIX(initDeflate) (szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user, SZ_Level level);
#endif

/**
@brief Create context only without initializing. The function `resetDeflate' should be called.
@param context ... 
@param pMalloc ... user's malloc
@param pFree ... user's free
@param user ... user data for malloc/free functions
@warn Both pMalloc and pFree should be provided together.
*/
#ifdef __cplusplus
SZ_Status SZ_PREFIX(createDeflate) (szContext* context, FUNC_MALLOC pMalloc=SZ_NULL, FUNC_FREE pFree=SZ_NULL, void* user=SZ_NULL);
#else
SZ_EXTERN SZ_Status SZ_PREFIX(createInflate) (szContext* context, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user);
#endif

/**
@brief Terminate context with deallocating used resources.
@param context ...
*/
SZ_EXTERN void SZ_PREFIX(termInflate) (szContext* context);
/**
@brief Reset internal states of context.
*/
#ifdef __cplusplus
void SZ_PREFIX(resetDeflate) (szContext* context, sz_s32 size, const sz_u8* src, SZ_Level level = SZ_Level_Fixed);
#else
SZ_EXTERN void SZ_PREFIX(resetDeflate) (szContext* context, sz_s32 size, const sz_u8* src, SZ_Level level);
#endif

/**
@brief Process deflating.
@param context ... 
@warning Size of output buffer "nextOut_", that is a number "availOut", needs above SZ_MIN_INFLATE_OUTBUFF_SIZE(258) in bytes.
*/
SZ_EXTERN SZ_Status SZ_PREFIX(deflate) (szContext* context);

#ifdef __cplusplus
}
#endif
#endif //INC_SZLIB_H_

//--------------------------------------------------------------------------------------------------------------
//---
//--- Implementation
//---
//--------------------------------------------------------------------------------------------------------------
#ifdef SZLIB_IMPLEMENTATION
#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include <stdio.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef __cplusplus
namespace szlib
{
#endif

    SZ_STRUCT_BEGIN(szContextInflate)
    {
        sz_s32 type_;
        FUNC_MALLOC malloc_;
        FUNC_FREE free_;
        void* user_;

        SZ_State state_;
        szZHeader zheader_;
        szBitStream bitStream_;
        sz_s16 lastBlockHeader_;
        sz_s32 lastRequestLength_;
        szCode lastCode_;
        sz_s32 windowPosition_;
        sz_u8 buffer_[SZ_MAX_WINDOW_SIZE];
        sz_u8* window_;
        sz_u8* data_;

        sz_s16 lenHlits_;
        sz_s16 lenHdists_;
        szLengthCode hlitsDists_[SZ_HLENS+SZ_HDISTS];

        szLengthCode* hlits_;
        szLengthCode* hdists_;

        szCodeTree treeLiteral_[SZ_TREESIZE_LITERAL];
        szCodeTree treeDistance_[SZ_TREESIZE_DISTANCE];
    }
    SZ_STRUCT_END(szContextInflate)

    SZ_STRUCT_BEGIN(szContextDeflate)
    {
        sz_s32 type_;
        FUNC_MALLOC malloc_;
        FUNC_FREE free_;
        void* user_;

        SZ_Level level_;
        SZ_State state_;
        sz_s32 availIn_;
        sz_s32 currentIn_;
        sz_s32 sizeIn_;
        const sz_u8* nextIn_;
        szWriteStream stream_;
        szLZSSHistory history_;
        sz_s32 inLiteralSize_;
        sz_s32 outLiteralSize_;
        szLZSSLiteral literals_[SZ_MAX_LITERAL_BUFFER_SIZE+1];

        szFreqCode freqCodes_[SZ_HLENS];
        szFreqCode freqDists_[SZ_HDISTS];
        szFreqCode freqCodeDists_[SZ_SYMBOL_LENGTH_SIZE];
        sz_u32 valueBuffer_[SZ_REVERSE_PACKAGE_MERGE_BUFFER_SIZE];
        sz_u32 typeBuffer_[SZ_REVERSE_PACKAGE_MERGE_BUFFER_SIZE];
        sz_u16 symbols_[SZ_HLENS+SZ_HDISTS];
        sz_u16 treeLengths_[SZ_SYMBOL_LENGTH_SIZE];
        sz_u16 hlit_;
        sz_u16 hdist_;
        sz_u16 hclen_;
        sz_u16 outSymbols_;
        sz_u16 currentSymbol_;

        sz_u32 adler_;
    }
    SZ_STRUCT_END(szContextDeflate)

    void heapsort(sz_s32 n, szFreqCode* v)
    {
        SZ_ASSERT(0<=n);
        SZ_ASSERT(SZ_NULL != v);
        --v; //the first index is 1
        sz_s32 i, j;
        szFreqCode x;
        for(sz_s32 k=n>>1; k>=1; --k){
            i=k;
            x = v[k];
            while((j=i<<1)<=n){
                if(j<n && (v[j+1].frequency_<v[j].frequency_)){
                    ++j;
                }

                if(x.frequency_<=v[j].frequency_){
                    break;
                }
                v[i] = v[j];
                i = j;
            }
            v[i] = x;
        }

        while(n>1){
            x = v[n];
            v[n] = v[1];
            --n;
            i = 1;
            while((j=i<<1)<=n){
                if(j<n && (v[j+1].frequency_<v[j].frequency_)){
                    ++j;
                }

                if(x.frequency_<=v[j].frequency_){
                    break;
                }
                v[i] = v[j];
                i = j;
            }
            v[i] = x;
        }
    }

    void takePackage(sz_s32 i, sz_s32 size, sz_u16* length, sz_u32* type[], sz_s32* currentPosition)
    {
        sz_s32 x = type[i][currentPosition[i]];
        if (size == x){
            takePackage(i+1, size, length, type, currentPosition);
            takePackage(i+1, size, length, type, currentPosition);
        }
        else{
            --length[x];
        }
        ++currentPosition[i];
    }

    void getLengths(sz_s32 size, sz_u16* lengths, szFreqCode* frequencies, sz_s32 limit, sz_u32* valueBuffer, sz_u32* typeBuffer)
    {
        SZ_ASSERT(0<=size && size<=0xFFFF);
        SZ_ASSERT(SZ_NULL != lengths);
        SZ_ASSERT(SZ_NULL != frequencies);

        static const sz_s32 MAX_LIMIT = SZ_MAX_BITS_LITERAL_CODE;

        sz_u32 minimumCost[MAX_LIMIT];
        sz_u8 flag[MAX_LIMIT];

        sz_u32* value[MAX_LIMIT];
        sz_u32* type[MAX_LIMIT];

        sz_s32 currentPosition[MAX_LIMIT];

        heapsort(size, frequencies);

        sz_s32 excess = ((1U << limit) - size);
        const sz_s32 half = (1U << (limit-1));
        minimumCost[limit-1] = size;

        for(sz_s32 i=0; i<limit; ++i){
            if(excess<half){
                flag[i] = 0;
            } else{
                flag[i] = 1;
                excess -= half;
            }
            excess <<= 1;
            if((i+2)<=limit){
                minimumCost[limit-2-i] = (minimumCost[limit-i-1] >> 1) + size;
            }
        }

        minimumCost[0] = flag[0];
        value[0] = valueBuffer;
        type[0] = typeBuffer;
        valueBuffer += minimumCost[0];
        typeBuffer += minimumCost[0];
        for(sz_s32 i=1; i<limit; ++i){
            sz_u32 cost = 2*minimumCost[i-1]+flag[i];
            if(cost<minimumCost[i]){
                minimumCost[i] = cost;
            }
            value[i] = valueBuffer;
            type[i] = typeBuffer;
            valueBuffer += minimumCost[i];
            typeBuffer += minimumCost[i];
        }

        for(sz_s32 i=0; i<size; ++i){
            lengths[i] = STATIC_CAST(sz_u16, limit);
        }
        for(sz_u32 i=0; i<minimumCost[limit-1]; ++i){
            value[limit-1][i] = frequencies[i].frequency_;
            type[limit-1][i] = i;
        }

        for(sz_s32 i=0; i<limit; ++i){
            currentPosition[i] = 0;
        }
        if(1 == flag[limit-1]){
            --lengths[0];
            ++currentPosition[limit-1];
        }

        for(sz_s32 j=limit-2; 0<=j; --j){
            sz_s32 i = 0;
            sz_u32 weight = 0;
            sz_s32 next = currentPosition[j+1];

            for(sz_u32 k=0; k<minimumCost[j]; ++k){
                weight = value[j+1][next] + value[j+1][next+1];

                if(frequencies[i].frequency_ < weight){
                    value[j][k] = weight;
                    type[j][k] = size;
                    next += 2;
                } else {
                    value[j][k] = frequencies[i].frequency_;
                    type[j][k] = i;
                    ++i;
                }
            }

            currentPosition[j] = 0;
            if(1==flag[j]){
                takePackage(j, size, lengths, type, currentPosition);
            }
        }
    }

    void calcHuffCodes(sz_s32 size, szFreqCode* codes, const sz_u16* lengths)
    {
        sz_u16 count[SZ_MAX_BITS_LITERAL_CODE+1];
        sz_u16 startCode[SZ_MAX_BITS_LITERAL_CODE+1];
        memset(count, 0, sizeof(sz_u16)*(SZ_MAX_BITS_LITERAL_CODE+1));

        //
        for(sz_s32 i=0; i<size; ++i){
            count[lengths[i]] += 1;
        }
        //
        startCode[0] = 0;
        sz_u16 code = 0;
        for(sz_s32 i=1; i<=SZ_MAX_BITS_LITERAL_CODE; ++i){
            startCode[i] = code;
            code += count[i];
            code <<= 1;
        }

        //
        for(sz_s32 i=0; i<size; ++i){
            code = startCode[lengths[i]];
            startCode[lengths[i]] += 1;
            codes[i].huffCode_ = 0;
            for(sz_s32 j=0; j<lengths[i]; ++j){
                codes[i].huffCode_ = (codes[i].huffCode_<<1) | (code & 0x01U);
                code >>= 1;
            }
        }
    }

#ifdef __cplusplus
namespace
{
#endif

/**
Default malloc
*/
#ifdef __cplusplus
SZ_STATIC void* sz_malloc(sz_size_t size, void* /*user*/)
#else
SZ_STATIC void* sz_malloc(sz_size_t size, void* user)
#endif
{
    return SZ_MALLOC(size);
}

/**
Default free
*/
#ifdef __cplusplus
SZ_STATIC void sz_free(void* ptr, void* /*user*/)
#else
SZ_STATIC void sz_free(void* ptr, void* user)
#endif
{
    SZ_FREE(ptr);
}

/**
Extra bits for length code
*/
static const sz_s8 LengthExtraBits[SZ_LENGTH_CODES] =
{
    0,0,0,0,0,0,0,0,1,1,
    1,1,2,2,2,2,3,3,3,3,
    4,4,4,4,5,5,5,5,0,
};

/**
Minimum length for length code
*/
static const sz_s16 LengthBase[SZ_LENGTH_CODES] =
{
    3,4,5,6,7,8,9,10,11,13,
    15,17,19,23,27,31,35,43,51,59,
    67,83,99,115,131,163,195,227,258,
};

/**
Extra bits for distance code
*/
static const sz_s8 DistanceExtraBits[SZ_DISTANCE_CODES] =
{
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,
};

/**
Minimum length for distance code
*/
static const sz_s16 DistanceBase[SZ_DISTANCE_CODES] =
{
    1,2,3,4,5,7,9,13,17,25,
    33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,
};

/**
Order of huffman code length
*/
static const sz_s8 HCLENS_Order[SZ_HCLEN_CODES] =
{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15,
};

SZ_STATIC inline sz_s32 maximum(sz_s32 x0, sz_s32 x1)
{
    return x0<x1? x1 : x0;
}

SZ_STATIC inline sz_s32 minimum(sz_s32 x0, sz_s32 x1)
{
    return x0<x1? x0 : x1;
}

SZ_STATIC sz_u32 adler32(sz_size_t size, const sz_u8* data) { 
    static const sz_u32 MOD_ADLER = 65521;
    sz_u32 a = 1;
    sz_u32 b = 0;
    while(0<size){
        sz_size_t t = (5550<size)? 5550 : size;
        size -= t;
        do{
            a += *data;
            b += a;
            ++data;
        }while(--t);
        a %= MOD_ADLER;
        b %= MOD_ADLER;
    }
    return a | (b<<16);
}

//--- Inflate
//--------------------------------------------------------------------------------------------------------------
SZ_STATIC inline sz_u8 getCompressinMethod(szZHeader* header)
{
    return header->compressionMethodInfo_ & 0xFU;
}

SZ_STATIC inline sz_u8 getCompressinInfo(szZHeader* header)
{
    return header->compressionMethodInfo_ >> 4;
}

SZ_STATIC inline sz_s32 getWindowSize(szZHeader* header)
{
    sz_u8 n = header->compressionMethodInfo_ >> 4;
    return (n<=7)? 1<<(n+8) : 0;
}

SZ_STATIC inline sz_u8 getCheck(szZHeader* header)
{
    return header->flags_ & 0xFU;
}

SZ_STATIC inline sz_bool hasPresetDictionary(szZHeader* header)
{
    return 0 != ((header->flags_ >> 5) & 0x01U);
}

SZ_STATIC inline sz_u8 getCompressionLevel(szZHeader* header)
{
    return header->flags_ >> 6;
}

SZ_STATIC inline void initBitStream(szBitStream* stream, sz_s32 size, const sz_u8* src)
{
    SZ_ASSERT(SZ_NULL != stream);
    SZ_ASSERT(SZ_NULL != src);
    stream->bit_ = 0;
    stream->current_ = 0;
    stream->size_ = size;
    stream->src_ = src;
}

SZ_STATIC inline sz_bool remainEnoughBits(szBitStream* stream, sz_s32 bits)
{
    sz_s32 bytes = minimum(stream->size_-stream->current_, 1024);
    return bits <= ((bytes<<3) + 8-stream->bit_);
}

SZ_STATIC inline void store(sz_s32* current, sz_s32* bit, szBitStream* stream)
{
    *current = stream->current_;
    *bit = stream->bit_;
}

SZ_STATIC inline void restore(szBitStream* stream, sz_s32 current, sz_s32 bit)
{
    stream->current_ = current;
    stream->bit_ = bit;
}

/**
@return actually read size in bytes
@param dst ... destination buffer
@param bytes ... size in bytes that try to read
@param stream
*/
SZ_STATIC sz_s32 readBytes4ZeroBitOffset(sz_u32* dst, sz_s32 bytes, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);
    SZ_ASSERT(0 == (bytes&0x03U));
    SZ_ASSERT(stream->bit_<=0);

    sz_s32 offset = 0;

    while(0<bytes && (stream->current_+4)<=stream->size_){
        const sz_u32* src = REINTERPRET_CAST(const sz_u32*, &stream->src_[stream->current_]);
        dst[offset] = src[0];
        ++offset;
        stream->current_ += 4;
        bytes -= 4;
    }
    return offset<<2;
}

/**
@return actually read size in bytes
@param dst ... destination buffer
@param bytes ... size in bytes that try to read
@param stream
*/
SZ_STATIC sz_s32 readBytesZeroBitOffset(sz_u8* dst, sz_s32 bytes, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);
    SZ_ASSERT(stream->bit_<=0);

    sz_s32 bytes4 = (bytes>>2) << 2;
    sz_s32 total = bytes;

    sz_s32 read = readBytes4ZeroBitOffset(REINTERPRET_CAST(sz_u32*, dst), bytes4, stream);
    bytes -= read;
    dst += read;

    while(0<bytes){
        if(stream->size_<=stream->current_){
            break;
        }
        dst[0] = stream->src_[stream->current_];
        ++dst;
        ++stream->current_;
        --bytes;
    }
    return total-bytes;
}

/**
@return actually read size in bits
@param bits ... size in bits that try to read
@param stream
*/
SZ_STATIC sz_s16 readBitsLE(sz_s32 bits, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);

    sz_s16 code = 0;
    sz_s32 count = 0;
    while(0<bits){
        sz_s16 b = stream->src_[stream->current_] >> stream->bit_;
        ++stream->bit_;
        --bits;
        code |= (b&0x01U)<<count;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++stream->current_;
            if(stream->size_<=stream->current_){
                break;
            }
        }
        ++count;
    }
    return (bits<=0)? code : -1;
}

/**
@return actually read size in bits
@param bits ... size in bits that try to read
@param stream
*/
SZ_STATIC sz_s16 readBitsBE(sz_s32 bits, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);

    sz_s16 code = 0;
    while(0<bits){
        sz_u8 b = stream->src_[stream->current_];
        code = (code<<1) | ((b>>stream->bit_) & 0x01U);
        ++stream->bit_;
        --bits;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++stream->current_;
            if(stream->size_<=stream->current_){
                break;
            }
        }
    }
    return (bits<=0)? code : -1;
}

SZ_STATIC sz_s16 readFixedLiteral(szBitStream* stream)
{
    sz_s16 code = readBitsBE(7, stream);
    if(0<=code && code<=23){//0b0010111=23
        return code+256;
    }

    if(stream->size_<=stream->current_){
        return -1;
    }
    code = (code<<1) | readBitsBE(1, stream);
    if(48<=code && code<=199){//0b00110000=48, 0b10111111=191, 0b11000000=192, 0b11000111=199
        return (code<=191)? code-48 : code-192+280;
    }else if(code<0){
        return -1;
    }

    if(stream->size_<=stream->current_){
        return -1;
    }
    code = (code<<1) | readBitsBE(1, stream);
    if(400<=code && code<=511){//0b110010000=400, 0b111111111=511
        return code-400+144;
    }
    return -1;
}

SZ_STATIC sz_bool readFixedCode(szCode* code, szBitStream* stream)
{
    code->distance_ = 0;
    code->literal_ = readFixedLiteral(stream);
    if(code->literal_<=SZ_HUFFMAN_ENDCODE){
        code->length_ = (code->literal_<SZ_HUFFMAN_ENDCODE)? 1 : 0;
        return (0<=code->literal_);
    }
    sz_s32 index = code->literal_-257;
    sz_s16 extraBits;
    if(8<=index && index<(SZ_LENGTH_CODES-1)){
        extraBits = readBitsLE(LengthExtraBits[index], stream);
        if(extraBits<0){
            return SZ_FALSE;
        }
        code->length_ = LengthBase[index] + extraBits;
    }else{
        code->length_ = LengthBase[index];
    }
    index = readBitsBE(5, stream);
    if(index<0 || SZ_DISTANCE_CODES<=index){
        return SZ_FALSE;
    }

    extraBits = readBitsLE(DistanceExtraBits[index], stream);
    if(extraBits<0){
        return SZ_FALSE;
    }
    code->distance_ = DistanceBase[index] + extraBits;
    return SZ_TRUE;
}

SZ_STATIC inline sz_bool proceedNextBoundary(szBitStream* stream)
{
    if(0<stream->bit_){
        stream->bit_ = 0;
        ++stream->current_;
    }
    return stream->current_<stream->size_;
}

SZ_STATIC SZ_Status readZHeader(szZHeader* header, szBitStream* stream)
{
    SZ_ASSERT(SZ_NULL != header);
    SZ_ASSERT(SZ_NULL != stream);
    if(readBytesZeroBitOffset(&header->compressionMethodInfo_, 1, stream)<1){
        return SZ_ERROR_FORMAT;
    }
    if(readBytesZeroBitOffset(&header->flags_, 1, stream)<1){
        return SZ_ERROR_FORMAT;
    }
    if(hasPresetDictionary(header)){
        if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &header->presetDictionary_), 4, stream)<4){
            return SZ_ERROR_FORMAT;
        }
    }else{
        header->presetDictionary_ = 0;
    }
    header->adler_ = 0;
    return SZ_OK;
}

SZ_STATIC void inflateFlush(szContext* context)
{
    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    szCode* code = &internal->lastCode_;
    sz_u8* dst = context->nextOut_;
    if(code->literal_<SZ_HUFFMAN_ENDCODE){
        dst[context->thisTimeOut_] = STATIC_CAST(sz_u8, code->literal_);
        //push
        internal->window_[internal->windowPosition_] = STATIC_CAST(sz_u8, code->literal_);
        ++internal->windowPosition_;
        if(SZ_MAX_WINDOW_SIZE<internal->windowPosition_){
            internal->windowPosition_ = 0;
        }

        ++context->thisTimeOut_;
    }else{
        SZ_ASSERT(1<=code->distance_);
        SZ_ASSERT(3<=code->length_);
        sz_s32 distance = code->distance_;
        sz_s32 length = code->length_;
        sz_u8* window = internal->window_;
        sz_s32 windowPosition = internal->windowPosition_;
        sz_s32 prev = (distance<=windowPosition)
            ? windowPosition - distance
            : windowPosition - distance + SZ_MAX_WINDOW_SIZE+1;
        for(sz_s32 i=0; i<length; ++i){
            window[windowPosition] = dst[context->thisTimeOut_] = window[prev];
            ++windowPosition;
            if(SZ_MAX_WINDOW_SIZE<windowPosition){
                windowPosition = 0;
            }
            ++prev;
            if(SZ_MAX_WINDOW_SIZE<prev){
                prev = 0;
            }
            ++context->thisTimeOut_;
        }
        internal->windowPosition_ = windowPosition;
    }
    code->length_ = 0;
}

SZ_STATIC SZ_Status inflateFixedHuffman(szContext* context)
{
    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    szBitStream* stream = &internal->bitStream_;
    szCode* code = &internal->lastCode_;
    if(0<code->length_){
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        inflateFlush(context);
    }

    for(;;){
        if(!readFixedCode(code, stream)){
            return SZ_ERROR_FORMAT;
        }
        if(code->literal_ == SZ_HUFFMAN_ENDCODE){
            return SZ_OK;
        }
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        inflateFlush(context);
    }
}

#ifdef SZ_TRACE
const char* bits(sz_s16 x)
{
    static char buffer[32];
    sz_s32 length=0;
    for(sz_s32 i=0; i<16; ++i){
        buffer[15-i] = (x & 0x01U) == 0 ? '0' : '1';
        x>>=1;
        ++length;
        if(0==x){
            break;
        }
    }
    for(sz_s32 i=0; i<(16-length); ++i){
        buffer[i] = ' ';
    }
    buffer[16] = '\0';
    return buffer;
}
#endif

SZ_STATIC sz_s16 findLiteral(sz_s32 treeSize, szCodeTree* tree, szBitStream* stream)
{
    --tree;
    sz_s32 node = 1;
#ifdef SZ_TRACE
    sz_s32 code = 0;
    sz_s32 readBits = 0;
#endif
    for(;;){
        sz_u8 b = (stream->src_[stream->current_] >> stream->bit_) & 0x01U;
        ++stream->bit_;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++stream->current_;
            if(stream->size_<=stream->current_){
                return -1;
            }
        }
#ifdef SZ_TRACE
        ++readBits;
#endif
        node = (node<<1) + b;
        if(treeSize<=node){
            return -1;
        }
#ifdef SZ_TRACE
        code = (code<<1) + b;
#endif
        if(tree[node].children_<=0){
            break;
        }
    }
#ifdef SZ_TRACE
    printf("code %d, literal %d, read %d bits\n", code, tree[node].literal_, readBits);
#endif
    return tree[node].literal_;
}

SZ_STATIC inline void clearCodeTree(sz_s32 size, szCodeTree* tree)
{
    memset(tree, 0, sizeof(szCodeTree)*size);
}

SZ_STATIC sz_bool createHuffmanTree(sz_s16 length, sz_s32 treeSize, szCodeTree* tree, szLengthCode* lengthCode)
{
    clearCodeTree(treeSize, tree);
    --tree;
    for(sz_s16 i=0; i<length; ++i){
        if(lengthCode[i].length_<=0){
            continue;
        }
        sz_s32 code = lengthCode[i].code_;
        sz_s32 node = 1;
        sz_s32 bits = lengthCode[i].length_;
        for(sz_s32 j=0; j<bits; ++j){
            sz_u32 bit = (code>>(bits-j-1)) & 0x01U;
            tree[node].children_ |= (0x01U<<bit);
            node = (node<<1) + bit;
            if(treeSize<=node){
                return SZ_FALSE;
            }
        }
        tree[node].literal_ = i;
    }
    return SZ_TRUE;
}

SZ_STATIC void decodeCanonicalHuffman(sz_s32 treeSize, szCodeTree* tree, szLengthCode* hclens, sz_s16 length, sz_s32 maxBits, sz_s16* bl_count, sz_s16* next_code)
{
    //restore code length codes
    for(sz_s32 i = 0; i<=maxBits; ++i){
        bl_count[i] = 0;
    }
    for(sz_s16 i = 0; i<length; ++i){
        SZ_ASSERT(hclens[i].length_<=maxBits);
        ++bl_count[hclens[i].length_];
    }
    sz_s16 code = 0;
    bl_count[0] = 0;
    for(sz_s32 bits = 1; bits<=maxBits; ++bits){
        code = (code + bl_count[bits-1])<<1;
        next_code[bits] = code;
    }
    for(sz_s16 i = 0; i<length; ++i){
        sz_s16 len = hclens[i].length_;
        if(0 != len){
            hclens[i].code_ = next_code[len];
            hclens[i].length_ = len;
            ++next_code[len];
        } else{
            hclens[i].code_ = -1;
        }
    }
#ifdef SZ_TRACE
    for(int i = 0; i<length; ++i){
        if(hclens[i].length_<=0){
            continue;
        }
        printf("[%d] len %d code %s (%d)\n", i, hclens[i].length_, bits(hclens[i].code_), hclens[i].code_);
    }
#endif

    if(!createHuffmanTree(length, treeSize, tree, hclens)){
        return;
    }

#ifdef SZ_TRACE
    for(int i=0; i<length; ++i){
        if(hclens[i].length_<=0){
            continue;
        }
        sz_s32 readBits = 0;
        int literal = findLiteral(&readBits, treeSize, tree, hclens[i].length_, hclens[i].code_);
        printf("[%d] len %d code %s literal %d\n", i, hclens[i].length_, bits(hclens[i].code_), literal);
    }
#endif
}

SZ_STATIC sz_bool loadDynamicHuffmanCodes(szContext* context)
{
    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    szBitStream* stream = &internal->bitStream_;

    if(!remainEnoughBits(stream, 14)){
        return SZ_FALSE;
    }
    internal->lenHlits_ = readBitsLE(5, stream) + 257;
    internal->lenHdists_ = readBitsLE(5, stream) + 1;
    sz_s32 hclen = readBitsLE(4, stream)+4;

    if(SZ_HLENS<internal->lenHlits_){
        return SZ_FALSE;
    }
    if(SZ_HDISTS<internal->lenHdists_){
        return SZ_FALSE;
    }
    if(SZ_HCLEN_CODES<hclen){
        return SZ_FALSE;
    }
    if(!remainEnoughBits(stream, hclen*3)){
        return SZ_FALSE;
    }

    //read hclens
    sz_s32 count;
    {
        szLengthCode hclens[SZ_HCLEN_CODES];
        for(count = 0; count<hclen; ++count){
            hclens[HCLENS_Order[count]].length_ = readBitsLE(3, stream);
        }
        for(;count<SZ_HCLEN_CODES; ++count){
            hclens[HCLENS_Order[count]].length_ = 0;
        }

        //decode length codes
        static const sz_s32 MAX_BITS = 7;
        sz_s16 bl_count[MAX_BITS+1];
        sz_s16 next_code[MAX_BITS+1];
        decodeCanonicalHuffman(SZ_TREESIZE_LITERAL, internal->treeLiteral_, hclens, SZ_HCLEN_CODES, MAX_BITS, bl_count, next_code);
    }

    internal->hlits_ = internal->hlitsDists_;
    internal->hdists_ = internal->hlitsDists_ + internal->lenHlits_;
    sz_s32 totalNeeds = internal->lenHlits_ + internal->lenHdists_;
    szLengthCode* hlens = internal->hlitsDists_;
    count = 0;
    while(count<totalNeeds){
        sz_s16 literal = findLiteral(SZ_TREESIZE_LITERAL, internal->treeLiteral_, stream);
        if(literal<0){
            return SZ_FALSE;
        }
        SZ_ASSERT(literal<=18);

        switch(literal){
        case 16:
        {
            sz_s32 repeat = readBitsLE(2, stream);
            if(repeat<0 || count<=0){
                return SZ_FALSE;
            }
            repeat += 3;
            for(sz_s32 i = 0; i<repeat; ++i){
                hlens[count] = hlens[count-1];
                ++count;
            }
        }
        break;
        case 17:
        {
            sz_s32 repeat = readBitsLE(3, stream);
            if(repeat<0){
                return SZ_FALSE;
            }
            repeat += 3;
            for(sz_s32 i = 0; i<repeat; ++i){
                hlens[count].length_ = 0;
                hlens[count].code_ = 0;
                ++count;
            }
        }
        break;
        case 18:
        {
            sz_s32 repeat = readBitsLE(7, stream);
            if(repeat<0){
                return SZ_FALSE;
            }
            repeat += 11;
            for(sz_s32 i = 0; i<repeat; ++i){
                hlens[count].length_ = 0;
                hlens[count].code_ = 0;
                ++count;
            }
        }
        break;
        default:
            hlens[count].length_ = literal;
            hlens[count].code_ = 0;
            ++count;
            break;
        }
    }

    {//decode length codes
        sz_s16 bl_count[SZ_MAX_BITS_LITERAL_CODE+1];
        sz_s16 next_code[SZ_MAX_BITS_LITERAL_CODE+1];
        decodeCanonicalHuffman(SZ_TREESIZE_LITERAL, internal->treeLiteral_, internal->hlits_, internal->lenHlits_, SZ_MAX_BITS_LITERAL_CODE, bl_count, next_code);
        decodeCanonicalHuffman(SZ_TREESIZE_DISTANCE, internal->treeDistance_, internal->hdists_, internal->lenHdists_, SZ_MAX_BITS_DISTANCE_CODE, bl_count, next_code);
    }
    return totalNeeds<=count;
}

SZ_STATIC sz_bool readDynamicCode(szCode* code, szCodeTree* literalTree, szCodeTree* distanceTree, szBitStream* stream)
{
    code->literal_ = findLiteral(SZ_TREESIZE_LITERAL, literalTree, stream);
    if(285<code->literal_){
        return SZ_FALSE;
    }

    if(code->literal_<=SZ_HUFFMAN_ENDCODE){
        code->length_ = (code->literal_<SZ_HUFFMAN_ENDCODE)? 1 : 0;
        return (0<=code->literal_);
    }
    sz_s32 index = code->literal_-257;
    sz_s16 extraBits;
    if(8<=index && index<(SZ_LENGTH_CODES-1)){
        extraBits = readBitsLE(LengthExtraBits[index], stream);
        if(extraBits<0){
            return SZ_FALSE;
        }
        code->length_ = LengthBase[index] + extraBits;
    }else{
        code->length_ = LengthBase[index];
    }

    code->distance_ = findLiteral(SZ_TREESIZE_DISTANCE, distanceTree, stream);
    if(code->distance_<0 || SZ_DISTANCE_CODES<=code->distance_){
        return SZ_FALSE;
    }

    index = code->distance_;
    extraBits = readBitsLE(DistanceExtraBits[index], stream);
    if(extraBits<0){
        return SZ_FALSE;
    }
    code->distance_ = DistanceBase[index] + extraBits;
    return SZ_TRUE;
}

SZ_STATIC SZ_Status inflateDynamicHuffman(szContext* context)
{
    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    szBitStream* stream = &internal->bitStream_;
    szCode* code = &internal->lastCode_;
    if(0<code->length_){
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        inflateFlush(context);
    }else{
        if(!loadDynamicHuffmanCodes(context)){
            return SZ_ERROR_FORMAT;
        }
    }

    for(;;){
        if(!readDynamicCode(code, internal->treeLiteral_, internal->treeDistance_, stream)){
            return SZ_ERROR_FORMAT;
        }
        if(code->literal_ == SZ_HUFFMAN_ENDCODE){
            return SZ_OK;
        }
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        inflateFlush(context);
    }
}

//--- Deflate
//--------------------------------------------------------------------------------------------------------------

sz_bool flushWriteStreamLE(szContext* context);
inline sz_bool writeByte(szContext* context, sz_u8 byte);
sz_bool writeBitsLE(szContext* context, sz_s16 size, sz_u16 bits);
sz_bool writeBitsBE(szContext* context, sz_s16 size, sz_u16 bits);
void writeFixedLiteral(szContext* context, szLZSSLiteral literal);
void writeDistance(szContext* context, szLZSSLiteral literal);
void generateCanonicalHuffmanLengths(szContext* context);
sz_u16 generateTreeSymbols(sz_u16* symbols, szFreqCode* freqs, sz_s32 hlit, const sz_u16* lenLengths, sz_s32 hdist, const sz_u16* distLengths);

SZ_STATIC sz_bool flushWriteStreamLE(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    szWriteStream* stream = &internal->stream_;
    SZ_ASSERT(0<=stream->bit_ && stream->bit_<8);

    if(0<stream->pendingBitsLE_){
        if(SZ_FALSE == writeBitsLE(context, stream->pendingBitsLE_, stream->pendingLE_)){
            return SZ_FALSE;
        }
        stream->pendingBitsLE_ = 0;
    }
    if(0<stream->bit_){
        if(context->availOut_<=context->thisTimeOut_){
            return SZ_FALSE;
        }
        stream->bit_ = 0;
        ++context->thisTimeOut_;
    }
    return SZ_TRUE;
}

SZ_STATIC sz_bool flushPendingBitsLE(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    szWriteStream* stream = &internal->stream_;
    SZ_ASSERT(0<=stream->bit_ && stream->bit_<8);

    if(0<stream->pendingBitsLE_){
        if(SZ_FALSE == writeBitsLE(context, stream->pendingBitsLE_, stream->pendingLE_)){
            return SZ_FALSE;
        }
        stream->pendingBitsLE_ = 0;
    }
    return SZ_TRUE;
}

#if 0
SZ_STATIC sz_bool flushPendingBitsBE(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    szWriteStream* stream = &internal->stream_;
    SZ_ASSERT(0<=stream->bit_ && stream->bit_<8);

    if(0<stream->pendingBitsBE_){
        if(SZ_FALSE == writeBitsBE(context, stream->pendingBitsBE_, stream->pendingBE_)){
            return SZ_FALSE;
        }
        stream->pendingBitsBE_ = 0;
    }
    return SZ_TRUE;
}
#endif

inline sz_bool writeByte(szContext* context, sz_u8 byte)
{
    return writeBitsLE(context, 8, byte);
}

SZ_STATIC sz_bool writeBitsLE(szContext* context, sz_s16 size, sz_u16 bits)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(0<=size);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    szWriteStream* stream = &internal->stream_;

    while(0<size){
        if(context->availOut_<=context->thisTimeOut_){
            stream->pendingBitsLE_ = size;
            stream->pendingLE_ = bits;
            return SZ_FALSE;
        }

        sz_s16 b = 8-stream->bit_;
        if(size<b){
            b = size;
        }
        context->nextOut_[context->thisTimeOut_] |= STATIC_CAST(sz_u8, bits<<stream->bit_);
        stream->bit_ += b;
        bits >>= b;
        size -= b;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++context->thisTimeOut_;
        }
    }
    return SZ_TRUE;
}

SZ_STATIC sz_bool writeBitsBE(szContext* context, sz_s16 size, sz_u16 bits)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(0<=size && size<=16);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    szWriteStream* stream = &internal->stream_;

    while(0<size){
        if(context->availOut_<=context->thisTimeOut_){
            return SZ_FALSE;
        }

        sz_u8 bit = (bits>>(size-1)) & 0x01U;
        context->nextOut_[context->thisTimeOut_] |= STATIC_CAST(sz_u8, bit<<stream->bit_);
        stream->bit_ += 1;
        --size;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++context->thisTimeOut_;
        }
    }
    return SZ_TRUE;
}

SZ_STATIC sz_s32 writeBytes(szContext* context, sz_s32 size, const sz_u8* bytes)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(REINTERPRET_CAST(szContextDeflate*, context->internal_)->stream_.bit_<=0);

    for(sz_s32 i=0; i<size; ++i){
        if(context->availOut_<=context->thisTimeOut_){
            return i;
        }
        context->nextOut_[context->thisTimeOut_] = bytes[i];
        ++context->thisTimeOut_;
    }
    return size;
}

SZ_STATIC inline sz_u32 hash_FNV1(const sz_u8* v, sz_s32 count)
{
    sz_u32 hash = 2166136261U;
    for(sz_s32 i=0; i<count; ++i){
        hash *= 16777619U;
        hash ^= v[i];
    }
    return hash;
}

SZ_STATIC void calcLengthCode(szLZSSLiteral* literal, sz_s32 length)
{
    SZ_ASSERT(SZ_HASH_LENGTH<=length && length<=SZ_MAX_LENGTH);
    sz_u16 code = 0;
    for(sz_s32 i=SZ_LENGTH_CODES-1; 0<=i; --i){
        if(LengthBase[i]<=length){
            code = STATIC_CAST(sz_u16, i);
            break;
        }
    }
    *literal = setLengthCode(*literal, STATIC_CAST(sz_u16, 0x101U + code));
    *literal = setLengthExtra(*literal, STATIC_CAST(sz_u16, length - LengthBase[code]));
}

SZ_STATIC void calcDistanceCode(szLZSSLiteral* literal, sz_u16 distance)
{
    SZ_ASSERT(1<=distance && distance<=SZ_MAX_DISTANCE);
    sz_u16 code = 0;
    for(sz_s32 i=SZ_DISTANCE_CODES-1; 0<=i; --i){
        if(DistanceBase[i]<=distance){
            code = STATIC_CAST(sz_u16, i);
            break;
        }
    }
    *literal = setDistanceCode(*literal, code);
    *literal = setDistanceExtra(*literal, STATIC_CAST(sz_u16, distance - DistanceBase[code]));
}

SZ_STATIC void initLZSSHistory(szLZSSHistory* history)
{
    history->empty_ = 0;
    history->count_ = 0;
    memset(history->entries_, 0xFFU, sizeof(szLZSSHEntry)*SZ_MAX_CHAIN_SIZE);
    for(sz_s32 i=1; i<SZ_MAX_CHAIN_SIZE; ++i){
        history->entries_[i-1].next_ = STATIC_CAST(sz_u16, i);
    }
}

SZ_STATIC sz_bool removeLZSSHistory(szLZSSHistory* history)
{
    sz_u16 end = history->count_<=0? SZ_MAX_CHAIN_SIZE-1 : history->count_-1;
    sz_u16 count = history->count_;
    while(end != count){
        if(SZ_CHAIN_EMPTY16 == history->entries_[count].history_){
            count = (count+1) & SZ_CHAIN_MASK;
            continue;
        }

        sz_u16 position = history->entries_[count].history_;
        szLZSSHEntry* curr = history->entries_ + position;
        szLZSSHEntry* prev = history->entries_ + curr->prev_;
        szLZSSHEntry* next = history->entries_ + curr->next_;

        if(prev->start_ == position){
            if(SZ_CHAIN_EMPTY16 == curr->next_){
                prev->start_ = SZ_CHAIN_EMPTY16;
            }else{
                prev->start_ = curr->next_;
                next->prev_ = curr->prev_;
            }
        }else{
            if(SZ_CHAIN_EMPTY16 != curr->next_){
                position = curr->next_;
                curr = history->entries_ + position;
                prev = history->entries_ + curr->prev_;
                next = history->entries_ + curr->next_;
            }
            prev->next_ = SZ_CHAIN_EMPTY16;
        }
        curr->history_ = SZ_CHAIN_EMPTY16;
        curr->prev_ = SZ_CHAIN_EMPTY16;
        curr->position_ = -1;

        //Link to empty list
        curr->next_ = history->empty_;
        history->empty_ = position;
        return SZ_TRUE;
    }
    return SZ_FALSE;
}

SZ_STATIC const sz_u8* calcLZSSEnd(const sz_u8* start, const sz_u8* end)
{
    sz_s32 length = STATIC_CAST(sz_s32, end-start);
    if(length<SZ_HASH_LENGTH){
        return SZ_NULL;
    }else if(SZ_MAX_LENGTH<length){
        length = SZ_MAX_LENGTH;
    }
    return start+length;
}

SZ_STATIC sz_bool addLZSSHistory(szLZSSHistory* history, Hash hash, const sz_u8* start, const sz_u8* src)
{
    if(SZ_CHAIN_EMPTY16 == history->empty_){
        if(!removeLZSSHistory(history)){
            return SZ_FALSE;
        }
    }
    sz_s32 position = STATIC_CAST(sz_s32, start-src);

    SZ_ASSERT(SZ_CHAIN_EMPTY16 != history->empty_);
    sz_u16 newPos = history->empty_;
    szLZSSHEntry* newEntry = history->entries_ + newPos;
    history->empty_ = newEntry->next_;

    sz_u16 startPos = STATIC_CAST(sz_u16, hash.value_ & SZ_CHAIN_MASK);
    szLZSSHEntry* curr = history->entries_ + startPos;
    if(SZ_CHAIN_EMPTY16 == curr->start_){
        curr->start_ = newPos;
        newEntry->prev_ = startPos;
    }else{
        sz_u16 prevPos = startPos;
        while(SZ_CHAIN_EMPTY16 != curr->next_){
            prevPos = curr->prev_;
            curr = history->entries_ + curr->next_;
        }
        curr->next_ = newPos;
        newEntry->prev_ = prevPos;
    }
    newEntry->next_ = SZ_CHAIN_EMPTY16;
    newEntry->hash_ = hash;
    newEntry->position_ = position;
    sz_u16 hpos = history->count_;
    history->count_ = (history->count_+1) & SZ_CHAIN_MASK;
    history->entries_[hpos].history_ = newPos;
    return SZ_TRUE;
}

SZ_STATIC sz_s32 findLongestMatch(szLZSSLiteral* result, Hash hash, szLZSSHistory* history, const sz_u8* start, const sz_u8* end, const sz_u8* src)
{
    result->literal_ = 0;

    sz_s32 length = STATIC_CAST(sz_s32, end-start);
    sz_s32 offset = STATIC_CAST(sz_s32, start-src);

    sz_u16 position = history->entries_[ hash.value_ & SZ_CHAIN_MASK ].start_;
    sz_s32 maxLength = 0;
    while(position != SZ_CHAIN_EMPTY16){
        szLZSSHEntry* current = history->entries_ + position;
        position = current->next_;

        if(current->hash_.value_ != hash.value_){
            continue;
        }
        sz_s32 distance = offset - current->position_;
        SZ_ASSERT(0<=distance);
        if(SZ_MAX_DISTANCE<distance){
            continue;
        }
        const sz_u8* s = src + current->position_;
        sz_s32 len = minimum(distance, length);
        sz_s32 l;
        for(l=0; l<len; ++l){
            if(s[l] != start[l]){
                break;
            }
        }

        if(SZ_HASH_LENGTH<=l && maxLength<l){
            calcDistanceCode(result, STATIC_CAST(sz_u16, distance));
            calcLengthCode(result, l);
            maxLength = l;
        }
    }
    return maxLength;
}

SZ_STATIC sz_bool writeFixedCode(szContext* context, sz_u16 code)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(code<=287);

    //Fixed Huffman codes
    //Lit Value Bits Codes
    //--------- ---- -----
    //  0 - 143    8  00110000 through 10111111
    //144 - 255    9 110010000 through 111111111
    //256 - 279    7   0000000 through 0010111
    //280 - 287    8  11000000 through 11000111
    if(code<=143){
        sz_u16 bits = code + 48; //00110000
        return writeBitsBE(context, 8, bits);
    }else if(code<=255){ //144-255
        sz_u16 bits = code - 144 + 400; //110010000
        return writeBitsBE(context, 9, bits);
    }else if(code<=279){ //256-279
        sz_u16 bits = code - 256;
        return writeBitsBE(context, 7, bits);
    }else{
        sz_u16 bits = code - 280 + 192; //11000000
        return writeBitsBE(context, 8, bits);
    }
}

SZ_STATIC void writeFixedLiteral(szContext* context, szLZSSLiteral literal)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(4<=(context->availOut_-context->thisTimeOut_));

    sz_u16 lengthCode = getLengthCode(literal);
    sz_u16 deistanceCode = getDistanceCode(literal);
    if(deistanceCode<=0){ //code itself
        writeFixedCode(context, lengthCode);

    }else{
        sz_u16 extra = getLengthExtra(literal);
        sz_u16 extraBits = LengthExtraBits[lengthCode-0x101U];
        writeFixedCode(context, lengthCode);
        if(0<extraBits){
            writeBitsLE(context, extraBits, extra);
        }
        writeDistance(context, literal);
    }
}

SZ_STATIC void writeDistance(szContext* context, szLZSSLiteral literal)
{
    sz_u16 deistanceCode = getDistanceCode(literal);
    SZ_ASSERT(1<=deistanceCode && deistanceCode<=SZ_MAX_DISTANCE);
    writeBitsBE(context, 5, deistanceCode);
    sz_u16 extra = getDistanceExtra(literal);
    sz_s16 extraBits = DistanceExtraBits[deistanceCode];
    if(0<extraBits){
        writeBitsLE(context, STATIC_CAST(sz_s16, extraBits), STATIC_CAST(sz_u16, extra));
    }
}

SZ_STATIC void generateCanonicalHuffmanLengths(szContext* context)
{
    sz_u16 lenLengths[SZ_HLENS];
    sz_u16 distLengths[SZ_HDISTS];

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    getLengths(SZ_HLENS, lenLengths, internal->freqCodes_, 15, internal->valueBuffer_, internal->typeBuffer_);
    calcHuffCodes(SZ_HLENS, internal->freqCodes_, lenLengths);

    getLengths(SZ_HDISTS, distLengths, internal->freqDists_, 7, internal->valueBuffer_, internal->typeBuffer_);
    calcHuffCodes(SZ_HDISTS, internal->freqDists_, distLengths);

    sz_s32 hlit;
    for(hlit=SZ_HLENS; (257<hlit)&&(0==lenLengths[hlit-1]); --hlit);
    sz_s32 hdist;
    for(hdist=SZ_HDISTS; (1<hdist)&&(0==distLengths[hdist-1]); --hdist);

    internal->outSymbols_ = generateTreeSymbols(internal->symbols_, internal->freqCodeDists_, hlit, lenLengths, hdist, distLengths);

    sz_u16 tmpLengths[SZ_SYMBOL_LENGTH_SIZE];
    getLengths(SZ_SYMBOL_LENGTH_SIZE, tmpLengths, internal->freqCodeDists_, 7, internal->valueBuffer_, internal->typeBuffer_);
    for(sz_s32 i=0; i<SZ_SYMBOL_LENGTH_SIZE; ++i){
        internal->treeLengths_[i] = tmpLengths[HCLENS_Order[i]];
    }
    sz_s32 hclen;
    for(hclen=SZ_SYMBOL_LENGTH_SIZE; 4<hclen && 0==internal->treeLengths_[hclen-1]; --hclen);
    calcHuffCodes(SZ_SYMBOL_LENGTH_SIZE, internal->freqCodeDists_, internal->treeLengths_);

    internal->hlit_ = STATIC_CAST(sz_u16, hlit);
    internal->hdist_ = STATIC_CAST(sz_u16, hdist);
    internal->hclen_ = STATIC_CAST(sz_u16, hclen);
    internal->currentSymbol_ = 0;
}

SZ_STATIC sz_u16 generateTreeSymbols(sz_u16* symbols, szFreqCode* freqs, sz_s32 hlit, const sz_u16* lenLengths, sz_s32 hdist, const sz_u16* distLengths)
{
    sz_s32 srcSize = hlit + hdist;
    sz_u16 src[SZ_HLENS+SZ_HDISTS];
    for(sz_s32 i=0; i<19; ++i){
        freqs[i].frequency_ = 0;
    }

    sz_s32 count=0;
    for(sz_s32 i=0; i<hlit; ++i,++count){
        src[count] = lenLengths[i];
    }
    for(sz_s32 i=0; i<hdist; ++i,++count){
        src[count] = distLengths[i];
    }

    sz_u16 countResult=0;
    count = 0;
    for(sz_s32 i=0, j=0, l=srcSize; i<l; i+=j){
        for(j=1; (i+j)<l && src[i+j]==src[i]; ++j);
        sz_s32 runLength = j;
        if(0==src[i]){
            if(runLength<3){
                while(0<runLength--){
                    symbols[countResult++] = 0;
                    ++freqs[0].frequency_;
                }
            } else{
                while(0<runLength){
                    sz_s32 repeat = (runLength<=SZ_MAX_SYMBOL_REPEAT)? runLength : SZ_MAX_SYMBOL_REPEAT;
                    if((runLength-3)<repeat && repeat<runLength){
                        repeat = runLength-3;
                    }
                    if(repeat<=10){
                        symbols[countResult++] = 17;
                        symbols[countResult++] = STATIC_CAST(sz_u16, repeat - 3);
                        ++freqs[17].frequency_;
                    }else{
                        symbols[countResult++] = 18;
                        symbols[countResult++] = STATIC_CAST(sz_u16, repeat - 11);
                        ++freqs[18].frequency_;
                    }
                    runLength -= repeat;
                }
            } //if(runLength<3){
        } else{ //if(0==src[i])
            symbols[countResult++] = src[i];
            ++freqs[src[i]].frequency_;
            --runLength;
            if(runLength<3){
                while(0<runLength--){
                    symbols[countResult++] = src[i];
                    ++freqs[src[i]].frequency_;
                }
            }else{
                while(0<runLength){
                    sz_s32 repeat = (runLength<6)? runLength : 6;
                    if((runLength-3)<repeat && repeat<runLength){
                        repeat = runLength-3;
                    }
                    symbols[countResult++] = 16;
                    symbols[countResult++] = STATIC_CAST(sz_u16, repeat - 3);
                    ++freqs[16].frequency_;
                    runLength -= repeat;
                }
            }
        } //if(0==src[i])
    } //for(sz_s32 i=0
    return countResult;
}

#ifdef __cplusplus
} //namespace{
#endif

void SZ_PREFIX(resetInflate)(szContext* context, sz_s32 size, const sz_u8* src)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(0<=size);
    SZ_ASSERT(SZ_NULL != src);
    SZ_ASSERT(SZ_NULL != context->internal_);

    context->totalOut_ = 0;
    context->availOut_ = 0;
    context->nextOut_ = SZ_NULL;

    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    SZ_ASSERT(SZ_NULL != internal->malloc_);
    SZ_ASSERT(SZ_NULL != internal->free_);
    SZ_ASSERT(SZ_CONTEXT_INFLATE == internal->type_);

    internal->state_ = SZ_State_Init;
    internal->lastBlockHeader_ = 0;
    internal->lastRequestLength_ = 0;
    internal->lastCode_.literal_ = 0;
    internal->lastCode_.length_ = 0;
    internal->lastCode_.distance_ = 0;
    internal->windowPosition_ = 0;
    memset(internal->buffer_, 0, SZ_MAX_WINDOW_SIZE);

    initBitStream(&internal->bitStream_, size, src);
}

SZ_Status SZ_PREFIX(initInflate)(szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user)
{
    SZ_Status status = SZ_PREFIX(createInflate)(context, pMalloc, pFree, user);
    if(SZ_OK != status){
        return status;
    }
    SZ_PREFIX(resetInflate)(context, size, src);
    return SZ_OK;
}

SZ_Status SZ_PREFIX(createInflate)(szContext* context, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user)
{
    SZ_ASSERT(SZ_NULL != context);
    pMalloc = (SZ_NULL == pMalloc)? sz_malloc : pMalloc;
    pFree = (SZ_NULL == pFree)? sz_free : pFree;

    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, pMalloc(sizeof(szContextInflate), user));
    if(SZ_NULL == internal){
        return SZ_ERROR_MEMORY;
    }
    internal->type_ = SZ_CONTEXT_INFLATE;

    context->internal_ = internal;
    internal->malloc_ = pMalloc;
    internal->free_ = pFree;
    internal->user_ = user;
    internal->window_ = internal->buffer_;

    return SZ_OK;
}

void SZ_PREFIX(termInflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);

    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    SZ_ASSERT(SZ_CONTEXT_INFLATE == internal->type_);

    internal->free_(internal, internal->user_);
    memset(context, 0, sizeof(szContext));
}

SZ_Status SZ_PREFIX(inflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);
    SZ_ASSERT(SZ_MIN_INFLATE_OUTBUFF_SIZE<=context->availOut_);

    szContextInflate* internal = REINTERPRET_CAST(szContextInflate*, context->internal_);
    SZ_ASSERT(SZ_CONTEXT_INFLATE == internal->type_);

    szBitStream* stream = &internal->bitStream_;

    context->thisTimeOut_ = 0;
    for(;;){
        switch(internal->state_){
        //--- SZ_State_Init
        //------------------------------------------------------------------
        case SZ_State_Init:
        {
            szZHeader zheader;
            SZ_Status result = readZHeader(&zheader, stream);
            switch(result){
            case SZ_OK:
                internal->state_ = SZ_State_Block;
                break;
            default:
                goto SZ_INFLATE_ERROR;
            }
        }
        continue;

        //--- SZ_State_Block
        //------------------------------------------------------------------
        case SZ_State_Block:
        {
            if(stream->size_<=stream->current_){
                return SZ_OK;
            }

            internal->lastBlockHeader_ = readBitsLE(SZ_BLOCK_HEADER_SIZE, stream);
            if(internal->lastBlockHeader_<0){
                goto SZ_INFLATE_ERROR;
            }
            sz_s32 blockType = (internal->lastBlockHeader_>>1) & SZ_FLAG_BLOCK_TYPE_MASK;
            if(SZ_BLOCK_TYPE_NOCOMPRESSION == blockType){
                proceedNextBoundary(stream);
                sz_u16 len;
                sz_u16 nlen;
                if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &len), 2, stream)<2){
                    goto SZ_INFLATE_ERROR;
                }
                if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &nlen), 2, stream)<2){
                    goto SZ_INFLATE_ERROR;
                }
                nlen = ~nlen;
                if(len != nlen){ // nlen is len's complement
                    goto SZ_INFLATE_ERROR;
                }
                if(stream->size_<(stream->current_+len)){
                    goto SZ_INFLATE_ERROR;
                }
                internal->lastRequestLength_ = len;
                internal->state_ = SZ_State_NoComp;
                continue;

            }else if(SZ_BLOCK_TYPE_FIXED_HUFFMAN == blockType){
                internal->state_ = SZ_State_Fixed;
                continue;
            }else if(SZ_BLOCK_TYPE_DYNAMIC_HUFFMAN == blockType){
                internal->state_ = SZ_State_Dynamic;
                continue;
            }else{
                goto SZ_INFLATE_ERROR;
            }
        }
        break;

        //--- SZ_State_NoComp
        //------------------------------------------------------------------
        case SZ_State_NoComp:
        {
            sz_s32 remain = context->availOut_ - context->thisTimeOut_;
            sz_s32 readLen = internal->lastRequestLength_<remain? internal->lastRequestLength_ : remain;
            if(0<readLen){
                if(readBytesZeroBitOffset(context->nextOut_+context->thisTimeOut_, readLen, stream)<readLen){
                    goto SZ_INFLATE_ERROR;
                }
            }
            internal->lastRequestLength_ -= readLen;
            context->thisTimeOut_ += readLen;
            context->totalOut_ += readLen;
            if(0<internal->lastRequestLength_){
                return SZ_OK;
            }
            internal->state_ = SZ_State_Block;
        }
        break;

        //--- SZ_State_Fixed
        //------------------------------------------------------------------
        case SZ_State_Fixed:
        {
            switch(inflateFixedHuffman(context)){
            case SZ_OK:
                internal->state_ = SZ_State_Block;
                context->totalOut_ += context->thisTimeOut_;
                break;
            case SZ_PENDING:
                context->totalOut_ += context->thisTimeOut_;
                return SZ_OK;
            default:
                goto SZ_INFLATE_ERROR;
            }
        }
        break;
        //--- SZ_State_Dynamic
        //------------------------------------------------------------------
        case SZ_State_Dynamic:
        {
            switch(inflateDynamicHuffman(context)){
            case SZ_OK:
                internal->state_ = SZ_State_Block;
                context->totalOut_ += context->thisTimeOut_;
                break;
            case SZ_PENDING:
                context->totalOut_ += context->thisTimeOut_;
                return SZ_OK;
            default:
                goto SZ_INFLATE_ERROR;
            }
        }
        break;
        //--- SZ_INFLATE_ERROR
        //------------------------------------------------------------------
        default:
            goto SZ_INFLATE_ERROR;
        }//switch(internal->state_)

        if(internal->lastBlockHeader_&SZ_FLAG_LASTBLOCK){ //last block bit is set
            return SZ_END;
        }else if(stream->size_<=stream->current_){
            break;
        }
    }//for(;;)

SZ_INFLATE_ERROR:
    return SZ_ERROR_FORMAT;
}

#if 0
sz_s16 SZ_PREFIX(findLiteral)(sz_s32* readBits, sz_s32 treeSize, szCodeTree* tree, sz_s16 len, sz_s16 inCode)
{
    --tree;
    sz_s32 node = 1;
    *readBits = 0;
    for(sz_s32 i=0; i<len; ++i){
        sz_u8 b = (inCode>>(len-i-1)) & 0x01U;
        ++(*readBits);
        node = (node<<1) + b;
        if(treeSize<=node){
            return -1;
        }
        if(tree[node].children_<=0){
            break;
        }
    }
    return tree[node].literal_;
}
#endif


//--- Deflate
//--------------------------------------------------------------------------------------------------------------
void SZ_PREFIX(resetDeflate)(szContext* context, sz_s32 size, const sz_u8* src, SZ_Level level)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(0<=size);
    SZ_ASSERT(SZ_NULL != src);
    SZ_ASSERT(SZ_NULL != context->internal_);

    context->totalOut_ = 0;
    context->availOut_ = 0;
    context->nextOut_ = SZ_NULL;

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    SZ_ASSERT(SZ_NULL != internal->malloc_);
    SZ_ASSERT(SZ_NULL != internal->free_);
    SZ_ASSERT(SZ_CONTEXT_DEFLATE == internal->type_);

    {
        FUNC_MALLOC mallocFunc = internal->malloc_;
        FUNC_FREE freeFunc = internal->free_;
        void* user = internal->user_;

        memset(internal, 0, sizeof(szContextDeflate));
        internal->type_ = SZ_CONTEXT_DEFLATE;
        internal->malloc_ = mallocFunc;
        internal->free_ = freeFunc;
        internal->user_ = user;
    }

    internal->level_ = level;
    internal->state_ = SZ_State_Init;
    internal->availIn_ = size;
    internal->currentIn_ = 0;
    internal->nextIn_ = src;
    initLZSSHistory(&internal->history_);
    internal->adler_ = adler32(size, src);
}

SZ_Status SZ_PREFIX(initDeflate)(szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user, SZ_Level level)
{
    SZ_Status status = SZ_PREFIX(createDeflate)(context, pMalloc, pFree, user);
    if(SZ_OK != status){
        return status;
    }
    SZ_PREFIX(resetDeflate)(context, size, src, level);
    return SZ_OK;
}

SZ_Status SZ_PREFIX(createDeflate)(szContext* context, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user)
{
    SZ_ASSERT(SZ_NULL != context);
    pMalloc = (SZ_NULL == pMalloc)? sz_malloc : pMalloc;
    pFree = (SZ_NULL == pFree)? sz_free : pFree;

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, pMalloc(sizeof(szContextDeflate), user));
    if(SZ_NULL == internal){
        return SZ_ERROR_MEMORY;
    }
    internal->type_ = SZ_CONTEXT_DEFLATE;

    context->internal_ = internal;
    internal->malloc_ = pMalloc;
    internal->free_ = pFree;
    internal->user_ = user;

    return SZ_OK;
}

void SZ_PREFIX(termDeflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    SZ_ASSERT(SZ_CONTEXT_DEFLATE == internal->type_);

    internal->free_(internal, internal->user_);
    memset(context, 0, sizeof(szContext));
}

SZ_Status SZ_PREFIX(deflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);
    SZ_ASSERT(SZ_MIN_DEFLATE_OUTBUFF_SIZE<=context->availOut_);

    szContextDeflate* internal = REINTERPRET_CAST(szContextDeflate*, context->internal_);
    SZ_ASSERT(SZ_CONTEXT_DEFLATE == internal->type_);

    context->thisTimeOut_ = 0;
    memset(context->nextOut_, 0, context->availOut_);
    for(;;){
        switch(internal->state_){
        //--- SZ_State_Init
        //------------------------------------------------------------------
        case SZ_State_Init:
        {
            context->nextOut_[context->thisTimeOut_++] = SZ_Z_COMPRESSION_TYPE | (SZ_LZ77_WINDOWSIZE_MINUS_8<<4); //Compression type and LZ77's window size
            switch(internal->level_)
            {
            case SZ_Level_NoCompression:
                context->nextOut_[context->thisTimeOut_++] = 0x01U;
                break;
            default:
                context->nextOut_[context->thisTimeOut_++] = 0x1AU | (SZ_Z_COMPRESSION_LEVEL_SLOWEST<<6); //Check flag and compression level
                break;
            }
            internal->state_ = SZ_State_Block;
        }
        continue;

        //--- SZ_State_Block
        //------------------------------------------------------------------
        case SZ_State_Block:
        {
            switch(internal->level_)
            {
            case SZ_Level_NoCompression:
            {
                sz_s32 size = internal->availIn_ - internal->currentIn_;
                while(SZ_MAX_BLOCK_SIZE<size){
                    size -= SZ_MAX_BLOCK_SIZE;
                }

                sz_u8 endBlock = 0;
                if(internal->availIn_<=(internal->currentIn_+size)){
                    endBlock = 1;
                }
                sz_u8 compression = SZ_BLOCK_TYPE_NOCOMPRESSION<<1;
                writeBitsLE(context, 3, endBlock|compression);
                flushWriteStreamLE(context);

                sz_u16 len = STATIC_CAST(sz_u16, size);
                sz_u16 nlen = ~len;
                writeBytes(context, 2, REINTERPRET_CAST(const sz_u8*, &len));
                writeBytes(context, 2, REINTERPRET_CAST(const sz_u8*, &nlen));
                internal->sizeIn_ = size;
                internal->state_ = SZ_State_NoComp;
            }
                break;
            default:
                internal->sizeIn_ = internal->availIn_ - internal->currentIn_;
                internal->state_ = SZ_State_LZSS;

                if(SZ_Level_Fixed == internal->level_){
                    writeBitsLE(context, 3, 1|(SZ_BLOCK_TYPE_FIXED_HUFFMAN<<1));
                }
                break;
            }; //switch(internal->level_)
        }
        continue;

        //--- SZ_State_NoComp
        //------------------------------------------------------------------
        case SZ_State_NoComp:
        {
            sz_s32 size = writeBytes(context, internal->sizeIn_, internal->nextIn_ + internal->currentIn_);
            internal->currentIn_ += size;
            internal->sizeIn_ -= size;
            if(internal->availIn_<=internal->currentIn_){
                internal->state_ = SZ_State_End;
            }else{
                if(internal->sizeIn_<=0){
                    internal->state_ = SZ_State_Block;
                }
                context->totalOut_ += context->thisTimeOut_;
                return SZ_PENDING;
            }
        }
        continue;
        //--- SZ_State_LZSS
        //------------------------------------------------------------------
        case SZ_State_LZSS:
        {
            for(sz_s32 i=0; i<SZ_HLENS; ++i){
                internal->freqCodes_[i].code_ = STATIC_CAST(sz_u16, i);
                internal->freqCodes_[i].frequency_ = 0;
            }
            for(sz_s32 i=0; i<SZ_HDISTS; ++i){
                internal->freqDists_[i].code_ = STATIC_CAST(sz_u16, i);
                internal->freqDists_[i].frequency_ = 0;
            }
            const sz_u8* scur = internal->nextIn_ + internal->currentIn_;
            const sz_u8* send = internal->nextIn_ + internal->availIn_;
            sz_s32 dstSize = 0;
            szLZSSLiteral* dcur = internal->literals_;

            while(scur<send){
                const sz_u8* s = scur;
                const sz_u8* e = calcLZSSEnd(scur, send);
                Hash hash;
                hash.value_ = (SZ_NULL != e)? hash_FNV1(scur, SZ_HASH_LENGTH) : 0;
                szLZSSLiteral result = {0};
                sz_s32 length = (SZ_NULL != e)
                    ? findLongestMatch(&result, hash, &internal->history_, scur, e, internal->nextIn_)
                    : 0;

                if(0<length){
                    scur += length;
                    internal->currentIn_ += length;
                    //Increment frequence of distance
                    internal->freqDists_[getDistanceCode(result)].frequency_ += 1;

                }else{
                    result = setLengthCode(result, *scur);
                    ++scur;
                    ++internal->currentIn_;
                }

                if(SZ_NULL != e){
                    addLZSSHistory(&internal->history_, hash, s, internal->nextIn_);
                }
                //Increment frequency of literal length
                internal->freqCodes_[getLengthCode(result)].frequency_ += 1;

                *dcur = result;
                ++dcur;
                if(SZ_MAX_LITERAL_BUFFER_SIZE<=++dstSize){
                    break;
                }
            }

            if(0<dstSize){
                if(internal->availIn_<=internal->currentIn_){
                    ++dstSize;
                    dcur->literal_ = 0;
                    *dcur = setLengthCode(*dcur, 0x100U);
                }
                internal->inLiteralSize_ = dstSize;
                internal->outLiteralSize_ = 0;

                if(SZ_Level_Dynamic == internal->level_){
                    sz_u8 endBlock = (internal->currentIn_<internal->availIn_)? 0 : 1;
                    sz_u8 compression = SZ_BLOCK_TYPE_DYNAMIC_HUFFMAN<<1;
                    writeBitsLE(context, 3, endBlock|compression);
                    internal->state_ = SZ_State_Dynamic;
                }else{
                    internal->state_ = SZ_State_Fixed;
                }

            }else{
                internal->state_ = SZ_State_End;
            }
        }
        continue;
        //--- SZ_State_Fixed
        //------------------------------------------------------------------
        case SZ_State_Fixed:
        {
            flushPendingBitsLE(context);
            while(internal->outLiteralSize_<internal->inLiteralSize_){
                sz_s32 availOut = context->availOut_-context->thisTimeOut_;
                if(availOut<4){
                    if(0<internal->stream_.bit_){
                        SZ_ASSERT(0<availOut);
                        internal->stream_.pendingBitsLE_ = internal->stream_.bit_;
                        internal->stream_.pendingLE_ = context->nextOut_[context->thisTimeOut_];
                        internal->stream_.bit_ = 0;
                    }
                    return SZ_PENDING;
                }
                writeFixedLiteral(context, internal->literals_[internal->outLiteralSize_]);
                ++internal->outLiteralSize_;
            }
            internal->state_ = SZ_State_LZSS;
        }
        continue;
        //--- SZ_State_Dynamic
        //------------------------------------------------------------------
        case SZ_State_Dynamic:
        {
            generateCanonicalHuffmanLengths(context);
            internal->state_ = SZ_State_Dynamic_Size;
        }
        continue;
        //--- SZ_State_Dynamic_Size
        //------------------------------------------------------------------
        case SZ_State_Dynamic_Size:
        {
            if(context->availOut_<SZ_MIN_DEFLATE_OUTBUFF_SIZE){
                return SZ_PENDING;
            }
            writeBitsLE(context, 5, internal->hlit_-257);
            writeBitsLE(context, 5, internal->hdist_-1);
            writeBitsLE(context, 4, internal->hclen_-4);
            for(sz_s32 i = 0; i<internal->hclen_; ++i){
                writeBitsLE(context, 3, internal->treeLengths_[i]);
            }
            internal->state_ = SZ_State_Dynamic_Lengths;
        }
        continue;
        //--- SZ_State_Dynamic_Lengths
        //------------------------------------------------------------------
        case SZ_State_Dynamic_Lengths:
        {
            for(; internal->currentSymbol_<internal->outSymbols_; ++internal->currentSymbol_){
                if(context->availOut_<2){
                    return SZ_PENDING;
                }
                sz_u16 code = internal->symbols_[internal->currentSymbol_];
                sz_u16 len = internal->treeLengths_[code];
                sz_u16 huffCode = internal->freqCodeDists_[code].huffCode_;
                writeBitsLE(context, len, huffCode);
                if(16<=code){
                    ++internal->currentSymbol_;
                    switch(code){
                    case 16:
                        writeBitsLE(context, internal->symbols_[internal->currentSymbol_], 2);
                        break;
                    case 17:
                        writeBitsLE(context, internal->symbols_[internal->currentSymbol_], 3);
                        break;
                    case 18:
                        writeBitsLE(context, internal->symbols_[internal->currentSymbol_], 7);
                        break;
                    default:
                        SZ_ASSERT(false);
                        break;
                    }
                }
            }
        }
        break;
        //--- SZ_State_End
        //------------------------------------------------------------------
        case SZ_State_End:
        {
            if(SZ_FALSE == flushWriteStreamLE(context)){
                context->totalOut_ += context->thisTimeOut_;
                return SZ_PENDING;
            }
            sz_u8 adler32[4];
            adler32[0] = STATIC_CAST(sz_u8, (internal->adler_>>24)&0xFFU);
            adler32[1] = STATIC_CAST(sz_u8, (internal->adler_>>16)&0xFFU);
            adler32[2] = STATIC_CAST(sz_u8, (internal->adler_>> 8)&0xFFU);
            adler32[3] = STATIC_CAST(sz_u8, (internal->adler_>> 0)&0xFFU);
            if(SZ_FALSE == writeBytes(context, sizeof(sz_u32), adler32)){
                context->totalOut_ += context->thisTimeOut_;
                return SZ_PENDING;
            }
            context->totalOut_ += context->thisTimeOut_;

            return SZ_END;
        }
        break;
        //--- SZ_INFLATE_ERROR
        //------------------------------------------------------------------
        default:
            goto SZ_DEFLATE_ERROR;
        }//switch(internal->state_)

    }//for(;;)
SZ_DEFLATE_ERROR:
    return SZ_ERROR_FORMAT;
}

#ifdef __cplusplus
}
#endif
#endif //SZLIB_IMPLEMENTATION
