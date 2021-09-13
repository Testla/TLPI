include ../Makefile.inc

GEN_EXE = 7-1 7-2.main

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 7-2.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

7-2.main : 7-2.main.c 7-2.o

7-2.test : 7-2.main
	./$< && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
