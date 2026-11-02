#ifndef __SM2_H__
#define __SM2_H__

#include "sm3.h"
#include "miracl.h"

#ifdef __cplusplus
extern "C" {
#endif

int sm3_e(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len,
          unsigned char *msg, int msg_len, unsigned char *e);

void sm2_keygen(char *wx, int *wxlen, char *wy, int *wylen, char *privkey, int *privkeylen);

void sm2_sign(unsigned char *hash, int hashlen, unsigned char *privkey, int privkeylen, unsigned char *cr, int *rlen,
              unsigned char *cs, int *slen);

int  sm2_verify(unsigned char *hash, int hashlen, unsigned char  *cr, int rlen, unsigned char *cs, int slen,
                unsigned char *wx, int wxlen, unsigned char *wy, int wylen);

int  encrypt(char *msg, int msglen, char *wx, int wxlen, char *wy, int wylen, char *outmsg);

int  decrypt(char *msg, int msglen, char *privkey, int privkeylen, char *outmsg);

void sm2_keyagreement_a1_3(unsigned char *kx1, int *kx1len,
                           unsigned char *ky1, int *ky1len,
                           unsigned char *ra,  int *ralen
                          );

int sm2_keyagreement_b1_9(
    unsigned char *kx1, int kx1len,
    unsigned char *ky1, int ky1len,
    unsigned char *pax, int paxlen,
    unsigned char *pay, int paylen,
    unsigned char *private_b,   int private_b_len,
    unsigned char *pbx, int pbxlen,
    unsigned char *pby, int pbylen,
    unsigned char *ida, int idalen,
    unsigned char *idb, int idblen,
    unsigned int  kblen,
    unsigned char *kbbuf,
    unsigned char *kx2, int *kx2len,
    unsigned char *ky2, int *ky2len,
    unsigned char *xv,  int *xvlen,
    unsigned char *yv,  int *yvlen,
    unsigned char *sb
);

int sm2_keyagreement_a4_10(
    unsigned char *kx1, int kx1len,
    unsigned char *ky1, int ky1len,
    unsigned char *pax, int paxlen,
    unsigned char *pay, int paylen,
    unsigned char *private_a,   int private_a_len,
    unsigned char *pbx, int pbxlen,
    unsigned char *pby, int pbylen,
    unsigned char *ida, int idalen,
    unsigned char *idb, int idblen,
    unsigned char *kx2, int kx2len,
    unsigned char *ky2, int ky2len,
    unsigned char *ra,  int ralen,
    unsigned int  kalen,
    unsigned char *kabuf,
    unsigned char *s1,
    unsigned char *sa
);

void sm2_keyagreement_b10(
    unsigned char *pax, int paxlen,
    unsigned char *pay, int paylen,
    unsigned char *pbx, int pbxlen,
    unsigned char *pby, int pbylen,
    unsigned char *kx1, int kx1len,
    unsigned char *ky1, int ky1len,
    unsigned char *kx2, int kx2len,
    unsigned char *ky2, int ky2len,
    unsigned char *xv, int xvlen,
    unsigned char *yv, int yvlen,
    unsigned char *ida, int idalen,
    unsigned char *idb, int idblen,
    unsigned char *s2
);

#ifdef __cplusplus
}
#endif


#endif
