# Official LineageOS 23.2 custom DTBO hook.
# Reuse the six DTB_OUT payloads; preserve Note entry and qcom,msm-id order.
MKDTBOIMG := $(HOST_OUT_EXECUTABLES)/mkdtboimg$(HOST_EXECUTABLE_SUFFIX)
$(BOARD_PREBUILT_DTBOIMAGE): $(INSTALLED_DTBIMAGE_TARGET) $(MKDTBOIMG) $(BOARD_DTBO_CFG)
	mkdir -p $(@D)
	$(MKDTBOIMG) cfg_create $@ $(BOARD_DTBO_CFG) -d $(DTB_OUT)/arch/$(KERNEL_ARCH)/boot/dts
