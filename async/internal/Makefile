# the compiler: gcc for C program, define as g++ for C++
CC = clang++
CXXFLAGS  = -Wall -O2 -std=c++11
INCLUDES =
LDFLAGS =
LIBS = -luv
SRC = uv_deque_test.cc ../Argument.cc

ifneq ($(shell uname),Darwin)
	LIBS += -pthread
endif

all: uv_deque_test

.PHONY: clean

clean:
	$(RM) uv_deque_test
	$(RM) -r *.dSYM

debug: CXXFLAGS += -g
debug: uv_deque_test

uv_deque_test: $(SRC)
	$(CC) $(INCLUDES) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS) $(LIBS)

test: uv_deque_test
	@./uv_deque_test