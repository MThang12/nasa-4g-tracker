#ifndef __DEMO_PROTOCOL_PARSER_H__
#define __DEMO_PROTOCOL_PARSER_H__

#include <stdbool.h>
#include "sal_os.h"

typedef enum
{
    OP_INVALID = 0,
    OP_GET,
    OP_POST,
    OP_ACTION,
    OP_URC,
    OP_NOTIFY,
    OP_MAX
} op_t;
static const char *const op_name[OP_MAX] =
{
    "INVALID",
    "GET",
    "POST",
    "ACTION",
    "URC",
    "NOTIFY"
};

typedef enum
{
    ARG_ID_NULL = 0,
    ARG_ID_SUBLIST,
    ARG_ID_ARGS,
    ARG_ID_REQ,
    ARG_ID_SIZE,
    ARG_ID_TYPE,
    ARG_ID_PATH,
    ARG_ID_RET,
    ARG_ID_CODE,
    ARG_ID_DATA,
    ARG_ID_DATA_NAME,
    ARG_ID_MSG,
    ARG_ID_ARGNAME,  // use combine ARG_ID_ARGS to get the value of a arg from a func.
    ARG_ID_MAGIC,
    ARG_ID_MAGIC_RET,
    ARG_ID_MAX,
    ARG_ID_USER_START
} pre_arg_id_t;
static char *const pre_arg_name[ARG_ID_MAX] =
{
    "NULL",
    "SUBLIST",
    "ARGS",
    "REQ",
    "SIZE",
    "TYPE",
    "PATH",
    "RET",
    "CODE",
    "DATA",
    "DATA_NAME",
    "MSG",
    "ARG-NAME",
    "MAGIC",
    "MAGIC-RET"
};

typedef enum
{
    TYPE_STR = 0,  // for arg define or GET/POST/ACTION option.
    TYPE_INT,      // for arg define or GET/POST/ACTION option.
    TYPE_RAW,      // for arg define or GET/POST/ACTION option.
    TYPE_RAW_GBK,  // for arg define or GET/POST/ACTION option.
    TYPE_LIST,     // for arg define or GET/POST/ACTION option.
    TYPE_LIST_RAW, // just for GET/POST/ACTION option.
    TYPE_MENU,     // just for menu define and POST SUBLIST.
    TYPE_FUNC,     // just for menu define and POST SUBLIST.
    TYPE_URC_DATA,  // for func of OP_URC
    TYPE_URC_NOTICE, // for func of OP_URC
    TYPE_URC_STATUS, // for func of OP_URC
    TYPE_MAX
} type_t;
static const char *const type_name[TYPE_MAX] =
{
    "STR",
    "INT",
    "RAW",  // defult decode with utf-8
    "RAW-GBK",
    "LIST",
    "LIST-RAW",
    "MENU",
    "FUNC",
    "DATA",
    "NOTICE",
    "STATUS"
};

typedef enum
{
    RET_OK,
    RET_FAIL,
    RET_MAX
} ret_t;
static const char *const ret_name[RET_MAX] =
{
    "OK",
    "FAIL"
};

#define ERRCODE_OK               0
#define ERRCODE_DEFAULT          (-1)
#define ERRCODE_MASK             (-256)
#define ERRCODE_RAW_RECV_TIMEOUT (ERRCODE_MASK + (-1))  // wait data time no need drop data

typedef struct
{
    int val;  // for TYPE_INT
    struct  // for not TYPE_INT
    {
        char *bufp;
        unsigned int size;
    };
} value_t;

typedef struct
{
    const bool isrange;

    union
    {
        struct
        {
            const int min;
            const int max;
        };
        struct
        {
            const value_t *const list_head;
            const unsigned int count;
        };
    };
} value_list_t;

typedef struct
{
    int arg_id;  // pre_arg_id_t OR user id.
    char *arg_name;
    const char *msg;  // extern comment
    type_t type;
    value_t *value;

    const value_list_t *const value_range;  // just for INT or STR, Other type can not set range.

    char flag;  // FLAG_ACQUIRED no need get value.
} arg_t;
#define FLAG_ACQUIRED 0x01  // already get the value successed.

typedef struct
{
    int errcode;
    const char *msg;
} ret_msg_t;

