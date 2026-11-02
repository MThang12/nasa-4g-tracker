
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_common.h"
#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"
#include "simcom_os.h"
#include "simcom_mqtts_client.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-MQTT] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-MQTT] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-MQTT] " __VA_ARGS__)

#define INDEX "index"
#define CLIENT_ID "client id"
#define SSL_ENABLE "ssl enable"
#define CONFIG_TYPE "config_type"
#define CONFIG_TYPE_VALUE "config_type_value"
#define CONFIG_SSL "config_SSL"
#define SERVER_HOST "server_host"
#define ALIVE_TIME "alive_time"
#define CLEAN_SESSION "clean session"
#define MQTT_USER_NAME "user name"
#define MQTT_PASSWORD "password"
#define TOPIC_DATA "topic"
#define MQTT_QOS "qos"
#define MQTT_DUP "dup"
#define PAYLOAD_DATA "payload data"
#define PAYLOAD_LENGTH "payload length"
#define MQTT_RATAINED "retained"
#define DISSCON_TIMEOUT "disconnect_timeout"

sal_msgq_ref mqtt_recv_publish_msgq = NULL;
static sal_task_ref mqttProcesser;
static int inited = 0;

static ret_msg_t mqtt_demo_task_init(void);

static ret_msg_t mqtt_init_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_set_clientid_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_configure_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_configure_ssl_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_connect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_subscribe_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_publish_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_disconnect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_release_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mqtt_deinit_cmd_proc(op_t op, arg_t *argv, int argn);


const menu_list_t func_mqtt_init =
{
    .menu_name = "MQTT init",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_init_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};


const value_list_t index_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};
const value_list_t clientid_range =
{
    .isrange = true,
    .min = 1,
    .max = 128,
};
const value_list_t ssl_enable_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};

arg_t mqtt_accq_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = CLIENT_ID,
        .msg = "String type, identifies the client id of MQTT",
    },
    {
        .type = TYPE_INT,
        .arg_name = SSL_ENABLE,
        .msg = "Integer type, 0:disable ssl; 1:enable ssl",
        .value_range = &ssl_enable_range,
    }
};

const menu_list_t func_mqtt_set_clientid =
{
    .menu_name = "MQTT set_clientid",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_accq_args,
        .argn = 3,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_set_clientid_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const value_list_t config_type_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};

const value_list_t config_type_value_range =
{
    .isrange = true,
    .min = 0, //(0~1 || 20~120)
    .max = 120,
};


arg_t mqtt_cfg_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = CONFIG_TYPE,
        .msg = "Integer type, configure config_type",
        .value_range = &config_type_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = CONFIG_TYPE_VALUE,
        .msg = "Integer type, configure value of config_type",
        .value_range = &config_type_value_range,
    },

};

const menu_list_t func_mqtt_configure =
{
    .menu_name = "MQTT configure",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_cfg_args,
        .argn = 3,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_configure_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};


const value_list_t config_ssl_range =
{
    .isrange = true,
    .min = 0,
    .max = 9,
};

arg_t mqtt_cfg_ssl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = CONFIG_SSL,
        .msg = "Integer type, configure config_ssl",
        .value_range = &config_ssl_range,
    },
};


const menu_list_t func_mqtt_configure_ssl =
{
    .menu_name = "MQTT configure ssl",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_cfg_ssl_args,
        .argn = 2,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_configure_ssl_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const value_list_t alive_time_range =
{
    .isrange = true,
    .min = 1,
    .max = 64800,
};
    
const value_list_t clean_session_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};

arg_t mqtt_connect_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = SERVER_HOST,
        .msg = "string type, host ,ip or domain(tcp://broker.emqx.io:1883)",
    },
    {
        .type = TYPE_INT,
        .arg_name = ALIVE_TIME,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &alive_time_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = CLEAN_SESSION,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &clean_session_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = MQTT_USER_NAME,
        .msg = "Integer type, host ,user name of mqtt",
    },
    {
        .type = TYPE_STR,
        .arg_name = MQTT_PASSWORD,
        .msg = "Integer type, host ,password of mqtt",
    },
};

