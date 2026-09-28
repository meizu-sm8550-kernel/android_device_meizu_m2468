#
# Copyright (C) 2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit common AOSP configurations
$(call inherit-product, build/make/target/product/full_base_telephony.mk)
$(call inherit-product, build/make/target/product/core_64_bit_only.mk)

# Inherit device-specific configurations
$(call inherit-product, device/meizu/m2468/device.mk)

# Inherit LineageOS configurations
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_DEVICE := m2468
PRODUCT_NAME := lineage_m2468
PRODUCT_BRAND := Meizu
PRODUCT_MODEL := Meizu 21 Note
PRODUCT_MANUFACTURER := Meizu

BUILD_FINGERPRINT := meizu/Meizu_21Note_CN/Meizu21Note:15/AQ3A.241229.001/1737086932:user/release-keys