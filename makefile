INCPATH=$(patsubst %,-I%,$(wildcard $(PLATFORM)/mod/*))

THIRDINCPATH+=-I/usr/local/openssl/include
THIRDINCPATH+=-I/usr/local/unixodbc/include

THIRDLIBPATH+=-L/usr/local/openssl/lib -lcrypto
THIRDLIBPATH+=-L/usr/local/unixodbc/lib -lodbc
THIRDLIBPATH+=-lrt -ldl -rdynamic -lpthread

MODHEADER=$(sort $(wildcard $(PLATFORM)/mod/*/*.h))
MODSOURCE=$(sort $(wildcard $(PLATFORM)/mod/*/*.c))
MODTARGET=$(patsubst %.c,%.o,$(MODSOURCE))

UNTSOURCE=$(sort $(wildcard $(PLATFORM)/unt/*/*.c))
UNTTARGET=$(patsubst %.c,%,$(UNTSOURCE))

INSSOURCE=$(sort $(wildcard $(PLATFORM)/ins/*.c))
INSTARGET=$(patsubst %.c,%,$(INSSOURCE))

SRVSOURCE=$(sort $(wildcard $(PLATFORM)/srv/*.c))
SRVTARGET=$(patsubst %.c,%,$(SRVSOURCE))

OPTION?=-O0 -g
OPTION?=-O3 -DNDEBUG

all: $(MODTARGET) $(UNTTARGET) $(INSTARGET) $(SRVTARGET)

%.o:%.c $(MODSOURCE) $(MODHEADER)
	@echo "编译模块[$(@F)]"
	gcc $< -o $@ -c\
		$(OPTION)\
		$(INCPATH)\
		$(THIRDINCPATH)

%:%.c $(MODTARGET)
	@echo "编译工具[$(@F)]"
	gcc $< -o $@\
		$(OPTION)\
		$(INCPATH)\
		$(MODTARGET)\
		$(THIRDINCPATH)\
		$(THIRDLIBPATH)

touch:
	@touch $(MODSOURCE) $(UNTSOURCE) $(INSSOURCE) $(SRVSOURCE)
clean:
	@rm -f $(MODTARGET) $(UNTTARGET) $(INSTARGET) $(SRVTARGET)

install:
	@tar czvf zoe.tgz\
		profile\
		srv/zoe\
		ins/dbs
