CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
TARGETS  := q4_receiver q4_sender
TEST     := test_q4

.PHONY: all clean sample test

all: $(TARGETS)

q4_receiver: q4_receiver.cpp q4_protocol.h
	$(CXX) $(CXXFLAGS) $< -o $@

q4_sender: q4_sender.cpp q4_protocol.h
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TEST)
	./$(TEST)

$(TEST): test_q4.cpp q4_protocol.h
	$(CXX) $(CXXFLAGS) $< -o $@

# Gera uma imagem PPM simples para simular o frame da camera.
sample:
	@printf 'P6\n64 64\n255\n' > imagem_camera.ppm
	@head -c 12288 /dev/urandom >> imagem_camera.ppm
	@echo "Imagem de exemplo gerada: imagem_camera.ppm"

clean:
	rm -f $(TARGETS) $(TEST)
