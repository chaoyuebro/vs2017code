#ifndef __GZIP_H__
#define __GZIP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// gzCompress: do the compressing
int gzCompress(const char *src, int srcLen, char *dest, int destLen);
// gzDecompress: do the decompressing
int gzDecompress(const char *src, int srcLen, const char *dst, int dstLen);
#endif