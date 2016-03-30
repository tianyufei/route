#make files.
-include $(SCRIPT_FILE)

BUILD_ROOT_PATH=$(ROOT_PATH)/workdir/build/$(Y_PRODUCT_NAME)
PATCH_COMMON_PATH=$(ROOT_PATH)/patch/$(Y_HARDWARE_CLASS)-$(Y_PLATFORM_TYPE)-$(Y_PLATFORM_VERSION)/common
PATCH_PRODUCT_PATH=$(ROOT_PATH)/patch/$(Y_HARDWARE_CLASS)-$(Y_PLATFORM_TYPE)-$(Y_PLATFORM_VERSION)/$(Y_PRODUCT_NAME)
TAR_FILE=$(ROOT_PATH)/workdir/source/$(Y_PLATFORM_TYPE)-$(Y_PLATFORM_VERSION).tar.gz
DLL_PATH=$(ROOT_PATH)/workdir/dl
TOOLBIN_PATH=$(ROOT_PATH)/tools/bin

.PHONY: compile clean

compile:
	mkdir -p $(ROOT_PATH)/workdir/build
	chmod 777 create-workspace.sh
	chmod 777 patch-workspace.sh
	./create-workspace.sh $(TAR_FILE) $(BUILD_ROOT_PATH) $(DLL_PATH)
	./patch-workspace.sh $(PATCH_COMMON_PATH) $(BUILD_ROOT_PATH) $(TOOLBIN_PATH)
	./patch-workspace.sh $(PATCH_PRODUCT_PATH) $(BUILD_ROOT_PATH) $(TOOLBIN_PATH)
#	$(TOOLBIN_PATH)/mfile $(BUILD_ROOT_PATH)/wrt_config $(BUILD_ROOT_PATH)/.config
	rm -rf $(BUILD_ROOT_PATH)/files
	ln -s $(BUILD_ROOT_PATH)/package/base-files/files $(BUILD_ROOT_PATH)/files
	echo 'export CONFIG_YAYA_SRC_TREE='$(ROOT_PATH)/src > $(BUILD_ROOT_PATH)/platform_env.mk
	echo 'export CONFIG_YAYA_OUT_TREE='$(ROOT_PATH)/out >> $(BUILD_ROOT_PATH)/platform_env.mk
	echo 'export OUT_PATH='$(OUT_PATH) >> $(BUILD_ROOT_PATH)/platform_env.mk
	echo 'export ROOT_PATH='$(ROOT_PATH) >> $(BUILD_ROOT_PATH)/platform_env.mk
	echo 'export BIN_PATH='$(OUT_PATH)/usr/bin/yaya >> $(BUILD_ROOT_PATH)/platform_env.mk
	echo 'export BUILD_TARGET='$(BUILD_TARGET) >> $(BUILD_ROOT_PATH)/platform_env.mk
	cp -rf $(SCRIPT_FILE) $(BUILD_ROOT_PATH)/platform_config.mk
	
clean:

