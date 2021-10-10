include ../Makefile.inc

LINUX_EXE = 16-1

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 16-1.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

16-1.test : 16-1.test.sh 16-1
	./$< ./16-1 && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
