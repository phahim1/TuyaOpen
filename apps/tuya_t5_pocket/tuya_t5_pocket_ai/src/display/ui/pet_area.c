/**
 * @file pet_area.c
 * Pet Display Area Component for AI Pocket Pet
 */

/*********************
 *      INCLUDES
 *********************/
#include "pet_area.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Pet animation
LV_IMG_DECLARE(ducky_walk);
LV_IMG_DECLARE(ducky_walk_to_left);
LV_IMG_DECLARE(ducky_blink);

/*********************
 *      DEFINES
 *********************/
#define STATUS_BAR_HEIGHT 24
#define BOTTOM_MENU_HEIGHT 26
#define PET_AREA_HEIGHT (AI_PET_SCREEN_HEIGHT - STATUS_BAR_HEIGHT - BOTTOM_MENU_HEIGHT)

// Pet animation constants
#define PET_ANIMATION_INTERVAL 20
#define PET_MOVEMENT_INTERVAL 50   // Natural movement timing
#define PET_MOVEMENT_STEP 2        // Smooth movement step
#define PET_MOVEMENT_LIMIT 80      // Movement boundaries
#define PET_BLINK_INTERVAL 3000    // Blink every 3 seconds
#define PET_WALK_DURATION_MIN 2000 // Minimum walk duration (ms)
#define PET_WALK_DURATION_MAX 8000 // Maximum walk duration (ms)
#define PET_IDLE_DURATION_MIN 3000 // Minimum idle duration (ms)
#define PET_IDLE_DURATION_MAX 10000 // Maximum idle duration (ms)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t *pet_area;
    lv_obj_t *pet_image_walk;
    lv_obj_t *pet_image_walk_left;
    lv_obj_t *pet_image_blink;
    lv_obj_t *current_pet_image; // Points to the currently active image

    lv_timer_t *pet_animation_timer;
    lv_timer_t *pet_movement_timer;

    // Pet movement state
    int16_t pet_x_pos;
    int8_t pet_direction;  // 1 = right, -1 = left
    uint32_t pet_state_timer;
    uint32_t pet_state_duration;
    bool pet_is_walking;
    ai_pet_state_t pet_state;
} pet_area_data_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void pet_animation_cb(lv_timer_t *timer);
static void pet_movement_cb(lv_timer_t *timer);
static void switch_pet_animation(lv_obj_t *new_animation);

/**********************
 *  STATIC VARIABLES
 **********************/
