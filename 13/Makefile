include ../Makefile.inc

GEN_EXE = 13-5

EXE = ${GEN_EXE} ${LINUX_EXE}

all-tests = 13-5.test

all : allexe test

allexe : ${EXE}

allgen : ${GEN_EXE}

test : ${all-tests}

# TODO: Add more test cases.
13-5.test.data :
	dd if=/dev/urandom bs=1M count=1 | tr -cd "[:print:]\n" > $@

13-5.test : 13-5 13-5.test.data
	bash -c "time ./$< 13-5.test.data > 13-5.output.my"
	tail 13-5.test.data > 13-5.output.tail
	cmp -s 13-5.output.my 13-5.output.tail && touch $@

clean : 
	${RM} ${EXE} *.o ${all-tests} 13-5.test.data 13-5.output.*

showall :
	@ echo ${EXE} ${all-tests}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
