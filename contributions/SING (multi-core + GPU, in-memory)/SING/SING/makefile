






../lib/libsing.a: singlib.o
	ar -crv ../lib/libsing.a singlib.o
singlib.o: singlib.cu singlib.h
	nvcc -c singlib.cu -o singlib.o -lcudart -lcuda


#/usr/local/cuda/bin/
#hello.a:
#	ar -crv libhello.a libhello.o

#hello.o: hello.cu hello.h
#	g++ -c hello.cpp -o libhello.o

