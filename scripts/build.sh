#!/bin/sh

build_lib()
{
    cd src; make; cd ..
}

build_test()
{
    cd test; make; cd ..
}

build_test_seq()
{
    cd test; make test_seq; cd ..
}
##############
# PROCESSING #
##############
build_lib
#build_test
#build_test_seq

