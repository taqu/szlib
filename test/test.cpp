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

#if 0
void printBits(sz_u32 x)
{
    do{
        printf("%d", (x&0x01U));
        x >>= 1;
    }while(0 != x);
}

static szFreqCode test_freqCodes[] =
{
    {16, 0, 0},{16, 1, 0},{16, 2, 0},{16, 3, 0},{16, 4, 0},{16, 5, 0},{16, 6, 0},{16, 7, 0},{16, 8, 0},{16, 9, 0},{16, 10, 0},{16, 11, 0},{16, 12, 0},{16, 13, 0},{16, 14, 0},{16, 15, 0},
{16, 16, 0},{16, 17, 0},{16, 18, 0},{16, 19, 0},{16, 20, 0},{16, 21, 0},{16, 22, 0},{16, 23, 0},{16, 24, 0},{16, 25, 0},{16, 26, 0},{16, 27, 0},{16, 28, 0},{16, 29, 0},{16, 30, 0},{16, 31, 0},
{16, 32, 0},{16, 33, 0},{16, 34, 0},{16, 35, 0},{16, 36, 0},{16, 37, 0},{16, 38, 0},{16, 39, 0},{16, 40, 0},{16, 41, 0},{16, 42, 0},{16, 43, 0},{16, 44, 0},{16, 45, 0},{16, 46, 0},{16, 47, 0},
{16, 48, 0},{16, 49, 0},{16, 50, 0},{16, 51, 0},{16, 52, 0},{16, 53, 0},{16, 54, 0},{16, 55, 0},{16, 56, 0},{16, 57, 0},{16, 58, 0},{16, 59, 0},{16, 60, 0},{16, 61, 0},{16, 62, 0},{16, 63, 0},
{16, 64, 0},{16, 65, 0},{16, 66, 0},{16, 67, 0},{16, 68, 0},{16, 69, 0},{16, 70, 0},{16, 71, 0},{16, 72, 0},{16, 73, 0},{16, 74, 0},{16, 75, 0},{16, 76, 0},{16, 77, 0},{16, 78, 0},{16, 79, 0},
{16, 80, 0},{16, 81, 0},{16, 82, 0},{16, 83, 0},{16, 84, 0},{16, 85, 0},{16, 86, 0},{16, 87, 0},{16, 88, 0},{16, 89, 0},{16, 90, 0},{16, 91, 0},{16, 92, 0},{16, 93, 0},{16, 94, 0},{16, 95, 0},
{16, 96, 0},{16, 97, 0},{16, 98, 0},{16, 99, 0},{16, 100, 0},{16, 101, 0},{16, 102, 0},{16, 103, 0},{16, 104, 0},{16, 105, 0},{16, 106, 0},{16, 107, 0},{16, 108, 0},{16, 109, 0},{16, 110, 0},{16, 111, 0},
{16, 112, 0},{16, 113, 0},{16, 114, 0},{16, 115, 0},{16, 116, 0},{16, 117, 0},{16, 118, 0},{16, 119, 0},{16, 120, 0},{16, 121, 0},{16, 122, 0},{16, 123, 0},{15, 124, 0},{15, 125, 0},{15, 126, 0},{15, 127, 0},
{15, 128, 0},{15, 129, 0},{15, 130, 0},{15, 131, 0},{15, 132, 0},{15, 133, 0},{15, 134, 0},{15, 135, 0},{15, 136, 0},{15, 137, 0},{15, 138, 0},{15, 139, 0},{15, 140, 0},{15, 141, 0},{15, 142, 0},{15, 143, 0},
{15, 144, 0},{15, 145, 0},{15, 146, 0},{15, 147, 0},{15, 148, 0},{15, 149, 0},{15, 150, 0},{15, 151, 0},{15, 152, 0},{15, 153, 0},{15, 154, 0},{15, 155, 0},{15, 156, 0},{15, 157, 0},{15, 158, 0},{15, 159, 0},
{15, 160, 0},{15, 161, 0},{15, 162, 0},{15, 163, 0},{15, 164, 0},{15, 165, 0},{15, 166, 0},{15, 167, 0},{15, 168, 0},{15, 169, 0},{15, 170, 0},{15, 171, 0},{15, 172, 0},{15, 173, 0},{15, 174, 0},{15, 175, 0},
{15, 176, 0},{15, 177, 0},{15, 178, 0},{15, 179, 0},{15, 180, 0},{15, 181, 0},{15, 182, 0},{15, 183, 0},{15, 184, 0},{15, 185, 0},{15, 186, 0},{15, 187, 0},{15, 188, 0},{15, 189, 0},{15, 190, 0},{15, 191, 0},
{15, 192, 0},{15, 193, 0},{15, 194, 0},{15, 195, 0},{15, 196, 0},{15, 197, 0},{15, 198, 0},{15, 199, 0},{15, 200, 0},{15, 201, 0},{15, 202, 0},{15, 203, 0},{15, 204, 0},{15, 205, 0},{15, 206, 0},{15, 207, 0},
{15, 208, 0},{15, 209, 0},{15, 210, 0},{15, 211, 0},{15, 212, 0},{15, 213, 0},{15, 214, 0},{15, 215, 0},{15, 216, 0},{15, 217, 0},{15, 218, 0},{15, 219, 0},{15, 220, 0},{15, 221, 0},{15, 222, 0},{15, 223, 0},
{15, 224, 0},{15, 225, 0},{15, 226, 0},{15, 227, 0},{15, 228, 0},{15, 229, 0},{15, 230, 0},{15, 231, 0},{15, 232, 0},{15, 233, 0},{15, 234, 0},{15, 235, 0},{15, 236, 0},{15, 237, 0},{15, 238, 0},{15, 239, 0},
{15, 240, 0},{15, 241, 0},{15, 242, 0},{15, 243, 0},{15, 244, 0},{15, 245, 0},{15, 246, 0},{15, 247, 0},{15, 248, 0},{15, 249, 0},{15, 250, 0},{15, 251, 0},{16, 252, 0},{16, 253, 0},{16, 254, 0},{16, 255, 0},
{0, 256, 0},{0, 257, 0},{0, 258, 0},{0, 259, 0},{0, 260, 0},{0, 261, 0},{0, 262, 0},{0, 263, 0},{0, 264, 0},{0, 265, 0},{0, 266, 0},{0, 267, 0},{0, 268, 0},{0, 269, 0},{0, 270, 0},{0, 271, 0},
{0, 272, 0},{0, 273, 0},{0, 274, 0},{0, 275, 0},{0, 276, 0},{0, 277, 0},{0, 278, 0},{0, 279, 0},{0, 280, 0},{0, 281, 0},{0, 282, 0},{0, 283, 0},{1, 284, 0},{126, 285, 0},
};

