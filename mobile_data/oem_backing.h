/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stddef.h>
#include <string.h>

// Device-local selection, never a global by-name symlink replacement.
// Ace 6 (project 24851) has no bundled static-NV image: the modem reads the
// active-slot oplusstanvbk partition (verified: 20 MiB, FEMNVBK header).
static inline const char *ktm_static_nv_backing(const char *slot)
{
    if (!slot || (strcmp(slot, "_a") && strcmp(slot, "_b")))
        return NULL;
    return !strcmp(slot, "_a") ? "/dev/block/bootdevice/by-name/oplusstanvbk_a" :
                                "/dev/block/bootdevice/by-name/oplusstanvbk_b";
}