const menu_list_t func_mqtt_connect =
{
    .menu_name = "MQTT connect",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_connect_args,
        .argn = 6,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_connect_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

/*const value_list_t topic_length_range =
{
    .isrange = true,
    .min = 1,
    .max = 1024,
};
*/
const value_list_t qos_range =
{
    .isrange = true,
    .min = 0,
    .max = 2,
};

const value_list_t dup_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};

arg_t mqtt_subscribe_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = TOPIC_DATA,
        .msg = "Integer type, input topic(length: 1~1024)",
    },
/*
    {
        .type = TYPE_INT,
        .arg_name = TOPIC_LEN,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &topic_length_range,
    },
*/
    {
        .type = TYPE_INT,
        .arg_name = MQTT_QOS,
        .msg = "Integer type, qos",
        .value_range = &qos_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = MQTT_DUP,
        .msg = "Integer type, dup",
        .value_range = &dup_range,
    },
};

const menu_list_t func_mqtt_subscribe =
{
    .menu_name = "MQTT subscribe topic",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_subscribe_args,
        .argn = 4,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_subscribe_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const value_list_t payload_length_range =
{
    .isrange = true,
    .min = 1,
    .max = 10240,
};

arg_t mqtt_publish_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = TOPIC_DATA,
        .msg = "Integer type, input topic(length: 1~1024)",
    },
/*
    {
        .type = TYPE_INT,
        .arg_name = TOPIC_LEN,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &topic_length_range,
    },
*/
    {
        .type = TYPE_STR,
        .arg_name = PAYLOAD_DATA,
        .msg = "Integer type, input topic(length: 1~10240)",
    },
    {
        .type = TYPE_INT,
        .arg_name = PAYLOAD_LENGTH,
        .msg = "Integer type, set length of payload",
        .value_range = &payload_length_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = MQTT_QOS,
        .msg = "Integer type, qos",
        .value_range = &qos_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = MQTT_DUP,
        .msg = "Integer type, dup",
        .value_range = &dup_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = MQTT_RATAINED,
        .msg = "Integer type, ratained",
        .value_range = &dup_range,
    },
};

const menu_list_t func_mqtt_publish =
{
    .menu_name = "MQTT publish one message",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_publish_args,
        .argn = 7,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_publish_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const value_list_t disconn_timeout_range =
{
    .isrange = true,
    .min = 60,
    .max = 180,
};

arg_t mqtt_disconn_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = DISSCON_TIMEOUT,
        .msg = "Integer type, disconn_timeout_range(60~180s)",
        .value_range = &disconn_timeout_range,
    }
};

const menu_list_t func_mqtt_disconnect =
{
    .menu_name = "MQTT disconnect",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_disconn_args,
        .argn = 2,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_disconnect_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};


arg_t mqtt_release_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = INDEX,
        .msg = "Integer type, identifies the index of MQTT",
        .value_range = &index_range,
    },
};

const menu_list_t func_mqtt_release =
{
    .menu_name = "MQTT release",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = mqtt_release_args,
        .argn = 1,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_release_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};


