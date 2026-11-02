/*
 * @Author: Louis_Qiu
 * @Date: 2023-11-17 11:08:24
 * @Last Modified by: Louis_Qiu
 * @Last Modified time: 2023-11-24 09:09:00
 */

#include <stdio.h>
#include <stdlib.h>

#include "simcom_demo_init.h"
#include "demo_protocol_parser.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"


static ret_msg_t show_meminfo(op_t op, arg_t *argv, int argn);


const menu_list_t func_show_meminfo =
{
    .menu_name = "show memory info",
    .menu_type = TYPE_FUNC,
    .parent = &debug_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = show_meminfo,
    },
};

const menu_list_t *debug_menu_sub[] =
{
    &func_show_meminfo,
    NULL  // MUST end by NULL
};

const menu_list_t debug_menu =
{
    .menu_name = "simcom debug",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = debug_menu_sub,
    .parent = &root_menu,
};


extern void _print_memory_info(void(*output_cb)(const char *type, void *ptr, int size, int size2, const char *funcname,
                               int line));

void memoryinfo_print(const char *type, void *ptr, int size, int size2, const char *funcname, int line)
{
    arg_t infos[5] = {0};
    int info_count = 0;
    int idx = 0;

    if ('0' == type[0])  // TOTAL
    {
        info_count = 3;
        infos[0].arg_name = "MEM_TYPE";
        infos[0].type = TYPE_STR;
        infos[0].value = sal_malloc_notrace(sizeof(value_t));
        infos[0].value->size = 5;
        infos[0].value->bufp = sal_malloc_notrace(infos[0].value->size + 1);
        sprintf(infos[0].value->bufp, "TOTAL");

        infos[1].arg_name = "HEAP_SIZE";
        infos[1].type = TYPE_INT;
        infos[1].value = sal_malloc_notrace(sizeof(value_t));
        infos[1].value->val = size;

        infos[2].arg_name = "STACK_SIZE";
        infos[2].type = TYPE_INT;
        infos[2].value = sal_malloc_notrace(sizeof(value_t));
        infos[2].value->val = size2;
    }
    else if ('1' == type[0])  // PEAK
    {
        info_count = 3;
        infos[0].arg_name = "MEM_TYPE";
        infos[0].type = TYPE_STR;
        infos[0].value = sal_malloc_notrace(sizeof(value_t));
        infos[0].value->size = 4;
        infos[0].value->bufp = sal_malloc_notrace(infos[0].value->size + 1);
        sprintf(infos[0].value->bufp, "PEAK");

        infos[1].arg_name = "HEAP_SIZE";
        infos[1].type = TYPE_INT;
        infos[1].value = sal_malloc_notrace(sizeof(value_t));
        infos[1].value->val = size;

        infos[2].arg_name = "STACK_SIZE";
        infos[2].type = TYPE_INT;
        infos[2].value = sal_malloc_notrace(sizeof(value_t));
        infos[2].value->val = size2;
    }
    else if ('2' == type[0])  // BLOCK
    {
        info_count = 3;
        infos[0].arg_name = "MEM_TYPE";
        infos[0].type = TYPE_STR;
        infos[0].value = sal_malloc_notrace(sizeof(value_t));
        infos[0].value->size = 13;
        infos[0].value->bufp = sal_malloc_notrace(infos[0].value->size + 1);
        sprintf(infos[0].value->bufp, "CONTROL_BLOCK");

        infos[1].arg_name = "COUNT";
        infos[1].type = TYPE_INT;
        infos[1].value = sal_malloc_notrace(sizeof(value_t));
        infos[1].value->val = size;

        infos[2].arg_name = "SIZE";
        infos[2].type = TYPE_INT;
        infos[2].value = sal_malloc_notrace(sizeof(value_t));
        infos[2].value->val = size2;
    }
    else  // memory info
    {
        info_count = 5;
        infos[0].arg_name = "MEM_TYPE";
        infos[0].type = TYPE_STR;
        infos[0].value = sal_malloc_notrace(sizeof(value_t));
        infos[0].value->size = strlen(type);
        infos[0].value->bufp = sal_malloc_notrace(infos[0].value->size + 1);
        sprintf(infos[0].value->bufp, "%s", type);

        infos[1].arg_name = "MEM_POINTER";
        infos[1].type = TYPE_INT;
        infos[1].value = sal_malloc_notrace(sizeof(value_t));
        infos[1].value->val = (int)ptr;

        infos[2].arg_name = "MEM_SIZE";
        infos[2].type = TYPE_INT;
        infos[2].value = sal_malloc_notrace(sizeof(value_t));
        infos[2].value->val = size;

        infos[3].arg_name = "MALLOC_FUNC";
        infos[3].type = TYPE_STR;
        infos[3].value = sal_malloc_notrace(sizeof(value_t));
        infos[3].value->size = strlen(funcname);
        infos[3].value->bufp = sal_malloc_notrace(infos[3].value->size + 1);
        sprintf(infos[3].value->bufp, "%s", funcname);

        infos[4].arg_name = "MALLOC_LINE";
        infos[4].type = TYPE_INT;
        infos[4].value = sal_malloc_notrace(sizeof(value_t));
        infos[4].value->val = line;
    }

    pl_demo_post_data(OP_POST, infos, info_count, NULL, 0);

    for (idx = 0; idx < info_count; idx++)
    {
        if (infos[idx].value)
        {
            sal_free(infos[idx].value->bufp);
            sal_free(infos[idx].value);
        }
    }
    pub_bzero(infos, sizeof(infos));
}

static ret_msg_t show_meminfo(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    _print_memory_info(memoryinfo_print);

    return ret;
}
