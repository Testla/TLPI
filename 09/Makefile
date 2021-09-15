include ../Makefile.inc

9_1_test_bullets = a b c d e

9_1_tests = $(patsubst %,9-1.test.%,${9_1_test_bullets})

GEN_EXE = 9-1 9-3.main 9-4 9-5

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = ${9_1_tests} 9-3.test 9-4.test 9-5.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

9-1 : 9-1.o ${TLPI_LIB}
	$(CC) $(CPPFLAGS) $(CFLAGS) $< ${TLPI_LIB} -o $@
	sudo chown root:root $@
	sudo chmod u+s $@

${9_1_tests} : 9-1
	./$< $(patsubst 9-1.test.%,%,$@) && touch $@

9-3.main : 9-3.o

9-3.test : 9-3.main
	sudo ./$< root && sudo ./$< $$(whoami) && touch $@

9-4 : 9-4.o ${TLPI_LIB}
	$(CC) $(CPPFLAGS) $(CFLAGS) $< ${TLPI_LIB} -o $@
	echo -n "Input user name for $@:" && read user && sudo chown $${user}:root $@
	sudo chmod u+s $@

9-4.test : 9-4
	./$< && touch $@

9-5 : 9-4
	rm -f $@
	cp ./$< $@
	sudo chown root:root $@
	sudo chmod u+s $@

9-5.test : 9-5
	./$< && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
