CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Isrc -Iagents

SRCS = src/main.cpp \
       src/market_data.cpp \
       src/matching_engine.cpp \
       src/flat_matching_engine.cpp \
       agents/bot.cpp \
       agents/momentum_bot.cpp

TARGET = trading_sim

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)