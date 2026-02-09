obj-m        += lstation.o
lstation-objs := station.o fifo.o

KDIR := /home/flavio/sdk/luckfox-pico/sysdrv/source/objs_kernel
PWD  := $(shell pwd)


all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-rockchip830-linux-uclibcgnueabihf- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
