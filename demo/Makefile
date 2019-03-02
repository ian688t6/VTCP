DIR_INC = inc
DIR_SRC = src

SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_SRC}/%.o,$(notdir ${SRC})) 

CFLAGS +=  -I./inc -I../libs/libvtcp/inc
ifeq ($(ARCH), x86)
LDFLAG := -L$(VCP_BUILD)/lib -lvtcp -lpthread
else
LDFLAG := -static -Wl,-dn -L$(VCP_BUILD)/lib -lvtcp -lm -lrt
endif

TARGET = demo

${TARGET}:${OBJ}
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAG) 
	@cp $@ $(VCP_BUILD)/bin

${DIR_SRC}/%.o:${DIR_SRC}/%.c
	$(CC) -c $(INC) $(CFLAGS) $< -o $@ 

.PHONY:clean
clean:
	@rm -rf $(DIR_SRC)/*.o
	@rm $(TARGET)
