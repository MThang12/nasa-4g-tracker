/**
  ******************************************************************************
  * @file    demo_encrypt.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of encrypt lib operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

  /* Note ----------------------------------------------------------------------
The mapped mbedtls APIs are as follows:

void mbedtls_entropy_init(mbedtls_entropy_context *ctx );
int mbedtls_entropy_func( void *data, unsigned char *output, size_t len );
void mbedtls_entropy_free(mbedtls_entropy_context *ctx );

void mbedtls_ctr_drbg_init(mbedtls_ctr_drbg_context *ctx );
int mbedtls_ctr_drbg_random( void *p_rng, unsigned char *output, size_t output_len );
int mbedtls_ctr_drbg_seed( mbedtls_ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );
void mbedtls_ctr_drbg_free(mbedtls_ctr_drbg_context *ctx );

void mbedtls_aes_init( mbedtls_aes_context *ctx );
int mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits );
int mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits );
int mbedtls_aes_crypt_ecb( mbedtls_aes_context *ctx, int mode, const unsigned char *input, unsigned char *output );
void mbedtls_aes_free(mbedtls_aes_context *ctx );

int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen );
int mbedtls_base64_decode( unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen );

void mbedtls_md_init( mbedtls_md_context_t *ctx );
int mbedtls_md(const mbedtls_md_info_t *md_info, const unsigned char *input, size_t ilen, unsigned char *output );
int mbedtls_md_setup( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info, int hmac );
const mbedtls_md_info_t *mbedtls_md_info_from_type(mbedtls_md_type_t md_type );
unsigned char mbedtls_md_get_size(const mbedtls_md_info_t *md_info );
void mbedtls_md_free( mbedtls_md_context_t *ctx );

void mbedtls_pk_init(mbedtls_pk_context *ctx );
int mbedtls_pk_parse_key( mbedtls_pk_context *ctx,
                  const unsigned char *key, size_t keylen,
                  const unsigned char *pwd, size_t pwdlen );
int mbedtls_pk_parse_public_key(mbedtls_pk_context *ctx, const unsigned char *key, size_t keylen );
void mbedtls_pk_free(mbedtls_pk_context *ctx );

// static inline mbedtls_rsa_context *mbedtls_pk_rsa( const mbedtls_pk_context pk )    //static inline no mapping required
// {
//     return( (mbedtls_rsa_context *) (pk).pk_ctx );
// }

void mbedtls_rsa_init( mbedtls_rsa_context *ctx, int padding, int hash_id );
size_t mbedtls_rsa_get_len( const mbedtls_rsa_context *ctx );
int mbedtls_rsa_check_pubkey(const mbedtls_rsa_context *ctx );
int mbedtls_rsa_check_privkey(const mbedtls_rsa_context *ctx );
int mbedtls_rsa_complete( mbedtls_rsa_context *ctx );
int mbedtls_rsa_pkcs1_encrypt( mbedtls_rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t ilen,
                       const unsigned char *input,
                       unsigned char *output );
int mbedtls_rsa_pkcs1_decrypt( mbedtls_rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t *olen,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t output_max_len );
int mbedtls_rsa_pkcs1_sign(mbedtls_rsa_context *ctx,
                    int (*f_rng)(void *, unsigned char *, size_t),
                    void *p_rng,
                    int mode,
                    mbedtls_md_type_t md_alg,
                    unsigned int hashlen,
                    const unsigned char *hash,
                    unsigned char *sig );
int mbedtls_rsa_pkcs1_verify(mbedtls_rsa_context *ctx,
                    int (*f_rng)(void *, unsigned char *, size_t),
                    void *p_rng,
                    int mode,
                    mbedtls_md_type_t md_alg,
                    unsigned int hashlen,
                    const unsigned char *hash,
                    const unsigned char *sig );
void mbedtls_rsa_free(mbedtls_rsa_context *ctx );
  ------------------------------------------------------------------------------*/

  /* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "uart_api.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/md_internal.h"

typedef enum{
    SC_RSA_1024                 = 1,
    SC_RSA_2048,
    SC_RSA_4096,
    SC_MD_FILE,
    SC_RSA_DEMO_MAX             = 99
}SC_DEMO_RSA_KEY_TYPE;


extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
extern void SslTestDemoInit(void);


static char pem_public_key_1024[] = 
{
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIGJAoGBAMq/EcbAeRonKSBEbD+U73tDJ6gI3O7/PMeGZejC7aV+rrFwnKNz7uMH\n"
"qHw/1FB1pRw3ZJhH8QbtelYFbPpi36VPnmJ2Q8HyrRqA7TOjbbts/7fGXo/1Ocn4\n"
"ttbqKP6oFkMx0heGOcAuEXaAqOAty7cIaqQsJDT0MOChfRTfyMXFAgMBAAE=\n"
"-----END RSA PUBLIC KEY-----"
};
static char pem_public_key_2048[] = 
{
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIIBCgKCAQEAtOKOQZXXqtSqE7tIaijM46MkIzTfwis86oSOel39AIaqqyS9vLSW\n"
"TV5YzJHYDu7Ag26cLiQwHjoqv9u8mM1OsZMngVKXpJwR+LZO64jw0A1VoHhHlIil\n"
"WT/Qs6bcn7suAfc4lNuTKnc1Did7S28SqpeoJU6jtKGYjup9JCKg+m7vB7A3rFUz\n"
"jOTNSaSsi5XFOxRodpn7oaCO910jXfP/2dLPfJLz6dg2jADpuXeXSym96KyfeiwD\n"
"GjiRPekvCfskrQIzzQ+4/yjcROVKaf5J1NkNk1YO/t+iyHLgFgFtSc1JXa6ieCKF\n"
"tgp1JblUlQOgmIwOnxbKALAD/e6ObzkwuQIDAQAB\n"
"-----END RSA PUBLIC KEY-----"
};
static char pem_public_key_4096[] = 
{
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIICCgKCAgEAwjA6PM/Mp8E79EsQzypGcvu6UjVVPbNNqMmHcciGv0d+Vutj2iMI\n"
"9cXb2A/WJ+FpII9Y6OwgGkYpOYfiAN5npelVD2hVxCqc5slmSQYkcUKCUTb0/+3S\n"
"LuYVEy3k7m65LZRoZbz5tZc9h8YNb8yilIDUO1ikRZFLzXjBqcR2rMJ8quT41xnC\n"
"ZOErsy2ZdjpNlBADSRlJtOddSHh7Gh1sWF7nJ1rQuoh1Pbc+tygJHeHPIOYwZaO5\n"
"jQwmAvipluDs/bSyHVMcWF3FjQR4DQgfqFGs8Jt8Gd9yM6Qu21YPekpLQBHfCqNg\n"
"kvaId1DYrDs21WMV4RHZWtwyIpoLF3rNUX5MRcdObLUBt2VhTJaYO7zuBuNWNIXN\n"
"lnn4+TsDd66Vf4r4qyFTzFWIS4sSk8TQgHl9tfoTggJooTrhB5V3f7mP8BIdJZMU\n"
"KV98mFn3JIwRFcF5NWW1b09WxhlUuFnX920EevFUbCKnR0inHs05l3YCIxMRddM8\n"
"nXcaWpsqz1eWWU6cO9x1IbLXcE9ZBMLoN17ayarl5c219t7j6ZHVvYoTSQKwjFDp\n"
"T5ZwYCetIdqz/UfpqaSUsNpJZbFJGjnjirBti3gXLkLTPIfbNzGFz0EJPwszKU6t\n"
"BmbKi31nryX2mpRw/Hh8XZGiPPK/2m2L9EMDhZFHpUFtOBpQjAWHO8kCAwEAAQ==\n"
"-----END RSA PUBLIC KEY-----"
};

static const char pem_private_key_1024[] =
{
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQDKvxHGwHkaJykgRGw/lO97QyeoCNzu/zzHhmXowu2lfq6xcJyj\n"
"c+7jB6h8P9RQdaUcN2SYR/EG7XpWBWz6Yt+lT55idkPB8q0agO0zo227bP+3xl6P\n"
"9TnJ+LbW6ij+qBZDMdIXhjnALhF2gKjgLcu3CGqkLCQ09DDgoX0U38jFxQIDAQAB\n"
"AoGASNOrWf5HQeqxLiMF0yz3fQpZU1WjrsHGHlGbh6mFXb4MmDZAPcJmG+zjS4ZO\n"
"CHzJy0Hmg4DMBlYFbKLUS3h6pq4v6q/Dy7yHaFq40B4vEYC3fb3eN2t0MmYbKjKU\n"
"anhN89Q7aWftG9yDp8dU6q+cR1iq+80AsOD9oXsnLJb7GAECQQDkbhUjYI7icuAb\n"
"tjVlF8zqFZ+AXCTB7RfQzelAuWlxg5AuiTXliRj7BfPTCCLn9Znrve+cQQzrH5mr\n"
"VmlUFg4BAkEA4zdtIrUj0EhxSgbBRPeY+Uj10ZYBnHYpnE5R+7TuD+MoyIEu7o7K\n"
"ssXdtcPzhUpSFT5ggE862h25OUizHN3/xQJBALZGXRmwp/NsZQXX3D2Bw2KaxH7B\n"
"iBcyoIledZzPFqOcrsVjeZ1iS4uq0Uf4uxnK8fqZwgQ2KB/KRG3i8wNE4gECQGz7\n"
"RsNg8p6SQJRrDQRRZfQbhJxUQM2U93xbLIN4Ur8YC5WwMJyHrL12nlLye7kw3pIF\n"
"6y9f0TCRFShesFpmOKkCQH76TKjhFPZvYU54jCOy/8qEBBUqx8AfMjbCGHEa6+qk\n"
"6Hs17iHRfG7AO1dNBDhi2Mptv+tIUCiR8Q2d6QqD//8=\n"
"-----END RSA PRIVATE KEY-----"
};
static const char pem_private_key_2048[] =
{
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAtOKOQZXXqtSqE7tIaijM46MkIzTfwis86oSOel39AIaqqyS9\n"
"vLSWTV5YzJHYDu7Ag26cLiQwHjoqv9u8mM1OsZMngVKXpJwR+LZO64jw0A1VoHhH\n"
"lIilWT/Qs6bcn7suAfc4lNuTKnc1Did7S28SqpeoJU6jtKGYjup9JCKg+m7vB7A3\n"
"rFUzjOTNSaSsi5XFOxRodpn7oaCO910jXfP/2dLPfJLz6dg2jADpuXeXSym96Kyf\n"
"eiwDGjiRPekvCfskrQIzzQ+4/yjcROVKaf5J1NkNk1YO/t+iyHLgFgFtSc1JXa6i\n"
"eCKFtgp1JblUlQOgmIwOnxbKALAD/e6ObzkwuQIDAQABAoIBAHc1hBk3Hm4oP3pH\n"
"iB7mirlPEAbf1sOJaVkJSTjd7lpFZf7qdN0Dirasggs35tNYh6lW4TkLgma5YOkW\n"
"cMGnNMCUwYlaHDRQ4wstYu+ZTVNgJlleS5ZZQ9eA1hM/2dCvNE/5VOHNn7xx2ZZd\n"
"cMUzC3NABYGoxPzLLtapNRX2eG7yR2aRo5LHhw6kQgcgs5VHIW6e+xzRvP1n3XMt\n"
"1V7PWmdtc/iaxJV4SyNBgN/0F2olBzOtFkxN5SY5HBEzqBP70b9yIRMKYQ3zVwei\n"
"ybV20SfngX5hiCBacvDib9g4ePN5kAPUBRDbNGSqYaVxuFJpNCuxmPSS/gPi3o2P\n"
"NXAkngECgYEA3vZM/qixY+dmTCFwI6Al/0c6TAEaaDE7uQ+QMiVkI7kKi9mPki33\n"
"8TkwREEMvCVNPgmc/4y56ne9SyBWgWuXFMKtICceNcYI+7srhvbylxjT7uADO6Bz\n"
"ySq5VsrLx2Hm5qBGrSCXOlYywa5uxb1521yYVsHcakuDy/Hhqjs4UPkCgYEAz7Ag\n"
"6issZ0w8NIvUnghHpelq8WMyNzw61KSJlrQV60wFkohp5ZaNrx68NXA7cUgO9JKt\n"
"DmI+w6sLdM7O/cee7SgrzwvXKO1EMWP6BI1FMB6ICukNl+oGtpKgelXuDw3TJi+I\n"
"imScpc5L60DiN7Y/hHf9Vw+OTIi4PtlSOjRDjcECgYEAy3fblb4ee/ws16bgLjK0\n"
"2jyHD9sl3RWo2d5BCfIja5Iu9KbrJUx76w/0YyU12PD1h3rAYllrRHRXM5rkNqEX\n"
"s6Ng2Gm6YPpDMm6Bb14mzgqjRWf6q0xmvZsWngTdaCuIyshTFyUZvOvyz9s+TYf3\n"
"D191yh6J5hrZwKEQ/gejAgkCgYApPDsMw02ppMU/WZP5XIN9OX72Nk++D8NOfOyk\n"
"c1T2xr1Efrkn0GCJKH0Nzx6+OyRdAtJBTNhRWSCJLJaEZKH1TEiGbz9k7NbhR4xY\n"
"YwQdrbItGGCOjPSQQ0SBhZYifwgxSLa/Y4KGmRMRvmwry4+I6ZnvMzN5lpuePt/I\n"
"lp/jAQKBgQCazgGSb5c9Un6+63RMWd/D28arnnc/uSTt4i2kGq0TNF3q2QUb4xZp\n"
"numBgOAMZr1GWzFpnzv6UlzWYTz9xgZyDSwXZA/YBT2k1N2QgFQGNSvH/0+jn3fW\n"
"mJbhdrvCKfipC4adXHMWgdyTWJYh0M0QyYGF3IxuK/ROjfRzGL3njQ==\n"
"-----END RSA PRIVATE KEY-----"
};
static const char pem_private_key_4096[] =
{
"-----BEGIN RSA PRIVATE KEY-----\n"
"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\n"
"-----END RSA PRIVATE KEY-----"
};

/*
*  GREE AES (128bits, PKCS7, ECB) ECB (IV = NULL) decryption interface.
*/
INT32 sAPI_AesDecrypt(const UINT8 * key,UINT32 keybits,const UINT8 input[16], UINT8 output[16])
{
    INT32 ret = 0;
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    ret = mbedtls_aes_setkey_dec(&ctx,key,keybits);
    if(ret != 0)
    {
        sAPI_Debug("sAPI_AesDecrypt ret[%d]\r\n",ret);
        goto exit;
    }

    ret = mbedtls_aes_crypt_ecb(&ctx,MBEDTLS_AES_DECRYPT,input,output);

exit:
    mbedtls_aes_free(&ctx);
    return ret;

}


