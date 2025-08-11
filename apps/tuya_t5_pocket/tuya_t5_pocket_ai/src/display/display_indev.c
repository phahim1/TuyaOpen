/**
 * @file pocket_button.c
 * @version 0.1
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "tdl_button_manage.h"
#include "tdl_joystick_manage.h"
#include "ai_pocket_pet_app.h"
#include "lv_vendor.h"
/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    char *name;
    TDL_BUTTON_TOUCH_EVENT_E event;
    uint32_t lv_key_code;
}BUTTON_CODE_MAP_T;

BUTTON_CODE_MAP_T button_code_map[] = {
    {"btn_enter", TDL_BUTTON_PRESS_DOWN, KEY_ENTER},
    {"btn_esc",   TDL_BUTTON_PRESS_DOWN, KEY_ESC},
};

typedef struct {
    TDL_JOYSTICK_TOUCH_EVENT_E event;
    uint32_t lv_key_code;
}JOYSTICK_CODE_MAP_T;

JOYSTICK_CODE_MAP_T joystick_code_map[] = {
    { TDL_JOYSTICK_UP, KEY_UP},
    { TDL_JOYSTICK_DOWN, KEY_DOWN},
    { TDL_JOYSTICK_LEFT, KEY_LEFT},
    { TDL_JOYSTICK_RIGHT, KEY_RIGHT},
};


/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/

static void __button_function_cb(char *name, TDL_BUTTON_TOUCH_EVENT_E event, void *argc)
{
    uint32_t i;
    (void)argc;

    for (i = 0; i < sizeof(button_code_map) / sizeof(button_code_map[0]); i++) {
        if (strcmp(name, button_code_map[i].name) == 0 && event == button_code_map[i].event) {
            PR_DEBUG("Button pressed: %s, event: %d, key code: %d", name, event, button_code_map[i].lv_key_code);

            lv_vendor_disp_lock();
            lv_demo_ai_pocket_pet_handle_input(button_code_map[i].lv_key_code);
            lv_vendor_disp_unlock();

            break;
        }
    }
}

static void __stick_function_cb(char *name, TDL_JOYSTICK_TOUCH_EVENT_E event, void *argc)
{
    uint32_t i;
    (void)argc;

     for (i = 0; i < sizeof(joystick_code_map) / sizeof(joystick_code_map[0]); i++) {
         if (event == joystick_code_map[i].event) {
             PR_DEBUG("joystick event: %d, key code: %d", event, joystick_code_map[i].lv_key_code);

             lv_vendor_disp_lock();
             lv_demo_ai_pocket_pet_handle_input(joystick_code_map[i].lv_key_code);
             lv_vendor_disp_unlock();

             break;
         }
     }
}

void pocket_pet_button_init(void)
{
    // button create
    TDL_BUTTON_CFG_T button_cfg = {.long_start_valid_time = 3000,
                                   .long_keep_timer = 1000,
                                   .button_debounce_time = 50,
                                   .button_repeat_valid_count = 2,
                                   .button_repeat_valid_time = 500};
    TDL_BUTTON_HANDLE button_hdl = NULL;
    tdl_button_set_task_stack_size(4096);  // debug by huatuo, set button task stack size to 4K
    for(uint32_t i=0; i<CNTSOF(button_code_map); i++) {
        tdl_button_create(button_code_map[i].name, &button_cfg, &button_hdl);

        tdl_button_event_register(button_hdl, button_code_map[i].event, __button_function_cb);
    }

    TDL_JOYSTICK_CFG_T joystick_cfg = {
        .button_cfg = {.long_start_valid_time = 3000,
                       .long_keep_timer = 1000,
                       .button_debounce_time = 50,
                       .button_repeat_valid_count = 2,
                       .button_repeat_valid_time = 500},
        .adc_cfg =
            {
                .adc_max_val = 8192,        /* adc max value */
                .adc_min_val = 0,           /* adc min value */
                .normalized_range = 10,     /* normalized range ±10 */
                .sensitivity = 2,           /* sensitivity should < normalized range */
            },
    };

    TDL_JOYSTICK_HANDLE sg_joystick_hdl = NULL;

    tdl_joystick_create(JOYSTICK_NAME, &joystick_cfg, &sg_joystick_hdl);

    for(uint32_t i=0; i<CNTSOF(joystick_code_map); i++) {
        tdl_joystick_event_register(sg_joystick_hdl, joystick_code_map[i].event, __stick_function_cb);
    }
}
