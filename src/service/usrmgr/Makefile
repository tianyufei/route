
CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(SRV_PATH) -lyaya_common -lyaya_apdb

PROCESS=usrmgr
TOOLS=usrdb
TOOLIB=libyaya_usrdb.so


include $(UTIL_MK)

compile:$(TOOLIB) $(PROCESS) $(TOOLS)
	$(call dbus_config_file_create,$(PROCESS))

$(PROCESS):$(OBJ_PATH)/usrmgr.o 
	$(CPP) $(CPPFLAGS) -lyaya_usrdb -o $(BIN_PATH)/$@ $^ 

$(TOOLS):$(OBJ_PATH)/user_db_tool.o
	$(CPP) $(CPPFLAGS) -lyaya_usrdb -o $(BIN_PATH)/$@ $^ 

$(TOOLIB):$(OBJ_PATH)/user_db.o
	$(CPP) $(CPPFLAGS) -shared -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




