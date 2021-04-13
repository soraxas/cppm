EXEC=test

all: ${EXEC}
	./${EXEC}

${EXEC}: test.cpp cppm.hpp
	g++ test.cpp -Wall -o ${EXEC}

clean:
	rm -rf ${EXEC}
