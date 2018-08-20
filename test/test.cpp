#define SZLIB_IMPLEMENTATION
#include "../szlib.h"

#ifdef USE_ZLIB
#include "zlib/zlib.h"
#endif
#include <vector>
#include <string.h>
#include <random>

#ifdef _MSC_VER
#else
#include <sys/stat.h>
#endif

#include "catch.hpp"

#ifdef __cplusplus
using namespace szlib;
#endif

#ifdef USE_ZLIB
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

int def(sz_u8* dst, sz_u32 srcSize, const sz_u8* src, sz_s32 level, sz_s32 strategy)
{
    int ret, flush;
    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    ret = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS, 8, strategy);

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
#endif

int inf2(std::vector<sz_u8>& dst, sz_u32 srcSize, sz_u8* src)
{
    int ret;
    szContext context;
    ret = initInflate(&context, srcSize, src);
    if(SZ_OK != ret){
        return ret;
    }


    const int Chunk = 1024;
    sz_u8 out[Chunk];
    sz_s32 outCount = 0;
    size_t total = 0;
    dst.clear();
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
}

int def2(std::vector<sz_u8>& dst, sz_u32 srcSize, const sz_u8* src, SZ_Level level)
{
    int ret;
    szContext context;
    ret = initDeflate(&context, srcSize, src, SZ_NULL, SZ_NULL, SZ_NULL, level);
    if(SZ_OK != ret){
        return ret;
    }

    const sz_s32 Chunk = 32;
    sz_u8 out[Chunk];
    sz_s32 outCount = 0;
    sz_s32 total = 0;
    for(;;){
        context.availOut_ = Chunk;
        context.nextOut_ = out;
        ret = deflate(&context);
        switch(ret)
        {
        case SZ_ERROR_MEMORY:
        case SZ_ERROR_FORMAT:
            break;
        default:
            total = outCount+context.thisTimeOut_;
            dst.resize(dst.size() + context.thisTimeOut_);
            memcpy(&dst[0]+outCount, out, context.thisTimeOut_);
            outCount += context.thisTimeOut_;
            if(SZ_END!=ret){
                continue;
            }
            break;
        };
        break;
    }
    termDeflate(&context);
    return ret == SZ_END? total : -1;
}

#ifdef USE_ZLIB

TEST_CASE("Decode Uncompressed")
{
    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*1.5);
    sz_u8* src = new sz_u8[MaxSrcSize];
    sz_s32 srcSize = MaxSrcSize;

    {
        std::mt19937 mt;
        std::random_device rand;
        std::uniform_int_distribution<sz_s32> dist_size(0xFFFF, MaxSrcSize);
        std::uniform_int_distribution<sz_u32> dist_byte(0, 16);
        mt.seed(rand());
        srcSize = dist_size(mt);;
        for(sz_s32 i=0; i<srcSize; ++i){
            src[i] = static_cast<sz_u8>(dist_byte(mt));
        }
    }

    static const sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = new sz_u8[MaxDstSize];
    sz_s32 dstSize = 0;
    dstSize = def(dst, srcSize, (const sz_u8*)src, 0, Z_DEFAULT_STRATEGY);

    sz_s32 srcSize2;
    std::vector<sz_u8> src2;
    srcSize2 = inf2(src2, dstSize, dst);
    REQUIRE(srcSize2 == srcSize);
    for(sz_s32 i=0; i<srcSize; ++i){
        REQUIRE(src[i] == src2[i]);
    }
    delete[] dst;
    delete[] src;
}

