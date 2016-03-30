CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(YOS_PATH) -I $(INC_PATH) -lyaya_common


BINNAME=romenv

compile: $(BINNAME)

$(BINNAME):$(OBJ_PATH)/rom_env.o 
	$(CPP) $(CPPFLAGS) -o $(BIN_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)


