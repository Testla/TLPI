include ../Makefile.inc

GEN_EXE = 5-1 5-2 5-3 5-5 5-6 5-7

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 5-1.test 5-2.test 5-3.test 5-5.test 5-6.test 5-7.test

LARGE_FILE_SIZE = 5555555555

ATOMIC_APPEND_SIZE = 1000000

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

5-1.test : 5-1
	./5-1 $@ ${LARGE_FILE_SIZE}
	[ $$(stat -c %s $@) -eq $$(expr ${LARGE_FILE_SIZE} + 4) ] || (rm -f $@; false)

5-2.test : 5-2
	echo -n 123 > $@
	./5-2 $@
	[ $$(cat $@) = "123x" ] || (rm -f $@; false)

5-3.test.f1 : 5-3
	rm -f $@; ./5-3 $@ ${ATOMIC_APPEND_SIZE} & ./5-3 $@ ${ATOMIC_APPEND_SIZE}

5-3.test.f2 : 5-3
	rm -f $@; ./5-3 $@ ${ATOMIC_APPEND_SIZE} x & ./5-3 $@ ${ATOMIC_APPEND_SIZE} x

5-3.test : 5-3.test.f1 5-3.test.f2
	size1=$$(stat -c %s 5-3.test.f1); size2=$$(stat -c %s 5-3.test.f2); echo f1 $$size1 f2 $$size2; [ $$size1 -eq $$(expr ${ATOMIC_APPEND_SIZE} \* 2) ] && touch $@

5-5 : 5-4.o

5-5.test : 5-5
	./5-5 && touch $@

5-6.test : 5-6
	./5-6 $@
	[ $$(xxd -p $@) = $$(printf 'Giddayworld\0' | xxd -p) ] || (rm -f $@; false)

5-7.test : 5-7
	./5-7 && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests} 5-3.test*

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
