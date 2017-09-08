BSNCODE=000

SOURCE+=$(BUSINESS)/$(BSNCODE)/trn/trn.c
SOURCE+=$(BUSINESS)/$(BSNCODE)/pkg/pkg.c
SOURCE+=$(BUSINESS)/$(BSNCODE)/emu/emu.c

TARGET+=$(BUSINESS)/$(BSNCODE)/trn/libtrn.so
TARGET+=$(BUSINESS)/$(BSNCODE)/pkg/libpkg.so
TARGET+=$(BUSINESS)/$(BSNCODE)/emu/libemu.so

INCPATH=$(patsubst %,-I%,$(wildcard $(PLATFORM)/mod/*))

all: $(TARGET)

lib%.so:%.c
	@echo "编译模块[$(@F)]"
	gcc $< -o $@\
		-fPIC -shared\
		 $(INCPATH)

touch:
	@touch $(SOURCE)
clean:
	@rm -f $(TARGET)

install:
	@tar czvf $(BSNCODE).tgz\
		profile\
		$(BSNCODE).xml\
		dbs/dbs.ini\
		pkg/pkg.ini\
		pkg/libpkg.so\
		trn/libtrn.so
