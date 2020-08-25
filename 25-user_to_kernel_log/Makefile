obj-m += utkl.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

PHONY += help
help:
	@echo  'make        - Compile module in current directory;'
	@echo  'make clean  - Remove generated files in current directory. Only the source .c file will remain;'
	@echo  'make help   - Print this message.'
