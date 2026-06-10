CXX       ?= g++
CXXFLAGS  ?= -std=c++14 -Wall -O2 -Iinclude
SRCDIR    := src
INCDIR    := include
OBJDIR    := obj
BINDIR    := bin
SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
TARGET    := $(BINDIR)/epaper_app

.PHONY: all clean

all: directories $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) -o $@ $^
	@echo "Binario: $(TARGET)"

# Cambiado .cc a .cpp
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
