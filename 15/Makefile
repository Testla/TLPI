include ../Makefile.inc

15_1_test_bullets = a b c

15_1_tests = $(patsubst %,15-1.test.%,${15_1_test_bullets})

GEN_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = ${15_1_tests}

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

15-1.test.a : 15-1.test.a.sh
	./$< && touch $@

15-1.test.b : 15-1.test.b.sh
	./$< && touch $@

15-1.test.c : 15-1.test.c.py
	./$< && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