TEST_CASE("Decode Fixed")
{
    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*2);
    sz_u8* src = new sz_u8[MaxSrcSize];
    sz_s32 srcSize = MaxSrcSize;
    static const sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = new sz_u8[MaxDstSize];
    std::vector<sz_u8> src2;
    for(int count=0; count<2; ++count){
        for(sz_s32 i=0; i<srcSize; ++i){
            src[i] = static_cast<sz_u8>(i);
        }

        sz_s32 dstSize = 0;
        dstSize = def(dst, srcSize, (const sz_u8*)src, 1, Z_FIXED);

        sz_s32 srcSize2;
        srcSize2 = inf2(src2, dstSize, dst);
        REQUIRE(srcSize2 == srcSize);
        for(sz_s32 i = 0; i<srcSize; ++i){
            REQUIRE(src[i] == src2[i]);
        }
    }
    delete[] dst;
    delete[] src;
}

TEST_CASE("Decode Dynamic")
{
    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*4);
    sz_u8* src = new sz_u8[MaxSrcSize];
    sz_s32 srcSize = MaxSrcSize;
    static const sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = new sz_u8[MaxDstSize];
    std::vector<sz_u8> src2;
    for(int count=0; count<8; ++count){
        std::mt19937 mt;
        std::random_device rand;
        std::uniform_int_distribution<sz_s32> dist_size(0xFFFF, MaxSrcSize);
        std::uniform_int_distribution<sz_u32> dist_byte(0, 16);
        unsigned int seed = rand();
        mt.seed(seed);
        srcSize = dist_size(mt);;
        for(sz_s32 i = 0; i<srcSize; ++i){
            src[i] = static_cast<sz_u8>(dist_byte(mt));
        }

        sz_s32 dstSize = 0;
        dstSize = def(dst, srcSize, (const sz_u8*)src, 9, Z_DEFAULT_STRATEGY);

        sz_s32 srcSize2;
        srcSize2 = inf2(src2, dstSize, dst);
        REQUIRE(srcSize2 == srcSize);
        for(sz_s32 i = 0; i<srcSize; ++i){
            REQUIRE(src[i] == src2[i]);
        }
    }
    delete[] dst;
    delete[] src;
}
#endif

TEST_CASE("Encode Uncompressed")
{
    std::mt19937 mt;
    std::random_device rand;
    mt.seed(rand());
    //mt.seed(12345);

    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*2);
    static const sz_s32 MaxDstSize = MaxSrcSize*2;
    sz_s32 srcSize = MaxSrcSize-15;
    sz_u8* src = new sz_u8[srcSize];
    for(sz_s32 i=0; i<srcSize; ++i){
        src[i] = static_cast<sz_u8>(mt());
    }
    std::vector<sz_u8> dst;
    sz_s32 dstSize = def2(dst, srcSize, src, SZ_Level_NoCompression);

    std::vector<sz_u8> dst2;
    sz_s32 dst2Size = inf2(dst2, (sz_s32)dst.size(), &dst[0]);

    REQUIRE(dst2Size == srcSize);
    for(sz_s32 i=0; i<srcSize; ++i){
        REQUIRE(dst2[i] == src[i]);
    }

    delete[] src;
}

TEST_CASE("Encode Fixed")
{
    std::mt19937 mt;
    std::random_device rand;
    mt.seed(rand());
    //mt.seed(12345);

    static const sz_s32 MaxSrcSize = static_cast<sz_s32>(0xFFFF*2);
    static const sz_s32 MaxDstSize = MaxSrcSize*2;
    sz_s32 srcSize = MaxSrcSize-15;
    sz_u8* src = new sz_u8[srcSize];
    for(sz_s32 i=0; i<srcSize; ++i){
        //src[i] = static_cast<sz_u8>(mt()&0x07U);
        src[i] = static_cast<sz_u8>(i);
    }
    std::vector<sz_u8> dst;
    sz_s32 dstSize = def2(dst, srcSize, src, SZ_Level_Fixed);

    std::vector<sz_u8> dst2;
    sz_s32 dst2Size = inf2(dst2, (sz_s32)dst.size(), &dst[0]);

    REQUIRE(dst2Size == srcSize);
    for(sz_s32 i=0; i<srcSize; ++i){
        REQUIRE(dst2[i] == src[i]);
    }

    delete[] src;
}