const menu_list_t func_mqtt_deinit =
{
    .menu_name = "MQTT Deinit",
    .menu_type = TYPE_FUNC,
    .parent = &mqtts_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = mqtt_deinit_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *mqtt_menu_sub[] =
{
    &func_mqtt_init,
    &func_mqtt_set_clientid,
    &func_mqtt_configure,
    &func_mqtt_configure_ssl,
    &func_mqtt_connect,
    &func_mqtt_subscribe,
    &func_mqtt_publish,
    &func_mqtt_disconnect,
    &func_mqtt_release,
    &func_mqtt_deinit,

    NULL  // MUST end by NULL
};

const menu_list_t mqtts_menu =
{
    .menu_name = "MQTT",
    .menu_type = TYPE_MENU,
    .__init = mqtt_demo_task_init,
    .__uninit = NULL,
    .submenu_list_head = mqtt_menu_sub,
    .parent = &root_menu,
};

static void mqtt_recv_subscribe_data_process(void *arg)
{
    SIM_MSG_T msg = {0, 0, 0, NULL};
    SC_STATUS status;
    SCmqttData *recv_data = NULL;
    arg_t out_arg[1] = {0};
    char title[50] = {0};
    char *put_data = NULL;
    const int data_length = 10*1024+1024+500;
    put_data = sal_malloc(data_length+1);
    if(put_data==NULL)
        LOG_ERROR("malloc data fail!!!");
    
    while(1)
    {
        status = sAPI_MsgQRecv(mqtt_recv_publish_msgq, &msg, SC_SUSPEND);
        if (status == SC_SUCCESS)
        {
            int i=0;
            if ((SC_SRV_MQTT != msg.msg_id) || (0 != msg.arg1)
                || (NULL == msg.arg3))
                continue;
            recv_data = (SCmqttData *)(msg.arg3);
            LOG_INFO("index:[%d],tpoic_len:[%d],tpoic:[%s]", recv_data->client_index, recv_data->topic_len,recv_data->topic_P); 
            LOG_INFO("payload_len:[%d]",recv_data->payload_len);
            i += snprintf(put_data+i, data_length-i, "topic: [%.*s]\r\n", recv_data->topic_len, recv_data->topic_P);
            i += snprintf(put_data+i, data_length-i, "payload: [");
            memcpy(put_data+i, recv_data->payload_P, recv_data->payload_len);
            i += recv_data->payload_len;
            i += snprintf(put_data+i, data_length-i, "]\r\n");
            if(i > data_length)
                LOG_ERROR("data over length of malloc!!");
                
            snprintf(title, sizeof(title), "recv subscribe data from mqtt server index[%d]", recv_data->client_index);
            out_arg[0].arg_name = title;
            out_arg[0].type = TYPE_RAW;
            out_arg[0].value = pl_demo_make_value(0, put_data, i);
            pl_demo_post_urc(&func_mqtt_subscribe, TYPE_URC_DATA, out_arg, 1, put_data, i);
            pl_demo_release_value(out_arg[0].value);
            
            /*these msg pointer must be free after using, don not change the free order*/
            sAPI_Free(recv_data->topic_P);
            sAPI_Free(recv_data->payload_P);
            sAPI_Free(recv_data);

            
            
        }
    }
}

static ret_msg_t mqtt_demo_task_init(void)
{
    ret_msg_t ret = {0};
    if (inited == 0)
    {
        ret.msg = "enter -->";

        if(sal_msgq_create(&mqtt_recv_publish_msgq, "RvMq", sizeof(SIM_MSG_T), 4) != 0)
        {
            LOG_ERROR("MQTT Init create msgq Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "MQTT Init create msgq Fail !";
            return ret;
        }
        if(sal_task_create(&mqttProcesser, NULL, SAL_4K, sal_task_priority_low_1, "MqTk", mqtt_recv_subscribe_data_process, NULL) != 0)
        {
            LOG_ERROR("MQTT Init create task Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "MQTT Init create task Fail !";
            return ret;
        }
        inited = 1;
    }
    return ret;
}


static ret_msg_t mqtt_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
    #define URC_SUCC "MQTT Init SUCCESS"
    #define URC_FAIL "MQTT Init Fail"
    ret_msg_t ret = {0};
    ret.errcode = sAPI_MqttStart(-1);
    
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;

    return ret;
}

/**
  * @brief notify MQTT lost connection
  * @param  int index, int cause;
  * @note   *1.Advice: using it before connection every time,
         since it will be cleared after using sAPI_MqttRel().
            *2.Must not call function of reconnection directly,
         if did it will cause receiving data of mqtt fail.
         Advice: Using sAPI_MsgQSend() send a MsgQ to notify
         other task of reconnection.
  * @retval void
*/
static void app_MqttLostConnCb(int index, int cause)
{
    char notice[100] = {0};
    arg_t out_arg[1] = {0};
    
    snprintf(notice, sizeof(notice) - 1, "MQTT LostConn Notice:index_%d,cause_%d", index, cause);
    
    out_arg[0].arg_name = "MQTT offline";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, notice, strlen(notice));
    pl_demo_post_urc(&func_mqtt_connect, TYPE_URC_NOTICE, out_arg, 1, notice, strlen(notice));
    pl_demo_release_value(out_arg[0].value);

    //Advice: Using sAPI_MsgQSend() send a MsgQ to notify other task of reconnection.
    //sAPI_MsgQSend(...);

    /*MUST!!! other task to do it. --Start--*/
#if 0
    /*Reconnection*/
    //network?
    while (1)
    {
        int i = 0;
        int pGreg = 0;
        sAPI_NetworkGetCgreg(&pGreg);
        //network ->> OK
        if (1 == pGreg || 5 == pGreg)
        {
            sAPI_Debug("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
        //network ->> ERROE,sleep 10s
        else
        {
            sAPI_Debug("MQTT NETWORK STATUS IS [%d]", pGreg);
            sAPI_TaskSleep(10 * 200);
        }
        i++;
    }

    /*to reconnect*/
    if (i == 0)
    {
        //when network ok, call sAPI_MqttConnect
        snprintf(notice, sizeof(notice) - 1, "\r\nMQTT NETWORK STATUS IS NORMAL\r\n");
        PrintfResp(notice);
    }
    else
    {
        //when netword ERROR -> OK, must release and stop MQTT(you used),then restart MQTT
        /*example:
        sAPI_MqttRel(0/1);//(you used)
        sAPI_MqttStop();

        sAPI_MqttStart(-1);
        sAPI_MqttAccq(...);
        sAPI_MqttConnect(...);
        ...
        */
        snprintf(notice, sizeof(notice) - 1, "\r\nMQTT NETWORK ERROR -> OK,close then open MQTT\r\n");
        PrintfResp(notice);
    }
#endif
    /*MUST!!! other task to do it. --End--*/
}


static ret_msg_t mqtt_deinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT Deinit SUCCESS"
#define URC_FAIL "MQTT Deinit Fail"
    ret_msg_t ret = {0};
    ret.errcode = sAPI_MqttStop();

    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;

    return ret;
}


static ret_msg_t mqtt_set_clientid_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT set_clientid SUCCESS"
#define URC_FAIL "MQTT set_clientid Fail"
    ret_msg_t ret = {0};
    int index = 0, ssl_enable =0;
    const char *client_id = NULL;
    
    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_value(&ssl_enable, argv, argn, SSL_ENABLE);
    pl_demo_get_data(&client_id, argv, argn, CLIENT_ID);

    ret.errcode = sAPI_MqttAccq(0, NULL, index, (char *)client_id, ssl_enable, mqtt_recv_publish_msgq);
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}


static ret_msg_t mqtt_configure_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT configure SUCCESS"
#define URC_FAIL "MQTT configure Fail"
    ret_msg_t ret = {0};
    int index = 0, config_type = 0, config_value = 0;
    
    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_value(&config_type, argv, argn, CONFIG_TYPE);
    pl_demo_get_value(&config_value, argv, argn, CONFIG_TYPE_VALUE);

    ret.errcode = sAPI_MqttCfg(0, NULL, index, config_type, config_value);
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

static ret_msg_t mqtt_configure_ssl_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT configure ssl SUCCESS"
#define URC_FAIL "MQTT configure ssl Fail"
    ret_msg_t ret = {0};
    int index = 0, config_ssl = 0 ;
    
    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_value(&config_ssl, argv, argn, CONFIG_SSL);

    ret.errcode = sAPI_MqttSslCfg(0, NULL, index, config_ssl);
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}


