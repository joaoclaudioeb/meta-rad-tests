# @file Makefile
# @brief Makefile for the ADS1256 driver.
# @details This code was based on a Makefile template for out-of-tree modules (https://community.nxp.com/t5/i-MX-Processors-Knowledge-Base/Incorporating-Out-of-Tree-Modules-in-YOCTO/ta-p/1373825?profile.language=en)
# @author Carlos Augusto Porto Freitas
# @author João Cláudio Elsen Barcellos
# @version 0.1.0
# @date 12/06/2026

obj-m += ads1256.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules

install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean
