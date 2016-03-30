
CPPFLAGS += -L$(LIB_PATH) -lyaya_common -I ./ -I $(YOS_PATH)

LOOP_SRC=$(addprefix $(OBJ_PATH)/,ievent_stub.o) 

LIBNAME=libyaya_event.so

compile: $(LIBNAME)

$(LIBNAME):$(LOOP_SRC)
	$(CPP) -shared $(CPPFLAGS) -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)


