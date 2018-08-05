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

static const sz_u8 SZ_Z_COMPRESSION_TYPE = 8;

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
static const sz_s32 SZ_MAX_DISTANCE = 32768 + SZ_MAX_LENGTH;

static const sz_s32 SZ_MIN_OUTBUFF_SIZE = 258;

static const sz_s32 SZ_HCLENS = 15;
static const sz_s32 SZ_HCLEN_CODES = SZ_HCLENS+4;

static const sz_s32 SZ_HLENS = 286;
static const sz_s32 SZ_HDISTS = 32;

static const sz_s32 SZ_MAX_BITS_LITERAL_CODE = 15;
static const sz_s32 SZ_MAX_BITS_DISTANCE_CODE = 15;

static const sz_s16 SZ_TREE_LEFT = 0x01U;
static const sz_s16 SZ_TREE_RIGHT = 0x02U;

static const sz_s32 SZ_TREESIZE_LITERAL = 1<<(SZ_MAX_BITS_LITERAL_CODE+2);
static const sz_s32 SZ_TREESIZE_DISTANCE = 1<<(SZ_MAX_BITS_DISTANCE_CODE+2);

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

#else
#define SZ_STATIC static

#define SZ_TRUE true
#define SZ_FALSE false

#define SZ_Z_COMPRESSION_TYPE (8)

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
#define SZ_MAX_DISTANCE (32768+258)

#define SZ_MIN_OUTBUFF_SIZE (258)

#define SZ_HCLENS (15)
#define SZ_HCLEN_CODES (19)

#define SZ_HLENS (286)
#define SZ_HDISTS (32)

#define SZ_MAX_BITS_LITERAL_CODE (15)
#define SZ_MAX_BITS_DISTANCE_CODE (15)

#define SZ_TREE_LEFT (0x01U)
#define SZ_TREE_RIGHT (0x02U)

#define SZ_TREESIZE_LITERAL (1<<(SZ_MAX_BITS_LITERAL_CODE+2))
#define SZ_TREESIZE_DISTANCE (1<<(SZ_MAX_BITS_DISTANCE_CODE+2))

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
#endif


typedef void*(*FUNC_MALLOC)(sz_size_t size, void* user);
typedef void(*FUNC_FREE)(void* ptr, void* user);

/**
Result status of processing
*/
#ifdef __cplusplus
enum SZ_Status
#else
typedef enum SZ_Status_t
#endif
{
    SZ_OK = 0,
    SZ_END = 1,
    SZ_PENDING = 2,
    SZ_ERROR_MEMORY = -1,
    SZ_ERROR_FORMAT = -2,
}
#ifdef __cplusplus
;
#else
SZ_Status;
#endif

/**
Internal states
*/
#ifdef __cplusplus
enum SZ_State
#else
typedef enum SZ_State_t
#endif
{
    SZ_State_Init =0,
    SZ_State_Block,
    SZ_State_NoComp,
    SZ_State_Fixed,
    SZ_State_Dynamic,
    SZ_State_End,
}
#ifdef __cplusplus
;
#else
SZ_State;
#endif

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

#ifdef __cplusplus
struct szContextInternal;
#else
struct szContextInternal_t;
#endif

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

#ifdef __cplusplus
    szContextInternal* internal_;
#else
    struct szContextInternal_t* internal_;
#endif
}
SZ_STRUCT_END(szContext)

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
@warning Size of output buffer "nextOut_", that is a number "availOut", needs above SZ_MIN_OUTBUFF_SIZE(258) in bytes.
*/
SZ_EXTERN SZ_Status SZ_PREFIX(inflate) (szContext* context);

#ifdef __cplusplus
}
#endif
#endif //INC_SZLIB_H_

