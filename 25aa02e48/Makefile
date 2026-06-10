CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -Wall -Wextra -pedantic -Iinclude
SRCDIR    := src
INCDIR    := include
OBJDIR    := obj
BINDIR    := bin
SRCS      := $(wildcard $(SRCDIR)/*.cc)
OBJS      := $(patsubst $(SRCDIR)/%.cc, $(OBJDIR)/%.o, $(SRCS))
TARGET    := $(BINDIR)/app_25aa

.PHONY: all clean

all: directories $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cc | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
