# szlib
[![Build Status](https://travis-ci.org/taqu/szlib.svg?branch=master)](https://travis-ci.org/taqu/szlib)  
szlib is a very simple zlib decompression library for C++.

# Usage
szlib is a single header only library, so put '#define SZLIB_IMPLEMENTATION' before including "szlib.h" to create the implementation.  
Set whole source data when initializing a context, then provide destination buffer to the cotext while processing.  
Note that size of the destination buffer should be SZ_MIN_OUTBUFF_SIZE(258) at least.  

## Sample code
```cpp
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
```
# License
This is free and unencumbered software released into the public domain.
