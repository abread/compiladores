ROOT = $(abspath $(CURDIR))/build
MAKEOPTS += ROOT=$(ROOT)

LANGUAGE = og

.PHONY:

all: $(LANGUAGE) .PHONY

$(LANGUAGE): src/$(LANGUAGE) .PHONY
	ln -sf src/$(LANGUAGE) .

src/$(LANGUAGE): build-cdk build-rts .PHONY
	#$(MAKE) -C src $(MAKEOPTS) ast/all.h ast/visitor_decls.h # to work with multiple jobs
	$(MAKE) -C src $(MAKEOPTS) ast/all.h ast/visitor_decls.h all

examples: $(LANGUAGE) .PHONY
	$(MAKE) -C examples $(MAKEOPTS) all

test: examples .PHONY
	$(MAKE) -C tests $(MAKEOPTS) test

clean: .PHONY
	rm -rf build
	# rm -rf librts libcdk # commented out in case you want to do things offline
	$(MAKE) -C librts $(MAKEOPTS) clean
	$(MAKE) -C libcdk $(MAKEOPTS) clean
	$(MAKE) -C src $(MAKEOPTS) clean
	$(MAKE) -C examples $(MAKEOPTS) clean
	$(MAKE) -C tests $(MAKEOPTS) clean
	rm -f $(LANGUAGE)

build-rts: librts .PHONY
	$(MAKE) -C librts $(MAKEOPTS) all
	$(MAKE) -C librts $(MAKEOPTS) install
librts:
	wget -O librts.tar.bz2 "https://web.tecnico.ulisboa.pt/~david.matos/w/pt/images/4/4d/Librts5-202002022020.tar.bz2"
	tar xf librts.tar.bz2
	rm librts.tar.bz2
	mv librts5-202002022020 librts

build-cdk: libcdk .PHONY
	$(MAKE) -C libcdk $(MAKEOPTS) all
	$(MAKE) -C libcdk $(MAKEOPTS) install
libcdk:
	wget -O libcdk.tar.bz2 "https://web.tecnico.ulisboa.pt/~david.matos/w/pt/images/f/fc/Libcdk15-202002022020.tar.bz2"
	tar xf libcdk.tar.bz2
	rm libcdk.tar.bz2
	mv libcdk15-202002022020 libcdk