static ret_msg_t mqtt_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT connect SUCCESS"
#define URC_FAIL "MQTT connect Fail"
    ret_msg_t ret = {0};
    int index = 0, alive_time = 0, clean_session = 0;
    const char *host_tmp = NULL;
    const char *username = NULL;
    const char *password = NULL;
    char host[257] = {0};

    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_value(&alive_time, argv, argn, ALIVE_TIME);
    pl_demo_get_value(&clean_session, argv, argn, CLEAN_SESSION);
    pl_demo_get_data(&host_tmp, argv, argn, SERVER_HOST);
    pl_demo_get_data(&username, argv, argn, MQTT_USER_NAME);
    pl_demo_get_data(&password, argv, argn, MQTT_PASSWORD);

    snprintf(host, 256, "%s", host_tmp);
    
    sAPI_MqttConnLostCb(app_MqttLostConnCb);
    ret.errcode = sAPI_MqttConnect(0, NULL, index, host, alive_time, clean_session, (char *)username, (char *)password);
    
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

static ret_msg_t mqtt_subscribe_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT subscribe SUCCESS"
#define URC_FAIL "MQTT subscribe Fail"
    ret_msg_t ret = {0};
    int index = 0, qos = 0, dup = 0;
    const char *topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_data(&topic_data, argv, argn, TOPIC_DATA);
    pl_demo_get_value(&qos, argv, argn, MQTT_QOS);
    pl_demo_get_value(&dup, argv, argn, MQTT_DUP);

    ret.errcode = sAPI_MqttSub(index, (char *)topic_data, strlen(topic_data), qos, dup);
    
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

