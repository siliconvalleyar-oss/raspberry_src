CXX       ?= g++
CXXFLAGS  ?= -Wall -O2 -Iinclude
LDLIBS    ?= -li2c
SRCDIR    := src
BINDIR    := bin
TARGETS   := ds1994_reader ds2482_reader

.PHONY: all clean

all: directories $(BINDIR)/ds1994_reader $(BINDIR)/ds2482_reader

$(BINDIR)/ds1994_reader: $(SRCDIR)/ds1994.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)
	@echo "Binario: $@"

$(BINDIR)/ds2482_reader: $(SRCDIR)/ds2482Ds1994.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)
	@echo "Binario: $@"

directories: $(BINDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)
