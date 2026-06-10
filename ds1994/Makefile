CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -Wall -Wextra -pedantic -pthread
SRCDIR    := src_deepseek
OBJDIR    := obj
BINDIR    := bin
SRCS      := $(SRCDIR)/main.cpp
OBJS      := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET    := $(BINDIR)/ds1994_app

.PHONY: all clean run

all: directories $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

run: $(TARGET)
	sudo $(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET)
