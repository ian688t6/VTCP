include config.mk

.PHONY:all clean cleanall
all:
	@echo "make build vtcp"	
	@make -C libs/libvtcp
	@make -C demo

clean:
	@echo "make clean vtcp"	
	@make clean -C libs/libvtcp
	@make clean -C demo
cleanall:clean
	@rm -rf build

