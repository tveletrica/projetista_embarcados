CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -pthread
TARGET   := q2
TEST     := test_q2
SRC      := q2.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TEST)
	./$(TEST)

# O teste inclui q2.cpp diretamente (main renomeado), logo compila so ele.
$(TEST): test_q2.cpp $(SRC)
	$(CXX) $(CXXFLAGS) test_q2.cpp -o $@

clean:
	rm -f $(TARGET) $(TEST)
