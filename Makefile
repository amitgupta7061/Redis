CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -I src
LDFLAGS  := -lpthread
TARGET   := rediss

SRCS := src/main.cpp \
        src/store/kv_store.cpp \
        src/parser/command_parser.cpp \
        src/commands/command_handler.cpp \
        src/network/tcp_server.cpp \
        src/server/server_app.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
