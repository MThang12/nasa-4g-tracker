#include <stdio.h>
#include <stdlib.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "simcom_ssl.h"
#include "demo_ssl.h"
#ifdef HAS_DEMO_TCPIP
#include "demo_tcpip.h"
#endif

#define LOG_ERROR(...) sal_log_error("[DEMO-SSL] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-SSL] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-SSL] " __VA_ARGS__)

#ifdef HAS_DEMO_TCPIP
extern struct demo_ssl_cfg *g_ssl_cfg_list[SSL_CHANNEL_MAX];
static struct demo_ssl_cfg g_ssl_cfg_default =
{
    .enable_sni      = false,
    .ignorelocaltime = true,

    .ssl_version      = SSL_ALL,
    .auth_mode        = SSL_AUTH_NONE,
    .negotiate_time_s = 300,

    .root_cert_data    = NULL,
    .client_cert_data  = NULL,
    .client_key_data   = NULL,

    .root_cert_path = NULL,
    .client_cert_path = NULL,
    .client_key_path = NULL,

    .client_is_on_use = FALSE,
};
#endif /*HAS_DEMO_TCPIP*/
static int demo_ssl_mode = demo_ssl_default;
static ret_msg_t  demo_ssl_cfg_set_init(void);
static ret_msg_t demo_ssl_cfg_set_cmd_proc(op_t op, arg_t *argv, int argn);

static ret_msg_t  demo_ssl_cfg_get_init(void);
static ret_msg_t demo_ssl_cfg_get_cmd_proc(op_t op, arg_t *argv, int argn);

int getFirstStrtoNum(const char* str);
char *int_to_string(int num);
char *get_first_char(const char *str);

const value_t client_id_head[] =
{
    {
        .bufp = "TCP_0",
        .size = 5,
    },
    {
        .bufp = "TCP_1",
        .size = 5,
    },
    {
        .bufp = "0:ssl_client_0",
        .size = 14,
    },
    {
        .bufp = "1:ssl_client_1",
        .size = 14,
    },
    {
        .bufp = "2:ssl_client_2",
        .size = 14,
    },
    {
        .bufp = "3:ssl_client_3",
        .size = 14,
    },
    {
        .bufp = "4:ssl_client_4",
        .size = 14,
    },
    {
        .bufp = "5:ssl_client_5",
        .size = 14,
    },
    {
        .bufp = "6:ssl_client_6",
        .size = 14
    },
    {
        .bufp = "7:ssl_client_7",
        .size = 14,
    },
    {
        .bufp = "8:ssl_client_8",
        .size = 14,
    },
    {
        .bufp = "9:ssl_client_9",
        .size = 14,
    },
};
const value_list_t client_id_range =
{
    .isrange = false,
    .list_head = client_id_head,
    .count = 12,
};

const value_t enable_sni_head[] =
{
    {
        .bufp = "0:not enable SNI",
        .size = 16,
    },
    {
        .bufp = "1:enable SNI",
        .size = 12,
    },
};
const value_list_t sni_range =
{
    .isrange = false,
    .list_head = enable_sni_head,
    .count = 2,
};

const value_t ignorelocaltime_head[] =
{
    {
        .bufp = "1:ignore time check for certification",
        .size = 37,
    },
    {
        .bufp = "0:care about time check for certification",
        .size = 41,
    },
};
const value_list_t ignorelocaltime_range =
{
    .isrange = false,
    .list_head = ignorelocaltime_head,
    .count = 2,
};

const value_t ssl_version_head[] =
{
    {
        .bufp = "4:ALL",
        .size = 5,
    },
    {
        .bufp = "0:SLL3.0",
        .size = 8,
    },
    {
        .bufp = "1:TLS1.0",
        .size = 8,
    },
    {
        .bufp = "2:TLS1.1",
        .size = 8,
    },
    {
        .bufp = "3:TLS1.2",
        .size = 8,
    },
};
const value_list_t ssl_version_range =
{
    .isrange = false,
    .list_head = ssl_version_head,
    .count = 5,
};

