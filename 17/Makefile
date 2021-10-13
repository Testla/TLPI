include ../Makefile.inc

LINUX_EXE = 17-1

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 17-1.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

17-1 : 17-1.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< ${TLPI_LIB} -lacl -o $@

17-1.test : 17-1.test.sh 17-1
	./$< ./17-1 && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
