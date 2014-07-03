SHELL=sh
RCCEROOT=../RCCE_V1.0.13
include $(RCCEROOT)/common/symbols
verbose=NO
RCCE_EMULATOR_ARCHIVE=$(RCCEROOT)/bin/OMP/libRCCE_bigflags_nongory_nopwrmgmt.a
LFLAGS=-lm
RCKSKEL_FILES=rckskel.c rckskel_data_parallel.c rckskel_resolution.c rckskel_task_parallel.c utils.c rckskel_datatypes.c
RCKSKEL_OBJS=rckskel.o rckskel_data_parallel.o rckskel_task_parallel.o rckskel_resolution.o utils.o rckskel_datatypes.o
RCKSKEL_INC=../src
RCKSKEL_ARCH=$(RCKSKEL_INC)/rckskel.a
RCKSKEL_CFLAGS=-g -I$(RCCEINCLUDE) -I$(RCKSKEL_INC)

test: test.o $(RCKSKEL_ARCH) $(RCCE_EMULATOR_ARCHIVE)
	$(CCOMPILE) -o test test.o $(RCKSKEL_ARCH) $(RCCE_EMULATOR_ARCHIVE) $(CFLAGS) $(LFLAGS)

test_seq: test_seq.o $(RCKSKEL_ARCH) $(RCCE_EMULATOR_ARCHIVE)
	$(CCOMPILE) -o test_seq test_seq.o $(RCKSKEL_ARCH) $(RCCE_EMULATOR_ARCHIVE) $(CFLAGS) $(LFLAGS)

test.o: test.c $(RCKSKEL_INC)/rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(CFLAGS) -I$(RCKSKEL_INC) test.c

test_seq.o: test_seq.c $(RCKSKEL_INC)/rckskel.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(CFLAGS) -I$(RCKSKEL_INC) test_seq.c
clean:
	rm *.o test