const value_t auth_mode_head[] =
{
    {
        .bufp = "0:no authentication",
        .size = 19,
    },
    {
        .bufp = "1:server authentication",
        .size = 23,
    },
    {
        .bufp = "2:server and client authentication",
        .size = 34,
    },
    {
        .bufp = "3:client authentication and no server authentication",
        .size = 52,
    },
};
const value_list_t auth_mode_range =
{
    .isrange = false,
    .list_head = auth_mode_head,
    .count = 4,
};

const value_list_t negotiatetime_range =
{
    .isrange = true,
    .max = 300,
    .min = 10,
};

value_t negotiatetime_value =
{
    .val = 300,
};
value_t root_cert_value =
{
    .bufp = "NULL",
    .size = sizeof("NULL"),
};
value_t client_cert_value =
{
    .bufp = "NULL",
    .size = sizeof("NULL"),
};
value_t client_key_value =
{
    .bufp = "NULL",
    .size = sizeof("NULL"),
};
value_t password_value =
{
    .bufp = "NULL",
    .size = sizeof("NULL"),
};

arg_t ssl_cfg_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_CLIENT_ID,
        .value_range = &client_id_range,
        // .msg = "Select the configured client."
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_SNI,
        .value_range = &sni_range,
        // .msg = "Select whether to enable SNI."
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_IGNORELOCALTIME,
        .value_range = &ignorelocaltime_range,
        // .msg = "Select whether to enable SNI."
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_SSL_VERSION,
        .value_range = &ssl_version_range,
        // .msg = "Select ssl version."
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_AUTH_MODE,
        .value_range = &auth_mode_range,
        // .msg = "Select auth mode."
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSSL_NEGOTIATE_TIME,
        .value_range = &negotiatetime_range,
        .msg = "Set the timeout value used in SSL negotiate stage.(second)",
        .value = &negotiatetime_value
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_ROOT_CERT,
        .msg = "Input root_cert(must exist in file_system) e.g: C:/baidu.pem.",
        .value = &root_cert_value
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_CLIENT_CERT,
        .msg = "Input client_cert(must exist in file_system) e.g: C:/local_cert.pem.",
        .value = &client_cert_value
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_CLIENT_KEY,
        .msg = "Input client_key(must exist in file_system) e.g: C:/local_key.pem.",
        .value = &client_key_value
    },
};

const menu_list_t func_ssl_cfg_set =
{
    .menu_name = "SSL Config Set",
    .menu_type = TYPE_FUNC,
    .parent = &ssl_menu,
    .function = {
        .argv = ssl_cfg_args,
        .argn = 9,
        .methods.__init = demo_ssl_cfg_set_init,
        .methods.cmd_handler = demo_ssl_cfg_set_cmd_proc,
    },
};

arg_t ssl_get_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSSL_CLIENT_ID,
        .value_range = &client_id_range,
    },
};

const menu_list_t func_ssl_cfg_get =
{
    .menu_name = "SSL Config Get",
    .menu_type = TYPE_FUNC,
    .parent = &ssl_menu,
    .function = {
        .argv = ssl_get_args,
        .argn = sizeof(ssl_get_args) / sizeof(arg_t),
        .methods.__init = demo_ssl_cfg_get_init,
        .methods.cmd_handler = demo_ssl_cfg_get_cmd_proc,
    },
};

const menu_list_t *ssl_menu_sub[] =
{
    &func_ssl_cfg_set,
    &func_ssl_cfg_get,
    NULL  // MUST end by NULL
};

