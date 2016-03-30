
CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(SRV_PATH) -lyaya_common

#PROCESS=usrmgr
#TOOLS=usrdb
TOOLIB=libyaya_apdb.so


include $(UTIL_MK)

#compile:$(PROCESS) $(TOOLS) $(TOOLIB)
# 	$(call dbus_config_file_create,$(PROCESS))

#$(PROCESS):$(OBJ_PATH)/usrmgr.o $(OBJ_PATH)/user_db_tool.o
#	$(CPP) $(CPPFLAGS) -o $(BIN_PATH)/$@ $^ 

#$(TOOLS):$(OBJ_PATH)/usr_db.o $(OBJ_PATH)/user_db_tool.o
#	$(CPP) $(CPPFLAGS) -o $(BIN_PATH)/$@ $^ 

$(TOOLIB):$(OBJ_PATH)/ap_db.o
	$(CPP) $(CPPFLAGS) -shared -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




