
EXEC=test

run: ${EXEC}
	./${EXEC}

${EXEC}: test.cpp cppm.hpp
	g++ test.cpp -Wall -o ${EXEC}
