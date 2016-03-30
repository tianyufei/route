
source_file=$(strip $(wildcard *.c)) $(strip $(wildcard *.cpp))

ifneq ($(source_file),)

prefix_file_list_tmp=$(patsubst %.c,%.d,$(addprefix $(OBJ_PATH)/,$(source_file)))
prefix_file_list=$(patsubst %.cpp,%.d,$(prefix_file_list_tmp))


$(OBJ_PATH)/%.o: %.c
	$(CC) -c $(CPPFLAGS) -o $@ $< 

$(OBJ_PATH)/%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $< 

$(OBJ_PATH)/%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,${OBJ_PATH}/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJ_PATH)/%.d: %.cpp
	@set -e; rm -f $@; \
		$(CPP) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,${OBJ_PATH}/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(prefix_file_list)

endif


