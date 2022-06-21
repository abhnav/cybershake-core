#!/bin/sh
# supply the testfile as an argument
testfile=$1
tar zxvf $testfile
./make_dirs.sh
mv awp.SMALL.cordfile  awp.SMALL.smoothed.media  SMALL_fx_src comp_x/input
