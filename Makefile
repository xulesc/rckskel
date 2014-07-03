SHELL=sh

TRGTS = rckskel.a test.bin

dox: 
	@echo make documentation
	doxygen

clean: 
	@echo clean documentation
	rm -rf docs/*
	cd ./src ; make clean
	cd ./test ; make clean

all: $(TRGTS)

rckskel.a:
	@echo make library
	cd ./src ; make 
	
test.bin:
	@echo make test
	cd ./test ; make