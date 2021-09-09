include ../Makefile.inc

GEN_EXE = 4-1 4-2

EXE = ${GEN_EXE} ${LINUX_EXE}

# https://stackoverflow.com/a/27838026

all-tests = 4-2-test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

4-2-test-files = file_with_hole copy_with_hole copy_by_cp

# I don't know why but number of blocks used by copy created by cp
# reported by stat -c %b is often significantly larger than the other two.

4-2-test : ${4-2-test-files}
	origin=$$(stat -c %s file_with_hole); hole=$$(stat -c %s copy_with_hole); cp=$$(stat -c %s copy_by_cp); \
	echo origin $$origin hole $$hole cp $$cp; \
	[ $$origin -eq $$hole ] && diff file_with_hole copy_with_hole && touch $@

file_with_hole : ../fileio/seek_io
	rm -f $@ \
    && ../fileio/seek_io $@ wa s2 wb s5 wc s111 wd s1000000 we \
    && dd if=/dev/zero bs=1k count=1 >> $@

../fileio/seek_io :
	cd ../fileio && make seek_io

copy_with_hole : 4-2 file_with_hole
	rm -f $@ && ./4-2 file_with_hole $@

copy_by_cp : file_with_hole
	rm -f $@ && cp file_with_hole $@

clean : 
	${RM} ${EXE} *.o ${4-2-test-files} ${all-tests}

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
