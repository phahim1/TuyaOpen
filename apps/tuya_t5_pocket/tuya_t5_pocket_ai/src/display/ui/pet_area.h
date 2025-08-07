/**
 * @file pet_area.h
 * Pet Display Area Component for AI Pocket Pet
 */

#ifndef PET_AREA_H
#define PET_AREA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl/lvgl.h"
#include "ai_pocket_pet_app.h"

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create the pet display area with the pet sprite
 * @param parent Parent object to attach the pet area to
 * @return Pointer to the created pet area object
 */
lv_obj_t* pet_area_create(lv_obj_t *parent);

/**
 * Start the pet animation timers
 */
void pet_area_start_animation(void);

/**
 * Stop the pet animation timers
 */
void pet_area_stop_animation(void);

/**
 * Get the current pet state
 * @return Current pet state
 */
ai_pet_state_t pet_area_get_state(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PET_AREA_H */
