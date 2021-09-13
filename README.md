# TLPI

My answers to exercises in [*The Linux Programming Interface*](https://man7.org/tlpi/index.html).

## Building

Makefiles in subdirectories are written like the ones in TLPI source code. Contents of this repository should be placed alongside contents in tlpi-dist.

    cp -r <some-prefix>/tlpi-dist/* ./

Running make specifying Makefile.answers as makefile will build all executables and run all tests. Note that test of 5-1 takes up more than 5 gibibytes of space in WSL1including size of file holes.

    make -f Makefile.answers

Or you may want to build each subdirectory independently.

    cd lib
    make
    cd ../04
    make
