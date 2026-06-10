TARGETS = sim800_main sim800_sms sim800_dtmf_tone \
          sim800_dtmf_02 sim800_dtmf_single \
          sim800_dtmf_detect sim800_dtmf_file_03 sim800_dtmf_old

CC = g++
CFLAGS = -Wall -std=c++11
BINDIR = bin

.PHONY: all clean

all: $(BINDIR) $(TARGETS)

$(BINDIR):
	mkdir -p $(BINDIR)

sim800_main: $(BINDIR)/sim800_main
sim800_sms: $(BINDIR)/sim800_sms
sim800_dtmf_tone: $(BINDIR)/sim800_dtmf_tone
sim800_dtmf_02: $(BINDIR)/sim800_dtmf_02
sim800_dtmf_single: $(BINDIR)/sim800_dtmf_single
sim800_dtmf_detect: $(BINDIR)/sim800_dtmf_detect
sim800_dtmf_file_03: $(BINDIR)/sim800_dtmf_file_03
sim800_dtmf_old: $(BINDIR)/sim800_dtmf_old

$(BINDIR)/sim800_main: sim800Cpp/main.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_sms: sim800Cpp/single_call_sim800/sim800.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_tone: sim800Cpp/dtmf_tone_01/sim800_dtmf_tone_date.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_02: sim800Cpp/dtmf_02/dtmf_tone_02.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_single: sim800Cpp/dtmf_singgle/sim800_dtmf.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_detect: sim800Cpp/detect_tone_dtmf/sim800_dtmf_detect_tone.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_file_03: sim800Cpp/dtmf_tone_file_03/dtmf_tone_03.cc
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/sim800_dtmf_old: sim800Cpp/olds_sim800/sim800_dtmf_get_tone_date.cc
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BINDIR)
