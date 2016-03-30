define dbus_config_file_create
	@echo "<!DOCTYPE busconfig PUBLIC" 					> com.yaya.$(1).conf
	@echo " \"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN\""		>> com.yaya.$(1).conf
	@echo " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">" 	>> com.yaya.$(1).conf
	@echo "<busconfig>"							>> com.yaya.$(1).conf
	@echo "	<policy context=\"default\">" 					>> com.yaya.$(1).conf
	@echo "		<allow own=\"com.yaya.$(1)\"/>"			>> com.yaya.$(1).conf
	@echo "		<allow send_destination=\"com.yaya.$(1)\"/>" 		>> com.yaya.$(1).conf
	@echo "	</policy>"							>> com.yaya.$(1).conf
	@echo "</busconfig>" 							>> com.yaya.$(1).conf
	@mkdir -p $(OUT_PATH)/etc/dbus-1/system.d
	@mv -f com.yaya.$(1).conf $(OUT_PATH)/etc/dbus-1/system.d
	@echo "$(1) &" 								>> $(BIN_PATH)/start_script.sh
	@echo  "killall -9 $(1)" 						>> $(BIN_PATH)/stop_script.sh
endef

define dbus_tool_file_create
	@echo "<!DOCTYPE busconfig PUBLIC" 					> com.yaya.$(1).conf
	@echo " \"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN\""		>> com.yaya.$(1).conf
	@echo " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">" 	>> com.yaya.$(1).conf
	@echo "<busconfig>"							>> com.yaya.$(1).conf
	@echo "	<policy context=\"default\">" 					>> com.yaya.$(1).conf
	@echo "		<allow own=\"com.yaya.$(1)\"/>"			>> com.yaya.$(1).conf
	@echo "		<allow send_destination=\"com.yaya.$(1)\"/>" 		>> com.yaya.$(1).conf
	@echo "	</policy>"							>> com.yaya.$(1).conf
	@echo "</busconfig>" 							>> com.yaya.$(1).conf
	@mkdir -p $(OUT_PATH)/etc/dbus-1/system.d
	@mv -f com.yaya.$(1).conf $(OUT_PATH)/etc/dbus-1/system.d
endef


define dbus_config_file_basic
	@echo "<!DOCTYPE busconfig PUBLIC" 					> com.yaya.conf
	@echo " \"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN\""		>> com.yaya.conf
	@echo " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">" 	>> com.yaya.conf
	@echo "<busconfig>"							>> com.yaya.conf
	@echo "	<policy context=\"default\">" 					>> com.yaya.conf
	@echo "		<allow own=\"com.yaya.yaya_log\"/>"		>> com.yaya.conf
	@echo "		<allow send_destination=\"com.yaya.yaya_log\"/>" 	>> com.yaya.conf
	@echo "		<allow own=\"com.yaya.yaya_cfm\"/>"		>> com.yaya.conf
	@echo "		<allow send_destination=\"com.yaya.yaya_cfm\"/>" 	>> com.yaya.conf
	@echo "		<allow own=\"com.yaya.yaya_diagnosis\"/>"		>> com.yaya.conf
	@echo "		<allow send_destination=\"com.yaya.yaya_diagnosis\"/>" 	>> com.yaya.conf
	@echo "		<allow own=\"com.yaya.uhttpd\"/>"			>> com.yaya.conf
	@echo "		<allow send_destination=\"com.yaya.uhttpd\"/>" 	>> com.yaya.conf
	@echo "	</policy>"							>> com.yaya.conf
	@echo "</busconfig>" 							>> com.yaya.conf
	@mkdir -p $(OUT_PATH)/etc/dbus-1/system.d
	@mv -f com.yaya.conf $(OUT_PATH)/etc/dbus-1/system.d
	@echo "$(1) &" 								>> $(BIN_PATH)/start_script.sh
	@echo  "killall -9 $(1)" 						>> $(BIN_PATH)/stop_script.sh
endef

