#!/bin/sh -e

##
./scripts/build.sh
##
#valgrind -v --leak-check=full --show-reachable=yes --trace-children=yes 
#./RCCE_V1.0.13/rccerun  -nue $1 -f RCCE_V1.0.13/hosts/rc.hosts $PWD/test/test

#./RCCE_V1.0.13/rccerun  -nue $1 -f RCCE_V1.0.13/hosts/rc.hosts $PWD/test/test_seq


