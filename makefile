CXX = g++
CXXFLAGS = -Wall -std=c++11 -g -O0 -std=gnu++20
LIBS = -lws2_32 -lwsock32 -lssl -lcrypto 
SRCDIR = src
INCDIR = include
SOURCES = main.cpp $(SRCDIR)/config.cpp $(SRCDIR)/log.cpp $(SRCDIR)/utils.cpp $(SRCDIR)/filewatcher.cpp $(SRCDIR)/apihandler.cpp $(SRCDIR)/websocket.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = locally.exe

# Regla para build con debug
debug: CXXFLAGS += -DDEBUG
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	del /Q *.o $(SRCDIR)\*.o $(TARGET) 2>nul || true

.PHONY: clean debug