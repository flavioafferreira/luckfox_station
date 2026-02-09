obj-m        += prog_station.o
prog_station-objs := station.o /special/fifo.o

KDIR := /home/flavio/sdk/luckfox-pico/sysdrv/source/objs_kernel
PWD  := $(shell pwd)

ccflags-y += -I$(PWD)/special

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-rockchip830-linux-uclibcgnueabihf- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
