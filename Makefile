CXX      := g++
CXXFLAGS := -std=c++17 -O3 -march=native -funroll-loops \
            -Wall -Wextra -Wno-unused-parameter \
            -Isrc -Iagents

SRCS := src/main.cpp \
        src/matching_engine.cpp \
        src/market_data.cpp \
        src/order.cpp \
        agents/bot.cpp \
        agents/momentum_bot.cpp

TARGET := trading_sim

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread
	@echo "Built with O3 + march=native"

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)