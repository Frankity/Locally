CXX = g++
CXXFLAGS = -Wall -std=gnu++20
INCDIR = include
LIBS = -lws2_32 -lwsock32 -lssl -lcrypto
SRCDIR = src
BINDIR = bin

# Archivos fuente
SRCS = main.cpp \
       $(SRCDIR)/log.cpp \
       $(SRCDIR)/config.cpp \
       $(SRCDIR)/utils.cpp \
       $(SRCDIR)/websocket.cpp \
       $(SRCDIR)/filewatcher.cpp \
       $(SRCDIR)/apihandler.cpp \
       $(SRCDIR)/server.cpp

# Archivos objeto
OBJECTS = $(SRCS:.cpp=.o)

# Binario
TARGET = $(BINDIR)/locally.exe

# Build por defecto
all: debug

# Compilación modo debug
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Compilación modo release
release: CXXFLAGS += -O2
release: $(TARGET)

# Vincula el binario
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

# Compilar .cpp → .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Limpiar objetos y binario
clean:
	rm -f *.o
	rm -f $(SRCDIR)/*.o
	rm -f $(TARGET)

.PHONY: clean debug release all
