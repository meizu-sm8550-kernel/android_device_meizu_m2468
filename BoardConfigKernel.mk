# Generated from the current verified source inventory; see integration-status.json.
# Official LineageOS 23.2 consumers are pinned in source-spec.json.
TARGET_KERNEL_VERSION := 5.15
TARGET_KERNEL_SOURCE := kernel/meizu/sm8550
TARGET_KERNEL_CONFIG := gki_defconfig vendor/kalama_GKI.config vendor/m2468_GKI.config
TARGET_KERNEL_ADDITIONAL_FLAGS += M2468_DTBS=1 LLVM=1 LLVM_IAS=1 KCFLAGS=-D__ANDROID_COMMON_KERNEL__
TARGET_KERNEL_ADDITIONAL_FLAGS += TARGET_BOARD_PLATFORM=kalama
TARGET_KERNEL_ADDITIONAL_FLAGS += CONFIG_TOUCHSCREEN_NT36XXX_I2C=n CONFIG_TOUCHSCREEN_ATMEL_MXT=n CONFIG_TOUCHSCREEN_DUMMY=n
TARGET_KERNEL_EXT_MODULE_ROOT := kernel/meizu/sm8550-modules
TARGET_KERNEL_EXT_MODULES := qcom/opensource/mmrm-driver \
    qcom/opensource/securemsm-kernel \
    qcom/opensource/mm-drivers/hw_fence \
    qcom/opensource/mm-drivers/msm_ext_display \
    qcom/opensource/mm-drivers/sync_fence \
    qcom/opensource/audio-kernel \
    qcom/opensource/camera-kernel \
    qcom/opensource/dataipa/drivers/platform/msm \
    qcom/opensource/datarmnet/core \
    qcom/opensource/datarmnet-ext/aps \
    qcom/opensource/datarmnet-ext/offload \
    qcom/opensource/datarmnet-ext/shs \
    qcom/opensource/datarmnet-ext/perf \
    qcom/opensource/datarmnet-ext/perf_tether \
    qcom/opensource/datarmnet-ext/sch \
    qcom/opensource/datarmnet-ext/wlan \
    qcom/opensource/display-drivers/msm \
    qcom/opensource/eva-kernel \
    qcom/opensource/video-driver \
    qcom/opensource/graphics-kernel \
    qcom/opensource/touch-drivers \
    qcom/opensource/wlan/platform \
    qcom/opensource/wlan/qcacld-3.0/.kiwi_v2 \
    qcom/opensource/bt-kernel

# Source image/module selection consumed by official LineageOS 23.2.
BOARD_USES_QCOM_MERGE_DTBS_SCRIPT := false
TARGET_DTB_LIST_WILDCARD := vendor/qcom/m2468
TARGET_NEEDS_DTBOIMAGE := true
BOARD_CUSTOM_DTBOIMG_MK := $(DEVICE_PATH)/dtbo.mk
BOARD_DTBO_CFG := $(DEVICE_PATH)/dtbo.cfg
BOARD_USES_SYSTEM_DLKMIMAGE := true
BOARD_USES_VENDOR_DLKMIMAGE := true

m2468_first_stage := $(strip $(shell cat $(DEVICE_PATH)/modules/modules.list.first_stage))
m2468_second_stage := $(strip $(shell cat $(DEVICE_PATH)/modules/modules.list.second_stage))
m2468_vendor_dlkm := $(strip $(shell cat $(DEVICE_PATH)/modules/modules.list.vendor_dlkm))
m2468_system_dlkm := $(strip $(shell cat $(DEVICE_PATH)/modules/modules.list.system_dlkm))
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD := $(m2468_first_stage)
BOARD_VENDOR_RAMDISK_RECOVERY_KERNEL_MODULES_LOAD := $(m2468_first_stage) $(m2468_second_stage)
BOARD_VENDOR_KERNEL_MODULES_LOAD := $(m2468_second_stage) $(m2468_vendor_dlkm)
BOARD_SYSTEM_KERNEL_MODULES_LOAD := $(m2468_system_dlkm)
BOOT_KERNEL_MODULES := $(m2468_first_stage) $(m2468_second_stage)
SYSTEM_KERNEL_MODULES := $(m2468_system_dlkm)
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_BLOCKLIST_FILE := $(DEVICE_PATH)/modules/modules.blocklist.vendor
BOARD_VENDOR_KERNEL_MODULES_BLOCKLIST_FILE := $(DEVICE_PATH)/modules/modules.blocklist.vendor
BOARD_SYSTEM_KERNEL_MODULES_BLOCKLIST_FILE := $(DEVICE_PATH)/modules/modules.blocklist.system
# Existing Note partition sizes, EROFS settings, vbmeta chain and fstab remain in BoardConfig.mk.
