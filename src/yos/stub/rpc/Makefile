
CPPFLAGS += -L$(LIB_PATH) -lyaya_common -I ./ -I $(YOS_PATH) $(DBUS_FLAGS)

LOOP_SRC=$(addprefix $(OBJ_PATH)/,irpc_dbus.o irpc_dbus_wrapper.o) 

LIBNAME=libyaya_rpc.so

compile: $(LIBNAME)

$(LIBNAME):$(LOOP_SRC)
	$(CPP) -shared $(CPPFLAGS) -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)