static pet_area_data_t g_pet_area_data;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t* pet_area_create(lv_obj_t *parent)
{
    pet_area_data_t *data = &g_pet_area_data;

    data->pet_area = lv_obj_create(parent);
    // Use full screen height minus status bar and bottom menu
    lv_obj_set_size(data->pet_area, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT - STATUS_BAR_HEIGHT - BOTTOM_MENU_HEIGHT);
    lv_obj_align(data->pet_area, LV_ALIGN_TOP_MID, 0, STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_opa(data->pet_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(data->pet_area, 0, 0);
    lv_obj_set_style_pad_all(data->pet_area, 0, 0); // Remove padding to maximize space

    // Disable scrolling for pet area
    lv_obj_clear_flag(data->pet_area, LV_OBJ_FLAG_SCROLLABLE);

    // Create a container for the GIF widgets to constrain rendering area
    lv_obj_t *gif_container = lv_obj_create(data->pet_area);
    lv_obj_set_size(gif_container, 159, 164); // Exact GIF dimensions
    lv_obj_align(gif_container, LV_ALIGN_CENTER, 0, -5); // Center with slight upward offset
    lv_obj_set_style_bg_opa(gif_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(gif_container, 0, 0);
    lv_obj_set_style_pad_all(gif_container, 0, 0);
    lv_obj_clear_flag(gif_container, LV_OBJ_FLAG_SCROLLABLE);

    // Create three separate GIF widgets for smooth animation transitions
    // This prevents black flashing by avoiding source switching

    // Walk right animation
    data->pet_image_walk = lv_gif_create(gif_container);
    lv_gif_set_src(data->pet_image_walk, &ducky_walk);
    lv_obj_align(data->pet_image_walk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(data->pet_image_walk, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(data->pet_image_walk, 159, 164);
    lv_obj_set_style_bg_opa(data->pet_image_walk, LV_OPA_TRANSP, 0);

    // Walk left animation
    data->pet_image_walk_left = lv_gif_create(gif_container);
    lv_gif_set_src(data->pet_image_walk_left, &ducky_walk_to_left);
    lv_obj_align(data->pet_image_walk_left, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(data->pet_image_walk_left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(data->pet_image_walk_left, 159, 164);
    lv_obj_set_style_bg_opa(data->pet_image_walk_left, LV_OPA_TRANSP, 0);

    // Blink animation
    data->pet_image_blink = lv_gif_create(gif_container);
    lv_gif_set_src(data->pet_image_blink, &ducky_blink);
    lv_obj_align(data->pet_image_blink, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(data->pet_image_blink, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(data->pet_image_blink, 159, 164);
    lv_obj_set_style_bg_opa(data->pet_image_blink, LV_OPA_TRANSP, 0);

    // Set initial active image and hide others
    data->current_pet_image = data->pet_image_blink;
    lv_obj_add_flag(data->pet_image_walk, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(data->pet_image_walk_left, LV_OBJ_FLAG_HIDDEN);

    // Initialize pet state
    data->pet_state = AI_PET_STATE_IDLE;
    data->pet_x_pos = 0;
    data->pet_direction = 1;  // Start facing right
    data->pet_is_walking = false;
    data->pet_state_timer = 0;
    data->pet_state_duration = PET_IDLE_DURATION_MIN + (rand() % (PET_IDLE_DURATION_MAX - PET_IDLE_DURATION_MIN));

    printf("Ducky GIF animation loaded - full resolution: %dx%d\n", ducky_walk.header.w, ducky_walk.header.h);

    return data->pet_area;
}

void pet_area_start_animation(void)
{
    pet_area_data_t *data = &g_pet_area_data;

    data->pet_animation_timer = lv_timer_create(pet_animation_cb, PET_ANIMATION_INTERVAL, data);
    data->pet_movement_timer = lv_timer_create(pet_movement_cb, PET_MOVEMENT_INTERVAL, data);
}

void pet_area_stop_animation(void)
{
    pet_area_data_t *data = &g_pet_area_data;

    if (data->pet_animation_timer) {
        lv_timer_del(data->pet_animation_timer);
        data->pet_animation_timer = NULL;
    }

    if (data->pet_movement_timer) {
        lv_timer_del(data->pet_movement_timer);
        data->pet_movement_timer = NULL;
    }
}

ai_pet_state_t pet_area_get_state(void)
{
    return g_pet_area_data.pet_state;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void pet_animation_cb(lv_timer_t *timer)
{
    pet_area_data_t *data = (pet_area_data_t *)lv_timer_get_user_data(timer);

    // The GIF animations are handled by the movement system
    // This callback can be used for additional pet state management

    // Ensure pet is always visible
    lv_obj_clear_flag(data->current_pet_image, LV_OBJ_FLAG_HIDDEN);

    // Update pet state based on movement system
    if (data->pet_is_walking) {
        data->pet_state = AI_PET_STATE_WALKING;
    } else {
        data->pet_state = AI_PET_STATE_IDLE;
    }
}

static void pet_movement_cb(lv_timer_t *timer)
{
    pet_area_data_t *data = (pet_area_data_t *)lv_timer_get_user_data(timer);

    // Update state timer
    data->pet_state_timer += PET_MOVEMENT_INTERVAL;

    // Check if it's time to change state
    if (data->pet_state_timer >= data->pet_state_duration) {
        // Switch between walking and idle
        data->pet_is_walking = !data->pet_is_walking;

        if (data->pet_is_walking) {
            // Start walking - choose random direction and duration
            data->pet_direction = (rand() % 2) ? 1 : -1;
            data->pet_state_duration = PET_WALK_DURATION_MIN + (rand() % (PET_WALK_DURATION_MAX - PET_WALK_DURATION_MIN));

            // Set appropriate animation based on direction
            if (data->pet_direction == 1) {
                switch_pet_animation(data->pet_image_walk);
            } else {
                switch_pet_animation(data->pet_image_walk_left);
            }
        } else {
            // Start idle - choose random duration
            data->pet_state_duration = PET_IDLE_DURATION_MIN + (rand() % (PET_IDLE_DURATION_MAX - PET_IDLE_DURATION_MIN));

            // Use blink animation when idle
            switch_pet_animation(data->pet_image_blink);
        }

        data->pet_state_timer = 0;
    }

    // Move pet if walking
    if (data->pet_is_walking) {
        data->pet_x_pos += data->pet_direction * PET_MOVEMENT_STEP;

        // Bounce off boundaries
        if (data->pet_x_pos > PET_MOVEMENT_LIMIT) {
            data->pet_x_pos = PET_MOVEMENT_LIMIT;
            data->pet_direction = -1;
            switch_pet_animation(data->pet_image_walk_left);
        } else if (data->pet_x_pos < -PET_MOVEMENT_LIMIT) {
            data->pet_x_pos = -PET_MOVEMENT_LIMIT;
            data->pet_direction = 1;
            switch_pet_animation(data->pet_image_walk);
        }
    } else {
        // Pet is idle - stays at current position
        // Animation is already set to blink in the state change logic
    }

    // Update pet position - move the container that holds the GIF widgets
    lv_obj_t *gif_container = lv_obj_get_parent(data->current_pet_image);
    if (gif_container) {
        lv_obj_set_x(gif_container, data->pet_x_pos);
    }
}

static void switch_pet_animation(lv_obj_t *new_animation)
{
    pet_area_data_t *data = &g_pet_area_data;

    // Hide current animation
    lv_obj_add_flag(data->current_pet_image, LV_OBJ_FLAG_HIDDEN);

    // Show new animation
    data->current_pet_image = new_animation;
    lv_obj_clear_flag(new_animation, LV_OBJ_FLAG_HIDDEN);
}
