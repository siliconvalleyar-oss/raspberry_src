CXX      = g++
CXXFLAGS = -Wall -std=c++11 -Iinclude
LDFLAGS  =

SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

LIBSRC   = $(SRCDIR)/sim800.cpp
LIBOBJ   = $(OBJDIR)/sim800.o

TARGETS  = sms_send call_dtmf call

all: $(BINDIR) $(OBJDIR) $(TARGETS:%=$(BINDIR)/%)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BINDIR)/sms_send: $(OBJDIR)/sms_send.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BINDIR)/call_dtmf: $(OBJDIR)/call_dtmf.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BINDIR)/call: $(OBJDIR)/call.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

run-sms: $(BINDIR)/sms_send
	sudo $<

run-call: $(BINDIR)/call
	sudo $<

run-dtmf: $(BINDIR)/call_dtmf
	sudo $<

.PHONY: all clean run-sms run-call run-dtmf
