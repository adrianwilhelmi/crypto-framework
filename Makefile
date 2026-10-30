CXXFLAGS=-Wall -Wextra -Wpedantic -g -std=c++20 -Isrc -Itest -fconstexpr-ops-limit=100000000
CXX=g++

TARGET = run_tests
SRC = test/main.cpp test/test_fp.cpp test/test_fpk.cpp test/test_elliptic.cpp test/test_serial.cpp test/test_bigint.cpp
HEADERS = src/fp.hpp src/fpk.hpp src/utils.hpp src/polynomial_utils.hpp src/elliptic.hpp src/serializer.hpp src/bigint.hpp

.PHONY: all run clean

all:$(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(TARGET)
