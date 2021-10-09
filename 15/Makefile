include ../Makefile.inc

15_1_test_bullets = a b c

15_1_tests = $(patsubst %,15-1.test.%,${15_1_test_bullets})

GEN_EXE = 15-4.main 15-6

LINUX_EXE = 15-3

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = ${15_1_tests} 15-2.test 15-3.test 15-4.test 15-6.test

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

15-2.test : 15-2.test.sh
	./$< && touch $@

15-3.test : 15-3
	./$< ./$< && echo "" && ./$< -l /proc/self/cwd && touch $@

15-4.main : 15-4.o

15-4.test : 15-4.test.sh 15-4.main
	./$< ./15-4.main && touch $@

15-6.test : 15-6.test.sh 15-6
	./$< ./15-6 && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
