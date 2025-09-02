/*
 * Copyright 2025 Alibaba Group Holding Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http: *www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "c_utils.h"
#include "tkl_mutex.h"
#include "tal_memory.h"
#include "tal_system.h"

#define MUTEX_WAIT_FOREVER 0x7FFFFFFF

// External log level variable
extern uint8_t g_util_log_lv;

/*****************************************************
 * Function: util_mutex_create
 * Description: 创建一个互斥锁对象。
 * Parameter: 无。
 * Return: util_mutex_t * --- 返回指向互斥锁结构体的指针。
 ****************************************************/
util_mutex_t *util_mutex_create(void)
{
    util_mutex_t *mutex = (util_mutex_t *)util_malloc(sizeof(util_mutex_t));
    if (mutex == NULL) {
        UTIL_LOG_E("util_mutex_create: memory allocation failed");
        return NULL;
    }

    TKL_MUTEX_HANDLE tkl_mutex_handle;
    OPERATE_RET ret = tkl_mutex_create_init(&tkl_mutex_handle);
    if (ret != OPRT_OK) {
        util_free(mutex);
        return NULL;
    }

    mutex->mutex_handle = (void *)tkl_mutex_handle;
    mutex->is_locked = false;
    return mutex;
}

/*****************************************************
 * Function: util_mutex_delete
 * Description: 删除指定的互斥锁对象。
 * Parameter:
 *     mutex --- 指向互斥锁结构体的指针。
 * Return: 无。
 ****************************************************/
void util_mutex_delete(util_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    if (mutex->mutex_handle != NULL) {
        if (mutex->is_locked) {
            tkl_mutex_unlock((TKL_MUTEX_HANDLE)mutex->mutex_handle);
        }
        tkl_mutex_release((TKL_MUTEX_HANDLE)mutex->mutex_handle);
    }

    util_free(mutex);
}

/*****************************************************
 * Function: util_mutex_lock
 * Description: 对指定的互斥锁进行加锁操作，带超时机制。
 * Parameter:
 *     mutex --- 指向互斥锁结构体的指针。
 *     timeout --- 加锁等待超时时间，单位为毫秒(ms)，可设为 MUTEX_WAIT_FOREVER 表示无限等待。
 * Return: int32_t --- 返回操作结果(util_result_t)。
 ****************************************************/
int32_t util_mutex_lock(util_mutex_t *mutex, int32_t timeout)
{
    UTIL_NULL_CHECK(mutex, UTIL_ERR_INVALID_PARAM);

    if (mutex->mutex_handle == NULL) {
        return UTIL_ERR_INVALID_PARAM;
    }

    if (mutex->is_locked) {
        return UTIL_ERR_ALREADY;
    }

    OPERATE_RET ret;
    if (timeout == MUTEX_WAIT_FOREVER) {
        ret = tkl_mutex_lock((TKL_MUTEX_HANDLE)mutex->mutex_handle);
    } else if (timeout <= 0) {
        ret = tkl_mutex_trylock((TKL_MUTEX_HANDLE)mutex->mutex_handle);
    } else {
        uint32_t start_time = tal_system_get_millisecond();
        uint32_t elapsed_time = 0;
        uint32_t poll_interval = (timeout < 10) ? 1 : (timeout / 10);

        if (poll_interval < 1)
            poll_interval = 1;
        if (poll_interval > 10)
            poll_interval = 10;

        while (elapsed_time < (uint32_t)timeout) {
            ret = tkl_mutex_trylock((TKL_MUTEX_HANDLE)mutex->mutex_handle);
            if (ret == OPRT_OK) {
                break;
            }
            tal_system_sleep(poll_interval);
            elapsed_time = tal_system_get_millisecond() - start_time;
            if (elapsed_time >= (uint32_t)timeout) {
                ret = OPRT_TIMEOUT;
                break;
            }
        }

        if (ret != OPRT_OK) {
            return UTIL_ERR_TIMEOUT;
        }
    }

    if (ret == OPRT_OK) {
        mutex->is_locked = true;
        return UTIL_SUCCESS;
    } else if (ret == OPRT_TIMEOUT) {
        return UTIL_ERR_TIMEOUT;
    } else {
        return UTIL_ERR_FAIL;
    }
}

/*****************************************************
 * Function: util_mutex_unlock
 * Description: 对指定的互斥锁进行解锁操作。
 * Parameter:
 *     mutex --- 指向互斥锁结构体的指针。
 * Return: int32_t --- 返回操作结果(util_result_t)。
 ****************************************************/
int32_t util_mutex_unlock(util_mutex_t *mutex)
{
    UTIL_NULL_CHECK(mutex, UTIL_ERR_INVALID_PARAM);

    if (mutex->mutex_handle == NULL) {
        return UTIL_ERR_INVALID_PARAM;
    }

    if (!mutex->is_locked) {
        return UTIL_ERR_ALREADY;
    }

    OPERATE_RET ret = tkl_mutex_unlock((TKL_MUTEX_HANDLE)mutex->mutex_handle);
    if (ret != OPRT_OK) {
        return UTIL_ERR_FAIL;
    }

    mutex->is_locked = false;
    return UTIL_SUCCESS;
}
