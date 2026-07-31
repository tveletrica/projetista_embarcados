CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -pthread
TARGET   := q3
TEST     := test_q3
SRC      := q3.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TEST)
	./$(TEST)

# O teste inclui q3.cpp diretamente (main renomeado), logo compila so ele.
$(TEST): test_q3.cpp $(SRC)
	$(CXX) $(CXXFLAGS) test_q3.cpp -o $@

clean:
	rm -f $(TARGET) $(TEST)
