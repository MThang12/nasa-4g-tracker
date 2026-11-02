/*
 * @Author: xiaobing.fang
 * @Date: 2023-12-17 14.17.42
 * @Last Modified by: xiaobing.fang
 * @Last Modified time: 2023-12-18 12.17.42
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
// #include "sc_spi.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-SYSTEM] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-SYSTEM] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-SYSTEM] " __VA_ARGS__)

#define Get_sys_info "Get Task Info"
#define Create_temp_task "Create test Task"
#define Del_temp_task "Del test Task"

static ret_msg_t get_task_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t create_test_task_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t delete_test_task_info_cmd_proc(op_t op, arg_t *argv, int argn);

#ifdef SIMCOM_GET_TASKSTACK_INFO

static int temp_task_flag = 0;
sTaskRef g_temp1_task_ref;
sTaskRef g_temp2_task_ref;
static void temp_task1(void *args);
static void temp_task2(void *args);
static ret_msg_t temp_task_create(void);
static ret_msg_t temp_task_delete(void);
#endif



const menu_list_t func_gettaskinfo =
{
    .menu_name = Get_sys_info,
    .menu_type = TYPE_FUNC,
    .parent = &system_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = get_task_info_cmd_proc,
    },
};

const menu_list_t func_createtemptask =
{
    .menu_name = Create_temp_task,
    .menu_type = TYPE_FUNC,
    .parent = &system_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = create_test_task_info_cmd_proc,
    },
};

const menu_list_t func_deletetemptask =
{
    .menu_name = Del_temp_task,
    .menu_type = TYPE_FUNC,
    .parent = &system_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = delete_test_task_info_cmd_proc,
    },
};


const menu_list_t *system_menu_sub[] =
{
    &func_gettaskinfo,
    &func_createtemptask,
    &func_deletetemptask,
    NULL  // MUST end by NULL
};


const menu_list_t system_menu =
{
    .menu_name = "SYSTEM",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = system_menu_sub,
    .parent = &root_menu,
};



static ret_msg_t get_task_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

#ifdef SIMCOM_GET_TASKSTACK_INFO
    if(1 == temp_task_flag){
    arg_t out_arg[1] = {0};
    unsigned long stackSize, stackInuse, stackPeak;
    char *taskName;
    static UINT8 taskRefListIdx = 0;
    sTaskRef taskRefList[] = {
        g_temp1_task_ref,
        g_temp2_task_ref,
    };
    char stackInfo[sizeof(taskRefList)/sizeof(sTaskRef)][100] = {0};

    for(taskRefListIdx = 0;taskRefListIdx<(sizeof(taskRefList)/sizeof(sTaskRef));taskRefListIdx++)
    {
        sAPI_GetTaskStackInfo(taskRefList[taskRefListIdx], &taskName, &stackSize, &stackInuse, &stackPeak);
        memset(stackInfo[taskRefListIdx], 0, 100);
        snprintf(stackInfo[taskRefListIdx], 100, "%s stack(S:%ld U:%ld P:%ld)", taskName, stackSize, stackInuse, stackPeak);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "TASK info";
        out_arg[0].value = pl_demo_make_value(0, stackInfo[taskRefListIdx], strlen(stackInfo[taskRefListIdx]));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        sAPI_TaskSleep(200*2);
    }

    pl_demo_release_value(out_arg[0].value);
    }else
    {
        ret.msg = "test task no create";
        ret.errcode = 0;
    }
#else
    ret.msg = "get_task_stack_info fun not open";
    ret.errcode = 0;
#endif
    return ret;
}



static ret_msg_t create_test_task_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_GET_TASKSTACK_INFO
    ret = temp_task_create();
    temp_task_flag = 1;
    ret.msg = "two temp task create success";
#else
    ret.msg = "get_task_stack_info fun not open";
    ret.errcode = 0;
#endif
    return ret;
}
static ret_msg_t delete_test_task_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_GET_TASKSTACK_INFO
    ret = temp_task_delete();
    temp_task_flag = 0;
    ret.msg = "two temp task del success";
#else
    ret.msg = "get_task_stack_info fun not open";
    ret.errcode = 0;
#endif
    return ret;
}
#ifdef SIMCOM_GET_TASKSTACK_INFO
static ret_msg_t temp_task_create()
{
    ret_msg_t ret = {0};
    ret.errcode = sAPI_TaskCreate(&g_temp1_task_ref,
                                      NULL,
                                      SAL_1K,
                                      150,
                                      "tmptask1",
                                      temp_task1,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "temp task1 creat fail";
        }
    ret.errcode = sAPI_TaskCreate(&g_temp2_task_ref,
                                      NULL,
                                      SAL_2K,
                                      160,
                                      "tmptask2",
                                      temp_task2,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "temp task2 creat fail";
        }
    return ret;
}

static ret_msg_t temp_task_delete()
{
    ret_msg_t ret = {0};

    if (g_temp1_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_temp1_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("temp task1 delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_temp1_task_ref = NULL;
        }
    }

    if (g_temp2_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_temp2_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("temp task2 delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_temp2_task_ref = NULL;
        }
    }
DONE:
    return ret;
}

static void temp_task1(void *args)
{

}
static void temp_task2(void *args)
{

}
#endif
