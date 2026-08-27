CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
LDFLAGS = -lutil -lpthread

SRCS = src/main.cpp src/server.cpp src/serial_port.cpp src/dictionary.cpp src/pattern.cpp
OBJS = $(SRCS:.cpp=.o)

all: at_server

at_server: $(OBJS)
$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

test: tests
./tests

tests: tests/test_main.cpp src/pattern.cpp src/dictionary.cpp
$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
rm -f $(OBJS) at_server tests

.PHONY: all clean test
