
#ifdef __cplusplus
#include <cstdint>
#include <cassert>
#else
#include <stdint.h>
#include <assert.h>
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

typedef intptr_t  sz_intptr_t;
typedef uintptr_t  sz_uintptr_t;
typedef ptrdiff_t  sz_ptrdiff_t;
typedef size_t sz_size_t;

typedef bool sz_bool;

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
#define REINTERPRET_CAST(TYPE, VALUE) reinterpret_cast<TYPE>(VALUE)

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
#define REINTERPRET_CAST(TYPE, VALUE) (TYPE)(VALUE)
#endif

SZ_STATIC inline sz_s32 maximum(sz_s32 x0, sz_s32 x1)
{
    return x0<x1? x1 : x0;
}

SZ_STATIC inline sz_s32 minimum(sz_s32 x0, sz_s32 x1)
{
    return x0<x1? x0 : x1;
}

//struct HashEntry
//{
//    HashEntry
//};
//
//struct HashTable
//{
//
//};
//
//void deflate(s32& dstSize, u8*& dst, s32 maxSearch, s32 maxMatch)
//{
//}

#include <vector>
#include <string.h>
#include <random>

#ifdef _MSC_VER
#else
#include <sys/stat.h>
#endif
#include "zlib/zlib.h"

int def(sz_u8* dst, sz_u32 srcSize, const sz_u8* src)
{
    int ret, flush;
    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    ret = deflateInit(&stream, 0);
    if(Z_OK != ret){
        return ret;
    }

    const int Chunk = 16384;
    sz_u8 in[Chunk];
    sz_u8 out[Chunk];
    sz_u32 count = 0;
    int outCount = 0;
    do{
        if(srcSize<=count){
            deflateEnd(&stream);
            return outCount;
        }
        sz_u32 size = Chunk;
        if(srcSize<(count+size)){
            size = srcSize-count;
        }
        stream.avail_in = size;
        memcpy(in, src+count, size);
        count += size;
        flush = (srcSize<=count)? Z_FINISH : Z_NO_FLUSH;
        stream.next_in = in;
        do{
            stream.avail_out = Chunk;
            stream.next_out = out;
            ret = deflate(&stream, flush);
            int s = Chunk - stream.avail_out;
            memcpy(dst+outCount, out, s);
            outCount += s;
        }while(stream.avail_out == 0);
    }while(flush != Z_FINISH);
    deflateEnd(&stream);
    return outCount;
}

int inf(sz_u8* dst, sz_u32 srcSize, sz_u8* src)
{
    int ret;
    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    ret = inflateInit(&stream);
    if(Z_OK != ret){
        return ret;
    }

    const int Chunk = 16384;
    sz_u8 in[Chunk];
    sz_u8 out[Chunk];
    sz_u32 count = 0;
    int outCount = 0;
    do{
        if(srcSize<=count){
            inflateEnd(&stream);
            return Z_OK;
        }
        sz_u32 size = Chunk;
        if(srcSize<(count+size)){
            size = srcSize-count;
        }
        stream.avail_in = size;
        memcpy(in, src+count, size);
        count += size;
        stream.next_in = in;

        do{
            stream.avail_out = Chunk;
            stream.next_out = out;
            ret = inflate(&stream, Z_NO_FLUSH);
            switch(ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
                break;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&stream);
                return ret;
            }
            int s = Chunk - stream.avail_out;
            memcpy(dst+outCount, out, s);
            outCount += s;
        } while(stream.avail_out == 0);
    } while(ret != Z_STREAM_END);
    inflateEnd(&stream);
    return ret == Z_STREAM_END? outCount : Z_DATA_ERROR;
}

struct szZHeader
{
    sz_u8 compressionMethodInfo_;
    sz_u8 flags_;
    sz_u32 presetDictionary_;
    sz_u32 adler_;
};

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

struct szBitStream
{
    sz_s32 bit_;
    sz_s32 current_;
    sz_s32 size_;
    const sz_u8* src_;
};

SZ_STATIC void initBitStream(szBitStream* stream, sz_s32 size, const sz_u8* src)
{
    SZ_ASSERT(SZ_NULL != stream);
    SZ_ASSERT(SZ_NULL != src);
    stream->bit_ = 0;
    stream->current_ = 0;
    stream->size_ = size;
    stream->src_ = src;
}

/**
@return actually read size in bits
@param dst ... destination buffer
@param bits ... size in bits that try to read
@param stream
*/
sz_s32 readBits(sz_u8* dst, sz_s32 bits, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);
    sz_s32 remain;
    sz_s32 readBits;
    sz_s32 mask;
    sz_s32 offset = 0;
    sz_s32 total = bits;

    dst[offset] = 0;
    sz_s32 lastBits = 0;
    while(0<bits){
        remain = 8-stream->bit_;
        readBits = minimum(remain, bits);
        mask = (1 << readBits) - 1;

        sz_s32 newBits = (stream->src_[stream->current_] >> stream->bit_) & mask;
        dst[offset] = dst[offset] | (newBits<<lastBits);
        lastBits = readBits;
        bits -= readBits;
        stream->bit_ += readBits;
        if(8<=stream->bit_){
            stream->bit_ = 0;
            ++stream->current_;
            if(stream->size_<=stream->current_){
                break;
            }
            lastBits = 0;
        }
    }
    return total-bits;
}

