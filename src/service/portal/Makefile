
CPPFLAGS += -L$(LIB_PATH) -I ./ -I $(SRV_PATH) -lyaya_common -lyaya_usrdb

PROCESS=portal
TOOLIB=libyaya_weblist.so


include $(UTIL_MK)

compile:$(TOOLIB) $(PROCESS)
	$(call dbus_config_file_create,$(PROCESS))

$(PROCESS):$(OBJ_PATH)/portal.o 
	$(CPP) $(CPPFLAGS) -o $(BIN_PATH)/$@ $^ 

$(TOOLIB):$(OBJ_PATH)/weblist.o
	$(CPP) $(CPPFLAGS) -lyaya_logtoolib -shared -o $(LIB_PATH)/$@ $^ 

include $(BUILD_SOURCE_MK)




