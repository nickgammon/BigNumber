CFLAGS=-I../src/ -DSTD_INCLUDED

all: test

test: test.cpp number.o BigNumber.o
	$(CXX) test.cpp number.o BigNumber.o $(CFLAGS) -o test

number.o: ../src/BigNumber/number.c ../src/BigNumber/number.h
	$(CC) -c ../src/BigNumber/number.c $(CFLAGS)

BigNumber.o: ../src/BigNumber/BigNumber.cpp ../src/BigNumber/BigNumber.h
	$(CXX) -c ../src/BigNumber/BigNumber.cpp $(CFLAGS)

clean:
	$(RM) test *.o