INT32 sAPI_AesEncrypt(const UINT8 * key,UINT32 keybits,const UINT8 input[16], UINT8 output[16])
{
    INT32 ret = 0;
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    ret = mbedtls_aes_setkey_enc(&ctx,key,keybits);
    if(ret != 0)
    {
        sAPI_Debug("sAPI_AesEncrypt ret[%d]\r\n",ret);
        goto exit;
    }

    ret = mbedtls_aes_crypt_ecb(&ctx,MBEDTLS_AES_ENCRYPT,input,output);
    
exit:
    mbedtls_aes_free(&ctx);
    return ret;
}

int sAPI_RsaEncrypt(char *output, const char *input, int length, const char *publickey)
{
    int ret = 0;
    int offset_input = 0, offset_output = 0;
    int block_size = 0, rsa_size = 0;
    mbedtls_pk_context ctx_pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_encrypt";

    mbedtls_pk_init(&ctx_pk);
    if (0 != mbedtls_pk_parse_public_key(&ctx_pk, (unsigned char *)publickey, strlen(publickey) + 1))
    {
        sAPI_Debug("Can't import public key");
        return -1;
    }

    mbedtls_ctr_drbg_init(&ctr_drbg);     
    mbedtls_entropy_init(&entropy);       

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));
    if (ret != 0)
    {
        sAPI_Debug(" failed ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    rsa_size = mbedtls_rsa_get_len(mbedtls_pk_rsa(ctx_pk));
    block_size = rsa_size - 11;
    sAPI_Debug("rsa_size = %d, block_size = %d, length = %d", rsa_size, block_size, length);
    while (length > 0)
    {
        int this_block_size = length > block_size ? block_size : length;
        ret = mbedtls_rsa_pkcs1_encrypt(mbedtls_pk_rsa(ctx_pk), mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC, this_block_size, (unsigned char *)(input + offset_input), (unsigned char *)(output + offset_output));
        if (ret != 0)
        {
            sAPI_Debug(" encrypt failed...");
            goto exit;
        }
        offset_input += this_block_size;
        offset_output += rsa_size;
        length -= this_block_size;
        sAPI_Debug("this_block_size = %d, block_size = %d", this_block_size, block_size);
        sAPI_Debug("offset_input = %d, offset_output = %d, length = %d", offset_input, offset_output, length);
    }

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_pk_free(&ctx_pk);

    return ret;
}


int sAPI_RsaDecrypt(char *output, int output_max_len, const char *input, int *length, const char *privatekey)
{
    int ret = -1, i = 0, block_size = 0, num_blocks = 0, out_len = 0;
    mbedtls_pk_context ctx_pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "rsa_decrypt";
    const char *input_ptr = input;
    char *output_ptr = output;
	
    mbedtls_pk_init(&ctx_pk);

    if (0 != mbedtls_pk_parse_key(&ctx_pk, (unsigned char *)privatekey, strlen(privatekey) + 1, NULL, 0))
    {
        sAPI_Debug("Can't import private key");
		return -1;
    }

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));
    if (ret != 0)
    {
        sAPI_Debug(" failed ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_rsa_complete(mbedtls_pk_rsa(ctx_pk))) != 0)
    {
        sAPI_Debug(" failed! mbedtls_rsa_complete returned %d", ret);
        goto exit;
    }

    block_size = mbedtls_rsa_get_len(mbedtls_pk_rsa(ctx_pk));
    num_blocks = (*length + block_size - 1) / block_size;

    sAPI_Debug("output_max_len = %d, block_size = %d, length = %d", output_max_len, block_size, *length);
    if (num_blocks * block_size > output_max_len) {
        sAPI_Debug("Output buffer is not large enough.");
        goto exit;
    }

    out_len = 0;

    for (i = 0; i < num_blocks; ++i) 
    {
        int block_len = (i == num_blocks - 1) ? *length - (i * block_size) : block_size;
        sAPI_Debug("block_len = %d, block_size = %d, length = %d", block_len, block_size, *length);
        if ((ret = mbedtls_rsa_pkcs1_decrypt(mbedtls_pk_rsa(ctx_pk), mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PRIVATE,
                (size_t *)&block_len, (unsigned char *)input_ptr, (unsigned char *)output_ptr, block_size)) != 0) {
            sAPI_Debug("Decryption failed: %d", ret);
            goto exit;
        }

        input_ptr += block_size;
        output_ptr += block_len;
        out_len += block_len;
        sAPI_Debug("input_ptr = %p, output_ptr = %p, out_len = %d", input_ptr, output_ptr, out_len);
        sAPI_Debug("block_len = %d, block_size = %d, length = %d", block_len, block_size, *length);
    }

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_pk_free(&ctx_pk);
    return ret;
}

/**
  * @brief  demo_RSA_Encrypt_Decrypt.
  * @param  key_type : SC_DEMO_RSA_KEY_TYPE
  * @param  test_len : The length of encrypted data to be tested, automatically filled in
  * @note  Please select an option to test from the items listed below.
  * 
  * ************************************************************
  * 1. TEST FOR RSA 1024          2. TEST FOR RSA 2048          
  * 3. TEST FOR RSA 4096          99. back                      
  * ************************************************************
  * 
  * Please enter an encryption length you want to test
  * 
  * key_type:RSA2048 test_len:1000
  * 
  * Encrypt data: 
  * 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
  * 
  * Decrypt data:
  * 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
  * @retval void
  */
void demo_RSA_Encrypt_Decrypt(int key_type, int test_len)
{
    char buf[100] = {0};
    int ret = 0, index = 0, rsa_len = 0;
    int encrypt_len = 0;
    char* encrypt_data = NULL;
    char* decrypt_data = NULL;
    const char* pem_public_key = NULL;
    const char* pem_private_key = NULL;

    // If the key length is rsa 2048, then the block length for big data partitioning is 256
    switch (key_type)
    {
    case SC_RSA_1024:
        sprintf(buf,"\r\nkey_type:RSA1024 test_len:%d\r\n",test_len);
        PrintfResp(buf); 
        rsa_len = 128;
        pem_public_key = pem_public_key_1024;
        pem_private_key = pem_private_key_1024;
        break;
    case SC_RSA_2048:
        sprintf(buf,"\r\nkey_type:RSA2048 test_len:%d\r\n",test_len);
        PrintfResp(buf); 
        rsa_len = 256;
        pem_public_key = pem_public_key_2048;
        pem_private_key = pem_private_key_2048;
        break;
    case SC_RSA_4096:
        sprintf(buf,"\r\nkey_type:RSA4096 test_len:%d\r\n",test_len);
        PrintfResp(buf); 
        rsa_len = 512;
        pem_public_key = pem_public_key_4096;
        pem_private_key = pem_private_key_4096;
        break;
    
    default:
        break;
    }

    char* data = sAPI_Malloc(test_len+1);
    if(data == NULL)
    {
        sAPI_Debug("error,[%s], malloc failed", __FUNCTION__); 
        goto exit;
    }
    memset(data,0,test_len+1);
    while(1)
    {
        data[index] = '0' + index % 10;
        index++;
        if(index >= test_len) break;
    }

    sAPI_Debug("\r\nEncrypt data size:[%ld] index=%d\r\n", strlen(data), index);
    PrintfResp("\r\nEncrypt data: \r\n"); 
    PrintfResp(data); 
    encrypt_len = ((test_len - 1)/(rsa_len - 11) + 1) * rsa_len ;
    encrypt_data = sAPI_Malloc(encrypt_len + 1);
    decrypt_data = sAPI_Malloc(encrypt_len + 1);
    if (encrypt_data == NULL || decrypt_data == NULL)
    {
        sAPI_Debug("error,[%s], malloc failed", __FUNCTION__); 
        goto exit;
    }

    memset(encrypt_data, 0, encrypt_len + 1);
    memset(decrypt_data, 0, encrypt_len + 1);
    ret = sAPI_RsaEncrypt(encrypt_data, (const char*)data, strlen(data), (const char*)pem_public_key);
    if (ret == 0)
    {
        sAPI_Debug("\r\nEncrypt successful \r\n"); 
        sAPI_Debug("Encrypt buffer data received size: %d\r\n", encrypt_len); 
        sAPI_Debug("encrypt_data: %s\r\n", encrypt_data); 
        ret = sAPI_RsaDecrypt(decrypt_data,encrypt_len + 1,encrypt_data,&encrypt_len,pem_private_key);
        if (ret >= 0)
        {
            sAPI_Debug("\r\nDecrypt successful \r\n"); 
            PrintfResp("\r\nDecrypt data: \r\n"); 
            PrintfResp(decrypt_data);
            sAPI_Debug("Decrypt buffer data received size: %d\r\n", strlen(decrypt_data)); 
        }
    }
    else
    {
        sAPI_Debug("Encrypt unsuccessful ret=%d\r\n", __FUNCTION__, ret); 
    }

exit:
    sAPI_Free(data);
    sAPI_Free(encrypt_data);
    sAPI_Free(decrypt_data);
}

// #ifdef HAS_DEMO_FS
#include "simcom_file.h"
static unsigned char hash[10240] = {0}; 

int mbedtls_md_file( const mbedtls_md_info_t *md_info, const char *path, unsigned char *output )
{
    int ret;
    SCFILE *fp = NULL;
    int read_n;
    mbedtls_md_context_t ctx;
    unsigned char buf[1024];

    if( md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    if( ( fp = sAPI_fopen( path, "rb" ) ) == NULL )
        return( MBEDTLS_ERR_MD_FILE_IO_ERROR );

    mbedtls_md_init( &ctx );

    if( ( ret = mbedtls_md_setup( &ctx, md_info, 0 ) ) != 0 )
        goto cleanup;

    if( ( ret = md_info->starts_func( ctx.md_ctx ) ) != 0 )
        goto cleanup;

    while( ( read_n = sAPI_fread( buf, 1024, 1, fp ) ) > 0 )
        if( ( ret = md_info->update_func( ctx.md_ctx, buf, read_n ) ) != 0 )
            goto cleanup;

    ret = md_info->finish_func( ctx.md_ctx, output );

cleanup:
    // mbedtls_platform_zeroize( buf, sizeof( buf ) );  //memset( buf, 0, sizeof( buf );
    sAPI_fclose( fp );
    mbedtls_md_free( &ctx );

    return( ret );
}

// #endif
/**
  * @brief  encrypt Demo.
  * @param  void
  * @note  
  * @retval void
  */
void MbedTLSDemo(void)
{
    UINT32 opt = 0, len = 0;
    int ret = 0;
    char path[] = "c:/hash_test.txt";
    char *note = "\r\n\r\nPlease select an option to test from the items listed below.\r\n";
    char *input_len = "\r\nPlease enter an encryption length you want to test\r\n";
    char *options_list[] = {
    "1. TEST FOR RSA 1024",
    "2. TEST FOR RSA 2048",
    "3. TEST FOR RSA 4096",
    "99. back",
     };

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));

        opt = UartReadValue();
        sAPI_Debug("opt = [%d]", opt);

        switch(opt)
        {
            case SC_RSA_1024:
            case SC_RSA_2048:
            case SC_RSA_4096:
                {
                    PrintfResp(input_len);
                    len = UartReadValue();
                    sAPI_Debug("len = [%d]", len);
                    sAPI_Debug("Start demo test encrypt!");
                    demo_RSA_Encrypt_Decrypt(opt, len);
                    break;
                }
            case SC_MD_FILE:
                {
                    if( ( ret = mbedtls_md_file( mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), path, hash ) ) != 0 )
                    {
                        sAPI_Debug( " failed\n  ! Could not open or read %s ret = %d\n\n", path, ret );
                    }
                    break;
                }
            case SC_RSA_DEMO_MAX:
                {
                    return;
                }
            
            default :
                 break;
        }

    }
}