/**
@return actually read size in bytes
@param dst ... destination buffer
@param bytes ... size in bytes that try to read
@param stream
*/
sz_s32 readBytes4ZeroBitOffset(sz_u32* dst, sz_s32 bytes, szBitStream* stream)
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
sz_s32 readBytes4(sz_u32* dst, sz_s32 bytes, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);
    SZ_ASSERT(0 == (bytes&0x03U));

    if(stream->bit_<=0){
        return readBytes4ZeroBitOffset(dst, bytes, stream);
    }

    sz_s32 remain = 32-stream->bit_;
    sz_u32 mask = (1 << remain) - 1;
    sz_s32 offset = 0;

    while(0<bytes && (stream->current_+5)<=stream->size_){
        const sz_u32* src = REINTERPRET_CAST(const sz_u32*, &stream->src_[stream->current_]);

        sz_u32 bits0 = (src[0] >> stream->bit_) & mask;
        sz_u32 bits1 = stream->src_[stream->current_+4];
        bits1 <<= stream->bit_;
        dst[offset] = bits0 | bits1;
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
sz_s32 readBytesZeroBitOffset(sz_u8* dst, sz_s32 bytes, szBitStream* stream)
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
@return actually read size in bytes
@param dst ... destination buffer
@param bytes ... size in bytes that try to read
@param stream
*/
sz_s32 readBytes(sz_u8* dst, sz_s32 bytes, szBitStream* stream)
{
    SZ_ASSERT(stream->current_<stream->size_);
    if(stream->bit_<=0){
        return readBytesZeroBitOffset(dst, bytes, stream);
    }

    sz_s32 bytes4 = (bytes>>2) << 2;
    sz_s32 total = bytes;

    sz_s32 read = readBytes4(REINTERPRET_CAST(sz_u32*, dst), bytes4, stream);
    bytes -= read;
    dst += read;

    while(0<bytes){
        if(stream->size_<=stream->current_){
            break;
        }
        if(readBits(dst, 8, stream)<8){
            break;
        }
        ++dst;
        --bytes;
    }
    return total-bytes;
}

SZ_STATIC inline sz_bool proceedNextBoundary(szBitStream* stream)
{
    if(0<stream->bit_){
        stream->bit_ = 0;
        ++stream->current_;
    }
    return stream->current_<=stream->size_;
}

SZ_STATIC sz_bool readZHeader(szZHeader* header, szBitStream* stream)
{
    SZ_ASSERT(SZ_NULL != header);
    SZ_ASSERT(SZ_NULL != stream);
    if(readBytesZeroBitOffset(&header->compressionMethodInfo_, 1, stream)<1){
        return SZ_FALSE;
    }
    if(readBytesZeroBitOffset(&header->flags_, 1, stream)<1){
        return SZ_FALSE;
    }
    if(hasPresetDictionary(header)){
        if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &header->presetDictionary_), 4, stream)<4){
            return SZ_FALSE;
        }
    }else{
        header->presetDictionary_ = 0;
    }
    header->adler_ = 0;
    return SZ_TRUE;
}

sz_s32 inflate(sz_u8* dst, sz_s32 srcSize, const sz_u8* src)
{
    szBitStream bitStream;
    initBitStream(&bitStream, srcSize, src);

    szZHeader zheader;
    if(!readZHeader(&zheader, &bitStream)){
        return -1;
    }

    sz_s32 method = getCompressinMethod(&zheader);

    sz_s32 bits;
    sz_u8 blockHeader;

    sz_s32 total = 0;
    for(;;){
        if(bitStream.size_<=bitStream.current_){
            break;
        }
        bits = readBits(&blockHeader, SZ_BLOCK_HEADER_SIZE, &bitStream);
        if(bits<SZ_BLOCK_HEADER_SIZE){
            break;
        }
        sz_s32 blockType = (blockHeader>>1) & SZ_FLAG_BLOCK_TYPE_MASK;
        if(SZ_BLOCK_TYPE_NOCOMPRESSION == blockType){
            proceedNextBoundary(&bitStream);
            sz_u16 len;
            sz_u16 nlen;
            if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &len), 2, &bitStream)<2){
                break;
            }
            if(readBytesZeroBitOffset(REINTERPRET_CAST(sz_u8*, &nlen), 2, &bitStream)<2){
                break;
            }
            nlen = ~nlen;
            if(len != nlen){ // nlen is len's complement
                break;
            }
            if(bitStream.size_<=(bitStream.current_+len)){
                break;
            }
            if(readBytesZeroBitOffset(dst, len, &bitStream)<len){
                break;
            }
            dst += len;
            total += len;
        }else if(SZ_BLOCK_TYPE_FIXED_HUFFMAN == blockType){
        }else if(SZ_BLOCK_TYPE_DYNAMIC_HUFFMAN == blockType){
        }else{
            break;
        }
        if(blockHeader&SZ_FLAG_LASTBLOCK){ //last block bit is set
            return total;
        }
    }
    return -1;
}

int main(int argc, char** argv)
{
    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*1.5);
    sz_u8* src = new sz_u8[MaxSrcSize];
    sz_s32 srcSize = MaxSrcSize;

    {
        std::mt19937 mt;
        std::random_device rand;
        std::uniform_int_distribution<sz_s32> dist_size(0xFFFF, MaxSrcSize);
        std::uniform_int_distribution<sz_u32> dist_byte(0, 255);
        mt.seed(rand());
        srcSize = dist_size(mt);;
        for(sz_s32 i=0; i<srcSize; ++i){
            src[i] = static_cast<sz_u8>(dist_byte(mt));
        }
    }

    static const sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = new sz_u8[MaxDstSize];
    sz_s32 dstSize = 0;
    dstSize = def(dst, srcSize, (const sz_u8*)src);

    sz_u8* src2 = new sz_u8[MaxSrcSize];
    sz_s32 srcSize2;
    srcSize2 = inflate(src2, dstSize, dst);
    for(sz_s32 i=0; i<srcSize; ++i){
        if(src[i] != src2[i]){
            return -1;
        }
    }
    delete[] src2;
    delete[] dst;
    delete[] src;
    return 0;
}
