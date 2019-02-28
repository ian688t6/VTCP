DIR_INC = inc
DIR_SRC = src

SRC := src/common/sock.c
SRC += src/common/timer.c
SRC += src/common/vtcp.c
SRC += src/common/vtcpmsg.c
SRC += src/tmng/tmng_reg.c
SRC += src/tmng/tmng_unreg.c
SRC += src/tmng/tmng_auth.c
SRC += src/tmng/tmng_hb.c

OBJ = $(patsubst %.c,%.o,${SRC}) 

TARGET = libvtcp.a
CFLAGS += -I$(DIR_INC) -Wall -static -std=c99 -DPACKAGE_VERSION="1.0" -D_GNU_SOURCE
LDFLAG := -lpthread

${TARGET}:${OBJ}
	$(AR) rc $@ $^ 
	@cp $(TARGET) $(VCP_BUILD)/lib
%.o:%.c
	$(CC) $(INC) $(CFLAGS) $(LDFLAG) -c  $< -o $@

.PHONY:clean
clean:
	@rm -rf ${OBJ}
	@rm -rf *.a