// /**
//   * @brief  encrypt demo
//   * @param  void
//   * @note   this demo will create encrypt based on AES and base64 then decode it
//   * @retval ret
//   */
// void sAPP_EncrypeDemo(void)
// {
//     UINT8 key[17] = { "0123456789ABCDEF" };
//     UINT8 Plaintext[64];
//     UINT8 ciphertext[64];

//     UINT8 tmpstr[64];
//     int inlen = 0, outlen = 0, tmpstrlen = 0;
//     int ret = 0;


//     UINT8 buffer[128];


//     memset(Plaintext, 0, sizeof(Plaintext));
//     memset(ciphertext, 0, sizeof(ciphertext));
//     memset(tmpstr, 0, sizeof(tmpstr));

//     /*aes test*/
//     inlen = snprintf((char*)Plaintext, sizeof(Plaintext), "%s", "1122334455667788");
//     ret = sAPI_AesEncrypt(key, 128, Plaintext, ciphertext);
//     if (ret == 0)
//     {
//         ret = sAPI_AesDecrypt(key, 128, ciphertext, tmpstr);
//         if (ret == 0)
//         {
//             if (memcmp(Plaintext, tmpstr, strlen((char*)Plaintext)) == 0)
//                 sAPI_Debug("aes test sucess!!");
//         }
//     }

//     /*base64 test*/


//     memset(buffer, 0, sizeof(buffer));

//     ret = sAPI_Base64Encode(buffer, sizeof(buffer), &outlen, (const UINT8*)"123456789", strlen("123456789"));
//     sAPI_Debug("ret[%d] outlen[%d] buffer[%s]", ret, outlen, buffer);
//     if (ret != 0)
//         return;


//     memset(tmpstr, 0, sizeof(tmpstr));
//     tmpstrlen = 0;

//     ret = sAPI_Base64Decode(tmpstr, sizeof(tmpstr), &tmpstrlen, buffer, outlen);
//     if (ret != 0)
//         return;

//     sAPI_Debug("ret[%d] tmpstrlen[%d] buffer[%s]", ret, tmpstrlen, tmpstr);

//     if (memcmp("123456789", tmpstr, strlen("123456789")) == 0)
//     {
//         sAPI_Debug("Base64 test sucess");
//     }





// }




