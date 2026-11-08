/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stddef.h>
#include <string.h>

#define KTM_CN_NV_PATH "/system/etc/aera/mobile/23821/oplusstanvbk.img"
#define KTM_CN_NV_SIZE 1110016

// Device-local selection, never a global by-name symlink replacement.
static inline const char *ktm_static_nv_backing(const char *project, const char *slot)
{
    if (!slot || (strcmp(slot, "_a") && strcmp(slot, "_b")))
        return NULL;
    if (project && !strcmp(project, "23821"))
        return KTM_CN_NV_PATH;
    return !strcmp(slot, "_a") ? "/dev/block/bootdevice/by-name/oplusstanvbk_a" :
                                "/dev/block/bootdevice/by-name/oplusstanvbk_b";
}