const menu_list_t ssl_menu =
{
    .menu_name = "SSL",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = ssl_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t demo_ssl_cfg_set_init(void)
{
    ret_msg_t ret = {0};
#ifdef HAS_DEMO_TCPIP
    for(int client_id = 0; client_id < SSL_CHANNEL_MAX; client_id++ )
    {
        if (NULL == g_ssl_cfg_list[client_id])
        {
            g_ssl_cfg_list[client_id] = sal_malloc(sizeof(struct demo_ssl_cfg));
            if (NULL == g_ssl_cfg_list[client_id])
            {
                LOG_ERROR("malloc fail!!");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "malloc fail!!";
                return ret;
            }
            memcpy(g_ssl_cfg_list[client_id], &g_ssl_cfg_default, sizeof(struct demo_ssl_cfg));
        }
    }
#endif
    demo_ssl_mode = demo_ssl_default;

    ret.errcode = ERRCODE_OK;
    ret.msg = "ssl cfg set init succ!!";
    return ret;
}

static ret_msg_t demo_ssl_cfg_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    const char *str_client_id = NULL;
    const char *str_enable_SNI = NULL;
    const char *str_ignorelocaltime = NULL;
    const char *str_ssl_version = NULL;
    const char *str_auth_mode = NULL;
    int negotiatetime_s = 0;
    const char *root_cert_path = NULL;
    const char *client_cert_path = NULL;
    const char *client_key_path = NULL;

    pl_demo_get_data(&str_client_id, argv, argn, ARGSSL_CLIENT_ID);
    pl_demo_get_data(&str_enable_SNI, argv, argn, ARGSSL_SNI);
    pl_demo_get_data(&str_ignorelocaltime, argv, argn, ARGSSL_IGNORELOCALTIME);
    pl_demo_get_data(&str_ssl_version, argv, argn, ARGSSL_SSL_VERSION);
    pl_demo_get_data(&str_auth_mode, argv, argn, ARGSSL_AUTH_MODE);
    pl_demo_get_value(&negotiatetime_s, argv, argn, ARGSSL_NEGOTIATE_TIME);
    pl_demo_get_data(&root_cert_path, argv, argn, ARGSSL_ROOT_CERT);
    pl_demo_get_data(&client_cert_path, argv, argn, ARGSSL_CLIENT_CERT);
    pl_demo_get_data(&client_key_path, argv, argn, ARGSSL_CLIENT_KEY);

    if (!str_client_id || !str_enable_SNI || !str_ignorelocaltime || !str_ssl_version || !str_auth_mode || negotiatetime_s > 300 || negotiatetime_s <10)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    unsigned char client_id = 0;
    if(strcasecmp("TCP_0", str_client_id) == 0)
    {
        demo_ssl_mode = demo_ssl_handshake_mode;
        client_id = 0;
    }
    else if(strcasecmp("TCP_1", str_client_id) == 0)
    {
        demo_ssl_mode = demo_ssl_handshake_mode;
        client_id = 1;
    }
    else
    {
        demo_ssl_mode = demo_ssl_cfg_mode;
        client_id = getFirstStrtoNum(str_client_id);
    }

    if (client_id >= SSL_CONFIG_MAX)
    {
        LOG_ERROR("client_id is invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "client_id is invalid";
        return ret;
    }

    if (demo_ssl_mode == demo_ssl_handshake_mode)
    {
#ifdef HAS_DEMO_TCPIP
        struct demo_ssl_cfg *ssl_cfg = sal_malloc(sizeof(struct demo_ssl_cfg));
        ssl_cfg->client_id = client_id;
        ssl_cfg->root_cert_path = (char *)root_cert_path;
        ssl_cfg->client_cert_path = (char *)client_cert_path;
        ssl_cfg->client_key_path = (char *)client_key_path;
        ssl_cfg->enable_sni = getFirstStrtoNum(str_enable_SNI);
        ssl_cfg->ignorelocaltime = getFirstStrtoNum(str_ignorelocaltime);
        ssl_cfg->ssl_version = getFirstStrtoNum(str_ssl_version);
        ssl_cfg->auth_mode = getFirstStrtoNum(str_auth_mode);
        ssl_cfg->negotiate_time_s = negotiatetime_s;

        LOG_INFO("client_id=%d,enable_sni=%d,ignorelocaltime=%d,ssl_version=%d,auth_mode=%d,negotiatetime_s=%d."
                ,ssl_cfg->client_id,ssl_cfg->enable_sni,ssl_cfg->ignorelocaltime
                ,ssl_cfg->ssl_version,ssl_cfg->auth_mode,ssl_cfg->negotiate_time_s);
        ret = demo_ssl_handshake_cfg(ssl_cfg);
#else
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "TCPIP is disabled!";
#endif
        return ret;
    }
    else
    {
        char *str = NULL;
        if( client_id < 0 || client_id >= SSL_CONFIG_MAX)
        {
            LOG_ERROR("client_id error: %d", client_id);
            ret.msg = "client_id error";
            ret.errcode = ERRCODE_DEFAULT;
            goto ERROR;
        }
        str = get_first_char(str_enable_SNI);
        ret.errcode = sAPI_SslSetContextIdMsg("enableSNI", client_id, str);
        if(str != NULL) sal_free(str);
        if( ret.errcode != ERRCODE_OK)
        {
            LOG_ERROR("enableSNI config failed");
            ret.msg = "enableSNI config failed";
            goto ERROR;
        }
        str = get_first_char(str_ignorelocaltime);
        ret.errcode = sAPI_SslSetContextIdMsg("ignorelocaltime", client_id, str);
        if(str != NULL) sal_free(str);
        if( ret.errcode != ERRCODE_OK)
        {
            LOG_ERROR("ignorelocaltime config failed");
            ret.msg = "ignorelocaltime config failed";
            goto ERROR;
        }
        str = get_first_char(str_ssl_version);
        ret.errcode = sAPI_SslSetContextIdMsg("sslversion", client_id, str);
        if(str != NULL) sal_free(str);
        if( ret.errcode != ERRCODE_OK)
        {
            LOG_ERROR("sslversion config failed");
            ret.msg = "sslversion config failed";
            goto ERROR;
        }
        str = get_first_char(str_auth_mode);
        ret.errcode = sAPI_SslSetContextIdMsg("authmode", client_id, str);
        if(str != NULL) sal_free(str);
        if( ret.errcode != ERRCODE_OK)
        {
            LOG_ERROR("authmode config failed");
            ret.msg = "authmode config failed";
            goto ERROR;
        }
        str = int_to_string(negotiatetime_s);
        ret.errcode = sAPI_SslSetContextIdMsg("negotiatetime", client_id, str);
        if(str != NULL) sal_free(str);
        if( ret.errcode != ERRCODE_OK)
        {
            LOG_ERROR("negotiatetime config failed");
            ret.msg = "negotiatetime config failed";
            goto ERROR;
        }
        if(root_cert_path != NULL && strcasecmp("NULL", root_cert_path) != 0)
        {
            ret.errcode = sAPI_SslSetContextIdMsg("cacert", client_id, (char *)root_cert_path);
            if( ret.errcode != ERRCODE_OK)
            {
                LOG_ERROR("cacert config failed");
                ret.msg = "cacert config failed";
                goto ERROR;
            }
        }
        if(client_cert_path != NULL && strcasecmp("NULL", client_cert_path) != 0)
        {
            ret.errcode = sAPI_SslSetContextIdMsg("clientcert", client_id, (char *)client_cert_path);
            if( ret.errcode != ERRCODE_OK)
            {
                LOG_ERROR("client_cert config failed");
                ret.msg = "client_cert config failed";
                goto ERROR;
            }
        }
        if(client_key_path != NULL && strcasecmp("NULL", client_key_path) != 0)
        {
            ret.errcode = sAPI_SslSetContextIdMsg("clientkey", client_id, (char *)client_key_path);
            if( ret.errcode != ERRCODE_OK)
            {
                LOG_ERROR("client_key config failed");
                ret.msg = "client_key config failed";
                goto ERROR;
            }
        }
        ret.msg = "ssl cfg set succ.";
        return ret;
    }

ERROR:
    return ret;
}

static ret_msg_t demo_ssl_cfg_get_init(void)
{
    ret_msg_t ret = {0};
    demo_ssl_mode = demo_ssl_default;
#ifdef HAS_DEMO_TCPIP
    for(int client_id = 0; client_id < SSL_CHANNEL_MAX; client_id++ )
    {
        if (NULL == g_ssl_cfg_list[client_id])
        {
            g_ssl_cfg_list[client_id] = sal_malloc(sizeof(struct demo_ssl_cfg));
            if (NULL == g_ssl_cfg_list[client_id])
            {
                LOG_ERROR("malloc fail!!");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "malloc fail!!";
                return ret;
            }
            memcpy(g_ssl_cfg_list[client_id], &g_ssl_cfg_default, sizeof(struct demo_ssl_cfg));
        }
    }
#endif
    ret.errcode = ERRCODE_OK;
    ret.msg = "ssl cfg set init succ!!";
    return ret;
}

static ret_msg_t demo_ssl_cfg_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *str_client_id = NULL;
    arg_t out_arg[1] = {0};
    char rsp_str[SAL_1K] = {0};
    
    pl_demo_get_data(&str_client_id, argv, argn, ARGSSL_CLIENT_ID);
    unsigned char client_id = 0;
    if(strcasecmp("TCP_0", str_client_id) == 0)
    {
        demo_ssl_mode = demo_ssl_handshake_mode;
        client_id = 0;
    }
    else if(strcasecmp("TCP_1", str_client_id) == 0)
    {
        demo_ssl_mode = demo_ssl_handshake_mode;
        client_id = 1;
    }
    else
    {
        demo_ssl_mode = demo_ssl_cfg_mode;
        client_id = getFirstStrtoNum(str_client_id);
    }

    if (demo_ssl_mode == demo_ssl_handshake_mode)
    {
#ifdef HAS_DEMO_TCPIP
        struct demo_ssl_cfg *cfgp = NULL;
        cfgp = g_ssl_cfg_list[client_id];
        snprintf(rsp_str, sizeof(rsp_str),
            "enable_sni = %d\r\n"
            "ignorelocaltime = %d\r\n"
            "ssl_version = %d\r\n"
            "auth_mode = %d\r\n"
            "negotiate_time_s = %d\r\n"
            "root_cert_path = %s\r\n"
            "client_cert_path = %s\r\n"
            "client_key_path = %s\r\n",
            (int)cfgp->enable_sni,
            (int)cfgp->ignorelocaltime,
            (int)cfgp->ssl_version,
            (int)cfgp->auth_mode,
            (int)cfgp->negotiate_time_s,
            cfgp->root_cert_path,
            cfgp->client_cert_path,
            cfgp->client_key_path);
#else
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP is disabled!";
            return ret;
#endif
    }
    else if (demo_ssl_mode == demo_ssl_cfg_mode)
    {
        SCsslContent msg = sAPI_SslGetContextIdMsg(client_id);  // Obtain the configuration parameters of the kernel
        snprintf(rsp_str, sizeof(rsp_str),
            "enable_sni = %d\r\n"
            "ignorelocaltime = %d\r\n"
            "ssl_version = %d\r\n"
            "auth_mode = %d\r\n"
            "negotiate_time_s = %d\r\n"
            "root_cert_path = %s\r\n"
            "client_cert_path = %s\r\n"
            "client_key_path = %s\r\n",
            msg.enable_SNI,
            msg.ignore_local_time,
            msg.ssl_version,
            msg.auth_mode,
            msg.negotiate_time,
            msg.ca_cert_path,
            msg.client_cert_path,
            msg.client_key_path);
    }
    else
    {
        LOG_ERROR("client_id is invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "client_id is invalid";
        return ret;
    }

    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, (const char *)rsp_str, strlen(rsp_str));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_post_raw((const char *)rsp_str, strlen(rsp_str));
    pl_demo_release_value(out_arg[0].value);
    ret.msg = "ssl cfg get successful!!!";

    return ret;
}

int getFirstStrtoNum(const char* str)
{
    if (str != NULL && *str != '\0') {
        return *str - '0';
    }
    return -1;
}

char *int_to_string(int num)
{
    int size = snprintf(NULL, 0, "%d", num); // Gets the length of the converted string
    char *str = sal_malloc(sizeof(char) * (size + 1));
    snprintf(str, size + 1, "%d", num);
    return str;
}

char* get_first_char(const char* str)
{
    char *result = (char *)malloc(2 * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    result[0] = str[0];
    result[1] = '\0';
    return result;
}