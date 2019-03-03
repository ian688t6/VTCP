$(shell mkdir -p build/include)
$(shell mkdir -p build/lib)
$(shell mkdir -p build/bin)


export VCP_TOP	:= $(shell pwd)
export VCP_BUILD := $(VCP_TOP)/build
export CROSS_COMPILE ?=
export ARCH ?= x86
export HOST ?=
export CC := $(CROSS_COMPILE)gcc
export AR := $(CROSS_COMPILE)ar
export LD := $(CROSS_COMPILE)ld
export STRIP := $(CROSS_COMPILE)strip
export OBJDUMP := $(CROSS_COMPILE)objdump
export NM := $(CROSS_COMPILE)nm

export CFLAGS = -Os -Wall -Werror
export INC := 

