/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stddef.h>
#include <string.h>

// Device-local selection, never a global by-name symlink replacement.
// KTM (PLQ110) carries its CN carrier NV in the active-slot partition.
static inline const char *ktm_static_nv_backing(const char *slot)
{
    if (!slot || (strcmp(slot, "_a") && strcmp(slot, "_b")))
        return NULL;
    return !strcmp(slot, "_a") ? "/dev/block/bootdevice/by-name/oplusstanvbk_a" :
                                "/dev/block/bootdevice/by-name/oplusstanvbk_b";
}
