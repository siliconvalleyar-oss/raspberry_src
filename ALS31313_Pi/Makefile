CC        ?= gcc
CFLAGS    ?= -std=c11 -Wall -Wextra -pedantic -D_BSD_SOURCE
SRCDIR    := src
OBJDIR    := obj
BINDIR    := bin
SRCS      := $(SRCDIR)/als31313.c
OBJS      := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET    := $(BINDIR)/als31313

.PHONY: all clean

all: directories $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