//----------------------------------------------------------------------------
//---
//--- Implementation
//---
//----------------------------------------------------------------------------
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

    SZ_STRUCT_BEGIN(szContextInternal)
    {
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
        sz_u8 buffer_[SZ_MAX_DISTANCE+SZ_MAX_LENGTH];
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
    SZ_STRUCT_END(szContextInternal)

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

SZ_STATIC void initBitStream(szBitStream* stream, sz_s32 size, const sz_u8* src)
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
    code->distance_ = readBitsBE(5, stream);
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

SZ_STATIC void flush(szContext* context)
{
    szContextInternal* internal = context->internal_;
    szCode* code = &internal->lastCode_;
    sz_u8* dst = context->nextOut_;
    if(code->literal_<SZ_HUFFMAN_ENDCODE){
        dst[context->thisTimeOut_] = STATIC_CAST(sz_u8, code->literal_);
        //push
        internal->window_[internal->windowPosition_] = STATIC_CAST(sz_u8, code->literal_);
        ++internal->windowPosition_;
        if(SZ_MAX_DISTANCE<=internal->windowPosition_){
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
            : windowPosition - distance + SZ_MAX_DISTANCE;
        for(sz_s32 i=0; i<length; ++i){
            window[windowPosition] = dst[context->thisTimeOut_] = window[prev];
            ++windowPosition;
            if(SZ_MAX_DISTANCE<=windowPosition){
                windowPosition = 0;
            }
            ++prev;
            if(SZ_MAX_DISTANCE<=prev){
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
    szContextInternal* internal = context->internal_;
    szBitStream* stream = &internal->bitStream_;
    szCode* code = &internal->lastCode_;
    if(0<code->length_){
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        flush(context);
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
        flush(context);
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
    szContextInternal* internal = context->internal_;
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
    szContextInternal* internal = context->internal_;
    szBitStream* stream = &internal->bitStream_;
    szCode* code = &internal->lastCode_;
    if(0<code->length_){
        sz_s32 remain = context->availOut_-context->thisTimeOut_;
        if(remain<code->length_){
            return SZ_PENDING;
        }
        flush(context);
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
        flush(context);
    }
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
    SZ_ASSERT(SZ_NULL != context->internal_->malloc_);
    SZ_ASSERT(SZ_NULL != context->internal_->free_);

    context->totalOut_ = 0;
    context->availOut_ = 0;
    context->nextOut_ = SZ_NULL;

    szContextInternal* internal = context->internal_;
    internal->state_ = SZ_State_Init;
    internal->lastBlockHeader_ = 0;
    internal->lastRequestLength_ = 0;
    internal->lastCode_.literal_ = 0;
    internal->lastCode_.length_ = 0;
    internal->lastCode_.distance_ = 0;
    internal->windowPosition_ = 0;
    memset(internal->buffer_, 0, SZ_MAX_LENGTH+SZ_MAX_DISTANCE);

    initBitStream(&internal->bitStream_, size, src);
}

SZ_Status SZ_PREFIX(initInflate)(szContext* context, sz_s32 size, const sz_u8* src, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(0<=size);
    SZ_ASSERT(SZ_NULL != src);
    pMalloc = (SZ_NULL == pMalloc)? sz_malloc : pMalloc;
    pFree = (SZ_NULL == pFree)? sz_free : pFree;

    szContextInternal* internal = REINTERPRET_CAST(szContextInternal*, pMalloc(sizeof(szContextInternal), user));
    if(SZ_NULL == internal){
        return SZ_ERROR_MEMORY;
    }
    context->internal_ = internal;
    internal->malloc_ = pMalloc;
    internal->free_ = pFree;
    internal->user_ = user;
    internal->window_ = internal->buffer_;
    internal->data_ = internal->buffer_ + SZ_MAX_DISTANCE;

    SZ_PREFIX(resetInflate)(context, size, src);
    return SZ_OK;
}

SZ_Status SZ_PREFIX(createInflate)(szContext* context, FUNC_MALLOC pMalloc, FUNC_FREE pFree, void* user)
{
    SZ_ASSERT(SZ_NULL != context);
    pMalloc = (SZ_NULL == pMalloc)? sz_malloc : pMalloc;
    pFree = (SZ_NULL == pFree)? sz_free : pFree;

    szContextInternal* internal = REINTERPRET_CAST(szContextInternal*, pMalloc(sizeof(szContextInternal), user));
    if(SZ_NULL == internal){
        return SZ_ERROR_MEMORY;
    }
    context->internal_ = internal;
    internal->malloc_ = pMalloc;
    internal->free_ = pFree;
    internal->user_ = user;
    internal->window_ = internal->buffer_;
    internal->data_ = internal->buffer_ + SZ_MAX_DISTANCE;

    return SZ_OK;
}

void SZ_PREFIX(termInflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);
    szContextInternal* internal = context->internal_;
    internal->free_(internal, internal->user_);
    memset(context, 0, sizeof(szContext));
}

SZ_Status SZ_PREFIX(inflate)(szContext* context)
{
    SZ_ASSERT(SZ_NULL != context);
    SZ_ASSERT(SZ_NULL != context->internal_);
    SZ_ASSERT(SZ_MIN_OUTBUFF_SIZE<=context->availOut_);
    szContextInternal* internal = context->internal_;
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
                if(stream->size_<=(stream->current_+len)){
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

#ifdef __cplusplus
}
#endif
#endif //SZLIB_IMPLEMENTATION