static ret_msg_t mqtt_publish_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT publish SUCCESS"
#define URC_FAIL "MQTT publish Fail"
    ret_msg_t ret = {0};
    int index = 0, qos = 0, dup = 0, ratained = 0, payload_length = 0;
    const char *topic_data = NULL;
    const char *payload_data = NULL;
    

    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_data(&topic_data, argv, argn, TOPIC_DATA);
    pl_demo_get_value(&qos, argv, argn, MQTT_QOS);
    pl_demo_get_value(&dup, argv, argn, MQTT_DUP);
    pl_demo_get_value(&ratained, argv, argn, MQTT_RATAINED);
    pl_demo_get_data(&payload_data, argv, argn, PAYLOAD_DATA);
    pl_demo_get_value(&payload_length, argv, argn, PAYLOAD_LENGTH);

    sAPI_MqttTopic(index, (char *)topic_data, strlen(topic_data));
    sAPI_MqttPayload(index, (char *)payload_data, payload_length);
    ret.errcode = sAPI_MqttPub(index, qos, 60, ratained, dup);
    
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

static ret_msg_t mqtt_disconnect_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT disconnect SUCCESS"
#define URC_FAIL "MQTT disconnect Fail"
    ret_msg_t ret = {0};
    int index = 0, timeout = 0;

    pl_demo_get_value(&index, argv, argn, INDEX);
    pl_demo_get_value(&timeout, argv, argn, DISSCON_TIMEOUT);

    ret.errcode = sAPI_MqttDisConnect(0, NULL, index, timeout);
    
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

static ret_msg_t mqtt_release_cmd_proc(op_t op, arg_t *argv, int argn)
{
#undef URC_SUCC
#undef URC_FAIL
#define URC_SUCC "MQTT release SUCCESS"
#define URC_FAIL "MQTT release Fail"
    ret_msg_t ret = {0};
    int index = 0;
    
    pl_demo_get_value(&index, argv, argn, INDEX);

    ret.errcode = sAPI_MqttRel(index);
    LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = (ret.errcode == SC_MQTT_RESULT_SUCCESS) ? URC_SUCC : URC_FAIL;
    
    return ret;
}

