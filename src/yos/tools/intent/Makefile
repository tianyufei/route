
CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(YOS_PATH) -I $(INC_PATH) -lyaya_common  


BINNAME=intent

include $(UTIL_MK)

compile: $(BINNAME)
	$(call dbus_tool_file_create,$(BINNAME))

$(BINNAME):$(OBJ_PATH)/intent.o 
	$(CPP) $(CPPFLAGS) -o $(BIN_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




