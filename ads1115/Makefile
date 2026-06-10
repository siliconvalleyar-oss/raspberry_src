CXX       ?= g++
CXXFLAGS  ?= -std=c++11 -Wall -Wextra -pedantic
INCLUDES  := -Iads1115
LDFLAGS   := -lwiringPi
SRCDIR    := ads1115
OBJDIR    := obj
BINDIR    := bin
SRCS      := $(SRCDIR)/ads1115.cc $(SRCDIR)/Adafruit_ADS1015.cpp
OBJS      := $(SRCS:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)
OBJS      := $(OBJS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET    := $(BINDIR)/ads1115_app

.PHONY: all clean

all: directories $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
