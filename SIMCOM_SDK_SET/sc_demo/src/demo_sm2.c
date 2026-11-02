/*
 * @Author: Louis_Qiu
 * @Date: 2022-12-31 09:52:16
 * @Last Modified by: Louis_Qiu
 * @Last Modified time: 2023-01-01 10:11:57
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sm2.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "uart_api.h"


typedef enum
{
    SC_SM2_DEMO_KEYGEN               = 1,
    SC_SM2_DEMO_INPUTKEY             = 2,
    SC_SM2_DEMO_SHOWKEY              = 3,
    SC_SM2_DEMO_ENCRYPT              = 4,
    SC_SM2_DEMO_DECRYPT              = 5,
    SC_SM2_DEMO_MAX                  = 99
} SC_SM2_DEMO_TYPE;

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);
extern void PrintfRespData(char *buff, UINT32 length);
extern void PrintfRespHexData(char *buff, UINT32 length);


char hex2value(char *hex)
{
    int i;
    char value[2] = {0};

    for (i = 0; i < 2; i++)
    {
        if (hex[i] >= '0' && hex[i] <= '9')
        {
            value[i] = hex[i] - '0';
        }
        else if (hex[i] >= 'a' && hex[i] <= 'f')
        {
            value[i] = 10 + hex[i] - 'a';
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F')
        {
            value[i] = 10 + hex[i] - 'A';
        }
        else
        {
            return -1;
        }
    }

    sAPI_Debug("hex:[%c][%c] value:[%d][%d]", hex[0], hex[1], value[0], value[1]);

    return (char)(((value[0] & 0x0f) << 4) | (value[1] & 0x0f));
}

int hex2valueString(char *out, unsigned int outBufLen, char *in, unsigned int inLen)
{
    int i = 0;
    int transitionLen = 0;

    inLen /= 2;

    transitionLen = inLen <= outBufLen ? inLen : outBufLen;

    for (i = 0; i < transitionLen; i++)
    {
        out[i] = hex2value(in + (i * 2));
    }

    return transitionLen;
}

void SM2Demo(void)
{
    int opt = 99;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. keygen",
        "2. input key",
        "3. show key",
        "4. encrypt",
        "5. decrypt",
        "99. back"
    };

    char public_x[32] = {0};
    char public_y[32] = {0};
    char private_key[32] = {0};

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
            default:
                break;

            case SC_SM2_DEMO_MAX:
            {
                return;
            }
            break;

            case SC_SM2_DEMO_KEYGEN:
            {
                int xlen, ylen, privkeylen;

                PrintfResp("\r\n----------keygen------------\r\n");

                memset(public_x, 0, 32);
                memset(public_y, 0, 32);
                memset(private_key, 0, 32);

                sm2_keygen(public_x, &xlen, public_y, &ylen, private_key, &privkeylen);

                goto PRINT_KEY;
            }
            break;

            case SC_SM2_DEMO_SHOWKEY:
            {
                PrintfResp("\r\n----------show key------------\r\n");
PRINT_KEY:
                PrintfResp("\r\n key len:[32]\r\n");
                PrintfResp("\r\n public_key_x:[");
                PrintfRespHexData((char *)public_x, 32);
                PrintfResp("]\r\n");
                PrintfResp("\r\n public_key_y:[");
                PrintfRespHexData((char *)public_y, 32);
                PrintfResp("]\r\n");
                PrintfResp("\r\n private_key: [");
                PrintfRespHexData((char *)private_key, 32);
                PrintfResp("]\r\n");
            }
            break;

            case SC_SM2_DEMO_INPUTKEY:
            {
                int transitionLen = 0;
                char tempstring[64] = {0};

                PrintfResp("\r\n----------input key------------\r\n");

                memset(public_x, 0, 32);
                memset(public_y, 0, 32);
                memset(private_key, 0, 32);

                PrintfResp("\r\nPlease INPUT 32bit public key x HEX value:\r\n");

                UartRead(tempstring, 64, TRUE);
                transitionLen = hex2valueString(public_x, 32, tempstring, 64);
                sAPI_Debug("transitionLen:%d", transitionLen);

                PrintfResp("\r\nPlease INPUT 32bit public key y HEX value:\r\n");

                UartRead(tempstring, 64, TRUE);
                transitionLen = hex2valueString(public_y, 32, tempstring, 64);
                sAPI_Debug("transitionLen:%d", transitionLen);

                PrintfResp("\r\nPlease INPUT 32bit private key HEX value:\r\n");

                UartRead(tempstring, 64, TRUE);
                transitionLen = hex2valueString(private_key, 32, tempstring, 64);
                sAPI_Debug("transitionLen:%d", transitionLen);

                goto PRINT_KEY;
            }
            break;

            case SC_SM2_DEMO_ENCRYPT:
            {
                int code = 0;
                int totalLen = 0;
                int transitionLen = 0;
                char *inData = NULL;
                char *encryptData = NULL;
                char *tempString = NULL;

                PrintfResp("\r\n----------encrypt------------\r\n");

                do
                {
                    PrintfResp("\r\nPlease INPUT DATA code:\r\n");
                    PrintfResp("\r\n0. source value    1. hex value\r\n");
                    code = UartReadValue();
                } while (code > 1 || code < 0);

                PrintfResp("\r\nPlease INPUT DATA len:\r\n");
                totalLen = UartReadValue();
                if (totalLen <= 0)
                {
                    break;
                }

                tempString = malloc(totalLen);
                if (1 == code)
                {
                    inData = malloc(totalLen / 2);
                }
                else
                {
                    inData = malloc(totalLen);
                }
                encryptData = malloc(64 + totalLen + 32);

                if (tempString && inData && encryptData)
                {
                    PrintfResp("\r\nPlease INPUT DATA:\r\n");
                    UartRead(tempString, totalLen, TRUE);
                }
                PrintfResp("\r\n");

                if (1 == code)
                {
                    transitionLen = hex2valueString(inData, totalLen / 2, tempString, totalLen);
                    sAPI_Debug("transitionLen:%d", transitionLen);
                }
                else
                {
                    memcpy(inData, tempString, totalLen);
                }

                encrypt(inData, totalLen, public_x, 32, public_y, 32, encryptData);

                PrintfResp("\r\n source data:[");
                PrintfRespData((char *)inData, totalLen);
                PrintfResp("]\r\n");

                PrintfResp("\r\n source data hex:[");
                PrintfRespHexData((char *)inData, totalLen);
                PrintfResp("]\r\n");

                PrintfResp("\r\n encypt data:[");
                PrintfRespHexData((char *)encryptData, 64 + totalLen + 32);
                PrintfResp("]\r\n");

                free(tempString);
                free(inData);
                free(encryptData);
            }
            break;

            case SC_SM2_DEMO_DECRYPT:
            {
                int code = 0;
                int totalLen = 0;
                int transitionLen = 0;
                char *inData = NULL;
                char *decryptData = NULL;
                char *tempString = NULL;

                PrintfResp("\r\n----------decrypt------------\r\n");

                do
                {
                    PrintfResp("\r\nPlease INPUT DATA code:\r\n");
                    PrintfResp("\r\n0. source value    1. hex value\r\n");
                    code = UartReadValue();
                } while (code > 1 || code < 0);

                PrintfResp("\r\nPlease INPUT DATA len:\r\n");
                totalLen = UartReadValue();
                sAPI_Debug("totalLen = [%d]", totalLen);
                if (totalLen <= 0)
                {
                    break;
                }

                tempString = malloc(totalLen);
                if (1 == code)
                {
                    inData = malloc(totalLen / 2);
                }
                else
                {
                    inData = malloc(totalLen);
                }
                decryptData = malloc(totalLen - 64 - 32);

                if (tempString && inData && decryptData)
                {
                    PrintfResp("\r\nPlease INPUT DATA:\r\n");
                    UartRead(tempString, totalLen, TRUE);
                }
                PrintfResp("\r\n");

                if (1 == code)
                {
                    transitionLen = hex2valueString(inData, totalLen / 2, tempString, totalLen);
                    sAPI_Debug("transitionLen:%d", transitionLen);
                }
                else
                {
                    memcpy(inData, tempString, totalLen);
                }

                decrypt(inData, totalLen, private_key, 32, decryptData);

                PrintfResp("\r\n decypt data:[");
                PrintfRespData((char *)decryptData, ((totalLen / 2) - 64 - 32));
                PrintfResp("]\r\n");

                PrintfResp("\r\n decypt data hex:[");
                PrintfRespHexData((char *)decryptData, ((totalLen / 2) - 64 - 32));
                PrintfResp("]\r\n");

                free(tempString);
                free(inData);
                free(decryptData);
            }
            break;
        }
    }
}
