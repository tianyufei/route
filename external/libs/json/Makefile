#make files.
TREE=json-c-0.11
SUBDIRS=$(TREE)

.PHONY: compile clean install uninstall

compile:
	@if [ ! -d $(TREE) ] ; then \
		tar -zxvf $(TREE).tar.gz; \
		cd $(TREE); \
		autoreconf -ivf; \
		libtoolize --automake --copy --debug --force; \
		./configure CFLAGS="-DWITH_COOKIES"; \
		make; \
		sudo make install; \
	fi;

clean:
	@rm -rf $(TREE);
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

