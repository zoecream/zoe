BSNCODE=001

TRNSOURCE=$(BUSINESS)/$(BSNCODE)/trn/trn.c
PKGSOURCE=$(BUSINESS)/$(BSNCODE)/pkg/pkg.c
EMUSOURCE=$(BUSINESS)/$(BSNCODE)/emu/emu.c

TRNTARGET=$(BUSINESS)/$(BSNCODE)/trn/libtrn.so
PKGTARGET=$(BUSINESS)/$(BSNCODE)/pkg/libpkg.so
EMUTARGET=$(BUSINESS)/$(BSNCODE)/emu/libemu.so

TARGET+=$(TRNTARGET)
TARGET+=$(PKGTARGET)
TARGET+=$(EMUTARGET)

all: $(TARGET)

$(TRNTARGET):$(TRNSOURCE)
	gcc -fPIC -shared $< -o $@ -I$(PLATFORM)/inc
$(PKGTARGET):$(PKGSOURCE)
	gcc -fPIC -shared $< -o $@ -I$(PLATFORM)/inc
$(EMUTARGET):$(EMUSOURCE)
	gcc -fPIC -shared $< -o $@ -I$(PLATFORM)/inc

touch:
	touch $(SOURCE)
clean:
	rm $(TARGET)
