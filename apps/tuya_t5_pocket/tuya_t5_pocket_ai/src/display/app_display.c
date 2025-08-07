/**
 * @file app_display.c
 * @brief Handle display initialization and message processing
 *
 * This source file provides the implementation for initializing the display system,
 * creating a message queue, and handling display messages in a separate task.
 * It includes functions to initialize the display, send messages to the display,
 * and manage the display task.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_memory.h"

#include "app_display.h"
#include "tuya_lvgl.h"

#include "lvgl.h"
#include "lv_vendor.h"

#include "ai_pocket_pet_app.h"

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TY_DISPLAY_TYPE_E type;
    int len;
    char *data;
} DISPLAY_MSG_T;

typedef struct {
    QUEUE_HANDLE  queue_hdl;
    THREAD_HANDLE thrd_hdl;

} TUYA_DISPLAY_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

static TUYA_DISPLAY_T sg_display = {0};

/***********************************************************
***********************function define**********************
***********************************************************/


static void __pocket_ui_task(void *args)
{
    // OPERATE_RET rt = OPRT_OK;
    DISPLAY_MSG_T msg_data = {0};

    (void)args;

    PR_DEBUG("ui init success");

    for (;;) {
        memset(&msg_data, 0, sizeof(DISPLAY_MSG_T));
        tal_queue_fetch(sg_display.queue_hdl, &msg_data, 0xFFFFFFFF);


        if (msg_data.data) {
            tkl_system_psram_free(msg_data.data);
        }
        msg_data.data = NULL;
    }
}

/**
 * @brief Initialize the display system
 *
 * @param None
 * @return OPERATE_RET Initialization result, OPRT_OK indicates success
 */
OPERATE_RET app_display_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    memset(&sg_display, 0, sizeof(TUYA_DISPLAY_T));

    lv_vendor_init(DISPLAY_NAME);

    lv_demo_ai_pocket_pet();

    lv_vendor_start();

    PR_DEBUG("lvgl init success");


    TUYA_CALL_ERR_RETURN(tal_queue_create_init(&sg_display.queue_hdl, sizeof(DISPLAY_MSG_T), 8));
    THREAD_CFG_T cfg = {
        .thrdname = "pocket_ui",
        .priority = THREAD_PRIO_2,
        .stackDepth = 1024 * 4,
    };
    TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&sg_display.thrd_hdl, NULL, NULL, __pocket_ui_task, NULL, &cfg));
    PR_DEBUG("pocket ui task create success");

    return rt;
}

/**
 * @brief Send display message to the display system
 *
 * @param tp Type of the display message
 * @param data Pointer to the message data
 * @param len Length of the message data
 * @return OPERATE_RET Result of sending the message, OPRT_OK indicates success
 */
OPERATE_RET app_display_send_msg(TY_DISPLAY_TYPE_E tp, uint8_t *data, int len)
{
    DISPLAY_MSG_T msg_data;

    msg_data.type = tp;
    msg_data.len = len;
    if (len && data != NULL) {
        msg_data.data = (char *)tkl_system_psram_malloc(len + 1);
        if (NULL == msg_data.data) {
            return OPRT_MALLOC_FAILED;
        }
        memcpy(msg_data.data, data, len);
        msg_data.data[len] = 0; //"\0"
    } else {
        msg_data.data = NULL;
    }

    tal_queue_post(sg_display.queue_hdl, &msg_data, 0xFFFFFFFF);

    return OPRT_OK;
}
