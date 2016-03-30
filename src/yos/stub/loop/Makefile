
CPPFLAGS += -L$(LIB_PATH) -lyaya_common -I ./ -I $(YOS_PATH)

LOOP_SRC=$(addprefix $(OBJ_PATH)/,iloop_stub.o) 

LIBNAME=libyaya_loop.so

compile: $(LIBNAME)

$(LIBNAME):$(LOOP_SRC)
	$(CPP) -shared $(CPPFLAGS) -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)