TEST_CASE("TestCHC")
{
    sz_s32 size = SZ_HLENS;
    sz_u16 lengths[SZ_HLENS];
    sz_u32 valueBuffer[SZ_REVERSE_PACKAGE_MERGE_BUFFER_SIZE];
    sz_u32 typeBuffer[SZ_REVERSE_PACKAGE_MERGE_BUFFER_SIZE];
    getLengths(size, lengths, test_freqCodes, 15, valueBuffer, typeBuffer);
    calcHuffCodes(size, test_freqCodes, lengths);

    for(sz_s32 i=0; i<size; ++i){
        printf("code=%d, freq=%d, len=%d, ", test_freqCodes[i].code_, test_freqCodes[i].frequency_, lengths[i]);
        printBits(test_freqCodes[i].huffCode_);
        printf("\n");
    }
}
#endif

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

#if 0
TEST_CASE("Encode Dynamic")
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
    sz_s32 dstSize = def2(dst, srcSize, src, SZ_Level_Dynamic);

    std::vector<sz_u8> dst2;
    sz_s32 dst2Size = inf2(dst2, (sz_s32)dst.size(), &dst[0]);

    REQUIRE(dst2Size == srcSize);
    for(sz_s32 i=0; i<srcSize; ++i){
        REQUIRE(dst2[i] == src[i]);
    }

    delete[] src;
}
#endif
