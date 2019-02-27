include config.mk

.PHONY:all clean

MODULE := libvtcp
MODULES := $(addprefix libs/, $(MODULE))

define modules_build
          for module in $(MODULES) ; do \
               echo $${module} && cd $${module} && $(MAKE) && cd -; \
          done ;
endef

define modules_clean
          for module in $(MODULES) ; do \
               echo $${module} && cd $${module} && $(MAKE) clean && cd -; \
          done ;
endef

.PHONY:all clean cleanall
all:
	@echo "make build vtcp"	
	$(call modules_build)
	@make -C dvrd

clean:
	@echo "make clean vtcp"	
	$(call modules_clean)
	@make clean -C dvrd
cleanall:clean
	@rm -rf build

