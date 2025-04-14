CXX = g++
CXXFLAGS = -std=c++17 #-Wall -Wextra 

TARGET = ipk25chat-client

all: $(TARGET)
	@echo "Project compiled succesfully!"

$(TARGET): $(wildcard src/*.cpp)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean