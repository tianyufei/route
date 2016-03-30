
CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(SRV_PATH) -lyaya_common  $(JSON_FLAGS) 

PROCESS=portal
TOOLS=cfmtool
TOOLIB=libyaya_cfm.so


include $(UTIL_MK)

compile:$(TOOLIB) $(TOOLS) $(PROCESS)
	$(call dbus_config_file_create,$(PROCESS))

$(TOOLIB):$(OBJ_PATH)/cfm_contents.o
	$(CPP) $(CPPFLAGS) -shared -o $(LIB_PATH)/$@ $^ 


$(PROCESS):$(OBJ_PATH)/cfm.o 
	$(CPP) $(CPPFLAGS) -lyaya_cfm -o $(BIN_PATH)/$@ $^ 

$(TOOLS):$(OBJ_PATH)/cfmtool.o
	$(CPP) $(CPPFLAGS) -lyaya_cfm -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




