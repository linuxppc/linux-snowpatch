#This checks for any specific ENV variables missing and add those.

ifeq ($(GIT_VERSION),)
GIT_VERSION = $(shell git describe --always --long --dirty || echo "unknown")
endif

ifeq ($(CFLAGS),)
CFLAGS := -std=gnu99 -O2 -Wall -Werror -DGIT_VERSION='"$(GIT_VERSION)"' -I$(CURDIR)/../include $(CFLAGS)
export CFLAGS
endif
