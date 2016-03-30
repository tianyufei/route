CPPFLAGS += $(SSL_FLAGS) -L$(LIB_PATH) -I ./ -I $(YOS_PATH) -I $(INC_PATH) -ldl

ifeq ($(BUILD_TARGET),target)
CPPFLAGS += -luci
endif

LIBNAME=libyaya_common.so

COMMON_SRC=$(addprefix $(OBJ_PATH)/,sysenv.o tools.o dllmgr.o icontext_stub.o) 

compile: $(LIBNAME)

$(LIBNAME):$(COMMON_SRC) 
	$(CPP) -shared $(CPPFLAGS) -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




