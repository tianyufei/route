CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(SRV_PATH) -lyaya_common

PROCESS=logtool
TOOLIB=libyaya_logtoolib.so


include $(UTIL_MK)

compile:$(TOOLIB) $(PROCESS) 

$(TOOLIB):$(OBJ_PATH)/logtool.o
	$(CPP) -shared $(CPPFLAGS) -o $(LIB_PATH)/$@ $^ 

$(PROCESS):$(OBJ_PATH)/logtool_main.o
	$(CPP) $(CPPFLAGS) -lyaya_logtoolib -o $(BIN_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




