include ../Makefile.inc

OPTI_LEVEL = 0 1 2 3

6_2_EXE = $(patsubst %,6-2.O%,${OPTI_LEVEL})

GEN_EXE = ${6_2_EXE} 6-3.main

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 6-3.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

${6_2_EXE} : 6-2.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -O$(patsubst 6-2.O%,%,$@) -o $@ $<

6-3.main : 6-3.o

6-3.test : 6-3.main
	./6-3.main && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests}

showall :
	@ echo ${EXE} ${all-tests} 6-3.main

${EXE} : ${TLPI_LIB}		# True as a rough approximation
