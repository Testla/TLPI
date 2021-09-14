include ../Makefile.inc

GEN_EXE = 8-2.main

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 8-2.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

8-2.main : 8-2.main.c 8-2.o

8-2.test : 8-2.main
	./$< && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
