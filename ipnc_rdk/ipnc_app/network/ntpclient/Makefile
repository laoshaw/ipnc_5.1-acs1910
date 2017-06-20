# Makefile for ntpclient/adjtimex
#
# Makefile by Uwe Beutin, modified by John Williams
# 
# ntpclient by Larry Dolittle

CC = $(BUILD_TOOL_PREFIX)gcc
	
EXEC = ntpclient

OBJS = ntpclient.o phaselock.o
INSTALLDIR = $(IPNC_EXEC_DIR)

CFLAGS += -Wall -O3 -D__USE_BSD
LDFLAGS += -s
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

install: $(EXEC)
	install -d $(INSTALLDIR)
	install $(EXEC) $(INSTALLDIR)

clean:
	rm -f $(EXEC) *.gdb *.elf *.o
