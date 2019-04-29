ifeq ($(OS),Windows_NT)
	EXT=.exe
	PLATFORM_OPTS=-static
	PLATFORM_LD_OPTS=-Wl,--no-as-needed
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		EXT=
		PLATFORM_OPTS=
		PLATFORM_LD_OPTS=
	else
		EXT=
		PLATFORM_OPTS=
		PLATFORM_LD_OPTS=-lrt -Wl,--no-as-needed
	endif
endif

default: queue_test$(EXT)

queue_test$(EXT): queue_test.cpp queue.hpp readerwriterqueue/readerwriterqueue.h readerwriterqueue/atomicops.h makefile
	g++ $(PLATFORM_OPTS) -std=c++14 -Wpedantic -Wall -DNDEBUG -O3 queue_test.cpp -o queue_test$(EXT) -pthread $(PLATFORM_LD_OPTS)

run: queue_test$(EXT)
	./queue_test$(EXT)
