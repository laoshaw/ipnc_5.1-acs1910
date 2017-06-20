# Makefile for update
#

CC = $(BUILD_TOOL_PREFIX)gcc
	
EXEC = update.cgi

OBJS = update.o crc32.o
INSTALLDIR = $(IPNC_EXEC_DIR)
CFLAGS += -Wall -O3 -I ./ -I$(PUBLIC_INCLUDE_DIR)
LDFLAGS += -s
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

install: $(EXEC)
	cp $(EXEC) $(TARGET_FS)/var/www/

clean:
	rm -f $(EXEC) *.gdb *.elf *.o
