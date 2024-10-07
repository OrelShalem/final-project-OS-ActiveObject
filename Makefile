CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
INCLUDES = -Icommon
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage --coverage

# Common sources and objects
COMMON_SRCS = $(wildcard common/*.cpp)
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)

# Server sources and objects
SERVER_SRCS = $(wildcard server/src/*.cpp)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
SERVER_TARGET = server_exe  

# Client sources and objects
CLIENT_SRCS = $(wildcard client/src/*.cpp)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
CLIENT_TARGET = client_exe

.PHONY: all clean coverage

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(COVERAGE_FLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJS) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(COVERAGE_FLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(COVERAGE_FLAGS) -c $< -o $@

clean:
	rm -f $(COMMON_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)
	find . -name "*.gcno" -o -name "*.gcda" | xargs rm -f

coverage: all
	# Run the test script
	./coverage.sh
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report