CXX       ?= g++
CXXFLAGS  ?= -Wall -O2 -Iinclude
LDLIBS    ?= -lpthread
SRCDIR    := src
BINDIR    := bin

.PHONY: all clean

all: directories $(BINDIR)/flush $(BINDIR)/app

$(BINDIR)/flush: $(SRCDIR)/flush.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

$(BINDIR)/app: $(SRCDIR)/main.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

directories: $(BINDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(BINDIR)/flush $(BINDIR)/app
