# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.59 2002/03/24 22:20:19 jnelson Exp $

.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean mrclean distclean depend all dist

INSTALLDIR = $(IPNC_EXEC_DIR)

GCC_FLAGS = -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual\
  -Wtraditional\
  -Wshadow\
  -Wconversion\
  -Waggregate-return\
  -Wmissing-prototypes\
  -Wnested-externs\
  -Wall \
  -Wundef -Wwrite-strings -Wredundant-decls -Winline

srcdir = .
VPATH = .:./../extras
LDFLAGS = -g -lpthread
ONVIF_SRC = onvif
LIBS = $(APP_LIB_DIR)/msg_util.a $(APP_LIB_DIR)/Appro_interface.a \
       $(APP_LIB_DIR)/file_msg_drv.a $(APP_LIB_DIR)/sysctrl.a \
       $(APP_LIB_DIR)/sys_msg_drv.a $(CMEM_LIB_DIR)/cmem.a470MV
ONVIF_LIB = $(ONVIF_SRC)/libonvif.a

CFLAGS = -O3 -pipe -Wall 
ONVIF1 = -DWITH_COOKIES -DWITH_ZLIB 
ONVIF2 =  -lz

CFLAGS += -I. -I$(PUBLIC_INCLUDE_DIR)

ifeq ($(IPNC_DEVICE), DMVA1)
CFLAGS += -D_DMVAx_
endif

ifeq ($(IPNC_DEVICE), DMVA2)
CFLAGS += -D_DMVAx_
endif

ifeq ($(IPNC_DEVICE), DM369)
CFLAGS += -D_DM369_
endif

ONVIF_SRC_FILES = $(ONVIF_SRC)/onvifC.c $(ONVIF_SRC)/onvifServer.c $(ONVIF_SRC)/stdsoap2.c $(ONVIF_SRC)/duration.c $(ONVIF_SRC)/onvif.c $(ONVIF_SRC)/onvifface.c
ONVIF_OBJ_FILES = $(ONVIF_SRC)/onvifC.o $(ONVIF_SRC)/onvifServer.o $(ONVIF_SRC)/stdsoap2.o $(ONVIF_SRC)/duration.o $(ONVIF_SRC)/onvif.o $(ONVIF_SRC)/onvifface.o

# Change these if necessary
YACC = bison -y 
LEX = flex 
CC = $(BUILD_TOOL_PREFIX)gcc 
CPP = $(BUILD_TOOL_PREFIX)gcc -E $(CFLAGS)
AR = $(BUILD_TOOL_PREFIX)ar rcs

SOURCES = alias.c boa.c buffer.c cgi.c cgi_header.c config.c escape.c \
	get.c hash.c ip.c log.c mmap_cache.c pipe.c queue.c read.c \
	request.c response.c select.c signals.c util.c sublog.c appro_api.c \
	net_config.c web_translate.c file_list.c auth.c para_list.c syslog_list.c accesslog_list.c actvcon_list.c tree.c psia_root.c \
	index.c description.c xadd.c xparse.c iface.c \
	$(ONVIF_SRC_FILES)	

OBJS = y.tab.o lex.yy.o $(SOURCES:.c=.o) timestamp.o


all:	boa boa_indexer

install: boa
	install boa $(INSTALLDIR)

boa:	$(OBJS) $(LIBS)
	$(AR) $(ONVIF_LIB) $(ONVIF_OBJ_FILES)
	$(CC) $(ONVIF1) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	
boa_indexer:	index_dir.o escape.o $(LIBS)
	$(CC) $(ONVIF1) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	rm -f $(OBJS) boa core lex.yy.c y.tab.c y.tab.h *~ boa_indexer index_dir.o $(ONVIF_LIB)

distclean:	mrclean

mrclean:	clean
	rm -f config.status config.cache config.h Makefile config.log

# parser dependencies

y.tab.c y.tab.h:	boa_grammar.y
	$(YACC) -d $^

lex.yy.c:	boa_lexer.l
	$(LEX) $^

# timestamp

timestamp.o:	$(SOURCES) boa_grammar.y boa_lexer.l

# depend stuff
.depend:
	$(CPP) -MM $(SOURCES) > .depend
        
depend:
	-rm -f .depend
	$(MAKE) .depend
        
-include .depend

# tags
tags:	$(SOURCES)
	ctags -o tags $^ *.h

# dist
dist:
	$(MAKE) clean
	./makedist.sh
        
# object dump
boa.objdump:    boa
	objdump --disassemble-all --source boa > $@

