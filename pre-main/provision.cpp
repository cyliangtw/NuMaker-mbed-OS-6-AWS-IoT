/* 
 * Copyright (c) 2019 Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "mbedtls/config.h"
#include "entropy_poll.h"
//#include "psa/crypto.h"
#if DEVICE_FLASH
#include "kvstore_global_api.h"
#include "KVStore.h"
#include "TDBStore.h"
#include "KVMap.h"
#include "kv_config.h"
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

/* Simulate provision process for development
 *
 * 1. Reset kvstore
 * 2. Inject entropy seed (if no entropy source)
 * 3. Initialize user filesystem (if enabled)
 * 4. Mark the device as provisioned
 *
 * WARNING: For mass production, remove this file and run real provision process.
 */

/* Check weak reference/definition at the link:
 * http://www.keil.com/support/man/docs/ARMLINK/armlink_pge1362065917715.htm */

extern "C" {
    MBED_USED void provision(void);
}

/* Stringize */
#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#define _GET_FILESYSTEM_concat(dev, ...) _get_filesystem_##dev(__VA_ARGS__)
#define GET_FILESYSTEM(dev, ...) _GET_FILESYSTEM_concat(dev, __VA_ARGS__)

/* Key for the device provisioned */
#define KV_KEY_PROVISION    "provision"

void provision(void)
{
#if DEVICE_FLASH

    int kv_reset(const char *kvstore_path);
    
    /* Initialize kvstore */
    int kv_status = kv_init_storage_config();
    if (kv_status != MBED_SUCCESS) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "Initialize kvstore failed", kv_status);
    }

    /* Get kvstore internal storage */
    KVMap &kv_map = KVMap::get_instance();
    KVStore *inner_store = kv_map.get_internal_kv_instance(NULL);
    if (inner_store == NULL) {
        MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "kvstore internal storage failed");
    }

    /* Check if the device has provisioned */
    KVStore::info_t kv_info;
    kv_status = inner_store->get_info(KV_KEY_PROVISION, &kv_info);
    if (kv_status == MBED_SUCCESS) {
        do {
            /* Get KV_KEY_PROVISION key */
            char buffer[4];
            size_t actual_size = 0;
            int kv_status = inner_store->get(KV_KEY_PROVISION, buffer, sizeof(buffer), &actual_size);
            if (kv_status != MBED_SUCCESS) {
                printf("Get \'%s\' failed: %d\r\n", KV_KEY_PROVISION, kv_status);
                break;
            }
            /* Check KV_KEY_PROVISION key's value */
            if (actual_size != 1 || buffer[0] != '1') {
                printf("\"%s\" not equal \"%s\"\r\n", KV_KEY_PROVISION, "1");
                break;
            }

            printf("The device has provisioned. Skip provision process\r\n");
            return;
        } while (0);
    } else if (kv_status == MBED_ERROR_ITEM_NOT_FOUND) {
        /* Not provisioned yet */
        printf("The device has not provisioned yet. Try to provision it...\r\n");
    } else {
        printf("Get \'%s\' key failed: %d\r\n", KV_KEY_PROVISION, kv_status);
    }

    /* Provision from here */
    printf("Provision for development...\r\n");
    
    printf("Reset kvstore...\r\n");

    /* Reset kvstore for clean kvstore */
    kv_status = kv_reset("/" STR(MBED_CONF_STORAGE_DEFAULT_KV) "/");
    if (kv_status != MBED_SUCCESS) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "kv_reset() failed", kv_status);
    }

    printf("\rReset kvstore...OK\r\n");

#if !DEVICE_TRNG && !TARGET_PSA && !TARGET_PSA_Target
#if !defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
    /* Inject trivial seed for development */

    printf("Inject NV seed...\r\n");

    psa_status_t psa_status;
    uint8_t seed[SEED_SIZE] = { 0 };

    /* First inject seed, expect OK or seed has injected by some provision process */
    psa_status = mbedtls_psa_inject_entropy(seed, sizeof(seed));
    if (psa_status != PSA_SUCCESS && psa_status != PSA_ERROR_NOT_PERMITTED) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "Inject entropy failed", psa_status);
    }

    /* Second inject seed, expect seed has injected above or by some provision process */
    psa_status = mbedtls_psa_inject_entropy(seed, sizeof(seed));
    if (psa_status != PSA_ERROR_NOT_PERMITTED) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "Re-jnject entropy expects PSA_ERROR_NOT_PERMITTED", psa_status);
    }

    printf("\rInject NV seed...OK\r\n");
#endif  /* !defined(MBEDTLS_ENTROPY_HARDWARE_ALT) */
#endif  /* #if !DEVICE_TRNG && !TARGET_PSA && !TARGET_PSA_Target */

    /* Mark the device as provisioned */
    kv_status = inner_store->set(KV_KEY_PROVISION, "1", 1, KVStore::WRITE_ONCE_FLAG);
    if (kv_status != MBED_SUCCESS) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_UNKNOWN), "Mark the device as provisioned failed", kv_status);
    }

    printf("Provision for development...OK\r\n");

#endif  /* #if DEVICE_FLASH */
}
