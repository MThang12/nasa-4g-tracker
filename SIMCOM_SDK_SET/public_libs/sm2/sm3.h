#ifndef __SM3_H__
#define __SM3_H__

#ifdef __cplusplus
extern "C" {
#endif


#define  SM3_LBLOCK         16
#define  SM3_CBLOCK         64
#define  SM3_DIGEST_LENGTH  32
#define  SM3_LAST_BLOCK     56

typedef struct SM3state_st
{
    unsigned long h[8];
    unsigned long Nl, Nh;
    unsigned long data[SM3_LBLOCK];
    unsigned int  num;
} SM3_CTX;

void SM3_Init(SM3_CTX *ctx);
void SM3_Update(SM3_CTX *ctx, const void *data, unsigned int len);
void SM3_Final(unsigned char *md, SM3_CTX *ctx);
unsigned char *sm3(const unsigned char *d, unsigned int n, unsigned char *md);
/*
d:  data
n:  byte length
md: 32 bytes digest
*/

#ifdef __cplusplus
}
#endif

#endif

#define nl2c(l,c)   (*((c)++) = (unsigned char)(((l) >> 24) & 0xff), \
                     *((c)++) = (unsigned char)(((l) >> 16) & 0xff), \
                     *((c)++) = (unsigned char)(((l) >> 8)  & 0xff), \
                     *((c)++) = (unsigned char)(((l)    )   & 0xff))

#define c_2_nl(c)   ((*(c) << 24) | (*(c+1) << 16) | (*(c+2) << 8) | *(c+3))
#define ROTATE(X, C) (((X) << (C)) | ((X) >> (32 - (C))))

#define TH 0x79cc4519
#define TL 0x7a879d8a
#define FFH(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define FFL(X, Y, Z) (((X) & (Y)) | ((X) & (Z)) | ((Y) & (Z)))
#define GGH(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define GGL(X, Y, Z) (((X) & (Y)) | ((~X) & (Z)))
#define P0(X)  ((X) ^ (((X) << 9) | ((X) >> 23)) ^ (((X) << 17) | ((X) >> 15)))
#define P1(X)  ((X) ^ (((X) << 15) | ((X) >> 17)) ^ (((X) << 23) | ((X) >> 9)))
