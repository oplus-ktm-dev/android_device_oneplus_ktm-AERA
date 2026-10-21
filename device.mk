#
# Copyright (C) 2025 The Android Open Source Project
# Copyright (C) 2025 SebaUbuntu's TWRP device tree generator
#
# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2024 The OrangeFox Recovery Project
# SPDX-License-Identifier: GPL-3.0-or-later
#

LOCAL_PATH := device/oneplus/ktm

# Shipping API level
BOARD_SHIPPING_API_LEVEL := 34
PRODUCT_SHIPPING_API_LEVEL := 34
PRODUCT_TARGET_VNDK_VERSION := 34

# Dynamic partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true

ifeq ($(AERA_ENABLE_MOBILE_DATA),1)
PRODUCT_PACKAGES += aera-cellular-provider
endif

PRODUCT_PACKAGES += \
    aera-audio-bridge \
    aera-audio-service \
    aera-browser-jail \
    lpflash \
    lpmake \
    lpunpack

# KTM/OnePlus Ace 6 Adreno 830 backend. The ABI-matched KGSL module, Gen8
# firmware, EGL/GLES userspace, and minimal mapper closure stay device-local;
# generic AERA remains portable and retains its software fallback.
PRODUCT_VENDOR_PROPERTIES += \
    ro.hardware.egl=adreno

# OTA certs
PRODUCT_EXTRA_RECOVERY_KEYS += \
	$(LOCAL_PATH)/security/local_OTA \
	$(LOCAL_PATH)/security/special_OTA

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += $(LOCAL_PATH)

# AERA-specific settings
$(call inherit-product, $(LOCAL_PATH)/aera_ktm.mk)
#