typedef struct
{
    ret_msg_t (*__init)(void);
    ret_msg_t (*__uninit)(void);
    ret_msg_t (*cmd_handler)(op_t op, arg_t *argv, int argn);
    ret_msg_t (*raw_handler)(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
    int (*needRAWData)(void);
} methods_t;

typedef struct
{
    arg_t *const argv;
    const unsigned int argn;

    const char *RAWSizeLink;
    const methods_t methods;
} func_t;

typedef struct menu_list
{
    const char *const menu_name;
    const type_t      menu_type;

    union
    {
        struct
        {
            ret_msg_t (*__init)(void);
            ret_msg_t (*__uninit)(void);
            const struct menu_list **const submenu_list_head;   // list MUST end by NULL
        };
        const func_t function;
    };

    const struct menu_list *parent;
} menu_list_t;


int pl_demo_register_root_menu(const menu_list_t *const menu_root);

void pl_demo_protocol_init(void);

/**
 * @brief Send POST cmd. If arg list include raw arg and raw_data is null, It must use pl_demo_post_raw to send raw data.
 *
 * @param op operation of the cmd.
 * @param argv arg list of the cmd.
 * @param argn counts of the args.
 * @param raw_data Indicate the raw data, if arg list include raw arg.
 * @param data_size raw data size of the.
 * @return int 0 will return if success. Otherwise errcode will return.
 */
int pl_demo_post_data(op_t op, arg_t *argv, int argn, const char *raw_data, int data_size);

/**
 * @brief Send urc for a function.
 *
 * @param func Pointer to the function menu.
 * @param type TYPE_URC_DATA, TYPE_URC_NOTICE.
 * @param argv arg list of the cmd.
 * @param argn counts of the args.
 * @param raw_data Indicate the raw data, if arg list include raw arg.
 * @param data_size raw data size of the.
 * @return int
 */
int pl_demo_post_urc(const menu_list_t *func, int type, arg_t *argv, int argn, const char *raw_data, int data_size);

/**
 * @brief Send raw data. Must use this API after call pl_demo_post_data or pl_demo_post_urc to post raw args.
 *
 * @param raw_data
 * @param data_size
 * @return int return the actually data size be sended.
 */
int pl_demo_post_raw(const char *raw_data, int data_size);

/**
 * @brief If error ocurred in raw send processing. Break send raw, need call this api. It will send '\0' instead of the remained data, and release the post lock.
 */
void pl_demo_post_raw_cancel(void);

void pl_demo_post_comments(char *comment_string);

inline int gen_arg_id(const char *arg_name, int len);

/**
 * @brief Malloc and fill value struct. If datap is NULL, val will be use. If datap not NULL, val will not use.
 *
 * @param val
 * @param datap
 * @param data_size
 * @return value_t*
 */
value_t *pl_demo_make_value(int val, const char *datap, int data_size);

#define pl_demo_release_value(value) \
    do \
    { \
        if (value) \
        { \
            sal_free(value->bufp); \
            sal_free(value); \
        } \
        value = NULL; \
    } while(0);

/**
 * @brief get the value from arg list by arg name when arg type is INT.
 *
 * @param out The value data check from arg list.
 * @param argv Arg list.
 * @param argn Arg count of the arg list.
 * @param arg_name Arg name which to get.
 * @return 0 if success, -1 when the arg name cannot found in the arg list.
 */
static inline int pl_demo_get_value(int *out, arg_t *argv, int argn, const char *arg_name)
{
    int idx = 0;
    int arg_id = gen_arg_id(arg_name, 0);

    for (idx = 0; idx < argn; idx++)
    {
        if (arg_id == argv[idx].arg_id
            && TYPE_INT == argv[idx].type
            && argv[idx].value)
        {
            *out = argv[idx].value->val;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief get the str/raw data from arg list by arg name when arg type is not INT.
 *
 * @param out Pointer to indicate the str/raw data check from arg list.
 * @param argv Arg list.
 * @param argn Arg count of the arg list.
 * @param arg_name Arg name which to get.
 * @return int Size of the str/raw data. -1 will return if the arg name cannot found in the arg list.
 */
static inline int pl_demo_get_data(const char **out, arg_t *argv, int argn, const char *arg_name)
{
    int idx = 0;
    int arg_id = gen_arg_id(arg_name, 0);

    for (idx = 0; idx < argn; idx++)
    {
        if (arg_id == argv[idx].arg_id
            && TYPE_INT != argv[idx].type
            && argv[idx].value)
        {
            *out = argv[idx].value->bufp;
            return argv[idx].value->size;
        }
    }

    return -1;
}

#endif /* __DEMO_PROTOCOL_PARSER_H__ */
