# Final object file
obj-m += msr_readwrite.o

# Path to our x86 build dir
KERNELDIR := /lib/modules/$(shell uname -r)/build/

# Tell make to take the architecture and cross-compiler in
#MAKEARCH := $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH := $(MAKE)

all: modules
modules:
	$(MAKEARCH) -C $(KERNELDIR) M=${shell pwd} modules

clean:
	$(MAKEARCH) -C $(KERNELDIR) M=${shell pwd} clean

install:
	cp *.ko /lib/modules/$(shell uname -r)/

