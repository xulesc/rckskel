#!/bin/sh

clean_lib()
{
    cd src; make clean; cd ..
}

clean_test()
{
    cd test; make clean; cd ..
}


##############
# PROCESSING #
##############
clean_lib
clean_test

