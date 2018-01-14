#define SZLIB_IMPLEMENTATION
#include "../szlib.h"
#include "zlib/zlib.h"
#include <stdlib.h>
#include <time.h>

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

int inf2(sz_s32* dstSize, sz_u8** pdst, sz_u32 srcSize, sz_u8* src)
{
    int ret;
    szContext context;
    ret = sz_initInflate(&context, srcSize, src, NULL, NULL, NULL);
    if(SZ_OK != ret){
        return ret;
    }

    const int Chunk = 16384;
    sz_u8 out[Chunk];

    sz_s32 outCount = 0;
    size_t total = 0;
    sz_u8* dst = *pdst;
    for(;;){
        context.availOut_ = Chunk;
        context.nextOut_ = out;
        ret = sz_inflate(&context);
        switch(ret)
        {
        case SZ_ERROR_MEMORY:
        case SZ_ERROR_FORMAT:
            break;
        default:
            total = outCount+context.thisTimeOut_;
            if(*dstSize<total){
                size_t capacity = *dstSize;
                while(capacity<total){
                    capacity += 1024;
                }
                *dstSize = capacity;
                dst = (sz_u8*)malloc(capacity);
                memcpy(dst, *pdst, outCount);
                free(*pdst);
                *pdst = dst;
            }
            memcpy(dst+outCount, out, context.thisTimeOut_);
            outCount += context.thisTimeOut_;
            if(SZ_END!=ret){
                continue;
            }
            break;
        };
        break;
    }
    SZ_PREFIX(termInflate)(&context);
    return ret == SZ_END? outCount : -1;
}

int test_uncompressed()
{
    static const sz_s32 MaxSrcSize = (sz_s32)(0xFFFF*1.5);
    sz_u8* src = (sz_u8*)malloc(MaxSrcSize);
    sz_s32 srcSize = MaxSrcSize;

    {
        time_t t = time(NULL);
        srand((unsigned int)t);
        srcSize = rand()%MaxSrcSize + 0xFFFF;
        for(sz_s32 i=0; i<srcSize; ++i){
            src[i] = (sz_u8)(rand()&0xFFU);
        }
    }

    sz_u8* dst = (sz_u8*)malloc(MaxSrcSize*3);
    sz_s32 dstSize = 0;
    dstSize = def(dst, srcSize, (const sz_u8*)src, 0, Z_DEFAULT_STRATEGY);

    sz_s32 buffSize=0;
    sz_u8* src2 = NULL;
    sz_s32 srcSize2 = inf2(&buffSize, &src2, dstSize, dst);
    int result = 0;
    for(sz_s32 i=0; i<srcSize; ++i){
        if(src[i] != src2[i]){
            result = -1;
            break;
        }
    }
    free(src2);
    free(dst);
    free(src);
    return result;
}

int test_fixed()
{
    static const sz_s32 MaxSrcSize = (sz_s32)(0xFFFF*1.5);
    sz_u8* src = (sz_u8*)malloc(MaxSrcSize);
    sz_s32 srcSize = MaxSrcSize;
    sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = (sz_u8*)malloc(MaxDstSize);
    sz_s32 dstSize = 0;
    sz_s32 buffSize=0;
    sz_u8* src2 = NULL;
    int result = 0;
    for(int count=0; count<128; ++count){
        {
            time_t t = time(NULL);
            srand((unsigned int)t);
            srcSize = rand()%MaxSrcSize + 0xFFFF;
            for(sz_s32 i=0; i<srcSize; ++i){
                src[i] = (sz_u8)(rand()&0xFFU);
            }
        }

        dstSize = def(dst, srcSize, (const sz_u8*)src, 1, Z_FIXED);

        sz_s32 srcSize2 = inf2(&buffSize, &src2, dstSize, dst);
        int result = 0;
        for(sz_s32 i=0; i<srcSize; ++i){
            if(src[i] != src2[i]){
                result = -1;
                break;
            }
        }
    }
    free(src2);
    free(dst);
    free(src);
    return result;
}

int test_dynamic()
{
    static const sz_s32 MaxSrcSize = (sz_s32)(0xFFFF*2);
    sz_u8* src = (sz_u8*)malloc(MaxSrcSize);
    sz_s32 srcSize = MaxSrcSize;
    sz_s32 MaxDstSize = MaxSrcSize*3;
    sz_u8* dst = (sz_u8*)malloc(MaxDstSize);
    sz_s32 dstSize = 0;
    sz_s32 buffSize=0;
    sz_u8* src2 = NULL;
    int result = 0;
    for(int count=0; count<128; ++count){
        {
            time_t t = time(NULL);
            srand((unsigned int)t);
            srcSize = rand()%MaxSrcSize + 0xFFFF;
            for(sz_s32 i=0; i<srcSize; ++i){
                src[i] = (sz_u8)(rand()&0xFFU);
            }
        }

        dstSize = def(dst, srcSize, (const sz_u8*)src, 6, Z_DEFAULT_STRATEGY);

        sz_s32 srcSize2 = inf2(&buffSize, &src2, dstSize, dst);
        int result = 0;
        for(sz_s32 i=0; i<srcSize; ++i){
            if(src[i] != src2[i]){
                result = -1;
                break;
            }
        }
    }
    free(src2);
    free(dst);
    free(src);
    return result;
}

int main(int argc, char** argv)
{
    if(test_uncompressed()<0){
        return -1;
    }
    if(test_fixed()<0){
        return -1;
    }
    if(test_dynamic()<0){
        return -1;
    }
    return 0;
}
