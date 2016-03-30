#make files.
GSOAP_TREE=gsoap-2.8
SUBDIRS=$(GSOAP_TREE)

ifeq ($(GSOAP_TREE),$(wildcard $(GSOAP_TREE))) 
define create_version
	@rm -rf $(1)
	@unzip gsoap_2.8.3.zip
endef
define clean_version
	@rm -rf $(1)
endef
else
define create_version
	@unzip gsoap_2.8.3.zip
endef
define clean_version
endef
endif

.PHONY: compile clean install uninstall

compile:
	$(call create_version,$(GSOAP_TREE))
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && libtoolize --automake --copy --debug --force && \
		./configure CFLAGS="-DWITH_COOKIES" && make); \
	done;

clean:
	$(call clean_version,$(GSOAP_TREE))
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && make clean); \
	done;

install:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && make install); \
	done;

uninstall:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && make uninstall); \
	done;

