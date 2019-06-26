CFLAGS = -I../remoteApi -I../include -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -DDO_NOT_USE_SHARED_MEMORY

OS = $(shell uname -s)
ifeq ($(OS), Linux)
	CFLAGS += -D__linux
else
	CFLAGS += -D__APPLE__
endif

all: 
	@rm -f bin/*.*
	@rm -f *.o 
	g++ -c -o fuzzyset.o fuzzyset.cpp
	g++ -c -o lingvar.o lingvar.cpp
	g++ -c -o rule.o rule.cpp
	g++ -c -o fuzzy_control.o fuzzy_control.cpp
	g++ $(CFLAGS) -c DuckieTownClient.cpp -o DuckieTownClient.o
	gcc $(CFLAGS) -c ../remoteApi/extApi.c -o extApi.o
	gcc $(CFLAGS) -c ../remoteApi/extApiPlatform.c -o extApiPlatform.o
	@mkdir -p bin
	g++ fuzzyset.o lingvar.o rule.o fuzzy_control.o extApi.o extApiPlatform.o DuckieTownClient.o -o ../../vrep.app/Contents/MacOS/DuckieTownClient -lpthread #-I.
	@rm -f *.o 
