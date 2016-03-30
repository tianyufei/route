#make files.
-include $(SCRIPT_FILE)

export UTIL_MK=$(ROOT_PATH)/tools/util.mk
export BUILD_PATH_MK=$(ROOT_PATH)/tools/build_path.mk
export BUILD_SOURCE_MK=$(ROOT_PATH)/tools/build_source.mk
export OBJ_PATH=$(OUT_PATH)/obj
export BIN_PATH=$(OUT_PATH)/usr/bin/yaya
export LIB_PATH=$(OUT_PATH)/lib
export INC_PATH=$(ROOT_PATH)/src/inc

ifeq ($(BUILD_TARGET),host)
export GCC=$(CC)
export CPP=g++
endif

export CPPFLAGS+=$(CFLAGS) -O2 -g -fPIC -Wall -std=gnu99 -I ./ -I $(ROOT_PATH)/src/inc
export DBUS_FLAGS=$(shell pkg-config --cflags dbus-1) $(shell pkg-config --libs dbus-1)
export SSL_FLAGS=$(shell pkg-config --cflags openssl) $(shell pkg-config --libs openssl)
#export JSON_FLAGS=$(shell pkg-config --cflags json) $(shell pkg-config --libs json)
export JSON_FLAGS=$(shell pkg-config --cflags json-c) $(shell pkg-config --libs json-c)



SUBDIRS=yos service

compile:
	@mkdir -p $(BIN_PATH)
	@mkdir -p $(LIB_PATH)
	@mkdir -p $(OBJ_PATH)
	@echo '#!/bin/sh' > $(BIN_PATH)/start_script.sh
	@echo '#!/bin/sh' > $(BIN_PATH)/stop_script.sh
	@chmod 777 $(BIN_PATH)/start_script.sh
	@chmod 777 $(BIN_PATH)/stop_script.sh
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(mkdir -p $(OBJ_PATH)/$$subdir); \
		(cd $$subdir && make OBJ_PATH=$(OBJ_PATH)/$$subdir ); \
	done;

clean:
	@rm $(OUT_PATH) -rf

install:


uninstall:


.PHONY: compile clean install uninstall






