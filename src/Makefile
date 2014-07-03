SHELL=sh
RCCEROOT=../RCCE_V1.0.13
include $(RCCEROOT)/common/symbols
verbose=NO
RCCE_EMULATOR_ARCHIVE=$(RCCEROOT)/bin/SCC_LINUX/libRCCE_bigflags_nongory_nopwrmgmt.a
LFLAGS=-lm
RCKSKEL_FILES=rckskel.c rckskel_data_parallel.c rckskel_resolution.c rckskel_task_parallel.c utils.c rckskel_datatypes.c
RCKSKEL_OBJS=rckskel.o rckskel_data_parallel.o rckskel_task_parallel.o rckskel_resolution.o utils.o rckskel_datatypes.o
RCKSKEL_ARCH=rckskel.a
RCKSKEL_CFLAGS=-g -I$(RCCEINCLUDE)

$(RCKSKEL_ARCH): $(RCKSKEL_OBJS)
	@echo Archive name = $(RCKSKEL_ARCH)
	ar -r $(RCKSKEL_ARCH) $(RCKSKEL_OBJS)

rckskel.o: rckskel.c rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) rckskel.c 

rckskel_data_parallel.o: rckskel_data_parallel.c rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) rckskel_data_parallel.c

rckskel_resolution.o: rckskel_resolution.c rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) rckskel_resolution.c

rckskel_task_parallel.o: rckskel_task_parallel.c rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) rckskel_task_parallel.c

rckskel_datatypes.o: rckskel_datatypes.c rckskel_datatypes.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) rckskel_datatypes.c

utils.o: utils.c rckskel.h
	$(CCOMPILE) -c $(RCKSKEL_CFLAGS) utils.c

clean:
	rm *.o *.a

