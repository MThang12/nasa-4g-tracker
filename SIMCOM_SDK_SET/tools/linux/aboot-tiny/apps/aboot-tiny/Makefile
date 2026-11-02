CONTIKI_PROJECT = libaboot-tiny.so adownload-tiny
CONTIKI = ../..

CONTIKI_SOURCEFILES += libaboot-tiny.c aboot-callback.c aboot-transport.c
CONTIKI_SOURCEFILES += preamble.c aboot-download.c alog.c udp-transport.c
CONTIKI_SOURCEFILES += at-fallback.c aboot-hotplug.c DiagServer.c

all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_NULLMAC
MAKE_NET = MAKE_NET_IPV6
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING

CONTIKI_VERSION_OVERRIDE ?= ${shell git --git-dir ${CONTIKI}/.git --work-tree ${CONTIKI} describe --tags --always --dirty}
RELSTR := $(CONTIKI_VERSION_OVERRIDE)

include $(CONTIKI)/Makefile.include
