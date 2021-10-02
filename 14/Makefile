include ../Makefile.inc

GEN_EXE = 14-1

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 14-1.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

14-1.test : 14-1
	bash -c "time ./$< 1000000 . && time ./$< 1000000 . x && touch $@"

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
