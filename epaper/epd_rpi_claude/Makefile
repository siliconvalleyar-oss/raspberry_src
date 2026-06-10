CXX      = g++
CXXFLAGS = -O2 -std=c++14 -Wall -Wextra \
           -Iinclude \
           -include include/hV_List_Constants.h \
           -DFONT_MODE=USE_FONT_TERMINAL \
           -DMAX_FONT_SIZE=4 \
           -DUSE_EXT_BOARD=1 \
           -DBCM2835_NO_DELAY_COMPATIBILITY \
           -Dbcm2835_gpio_fsel_t=uint8_t \
           -DcharAt\(i\)=operator\[\]\(i\) \
           -Dsubstring\(a,b\)=substr\(a,\(b\)-\(a\)\) \
           -DtoCharArray\(buf,len\)=copy\(\(buf\),\(len\)-1,0\),\(buf\)\[\(len\)-1\]=0 \
           -DeScreen_EPD_343_PS_0B=0x01575042 \
           -DeScreen_EPD_350_KS_0C=0x015E4B43 \
           -DeScreen_EPD_437_KS_0C=0x01B54B43

LDFLAGS  = -lbcm2835

SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

TARGET   = $(BINDIR)/epd_demo

SRCS     = $(SRCDIR)/main.cpp \
           $(SRCDIR)/hV_HAL_Peripherals.cpp \
           $(SRCDIR)/hV_Board.cpp \
           $(SRCDIR)/hV_Utilities_Common.cpp \
           $(SRCDIR)/hV_Utilities_PDLS.cpp \
           $(SRCDIR)/hV_Font_Terminal.cpp \
           $(SRCDIR)/hV_Screen_Buffer.cpp \
           $(SRCDIR)/Screen_EPD_EXT3.cpp

OBJS     = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "Build successful! Run with: sudo ./$(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -f $(OBJS) $(TARGET)

install-dep:
	sudo apt-get update
	sudo apt-get install -y libbcm2835-dev

run: $(TARGET)
	sudo ./$(TARGET)

.PHONY: all clean install-dep run
