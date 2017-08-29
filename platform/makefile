MODSRCPATH=$(PLATFORM)/src/mod
MODINCPATH=$(PLATFORM)/inc
MODLIBPATH=$(PLATFORM)/lib

UNTSRCPATH=$(PLATFORM)/src/unt
UNTBINPATH=$(PLATFORM)/bin/unt

INSSRCPATH=$(PLATFORM)/src/ins
INSBINPATH=$(PLATFORM)/bin/ins

SRVSRCPATH=$(PLATFORM)/src/srv
SRVBINPATH=$(PLATFORM)/bin/srv

SOURCE+=$(wildcard $(MODSRCPATH)/*.c)
SOURCE+=$(wildcard $(UNTSRCPATH)/*.c)
SOURCE+=$(wildcard $(SRVSRCPATH)/*.c)
SOURCE+=$(wildcard $(INSSRCPATH)/*.c)
SOURCE+=$(wildcard $(MODINCPATH)/*.h)

EXCEPT+=$(MODSRCPATH)/xml.c
EXCEPT+=$(UNTSRCPATH)/xml.c

TARGET+=$(patsubst $(MODSRCPATH)/%.c,$(MODLIBPATH)/lib%.a,$(filter-out $(EXCEPT),$(wildcard $(MODSRCPATH)/*.c)))
TARGET+=$(patsubst $(UNTSRCPATH)/%.c,$(UNTBINPATH)/%,$(filter-out $(EXCEPT),$(wildcard $(UNTSRCPATH)/*.c)))
TARGET+=$(patsubst $(INSSRCPATH)/%.c,$(INSBINPATH)/%,$(filter-out $(EXCEPT),$(wildcard $(INSSRCPATH)/*.c)))
TARGET+=$(patsubst $(SRVSRCPATH)/%.c,$(SRVBINPATH)/%,$(filter-out $(EXCEPT),$(wildcard $(SRVSRCPATH)/*.c)))

OPTION?=-O0 -g
OPTION?=-O3 -DNDEBUG

all:mkdir $(TARGET)

$(MODLIBPATH)/liblog.a:$(MODSRCPATH)/log.c\
                       $(MODINCPATH)/log.h
	@echo "编译模块[LOG]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)

$(MODLIBPATH)/libini.a:$(MODSRCPATH)/ini.c\
                       $(MODINCPATH)/ini.h
	@echo "编译模块[INI]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)

$(MODLIBPATH)/libmmp.a:$(MODSRCPATH)/mmp.c\
                       $(MODINCPATH)/mmp.h\
                       $(MODINCPATH)/log.h
	@echo "编译模块[MMP]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)

$(MODLIBPATH)/libpkg.a:$(MODSRCPATH)/pkg.c\
                       $(MODINCPATH)/pkg.h\
                       $(MODINCPATH)/mmp.h\
                       $(MODINCPATH)/log.h
	@echo "编译模块[PKG]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)\
		-I/usr/local/openssl/include

$(MODLIBPATH)/libdbs.a:$(MODSRCPATH)/dbs.c\
                       $(MODINCPATH)/dbs.h\
                       $(MODINCPATH)/log.h
	@echo "编译模块[DBS]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)\
		-I/usr/local/unixodbc/include

$(MODLIBPATH)/libjsn.a:$(MODSRCPATH)/jsn.c\
                       $(MODINCPATH)/jsn.h
	@echo "编译模块[JSN]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)

$(MODLIBPATH)/libxml.a:$(MODSRCPATH)/xml.c\
                       $(MODINCPATH)/xml.h
	@echo "编译模块[XML]"
	gcc $< -o $@ -c\
		$(OPTION)\
		-I$(MODINCPATH)

$(UNTBINPATH)/log:$(UNTSRCPATH)/log.c\
                  $(MODLIBPATH)/liblog.a
	@echo "编译测试[LOG]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-llog\
		-lpthread

$(UNTBINPATH)/ini:$(UNTSRCPATH)/ini.c\
                  $(MODLIBPATH)/libini.a\
                  $(MODLIBPATH)/liblog.a
	@echo "编译测试[INI]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-lini -llog\
		-lpthread

$(UNTBINPATH)/mmp:$(UNTSRCPATH)/mmp.c\
                  $(MODLIBPATH)/libmmp.a\
                  $(MODLIBPATH)/liblog.a
	@echo "编译测试[MMP]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-lmmp -llog\
		-lpthread

$(UNTBINPATH)/pkg:$(UNTSRCPATH)/pkg.c\
                  $(MODLIBPATH)/libpkg.a\
                  $(MODLIBPATH)/libjsn.a\
                  $(MODLIBPATH)/libmmp.a\
                  $(MODLIBPATH)/liblog.a
	@echo "编译测试[PKG]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-lpkg -ljsn -lmmp -llog\
		-L/usr/local/openssl/lib -lcrypto\
		-ldl -rdynamic -lpthread

$(UNTBINPATH)/dbs:$(UNTSRCPATH)/dbs.c\
                  $(MODLIBPATH)/libdbs.a\
                  $(MODLIBPATH)/liblog.a
	@echo "编译测试[DBS]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-ldbs -llog\
		-L/usr/local/unixodbc/lib -lodbc\
		-lpthread

$(UNTBINPATH)/jsn:$(UNTSRCPATH)/jsn.c\
                  $(MODLIBPATH)/libjsn.a
	@echo "编译测试[JSN]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-ljsn

$(UNTBINPATH)/xml:$(UNTSRCPATH)/xml.c\
                  $(MODLIBPATH)/libxml.a
	@echo "编译测试[XML]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-lxml

$(INSBINPATH)/emu:$(INSSRCPATH)/emu.c\
                  $(MODLIBPATH)/liblog.a\
                  $(MODLIBPATH)/libjsn.a\
                  $(MODLIBPATH)/libmmp.a\
                  $(MODLIBPATH)/libpkg.a
	@echo "编译工具[EMU]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-llog -ljsn -lmmp -lpkg\
		-L/usr/local/openssl/lib -lcrypto\
		-ldl -rdynamic -lpthread

$(INSBINPATH)/dbs:$(INSSRCPATH)/dbs.c\
                  $(MODLIBPATH)/liblog.a\
                  $(MODLIBPATH)/libdbs.a
	@echo "编译工具[DBS]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-llog -ldbs\
		-L/usr/local/unixodbc/lib -lodbc\
		-lpthread

$(SRVBINPATH)/zoe:$(SRVSRCPATH)/zoe.c\
                  $(MODLIBPATH)/liblog.a\
                  $(MODLIBPATH)/libini.a\
                  $(MODLIBPATH)/libmmp.a\
                  $(MODLIBPATH)/libpkg.a\
                  $(MODLIBPATH)/libdbs.a\
                  $(MODLIBPATH)/libjsn.a
	@echo "编译服务[ZOE]"
	gcc $< -o $@\
		$(OPTION)\
		-I$(MODINCPATH)\
		-L$(MODLIBPATH)\
		-llog -lini -lmmp -lpkg -ldbs -ljsn\
		-L/usr/local/openssl/lib -lcrypto\
		-L/usr/local/unixodbc/lib -lodbc\
		-lrt -ldl -rdynamic -lpthread

mkdir:
	@mkdir -p $(MODLIBPATH)
	@mkdir -p $(UNTBINPATH)
	@mkdir -p $(INSBINPATH)
	@mkdir -p $(SRVBINPATH)
touch:
	touch $(SOURCE)
clean:
	rm $(TARGET)
