include ../Makefile.inc

LINUX_EXE = 12-1 12-2 12-3

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 12-1.test 12-2.test 12-3.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

12-1.test : 12-1
	sleep 1 & ./$< $${USER} && ./$< $${USER} $$! 2 && touch $@

12-2.test : 12-2.test.sh 12-2
	./$< ./12-2 $@

12-3.test : 12-3
	sudo ./$< /dev/null && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
