ROOT = $(abspath $(CURDIR))/build
MAKEOPTS += ROOT=$(ROOT)

LANGUAGE = og

.PHONY: all clean build-src examples test build-rts build-cdk
all: $(LANGUAGE)

$(LANGUAGE): build-src
	cp src/$(LANGUAGE) .
build-src: build-cdk build-rts
	$(MAKE) -C src $(MAKEOPTS) ast/all.h ast/visitor_decls.h # to work with multiple jobs
	$(MAKE) -C src $(MAKEOPTS) all

examples: build-src
	$(MAKE) -C examples $(MAKEOPTS) all

test: examples

clean:
	rm -rf build
	# rm -rf librts libcdk # commented out in case you want to do things offline
	$(MAKE) -C librts $(MAKEOPTS) clean
	$(MAKE) -C libcdk $(MAKEOPTS) clean
	$(MAKE) -C src $(MAKEOPTS) clean
	$(MAKE) -C examples $(MAKEOPTS) clean
	rm -f $(LANGUAGE)

build-rts: librts
	$(MAKE) -C librts $(MAKEOPTS) all
	$(MAKE) -C librts $(MAKEOPTS) install
librts:
	wget -O librts.tar.bz2 "https://web.tecnico.ulisboa.pt/~david.matos/w/pt/images/4/4d/Librts5-202002022020.tar.bz2"
	tar xf librts.tar.bz2
	rm librts.tar.bz2
	mv librts5-202002022020 librts

build-cdk: libcdk
	$(MAKE) -C libcdk $(MAKEOPTS) all
	$(MAKE) -C libcdk $(MAKEOPTS) install
libcdk:
	wget -O libcdk.tar.bz2 "https://web.tecnico.ulisboa.pt/~david.matos/w/pt/images/f/fc/Libcdk15-202002022020.tar.bz2"
	tar xf libcdk.tar.bz2
	rm libcdk.tar.bz2
	mv libcdk15-202002022020 libcdk
