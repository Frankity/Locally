CXX = g++
CXXFLAGS = -Wall -std=gnu++20
INCDIR = include
SRCDIR = src
BINDIR = bin

ifeq ($(OS),Windows_NT)
	LIBS = -lws2_32 -lwsock32 -lssl -lcrypto
	TARGET = $(BINDIR)/locally.exe
	MKDIR = if not exist $(BINDIR) mkdir $(BINDIR)
	RM = del /Q
else
	LIBS = -lssl -lcrypto
	TARGET = $(BINDIR)/locally
	MKDIR = mkdir -p $(BINDIR)
	RM = rm -f
endif

SRCS = main.cpp \
	   $(SRCDIR)/log.cpp \
	   $(SRCDIR)/config.cpp \
	   $(SRCDIR)/utils.cpp \
	   $(SRCDIR)/websocket.cpp \
	   $(SRCDIR)/filewatcher.cpp \
	   $(SRCDIR)/apihandler.cpp \
	   $(SRCDIR)/httpfilehandler.cpp \
	   $(SRCDIR)/server.cpp 

OBJECTS = $(SRCS:.cpp=.o)

all: debug

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

release: CXXFLAGS += -O2
release: $(TARGET)

$(TARGET): 
	   $(OBJECTS)
	   $(MKDIR)
	   $(CXX) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	   $(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	   $(RM) *.o
	   $(RM) $(SRCDIR)/*.o
	   $(RM) $(TARGET)

.PHONY: clean debug release all